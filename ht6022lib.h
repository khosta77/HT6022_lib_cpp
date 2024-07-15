#ifndef OSCILLOSCOPES_HANTEK_HT6022_LIB_CPP_H_
#define OSCILLOSCOPES_HANTEK_HT6022_LIB_CPP_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <thread>
#include <chrono>
#include <exception>
#include <string>
#include <libusb-1.0/libusb.h>

namespace oscilloscopes
{

    namespace hantek
    {

        /** @brief Ошибка
         * */
        typedef enum
        {
            HT6022_SUCCESS  = 0,
            HT6022_ERROR_INVALID_PARAM = -2,
            HT6022_ERROR_ACCESS        = -3,
            HT6022_ERROR_NO_DEVICE     = -4,
            HT6022_ERROR_TIMEOUT       = -7,
            HT6022_ERROR_NO_MEM        = -11,
            HT6022_ERROR_OTHER         = -99
        } HT6022_ErrorTypeDef;

#define IS_HT6022_ERROR(ERROR) (((ERROR) == HT6022_SUCCESS) || \
                                ((ERROR) == HT6022_ERROR_INVALID_PARAM) || \
                                ((ERROR) == HT6022_ERROR_ACCESS )   || \
                                ((ERROR) == HT6022_ERROR_NO_DEVICE) || \
                                ((ERROR) == HT6022_ERROR_TIMEOUT)   || \
                                ((ERROR) == HT6022_ERROR_NO_MEM)    || \
                                ((ERROR) == HT6022_ERROR_OTHER))

        /** @brief Размер буфера
         */   
        typedef enum
        {
            HT6022_1KB   = 0x00000400, /*!< 1024 Bytes */
            HT6022_2KB   = 0x00000800, /*!< 2048 Bytes */
            HT6022_4KB   = 0x00001000, /*!< 4096 Bytes */
            HT6022_8KB   = 0x00002000, /*!< 8192 Bytes */
            HT6022_16KB  = 0x00004000, /*!< 16384 Bytes */
            HT6022_32KB  = 0x00008000, /*!< 32768 Bytes */
            HT6022_64KB  = 0x00010000, /*!< 65536 Bytes */
            HT6022_128KB = 0x00020000, /*!< 131072 Bytes */
            HT6022_256KB = 0x00040000, /*!< 262144 Bytes */
            HT6022_512KB = 0x00080000, /*!< 524288 Bytes */
            HT6022_1MB   = 0x00100000  /*!< 1048576 Bytes */
        } HT6022_DataSizeTypeDef;

#define  IS_HT6022_DATASIZE(SIZE) (((SIZE) == HT6022_1KB)  ||\
                                   ((SIZE) == HT6022_2KB)  ||\
                                   ((SIZE) == HT6022_4KB)  ||\
                                   ((SIZE) == HT6022_8KB)  ||\
                                   ((SIZE) == HT6022_16KB) ||\
                                   ((SIZE) == HT6022_32KB) ||\
                                   ((SIZE) == HT6022_64KB) ||\
                                   ((SIZE) == HT6022_128KB)||\
                                   ((SIZE) == HT6022_256KB)||\
                                   ((SIZE) == HT6022_512KB)||\
                                   ((SIZE) == HT6022_1MB))

        /** @brief Размер калибровочного буфера, что то такое?
         */
        typedef enum
        {
            HT6022_32B   = 0x00000010, /*!< 32 Bytes */
            HT6022_64B   = 0x00000020, /*!< 64 Bytes */
            HT6022_128B  = 0x00000080 /*!< 128 Bytes */
        } HT6022_CVSizeTypeDef;

