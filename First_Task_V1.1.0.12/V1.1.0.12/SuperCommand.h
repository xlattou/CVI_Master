/****************************************************************************************
*         SuperCmd.h                                                                    *
* File Name   : SuperCmd.h                                                              *
* Copyright   : (c) 2010 Superxon Inc. All Rights Reserved.                             *
* Description :                                                                         *
* Author      : Triton.Zhang                                                            *
* History     : 2010-03-04<Create>                                                      *
****************************************************************************************/
//=======================================================================================
// Includes Head file
//=======================================================================================
#ifndef _SUPER_CMD_H_
#define _SUPER_CMD_H_


#ifndef BYTE
typedef unsigned char       BYTE;   //
#endif

#ifndef INT8S
typedef signed char         INT8S;                      //8bit
#endif

#ifndef INT8U
typedef unsigned char       INT8U;                      //8bit
#endif

#ifndef INT16U
typedef unsigned short int  INT16U;                     //16bit
#endif

#ifndef INT16S
typedef signed short int    INT16S;                     //16bit
#endif

#ifndef INT32U
typedef unsigned int        INT32U;                     //32bit
#endif

#ifndef INT32S
typedef signed int          INT32S;                     //32bit  
#endif

// 定义出错编码，其中0为正确返回,1-127 为模块返回错误码
// 128 -255 为主机返回错误码
#define ERR_SUCCESS     0                                // 正确

#define ERR_MOD_FORMAT      0x01    // 命令帧格式错误
#define ERR_MOD_PARA        0x02    // 命令或参数不支持
#define ERR_MOD_CHKSUM      0x03    // 校验和1 错误
#define ERR_MOD_NOTSUPPORT  0x05    // 模块不支持的命令
#define ERR_MOD_BUSY        0x12    // 模块正忙

#define ERR_MASTER_BASE     128

#define ERR_CMD_UNKNOW_CMD  ERR_MASTER_BASE + 1       // 未知命令
#define ERR_CMD_FORMAT      ERR_MASTER_BASE + 2       // 命令格式不正确: ()不配套,
#define ERR_CMD_PARAMETER   ERR_MASTER_BASE + 3       // 参数非法
#define ERR_CMD_PARA_NUMBER ERR_MASTER_BASE + 4       // 参数个数不正确

#define ERR_SYS_ERROR       ERR_MASTER_BASE + 32      // 系统错误
#define ERR_SYS_UNREG_BKFUN ERR_MASTER_BASE + 33      // 未注册回调函数
#define ERR_SYS_PNT_NULL    ERR_MASTER_BASE + 34      // 指针为空

#define ERR_FRAME_NULL      ERR_MASTER_BASE + 96      // 帧错误
#define ERR_FRAME_FORMAT    ERR_MASTER_BASE + 97      // 帧参数错误

//=======================================================================================
//=======================================================================================
//#define CMD_COLOR   ((COLORREF)(0x007F00))
//#define DAT_COLOR   ((COLORREF)(0xFF0000))  
//#define MSG_COLOR   ((COLORREF)(0x7F7F7F))

#define MCU_PORT    0x10
#define MCU_ADC     0x11
#define MCU_DAC     0x12
#define MCU_MI2C    0x13
#define MCU_PLA     0x14
#define MCU_FLASH   0x15
#define MCU_PWM     0x16
#define MCU_SPI     0x17    // 保留

#define FMW_VER     0x18
#define COM_CFG     0x19
#define MCU_MON     0x1A
#define MCU_DACA    0x1B

#define TAB_SEG     0x1C
#define MCU_CFG     0x1D

#define DRV_SER     0x1F


//=======================================================================================
//=======================================================================================
#define FRAME_HEAD              0x55
#define FRAME_TAIL              0x16
#define FRAME_FINISHED          0x03
#define FRAME_FORMAT_SIZE       0x08

#define FRAME_FINISH_ADDR_E2       0x00  // 如果是Aduc7020，则有E2
#define FRAME_FINISH_ADDR_A0       0x80    // DS4830，要写在A2 or A0的table 0x80中

#define MODULE_FINISHED_ADDR_E2    0x01
#define MODULE_FINISHED_ADDR_A0    0x81

#define FRAME_BASE_ADDR_E2         0x02
#define FRAME_BASE_ADDR_A0         0x82

#define FRAME_HEAD_ADDR         0x00
#define ADDRE_FRAME_HEAD_E2        (FRAME_BASE_ADDR_E2 + FRAME_HEAD_ADDR)
#define ADDRE_FRAME_HEAD_A0        (FRAME_BASE_ADDR_A0 + FRAME_HEAD_ADDR)

#define FRAME_LEN_ADDR          0x05
#define ADDRE_FRAME_LEN_E2         (FRAME_LEN_ADDR + FRAME_BASE_ADDR_E2)
#define ADDRE_FRAME_LEN_A0         (FRAME_LEN_ADDR + FRAME_BASE_ADDR_A0)

typedef int (*pBkFun)(int,BYTE,BYTE,BYTE,BYTE*);
//=======================================================================================
//=======================================================================================
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int __stdcall SuperCmdSer(int usbHandle,char* ArgIn, char* ArgOut);
__declspec(dllexport) int __stdcall RegistCallBackFunciton(char* ArgIn, pBkFun pFun);
__declspec(dllexport) int __stdcall GetErroInfo(int wErr,char* ErrInfo);


__declspec(dllexport) void __stdcall SetSuperDev(int wSetDev);
__declspec(dllexport) int __stdcall  GetSuperDev(void);

#ifdef __cplusplus
}
#endif


#endif

//=======================================================================================
// End Of File
//=======================================================================================
