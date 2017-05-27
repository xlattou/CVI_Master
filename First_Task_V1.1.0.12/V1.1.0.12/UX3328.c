#include "cvidll.h" 
#include <ansi_c.h>
#include <userint.h>
#include "ux3328.h"

//union uUX3328 localUX3328;
int UX3328_ADDR=0xA2;  

int ux3328_select_table (int inst_I2C, int Table) 
{
	int error;
	
	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x7F, Table, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_ENTRY_PW2 (int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x7B, 'S', 'O', 'E', 'B', 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328_ENTRY_PW1 (int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x7B, 0, 0, 0, 0, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_ENTRY_PW0(int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x7B, 0xFF, 0xFF, 0xFF, 0xFF, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_SET_PW (int inst_I2C)
{
	int error; 
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 0xB3, 0, 0, 0, 0, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 0xB7, 'S', 'O', 'E', 'B', 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328_set_mode (int inst_I2C, int mode)
{
	int error, reg_add;  
	union uUX3328 localUX3328;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) return -1;
	
	//1 ��ʾenable
	localUX3328.sStr.uTable.TABLE3.TX_CTRL1.ModLutEn=!mode;
//	localUX3328.sStr.uTable.TABLE3.TX_CTRL1.BiasLutEn=mode; 
	//130
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_CTRL1)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//0 ��ʾenable
	localUX3328.sStr.uTable.TABLE3.TX_CTRL4.ApdLutEn=!mode; 
	//137
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_CTRL4)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_get_bias_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.bias_dac;
		
	return 0; 
}

int ux3328_get_biaslut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.BIAS_LUT;
		
	return 0; 
}

int ux3328_get_mod_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.mod_dac;
		
	return 0; 
}

int ux3328_get_modlut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.MOD_LUT;
		
	return 0; 
}

int ux3328_get_apdlut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.APD_LUT;
		
	return 0; 
}

int ux3328_get_temper (int inst_I2C, float *val)
{
	int 	error, reg_add;
	float 	slope, offset;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
//	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
//	
//	localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE =(localUX3328.pStr[reg_add]<<8) + localUX3328.pStr[reg_add+1];
//	localUX3328.sStr.uTable.TABLE3.TEMP_OFFSET=(localUX3328.pStr[reg_add+2]<<8) + localUX3328.pStr[reg_add+3]; 
	
//	slope  = localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE/256.;
//	offset = localUX3328.sStr.uTable.TABLE3.TEMP_OFFSET/256.;	
	
//	*val=localUX3328.sStr.uTable.TABLE3.TEMP_DATA*slope/2.+offset;
	
	*val=localUX3328.sStr.uTable.TABLE3.TEMP_DATA-UX3328_INDEXTEMP_COMP;  
		
	return 0; 
}

int ux3328_get_apd_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.apd_dac;
		
	return 0; 
}

