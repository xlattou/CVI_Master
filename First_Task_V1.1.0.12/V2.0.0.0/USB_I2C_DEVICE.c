#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h>    
#include "CH341DLL.H"  


int OpenUSB(ULONG iIndex)
{	if(CH341OpenDevice(iIndex) == INVALID_HANDLE_VALUE)
	{	CH341ResetDevice(iIndex);				// 复位USB
		CH341SetTimeout(iIndex, 0x2710, 0x2710);	// 指定USB超时时间,1S

		if(CH341OpenDevice(iIndex) == INVALID_HANDLE_VALUE)
			return -1;  
	}		
	return 0; 
}


int CloseUSB(ULONG iIndex)
{
	CH341CloseDevice(iIndex);
	return 0;	
}


int InitialUSB(ULONG iIndex, ULONG iMode)
{

	if(OpenUSB(iIndex)<0)//open USB error
		return -1;  
	
	if (!CH341SetStream(iIndex,iMode))			//设置通讯速率
	{	CH341ResetDevice(iIndex);				// 复位USB
		CH341SetTimeout(iIndex, 0x2710, 0x2710);	// 指定USB超时时间,1S
		if(OpenUSB(iIndex)<0)//open USB error
			return -1;
		if (!CH341SetStream(iIndex,iMode))			//设置通讯速率
			return -1;  
	}
	
	return 0;
}

int I2C_SLAVE_SEARCH_USB (int iIndex, ULONG iMode, int device_addr)
//没找到slave就返回-1，否则返回0 
{	int existed;
	unsigned char OutBuf[300], InBuf[300]; 
	int Status;
	
  		if (iIndex<0) return -1;
		existed =0;
				InitialUSB(iIndex, iMode); //打开USB句柄 
					OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
					OutBuf[1] = 0;	//reg_add = 0
					Status = CH341StreamI2C(iIndex,2,OutBuf,0,InBuf); 	//写I2C从器件地址和偏移地址
					OutBuf[0] ++;					//e.g. A1,for I2C read 
					Status = CH341StreamI2C(iIndex,1,OutBuf,2,InBuf);	//restart，读2byte数据
					
					if ( (Status == 1) && ((InBuf[0] != 0xff) || (InBuf[1] != 0xff)) )   //found
					{
				    	existed =0;
					} 
					else
					{
				    	existed =-1;
					} 
				CloseUSB(iIndex);
		
		return existed;
}


int I2C_BYTE_CURRENT_ADDRESS_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, BYTE *rom_value)
//立即读
{
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE + 0x1;	//e.g. A1,for I2C read
	Status = CH341StreamI2C(iIndex,1,OutBuf,1,InBuf); 	//写I2C从器件地址和偏移地址
	if (Status==0) { return -1; }
	*rom_value = InBuf[0];
	
 	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
	
  return 0;
}


int I2C_BYTEs_READ_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr)
//注意，rom_value_arr[256]，要全部传进来，但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据
{
	BYTE rom_value;
 	int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 	int error, Status, i;
 	unsigned char OutBuf[300], InBuf[300];  
 
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	Status = CH341StreamI2C(iIndex,2,OutBuf,0,InBuf); 	//写I2C从器件地址和偏移地址
	OutBuf[0] ++;					//e.g. A1,for I2C read 
	Status = CH341StreamI2C(iIndex,1,OutBuf,rom_Length,InBuf);	//restart，读数据
	if (Status==0) { return -1; }


	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
	
	for (i=0; i<rom_Length; i++)
		rom_value_arr[rom_startaddress + i] = InBuf[i];
  
  return 0;
}

int I2C_BYTE_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE *rom_value)
{int error;
 unsigned char rom_value_arr[256];
 
	error = I2C_BYTEs_READ_USB  (iIndex, 1, device_addr, rom_startaddress, 1, rom_value_arr);
	*rom_value = rom_value_arr[rom_startaddress];

  return error;
}


