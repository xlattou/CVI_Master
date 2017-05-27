#include <utility.h>
#include "I2C_LPT.h"
#include "I2C_PROTOCOL.h"


/*************************************************************************/
void I2C_START(int LPTHandle)
{
  SetSCL(LPTHandle, 1); // scl=1;
  SetSDA(LPTHandle, 1); // SDA=1;
  SetSDA(LPTHandle, 0); // SDA=0, SCL holds 1; "START"
  SetSCL(LPTHandle, 0);//
  return;
}

unsigned char I2C_GET_ACK(int LPTHandle)
{int ii; 
  SetSCL(LPTHandle, 0);// 
  SetSDA(LPTHandle, 1); // sda=1; LPT release sda bus; waiting for reading acknowledge signal from EEPROM
  SetSCL(LPTHandle, 1); // scl=1;
  ii=0;
  while ( GetSDA(LPTHandle) && (ii<10) ) {ii++;}// if resceive ack!=0 then countinue to wait
  
  SetSCL(LPTHandle, 0); // scl=1,0
  SetSDA(LPTHandle, 1); // 
  
  if  (ii>=9)
  { SetSCL(LPTHandle, 1); // release SCL
    I2C_STOP(LPTHandle) ; //jack 2006/05/15
  }
  
  if (ii>=9) return 1;
  return 0;
    
}

void I2C_WRITE_WORD_VALUE(int LPTHandle, unsigned char rom_value)
{int ii; 
  unsigned char byte_temp[8];

  SetSCL(LPTHandle, 0);// 
  for (ii=7; ii>=0; ii--)
	  byte_temp[ii] = (unsigned char)((rom_value & (0x01 << ii)) >> ii);
  for (ii=7; ii>=0; ii--)
  {	SetSDA(LPTHandle, byte_temp[ii]); SetSCL(LPTHandle, 1); SetSCL(LPTHandle, 0); /* sda=byte_temp[ii]; scl=1,0 */ }
  return;
}

unsigned char I2C_READ_WORD_VALUE(int LPTHandle)
{ unsigned char byte_temp=0; int i;
  SetSCL(LPTHandle, 0);// 
  for (i=7; i>=0; i--)
  {	  SetSCL(LPTHandle, 1); byte_temp = byte_temp | (GetSDA(LPTHandle) <<i); SetSCL(LPTHandle, 0); // scl=1,0  
  }
  return byte_temp;
}

void I2C_SET_ACK(int LPTHandle)
{
  SetSCL(LPTHandle, 0); // scl=0; // add this line 2003/9/12
  SetSDA(LPTHandle, 0); // sda=0; 
  SetSCL(LPTHandle, 1); // scl=1;
  SetSCL(LPTHandle, 0); // scl=0;
  SetSDA(LPTHandle, 1); // sda=1; release SDA, so that can read the target's output correctly, 
             // otherwise the read-back byte will surely be '0'.
}

void I2C_SET_NONE_ACK(int LPTHandle)
{
  SetSDA(LPTHandle, 1); // sda=1; 
  SetSCL(LPTHandle, 1); // scl=1;
  SetSCL(LPTHandle, 0); // scl=0;
}

void I2C_STOP(int LPTHandle)
{
  SetSCL(LPTHandle, 0); // scl=0;
  SetSDA(LPTHandle, 0); // sda=0; 
  SetSCL(LPTHandle, 1); // scl=1;
  SetSDA(LPTHandle, 1); // SDA=1;
  return;
}



/*************************************************************************/

int I2C_BYTE_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value)
{ 
  // START
  I2C_START(LPTHandle);

  //dummy byte write;  device address code sda will be "str_device0_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }


  //get Data Byte high order bit first
  *rom_value=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_NONE_ACK(LPTHandle);

  //STOP
  I2C_STOP(LPTHandle) ;

  return 0;

}

int I2C_BYTE_CURRENT_ADDRESS_READ (int LPTHandle, unsigned char device_addr, unsigned char *rom_value)
{  
  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }


  //get Data Byte high order bit first
  *rom_value=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_NONE_ACK(LPTHandle);

  //STOP
  I2C_STOP(LPTHandle) ;

  return 0;

}


int I2C_2BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1)
{unsigned char rom_value;

  // START
  I2C_START(LPTHandle);

  //dummy byte write;  device address code sda will be "str_device0_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }


  //get Data Byte high order bit first
  *rom_value0=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value1=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge 
  I2C_SET_NONE_ACK(LPTHandle);

  //STOP
  I2C_STOP(LPTHandle) ;
  
  return 0;
}

int I2C_4BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3)
{unsigned char rom_value;

  // START
  I2C_START(LPTHandle);

  //dummy byte write;  device address code sda will be "str_device0_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }


  //get Data Byte high order bit first
  *rom_value0=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value1=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value2=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value3=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge 
  I2C_SET_NONE_ACK(LPTHandle);

  //STOP
  I2C_STOP(LPTHandle) ;
  
  return 0;
}

