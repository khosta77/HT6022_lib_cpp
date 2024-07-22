#ifndef HT6022_LIB_CPP_H_
#define HT6022_LIB_CPP_H_

#include <cstdint>

// Макросы для настройки firm ware
#define HT6022_FIRMWARE_SIZE         458
#define HT6022_FIRMWARE_VENDOR_ID    0x4B4
#define HT6022_FIRMWARE_REQUEST_TYPE 0x40
#define HT6022_FIRMWARE_REQUEST      0xA0
#define HT6022_FIRMWARE_INDEX        0x00

// Макросы для настройки параметров осцилографа
#define HT6022_VENDOR_ID                  0x4B5
#define HT6022_MODEL                      0x6022
#define HT6022_IR1_REQUEST_TYPE           0x40
#define HT6022_IR1_REQUEST                0xE0
#define HT6022_IR1_VALUE                  0x00
#define HT6022_IR1_INDEX                  0x00
#define HT6022_IR1_SIZE                   0x01
#define HT6022_IR2_REQUEST_TYPE           0x40
#define HT6022_IR2_REQUEST                0xE1
#define HT6022_IR2_VALUE                  0x00
#define HT6022_IR2_INDEX                  0x00
#define HT6022_IR2_SIZE                   0x01
#define HT6022_SR_REQUEST_TYPE            0x40
#define HT6022_SR_REQUEST                 0xE2
#define HT6022_SR_VALUE                   0x00
#define HT6022_SR_INDEX                   0x00
#define HT6022_SR_SIZE                    0x01
#define HT6022_SETCALLEVEL_REQUEST_TYPE   0xC0
#define HT6022_SETCALLEVEL_REQUEST        0xA2
#define HT6022_SETCALLEVEL_VALUE          0x08
#define HT6022_SETCALLEVEL_INDEX          0x00
#define HT6022_GETCALLEVEL_REQUEST_TYPE   0x40
#define HT6022_GETCALLEVEL_REQUEST        0xA2
#define HT6022_GETCALLEVEL_VALUE          0x08
#define HT6022_GETCALLEVEL_INDEX          0x00
#define HT6022_READ_CONTROL_REQUEST_TYPE  0x40
#define HT6022_READ_CONTROL_REQUEST       0xE3
#define HT6022_READ_CONTROL_VALUE         0x00
#define HT6022_READ_CONTROL_INDEX         0x00
#define HT6022_READ_CONTROL_SIZE          0x01
#define HT6022_READ_CONTROL_DATA          0x01
#define HT6022_READ_BULK_PIPE             0x86

extern uint8_t HT6022_Firmware[];
extern uint8_t HT6022_AddressList[256];

#endif  // HT6022_LIB_CPP_H_
