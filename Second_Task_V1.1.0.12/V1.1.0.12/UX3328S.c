#include "cvidll.h" 
#include <ansi_c.h>
#include <userint.h>
#include "ux3328s.h"

//union uUX3328S UX3328S;
int UX3328S_ADDR=0xA2;  

int ux3328s_select_table (int inst_I2C, int Table) 
{
	int error;
	
	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x7F, Table, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	return 0;
}

int ux3328s_get_table (int inst_I2C, int *Table) 
{
	int error;
	INT8U str;
	
	error = I2C_BYTE_READ_DLL (inst_I2C, UX3328S_ADDR, 0x7F, &str); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	*Table=(int)str;
	return 0;
}

int ux3328s_ENTRY_PW2 (int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x7B, 'S', 'O', 'E', 'B', 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328s_ENTRY_PW1 (int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x7B, 0, 0, 0, 0, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_SET_PW (int inst_I2C)
{
	int error; 
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0xCC, 0, 0, 0, 0, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0xD0, 'S', 'O', 'E', 'B', 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328s_set_mode (int inst_I2C, int mode)
{
	int error, reg_add;  
	
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) return -1;
	
	//1 标示enable
	localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL1.ModLutEn=!mode;
//	UX3328.sStr.uTable.TABLE3.TX_CTRL1.BiasLutEn=mode; 
	//130
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL1)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//0 标示enable
//	localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL4.ApdLutEn=!mode; 
	//137
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL4)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_get_bias_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.bias_dac;
		
	return 0; 
}

int ux3328s_get_biaslut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.BIASCT_R*4+localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_STATUS1.BIASCT_R;
		
	return 0; 
}

int ux3328s_get_mod_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.Imodc;
		
	return 0; 
}

int ux3328s_get_modlut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.IMOD_LUT;
		
	return 0; 
}

int ux3328s_get_apdlut_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.APD_LUT;
		
	return 0; 
}

int ux3328s_get_temper (int inst_I2C, float *val)
{
	int 	error, reg_add;
	float 	slope, offset;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_DATA-UX3328S_INDEXTEMP_COMP;  
		
	return 0; 
}

int ux3328s_get_apd_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.APD_DAC;
		
	return 0; 
}

int ux3328s_set_bias_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328S localUX3328S;
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.bias_dac=val; 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.bias_dac)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_mod_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	int tableindex;
	union uUX3328S localUX3328S;
	
	error=ux3328s_get_table(inst_I2C,&tableindex);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}   
	
	error=ux3328s_select_table (inst_I2C, 3);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}  
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.Imodc=val; 
	
	error=ux3328s_select_table (inst_I2C, 3);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.Imodc)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error=ux3328s_select_table (inst_I2C, tableindex);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328s_set_apd_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328S localUX3328S;
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.APD_DAC=val; 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.APD_DAC)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_calibration_temper (int inst_I2C, int temper_comp, float slope0, short int offset0, float slope1, short int offset1)
{
	int error, reg_add;
	union uUX3328S localUX3328S;
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_COMP)-(int)(&localUX3328S.sStr.tempHighAlarm);   
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_COMP = temper_comp;
	
	localUX3328S.pStr[reg_add]  =((INT16U)(slope1*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(slope1*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2] = offset1 >> 8;	
	localUX3328S.pStr[reg_add+3] = offset1 & 0xFF;
	
	localUX3328S.pStr[reg_add]  =((INT16U)(slope0*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(slope0*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2] = offset0 >> 8;	
	localUX3328S.pStr[reg_add+3] = offset0 & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 8, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_calibration_bias (int inst_I2C, float slope, short int offset)
{
	int error, reg_add;
	union uUX3328S localUX3328S;
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm);   
	
	localUX3328S.pStr[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2] = offset >> 8;	
	localUX3328S.pStr[reg_add+3] = offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_calibration_tx (int inst_I2C, float tx_power, float *slope, short int *offset)
{
	int   error, reg_add;
	float temp, k;
	union uUX3328S localUX3328S,localUX3328S_temp;
	unsigned char str_temp[256],str,str1;
	int tempArray[4],index;
	BOOL Flag=FALSE;
	int count=0;

	memset(str_temp,0,sizeof(str_temp));
	
//	Delay(1); 
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm);   
		
	//清除当前校准系数
	localUX3328S.pStr[reg_add]  =((INT16U)(1*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(1*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2]=0;	
	localUX3328S.pStr[reg_add+3]=0+1;
	tempArray[0]=localUX3328S.pStr[reg_add]  ;	
	tempArray[1]=localUX3328S.pStr[reg_add+1];
	tempArray[2]=localUX3328S.pStr[reg_add+2];	
	tempArray[3]=localUX3328S.pStr[reg_add+3];
 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	Delay(0.5);
	//回读判断
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0x7F, 1, localUX3328S_temp.pStr);  
	Delay(0.2);
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S_temp.pStr);  
	

	count=0;
	do
	{   Flag=FALSE;  
		for(index=0;index<4;index++)
		{
			if(3 !=localUX3328S_temp.pStr[127])
			{
				MessagePopup ("ERROR", "A2 Table索引号!=3!");
				return -1;
			}
			if(tempArray[index]!=localUX3328S_temp.pStr[index+reg_add])
			{
			//	MessagePopup ("ERROR", "清除校准系数失败 !");
				Flag=TRUE;
			}
			if(Flag)
			{
				error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
				if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
				Delay(0.5);
				//回读判断
				error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S_temp.pStr);  
			 	break;
			}
		}
	}while(count<10 && Flag);
	
	//转化为0.1uW
	temp = pow (10.0, (tx_power/10.)) * 10000.; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 

	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	k = temp/(localUX3328S.pStr[102]*256.+localUX3328S.pStr[103]);  

	localUX3328S.pStr[reg_add]  =((INT16U)(k*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(k*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2]=0;	
	localUX3328S.pStr[reg_add+3]=0;
	tempArray[0]=localUX3328S.pStr[reg_add]  ;	
	tempArray[1]=localUX3328S.pStr[reg_add+1];
	tempArray[2]=localUX3328S.pStr[reg_add+2];	
	tempArray[3]=localUX3328S.pStr[reg_add+3];
	
//	Delay(2); 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	*slope = k;
	*offset= 1;
	
	Delay(0.5);
	//回读判断
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0x7F, 1, localUX3328S_temp.pStr);   
	Delay(0.2); 
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0xB5, 4, localUX3328S_temp.pStr); 
	
	count=0;
	do
	{   Flag=FALSE;
		for(index=0;index<4;index++)
		{
			if(3 !=localUX3328S_temp.pStr[127])
			{
				MessagePopup ("ERROR", "A2 Table索引号!=3!");
				return -1;
			}
			if(tempArray[index]!=localUX3328S_temp.pStr[index+reg_add])   
			{
			//	MessagePopup ("ERROR", "KB准系数回读比对失败 !");
				Flag=TRUE;
			}
			if(Flag)
			{
				error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
				if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
				Delay(0.5);
				//回读判断
				error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0xB5, 4, localUX3328S_temp.pStr); 
				break;
			}
		}
	}while(count<10 && Flag);
	
	return 0;
}

int ux3328s_get_calibration_tx (int inst_I2C, float *slope, short int *offset)
{
	int   error, reg_add;
	union uUX3328S localUX3328S;

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_SLOPE =(localUX3328S.pStr[reg_add]<<8) + localUX3328S.pStr[reg_add+1];
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_OFFSET=(localUX3328S.pStr[reg_add+2]<<8) + localUX3328S.pStr[reg_add+3]; 
	
	*slope  = localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_SLOPE/256.;
	*offset = localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_OFFSET;	
	
	return 0;
}

int ux3328s_get_calibration_temper (int inst_I2C, int *temp_comp, float *slope0, short int *offset0, float *slope1, short int *offset1)
{
	int error, reg_add; 
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_SLOPE1)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_SLOPE1 =(localUX3328S.pStr[reg_add]<<8) + localUX3328S.pStr[reg_add+1];
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_OFFSET1=(localUX3328S.pStr[reg_add+2]<<8) + localUX3328S.pStr[reg_add+3]; 
	
	*slope1  = localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_SLOPE1/256.;
	*offset1 = localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_OFFSET1;
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_SLOPE0 =(localUX3328S.pStr[reg_add+4]<<8) + localUX3328S.pStr[reg_add+1];
	localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_OFFSET0=(localUX3328S.pStr[reg_add+6]<<8) + localUX3328S.pStr[reg_add+3]; 
	
	*slope0  = localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_SLOPE0/256.;
	*offset0 = localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_OFFSET0;
	
	*temp_comp = localUX3328S.sStr.uTable.UX3328S_TABLE3.TEMP_COMP;

	return 0;
}

