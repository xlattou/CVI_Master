#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h>    
#include "CH341DLL.H"  
#include "SiUSBXp.h" 		//F320的头文件
#include "F320_I2C_DEVICE.h"



HANDLE F320OpenDevice(ULONG iIndex)
{
	HANDLE UsbHandle;
	SI_STATUS status;
	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		return UsbHandle;
	}
	else
	{
		return NULL;
	}
}

BOOL F320CloseDevice(HANDLE UsbHandle)
{
	SI_STATUS status;
	status = SI_Close(UsbHandle);
	if(status == SI_SUCCESS)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

BOOL F320StreamSPI(ULONG iIndex, ULONG iRWLength, PVOID iBuffer, PVOID oBuffer)
{//iBuffer, DLL输入到F320，F320将它输出到SPI从设备
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;
	
	if(iRWLength > 280)
	{
		return -1;
	}

	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = SPI_RW;
		UsbPacket.dat_length[1] = iRWLength % 256;
		UsbPacket.dat_length[0] = (iRWLength / 256);
		memcpy(UsbPacket.io_dat, iBuffer, iRWLength);
		UsbPacket.error_code = 0;
		
		if(((HEADER_LENGTH + iRWLength) % 64) == 0)
			{
				UsbPacket.io_dat[iRWLength] = 0xAA;
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength + 1, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength + 1)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			else
			{
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iRWLength, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iRWLength || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return -1;
		}
		memcpy(oBuffer, UsbPacket.io_dat, iRWLength);
		
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}


BOOL F320StreamI2C(ULONG iIndex, ULONG iWriteLength, PVOID iWriteBuffer, ULONG iReadLength, PVOID oReadBuffer)
//	int			iIndex,  // 指定F320设备序号
//	int			iWriteLength // 准备写出的数据字节数
//	PVOID			iWriteBuffer // 指向一个缓冲区,放置准备写出的数据,首字节通常是I2C设备地址及读写方向位
//	int			iReadLength  // 准备读取的数据字节数
//	PVOID			oReadBuffer  // 指向一个缓冲区,返回后是读入的数据 
/*
如果要往A0h[0x80]写入0x11，那么就应该是
iWriteBuffer[0]=0xA0;
iWriteBuffer[1]=0x80;
iWriteBuffer[2]=0x11;
F320StreamI2C(iIndex, 2, PVOID iWriteBuffer, 0, oReadBuffer);
A0是从IIC的地址，这个不算在发送字节内
同样在读的时候虽然不发东西，但在iWriteBuffer的第一个字节应该放从IIC地址，但iWriteBuffer为0
DLL在处理读写时是判断length变量是否为0

如果要从A0h[0x80]读值，那么就应该是:
iWriteBuffer[0]=0xA0;
iWriteBuffer[1]=0x80;
F320StreamI2C(iIndex, 1, PVOID iWriteBuffer, 0, oReadBuffer);
F320StreamI2C(iIndex, 0, PVOID iWriteBuffer, 1, oReadBuffer);
*/
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	BYTE       End = 0xAA;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;

	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		if(iWriteLength)
		{
			if(iWriteLength > 280)
			{
				SI_Close(UsbHandle);
				return -1;
			}
			UsbPacket.op_command =IIC_W;
			UsbPacket.dat_length[1] = (unsigned char)(iWriteLength % 256);
			UsbPacket.dat_length[0] = (unsigned char)(iWriteLength / 256);
			memcpy(UsbPacket.io_dat, iWriteBuffer, iWriteLength + 1);

			if(((HEADER_LENGTH + iWriteLength + 1) % 64) == 0)
			{
				UsbPacket.io_dat[iWriteLength + 1] = End;
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength + 2, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength + 2)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			else
			{
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength + 1, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength + 1)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}
		if(iReadLength)
		{
			unsigned char length;
			
			if(iWriteLength == 0)
			{
				UsbPacket.io_dat[0] = *((unsigned char * )(iWriteBuffer));
				length =  HEADER_LENGTH + 1;
			}
			else
			{
				length =  HEADER_LENGTH;
			}
			UsbPacket.op_command = IIC_R;
			UsbPacket.dat_length[1] = (unsigned char)(iReadLength % 256);
			UsbPacket.dat_length[0] = (unsigned char)(iReadLength / 256);
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, length, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != length)
			{
				SI_Close(UsbHandle);
				return -1;
			}

			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iReadLength, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iReadLength || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
			memcpy(oReadBuffer, UsbPacket.io_dat, iReadLength);
		}

		if(iWriteLength == 0 && iReadLength == 0)
		{
			UsbPacket.op_command = IIC_SCH;
			UsbPacket.error_code = 0;
			UsbPacket.dat_length[1] = 0;
			UsbPacket.dat_length[0] = 0;
			memcpy(UsbPacket.io_dat, iWriteBuffer, iWriteLength + 1);

			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength + 1, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength + 1)
			{
				SI_Close(UsbHandle);
				return -1;
			}
			
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}

		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}