#define  IS_HT6022_CVSIZE(SIZE) (((SIZE) == HT6022_32B)  ||\
                                 ((SIZE) == HT6022_64B)  ||\
                                 ((SIZE) == HT6022_128B))

        /** @brief Частота семплирования
         */
        typedef enum
        {
            HT6022_24MSa  = 0x30, /*!< 24MSa per channel */
            HT6022_16MSa  = 0x10, /*!< 16MSa per channel */
            HT6022_8MSa   = 0x08, /*!< 8MSa per channel */
            HT6022_4MSa   = 0x04, /*!< 4MSa per channel */
            HT6022_1MSa   = 0x01, /*!< 1MSa per channel */
            HT6022_500KSa = 0x32, /*!< 500KSa per channel */
            HT6022_200KSa = 0x14, /*!< 200KSa per channel */
            HT6022_100KSa = 0x0A  /*!< 100KSa per channel */
        } HT6022_SRTypeDef;

#define  IS_HT6022_SR(SR) (((SR) == HT6022_24MSa)  ||\
                          ((SR) == HT6022_16MSa)  ||\
                          ((SR) == HT6022_8MSa)   ||\
                          ((SR) == HT6022_4MSa)   ||\
                          ((SR) == HT6022_1MSa)   ||\
                          ((SR) == HT6022_500KSa) ||\
                          ((SR) == HT6022_200KSa) ||\
                          ((SR) == HT6022_100KSa))

        /** @brief Масштаб
         */
        typedef enum
        {
            HT6022_10V   = 0x01, /*!< -5V    to 5V    */
            HT6022_5V    = 0x02, /*!< -2.5V  to 2.5V  */
            HT6022_2V    = 0x05, /*!< -1V    to 1V    */
            HT6022_1V    = 0x0A  /*!< -500mv to 500mv */
        } HT6022_IRTypeDef;

