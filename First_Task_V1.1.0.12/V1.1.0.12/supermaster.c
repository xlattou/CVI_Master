#include "supermaster.h"
#include <utility.h>

int I2C_Write(int USB_Handle,BYTE nDev, BYTE nReg, BYTE nLen, BYTE* pbyDat)
{
    int i;   
	BYTE pbyVal[256];    
						 
	if (-1 == USB_Handle)
	{
		return -1;
	}
    memset(pbyVal,0,256);

    for (i = 0; i < nLen; i++)
    {
        pbyVal[(nReg + i)%256] = pbyDat [i];
    }

    return   I2C_BYTEs_WRITE_DLL(USB_Handle, nDev, nReg, nLen, pbyVal, 0);
}

int I2C_Read(int USB_Handle,BYTE nDev,BYTE nReg,BYTE nLen, BYTE* pbyBuf)
{
    int i, wRes;
	BYTE pbyVal[256]; 
	
	if (-1 == USB_Handle)
	{
		return -1;
	}
    memset(pbyVal,0,256);

    wRes = I2C_BYTEs_READ_DLL (USB_Handle, nDev,nReg,nLen, pbyVal);
    if (0 == wRes)
    {
        for (i = 0; i < nLen; i++)
        {
            pbyBuf[i] = pbyVal[nReg + i];
        }
    }
    return wRes;
}

int RegisterI2cCallBackFunction(void)
{
    char strCmdR [] = "I2C_READ";
	char strCmdW [] = "I2C_WRITE";	
    int wRes = 0; 
	
    wRes += RegistCallBackFunciton(strCmdR,&I2C_Read);
    
    wRes += RegistCallBackFunciton(strCmdW,&I2C_Write);
	
    return wRes;
}

int SetCommand(int usbHandle, char *strCmd, char *strOupt)
{
	int wRes = 0;
	int wValue = 0;
	
	wRes = SuperCmdSer(usbHandle, strCmd,strOupt);
    if (ERR_SUCCESS == wRes)
    {
		;
    }
	else
	{
		GetErroInfo(wRes,strOupt);
	}
	return wRes;
}		