BOOL F320ExecuteCmd(ULONG iIndex, BYTE iCommand, ULONG iWriteLength, ULONG iReadLength, PVOID ioBuffer)
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;

	if(iCommand == GET_DVER)   //获取DLL版本号
	{
		((unsigned char*)ioBuffer)[0] = DLLVersion;
		return 0;
	}
	if(iCommand == IIC_SCH || iCommand == IIC_R || iCommand == IIC_W || iCommand == SPI_R || iCommand == SPI_W || iCommand == SPI_RW)
	{
		return -1;
	}
	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = iCommand;
		if(iWriteLength)
		{
			UsbPacket.dat_length[1] = iWriteLength % 256;
			UsbPacket.dat_length[0] = (iWriteLength / 256);
			memcpy(UsbPacket.io_dat, ioBuffer, iWriteLength);
		}
		else if(iReadLength)
		{
			UsbPacket.dat_length[1] = iReadLength % 256;
			UsbPacket.dat_length[0] = (iReadLength / 256);
		}
		else
		{
			UsbPacket.dat_length[1] = 0;
			UsbPacket.dat_length[0] = 0;
		}
		UsbPacket.error_code = 0;
		
		SI_SetTimeouts(0, dwTimeout);
		status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength, &BytesWritten, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength)
		{
			SI_Close(UsbHandle);
			return -1;
		}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iReadLength, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iReadLength || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return -1;
		}
		memcpy(ioBuffer, UsbPacket.io_dat, iReadLength);
		
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}

int I2C_SLAVE_SEARCH_F320 (int iIndex, int device_addr)
//没找到slave就返回-1，否则返回0 
{	
	int existed;
	unsigned char OutBuf[300], InBuf[300]; 
	int error;
	void *hUSBDevice;

								  
	if (iIndex<0) return -1;
	existed =0;

	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = 0;	//reg_add = 0
	//USBStatus = F320StreamI2C(iIndex, 1, OutBuf, 0, InBuf);	//写I2C从器件地址和偏移地址
	error = F320StreamI2C(iIndex, 0, OutBuf, 0, InBuf);	//restart，读2byte数据
	
	if (error <0)    //not found
		existed =-1;    
	else
		existed =0;

	return existed;
}

int I2C_BYTE_CURRENT_ADDRESS_READ_F320 (int iIndex, int device_addr, unsigned char *rom_value)
//立即读
{
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	error = F320StreamI2C(iIndex, 0, OutBuf, 1, InBuf);	//读数据
	if(error<0) { return -1; }
	
	*rom_value = InBuf[0]; 
	
	
  return 0;
}

int I2C_BYTE_CURRENT_ADDRESS_WRITE_F320 (int iIndex, int device_addr, int rom_startaddress, float T_wait)
//注意，只通过I2C发device_addr和rom_startaddress
{unsigned char rom_value;
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300]; 
 
 	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	error = F320StreamI2C(iIndex, 1, OutBuf, 0, InBuf);	//写数据
	if(error<0) { return -1; }
	
    Delay(T_wait);
  
  return 0;
}


