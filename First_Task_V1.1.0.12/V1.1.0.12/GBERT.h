/****************************************************************************
 *
 * File:                gbert.h
 *
 * Author:              Roger Li
 *
 * Description:         gbert基本功能代码
 *
 * Time:                2010-07-08
 *
 * version:				V2.0.0.2
 *
版本历史
V2.0.0.2:
增加定义
enum enum_GBERT_Pattern
enum enum_GBERT_Application
从文件GBERT_DLL.h继承而来

V2.0.0.1 
增加函数 GVPM_GET_SN
增加函数 GBERT_Check_Ratio
增加函数 GBERT_GET_SN

V2.0.0.0
1.添加GVPM锁定模式输出函数
2.GVPM_SET_Attenuate函数添加波长参数，避免多台GVPM同时使用，使用全局变量GOAM_WaveLength导致混乱
3.GVPM_Init函数添加衰减器在lock模式下的处理

****************************************************************************/

#ifndef _GBERT_H_
#define _GBERT_H_ 

#include <utility.h>
#include <ansi_c.h>
#include <formatio.h>

enum enum_GBERT_Pattern 
{   
	GBERT_PRBS7,
	GBERT_PRBS23,
	GBERT_PRBS31,
	GBERT_User_defined_64bit
}; //GBERT_Pattern;	

enum enum_GBERT_Application 
{   
	GBERT_R200Mbps,
	GBERT_R125Mbps,
	GBERT_R1250Mbps,
	GBERT_R1062Mbps,
	GBERT_R2125Mbps,
	GBERT_R4250Mbps,
	GBERT_R2500Mbps,
	GBERT_R155Mbps,
	GBERT_R622Mbps,
	GBERT_R2488Mbps
}; //GBERT_Application;	

//int GBERT_ErrorCode; 

extern BOOL GBERT_Init(int *GBERT_USBHandle, unsigned char *GBERT_SerialNumber, int Pattern, int Rate, int myBERT_SFPTxDisable);
extern BOOL GBERT_Init_SFPTxDisable(int *GBERT_USBHandle, unsigned char *GBERT_SerialNumber, int myBERT_SFPTxDisable);
extern BOOL GBERT_Start(int GBERT_USBHandle, double testTime);
extern BOOL GBERT_Stop(int GBERT_USBHandle);
extern BOOL GBERT_RESET(int GBERT_USBHandle); 
extern BOOL GBERT_Check(int GBERT_USBHandle, unsigned int *ErrorBit);  		//GBERT_ErrorCode==1 一个测试周期没有完成 延时1s后等待下一次循环
extern BOOL GBERT_Check_Start(int GBERT_USBHandle, unsigned int *ErrorBit); //GBERT_ErrorCode <0 状态读取有错误 延时1s后等待下一次，常用于发起开始测试命令后，读取测试状态
extern int GBERT_READ_Status (int inst_GBERT, unsigned int *ErrorBit);		//读取GEBRT状态, 不根据读取的状态做复位GBERT处理 
extern BOOL GBERT_Check_Ratio(int GBERT_USBHandle, double *ErrorRatio, int *errorcode);     //误码率
extern int GBERT_GET_SN(int panel, int control);
//2009-04-09 add by roger for GVPM
extern int GVPM_GET_SN(int panel, int control); //100407
extern int GVPM_Init (int *GVPM_USBHandle, unsigned char *GVPM_SerialNumber, int Wavelength, float AttValue);
extern int GVPM_SET_Attenuate (int GVPM_USBHandle, float AttValue, int Wavelength);
extern int GVPM_SET_OutputEnable (int GVPM_USBHandle, int GVPM_OutputEnable); 
extern int GVPM_SET_LockEnable (int inst_GVPM, int lockEnable);	
extern int GVPM_SET_LockAttenuate (int inst_GVPM, double LockAttValue, int Wavelength, int checklock); //checklock标志是否检查GVPM已到达稳定的Lock输出状态

#endif  
