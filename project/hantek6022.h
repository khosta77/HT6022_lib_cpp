#ifndef OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_
#define OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_

#include "oscilloscopes.h"

#include <libusb-1.0/libusb.h>
#include "ht6022lib.h"//удалить позже

namespace oscilloscopes
{

    namespace hantek
    {

        class Hantek6022 final : public Oscilloscope
        {
        public:
            // Размер буфера считываемой памяти
            static constexpr size_t _1KB   = 0x00000400;  // 1024 Bytes
            static constexpr size_t _2KB   = 0x00000800;  // 2048 Bytes
            static constexpr size_t _4KB   = 0x00001000;  // 4096 Bytes
            static constexpr size_t _8KB   = 0x00002000;  // 8192 Bytes
            static constexpr size_t _16KB  = 0x00004000;  // 16384 Bytes
            static constexpr size_t _32KB  = 0x00008000;  // 32768 Bytes
            static constexpr size_t _64KB  = 0x00010000;  // 65536 Bytes
            static constexpr size_t _128KB = 0x00020000;  // 131072 Bytes
            static constexpr size_t _256KB = 0x00040000;  // 262144 Bytes
            static constexpr size_t _512KB = 0x00080000;  // 524288 Bytes
            static constexpr size_t _1MB   = 0x00100000;  // 1048576 Bytes

            // Частота сэмплирования
            static constexpr size_t _24MSa  = 0x30;  // 24MSa per channel
            static constexpr size_t _16MSa  = 0x10;  // 16MSa per channel
            static constexpr size_t _8MSa   = 0x08;  // 8MSa per channel
            static constexpr size_t _4MSa   = 0x04;  // 4MSa per channel
            static constexpr size_t _1MSa   = 0x01;  // 1MSa per channel
            static constexpr size_t _500KSa = 0x32;  // 500KSa per channel
            static constexpr size_t _200KSa = 0x14;  // 200KSa per channel
            static constexpr size_t _100KSa = 0x0A;   // 100KSa per channel

            // Масштаб
            static constexpr size_t _10V = 0x01;  // -5V    to 5V
            static constexpr size_t _5V  = 0x02;  // -2.5V  to 2.5V
            static constexpr size_t _2V  = 0x05;  // -1V    to 1V
            static constexpr size_t _1V  = 0x0A;  // -500mv to 500mv

        private:
            struct DeviceTypeDef
            {
                libusb_device_handle *_handle;
                uint8_t _address;

                DeviceTypeDef() : _handle(nullptr), _address(0x00) {}
            };

            DeviceTypeDef _device;

            OscSigframe _oscSignal;

            /** @brief init_usb - Инициализация usb
             * */
            void init_usb();

            /* @brief firmwareUpload - загрузка встроенного ПО. Это надо только при первом запуске
             *                         осцилографа, есть нюанс. Если без прерывания это делать, будет не
             *                         определение устройства. Надо использовать задержку
             * */
            int firmwareUpload();

            /** @brief openDevice - Открыть девайс и взять его данные
             * */
            void openDevice();

            /** @brief closeDevice - закрыть устройство
             * */
            void closeDevice();

            /** @brief readData - считать данные из каналов
             * */
            void readData( uint8_t* CH1, uint8_t* CH2, const size_t& DS, size_t TimeOut ); 
 
            /** @brief init - инициализация осциллографа
             *  @param SR - частота семплирования
             *              * _24MSa
             *              * _16MSa
             *              * _8MSa
             *              * _4MSa
             *              * _1MSa
             *              * _500KSa
             *              * _200KSa
             *              * _100KSa
             *  @param ILx - уровень канала
             *              * _10V диапазон от -5V    до 5V    
             *              * _5V  диапазон от -2.5V  до 2.5V  
             *              * _2V  диапазон от -1V    до 1V    
             *              * _1V  диапазон от -500mv до 500mv 
             * */
            void init( const size_t& SR, const size_t& IL1, const size_t& IL2 );

            void close();

            uint8_t setLevelForOscilloscope( const size_t& level );

        public:
            /** Конструктор
             *  @param SR - частота семплирования
             *              * _24MSa
             *              * _16MSa
             *              * _8MSa
             *              * _4MSa
             *              * _1MSa
             *              * _500KSa
             *              * _200KSa
             *              * _100KSa
             *  @param ILx - уровень канала
             *              * _10V диапазон от -5V    до 5V
             *              * _5V  диапазон от -2.5V  до 2.5V
             *              * _2V  диапазон от -1V    до 1V
             *              * _1V  диапазон от -500mv до 500mv
             * */
            Hantek6022( const size_t& SR = _4MSa, const size_t& IL1 = 1'000, const size_t& IL2 = 1'000 );

            ~Hantek6022();

            size_t getChannelsSize() override { return 2; }

            std::string whoAmI() override { return std::string("Hantek 6022BE"); }

            /** @brief setSampleRate - задать частоту семплирования
             *  @param SR - значение частоты
             *              * HT6022_24MSa
             *              * HT6022_16MSa
             *              * HT6022_8MSa
             *              * HT6022_4MSa
             *              * HT6022_1MSa
             *              * HT6022_500KSa
             *              * HT6022_200KSa
             *              * HT6022_100KSa
             * */
            void setSampleRate( const size_t& SR ) override;

            size_t getSampleRate() override
            {
                return _oscSignal[0]._sampleRate;
            }
            
            std::vector<size_t> getRangeSampleRate() override
            {
                return std::vector<size_t>{ _100KSa, _200KSa, _500KSa, _1MSa, _4MSa, _8MSa, _16MSa, _24MSa };
            }


