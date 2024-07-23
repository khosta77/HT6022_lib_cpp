#ifndef OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_
#define OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_

#include "oscilloscopes.h"

#include <libusb-1.0/libusb.h>

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
            Hantek6022( const size_t& SR = _4MSa, const size_t& IL1 = _1V, const size_t& IL2 = _1V );

            ~Hantek6022();

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

        };  // Hantek6022

    };  // hantek

};  // oscilloscopes

#endif  // OSCILLOSCOPES_HANTEK_HANTEK6022_CPP_H_