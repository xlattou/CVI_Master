#include "NT25L90.h"
#include <utility.h>
#include <userint.h>

int MEGA88NT25_Password1(int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, 0xA2, 0x7B, 0, 0, 0, 0, 0.1);
	if (error<0) {MessagePopup ("ERROR", "NO Acknowledge from target !");  return -1;}
	
	return 0;
}

int MEGA88NT25_Password2(int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, 0xA2, 0x7B, 'S', 'O', 'E', 'B', 0.1);
	if (error<0) {MessagePopup ("ERROR", "NO Acknowledge from target !");  return -1;}
	
	return 0;
}

int MEGA88NT25_SET_TRxConfigFlag(int inst_I2C, struct sTRxConfig_DEF_Nt25L90 TRxConfigFlag)
{
	int error, reg_add;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 1);
	if (error) return -1;
	
	Nt25l90A2table1.sStr.sTRxConfig = TRxConfigFlag; 
	reg_add =(int)(&Nt25l90A2table1.sStr.sTRxConfig) - (int)(&Nt25l90A2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set ben_active error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_Table(int inst_I2C, BYTE Table)
{
	int error, reg_add; 
	union uA2 localA2;

	localA2.sStrA2.LUTIndex = Table; 
	
	reg_add = 127;
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, reg_add, localA2.sStrA2.LUTIndex, 0.1);	
	if (error<0) {MessagePopup ("ERROR", "MEGA88 set table error!"); return -1;}   
		
	return 0;
}

int MEGA88NT25_SET_FactoryFlag (int inst_I2C, int flag)
{
	int error, reg_add;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 1);
	if (error) return -1;
	
	Nt25l90A2table1.sStr.FactoryFlag = flag;    
		
	reg_add =(int)(&Nt25l90A2table1.sStr.FactoryFlag) - (int)(&Nt25l90A2table1.sStr.FirstByte);    
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table1.pStr, 0.1); 
	if (error<0) return -1; 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_GET_FactoryFlag (int inst_I2C, int *flag)
{
	int error, reg_add;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 1);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, Nt25l90A2table1.pStr); 
	if (error<0) return -1; 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	*flag = Nt25l90A2table1.sStr.FactoryFlag; 
	
	return 0;
}

int MEGA88NT25_SET_Reg_EEPROM (int inst_I2C, int reg[14])
{
	int error, reg_add;
	int i;

	error = MEGA88NT25_SET_Table (inst_I2C, 1);
	if (error) return -1;
	
	reg_add =(int)(&Nt25l90A2table1.sStr.fTXSET0) - (int)(&Nt25l90A2table1.sStr.FirstByte); 
	
	for (i=0; i<14; i++)
	{Nt25l90A2table1.pStr[reg_add+i]=reg[i];}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 14, Nt25l90A2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_Reg_RAM (int inst_I2C, int reg[14])
{
	int error, reg_add;
	int i;

	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fTXSET0) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	
	//只能单个字节写入，否则写不进去
	for (i=0; i<14; i++)
	{
		error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, reg_add+i, reg[i], 0.1);
		if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	}
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_LOS_RAM (int inst_I2C, int DAC)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fLOSLEVEL=DAC;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fLOSLEVEL) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_LOS_EEPROM (int inst_I2C, int DAC)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 1);
	if (error) return -1;

	Nt25l90A2table1.sStr.fLOSLEVEL=DAC;
	
	reg_add =(int)(&Nt25l90A2table1.sStr.fLOSLEVEL) - (int)(&Nt25l90A2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_GET_Reg_RAM (int inst_I2C, INT8U reg[14])
{
	int error, reg_add;
	int i;

	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fTXSET0) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, reg_add, 14, Nt25l90A2table2.pStr);
	if (error) {MessagePopup ("ERROR", "MEGA88 read nt25l90 register to ram error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	for (i=0; i<14; i++)
	{reg[i]=Nt25l90A2table2.pStr[reg_add+i];}
	
	return 0;
}

int MEGA88NT25_GET_NVM_count (int inst_I2C, int *num)
{
	int error, reg_add;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, Nt25l90A2table1.pStr); 
	if (error<0) return -1; 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	*num =(int) (Nt25l90A2table1.sStr.fSTATUS1 & 0x07); //取低3位 
	
	return 0;
}

int MEGA88NT25_Save_NVM (int inst_I2C)
{
	int error, reg_add; 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;
	
	reg_add = (int)(&Nt25l90A2table2.sStr.fUSER_NVM_PASS)-(int)(&Nt25l90A2table2.sStr.FirstByte); 
	Nt25l90A2table2.sStr.fUSER_NVM_PASS = 0x64;
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) return -1;   
	
	reg_add = (int)(&Nt25l90A2table2.sStr.fUSER_NVM_PRGM)-(int)(&Nt25l90A2table2.sStr.FirstByte); 
	Nt25l90A2table2.sStr.fUSER_NVM_PRGM = 0xC6;
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) return -1;   
	
	//The entire copy procedure should take no longer than 2.56s. 
	Delay(3);
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

//从手动模式到自动模式，最长需idle (256+96+128)*3.9ms=1.872秒等待pageA0/A2[]单元存储到EEPROM,
//因此保险起见，当设置到自动模式时，统一等待2s
int MEGA88NT25_SET_Mode_Auto(int inst_I2C)
{
	int error, reg_add;
	union uA2 localA2;
	
	localA2.sStrA2.mode=0;
		
	reg_add = 122; 
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, reg_add, localA2.sStrA2.mode, 2);
	if (error<0) return -1;

	return 0;
}