int ux3328_set_bias_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	localUX3328.sStr.uTable.TABLE3.bias_dac=val; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.bias_dac)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_mod_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	localUX3328.sStr.uTable.TABLE3.mod_dac=val; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.mod_dac)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_apd_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	localUX3328.sStr.uTable.TABLE3.apd_dac=val; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.apd_dac)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_calibration_temper (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm);   
	
	localUX3328.pStr[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localUX3328.pStr[reg_add+2] = offset >> 8;	
	localUX3328.pStr[reg_add+3] = offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 4, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_calibration_bias (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.BIAS_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm);   
	
	localUX3328.pStr[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localUX3328.pStr[reg_add+2] = offset >> 8;	
	localUX3328.pStr[reg_add+3] = offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 4, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_calibration_tx (int inst_I2C, float tx_power, float *slope, short int *offset)
{
	int   error, reg_add;
	float temp, k;
	union uUX3328 localUX3328; 

	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm);   
		
	//�����ǰУ׼ϵ��
	localUX3328.pStr[reg_add]  =((INT16U)(1*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(1*256)) & 0xFF;
	localUX3328.pStr[reg_add+2]=0;	
	localUX3328.pStr[reg_add+3]=0+1;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 4, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
  
	//ת��Ϊ0.1uW
	temp = pow (10.0, (tx_power/10.)) * 10000.; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	k = temp/(localUX3328.pStr[102]*256.+localUX3328.pStr[103]);  
		
	localUX3328.pStr[reg_add]  =((INT16U)(k*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(k*256)) & 0xFF;
	localUX3328.pStr[reg_add+2]=0;	
	localUX3328.pStr[reg_add+3]=0;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 4, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	*slope = k;
	*offset= 1;
	
	return 0;
}

int ux3328_get_calibration_tx (int inst_I2C, float *slope, short int *offset)
{
	int   error, reg_add;
	union uUX3328 localUX3328; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	localUX3328.sStr.uTable.TABLE3.TX_SLOPE =(localUX3328.pStr[reg_add]<<8) + localUX3328.pStr[reg_add+1];
	localUX3328.sStr.uTable.TABLE3.TX_OFFSET=(localUX3328.pStr[reg_add+2]<<8) + localUX3328.pStr[reg_add+3]; 
	
	*slope  = localUX3328.sStr.uTable.TABLE3.TX_SLOPE/256.;
	*offset = localUX3328.sStr.uTable.TABLE3.TX_OFFSET;	
	
	return 0;
}

int ux3328_get_calibration_temper (int inst_I2C, float *slope, short int *offset)
{
	int error, reg_add; 
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE =(localUX3328.pStr[reg_add]<<8) + localUX3328.pStr[reg_add+1];
	localUX3328.sStr.uTable.TABLE3.TEMP_OFFSET=(localUX3328.pStr[reg_add+2]<<8) + localUX3328.pStr[reg_add+3]; 
	
	*slope  = localUX3328.sStr.uTable.TABLE3.TEMP_SLOPE/256.;
	*offset = localUX3328.sStr.uTable.TABLE3.TEMP_OFFSET;	

	return 0;
}

int ux3328_set_calibration_vcc (int inst_I2C, float slope, short int offset)
{
	int error, reg_add; 
	union uUX3328 localUX3328; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.VCC_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	localUX3328.pStr[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localUX3328.pStr[reg_add+2] = offset >> 8;	
	localUX3328.pStr[reg_add+3] = offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 4, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_get_calibration_vcc (int inst_I2C, float *slope, short int *offset)
{
	int error, reg_add; 
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.VCC_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	localUX3328.sStr.uTable.TABLE3.VCC_SLOPE =(localUX3328.pStr[reg_add]<<8) + localUX3328.pStr[reg_add+1];
	localUX3328.sStr.uTable.TABLE3.VCC_OFFSET=(localUX3328.pStr[reg_add+2]<<8) + localUX3328.pStr[reg_add+3]; 
	
	*slope  = localUX3328.sStr.uTable.TABLE3.VCC_SLOPE/256.;
	*offset = localUX3328.sStr.uTable.TABLE3.VCC_OFFSET;	

	return 0;
}

int ux3328_get_calibration_bias (int inst_I2C, float *slope, short int *offset)
{
	int error, reg_add; 
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.BIAS_SLOPE)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	localUX3328.sStr.uTable.TABLE3.BIAS_SLOPE =(localUX3328.pStr[reg_add]<<8) + localUX3328.pStr[reg_add+1];
	localUX3328.sStr.uTable.TABLE3.BIAS_OFFSET=(localUX3328.pStr[reg_add+2]<<8) + localUX3328.pStr[reg_add+3]; 
	
	*slope  = localUX3328.sStr.uTable.TABLE3.BIAS_SLOPE/256.;
	*offset = localUX3328.sStr.uTable.TABLE3.BIAS_OFFSET;	

	return 0;
}

int ux3328_get_calibration_mode (int inst_I2C, INT8U val)
{
	int error, reg_add; 
	INT8U temp;
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.DDM_CTRL)-(int)(&localUX3328.sStr.tempHighAlarm); 
	
	temp = localUX3328.pStr[reg_add] & 0x3F; 
	 
	localUX3328.pStr[reg_add] = temp | (val<<6); 

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0; 
}

int ux3328_set_checksum_A2_Table (int inst_I2C)
{
int 	error,i;
INT8U 	checksum;
INT8U   CC_BASE;
INT8U	CC_EXT;
INT8U 	rom_value_arr[256]={0xFF};	
union uUX3328 localUX3328; 
int     count = 0; 

	SetWaitCursor (1); 

	//a2 table check sum 
	checksum=0; 
	
	error = ux3328_select_table (inst_I2C, 4);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<60; i++)
	{checksum += localUX3328.pStr[i+128]; }

	error = ux3328_select_table (inst_I2C, 5);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<60; i++)
	{checksum += localUX3328.pStr[i+128]; }

	error = ux3328_select_table (inst_I2C, 6);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<60; i++)
	{checksum += localUX3328.pStr[i+128]; }
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<51; i++)
	{checksum += localUX3328.pStr[i+128]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328_ADDR, 202, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	/*******************************************************************************/
	//��A0
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA0, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//re-calculate check-sum, A0h[0~62]
	CC_BASE=0;
	for (count=0; count<63; count++)
	{ CC_BASE += localUX3328.pStr[count]; }
	localUX3328.pStr[63] = CC_BASE;
	
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA0, 63, CC_BASE, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	//re-calculate check-sum, A0h[64~94]
	CC_EXT=0;
	for (count=64; count<95; count++)
	{ CC_EXT += localUX3328.pStr[count]; }
	localUX3328.pStr[95] = CC_EXT;
	
	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA0, 95, CC_EXT, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}  
	
	/*******************************************************************************/
	//��A2
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	CC_EXT=0;
	for (count=0; count<95; count++)
	{CC_EXT += localUX3328.pStr[count]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, 95, CC_EXT, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	SetWaitCursor (0); 
	
	return 0;
}

