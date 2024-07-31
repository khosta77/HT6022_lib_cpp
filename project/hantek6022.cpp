#include "hantek6022.h"
#include "ht6022lib.h"

static std::pair<int, uint8_t> findDevice( libusb_device **DeviceList, const int& DEVICE_COUNT,
                                           const int& VENDOR_ID, const int& MODEL )
{
    struct libusb_device_descriptor desc;
    for( int deviceIterator = 0; deviceIterator < DEVICE_COUNT; ++deviceIterator )
    {
        uint8_t address = libusb_get_device_address(DeviceList[deviceIterator]);
        if( HT6022_AddressList[address] == 0 )
        {
            if( libusb_get_device_descriptor( DeviceList[deviceIterator], &desc ) == 0 )  // VID и PID
            {
                if( ( desc.idVendor  == VENDOR_ID ) && ( desc.idProduct == MODEL ) )
                    return std::pair<int, uint8_t>( deviceIterator, address);
            }
        }
    }
    return std::pair<int, uint8_t>( DEVICE_COUNT, 0 );
}

void oscilloscopes::hantek::Hantek6022::init_usb()
{
    if( libusb_init(nullptr) != 0 )
        throw OscilloscopeException("В методе init_usb произошло что-то неизвестное!!!");
}
 
int oscilloscopes::hantek::Hantek6022::firmwareUpload()
{
    libusb_device **DeviceList;
    const int DEVICE_COUNT = libusb_get_device_list( nullptr, &DeviceList );
    if( DEVICE_COUNT <= 0 ) 
        throw OscilloscopeException("Метод firmwareUpload, подключенные девайсы отстутствуют.");

    int deviceIter = findDevice( DeviceList, DEVICE_COUNT, HT6022_FIRMWARE_VENDOR_ID, HT6022_MODEL ).first;
    if( deviceIter == DEVICE_COUNT )
    {
        libusb_free_device_list( DeviceList, 1 );
        return -1;
    }

    libusb_device_handle *Dev_handle;
    int r = libusb_open( DeviceList[deviceIter], &Dev_handle );
    libusb_free_device_list(DeviceList, 1);

    if( r == oscilloscopes::OSCILLOSCOPE_ERROR_ACCESS )  // если в UNIX не использовать sudo будет ошибка доступа
#ifdef __linux__
        throw OscilloscopeException("В методе firmwareUpload произошло что-то неизвестное!!! Возможно надо использовать sudo");
#else
        throw AccessOscilloscopeException("firmwareUpload()");
#endif


    if( libusb_kernel_driver_active( Dev_handle, 0 ) == 1 )
    {
        if( libusb_detach_kernel_driver( Dev_handle, 0 ) != 0 )
        {
            libusb_close(Dev_handle);
            throw OscilloscopeException("В методе firmwareUpload функция libusb_detach_kernel_driver произошло что-то неизвестное!!!");
        }
    }

    if( libusb_claim_interface( Dev_handle, 0 ) < 0 )
    {
        libusb_close(Dev_handle);
        throw OscilloscopeException("В методе firmwareUpload функция libusb_claim_interface произошло что-то неизвестное!!!");
    }

    int n = HT6022_FIRMWARE_SIZE;
    uint8_t *Firmware = HT6022_Firmware;
    unsigned int Size = 0;
    unsigned int Value = 0;
    while(n)
    {
        Size  = ( *Firmware + ( ( *( Firmware + 1 ) ) << 0x08 ) );
        Firmware += 2;
        Value = ( *Firmware + ( ( *( Firmware + 1 ) ) << 0x08 ) );
        Firmware += 2;

        if( libusb_control_transfer( Dev_handle, HT6022_FIRMWARE_REQUEST_TYPE, HT6022_FIRMWARE_REQUEST,
                                     Value, HT6022_FIRMWARE_INDEX, Firmware, Size, 0 ) != Size )
        {
            libusb_release_interface( Dev_handle, 0 );
            libusb_close(Dev_handle);
            throw OscilloscopeException("В методе firmwareUpload в цикле произошло что-то неизвестное!!!");
        }
        Firmware += Size;
        --n;
    }

    libusb_release_interface( Dev_handle, 0 );
    libusb_close(Dev_handle);
    return 0;
}

