#include "hantek6022.h"
#include "ht6022lib.h"

static const std::vector<size_t> rangeSampleRate = std::vector<size_t>{ 100, 200, 500, 1'000, 4'000, 8'000, 16'000, 24'000 };

static const std::vector<size_t> rangeInputLevel = std::vector<size_t>{ 1, 2, 5, 10 };

static const std::vector<size_t> rangeSignalFrame = std::vector<size_t>{
    oscilloscopes::hantek::Hantek6022::_1KB,
    oscilloscopes::hantek::Hantek6022::_2KB,
    oscilloscopes::hantek::Hantek6022::_4KB,
    oscilloscopes::hantek::Hantek6022::_8KB,
    oscilloscopes::hantek::Hantek6022::_16KB,
    oscilloscopes::hantek::Hantek6022::_32KB,
    oscilloscopes::hantek::Hantek6022::_64KB,
    oscilloscopes::hantek::Hantek6022::_128KB,
    oscilloscopes::hantek::Hantek6022::_256KB,
    oscilloscopes::hantek::Hantek6022::_512KB,
    oscilloscopes::hantek::Hantek6022::_1MB
};

static const std::string oscilloscopeInfo = std::string("Hantek 6022BE");

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

inline bool oscilloscopes::hantek::Hantek6022::isCurrentSize( const size_t& s ) const
{
    using HT = oscilloscopes::hantek::Hantek6022;
    if( ( ( s == HT::_1KB) || ( s == HT::_2KB ) || ( s == HT::_4KB) || ( s == HT::_8KB)
        || ( s == HT::_16KB) || ( s == HT::_32KB) || ( s == HT::_64KB) || ( s == HT::_128KB)
        || ( s == HT::_256KB) || ( s == HT::_512KB) || ( s == HT::_1MB) ) )
    {
        return true;
    }    
    return false;
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
    case 1:
        return _1V;
    case 2:
        return _2V;
    case 5:
        return _5V;
    case 10:
        return _10V;
    };
    throw InvalidParamOscilloscope("setLevelForOscilloscope");
    return -1;
}

uint8_t oscilloscopes::hantek::Hantek6022::setScaleForOscilloscope( const size_t& scale )
{
    switch( scale )
    {
    case 100:
        return _100KSa;
    case 200:
        return _200KSa;
    case 500:
        return _500KSa;
    case 1'000:
        return _1MSa;
    case 4'000:
        return _4MSa;
    case 8'000:
        return _8MSa;
    case 16'000:
        return _16MSa;
    case 24'000:
        return _24MSa;
    };
    throw InvalidParamOscilloscope("setLevelForOscilloscope");
    return -1;
}

uint8_t oscilloscopes::hantek::Hantek6022::checkLevel( const float& level, const size_t& inLevel )
{
    if( ( ( ( -5.0 < level ) && ( level < 5.0 )) && ( inLevel == 1 ) ) )
        return 0;
    if( ( ( ( -10.0 < level ) && ( level < 10.0 )) && ( inLevel == 2 ) ) )
        return 0;
    if( ( ( ( -25.0 < level ) && ( level < 25.0 )) && ( inLevel == 5 ) ) )
        return 0;
    if( ( ( ( -35.0 < level ) && ( level < 35.0 )) && ( inLevel == 10 ) ) )
        return 0;
    return 1;
}

oscilloscopes::hantek::Hantek6022::Hantek6022( const size_t& SR, const size_t& IL1, const size_t& IL2 ) :
    triggerMustWork(true), userWantReadFrameSignal(false)
{
    init( SR, IL1, IL2 );
}

oscilloscopes::hantek::Hantek6022::~Hantek6022()
{
    close();
}

const size_t oscilloscopes::hantek::Hantek6022::getChannelsSize() const
{
    return 2;
}

const std::string oscilloscopes::hantek::Hantek6022::whoAmI() const
{
    return oscilloscopeInfo;
}

size_t oscilloscopes::hantek::Hantek6022::setSampleRate( const size_t& SR )
{
    {
        std::lock_guard<std::mutex> guard1( _usb_save, std::adopt_lock );
        {
            std::lock_guard<std::mutex> guard2( _usb_device, std::adopt_lock );

            THROW( ( _device._handle == nullptr ), "setSampleRate" );
            uint8_t SampleRate = setScaleForOscilloscope(SR);
            int r = libusb_control_transfer( _device._handle, HT6022_SR_REQUEST_TYPE, HT6022_SR_REQUEST,
                                             HT6022_SR_VALUE, HT6022_SR_INDEX, &SampleRate, HT6022_SR_SIZE,
                                             0 );

            THROW( ( ( r >= 0 ) ? 0 : r), "setSampleRate" );
        }
    }

    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );
    std::lock( _oscSignal_save, _oscSignal_osc );

    _oscSignal[0]._sampleRate = SR;
    _oscSignal[1]._sampleRate = SR;
    return _oscSignal[1]._sampleRate;
}