int I2C_BYTEs_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr, float T_wait)
//注意，只支持最大8byte连写， 起始地址值应该是8的倍数
//注意，rom_value_arr[256]，要全部传进来，但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据 
{BYTE rom_value;
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300]; 
 
 	
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	for (i=0;i<rom_Length;i++)
		OutBuf[i+2] = rom_value_arr[rom_startaddress+i];
	Status = CH341StreamI2C(iIndex,rom_Length+2,OutBuf,0,InBuf);
	if (Status<0) { return -1; }
	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
    Delay(T_wait);
  
  return 0;
}

int I2C_BYTE_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE rom_value, float T_wait)
{
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
 	
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	OutBuf[2] = rom_value;
	Status = CH341StreamI2C(iIndex,3,OutBuf,0,InBuf);
	if (Status<0) { return -1; }
	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
    Delay(T_wait);
  
  return 0;
}








int __stdcall InitialUSB_DLL(int iIndex) //打开USB句柄
{
 return InitialUSB(iIndex, 1);
}

int __stdcall CloseUSB_DLL(int iIndex) //关闭USB句柄
{if (iIndex<0) return -1;
 return CloseUSB(iIndex);
}


int __stdcall GetUSBHostDescr(  // 读取设备描述符
				int	iIndex,  // 指定CH341设备序号
				unsigned char *OutBuf) // 指向一个足够大的缓冲区,用于保存描述符
{int length=10;
 int error;
    if (iIndex<0) return -1;
	
	//InitialUSB(iIndex, 1); //打开USB句柄
	if (CH341GetDeviceDescr(iIndex,  OutBuf, &length)) 
		error=0;
	else
		error=-1;
	//CloseUSB(iIndex); //关闭USB句柄     
	
	return error;
}



int __stdcall GetUSBHostConfigDescr(  // 读取配置描述符
				int	iIndex,  // 指定CH341设备序号
				unsigned char *OutBuf)  // 指向一个足够大的缓冲区,用于保存描述符
{int length=10;
 int error;
    if (iIndex<0) return -1;

	//InitialUSB(iIndex, 1); //打开USB句柄
	if (CH341GetConfigDescr(iIndex,  OutBuf, &length)) 
		error=0;
	else
		error=-1;
	//CloseUSB(iIndex); //关闭USB句柄     

	return error;
}
				
int __stdcall USBHost_GetStatus_DLL (int iIndex, unsigned int *iStatus)
//
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //打开USB句柄
	error = CH341GetStatus(iIndex, iStatus);
	//CloseUSB(iIndex); //关闭USB句柄     
 return error;        	
}

int __stdcall USBHost_GetInput_DLL (int iIndex, unsigned int *iStatus)
// 位7-位0对应CH341的D7-D0引脚
// 位8对应CH341的ERR#引脚, 位9对应CH341的PEMP引脚, 位10对应CH341的INT#引脚, 位11对应CH341的SLCT引脚, 位23对应CH341的SDA引脚
// 位13对应CH341的BUSY/WAIT#引脚, 位14对应CH341的AUTOFD#/DATAS#引脚,位15对应CH341的SLCTIN#/ADDRS#引脚
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //打开USB句柄
	error = CH341GetInput(iIndex, iStatus);
	//CloseUSB(iIndex); //关闭USB句柄     
	
	return  error;       	
}

int __stdcall USBHost_SetD5D0_DLL (int iIndex, int iSetDirOut, int iSetDataOut)
// 设置D5-D0各引脚的I/O方向,某位清0则对应引脚为输入,某位置1则对应引脚为输出,并口方式下默认值为0x00全部输入
// 设置D5-D0各引脚的输出数据,如果I/O方向为输出,那么某位清0时对应引脚输出低电平,某位置1时对应引脚输出高电平
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //打开USB句柄
	error = CH341Set_D5_D0(iIndex,  iSetDirOut,  iSetDataOut );
	//CloseUSB(iIndex); //关闭USB句柄     
	
	return  error;       	
}

int __stdcall USBHost_ResetDevice_DLL(int iIndex)
{
	if (iIndex<0) return -1;
	return CH341ResetDevice(iIndex);
}