int I2C_8BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3, 
			unsigned char *rom_value4, unsigned char *rom_value5, unsigned char *rom_value6, unsigned char *rom_value7)
{unsigned char rom_value;

  // START
  I2C_START(LPTHandle);

  //dummy byte write;  device address code sda will be "str_device0_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }


  //get Data Byte high order bit first
  *rom_value0=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value1=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value2=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value3=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value4=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value5=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value6=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge
  I2C_SET_ACK(LPTHandle);

  //get Data Byte high order bit first
  *rom_value7=I2C_READ_WORD_VALUE(LPTHandle);

  //set none acknowledge 
  I2C_SET_NONE_ACK(LPTHandle);

  //STOP
  I2C_STOP(LPTHandle) ;
  
  return 0;
}

int I2C_BYTEs_READ  (int LPTHandle, unsigned char device_addr, unsigned char rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
{unsigned char rom_value;
 int rom_addr; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle

  I2C_START(LPTHandle);

  //dummy byte write;  device address code sda will be "str_device0_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);
  //Delay(0.005);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=0 high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_startaddress);
  //Delay(0.005);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  // START
  I2C_START(LPTHandle);

  // device address code sda will be "str_device0_addr+1"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr+1);
  //Delay(0.005);
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  for (rom_addr=rom_startaddress; rom_addr<(rom_startaddress+rom_Length); rom_addr++)
  {
	  //get Data Byte high order bit first
	  rom_value=I2C_READ_WORD_VALUE(LPTHandle);
	  rom_value_arr[rom_addr]= rom_value;
	  //Delay(0.001);

	  if (rom_addr==(rom_startaddress+rom_Length-1))
		I2C_SET_NONE_ACK(LPTHandle);//set none acknowledge
	  else
	    I2C_SET_ACK(LPTHandle);     //set acknowledge
  }

  //STOP
  I2C_STOP(LPTHandle) ;
  
  return 0;
}



int I2C_BYTE_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char rom_value, float T_wait) 
{
  // START
  I2C_START(LPTHandle);

  //device address code sda will be "str_device_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //set WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //STOP
  I2C_STOP(LPTHandle) ;

  Delay(T_wait); // Twr_max=10ms, waiting for WWPROM internal data written.
  
  return 0;
}

int I2C_2BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_addr, 
					unsigned char rom_value1, unsigned char rom_value2, float T_wait) 
{
  // START
  I2C_START(LPTHandle);

  //device address code sda will be "str_device_addr+0"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_addr);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //set WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value1);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //set WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value2);

  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  {	return -1;  }

  //STOP
  I2C_STOP(LPTHandle) ;

  Delay(T_wait); // Twr_max=10ms, waiting for WWPROM internal data written.
  
  return 0;
}


int I2C_4BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_startaddress, 
     unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, float T_wait) 
{
  // START
  I2C_START(LPTHandle);
 
  //device address , for "Write"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -1; }
 
  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_startaddress);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -2; }
 
  //set 1st WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value1);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -3; }
 
  //set 2nd WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value2);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -4; }
 
  //set 3rd WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value3);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -5; }
 
  //set 4th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value4);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -6; }

  //STOP
  I2C_STOP(LPTHandle) ;
 
  Delay(T_wait); // Twr_max=10ms, waiting for EEPROM internal data written.
  
  return 0;
}

//The following pseudo code is to operate the "Eight Byte Continuous Write"
//Designed by WalkMan Studio, 2005
int I2C_8BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_startaddress, 
     unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, 
     unsigned char rom_value5, unsigned char rom_value6, unsigned char rom_value7, unsigned char rom_value8, float T_wait) 
{
  // START
  I2C_START(LPTHandle);
 
  //device address , for "Write"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -1; }
 
  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_startaddress);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -2; }
 
  //set 1st WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value1);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -3; }
 
  //set 2nd WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value2);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -4; }
 
  //set 3rd WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value3);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -5; }
 
  //set 4th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value4);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -6; }

  //set 5th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value5);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -7; }

  //set 6th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value6);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -8; }

  //set 7th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value7);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -9; }

  //set 8th WORD_VALUE , high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_value8);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -10; }

  
  //STOP
  I2C_STOP(LPTHandle) ;
 
  Delay(T_wait); // Twr_max=10ms, waiting for EEPROM internal data written.
  
  return 0;
}

int I2C_BYTEs_WRITE (int LPTHandle, unsigned char device_addr, unsigned char rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait)
{unsigned char rom_value;
 int rom_addr, error; // if define rom_addr as unsigned char, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error_count;
 int i;


  
  // START
  I2C_START(LPTHandle);
 
  //device address , for "Write"
  I2C_WRITE_WORD_VALUE(LPTHandle, device_addr);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -1; }
 
  //Word Address=rom_addr high order bit first
  I2C_WRITE_WORD_VALUE(LPTHandle, rom_startaddress);
 
  //get acknowledge
  if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
  { return -2; }
 
  
  for (i=0; i<rom_Length; i++)
  {//set 1st WORD_VALUE , high order bit first
   I2C_WRITE_WORD_VALUE(LPTHandle, rom_value_arr[rom_startaddress+i]);
  //get acknowledge
   if (I2C_GET_ACK(LPTHandle)) //error with no ack signal from target
   { return -3; }
  }

  
  //STOP
  I2C_STOP(LPTHandle) ;
 
  Delay(T_wait); // Twr=100ms, waiting for EEPROM(ADuC7020) internal data written.
  
  return 0;
}