const size_t oscilloscopes::hantek::Hantek6022::getSampleRate()
{
    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );
    std::lock( _oscSignal_save, _oscSignal_osc );

    return _oscSignal[0]._sampleRate;
}

const std::vector<size_t> oscilloscopes::hantek::Hantek6022::getRangeSampleRate() const
{
    return rangeSampleRate;
}

size_t oscilloscopes::hantek::Hantek6022::setInputLevel( const uint8_t& CHx, const size_t& IL )
{
    {
        std::lock_guard<std::mutex> guard1( _usb_save, std::adopt_lock );
        {
            std::lock_guard<std::mutex> guard2( _usb_device, std::adopt_lock );

            THROW( ( ( ( CHx != 0 ) && ( CHx != 1 ) ) || ( _device._handle == nullptr ) ), "setInputLevel" );
            uint8_t InputRange = setLevelForOscilloscope(IL);
            int r = libusb_control_transfer( _device._handle,
                                             ( (CHx) ? HT6022_IR1_REQUEST_TYPE : HT6022_IR2_REQUEST_TYPE ),
                                             ( (CHx) ? HT6022_IR1_REQUEST : HT6022_IR2_REQUEST ),
                                             ( (CHx) ? HT6022_IR1_VALUE : HT6022_IR2_VALUE ),
                                             ( (CHx) ? HT6022_IR1_INDEX : HT6022_IR2_INDEX ),
                                             &InputRange, ( (CHx) ? HT6022_IR1_SIZE : HT6022_IR2_SIZE ), 0 );

            THROW( ( ( r >= 0 ) ? 0 : r), "setInputLevel" );
        }
    }

    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );

    _oscSignal[CHx]._inputLevel = IL;
    return _oscSignal[CHx]._inputLevel;
}

const size_t oscilloscopes::hantek::Hantek6022::getInputLevel( const uint8_t& CHx )
{
    THROW( ( ( ( CHx != 0 ) && ( CHx != 1 ) ) ), "getInputLevel" );

    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );
    std::lock( _oscSignal_save, _oscSignal_osc );
    return _oscSignal[CHx]._inputLevel;
}


const std::vector<size_t> oscilloscopes::hantek::Hantek6022::getRangeInputLevel() const
{
    return rangeInputLevel;
}

oscilloscopes::OscSigframe oscilloscopes::hantek::Hantek6022::getSignalFrame( const size_t& FS )
{
    //THROW( ( isCurrentSize( FS ) ), "getSignalFrame" );

    uint8_t *data = new uint8_t[( sizeof(uint8_t) * FS * 2 )];
    int r = 0;
    int nread;

    userWantReadFrameSignal = true;
    {
        std::lock_guard<std::mutex> guard1( _usb_save, std::adopt_lock );
        {
            std::lock_guard<std::mutex> guard2( _usb_device, std::adopt_lock );

            THROW( ( ( _device._handle == nullptr ) ), "getSignalFrame" );
            *data = HT6022_READ_CONTROL_DATA;
            r = libusb_control_transfer( _device._handle, HT6022_READ_CONTROL_REQUEST_TYPE,
                                         HT6022_READ_CONTROL_REQUEST, HT6022_READ_CONTROL_VALUE,
                                         HT6022_READ_CONTROL_INDEX, data, HT6022_READ_CONTROL_SIZE, 0 );
    
            THROW( ( ( r >= 0 ) ? 0 : r), "getSignalFrame", data );

            r = libusb_bulk_transfer( _device._handle, HT6022_READ_BULK_PIPE, data, ( FS * 2 ), &nread, 0 );

            THROW( ( ( r >= 0 ) ? 0 : r), "getSignalFrame", data );
            THROW( ( ( nread != ( FS * 2 ) ) ? -1 : 0 ), "getSignalFrame", data );
        }
    }
    userWantReadFrameSignal = false;

    uint8_t *data_temp = data;
    std::vector<int> signalCh0, signalCh1;
    for( size_t i = 0; i < FS; ++i )
    {
        signalCh0.push_back( static_cast<int>( ( *data_temp++ ) - 127 ) );
        signalCh1.push_back( static_cast<int>( ( *data_temp++ ) - 127 ) );
    }
    delete []data;

    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );

    _oscSignal[0]._signal = signalCh0;
    _oscSignal[1]._signal = signalCh1;
    _oscSignal[0]._signalSize = FS;
    _oscSignal[1]._signalSize = FS;
    return _oscSignal;
}

const std::vector<size_t> oscilloscopes::hantek::Hantek6022::getRangeSignalFrame() const
{
    return rangeSignalFrame;
}