int I2C_BYTEs_READ_F320 (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
//注意，rom_value_arr[256]，要全部传进来，但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据
{unsigned char rom_value;
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	error = F320StreamI2C(iIndex, 1, OutBuf, rom_Length, InBuf);	//读数据
	if(error<0) { return -1; }
	
	for (i=0; i<rom_Length; i++)
		rom_value_arr[rom_startaddress + i] = InBuf[i];
  
  return 0;
}


int I2C_BYTE_READ_F320 (int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value)
{int error;
 unsigned char rom_value_arr[256];
 
	error = I2C_BYTEs_READ_F320  (iIndex, device_addr, rom_startaddress, 1, rom_value_arr);
	*rom_value = rom_value_arr[rom_startaddress];

  return error;
}


int I2C_BYTEs_WRITE_F320 (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait)
//注意，只支持最大8byte连写， 起始地址值应该是8的倍数
//注意，rom_value_arr[256]，要全部传进来，但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据 
{unsigned char rom_value;
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300]; 
 
 	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	for (i=0;i<rom_Length;i++)
		OutBuf[i+2] = rom_value_arr[rom_startaddress+i];
	error = F320StreamI2C(iIndex, rom_Length+1, OutBuf, 0, InBuf);	//读数据
	if(error<0) { return -1; }
	
    Delay(T_wait);
  
  return 0;
}

int I2C_BYTE_WRITE_F320 (int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait)
{
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 unsigned char rom_value_arr[256];
 	
	rom_value_arr[rom_startaddress] = rom_value;
	error = I2C_BYTEs_WRITE_F320(iIndex, device_addr, rom_startaddress, 1, rom_value_arr, T_wait);
	if (error<0) { return -1; }
  
  return 0;
}


/*==========================F320 to SPI====================================================*/
int SPI_BYTEs_READWRITE_F320 (int iIndex, int iRWLength, unsigned char *iBuffer, unsigned char *oBuffer)
{							  
	return F320StreamSPI(iIndex, iRWLength, iBuffer, oBuffer); 
} 


/*==========================F320 to EPP====================================================*/
int F320StreamEPP(ULONG iIndex, BYTE iCommand, ULONG iWriteLength, ULONG iReadLength, PVOID ioBuffer)
{
 HANDLE     UsbHandle;
 SI_STATUS  status;
 USB_PACKET UsbPacket;
 DWORD      BytesWritten;
 DWORD      BytesReturned;
 DWORD      tmpReadTO;
 DWORD      tmpWriteTO;
 
	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = iCommand;
		if(iCommand == EPP_W && iWriteLength != 0)
		{
			UsbPacket.dat_length[1] = iWriteLength % 256;
			UsbPacket.dat_length[0] = (iWriteLength / 256);
			memcpy(UsbPacket.io_dat, ioBuffer, iWriteLength + 1);

			UsbPacket.error_code = 0;

			if(((HEADER_LENGTH + iWriteLength + 1) % 64) == 0)
			{
				UsbPacket.io_dat[iWriteLength + 1] = 0xAA;
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength + 2, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength + 2)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			else
			{
				
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iWriteLength + 1, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iWriteLength + 1)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}

			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle, &UsbPacket, HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}
		else if(iCommand == EPP_R && iReadLength != 0)
		{
			UsbPacket.dat_length[1] = iReadLength % 256;
			UsbPacket.dat_length[0] = (iReadLength / 256);
			memcpy(UsbPacket.io_dat, ioBuffer, 1);

			UsbPacket.error_code = 0;

			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + 1, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + 1)
			{
			SI_Close(UsbHandle);
			return -1;
			}

			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle, &UsbPacket, HEADER_LENGTH + iReadLength, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iReadLength || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}

			memcpy(ioBuffer, UsbPacket.io_dat, iReadLength);
		}
		else
		{
			SI_Close(UsbHandle);
			return -1;
		}

		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}