void oscilloscopes::hantek::Hantek6022::openDevice()
{
    libusb_device **DeviceList;
    libusb_device_handle *DeviceHandle;

    const int DeviceCount = libusb_get_device_list( nullptr, &DeviceList );
    if( DeviceCount <= 0 ) 
        throw OscilloscopeException("Метод openDevice, подключенные девайсы отстутствуют.");

    auto AddIt = findDevice( DeviceList, DeviceCount, HT6022_VENDOR_ID, HT6022_MODEL );
    int DeviceIterator = AddIt.first;
    uint8_t Address = AddIt.second;

    if( DeviceIterator == DeviceCount )
    {
        libusb_free_device_list( DeviceList, 1 );
        throw NoDeviceOscilloscope("openDevice");
    }

    int r = libusb_open( DeviceList[DeviceIterator], &DeviceHandle );
    libusb_free_device_list(DeviceList, 1);
    
    THROW( r, "openDevice" );

    if( libusb_kernel_driver_active( DeviceHandle, 0 ) == 1 )
    {
        if( libusb_detach_kernel_driver( DeviceHandle, 0 ) != 0 )
        {
            libusb_close(DeviceHandle);
            throw OscilloscopeException("В методе openDevice, функция libusb_detach_kernel_driver вернула не 0");
        }
    }

    if( libusb_claim_interface( DeviceHandle, 0 ) != 0 )
    {
        libusb_close(DeviceHandle);
        throw OscilloscopeException("В методе openDevice, функция libusb_claim_interface вернула не 0");
    }

    HT6022_AddressList[Address] = 0x01;	
    _device._address = Address;
    _device._handle = DeviceHandle;
}

void oscilloscopes::hantek::Hantek6022::closeDevice()
{
    if( _device._handle != nullptr )
    {
        libusb_release_interface( _device._handle, 0 );
        libusb_close(_device._handle);
        HT6022_AddressList[_device._address] = 0x00;
        _device._handle = nullptr;
        _device._address = 0;
    }
}

void oscilloscopes::hantek::Hantek6022::readData( uint8_t* CH1, uint8_t* CH2, const size_t& DS,
                                                  size_t  TimeOut )
{
    THROW( ( ( _device._handle == nullptr ) || ( CH1 == nullptr ) || ( CH2 == nullptr ) ), "readData" );

    uint8_t *data = new uint8_t[( sizeof(uint8_t) * DS * 2 )];
    THROW( ( data == nullptr ), "readData" );

    *data = HT6022_READ_CONTROL_DATA;
    int r = libusb_control_transfer( _device._handle, HT6022_READ_CONTROL_REQUEST_TYPE,
                                     HT6022_READ_CONTROL_REQUEST, HT6022_READ_CONTROL_VALUE,
                                     HT6022_READ_CONTROL_INDEX, data, HT6022_READ_CONTROL_SIZE, 0 );

    THROW( ( ( r >= 0 ) ? 0 : r), "readData", data );

    int nread;
    r = libusb_bulk_transfer( _device._handle, HT6022_READ_BULK_PIPE, data, ( DS * 2 ), &nread, TimeOut ); 

    THROW( ( ( r >= 0 ) ? 0 : r), "readData", data );
    THROW( ( ( nread != ( DS * 2 ) ) ? -1 : 0 ), "readData", data );

    uint8_t *data_temp = data;
    while(nread)
    {
        *CH1++ = *data_temp++;
        *CH2++ = *data_temp++;
        nread -= 2;
    }

    delete []data;
}