#define  IS_HT6022_IR(IR)   (((IR) == HT6022_10V) ||\
                             ((IR) == HT6022_5V)  ||\
                             ((IR) == HT6022_2V)  ||\
                             ((IR) == HT6022_1V))

        /** @brief Ошибка матрицы, с выводом сообщения
         * */
        class OscilloscopeException : public std::exception
        {
        public:
            explicit OscilloscopeException( const std::string &msg ) : m_msg(msg) {}
            const char *what() const noexcept override
            {
                return m_msg.c_str();
            }

        private:
            std::string m_msg;
        };

        class InvalidParamOscilloscope : public OscilloscopeException
        {
        public:
            InvalidParamOscilloscope( const std::string &methodName, const std::string &paramName) :
                OscilloscopeException( ( "В методе " + methodName + " проблема с параметром "
                                        + paramName ) ) {}
        };

        class AccessOscilloscopeException : public OscilloscopeException
        {
        public:
            AccessOscilloscopeException( const std::string &methodName ) : OscilloscopeException(
                    ( "В методе " + methodName + " проблема с доступом" ) ) {}
        };

        class NoDeviceOscilloscope : public OscilloscopeException
        {
        public:
            NoDeviceOscilloscope( const std::string &methodName ) : OscilloscopeException(
                    ( "В методе " + methodName + " не обнаружено устройство" ) ) {}
        };

        class TimeOut : public OscilloscopeException
        {
        public:
            TimeOut( const std::string &methodName ) : OscilloscopeException(
                    ( "В методе " + methodName + " timeout" )) {}
        };

        class MemoryException : public OscilloscopeException
        {
        public:
            MemoryException( const std::string &methodName, const std::string &paramName ) :
                OscilloscopeException( ( "В методе " + methodName + " под параметр "
                            + paramName + " нет памяти")) {}

        };

        class ht6022be
        {
        private:
            struct HT6022_DeviceTypeDef
            {
                libusb_device_handle *DeviceHandle;
                uint8_t Address;

                HT6022_DeviceTypeDef() : DeviceHandle(nullptr), Address(0x00) {}
            };

            HT6022_DeviceTypeDef Device;

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

            /** @brief setCH2InputRate - задать уровень для канал CH2
             *  @param chn - номер канал:
             *              * false CH1
             *              * true  CH2
             *  @param IR - значение уровня:
             *              * HT6022_10V диапазон от -5V    до 5V    
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V  
             *              * HT6022_2V  диапазон от -1V    до 1V    
             *              * HT6022_1V  диапазон от -500mv до 500mv 
             * */
            void setInputRate( const bool& chn, HT6022_IRTypeDef IR ); 

            /** @brief readData - считать данные из каналов
             * */
            void readData( uint8_t* CH1, uint8_t* CH2, HT6022_DataSizeTypeDef DataSize, size_t TimeOut ); 
 
            /** @brief setCalValues - не очень понятно что это и для чего нужно. Стоит оставить на всякий
             * */
            void setCalValues( unsigned char* CalValues, HT6022_CVSizeTypeDef CVSize );

            /** @brief getCalValues - не очень понятно что это и для чего нужно. Стоит оставить на всякий
             * */
            void getCalValues( unsigned char* CalValues, HT6022_CVSizeTypeDef CVSize );

            /** @brief init - инициализация осциллографа
             *  @param SR - частота семплирования
             *              * HT6022_24MSa
             *              * HT6022_16MSa
             *              * HT6022_8MSa
             *              * HT6022_4MSa
             *              * HT6022_1MSa
             *              * HT6022_500KSa
             *              * HT6022_200KSa
             *              * HT6022_100KSa
             *  @param IRx - уровень канала
             *              * HT6022_10V диапазон от -5V    до 5V    
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V  
             *              * HT6022_2V  диапазон от -1V    до 1V    
             *              * HT6022_1V  диапазон от -500mv до 500mv 
             * */
            void init( HT6022_SRTypeDef SR, HT6022_IRTypeDef IR1, HT6022_IRTypeDef IR2 );

            void close();

        public:
            /** Конструктор
             *  @param SR - частота семплирования
             *              * HT6022_24MSa
             *              * HT6022_16MSa
             *              * HT6022_8MSa
             *              * HT6022_4MSa
             *              * HT6022_1MSa
             *              * HT6022_500KSa
             *              * HT6022_200KSa
             *              * HT6022_100KSa
             *  @param IRx - уровень канала
             *              * HT6022_10V диапазон от -5V    до 5V    
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V  
             *              * HT6022_2V  диапазон от -1V    до 1V    
             *              * HT6022_1V  диапазон от -500mv до 500mv 
             * */
            ht6022be( HT6022_SRTypeDef SR = HT6022_1MSa, HT6022_IRTypeDef IR1 = HT6022_1V,
                      HT6022_IRTypeDef IR2 = HT6022_1V );

            ~ht6022be();

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
            void setSampleRate( HT6022_SRTypeDef SR );

            /** @brief setCH2InputRate - задать уровень для канал CH2
             *  @param IR - значение уровня:
             *              * HT6022_10V диапазон от -5V    до 5V    
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V  
             *              * HT6022_2V  диапазон от -1V    до 1V    
             *              * HT6022_1V  диапазон от -500mv до 500mv 
             * */
            void setCH1InputRate( HT6022_IRTypeDef IR );

            /** @brief setCH2InputRate - задать уровень для канал CH2
             *  @param IR - значение уровня:
             *              * HT6022_10V диапазон от -5V    до 5V    
             *              * HT6022_5V  диапазон от -2.5V  до 2.5V  
             *              * HT6022_2V  диапазон от -1V    до 1V    
             *              * HT6022_1V  диапазон от -500mv до 500mv 
             * */
            void setCH2InputRate( HT6022_IRTypeDef IR );

            /** @brief readFrame - метод считывание данных из каналов
             *  @param DS - диапазаон считываемых значений
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
             *  @param calibrX - калибровочное значение. Расчитывается при отключенных щупах, среднее
             *                   значение с массива. Один раз достаточно посчитать
             * */
            std::pair<std::vector<int>, std::vector<int>> readFrame( HT6022_DataSizeTypeDef DS,
                                                                     const double &calibr1 = 0.0,
                                                                     const double &calibr2 = 0.0 );
 
        };  // HT6022BE
        

    };  // hantek

};  // oscilloscopes

#endif  // OSCILLOSCOPES_HANTEK_HT6022_LIB_CPP_H_
