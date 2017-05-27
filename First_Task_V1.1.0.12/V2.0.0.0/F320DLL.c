// F320DLL.cpp : Defines the entry point for the DLL application.
//
#define _F320DLL_

#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h>    
#include "SiUSBXp.h"
#include "F320DLL.h"

#define  NUM_MAX_BYTES_BUF 280

typedef struct _USB_PACKET
{
	unsigned char op_command;
	unsigned char error_code;
	unsigned char dat_length[2];
	unsigned char io_dat[NUM_MAX_BYTES_BUF];
}USB_PACKET, *pUSB_PACKET;

#define HEADER_LENGTH (sizeof(USB_PACKET) - NUM_MAX_BYTES_BUF)
#define dwTimeout 2000


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
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL F320StreamSPI(ULONG iIndex, ULONG iRWLength, PVOID iBuffer, PVOID oBuffer)
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;
	
	if(iRWLength > 280)
	{
		return FALSE;
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
		
		SI_SetTimeouts(0, dwTimeout);
		status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength, &BytesWritten, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength)
		{
			SI_Close(UsbHandle);
			return FALSE;
		}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iRWLength, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iRWLength || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return FALSE;
		}
		memcpy(oBuffer, UsbPacket.io_dat, iRWLength);
		
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}
BOOL F320StreamI2C(ULONG iIndex, ULONG iWriteLength, PVOID iWriteBuffer, ULONG iReadLength, PVOID oReadBuffer)
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
				return FALSE;
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
					return FALSE;
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
					return FALSE;
				}
			}
			
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return FALSE;
			}
		}
		if(iReadLength)
		{
			UsbPacket.op_command = IIC_R;
			UsbPacket.dat_length[1] = (unsigned char)(iReadLength % 256);
			UsbPacket.dat_length[0] = (unsigned char)(iReadLength / 256);
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH)
			{
				SI_Close(UsbHandle);
				return FALSE;
			}

			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iReadLength, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iReadLength || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return FALSE;
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
				return FALSE;
			}
			
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return FALSE;
			}
		}

		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
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

	if(iCommand == GET_DVER)
	{
		((unsigned char*)ioBuffer)[0] = 0x10;
		return TRUE;
	}
	if(iCommand == IIC_SCH || iCommand == IIC_R || iCommand == IIC_W || iCommand == SPI_R || iCommand == SPI_W || iCommand == SPI_RW)
	{
		return FALSE;
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
			return FALSE;
		}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + iReadLength, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + iReadLength || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return FALSE;
		}
		
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}

