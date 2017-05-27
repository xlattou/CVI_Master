#include <utility.h>
 
#include "I2C_LPT.h" 
#include "I2C_PROTOCOL.h" 

int InitialLPT(int LPTHandle)
//没找到host就返回-1，否则返回0
{unsigned char HOST_SDA_IN, HOST_SDA_IN_error;
 int error;

			if (LPTHandle<=0) return -1;

			//try I2C_position=0; 
			I2C_STOP(LPTHandle);
			HOST_SDA_IN_error=0;
			SetSDA(LPTHandle, 0);
			HOST_SDA_IN=GetSDA(LPTHandle);
			if (HOST_SDA_IN != 0) {HOST_SDA_IN_error++;}
			SetSDA(LPTHandle, 1);
			HOST_SDA_IN=GetSDA(LPTHandle);
			if (HOST_SDA_IN != 1) {HOST_SDA_IN_error++;}
			if (HOST_SDA_IN_error == 0)//成功
			{   I2C_STOP(LPTHandle);
				return 0;
			}
			else //try failed
			{	I2C_STOP(LPTHandle);
				return -1;
			}
			
	return 0;
			
}


int I2C_SLAVE_SEARCH_LPT (int LPTHandle, int device_addr)
//没找到slave就返回-1，否则返回0 
{	int existed;
	unsigned char OutBuf[300], InBuf[300]; 
	int Status;
  		if (LPTHandle<=0) return -1;
  		
			  // START
			  I2C_START(LPTHandle);
			  //device address code sda will be "str_device_addr+0"
			  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);
			  //get acknowledge
			  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
			  { //STOP
			    I2C_STOP(LPTHandle);
			    return -1;
			  }
			  else						  //found
			  { //STOP
			    I2C_STOP(LPTHandle);
			    return 0;
 			  }
		
	return 0;
}

int I2C_BYTE_CURRENT_ADDRESS_READ_LPT (int LPTHandle, int device_addr, unsigned char *rom_value)
//单字节立即读
{   if (LPTHandle<=0) return -1;
	return I2C_BYTE_CURRENT_ADDRESS_READ(LPTHandle, device_addr, rom_value);
}

int I2C_BYTE_READ_LPT (int LPTHandle, int device_addr, int rom_startaddress, unsigned char *rom_value)
//单字节指定地址读
{	if (LPTHandle<=0) return -1;
	return I2C_BYTE_READ(LPTHandle, device_addr, rom_startaddress, rom_value);
}

int I2C_BYTEs_READ_LPT (int LPTHandle, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
//多字节指定首地址连续读
//注意，rom_value_arr[]数组要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据
{   if (LPTHandle<=0) return -1;
	return I2C_BYTEs_READ(LPTHandle, device_addr, rom_startaddress, rom_Length, rom_value_arr);
}

int I2C_BYTE_WRITE_LPT (int LPTHandle, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait)
//单字节指定地址写
{   if (LPTHandle<=0) return -1;
	return I2C_BYTE_WRITE(LPTHandle, device_addr, rom_startaddress, rom_value, T_wait);
}

int I2C_BYTEs_WRITE_LPT (int LPTHandle, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait)
//多字节指定首地址连续写 
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。
//注意，对于EEPROM，一般地只能8byte连写；而对于7020，一般就没有最大长度的限制。
{   if (LPTHandle<=0) return -1;
	return I2C_BYTEs_WRITE(LPTHandle, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
}