oscilloscopes::OscSignal oscilloscopes::hantek::Hantek6022::getSignalFromTrigger( const uint8_t& CHx,
        const int& level, const int& comp )
{
    //std::cout << ">" << ((int)CHx) << ' ' << level << 
    size_t inputLevel = 0;
    {
        std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
        {
            std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );

            inputLevel = _oscSignal[CHx]._inputLevel;
        }
    }

    //THROW( ( ( ( CHx != 0 ) && ( CHx != 2 ) ) || ( checkLevel( level, inputLevel ) )
    //            || ( ( comp != 1 ) && ( comp != 2 ) ) ), "getSignalFromTrigger" );


    const size_t DATA_SIZE = _16KB;
    const size_t STEP = 512;

    // Размер буфера, который дополнительно сохраняем после срабатывания программного триггера
    const size_t BUFFER_SIGNAL_SIZE = 1000;
    size_t eraseSize = 0;  // длина удаляемого хвоста сигнал

    bool isTriggerSuccess = true;
    bool lastRead = true;
    std::vector<int> signal;

    while( ( ( isTriggerSuccess || lastRead ) && ( triggerMustWork ) ) )
    {
        if( !isTriggerSuccess )
            lastRead = false;

        uint8_t *data = new uint8_t[( sizeof(uint8_t) * DATA_SIZE * 2 )];
        *data = HT6022_READ_CONTROL_DATA;

        int r = 0;
        int nread;

        if( userWantReadFrameSignal )
            std::this_thread::sleep_for( std::chrono::nanoseconds(50) );

        {
            std::lock_guard<std::mutex> guard1( _usb_save, std::adopt_lock );
            {
                std::lock_guard<std::mutex> guard2( _usb_device, std::adopt_lock );

                THROW( ( ( _device._handle == nullptr ) ), "getSignalFromTrigger" );
                r = libusb_control_transfer( _device._handle, HT6022_READ_CONTROL_REQUEST_TYPE,
                                             HT6022_READ_CONTROL_REQUEST, HT6022_READ_CONTROL_VALUE,
                                             HT6022_READ_CONTROL_INDEX, data, HT6022_READ_CONTROL_SIZE, 0 );

                THROW( ( ( r >= 0 ) ? 0 : r), "getSignalFromTrigger", data );

                r = libusb_bulk_transfer( _device._handle, HT6022_READ_BULK_PIPE, data,
                                          ( DATA_SIZE * 2 ), &nread, 0 ); 

                THROW( ( ( r >= 0 ) ? 0 : r), "getSignalFromTrigger", data );
                THROW( ( ( nread != ( DATA_SIZE * 2 ) ) ? -1 : 0 ), "getSignalFromTrigger", data );
            }
        }

        uint8_t *data_temp = data;

        if( CHx == 1 )
            data_temp++;

        int intLevel = static_cast<int>( level );
        int buffer = 0;
        for( size_t i = 0; i < DATA_SIZE; ++i )
        {
            signal.push_back( static_cast<int>( ( *data_temp ) - 127 ) );
            data_temp += 2;
            if( ( ( i % STEP == 0 ) && isTriggerSuccess ) )
            {
                if( ( ( signal.back() >= intLevel ) && ( comp == 1 ) ) )  // Надо заменить на bool
                {
                    isTriggerSuccess = false;
                    eraseSize = signal.size();
                }
        
                if( ( ( signal.back() >= intLevel ) && ( comp == 2 ) ) )
                    buffer = signal.back();
                
                if( ( ( signal.back() <= intLevel ) && ( comp == 2 ) && ( buffer >= intLevel ) ) )
                {
                    isTriggerSuccess = false;
                    eraseSize = signal.size();
                }
            }
        }

        if( isTriggerSuccess )
        {
            if( signal.size() > _1MB )
                signal.clear();
        }

        delete []data;
    }

    if( !triggerMustWork )
    {
        OscSignal oscSig = oscilloscopes::OscSignal();
        return oscSig;
    }

    eraseSize -= ( ( eraseSize > BUFFER_SIGNAL_SIZE ) ? BUFFER_SIGNAL_SIZE : eraseSize );
    signal.erase( signal.begin(), signal.begin() + eraseSize );

    std::lock_guard<std::mutex> guard1( _oscSignal_save, std::adopt_lock );
    std::lock_guard<std::mutex> guard2( _oscSignal_osc, std::adopt_lock );
    std::lock( _oscSignal_save, _oscSignal_osc );

    _oscSignal[CHx]._signal = signal;
    _oscSignal[CHx]._signalSize = _oscSignal[CHx]._signal.size();
    return _oscSignal[CHx];
}

const void oscilloscopes::hantek::Hantek6022::onTrigger()
{
    if( !triggerMustWork )
        triggerMustWork = true;
}

const void oscilloscopes::hantek::Hantek6022::offTrigger()
{
    if( triggerMustWork )
        triggerMustWork = false;
}


