#ifndef __UART_HANDLE_H 
#define __UART_HANDLE_H 

#include "includes.h"

// #define UART_HEADER_1_INDEX
// #define UART_HEADER_2_INDEX

/*
    定义数组的下标索引，用于查表

    只适用于一般的控制命令，
    例如 ： 帧头 + 传输方向 + 控制命令 + 设备ID + 参数
    对于 帧头1 + 传输方向 + 控制命令 + 设备ID + 通道1 + 通道2 + 通道3 + 通道4 + 通道5 + 通道6 + 通道7 + 通道8 + 帧尾
    这种特殊的不适用
*/
enum
{
    UART_HEADER_INDEX = 0x00, // 
    UART_DIR_INDEX,
    UART_CMD_PARAM_INDEX,
    UART_DEVICE_ID_INDEX, // 设备ID（设备自己的地址）
    // UART_TAIL_INDEX = 0x05, // 帧尾
};

// 定义控制命令
enum
{
    UART_CMD_PARAM_ON_OFF = 0x01, /* 开机/关机（根据设备ID，可以是所有设备，也可以是指定设备） */
    UART_CMD_PARAM_SET_DEVICE_ID = 0x02, /* 设置设备ID（会从当前设备开始，到最后一个设备，一直递增） */
    UART_CMD_PARAM_VIEW_CUR_DEVICE_ID = 0x03, /* 查看当前设备的ID */

    UART_CMD_PARAM_SET_ALL_RELAYS_STARTUP_SEQUENCE = 0x04, /* 设置指定设备的所有继电器的开机时序 */
    UART_CMD_PARAM_SET_ALL_RELAYS_SHUTDOWN_SEQUENCE = 0x05, /* 设置指定设备的所有继电器的关机时序 */

    UART_CMD_PARAM_VIEW_ALL_RELAYS_STARTUP_SEQUENCE = 0x06, /* 查看指定设备的所有继电器的开机时序 */
    UART_CMD_PARAM_VIEW_ALL_RELAYS_SHUTDOWN_SEQUENCE = 0x07, /* 查看指定设备的所有继电器的关机时序 */

    UART_CMD_PARAM_VIEW_ALL_RELAYS_STATUS = 0x08, /* 查看指定设备的所有继电器的开关状态 */
    UART_CMD_PARAM_SET_ALL_RELAYS = 0x09, /* 设置指定设备的所有继电器的开关状态 */
};


#endif