int ux3328_set_checksum_A2 (int inst_I2C)
{
int 	error,i;
INT8U 	checksum;
INT8U 	rom_value_arr[256]={0xFF};
union uUX3328 localUX3328; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, rom_value_arr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	checksum=0;
	for (i=0; i<95; i++)
	{checksum += rom_value_arr[i]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, 95, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_checksum_A0 (int inst_I2C)
{
int 	error,i;
INT8U 	checksum;
INT8U 	rom_value_arr[256]={0xFF};	

	//a0 check sum
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA0, 0, 256, rom_value_arr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	//re-calculate check-sum, A0h[0~62]
	checksum=0;
	for (i=0; i<63; i++)
	{checksum += rom_value_arr[i]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA0, 63, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//re-calculate check-sum, A0h[64~94]
	checksum=0;
	for (i=64; i<95; i++)
	{checksum += rom_value_arr[i]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA0, 95, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_default (int inst_I2C)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	//��localUX3328оƬĬ������£���Ҫ��һ��table4��table5��table6��д���������ܱ���оƬ�ڲ�EEPROM��ֵ��RAM��ֵһ�£���������У��Ͳ���Ч
	error = ux3328_select_table (inst_I2C, 4);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x80, 4, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = ux3328_select_table (inst_I2C, 5);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x80, 5, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = ux3328_select_table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328_ADDR, 0x80, 6, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//�л���table3
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//set 0x80 to 0xAA
	localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier0=0xAA; 
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier0)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	localUX3328.sStr.uTable.TABLE3.TX_CTRL4.TX_TEST1=0x00;
	localUX3328.sStr.uTable.TABLE3.TX_CTRL4.TX_TEST2=0x09;
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_CTRL4)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	localUX3328.sStr.uTable.TABLE3.TX_CTRL5.TX_TEST3=0x01;
	localUX3328.sStr.uTable.TABLE3.TX_CTRL5.TX_TEST4=0x00;
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TX_CTRL5)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	localUX3328.sStr.uTable.TABLE3.TEMP_RX_CTRL.TX_TEST5=0x01;
	localUX3328.sStr.uTable.TABLE3.TEMP_RX_CTRL.TX_TEST6=0x00;
	localUX3328.sStr.uTable.TABLE3.TEMP_RX_CTRL.TX_TEST7=0x00; 
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.TEMP_RX_CTRL)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	localUX3328.sStr.uTable.TABLE3.INTER_TEST6=0x00; 
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.INTER_TEST6)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 2, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//����Ϊ����ֵ�ڲ�У׼ģʽ
	error = ux3328_get_calibration_mode (inst_I2C, 2);
	if (error) return -1; 

	//set 0xCB to 0x55
	localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier1=0x55; 
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier1)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_set_calibration_A2_T (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uA2 localA2;
	
	reg_add = (int)(&localA2.sStrA2.tempHighAlarm)-(int)(&localA2.sStrA2.tempSlope);  
	
	localA2.pStrA2[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localA2.pStrA2[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localA2.pStrA2[reg_add+2]= offset >> 8;	
	localA2.pStrA2[reg_add+3]= offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2.pStrA2, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;  
}

int ux3328_set_calibration_A2_V (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uA2 localA2;
	
	reg_add = (int)(&localA2.sStrA2.tempHighAlarm)-(int)(&localA2.sStrA2.voltageSlope);  
	
	localA2.pStrA2[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localA2.pStrA2[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localA2.pStrA2[reg_add+2]= offset >> 8;	
	localA2.pStrA2[reg_add+3]= offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2.pStrA2, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;  
}

int ux3328_set_calibration_A2_Tx_I (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uA2 localA2;
	
	reg_add = (int)(&localA2.sStrA2.tempHighAlarm)-(int)(&localA2.sStrA2.tx_ISlope);  
	
	localA2.pStrA2[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localA2.pStrA2[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localA2.pStrA2[reg_add+2]= offset >> 8;	
	localA2.pStrA2[reg_add+3]= offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2.pStrA2, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;  
}

int ux3328_set_calibration_A2_Tx_PWR (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uA2 localA2;
	
	reg_add = (int)(&localA2.sStrA2.tempHighAlarm)-(int)(&localA2.sStrA2.tx_PwrSlope);  
	
	localA2.pStrA2[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localA2.pStrA2[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localA2.pStrA2[reg_add+2]= offset >> 8;	
	localA2.pStrA2[reg_add+3]= offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2.pStrA2, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;  
}

int ux3328_set_calibration_A2_Rx_PWR (int inst_I2C, float Rx_PWR4, float Rx_PWR3, float Rx_PWR2, float Rx_PWR1, float Rx_PWR0)
{
	int error, reg_add, temp; 
	union uA2 localA2;
	
	//��Ϊ���������Լ��ṹ�嶨�����⣬�˴��ĵ�ַֻ���ù̶���ֵ�ķ�ʽ
//	reg_add = (int)(&A2.sStrA2.tempHighAlarm)-(int)(&A2.sStrA2.rx_PWR4); 
	reg_add = 56;
	
	temp = MyDLLFloattoInt (Rx_PWR4);
	localA2.pStrA2[reg_add]   =(temp & 0xFF000000) >> 24;
	localA2.pStrA2[reg_add+1] =(temp & 0xFF0000) >> 16; 
	localA2.pStrA2[reg_add+2] =(temp & 0xFF00) >> 8;  
	localA2.pStrA2[reg_add+3] =(temp & 0xFF) >> 0;  
	
	temp = MyDLLFloattoInt (Rx_PWR3);
	localA2.pStrA2[reg_add+4] =(temp & 0xFF000000) >> 24;
	localA2.pStrA2[reg_add+5] =(temp & 0xFF0000) >> 16; 
	localA2.pStrA2[reg_add+6] =(temp & 0xFF00) >> 8;  
	localA2.pStrA2[reg_add+7] =(temp & 0xFF) >> 0;  
	
	temp = MyDLLFloattoInt (Rx_PWR2);
	localA2.pStrA2[reg_add+8]  =(temp & 0xFF000000) >> 24;
	localA2.pStrA2[reg_add+9]  =(temp & 0xFF0000) >> 16; 
	localA2.pStrA2[reg_add+10] =(temp & 0xFF00) >> 8;  
	localA2.pStrA2[reg_add+11] =(temp & 0xFF) >> 0;  
	
	temp = MyDLLFloattoInt (Rx_PWR1);
	localA2.pStrA2[reg_add+12] =(temp & 0xFF000000) >> 24;
	localA2.pStrA2[reg_add+13] =(temp & 0xFF0000) >> 16; 
	localA2.pStrA2[reg_add+14] =(temp & 0xFF00) >> 8;  
	localA2.pStrA2[reg_add+15] =(temp & 0xFF) >> 0;  
	
	temp = MyDLLFloattoInt (Rx_PWR0);
	localA2.pStrA2[reg_add+16] =(temp & 0xFF000000) >> 24;
	localA2.pStrA2[reg_add+17] =(temp & 0xFF0000) >> 16; 
	localA2.pStrA2[reg_add+18] =(temp & 0xFF00) >> 8;  
	localA2.pStrA2[reg_add+19] =(temp & 0xFF) >> 0;  
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 20, localA2.pStrA2, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0; 
}

int ux3328_fit_mod_iClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD, double Imod030_10, double Imod82_105)
{
	double Imod, Imod33, Imod10, Imod60, Imod82, Imod030, Imod105;
	int    error, DAC[6], Temper[6], i, j, fitNumber=6, Temper_out_arr[UX3328_TEMP_MAX], LUT_out_arr[UX3328_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328_AD_NUM];
	short int offset;
	
	//���㵱ǰImod����
	Imod=DAC_MOD/255.*80.;
	
	//���㵱ǰ�¶�
	error = ux3328_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//����66���Imod
	if (temper<66)                     	
	{
		Imod33=Imod+Imod10_33*(66-temper);
	}
	else if (temper>=66 && temper<93)	
	{
		Imod33=Imod-Imod33_60*(temper-66);
	}
	else if (temper>=93 && temper<111)
	{
		Imod60=Imod-Imod60_82*(temper-93); 
		Imod33=Imod60-Imod33_60*(93-66);
	}
	else
	{
		MessagePopup ("��ʾ", "UX3328�ں��¶ȸ���111�ȣ�������MOD����¶ȷ�Χ��   "); 
		return -1;
	} 

	//������ϲ�������10��33��60��82��Imod       //30  66  93  111    132
	Imod10=Imod33-Imod10_33*(66-30);
	Imod60=Imod33+Imod33_60*(93-66); 
	Imod82=Imod60+Imod60_82*(111-93); 
	Imod030=Imod10-Imod030_10*(30-(-6));
	Imod105=Imod82+Imod82_105*(132-111);
	
	//����Imod�����Ӧ�¶��µ�DAC
	DAC[0]=Imod030/80.*255;
	DAC[1]=Imod10/80.*255;
	DAC[2]=Imod33/80.*255;
	DAC[3]=Imod60/80.*255;
	DAC[4]=Imod82/80.*255;
	DAC[5]=Imod105/80.*255;
	
	//���DAC�Ƿ����������� 
	for (i=0; i<6; i++)
	{
		if (DAC[i]>255) {MessagePopup ("ERROR", "�������DACֵ���"); return -1;} 
	}
	
	Temper[0]=-6;
	Temper[1]=30;
	Temper[2]=66;
	Temper[3]=93;
	Temper[4]=111;
	Temper[5]=132;
	
	//�ֶ����-40~126�Ĳ��ұ�
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	if (slope_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//������ұ�
	for (i=0; i<UX3328_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		Temper_out_arr[i] = temper;
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��4095��Χ��ֵ����
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//�ͷ��ڴ�
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//����ux3328���¶�AD���飬���Ҷ�Ӧ���¶��������������¶�������ȡDACֵ��ux3328�Ĳ��ұ�����
	for (i=0; i<UX3328_AD_NUM; i++)
	{
		//��ȡAD�����Ӧ���¶�ֵ
		temp=UX3328_AD[i]-UX3328_INDEXTEMP_COMP;
		//�����¶�ֵ����������е�����
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	//д���ұ�
	error = ux3328_set_lut_ex (inst_I2C, ux3328_lut, 5);
	if (error) return -1;
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

int ux3328_fit_mod_cClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD)
{
	double Imod, Imod33, Imod10, Imod60, Imod82;
	int    error, DAC[4], Temper[4], i, j, fitNumber=4, Temper_out_arr[UX3328_TEMP_MAX], LUT_out_arr[UX3328_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328_AD_NUM];
	short int offset;
	
	//���㵱ǰImod����
	Imod=DAC_MOD/255.*80.;
	
	//���㵱ǰ�¶�
	error = ux3328_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//����66���Imod
	if (temper<66)                     	
	{
		Imod33=Imod+Imod10_33*(66-temper);
	}
	else if (temper>=66 && temper<93)	
	{
		Imod33=Imod-Imod33_60*(temper-66);
	}
	else if (temper>=93 && temper<111)
	{
		Imod60=Imod-Imod60_82*(temper-93); 
		Imod33=Imod60-Imod33_60*(93-66);
	}
	else
	{
		MessagePopup ("��ʾ", "UX3328�ں��¶ȸ���111�ȣ�������MOD����¶ȷ�Χ��   "); 
		return -1;
	} 

	//������ϲ�������10��33��60��82��Imod    //30  66  93  111 
	Imod10=Imod33-Imod10_33*(66-30);
	Imod60=Imod33+Imod33_60*(93-66); 
	Imod82=Imod60+Imod60_82*(111-93); 
	
	//����Imod�����Ӧ�¶��µ�DAC
	DAC[0]=Imod10/80.*255;
	DAC[1]=Imod33/80.*255;
	DAC[2]=Imod60/80.*255;
	DAC[3]=Imod82/80.*255;
	
	//���DAC�Ƿ����������� 
	for (i=0; i<4; i++)
	{
		if (DAC[i]>255) {MessagePopup ("ERROR", "�������DACֵ���"); return -1;} 
	}
	
	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;
	Temper[3]=111;
	
	//�ֶ����-40~126�Ĳ��ұ�
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	if (slope_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//������ұ�
	for (i=0; i<UX3328_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		Temper_out_arr[i] = temper;
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��4095��Χ��ֵ����
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//�ͷ��ڴ�
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//����ux3328���¶�AD���飬���Ҷ�Ӧ���¶��������������¶�������ȡDACֵ��ux3328�Ĳ��ұ�����
	for (i=0; i<UX3328_AD_NUM; i++)
	{
		//��ȡAD�����Ӧ���¶�ֵ
		temp=UX3328_AD[i]-UX3328_INDEXTEMP_COMP;
		//�����¶�ֵ����������е�����
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	//д���ұ�
	error = ux3328_set_lut_ex (inst_I2C, ux3328_lut, 5);
	if (error) return -1;
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

int ux3328_get_lut (int inst_I2C, INT8U LUT[60])
{
	int error, i;
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<60; i++)
	{LUT[i]=localUX3328.pStr[128+i];}
	
	return 0;
}

int ux3328_set_lut (int inst_I2C, INT8U LUT[60])
{
	int error, i;
	union uUX3328 localUX3328; 
	
	for (i=0; i<60; i++)
	{localUX3328.pStr[128+i]=LUT[i];}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}	 

int ux3328_set_table3 (int inst_I2C, int reg[60])
{
	int error, i;
	union uUX3328 localUX3328; 
	
	for (i=0; i<60; i++)
	{localUX3328.pStr[128+i]=reg[i];}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//����table3д��A0[128-187]
//	Delay(0.5);
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128, 60, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328_set_apc_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	localUX3328.sStr.uTable.TABLE3.Iapcset=val; 
	
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.Iapcset)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_get_apc_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.Iapcset;
		
	return 0; 
}

int ux3328_set_calibration_rx_clear (int inst_I2C)
{
	int err, reg_add;
	union uUX3328 localUX3328; 
	
	//158
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.RX_SLOPE2)-(int)(&localUX3328.sStr.tempHighAlarm);   
		
	//�����ǰУ׼ϵ��
	localUX3328.pStr[reg_add]  =((INT16U)(1*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(1*256)) & 0xFF;
	localUX3328.pStr[reg_add+2]=0;	
	localUX3328.pStr[reg_add+3]=0;
	localUX3328.pStr[reg_add+4]=((INT16U)(1*256)) >> 8;	
	localUX3328.pStr[reg_add+5]=((INT16U)(1*256)) & 0xFF;
	localUX3328.pStr[reg_add+6]=0;	
	localUX3328.pStr[reg_add+7]=0;
	localUX3328.pStr[reg_add+8]=((INT16U)(1*256)) >> 8;	
	localUX3328.pStr[reg_add+9]=((INT16U)(1*256)) & 0xFF;
	localUX3328.pStr[reg_add+10]=0;	
	localUX3328.pStr[reg_add+11]=0;

	//RX_COMP1
	localUX3328.pStr[reg_add+12]=0;	
	localUX3328.pStr[reg_add+13]=0;
	//RX_COMP0 
	localUX3328.pStr[reg_add+14]=0;	
	localUX3328.pStr[reg_add+15]=0;
	
	err = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 16, localUX3328.pStr, 0.1); 
	if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328_set_calibration_rx (int inst_I2C, float rx_power, float *slope, short int *offset)
{
	int   error, reg_add;
	float temp, k;
	union uUX3328 localUX3328; 

	error = ux3328_set_calibration_rx_clear (inst_I2C);
	if (error) return -1;
	
	//158
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.RX_SLOPE2)-(int)(&localUX3328.sStr.tempHighAlarm);   
		
	//ת��Ϊ0.1uW
	temp = pow (10.0, (rx_power/10.)) * 10000.; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	k = temp/(localUX3328.pStr[104]*256.+localUX3328.pStr[105]);  
		
	localUX3328.pStr[reg_add]  =((INT16U)(k*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(k*256)) & 0xFF;
	localUX3328.pStr[reg_add+2]=0;	
	localUX3328.pStr[reg_add+3]=0;
	localUX3328.pStr[reg_add+4]=((INT16U)(k*256)) >> 8;	
	localUX3328.pStr[reg_add+5]=((INT16U)(k*256)) & 0xFF;
	localUX3328.pStr[reg_add+6]=0;	
	localUX3328.pStr[reg_add+7]=0;
	localUX3328.pStr[reg_add+8]=((INT16U)(k*256)) >> 8;	
	localUX3328.pStr[reg_add+9]=((INT16U)(k*256)) & 0xFF;
	localUX3328.pStr[reg_add+10]=0;	
	localUX3328.pStr[reg_add+11]=0;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 12, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	*slope = k;
	*offset= 0;
	
	return 0;
}

int ux3328_set_calibration_rx_multi(int inst_I2C, double rx_power[4], unsigned short int ADC[4])
{
	double slope[3], offset[3], temp[2];
	int i, reg_add, err;
	INT16U temp_comp;
	union uUX3328 localUX3328; 
	
	//����slope��offset
	for (i=0; i<3; i++)
	{
		//ת��Ϊ0.1uW
		temp[0] = pow (10.0, (rx_power[i]/10.)) * 10000.;
		temp[1] = pow (10.0, (rx_power[i+1]/10.)) * 10000.; 
		
		slope[i] = (temp[0]-temp[1])/(ADC[i]-ADC[i+1]);
		offset[i]= temp[0]- slope[i]*ADC[i];
	}
	
	//158
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.RX_SLOPE2)-(int)(&localUX3328.sStr.tempHighAlarm);   

	localUX3328.pStr[reg_add]  =((INT16U)(slope[2]*256)) >> 8;	
	localUX3328.pStr[reg_add+1]=((INT16U)(slope[2]*256)) & 0xFF;
	localUX3328.pStr[reg_add+2]=((INT16S)(offset[2])) >> 8;	
	localUX3328.pStr[reg_add+3]=((INT16S)(offset[2])) & 0xFF;
	
	localUX3328.pStr[reg_add+4]=((INT16U)(slope[1]*256)) >> 8;	
	localUX3328.pStr[reg_add+5]=((INT16U)(slope[1]*256)) & 0xFF;
	localUX3328.pStr[reg_add+6]=((INT16S)(offset[1])) >> 8;	
	localUX3328.pStr[reg_add+7]=((INT16S)(offset[1]))& 0xFF;
	
	localUX3328.pStr[reg_add+8]=((INT16U)(slope[0]*256)) >> 8;	
	localUX3328.pStr[reg_add+9]=((INT16U)(slope[0]*256)) & 0xFF;
	localUX3328.pStr[reg_add+10]=((INT16S)(offset[0])) >> 8;	
	localUX3328.pStr[reg_add+11]=((INT16S)(offset[0])) & 0xFF;
	
	//RX_COMP1
	localUX3328.pStr[reg_add+12]=((INT16U)(ADC[2])) >> 8;	
	localUX3328.pStr[reg_add+13]=((INT16U)(ADC[2])) & 0xFF; 
	
	//RX_COMP0 
	localUX3328.pStr[reg_add+14]=((INT16U)(ADC[1])) >> 8;		
	localUX3328.pStr[reg_add+15]=((INT16U)(ADC[1])) & 0xFF; 
	
	err = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 16, localUX3328.pStr, 0.1); 
	if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_get_table3(int inst_I2C, INT8U table_data[])
{
	int error, i; 
	union uUX3328 localUX3328; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<UX3328_TABLE3_NUM; i++)
	{table_data[i]=localUX3328.pStr[128+i];}
	
	return 0;
}

int ux3328_set_table3_ex(int inst_I2C, INT8U table_data[])
{
	int error, i;
	INT8U Slave_Address;
	union uUX3328 localUX3328; 
	
	//�����ַ�洢���ֽ�
	table_data[49]=table_data[49] & 0xFC;
	
	for (i=0; i<60; i++)
	{localUX3328.pStr[128+i]=table_data[i];}
	
	//д128-178 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 51, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//��������ֹд179-186 
	
	//д187
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 187, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328_fit_apd (int inst_I2C, double kl, double kh, INT8U APD_DAC)
{
	double Vapd, Vapd30;
	int    error, DAC[3], Temper[3], i, j, fitNumber=3, Temper_out_arr[UX3328_TEMP_MAX], LUT_out_arr[UX3328_TEMP_MAX];
	float  temper, k, b;
	double slope, offset, *slope_arr, *offset_arr;
	int    TemperLimit_H=125, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328_AD_NUM];
	
	//���㵱ǰapd��ѹ
	Vapd=APD_DAC;
	
	//���㵱ǰ�¶�
	error = ux3328_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//����33���APD DAC	//66
	if (temper<66)                     	
	{
		Vapd30=Vapd+kl*(66-temper);
	}
	else
	{
		Vapd30=Vapd-kh*(temper-66); 
	} 

	//����Imod�����Ӧ�¶��µ�DAC
	DAC[0]=Vapd30-kl*(66-30);
	DAC[1]=Vapd30;
	DAC[2]=Vapd30+kh*(93-66);
	
	//���DAC�Ƿ����������� 
	for (i=0; i<fitNumber; i++)
	{
		if (DAC[i]>255) {MessagePopup ("ERROR", "�������DACֵ���"); return -1;} 
	}
	
/*	Temper[0]=0;
	Temper[1]=30;
	Temper[2]=60;	 */
	
	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;
	
	//�ֶ����-40~126�Ĳ��ұ�
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	if (slope_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//������ұ�
	for (i=0; i<UX3328_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		Temper_out_arr[i] = temper;
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��4095��Χ��ֵ����
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//�ͷ��ڴ�
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//����ux3328���¶�AD���飬���Ҷ�Ӧ���¶��������������¶�������ȡDACֵ��ux3328�Ĳ��ұ�����
	for (i=0; i<UX3328_AD_NUM; i++)
	{
		//��ȡAD�����Ӧ���¶�ֵ
		temp=UX3328_AD[i]-UX3328_INDEXTEMP_COMP;
		//�����¶�ֵ����������е�����
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	//д���ұ�
//	error = ux3328_select_table (inst_I2C, 6);
//	if (error) return -1;
	
	error = ux3328_set_lut_ex (inst_I2C, ux3328_lut, 6);
	if (error) return -1;
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

//��Ҫ����table3, table5, table6 =>  ��table6���ݵ�A2[128-187], table3���ݵ�A0[128-187]  table5���ݵ�A0[188-247]  
int ux3328_set_backup_A2_Table (int inst_I2C, int table)   
{
	int 	error,i;
	INT8U 	reg[255];
	union uUX3328 localUX3328; 

	if (5 == table)
	{
		error = ux3328_select_table (inst_I2C, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

		for (i=0; i<60; i++)
		{
			reg[128+60+i] = localUX3328.pStr[128+i];
		}
		//д��A0[188~247]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128+60, 60, reg, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	if (6 == table)
	{
		error = ux3328_select_table (inst_I2C, 6);
		if (error) return -1; 
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
		error = ux3328_select_table (inst_I2C, 0);
		if (error) return -1;
		//д��A2[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
		
		error = ux3328_select_table (inst_I2C, 3);
		if (error) return -1;
	} 
	if(3 == table)
	{
		error = ux3328_select_table (inst_I2C, 3);
		if (error) return -1;
		
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
		//д��A0[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128, 60, localUX3328.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 	  
	}
	return 0;
}	 

int ux3328_check_eepflag (int inst_I2C, BOOL flag)
{
	int error;
	union uUX3328 localUX3328; 
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	if (0xAA != localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier0)
	{
		if (!flag)
		{
			MessagePopup ("ERROR", "EEPROMIdenfier0 is not 0xAA !"); 
		}
		return -1;
	}
	
	if (0x55 != localUX3328.sStr.uTable.TABLE3.EEPROMIdenfier1)
	{
		if (!flag)
		{
			MessagePopup ("ERROR", "EEPROMIdenfier1 is not 0x55 !"); 
		}
		return -1;
	}
	
	if (0x00 != localUX3328.sStr.uTable.TABLE3.INIT_Status)
	{
		/***��ʱ����Ux3328 checksum��鹦��**wenyao.xi**20160315***/	
	
		if (!flag)
		{
			MessagePopup ("ERROR", "INIT_Status is not 0x00 !"); 
		}
		return -1;
	
	}
		
	return 0; 
}

//������table3, table5, table6����������д��A2 
int ux3328_write_backup_A2_Table (int inst_I2C)   
{
	int 	error,i;
	INT8U 	rom_value[256]={0xFF}, lut_arr[60]={0xFF};	

	//д���ֵ
	error = ux3328_set_default (inst_I2C);
	if (error) return -1; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA0, 0, 256, rom_value); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	//table3
	for (i=0; i<60; i++)
	{
		lut_arr[i] = rom_value[128+i];
	}
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = ux3328_set_table3_backup(inst_I2C, lut_arr);
	if (error) return -1;
	///////////////////////////////////////////////////
	//table 5
	for (i=0; i<60; i++)
	{
		lut_arr[i] = rom_value[128+60+i];
	}
	error = ux3328_select_table (inst_I2C, 5);
	if (error) return -1;
	
	error = ux3328_set_lut (inst_I2C, lut_arr);
	if (error) return -1;
	///////////////////////////////////////////////////
	//table6
	error = ux3328_select_table (inst_I2C, 0);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, rom_value); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<60; i++)
	{
		lut_arr[i] = rom_value[128+i];
	}
	error = ux3328_select_table (inst_I2C, 6);
	if (error) return -1;
	
	error = ux3328_set_lut (inst_I2C, lut_arr);
	if (error) return -1;
	////////////////////////////////////////////////////
	
	error = ux3328_set_checksum_A2_Table (inst_I2C); 
	if (error) return -1;  
	
	return 0;
}

int ux3328_set_table3_backup(int inst_I2C, INT8U table_data[])
{
	int error, i;
	INT8U Slave_Address;
	union uUX3328 localUX3328; 
	
	//�����ַ�洢���ֽ�
	table_data[49]=table_data[49] & 0xFC;
	
	for (i=0; i<51; i++)
	{localUX3328.pStr[128+i]=table_data[i];}
	
	//�������� 
	for (i=0; i<4; i++)
	{localUX3328.pStr[179+i]=0x00;}
	
	localUX3328.pStr[183] = 0x53;
	localUX3328.pStr[184] = 0x4F; 
	localUX3328.pStr[185] = 0x45; 
	localUX3328.pStr[186] = 0x42;
	
	//SECURITY_SEL
	localUX3328.pStr[187] = table_data[59]; 
	
	//д128-187 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328_set_lut_ex (int inst_I2C, INT8U LUT[60], int table)
{
	int 	error, i;
	INT8U 	lut_arr[256];
	union uUX3328 localUX3328; 
	
	error = ux3328_select_table (inst_I2C, table);
	if (error) return -1;
	
	for (i=0; i<60; i++)
	{localUX3328.pStr[128+i]=LUT[i];}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//����table3д��A0[179-238]
	if (5 == table)
	{
		for (i=0; i<60; i++)
		{
			lut_arr[128+60+i]=LUT[i];  
		}
		//д��A0[179~238]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128+60, 60, lut_arr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	//����table6д��A2[128-187]
	if (6 == table)
	{
		error = ux3328_select_table (inst_I2C, 0);
		if (error) return -1;
		//д��A2[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, 128, 60, localUX3328.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	
	return 0;
}

int ux3328_check_A2flag(int inst_I2C, BOOL flag)
{
	int error=0;
	
	error = ux3328_set_FactoryMode(inst_I2C);
	if (error) return -1;
	
	//���EEPROM��־λ�Ƿ�����
	error = ux3328_check_eepflag (inst_I2C, flag);
	if (error) return -1;
	
	return 0;
}

//¼��2�����룬��ѡA2 table3
int ux3328_set_FactoryMode(int InstI2C)
{
	int error;
	
	error = ux3328_ENTRY_PW2 (InstI2C);
	if (error) return -1;
	 
	error = ux3328_select_table (InstI2C, 3);
	if (error) return -1;
	
	return 0;
}

//¼��0������
int un3328_set_UserMode(int InstI2C)
{
	int error;
	
	error = ux3328_ENTRY_PW0(InstI2C);
	if (error) return -1;
	
	return 0;
}

int ux3328_set_los_dac(int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328 localUX3328; 
	
	localUX3328.sStr.uTable.TABLE3.RxAlarmDAC=val; 
	
	// reg_add=140
	reg_add = (int)(&localUX3328.sStr.uTable.TABLE3.RxAlarmDAC)-(int)(&localUX3328.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328_ADDR, reg_add, 1, localUX3328.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328_get_biasct_r_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328 localUX3328;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328_ADDR, 0, 256, localUX3328.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328.sStr.uTable.TABLE3.BIASCT_R;
		
	return 0; 
}

int ux3328_fit_APC_cClass (int inst_I2C, double Ibias10_33, double Ibias33_60, INT8U DAC_APC)
{
	double Ibias, Ibias33, Ibias10, Ibias60, Ibias82;
	int    error, DAC[4], Temper[4], i, j, fitNumber=3, Temper_out_arr[UX3328_TEMP_MAX], LUT_out_arr[UX3328_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328_AD_NUM];
	short int offset;
	
	const  float APC_SLOPE = 0.392;
	
	//���㵱ǰIbias����
	Ibias=DAC_APC*APC_SLOPE;
	
	//���㵱ǰ�¶�
	error = ux3328_get_temper (inst_I2C, &temper);
	if (error) 
	{
		return -1; 
	}
		
	//����66���Imod
	if (temper<66)                     	
	{
		Ibias33=Ibias+Ibias10_33*(66-temper);
	}
	else if (temper>=66 && temper<93)	
	{
		Ibias33=Ibias-Ibias33_60*(temper-66);
	}  
	else
	{
		MessagePopup ("��ʾ", "UX3328�ں��¶ȸ���111�ȣ�������MOD����¶ȷ�Χ��   "); 
		return -1;
	} 

	//������ϲ�������10��33��60��82��Imod    //30  66  93  111 
	Ibias10=Ibias33-Ibias10_33*(66-30);
	Ibias60=Ibias33+Ibias33_60*(93-66);   
	
	//����Imod�����Ӧ�¶��µ�DAC
	DAC[0]=Ibias10/APC_SLOPE;
	DAC[1]=Ibias33/APC_SLOPE;
	DAC[2]=Ibias60/APC_SLOPE;
	
	//���DAC�Ƿ����������� 
	for (i=0; i<3; i++)
	{
		if (DAC[i]>255) 
		{
			MessagePopup ("ERROR", "�������DACֵ���"); 
			return -1;
		} 
	}	  

	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;

	//�ֶ����-40~126�Ĳ��ұ�
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	if (slope_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("��ʾ", "���������Ҫ�˳���   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//������ұ�
	for (i=0; i<UX3328_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		Temper_out_arr[i] = temper;
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��4095��Χ��ֵ����
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//�ͷ��ڴ�
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//����ux3328���¶�AD���飬���Ҷ�Ӧ���¶��������������¶�������ȡDACֵ��ux3328�Ĳ��ұ�����
	for (i=0; i<UX3328_AD_NUM; i++)
	{
		//��ȡAD�����Ӧ���¶�ֵ
		temp=UX3328_AD[i]-UX3328_INDEXTEMP_COMP;
		//�����¶�ֵ����������е�����
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	//д���ұ�
	error = ux3328_set_lut_ex (inst_I2C, ux3328_lut, 4);
	if (error) return -1;
	
	error = ux3328_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}