int ux3328s_set_calibration_vcc (int inst_I2C, float slope, short int offset)
{
	int error, reg_add; 
	union uUX3328S localUX3328S;
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	localUX3328S.pStr[reg_add]  =((INT16U)(slope*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(slope*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2] = offset >> 8;	
	localUX3328S.pStr[reg_add+3] = offset & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 4, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_get_calibration_vcc (int inst_I2C, float *slope, short int *offset)
{
	int error, reg_add; 
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_SLOPE =(localUX3328S.pStr[reg_add]<<8) + localUX3328S.pStr[reg_add+1];
	localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_OFFSET=(localUX3328S.pStr[reg_add+2]<<8) + localUX3328S.pStr[reg_add+3]; 
	
	*slope  = localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_SLOPE/256.;
	*offset = localUX3328S.sStr.uTable.UX3328S_TABLE3.VCC_OFFSET;	

	return 0;
}

int ux3328s_get_calibration_bias (int inst_I2C, float *slope, short int *offset)
{
	int error, reg_add; 
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_SLOPE)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_SLOPE =(localUX3328S.pStr[reg_add]<<8) + localUX3328S.pStr[reg_add+1];
	localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_OFFSET=(localUX3328S.pStr[reg_add+2]<<8) + localUX3328S.pStr[reg_add+3]; 
	
	*slope  = localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_SLOPE/256.;
	*offset = localUX3328S.sStr.uTable.UX3328S_TABLE3.BIAS_OFFSET;	

	return 0;
}

int ux3328s_get_calibration_mode (int inst_I2C, INT8U val)
{
	int error, reg_add; 
	union uUX3328S localUX3328S;
	INT8U temp;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_DDM_CTRL)-(int)(&localUX3328S.sStr.tempHighAlarm); 
	
	temp = localUX3328S.pStr[reg_add] & 0x3F; 
	 
	localUX3328S.pStr[reg_add] = temp | (val<<6); 

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0; 
}

int ux3328s_set_checksum_A2_Table (int inst_I2C)
{
	int 	error,i;
	union uUX3328S localUX3328S;
	INT8U 	checksum;
	INT8U 	rom_value_arr[256]={0xFF};	

	SetWaitCursor (1); 

	//a2 table check sum 
	checksum=0; 
	
	error = ux3328s_select_table (inst_I2C, 4);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
//	for (i=0x80; i<=0xE8; i++)
//	{checksum += localUX3328S.pStr[i]; }
	
	for (i=0; i<105; i++)
	{checksum += localUX3328S.pStr[i+128]; }   

	error = ux3328s_select_table (inst_I2C, 5);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
//	for (i=0x80; i<=0xE8; i++)
//	{checksum += localUX3328S.pStr[i]; }
	
	for (i=0; i<105; i++)
	{checksum += localUX3328S.pStr[i+128]; }
	
	error = ux3328s_select_table (inst_I2C, 6);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
//	for (i=0x80; i<=0xE8; i++)
//	{checksum += localUX3328S.pStr[i]; }
	
	for (i=0; i<105; i++)
	{checksum += localUX3328S.pStr[i+128]; }
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0x81; i<=0x8A; i++)
	{checksum += localUX3328S.pStr[i]; }
	
	for (i=0x8E; i<=0xCB; i++)
	{checksum += localUX3328S.pStr[i]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0xD4, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	SetWaitCursor (0); 
	
	return 0;
}

int ux3328s_set_checksum_A2 (int inst_I2C)
{
int 	error,i;
INT8U 	checksum;
INT8U 	rom_value_arr[256]={0xFF};	

	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, rom_value_arr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	checksum=0;
	for (i=0; i<95; i++)
	{checksum += rom_value_arr[i]; }

	error = I2C_BYTE_WRITE_DLL (inst_I2C, 0xA2, 95, checksum, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_checksum_A0 (int inst_I2C)
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


int ux3328s_set_default (int inst_I2C)
{
	int error, reg_add;
	
	//在UX3328芯片默认情况下，需要做一次table4、table5、table6的写操作，才能保持芯片内部EEPROM的值与RAM的值一致，这样计算校验和才有效
	error = ux3328s_select_table (inst_I2C, 4);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x80, 4, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = ux3328s_select_table (inst_I2C, 5);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x80, 5, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	error = ux3328s_select_table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x80, 6, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	//切换到table3
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = ux3328s_set_table3_default (inst_I2C);
	if (error) return -1; 
	
	error = ux3328s_set_checksum_A2_Table (inst_I2C);
	if (error) return -1;
	
	//设置为采样值内部校准模式
	error = ux3328s_get_calibration_mode (inst_I2C, 2);
	if (error) return -1; 

	return 0;
}


int ux3328s_set_calibration_A2_T (int inst_I2C, float slope, short int offset)
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

int ux3328s_set_calibration_A2_V (int inst_I2C, float slope, short int offset)
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

int ux3328s_set_calibration_A2_Tx_I (int inst_I2C, float slope, short int offset)
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

int ux3328s_set_calibration_A2_Tx_PWR (int inst_I2C, float slope, short int offset)
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

int ux3328s_set_calibration_A2_Rx_PWR (int inst_I2C, float Rx_PWR4, float Rx_PWR3, float Rx_PWR2, float Rx_PWR1, float Rx_PWR0)
{
	int error, reg_add, temp; 
	union uA2 localA2;
	
	//因为变量类型以及结构体定义问题，此处的地址只能用固定赋值的方式
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

int ux3328s_fit_mod_iClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD, double Imod030_10, double Imod82_105)
{
	double Imod, Imod33, Imod10, Imod60, Imod82, Imod030, Imod105;
	int    error, DAC[6], Temper[6], i, j, fitNumber=6, Temper_out_arr[UX3328S_TEMP_MAX], LUT_out_arr[UX3328S_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328S_AD_NUM];
	short int offset;
	
	//计算当前Imod电流
	Imod=DAC_MOD/255.*80.;
	
	//计算当前温度
	error = ux3328s_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//计算33°的Imod
/*	if (temper<33)                     	
	{
		Imod33=Imod+Imod10_33*(33-temper);
	}
	else if (temper>=33 && temper<60)	
	{
		Imod33=Imod-Imod33_60*(temper-33);
	}
	else if (temper>=60 && temper<82)
	{
		Imod60=Imod-Imod60_82*(temper-60); 
		Imod33=Imod60-Imod33_60*(60-33);
	}   */
	
	//计算66°的Imod
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
		MessagePopup ("提示", "UX3328内核温度高于111度，超出了MOD拟合温度范围！   "); 
		return -1;
	} 

	//根据拟合参数计算10、33、60、82的Imod      //30  66  93  111    132
	Imod10=Imod33-Imod10_33*(66-30);
	Imod60=Imod33+Imod33_60*(93-66); 
	Imod82=Imod60+Imod60_82*(111-93); 
	Imod030=Imod10-Imod030_10*(30-(-6));
	Imod105=Imod82+Imod82_105*(132-111);
	
	//根据Imod计算对应温度下的DAC
	DAC[0]=Imod030/80.*255;
	DAC[1]=Imod10/80.*255;
	DAC[2]=Imod33/80.*255;
	DAC[3]=Imod60/80.*255;
	DAC[4]=Imod82/80.*255;
	DAC[5]=Imod105/80.*255;
	
	//检查DAC是否有溢出的情况 
	for (i=0; i<6; i++)
	{
		if (DAC[i]>0x3FF) {MessagePopup ("ERROR", "计算出的DAC值溢出"); return -1;} 
	}
	
/*	Temper[0]=-30;
	Temper[1]=10;
	Temper[2]=33;
	Temper[3]=60;
	Temper[4]=82;
	Temper[5]=105;   */
	Temper[0]=-6;
	Temper[1]=30;
	Temper[2]=66;
	Temper[3]=93;
	Temper[4]=111;
	Temper[5]=132;
	
	//分段拟合-40~126的查找表
	slope_arr  = malloc ((fitNumber-1)*8); //开辟内存空间
	if (slope_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //计算slope和offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//计算查找表
	for (i=0; i<UX3328S_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //得到对应温度的slope和offset
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
		
		//对LUT计算值超出0和4095范围的值处理
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//释放内存
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//根据ux3328的温度AD数组，查找对应的温度索引，并根据温度索引获取DAC值到ux3328的查找表数组
	for (i=0; i<UX3328S_AD_NUM; i++)
	{
		//获取AD数组对应的温度值
		temp=UX3328S_AD[i]-UX3328S_INDEXTEMP_COMP;
		//查找温度值在拟合数组中的索引
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328S_TEMP_MAX; j++)
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
	
	//写查找表
//	error = ux3328_select_table (inst_I2C, 5);
//	if (error) return -1;
	
	error = ux3328s_set_lut_ex (inst_I2C, ux3328_lut, 5);
	if (error) return -1;
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

int ux3328s_fit_mod_cClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD)
{
	double Imod, Imod33, Imod10, Imod60, Imod82;
	int    error, DAC[4], Temper[4], i, j, fitNumber=4, Temper_out_arr[UX3328S_TEMP_MAX], LUT_out_arr[UX3328S_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT16U  ux3328s_lut[UX3328S_AD_NUM];
	INT8U   ux3328s_lut_to_table[105];
	short int offset;
	int     index;
	//计算当前Imod电流
//	Imod=DAC_MOD/255.*80.;
	
	//计算当前温度
	error = ux3328s_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//计算33°的Imod
/*	if (temper<33)                     	
	{
		Imod33=Imod+Imod10_33*(33-temper);
	}
	else if (temper>=33 && temper<60)	
	{
		Imod33=Imod-Imod33_60*(temper-33);
	}
	else if (temper>=60 && temper<82)
	{
		Imod60=Imod-Imod60_82*(temper-60); 
		Imod33=Imod60-Imod33_60*(60-33);
	}		 
*/
/*
	//计算66°的Imod
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
		MessagePopup ("提示", "UX3328内核温度高于111度，超出了MOD拟合温度范围！   "); 
		return -1;
	} 

	//根据拟合参数计算10、33、60、82的Imod  //30  66  93  111
	Imod10=Imod33-Imod10_33*(66-30);
	Imod60=Imod33+Imod33_60*(93-66); 
	Imod82=Imod60+Imod60_82*(111-93); 
	
	//根据Imod计算对应温度下的DAC
	DAC[0]=Imod10/80.*255*4;
	DAC[1]=Imod33/80.*255*4;
	DAC[2]=Imod60/80.*255*4;
	DAC[3]=Imod82/80.*255*4;
*/	
	//计算66°的DAC
	if (temper<66)                     	
	{
		DAC[1]=DAC_MOD+Imod10_33*(66-temper);
	}
	else if (temper>=66 && temper<93)	
	{
		DAC[1]=DAC_MOD-Imod33_60*(temper-66);
	}
	else if (temper>=93 && temper<111)
	{
		DAC[2]=DAC_MOD-Imod60_82*(temper-93); 
		DAC[1]=DAC[2]-Imod33_60*(93-66);
	}
	else
	{
		MessagePopup ("提示", "UX3328内核温度高于111度，超出了MOD拟合温度范围！   "); 
		return -1;
	} 


	DAC[0]=DAC[1]-Imod10_33*(66-30);
	DAC[2]=DAC[1]+Imod33_60*(93-66); 
	DAC[3]=DAC[2]+Imod60_82*(111-93); 

	DAC[0]=DAC[0]*4;
	DAC[1]=DAC[1]*4;
	DAC[2]=DAC[2]*4;
	DAC[3]=DAC[3]*4;

	//检查DAC是否有溢出的情况 
	for (i=0; i<4; i++)
	{
		if (DAC[i]>0x3FF) {MessagePopup ("ERROR", "计算出的DAC值溢出"); return -1;} 
	}
	
	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;
	Temper[3]=111;
	
	//分段拟合-40~126的查找表
	slope_arr  = malloc ((fitNumber-1)*8); //开辟内存空间
	if (slope_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //计算slope和offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//计算查找表
	for (i=0; i<UX3328S_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //得到对应温度的slope和offset
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
		
		//对LUT计算值超出0和4095范围的值处理
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0x3FF) 	LUT_out_arr[i] = 0x3FF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//释放内存
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//根据ux3328的温度AD数组，查找对应的温度索引，并根据温度索引获取DAC值到ux3328的查找表数组
	for (i=0; i<UX3328S_AD_NUM; i++)
	{
		//获取AD数组对应的温度值
		temp=UX3328S_AD[i]-UX3328S_INDEXTEMP_COMP;
		//查找温度值在拟合数组中的索引
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328S_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328s_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	for (index = 0; index < UX3328S_AD_NUM/4; index++)
	{
		ux3328s_lut_to_table[index*5]   =                                          ux3328s_lut[index*4]   /4 ;
		ux3328s_lut_to_table[index*5+1] = ((ux3328s_lut[index*4]   & 0x03) * 64) + ux3328s_lut[index*4+1] /16;   
		ux3328s_lut_to_table[index*5+2] = ((ux3328s_lut[index*4+1] & 0x0F) * 16) + ux3328s_lut[index*4+2] / 64;
		ux3328s_lut_to_table[index*5+3] = ((ux3328s_lut[index*4+2] & 0x3F) * 4)  + ux3328s_lut[index*4+3] / 256;  
		ux3328s_lut_to_table[index*5+4] = ((ux3328s_lut[index*4+3] & 0xFF)); 
	}
	//写查找表
	error = ux3328s_select_table (inst_I2C, 5);
	if (error) return -1;
	error = ux3328s_set_lut(inst_I2C, ux3328s_lut_to_table);
	if (error) return -1;
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

int ux3328s_get_lut (int inst_I2C, INT8U LUT[105])
{
	int error, i;
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<105; i++)
	{LUT[i]=localUX3328S.pStr[128+i];}
	
	return 0;
}

int ux3328s_set_lut (int inst_I2C, INT8U LUT[105])
{
	int error, i;
	union uUX3328S localUX3328S;
	
	for (i=0; i<105; i++)
	{
		localUX3328S.pStr[128+i]=LUT[i];
	}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 105, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328s_set_table3 (int inst_I2C, int reg[128])
{
	int error, i;
	union uUX3328S localUX3328S;
	
	for (i=0; i<128; i++)	  
	{
		localUX3328S.pStr[128+i]=reg[i];
	}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 76, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//备份table3写到A0[128-187]
//	Delay(0.5);
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128, 60, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328s_set_apc_dac (int inst_I2C, INT8U val)
{
	int error, reg_add;
	int tableindex;
	union uUX3328S localUX3328S;
	
	error=ux3328s_get_table(inst_I2C,&tableindex);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}   
//	Delay(0.1);
	error=ux3328s_select_table (inst_I2C, 3);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}  
//	Delay(0.1);
	localUX3328S.sStr.uTable.UX3328S_TABLE3.Iapcset=val; 
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.Iapcset)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
//	Delay(0.1);
	error=ux3328s_select_table (inst_I2C, tableindex);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	return 0;   
}

int ux3328s_get_apc_dac (int inst_I2C, INT8U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.Iapcset;
		
	return 0; 
}

int ux3328s_set_calibration_rx_clear (int inst_I2C)
{
	int err, reg_add;
	union uUX3328S localUX3328S;
	
	//158
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RX_SLOPE2)-(int)(&localUX3328S.sStr.tempHighAlarm);   
		
	//清除当前校准系数
	localUX3328S.pStr[reg_add]  =((INT16U)(1*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(1*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2]=0;	
	localUX3328S.pStr[reg_add+3]=0;
	localUX3328S.pStr[reg_add+4]=((INT16U)(1*256)) >> 8;	
	localUX3328S.pStr[reg_add+5]=((INT16U)(1*256)) & 0xFF;
	localUX3328S.pStr[reg_add+6]=0;	
	localUX3328S.pStr[reg_add+7]=0;
	localUX3328S.pStr[reg_add+8]=((INT16U)(1*256)) >> 8;	
	localUX3328S.pStr[reg_add+9]=((INT16U)(1*256)) & 0xFF;
	localUX3328S.pStr[reg_add+10]=0;	
	localUX3328S.pStr[reg_add+11]=0;
		  
	//RX_COMP1
	localUX3328S.pStr[reg_add+12]=0;	
	localUX3328S.pStr[reg_add+13]=0;
	//RX_COMP0 
	localUX3328S.pStr[reg_add+14]=0;	
	localUX3328S.pStr[reg_add+15]=0;
//	Delay(0.3);
	err = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 16, localUX3328S.pStr, 0.1); 
	if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	return 0;
}

int ux3328s_set_calibration_rx (int inst_I2C, float rx_power, float *slope, short int *offset, BOOL Flag)
{
	int   error, reg_add;
	union uUX3328S localUX3328S;
	float temp, k;

	error = ux3328s_set_calibration_rx_clear (inst_I2C);
	if (error) return -1;
	
	//158
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RX_SLOPE2)-(int)(&localUX3328S.sStr.tempHighAlarm);   
		
	//转化为0.1uW
	temp = pow (10.0, (rx_power/10.)) * 10000.; 

	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	k = temp/(localUX3328S.pStr[104]*256.+localUX3328S.pStr[105]);  
		
	localUX3328S.pStr[reg_add]  =((INT16U)(k*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(k*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2]=0;	
	localUX3328S.pStr[reg_add+3]=0;
	localUX3328S.pStr[reg_add+4]=((INT16U)(k*256)) >> 8;	
	localUX3328S.pStr[reg_add+5]=((INT16U)(k*256)) & 0xFF;
	localUX3328S.pStr[reg_add+6]=0;	
	localUX3328S.pStr[reg_add+7]=0;
	localUX3328S.pStr[reg_add+8]=((INT16U)(k*256)) >> 8;	
	localUX3328S.pStr[reg_add+9]=((INT16U)(k*256)) & 0xFF;
	localUX3328S.pStr[reg_add+10]=0;	
	localUX3328S.pStr[reg_add+11]=0;
	
	if (Flag)
	{
		localUX3328S.sStr.uTable.UX3328S_TABLE3.RX_OFFSET2 = 0x100;	  //OP客户要求无光监控为-40dBm
	} 
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 12, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	*slope = k;
	*offset= 0;
	
	return 0;
}

int ux3328s_set_calibration_rx_multi(int inst_I2C, double rx_power[4], unsigned short int ADC[4])
{
	double slope[3], offset[3], temp[2];
	int i, reg_add, err;
	union uUX3328S localUX3328S,localUX3328S_temp;
	INT16U temp_comp;
	BOOL Flag;
	int count;
	
	//计算slope和offset
	for (i=0; i<3; i++)
	{
		//转化为0.1uW
		temp[0] = pow (10.0, (rx_power[i]/10.)) * 10000.;
		temp[1] = pow (10.0, (rx_power[i+1]/10.)) * 10000.; 
		
		slope[i] = (temp[0]-temp[1])/(ADC[i]-ADC[i+1]);
		offset[i]= temp[0]- slope[i]*ADC[i];
	}
	
	//158
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RX_SLOPE2)-(int)(&localUX3328S.sStr.tempHighAlarm);   

	localUX3328S.pStr[reg_add]  =((INT16U)(slope[2]*256)) >> 8;	
	localUX3328S.pStr[reg_add+1]=((INT16U)(slope[2]*256)) & 0xFF;
	localUX3328S.pStr[reg_add+2]=((INT16S)(offset[2])) >> 8;	
	localUX3328S.pStr[reg_add+3]=((INT16S)(offset[2])) & 0xFF;
	
	localUX3328S.pStr[reg_add+4]=((INT16U)(slope[1]*256)) >> 8;	
	localUX3328S.pStr[reg_add+5]=((INT16U)(slope[1]*256)) & 0xFF;
	localUX3328S.pStr[reg_add+6]=((INT16S)(offset[1])) >> 8;	
	localUX3328S.pStr[reg_add+7]=((INT16S)(offset[1]))& 0xFF;
	
	localUX3328S.pStr[reg_add+8]=((INT16U)(slope[0]*256)) >> 8;	
	localUX3328S.pStr[reg_add+9]=((INT16U)(slope[0]*256)) & 0xFF;
	localUX3328S.pStr[reg_add+10]=((INT16S)(offset[0])) >> 8;	
	localUX3328S.pStr[reg_add+11]=((INT16S)(offset[0])) & 0xFF;
	
	//RX_COMP1
	localUX3328S.pStr[reg_add+12]=((INT16U)(ADC[2])) >> 8;	
	localUX3328S.pStr[reg_add+13]=((INT16U)(ADC[2])) & 0xFF; 
	
	//RX_COMP0 
	localUX3328S.pStr[reg_add+14]=((INT16U)(ADC[1])) >> 8;		
	localUX3328S.pStr[reg_add+15]=((INT16U)(ADC[1])) & 0xFF; 
	
	err = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 16, localUX3328S.pStr, 0.1); 
	if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	Delay(0.5);
	err = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, reg_add, 16, localUX3328S_temp.pStr); 
	if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	count=0;
	do
	{  	Flag=TRUE;  
		for(i=reg_add;i<(reg_add+16);i++)
		{
			if(localUX3328S.pStr[i]!=localUX3328S_temp.pStr[i])	
			{
			//	MessagePopup ("ERROR", "校准系数检查失败!");
				Flag=FALSE;
			}
			if(!Flag)
			{
				err = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 16, localUX3328S.pStr, 0.1); 
				if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
				Delay(0.5);
				err = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, reg_add, 16, localUX3328S_temp.pStr); 
				if (err<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}  
				break;
			}
		}
		count++;
	}while(count<10 && !Flag);
	
	return 0;
}

int ux3328s_get_table3(int inst_I2C, INT8U table_data[])
{
	int error, i; 
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<UX3328S_TABLE3_NUM; i++)
	{table_data[i]=localUX3328S.pStr[128+i];}
	
	return 0;
}

int ux3328s_set_table3_ex(int inst_I2C, INT8U table_data[])
{
	int error, i;
	union uUX3328S localUX3328S;
	INT8U Slave_Address;
	
	//处理地址存储器字节
//	table_data[49]=table_data[49] & 0xFC;		 
	
	for (i=0; i<128; i++)
	{localUX3328S.pStr[128+i]=table_data[i];}
	
	//写128-178 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 76, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//密码区禁止写179-186 
	
	//写187
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0xD5, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328s_fit_apd (int inst_I2C, double kl, double kh, INT8U APD_DAC)
{
	double Vapd, Vapd30;
	int    error, DAC[3], Temper[3], i, j, fitNumber=3, Temper_out_arr[UX3328S_TEMP_MAX], LUT_out_arr[UX3328S_TEMP_MAX];
	float  temper, k, b;
	double slope, offset, *slope_arr, *offset_arr;
	int    TemperLimit_H=125, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT8U  ux3328_lut[UX3328S_AD_NUM];

	//计算当前apd电压
	Vapd=APD_DAC;
	
	//计算当前温度
	error = ux3328s_get_temper (inst_I2C, &temper);
	if (error) return -1; 
		
	//计算33°的APD DAC  //66
	if (temper<66)                     	
	{
		Vapd30=Vapd+kl*(66-temper);
	}
	else
	{
		Vapd30=Vapd-kh*(temper-66); 
	} 

	//根据Imod计算对应温度下的DAC
	DAC[0]=Vapd30-kl*(66-30);
	DAC[1]=Vapd30;
	DAC[2]=Vapd30+kh*(93-66);
	
	//检查DAC是否有溢出的情况 
	for (i=0; i<fitNumber; i++)
	{
		if (DAC[i]>0x3FF) {MessagePopup ("ERROR", "计算出的DAC值溢出"); return -1;} 
	}
	
/*	Temper[0]=0;
	Temper[1]=30;
	Temper[2]=60;	 */
	
	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;
	
	//分段拟合-40~126的查找表
	slope_arr  = malloc ((fitNumber-1)*8); //开辟内存空间
	if (slope_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //计算slope和offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//计算查找表
	for (i=0; i<UX3328S_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //得到对应温度的slope和offset
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
		
		//对LUT计算值超出0和4095范围的值处理
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0xFF) 	LUT_out_arr[i] = 0xFF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//释放内存
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//根据ux3328的温度AD数组，查找对应的温度索引，并根据温度索引获取DAC值到ux3328的查找表数组
	for (i=0; i<UX3328S_AD_NUM; i++)
	{
		//获取AD数组对应的温度值
		temp=UX3328S_AD[i]-UX3328S_INDEXTEMP_COMP;
		//查找温度值在拟合数组中的索引
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328S_TEMP_MAX; j++)
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
	
	//写查找表
//	error = ux3328_select_table (inst_I2C, 6);
//	if (error) return -1;
	
	error = ux3328s_set_lut_ex (inst_I2C, ux3328_lut, 6);
	if (error) return -1;
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

//需要备份table3, table5, table6 =>  把table6备份到A2[128-187], table3备份到A0[128-187]  table5备份到A0[188-247]  
int ux3328s_set_backup_A2_Table (int inst_I2C, int table)   
{
	int 	error,i;
	INT8U 	reg[255];
	union uUX3328S localUX3328S;

	/**还有问题**Eric.Yao**/
	if (5 == table)
	{
		error = ux3328s_select_table (inst_I2C, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

		//写到A0[188~247]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128+60, 105, localUX3328S.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	if (6 == table)
	{
		error = ux3328s_select_table (inst_I2C, 6);
		if (error) return -1; 
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
		error = ux3328s_select_table (inst_I2C, 0);
		if (error) return -1;
		//写到A2[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 105, localUX3328S.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
		
		error = ux3328s_select_table (inst_I2C, 3);
		if (error) return -1;
	} 
	if(3 == table)
	{
		error = ux3328s_select_table (inst_I2C, 3);
		if (error) return -1;
		
		error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 128, 128, localUX3328S.pStr); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
		//写到A0[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128, 128, localUX3328S.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 	  
	}
	return 0;
}	 

int ux3328s_check_eepflag (int inst_I2C, BOOL flag)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	if (0xAA != localUX3328S.sStr.uTable.UX3328S_TABLE3.EEPROMIdenfier0)
	{
		if (!flag)
		{
			MessagePopup ("ERROR", "EEPROMIdenfier0 is not 0xAA !"); 
		}
		return -1;
	}
	
	if (0x55 != localUX3328S.sStr.uTable.UX3328S_TABLE3.EEPROMIdenfier1)
	{
		if (!flag)
		{
			MessagePopup ("ERROR", "EEPROMIdenfier1 is not 0x55 !"); 
		}
		return -1;
	}
	
/*	if (   (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.dsfail0) 
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.dsfail1)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.dsfail2)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.dsfail3)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.EEPROM_FAIL0)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.EEPROM_FAIL1)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.EEPROM_FAIL2)
		|| (0x00 != localUX3328S.sStr.uTable.UX3328S_TABLE3.INIT_Status.EEPROM_FAIL3))
	{
		if (!flag)
		{
			MessagePopup ("ERROR", "INIT_Status is not 0x00 !"); 
		}
		return -1;
	}
*/
	return 0; 
}

//将备份table3, table5, table6的数据重新写入A2 
int ux3328s_write_backup_A2_Table (int inst_I2C)   
{
	int 	error,i;
	INT8U 	rom_value[256]={0xFF}, lut_arr[128]={0xFF};	

	//写入初值
	error = ux3328s_set_default (inst_I2C);
	if (error) return -1; 
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA0, 0, 256, rom_value); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	//table3
	for (i=0; i<128; i++)
	{
		lut_arr[i] = rom_value[128+i];
	}
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	error = ux3328s_set_table3_backup(inst_I2C, lut_arr);
	if (error) return -1;
	///////////////////////////////////////////////////
	//table 5
	for (i=0; i<128; i++)
	{
		lut_arr[i] = rom_value[128+i];
	}
	error = ux3328s_select_table (inst_I2C, 5);
	if (error) return -1;
	
	error = ux3328s_set_lut (inst_I2C, lut_arr);
	if (error) return -1;
	///////////////////////////////////////////////////
	//table6
	error = ux3328s_select_table (inst_I2C, 0);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, rom_value); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	for (i=0; i<128; i++)
	{
		lut_arr[i] = rom_value[128+i];
	}
	error = ux3328s_select_table (inst_I2C, 6);
	if (error) return -1;
	
	error = ux3328s_set_lut (inst_I2C, lut_arr);
	if (error) return -1;
	////////////////////////////////////////////////////
	
	error = ux3328s_set_checksum_A2_Table (inst_I2C); 
	if (error) return -1;  
	
	return 0;
}

int ux3328s_set_table3_backup(int inst_I2C, INT8U table_data[])
{
	int error, i;
	union uUX3328S localUX3328S;
	INT8U Slave_Address;
	
	//处理地址存储器字节
	table_data[49]=table_data[49] & 0xFC;
	
	for (i=0; i<51; i++)
	{
		localUX3328S.pStr[128+i]=table_data[i];
	}		 
	
	//设置密码 
	for (i=0; i<4; i++)
	{localUX3328S.pStr[179+i]=0x00;}
	
	localUX3328S.pStr[183] = 0x53;
	localUX3328S.pStr[184] = 0x4F; 
	localUX3328S.pStr[185] = 0x45; 
	localUX3328S.pStr[186] = 0x42;
	
	//SECURITY_SEL
	localUX3328S.pStr[187] = table_data[59]; 
	
	//写128-187 
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 60, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	return 0;
}

int ux3328s_set_lut_ex (int inst_I2C, INT8U LUT[60], int table)
{
	int 	error, i;
	union uUX3328S localUX3328S;
	INT8U 	lut_arr[256];
	
	error = ux3328s_select_table (inst_I2C, table);
	if (error) return -1;
	
	for (i=0; i<105; i++)
	{localUX3328S.pStr[128+i]=LUT[i];}
	
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 60, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	//备份table3写到A0[179-238]
	if (5 == table)
	{
		for (i=0; i<105; i++)
		{
			lut_arr[128+60+i]=LUT[i];  
		}
		//写到A0[179~238]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA0, 128+60, 60, lut_arr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	//备份table6写到A2[128-187]
	if (6 == table)
	{
		error = ux3328s_select_table (inst_I2C, 0);
		if (error) return -1;
		//写到A2[128~187]
		error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 128, 60, localUX3328S.pStr, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	}
	
	return 0;
}

int ux3328s_check_A2flag(int inst_I2C, BOOL flag)
{
	int error=0;
	
	//录入密码
	error = ux3328s_ENTRY_PW2 (inst_I2C);
	if (error) return -1;
	 
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;

	//检查EEPROM标志位是否正常
	error = ux3328s_check_eepflag (inst_I2C, flag);
	if (error) return -1;
	
	return 0;
}

int ux3328s_set_table3_default (int inst_I2C)
{
	int error;
	union uUX3328S localUX3328S;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	memset (localUX3328S.pStr, 0, sizeof (localUX3328S.pStr));
	localUX3328S.pStr[0x80] = 0xAA;
	localUX3328S.pStr[0x81] = 0x1A;    
	localUX3328S.pStr[0x82] = 0x43;      
	localUX3328S.pStr[0x83] = 0xB2;      
	localUX3328S.pStr[0x84] = 0x50;      
	localUX3328S.pStr[0x85] = 0xC0;      
	localUX3328S.pStr[0x86] = 0x1A;      
	localUX3328S.pStr[0x87] = 0x37;      
	localUX3328S.pStr[0x88] = 0x48;      
	localUX3328S.pStr[0x89] = 0x65;      
	localUX3328S.pStr[0x8A] = 0x1F;      
	localUX3328S.pStr[0x8B] = 0x00;      
	localUX3328S.pStr[0x8C] = 0x00;      
	localUX3328S.pStr[0x8D] = 0x00;      
	localUX3328S.pStr[0x8E] = 0x00;      
	localUX3328S.pStr[0x8F] = 0x1C;      
	localUX3328S.pStr[0x90] = 0x00;
	localUX3328S.pStr[0x91] = 0x3E;    
	localUX3328S.pStr[0x92] = 0x00;      
	localUX3328S.pStr[0x93] = 0x01;      
	localUX3328S.pStr[0x94] = 0x28;      
	localUX3328S.pStr[0x95] = 0x00;      
	localUX3328S.pStr[0x96] = 0x00;      
	localUX3328S.pStr[0x97] = 0x00;      
	localUX3328S.pStr[0x98] = 0x04;      
	localUX3328S.pStr[0x99] = 0x04;      
	localUX3328S.pStr[0x9A] = 0x20;      
	localUX3328S.pStr[0x9B] = 0xC0;      
	localUX3328S.pStr[0x9C] = 0x00;      
	localUX3328S.pStr[0x9D] = 0x00;      
	localUX3328S.pStr[0x9E] = 0x8C;      
	localUX3328S.pStr[0x9F] = 0x2D;  
	localUX3328S.pStr[0xA0] = 0x00;
	localUX3328S.pStr[0xA1] = 0x00;    
	localUX3328S.pStr[0xA2] = 0x00;      
	localUX3328S.pStr[0xA3] = 0x00;      
	localUX3328S.pStr[0xA4] = 0x00;      
	localUX3328S.pStr[0xA5] = 0x01;      
	localUX3328S.pStr[0xA6] = 0xD7;      
	localUX3328S.pStr[0xA7] = 0x88;      
	localUX3328S.pStr[0xA8] = 0x8E;      
	localUX3328S.pStr[0xA9] = 0x01;      
	localUX3328S.pStr[0xAA] = 0xD7;      
	localUX3328S.pStr[0xAB] = 0x88;      
	localUX3328S.pStr[0xAC] = 0x8E;      
	localUX3328S.pStr[0xAD] = 0x01;      
	localUX3328S.pStr[0xAE] = 0x08;      
	localUX3328S.pStr[0xAF] = 0xF8;
	localUX3328S.pStr[0xB0] = 0x94;
	localUX3328S.pStr[0xB1] = 0x00;    
	localUX3328S.pStr[0xB2] = 0xD4;      
	localUX3328S.pStr[0xB3] = 0xFF;      
	localUX3328S.pStr[0xB4] = 0x12;      
	localUX3328S.pStr[0xB5] = 0x03;      
	localUX3328S.pStr[0xB6] = 0xA3;      
	localUX3328S.pStr[0xB7] = 0x00;      
	localUX3328S.pStr[0xB8] = 0x00;      
	localUX3328S.pStr[0xB9] = 0x00;      
	localUX3328S.pStr[0xBA] = 0x2D;      
	localUX3328S.pStr[0xBB] = 0x00;      
	localUX3328S.pStr[0xBC] = 0x00;      
	localUX3328S.pStr[0xBD] = 0x00;      
	localUX3328S.pStr[0xBE] = 0x00;      
	localUX3328S.pStr[0xBF] = 0x00;
	localUX3328S.pStr[0xC0] = 0x00;
	localUX3328S.pStr[0xC1] = 0x00;    
	localUX3328S.pStr[0xC2] = 0x00;      
	localUX3328S.pStr[0xC3] = 0x00;      
	localUX3328S.pStr[0xC4] = 0x00;      
	localUX3328S.pStr[0xC5] = 0x00;      
	localUX3328S.pStr[0xC6] = 0x00;      
	localUX3328S.pStr[0xC7] = 0x00;      
	localUX3328S.pStr[0xC8] = 0x00;      
	localUX3328S.pStr[0xC9] = 0xA2;      
	localUX3328S.pStr[0xCA] = 0x40;      
	localUX3328S.pStr[0xCB] = 0x00;      
	localUX3328S.pStr[0xCC] = 0x00;      
	localUX3328S.pStr[0xCD] = 0x00;      
	localUX3328S.pStr[0xCE] = 0x00;      
	localUX3328S.pStr[0xCF] = 0x00;
	localUX3328S.pStr[0xD0] = 0x53;
	localUX3328S.pStr[0xD1] = 0x4F;    
	localUX3328S.pStr[0xD2] = 0x45;      
	localUX3328S.pStr[0xD3] = 0x42;      
	localUX3328S.pStr[0xD4] = 0x00;      
	localUX3328S.pStr[0xD5] = 0x55;   

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	return 0;
}


int ux3328s_fit_APC_cClass (int inst_I2C, double Ibias10_33, double Ibias33_60, INT16U DAC_APC)
{
	double Ibias, Ibias33, Ibias10, Ibias60, Ibias82;
	int    error, Temper[4], i, j, fitNumber=3, Temper_out_arr[UX3328S_TEMP_MAX], LUT_out_arr[UX3328S_TEMP_MAX];
	float  temper, k, b;
	double slope, *slope_arr, *offset_arr;
	int    TemperLimit_H=150, TemperLimit_L=-35, temp, deltaMAX=0x7FFF, delta;
	INT16U  ux3328s_lut[UX3328S_AD_NUM], DAC[4];
	short int offset;
	INT8U   ux3328s_lut_to_table[105];
	int     index;
	const  float APC_SLOPE = 0.392;
	
	//计算当前Ibias电流
	Ibias=DAC_APC*APC_SLOPE;
	
	//计算当前温度
	error = ux3328s_get_temper (inst_I2C, &temper);
	if (error) 
	{
		return -1; 
	}
		
	//计算66°的Imod
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
		MessagePopup ("提示", "UX3328内核温度高于111度，超出了MOD拟合温度范围！   "); 
		return -1;
	} 

	//根据拟合参数计算10、33、60、82的Imod    //30  66  93  111 
	Ibias10=Ibias33-Ibias10_33*(66-30);
	Ibias60=Ibias33+Ibias33_60*(93-66);   
	
	//根据Imod计算对应温度下的DAC
	DAC[0]=Ibias10/APC_SLOPE;
	DAC[1]=Ibias33/APC_SLOPE;
	DAC[2]=Ibias60/APC_SLOPE;
	
	//检查DAC是否有溢出的情况 
	for (i=0; i<3; i++)
	{
		if (DAC[i]>0x3FF) 
		{
			MessagePopup ("ERROR", "计算出的DAC值溢出"); 
			return -1;
		} 
	}	  

	Temper[0]=30;
	Temper[1]=66;
	Temper[2]=93;

	//分段拟合-40~126的查找表
	slope_arr  = malloc ((fitNumber-1)*8); //开辟内存空间
	if (slope_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  
	
	offset_arr = malloc ((fitNumber-1)*8);
	if (offset_arr == NULL) {MessagePopup ("提示", "程序错误，需要退出！   "); return -1;}  	
  
	for (i=0; i<fitNumber-1; i++)  //计算slope和offset
	{
		slope_arr[i]  =(double)(DAC[i+1]-DAC[i])/(double)(Temper[i+1]-Temper[i]);
		offset_arr[i] =(double)(DAC[i+1]-slope_arr[i]*Temper[i+1]);
	}
	//计算查找表
	for (i=0; i<UX3328S_TEMP_MAX; i++)
	{
		temper = i-40;
		for (j=0; j<fitNumber-1; j++) //得到对应温度的slope和offset
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
		
		//对LUT计算值超出0和4095范围的值处理
		if ((temper*slope + offset)<0)        	LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>0x3FF) 	LUT_out_arr[i] = 0x3FF;
		else                                  	LUT_out_arr[i] = temper*slope + offset;
	}
	
	//释放内存
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 	
	
	//根据ux3328的温度AD数组，查找对应的温度索引，并根据温度索引获取DAC值到ux3328的查找表数组
	for (i=0; i<UX3328S_AD_NUM; i++)
	{
		//获取AD数组对应的温度值
		temp=UX3328S_AD[i]-UX3328S_INDEXTEMP_COMP;
		//查找温度值在拟合数组中的索引
		deltaMAX=0x7FFF;
		for (j=0; j<UX3328S_TEMP_MAX; j++)
		{
			delta = fabs(temp-Temper_out_arr[j]); 
		 	if (delta>deltaMAX) 
			{
				ux3328s_lut[i]=LUT_out_arr[j-1];	
				break;
			}
			deltaMAX=delta;	
		}
	}
	
	for (index = 0; index < UX3328S_AD_NUM/4; index++)
	{
		ux3328s_lut_to_table[index*5]   =                                          ux3328s_lut[index*4]   /4 ;
		ux3328s_lut_to_table[index*5+1] = ((ux3328s_lut[index*4]   & 0x03) * 64) + ux3328s_lut[index*4+1] /16;   
		ux3328s_lut_to_table[index*5+2] = ((ux3328s_lut[index*4+1] & 0x0F) * 16) + ux3328s_lut[index*4+2] / 64;
		ux3328s_lut_to_table[index*5+3] = ((ux3328s_lut[index*4+2] & 0x3F) * 4)  + ux3328s_lut[index*4+3] / 256;  
		ux3328s_lut_to_table[index*5+4] = ((ux3328s_lut[index*4+3] & 0xFF)); 
	}
	
	//写查找表
	error = ux3328s_set_lut_ex (inst_I2C, ux3328s_lut_to_table, 4);
	if (error) return -1;
	
	error = ux3328s_select_table (inst_I2C, 3);
	if (error) return -1;
	
	return 0;
}

//录入2级密码，并选A2 table3
int ux3328s_set_FactoryMode(int InstI2C)
{
	int error;
	
	error = ux3328s_ENTRY_PW2 (InstI2C);
	if (error) 
	{
		return -1;
	}
//	Delay(0.3);  
	error = ux3328s_select_table (InstI2C, 3);
	if (error) 
	{
		return -1;
	}
	
	return 0;
}

int ux3328s_set_UserMode(int InstI2C)
{
	int error;
	
	error = ux3328s_ENTRY_PW0(InstI2C);
	if (error) return -1;
	
	return 0;
}

int ux3328s_ENTRY_PW0(int inst_I2C)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, 0x7B, 0xFF, 0xFF, 0xFF, 0xFF, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_set_los_dac(int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328S localUX3328S;
	
	localUX3328S.sStr.uTable.UX3328S_TABLE3.RXAlarmDAC=val; 
	
	// reg_add=140
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RXAlarmDAC)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTEs_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, 1, localUX3328S.pStr, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}
int ux3328s_get_los_dac(int inst_I2C, int *val)
{
	int    error, reg_add;  
	union uUX3328S localUX3328S;
	INT8U  value;
	
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RXAlarmDAC)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTE_READ_DLL (inst_I2C, UX3328S_ADDR, reg_add, &value);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	*val = value;
	return 0;
}

