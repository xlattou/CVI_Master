#include <utility.h>
#include <I2C_LPT.h>

float I2C_DELAY=0.000;

int I2C_position=0;
//如果并口转I2C电路板异常，则I2C_position=-1;
//如果并口转I2C电路板的逻辑是SDA:!D1/!S5, SCL:!D0，则I2C_position=0;
//如果并口转I2C电路板的逻辑是SDA:!D1/ S5, SCL:!D0，则I2C_position=1;


void SetSCL(int LPTHandle, unsigned char value)
{unsigned char byte_temp, value_not;

    byte_temp = inp(LPTHandle);//注释掉此句话是为了提速

	value_not = !value ;

    if (I2C_position) //SCL_out=!D0
  		byte_temp = (byte_temp & 0xfe) | (value_not);
	else			  //SCL_out=!D0
  		byte_temp = (byte_temp & 0xfe) | (value_not);
	
    outp(LPTHandle, byte_temp);

	Delay(I2C_DELAY);
	return;
}

void SetSDA(int LPTHandle, unsigned char value)
{unsigned char byte_temp, value_not;

    byte_temp = inp(LPTHandle);//注释掉此句话是为了提速
	
  	value_not = !value;	

    if (I2C_position) //SDA_out=!D1
  		byte_temp = (byte_temp & 0xfd) | (value_not << 1);
	else			  //SDA_out=!D1
  		byte_temp = (byte_temp & 0xfd) | (value_not << 1);
  	
	outp(LPTHandle, byte_temp);

	Delay(I2C_DELAY);
  	return;
}

unsigned char GetSDA(int LPTHandle)
{unsigned char byte_temp;

    byte_temp = inp(LPTHandle+1); 

    if (I2C_position) //SDA_in=S5
  		byte_temp = ((byte_temp & 0x20) >> 5); 
	else			  //SDA_out=!S5
  		byte_temp = !((byte_temp & 0x20) >> 5); 
	
  	return byte_temp;
}