            /** @brief setCH2InputRate - задать уровень для канал CH2
             *  @param CHx - номер канал:
             *              * 0 CH1
             *              * 1 CH2
             *  @param IL - значение уровня:
             *              * HT6022_10V диапазон от -5V    до 5V
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V
             *              * HT6022_2V  диапазон от -1V    до 1V
             *              * HT6022_1V  диапазон от -500mv до 500mv
             * */
            void setInputLevel( const uint8_t& CHx, const size_t& IL ) override;

            size_t getInputLevel( const uint8_t &CHx ) override
            {
                return _oscSignal[CHx]._inputLevel;
            }
            
            std::vector<size_t> getRangeInputLevel() override
            {
                return std::vector<size_t>{ 1'000, 2'000, 5'000, 10'000 };
            }

            /** @brief getSignalFrame - метод считывание данных из каналов
             *  @param FS - диапазаон считываемых значений
             *              * HT6022_1KB
             *              * HT6022_2KB
             *              * HT6022_4KB
             *              * HT6022_8KB
             *              * HT6022_16KB
             *              * HT6022_32KB
             *              * HT6022_64KB
             *              * HT6022_128KB
             *              * HT6022_256KB
             *              * HT6022_512KB
             *              * HT6022_1MB
             * */
            OscSigframe getSignalFrame( const size_t& FS ) override;

            std::vector<size_t> getRangeSignalFrame() override
            {
                return std::vector<size_t>{ _1KB, _2KB, _4KB, _8KB, _16KB, _32KB, _64KB, _128KB, _256KB, _512KB, _1MB };
            }

            float getCoefficient( const size_t& mv )
            {
                switch( mv )
                {
                    case 1'000:
                        return 25.0;
                    case 2'000:
                        return 12.5;
                    case 5'000:
                        return 6.25;
                    case 10'000:
                        return 3.125;
                };
                throw InvalidParamOscilloscope("getCoefficient");
                return -1;
            }

            OscSignal getSignalFromTrigger( const uint8_t CHx, const float& level, const uint8_t comp )
            {
                const size_t DATA_SIZE = _16KB;
                bool isTriggerSuccess = true;
                bool lastRead = true;
                size_t k = 0;
                _oscSignal[CHx]._signal.clear();
                while( ( isTriggerSuccess || lastRead ) )
                {
                    if( !isTriggerSuccess )
                        lastRead = false;

                    THROW( ( ( _device._handle == nullptr ) ), "" );

                    uint8_t *data = new uint8_t[( sizeof(uint8_t) * DATA_SIZE * 2 )];
                    THROW( ( data == nullptr ), "" );

                    *data = HT6022_READ_CONTROL_DATA;
                    int r = libusb_control_transfer( _device._handle, HT6022_READ_CONTROL_REQUEST_TYPE,
                                                     HT6022_READ_CONTROL_REQUEST, HT6022_READ_CONTROL_VALUE,
                                                     HT6022_READ_CONTROL_INDEX, data, HT6022_READ_CONTROL_SIZE, 0 );

                    THROW( ( ( r >= 0 ) ? 0 : r), "", data );

                    int nread;
                    r = libusb_bulk_transfer( _device._handle, HT6022_READ_BULK_PIPE, data, ( DATA_SIZE * 2 ), &nread, 0 ); 

                    THROW( ( ( r >= 0 ) ? 0 : r), "", data );
                    THROW( ( ( nread != ( DATA_SIZE * 2 ) ) ? -1 : 0 ), "", data );

                    uint8_t *data_temp = data;

                    //auto control = std::pair<bool, size_t>( false, 0 );
                    if( CHx == 1 )
                        data_temp++;

                    float coefficient = getCoefficient( _oscSignal[CHx]._inputLevel );
                    
                    const size_t STEP = 512;
                    float buffer = 0.0;
                    for( size_t i = 0; i < DATA_SIZE; ++i )
                    {
                        _oscSignal[CHx]._signal.push_back( ( ( (*data_temp) - 127.0 ) / coefficient ) );
                        data_temp += 2;
                        if( ( ( i % STEP == 0 ) && isTriggerSuccess ) )
                        {
                            if( ( ( _oscSignal[CHx]._signal.back() >= level ) && ( comp == 1 ) ) )
                            {
                                isTriggerSuccess = false;
                                k = _oscSignal[CHx]._signal.size();
                            }
                            
                            if( ( ( _oscSignal[CHx]._signal.back() >= level ) && ( comp == 2 ) ) )
                            {
                                buffer = _oscSignal[CHx]._signal.back();
                            }

                            if( ( ( _oscSignal[CHx]._signal.back() <= level ) && ( comp == 2 ) && ( buffer >= level ) ) )
                            {
                                isTriggerSuccess = false;
                                k = _oscSignal[CHx]._signal.size();
                            }
                        }
                    }

                    if( isTriggerSuccess )
                    {
                        if( _oscSignal[CHx]._signal.size() > _1MB )
                            _oscSignal[CHx]._signal.clear();
                    }

                    delete []data;
                }

                k -= ( ( _oscSignal[CHx]._signal.size() >= 1000 ) ? 1000 : ( _oscSignal[CHx]._signal.size() - 1 ) );

                _oscSignal[CHx]._signal.erase( _oscSignal[CHx]._signal.begin(), _oscSignal[CHx]._signal.begin() + k );
                _oscSignal[CHx]._signalSize = _oscSignal[CHx]._signal.size();
                return _oscSignal[CHx];
            }

        };  // Hantek6022

    };  // hantek

};  // oscilloscopes

#endif  // OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_
