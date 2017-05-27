#ifndef _SUPERMASTER_H_
#define _SUPERMASTER_H_ 

#include <windows.h>  
#include "SuperCommand.h"
#include "CH341A_DLL.h"
#include "global.h"

extern int I2C_Write(int USB_Handle,BYTE nDev, BYTE nReg, BYTE nLen, BYTE* pbyDat);
extern int I2C_Read(int USB_Handle,BYTE nDev,BYTE nReg,BYTE nLen, BYTE* pbyBuf);
extern int RegisterI2cCallBackFunction(void);
int SetCommand(int usbHandle,char *strCmd, char *strOupt);   //Ôö¼ÓÁË¾ä±ú 

#endif 

