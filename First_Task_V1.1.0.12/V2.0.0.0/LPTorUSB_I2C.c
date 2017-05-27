#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h>    
#include "CH341DLL.H"  
#include "USB_I2C_DEVICE.h"
#include "LPT_I2C_DEVICE.h"



int PCHost_I2C_BYTE_READ (int iIndex, int device_addr, int rom_startaddress, BYTE *rom_value)
{int error;
 unsigned char rom_value_arr[256];
 
	error = -1;
	
	{	error = I2C_BYTEs_READ_USB  (iIndex, 1, device_addr, rom_startaddress, 1, rom_value_arr);}
	
	*rom_value = rom_value_arr[rom_startaddress];
	
	return error;
}

int PCHost_I2C_BYTEs_READ (int iIndex, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr)
{int error;
	error = -1;
	
	{	error = I2C_BYTEs_READ_USB  (iIndex, 1, device_addr, rom_startaddress, rom_Length, rom_value_arr);}
	
	return error;
}

extern int PCHost_I2C_BYTE_WRITE (int iIndex, int device_addr, int rom_startaddress, BYTE rom_value, float T_wait)
{int error;
 unsigned char rom_value_arr[256];
 
	error = -1;																		    

	rom_value_arr[rom_startaddress] = rom_value;
	
	{
		error = I2C_BYTEs_WRITE_USB  (iIndex, 1, device_addr, rom_startaddress, 1, rom_value_arr, T_wait);
	}

	return error;
}

int PCHost_I2C_TwoBYTE_WRITE (int iIndex, int device_addr, int rom_startaddress, BYTE *rom_value_arr, float T_wait)
{int error;

	error = -1;	
	
	{
		error = I2C_BYTEs_WRITE_USB  (iIndex, 1, device_addr, rom_startaddress, 2, rom_value_arr, T_wait);
	}
	
	return error;
}

int PCHost_I2C_FourBYTE_WRITE (int iIndex, int device_addr, int rom_startaddress, BYTE *rom_value_arr, float T_wait)
{int error;

	error = -1;	
	
	{
		error = I2C_BYTEs_WRITE_USB  (iIndex, 1, device_addr, rom_startaddress, 4, rom_value_arr, T_wait);
	}
	
	return error;
}

int PCHost_I2C_EightBYTE_WRITE (int iIndex, int device_addr, int rom_startaddress, BYTE *rom_value_arr, float T_wait)
{int error;

	error = -1;	
	
	{
		error = I2C_BYTEs_WRITE_USB  (iIndex, 1, device_addr, rom_startaddress, 8, rom_value_arr, T_wait);
	}
	
	return error;
}

int PCHost_I2C_BYTEs_WRITE (int iIndex, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr, float T_wait)
{int error;

	error = -1;	
	
	{	error = I2C_BYTEs_WRITE_USB  (iIndex, 1, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
	}
	
	return error;
}


int PCHost_I2C_BYTE_CURRENT_ADDRESS_READ (int iIndex, int device_addr, BYTE *rom_value)
{int error;
	error = -1;	

	{
		error = I2C_BYTE_CURRENT_ADDRESS_READ_USB (iIndex, 1,device_addr, rom_value);
	}
	
	return error;
}