int ux3328s_get_biasct_r_dac (int inst_I2C, INT16U *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val=localUX3328S.sStr.uTable.UX3328S_TABLE3.BIASCT_R*4 + localUX3328S.sStr.uTable.UX3328S_TABLE3.TX_STATUS1.BIASCT_R;
		
	return 0; 
}

int ux3328s_set_los_RX_CTRL2(int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328S localUX3328S;

	// reg_add=155
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.RX_CTRL2)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, val, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_get_laser_tempe (int inst_I2C, double *temp)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*temp=(localUX3328S.pStr[106]*256+localUX3328S.pStr[107])/256.;
		
	return 0; 
}

int ux3328s_get_Tec_Current (int inst_I2C, double *Current)
{
	int error;
	union uUX3328S localUX3328S;
	int val;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	val=localUX3328S.pStr[108]*256+localUX3328S.pStr[109];
	if(val>32767)
	{
		val=val-65536;	
	}
	*Current=val *0.1;
		
	return 0; 
}

int ux3328s_get_LDtempe_TecCurr (int inst_I2C,double *TECtemp, double *TECCurrent)
{
	int error;
	union uUX3328S localUX3328S;
	int val;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*TECtemp=(localUX3328S.pStr[106]*256+localUX3328S.pStr[107])/256.;  
	
	val=localUX3328S.pStr[108]*256+localUX3328S.pStr[109];
	if(val>32767)
	{
		val=val-65536;	
	}
	*TECCurrent=val *0.1;
		
	return 0; 
}

int ux3328s_set_TX_CTRL2(int inst_I2C, INT8U val)
{
	int error, reg_add;
	union uUX3328S localUX3328S;

	// reg_add=155
	reg_add = (int)(&localUX3328S.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL2)-(int)(&localUX3328S.sStr.tempHighAlarm);  
	error = I2C_BYTE_WRITE_DLL (inst_I2C, UX3328S_ADDR, reg_add, val, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int ux3328s_get_PID_LOOKFLAG (int inst_I2C, int *val)
{
	int error;
	union uUX3328S localUX3328S;
		
	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val= localUX3328S.pStr[119] & 0x04;
		
	return 0; 
}

int ux3328s_Get_TX_CTRL2 (int inst_I2C, int *val)
{
	int error;
	union uUX3328S localUX3328S;
	unsigned char tempval;
		
//	error = I2C_BYTEs_READ_DLL (inst_I2C, UX3328S_ADDR, 0, 256, localUX3328S.pStr); 
	error = I2C_BYTE_READ_DLL (inst_I2C, UX3328S_ADDR, 0x82,&tempval); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	
	*val= tempval;
		
	return 0; 
}