int MEGA88NT25_SET_Mode_Manual(int inst_I2C)
{
	int error, reg_add;
	union uA2 localA2;
	
	localA2.sStrA2.mode=1;
		
	reg_add = 122; 
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, reg_add, localA2.sStrA2.mode, 0.1);
	if (error<0) return -1;

	return 0;
}

int MEGA88NT25_SET_APC_RAM (int inst_I2C, int DAC)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fAPCSET=DAC;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fAPCSET) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_MOD_RAM (int inst_I2C, int DAC)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fIMODSET=DAC;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fIMODSET) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_GET_MOD_RAM (int inst_I2C, int *DAC)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	reg_add =(int)(&Nt25l90A2table2.sStr.fIMODSET) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;}   
	
	*DAC = Nt25l90A2table2.sStr.fIMODSET;	
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_LUT_APC (int inst_I2C, BYTE DAC)
{
	int error;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 5);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, 253, DAC, 0.1);
	if (error) return -1;
	
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, 250, DAC, 0.1);
	if (error) return -1;
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;

	return 0;  	
}

int MEGA88NT25_SET_LUT_MOD (int inst_I2C, BYTE DAC[81])
{
	int error, i;
	unsigned char myARR[256]; 
	
	for (i=0; i<81; i++)
	{
		myARR[128+i] = DAC[i];
	}

	//for table4
	error =  MEGA88NT25_SET_Table (inst_I2C, 4);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, 128, 81, myARR, 0.2);
	if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 

	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;

	for (i=0; i<81; i++)
	{
		if (myARR[128+i] != DAC[i])
		{MessagePopup ("Error", "MOD lut set error"); return -1;}
	}
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;

	return 0;  	
}

int MEGA88NT25_SET_A2TemperKBl (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T6_A2_TEMP_K1;
	
	Nt25l90A2table2.pStr[reg_add]   = gain >> 8;
	Nt25l90A2table2.pStr[reg_add+1] = gain & 0xFF;
	Nt25l90A2table2.pStr[reg_add+2] = offset >> 8;
	Nt25l90A2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88NT25_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_A2TemperKBh (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T6_A2_TEMP_K2;
	
	Nt25l90A2table2.pStr[reg_add]   = gain >> 8;
	Nt25l90A2table2.pStr[reg_add+1] = gain & 0xFF;
	Nt25l90A2table2.pStr[reg_add+2] = offset >> 8;
	Nt25l90A2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88NT25_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_A2TemperKB_Mon (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T3_TEMP_MON_G;
	
	Nt25l90A2table2.pStr[reg_add]   = gain >> 8;
	Nt25l90A2table2.pStr[reg_add+1] = gain & 0xFF;
	Nt25l90A2table2.pStr[reg_add+2] = offset >> 8;
	Nt25l90A2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88NT25_SET_Table (inst_I2C, 3);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_SET_A2Temper (int inst_I2C, float val)
{
	signed short t0;
	int error, reg_add; 
	
	t0 = (signed short int)(val * 256.);

	reg_add = 128+T6_A2_TEMP_T0;
	
	Nt25l90A2table2.pStr[reg_add]   = t0 >> 8;
	Nt25l90A2table2.pStr[reg_add+1] = t0 & 0xFF;

	error =  MEGA88NT25_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 2, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88NT25_GET_A2Temper_Mon (int inst_I2C, float *val)
{
	int error, reg_add;
	INT16U TEMP; 
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fTEMP_MSB) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, reg_add, 2, Nt25l90A2table2.pStr);
	if (error) {MessagePopup ("ERROR", "MEGA88 read nt25l90 register to ram error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
		
	TEMP = ((INT16U)Nt25l90A2table2.sStr.fTEMP_MSB<<8) + Nt25l90A2table2.sStr.fTEMP_LSB;
	*val = (TEMP * 192.3/65535) -53.7;

	return 0; 
}

int MEGA88NT25_GET_Temper_Index (int inst_I2C, float *val)
{
	int error, i;
	unsigned char myARR[256]; 
	signed short int temp;
	union uA2 localA2;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;

	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	} 
	
	temp = (signed short int) (((0x00FF & localA2.sStrA2.reserved3[1]) << 8 ) + (0x00FF & localA2.sStrA2.reserved3[0]));  
	*val = (float) (temp/256.);

	return 0;
}

int MEGA88NT25_SET_fAPCCTRL0 (int inst_I2C, BYTE Data)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fAPCCTRL0=Data;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fAPCCTRL0) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;	
}

int MEGA88NT25_SET_fBIASDAC_MSB (int inst_I2C, BYTE MSB)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fBIASDAC_MSB=MSB;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fBIASDAC_MSB) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;	
}

int MEGA88NT25_SET_fBIASDAC_LSB (int inst_I2C, BYTE LSB)
{
	int error, reg_add;
	
	error = MEGA88NT25_SET_Table (inst_I2C, 2);
	if (error) return -1;

	Nt25l90A2table2.sStr.fBIASDAC_LSB=LSB;
	
	reg_add =(int)(&Nt25l90A2table2.sStr.fBIASDAC_LSB) - (int)(&Nt25l90A2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 1, Nt25l90A2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88NT25_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;	
}