void oscilloscopes::hantek::Hantek6022::init( const size_t& SR, const size_t& IL1, const size_t& IL2 )
{
    init_usb();
    if( firmwareUpload() == 0 )  // Минимальная задержка при иницализации
        std::this_thread::sleep_for(std::chrono::nanoseconds(5'000'000'000));
    openDevice();

    _oscSignal[0] = oscilloscopes::OscSignal();
    _oscSignal[1] = oscilloscopes::OscSignal();

    setSampleRate(SR);
    setInputLevel( 0, IL1 );
    setInputLevel( 1, IL2 );
}

void oscilloscopes::hantek::Hantek6022::close()
{
    closeDevice();
    libusb_exit(nullptr);
}

uint8_t oscilloscopes::hantek::Hantek6022::setLevelForOscilloscope( const size_t& level )
{
    switch( level )
    {
    case 1'000:
        return _1V;
    case 2'000:
        return _2V;
     case 5'000:
        return _5V;
     case 10'000:
        return _10V;
    };
    throw InvalidParamOscilloscope("setLevelForOscilloscope");
    return -1;
}

oscilloscopes::hantek::Hantek6022::Hantek6022( const size_t& SR, const size_t& IL1, const size_t& IL2 )
{
    init( SR, IL1, IL2 );
}

oscilloscopes::hantek::Hantek6022::~Hantek6022()
{
    close();
}

void oscilloscopes::hantek::Hantek6022::setSampleRate( const size_t& SR )
{
    THROW( ( _device._handle == nullptr ), "setSampleRate" );
    uint8_t SampleRate = SR;
    int r = libusb_control_transfer( _device._handle, HT6022_SR_REQUEST_TYPE, HT6022_SR_REQUEST,
                                     HT6022_SR_VALUE, HT6022_SR_INDEX, &SampleRate, HT6022_SR_SIZE, 0 );

    THROW( ( ( r >= 0 ) ? 0 : r), "setSampleRate" );

    _oscSignal[0]._sampleRate = SR;
    _oscSignal[1]._sampleRate = SR;
}

void oscilloscopes::hantek::Hantek6022::setInputLevel( const uint8_t& CHx, const size_t& IL )
{
    THROW( ( ( ( CHx != 0 ) && ( CHx != 1 ) ) || ( _device._handle == nullptr ) ), "setInputLevel" );

    uint8_t InputRange = setLevelForOscilloscope(IL);
    int r = libusb_control_transfer( _device._handle,
                                     ( (CHx) ? HT6022_IR1_REQUEST_TYPE : HT6022_IR2_REQUEST_TYPE ),
                                     ( (CHx) ? HT6022_IR1_REQUEST : HT6022_IR2_REQUEST ),
                                     ( (CHx) ? HT6022_IR1_VALUE : HT6022_IR2_VALUE ),
                                     ( (CHx) ? HT6022_IR1_INDEX : HT6022_IR2_INDEX ),
                                     &InputRange, ( (CHx) ? HT6022_IR1_SIZE : HT6022_IR2_SIZE ), 0 );

    THROW( ( ( r >= 0 ) ? 0 : r), "setInputLevel" );
    _oscSignal[CHx]._sampleRate = IL;
}

oscilloscopes::OscSigframe oscilloscopes::hantek::Hantek6022::getSignalFrame( const size_t& FS )
{
    _oscSignal[0]._signal.clear();
    _oscSignal[1]._signal.clear();

    uint8_t *CH1 = new uint8_t[FS];
    uint8_t *CH2 = new uint8_t[FS];

    readData( CH1, CH2, FS, 0 );
    for( size_t i = 0; i < FS; ++i )
    {
        _oscSignal[0]._signal.push_back( ((int)CH1[i]) );
        _oscSignal[1]._signal.push_back( ((int)CH2[i]) );
    }

    _oscSignal[0]._signalSize = FS;
    _oscSignal[1]._signalSize = FS;

    return _oscSignal;
}


