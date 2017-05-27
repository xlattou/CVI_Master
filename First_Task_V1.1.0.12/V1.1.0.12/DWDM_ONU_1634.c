#include "supermaster.h" 
#include <userint.h>
#include <ansi_c.h>
#include "DWDM_ONU_DS4830.h"

//--------------------------------------------------------------------------------------------------------------------------------

int DWDM_ONU_DS4830_Enter_Password(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	
	error=SetCommand (handle, "I2C_write(0xA2,123,0x53,0x4F,0x45,0x42)",strOupt);//写密码
	if(error!=0) 
	{
		return -1; 
	}		 
	
	return 0;
}

int DWDM_ONU_DS4830_GET_FirmwareVer(int handle, char *FirmwareVer) 		//读取firmware版本
{
	int  error=0;
	char *tmp = 0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "MCU_GET_VERSION()",strOupt);
	if (error != 0) 
	{
		return -1; 
	}
	
	tmp = strOupt+9;
	
	strcpy(strOupt, tmp);
	
	strOupt[4] = 0;
	
	strcpy(FirmwareVer,strOupt);	
	
	return 0;	
}

//-----------------------------------------------------------------------------------------------------------------------------------------

int DWDM_ONU_DS4830_Set_APC_Auto(int handle) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,80,0x41)", strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;	 
}

int DWDM_ONU_DS4830_Set_APC_Manual(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,80,0x4D)", strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;   
}

int DWDM_ONU_DS4830_Set_APC(int handle, unsigned short APC_DAC)
{
	int error=0;							//已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt,"mcu_set_table(base,0,82,0x%X,0x%X)",(BYTE)(APC_DAC & 0xFF), (BYTE)((APC_DAC >> 8)& 0xFF));
	error=SetCommand (handle, strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Read_APC(int handle, unsigned char *APC_DAC)
{
	int error=0;
	int s32temp[2]={0};         						  //已改  
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,82,2)", strOupt);
	if(error!=0)
	{
		return -1;
	}
	sscanf(strOupt, "0x%x,0x%x", &s32temp[0], &s32temp[1]);			
	
	*APC_DAC=s32temp[0]+s32temp[1]*256;
	
	return 0; 	
}			

int DWDM_ONU_DS4830_Write_APC_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset)
{
	INT8U  lslop, mslop, loffset, moffset,ladc,madc;   
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	lslop = wSlop&0x00ff;
	mslop = wSlop>>8&0x00ff;
	loffset = wOffset&0x00ff;
	moffset = wOffset>>8&0x00ff;
	ladc = value&0x00ff;
	madc = value>>8&0x00ff;  
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	sprintf(strInpt, "MCU_SET_TABLE(LUK,0,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X)", address, ladc, madc, lslop, mslop, loffset, moffset);
	SetCommand(handle, strInpt, strOupt);    
	if(0 != strcmp(strOupt, "OK"))
	{
		MessagePopup ("ERROR", "输入命令出错!     "); 
		return -1;
	} 
	return 0;
}

int DWDM_ONU_DS4830_Set_MOD_Auto(int handle) 
{
	int error=0;							   //已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,0x22,0x41)", strOupt);	
	if(error!=0)			  
	{
		return -1;
	}
	
	return 0;   
}

int DWDM_ONU_DS4830_Set_MOD_Manual(int handle) 
{
	int error=0;							   //已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,0x22,0x4D)", strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}				

int DWDM_ONU_DS4830_Set_MOD(int handle, unsigned short MOD_set)  
{
	unsigned char u8mod[2]={0};
	int           error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	u8mod[1]= (unsigned char)(MOD_set>>8);				   //已改
	u8mod[0]= (unsigned char)(MOD_set); 
	
	sprintf(strInpt,"MCU_I2C_WRITE(0xA2,0xAB,0x%x,0x%x)",u8mod[1], u8mod[0]);
	error=SetCommand (handle, strInpt, strOupt);				
	if (error!=0) 
	{	
		return -1; 
	}
	
	return 0;  	
}

int DWDM_ONU_DS4830_Read_MOD(int handle, unsigned short* MOD_read) 
{					
	int           u8mod[2]={0};
	int           error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt,"MCU_I2C_READ(0xA2,0xAB,2)");		  //已改
	error=SetCommand (handle, strInpt, strOupt);				
	if (error!=0) 
	{	
		return -1; 
	}
	sscanf(strOupt, "0x%x,0x%x", &u8mod[1],&u8mod[0]); 
	
	*MOD_read=((unsigned short)u8mod[0]<<8)+u8mod[1];
	
	return 0;  	
}

int DWDM_ONU_DS4830_Write_MOD_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset)
{
	INT8U  lslop, mslop, loffset, moffset,ladc,madc;   
	
	char strInpt[256];
	char strOupt[256];
																   //已改
	lslop = wSlop&0x00ff;
	mslop = wSlop>>8&0x00ff;
	loffset = wOffset&0x00ff;
	moffset = wOffset>>8&0x00ff;
	ladc = value&0x00ff;
	madc = value>>8&0x00ff;  
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	sprintf(strInpt, "MCU_SET_TABLE(LUK,2,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X)", address, ladc, madc, lslop, mslop, loffset, moffset);
	SetCommand(handle, strInpt, strOupt);    
	if(0 != strcmp(strOupt, "OK"))
	{
		MessagePopup ("ERROR", "输入命令出错!     "); 
		return -1;
	} 
	return 0;
}

int DWDM_ONU_DS4830_Read_MOD_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset)
{
	int  lslop, mslop, loffset, moffset, ladc, madc;   				  //已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	sprintf(strInpt, "MCU_GET_TABLE(LUK,2,0x%02X, 6)", address);
	SetCommand(handle, strInpt, strOupt);    
	sscanf(strOupt, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,", &ladc, &madc, &lslop, &mslop, &loffset, &moffset);     
	
	*value = ladc | (madc<<8);
	*wSlop = lslop | (mslop<<8);
	*wOffset = loffset | (moffset<<8);
	
	return 0;
}

int DWDM_ONU_DS4830_Read_Txpower_ADC(int handle, unsigned short* ADC)
{
	int error=0;
	int s32temp[2]={0};										//已改   
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	SetCommand (handle, "MCU_I2C_READ(0xA2,0xEE,2)", strOupt);
	sscanf(strOupt, "0x%x,0x%x", &s32temp[0], &s32temp[1]);			
	
	*ADC=s32temp[0]*256+s32temp[1];
	
	return 0; 	
}

int DWDM_ONU_DS4830_Write_Txpower_Cal(int handle, unsigned short* ADC,double* AOP_in, float Tx_unit) 
{
	double slope=0.0;
	double offset=0.0;
	
	INT16U wSlope=0;
	INT16U wOffset=0; 

	double AOP[2]={0};	
	
	int   error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	AOP[0]=pow(10,AOP_in[0]/10.)/ (Tx_unit*1E-3);
	AOP[1]=pow(10,AOP_in[1]/10.)/ (Tx_unit*1E-3);
	 
	slope = (AOP[1]-AOP[0]) * 128 / (ADC[1]-ADC[0]);
	offset= AOP[0]-slope / 128 * ADC[0];
	
	wSlope = slope;
	wOffset = offset;
	
	sprintf(strInpt, "mcu_set_table(base,0,0x16,0x02,0x00,0x%x,0x%x,0x%x,0x%x)", (BYTE)(wSlope & 0xFF), (BYTE)((wSlope >> 8) & 0xFF), (BYTE)(wOffset & 0xFF), (BYTE)((wOffset >> 8) & 0xFF));
	error=SetCommand (handle, strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}

	return 0; 	
}

//--------------------------------------------------------------------------------------------------------------------------------

int DWDM_ONU_DS4830_Update_Base0(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(BASE,0)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 		
}

int DWDM_ONU_DS4830_Update_Base1(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(BASE,1)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Update_Base2(int handle)  
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(BASE,2)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Update_Base3(int handle)    
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(BASE,3)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Update_LUK0(int handle)  
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,0)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Update_LUK1(int handle)   
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,1)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Update_LUK2(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,2)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Update_LUK3(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,3)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Update_LUK4(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,4)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Update_LUK5(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,5)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Update_LUK6(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(LUK,6)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Update_DRIVER1(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(DRIVER,1)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0;   
}

int DWDM_ONU_DS4830_Update_DRIVER0(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand(handle, "MCU_UPDATE_FLASH(DRIVER,0)",strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0;   
}

int DWDM_ONU_DS4830_Set_APD_Auto(int handle)
{
	int mode;
	int  data0, data1, data2, data3, error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "MCU_I2C_READ(0XA2,0XBD,1)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	
	sscanf(strOupt, "0x%x", &mode);
	
	mode = mode | 0x01;	
	
	sprintf(strInpt,"MCU_I2C_WRITE(0XA2,0XBD,0X%X)",mode);
	error=SetCommand (handle, strInpt, strOupt);
	if (error != 0) 
	{	
		return -1;
	}
	
	return 0;
}	

int DWDM_ONU_DS4830_Set_APD_Manual(int handle)
{
	int mode;
	int  data0, data1, data2, data3, error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "MCU_I2C_READ(0XA2,0XBD,1)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	
	sscanf(strOupt, "0x%x", &mode);
	
	mode = mode & 0xFE;	
	
	sprintf(strInpt,"MCU_I2C_WRITE(0XA2,0XBD,0X%X)",mode);
	error=SetCommand (handle, strInpt, strOupt);
	if (error != 0) 
	{	
		return -1;
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Set_APD_DAC(int handle, int add, unsigned char APD_DAC)
{
	int mode;
	int  data0, data1, data2, data3, error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt,"MCU_I2C_WRITE(0XA2,0X%X,0X%X)",add, APD_DAC);
	error=SetCommand (handle, strInpt, strOupt);
	if (error != 0) 
	{	
		return -1;
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Set_APD(int handle, unsigned char APD_set) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt,"MCU_I2C_WRITE(0XA2,0XBE,0X%X)",APD_set);
	error=SetCommand (handle, strInpt, strOupt);
	if (error != 0) 
	{	
		return -1;
	}
	
	return 0; 	
}
	
int DWDM_ONU_DS4830_Read_APD(int handle, unsigned short* APD_set) 
{
	int s32temp[2]={0};     
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_get_table(base,0,0x24,2)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	sscanf(strOupt, "0x%x,0x%x", &s32temp[0], &s32temp[1]);			
	
	*APD_set=s32temp[0]+s32temp[1]*256;

	return 0; 	
}	 

int DWDM_ONU_DS4830_Write_APD_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset)  
{
 	short          sSlop[2];
	short          sOffset[2]; 
	int            error=0;
	INT8U  lslop, mslop, loffset, moffset,ladc,madc;  
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	lslop = wSlop&0x00ff;
	mslop = wSlop>>8&0x00ff;
	loffset = wOffset&0x00ff;
	moffset = wOffset>>8&0x00ff;
	ladc = value&0x00ff;
	madc = value>>8&0x00ff;  
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	sprintf(strInpt, "MCU_SET_TABLE(LUK,2,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X)", address, ladc, madc, lslop, mslop, loffset, moffset);
	SetCommand(handle, strInpt, strOupt);    
	if(0 != strcmp(strOupt, "OK"))
	{
		MessagePopup ("ERROR", "输入命令出错!     "); 
		return -1;
	} 

	return 0;   
}

int DWDM_ONU_DS4830_Read_APD_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset)
{
	int  lslop, mslop, loffset, moffset, ladc, madc;   
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	sprintf(strInpt, "MCU_GET_TABLE(LUK,2,0x%02X, 6)", address);
	SetCommand(handle, strInpt, strOupt);    
	sscanf(strOupt, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,", &ladc, &madc, &lslop, &mslop, &loffset, &moffset);     
	
	*value = ladc | (madc<<8);
	*wSlop = lslop | (mslop<<8);
	*wOffset = loffset | (moffset<<8);
	
	return 0;
}

int DWDM_ONU_DS4830_Set_LOS_Auto(int handle)
{
	int mode;
	int  data0, data1, data2, data3, error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "MCU_SET_TABLE(base,0, 48, 0x41)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	
	return 0;
}	

int DWDM_ONU_DS4830_Set_LOS_Manual(int handle)
{
	int mode;
	int  data0, data1, data2, data3, error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "MCU_SET_TABLE(base,0, 48, 0x4D)", strOupt);                 
	if(error!=0) 
	{	
		return -1; 
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Set_LOS(int handle, unsigned short LOS)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(base, 0, 50, 0x%x, 0x%x)", LOS&0x00ff, LOS>>8); 
	SetCommand (handle, strInpt, strOupt);
	if (error != 0) 
	{	
		return -1; 
	}			

	return 0; 	
}

int DWDM_ONU_DS4830_Read_LOS(int handle, unsigned short *LOS)
{
	int error=0;
	int s32temp[2]={0};    
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	SetCommand (handle, "MCU_GET_TABLE(base, 0, 50, 2)", strOupt);
	sscanf(strOupt, "0x%x,0x%x", &s32temp[0], &s32temp[1]);			
    
	
	*LOS = s32temp[0]+s32temp[1]*256;
	
	return 0; 	
}

int DWDM_ONU_DS4830_Read_Rx_ADC(int handle, unsigned short* ADC)
{
	int error=0;
	int s32temp[2]={0};    
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	SetCommand (handle, "MCU_I2C_READ(0xA2,0xF0,2)", strOupt);
	sscanf(strOupt, "0x%x,0x%x", &s32temp[0], &s32temp[1]);			
	*ADC=s32temp[0]*256+s32temp[1];      
	
	return 0; 	
}

int DWDM_ONU_DS4830_Write_Rx_Cal(int handle, unsigned short* ADC_Array,float* PWR_Array_P,int RxCaliNo)
{
	unsigned short wSlop;
	unsigned short wOffset;
	short          sSlop;
	short          sOffset;   
	unsigned char  lslop;
	unsigned char  mslop;
	unsigned char  loffset;
	unsigned char  moffset;
	unsigned char  ladc;
	unsigned char  madc; 
	char           strTemp[256];
	int            i=0;
	int            j=0;
	float   	   PWR_Array[30]={0.0};
	
	char strInpt[256];
	char strOupt[256];
	
	for(i=0;i<RxCaliNo;i++)
	{	
		PWR_Array[i]=pow(10,PWR_Array_P[i]/10.)/0.1E-3;
	}
			
	j=0;			
	for (i=1; i<RxCaliNo; i++)
	{
		sSlop = ((PWR_Array[i] - PWR_Array[i-1]) * 128. / (ADC_Array[i] - ADC_Array[i-1])); 
	    sOffset = (PWR_Array[i] - sSlop * ADC_Array[i] / 128.) * 1;
				
		wSlop = sSlop;
		wOffset = sOffset;
				
		lslop = wSlop & 0x00ff;
		mslop = (wSlop>>8) & 0x00ff;
		loffset = wOffset & 0x00ff;
		moffset = (wOffset>>8) & 0x00ff;
		ladc = ADC_Array[i] & 0x00ff;
		madc = (ADC_Array[i]>>8) & 0x00ff;  
				
		memset(strTemp, 0, 256); 
		memset(strInpt, 0, 256);
		memset(strOupt, 0, 256); 
			
		strcpy(strInpt, "MCU_SET_TABLE(BASE,1,");
		sprintf(strTemp,"0x%02X", j);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ",");   
		sprintf(strTemp,"0x%02X", ladc);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ",");   
		sprintf(strTemp,"0x%02X", madc);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ",");   
		sprintf(strTemp,"0x%02X", lslop);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ","); 
		sprintf(strTemp,"0x%02X", mslop);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ",");   
		sprintf(strTemp,"0x%02X", loffset);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ",");   
		sprintf(strTemp,"0x%02X", moffset);     
		strcat(strInpt, strTemp);
		strcat(strInpt, ")");
		SetCommand(handle, strInpt, strOupt);  
		j=j+6;
	}
			
	j = j-6;
	memset(strTemp, 0, 256); 
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
		
	strcpy(strInpt, "MCU_SET_TABLE(BASE,1,");
	sprintf(strTemp,"0x%02X", j);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ",");   
	sprintf(strTemp,"0x%02X", 0xff);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ",");  
	sprintf(strTemp,"0x%02X", 0xff);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ","); 
	sprintf(strTemp,"0x%02X", lslop);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ","); 
	sprintf(strTemp,"0x%02X", mslop);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ",");   
	sprintf(strTemp,"0x%02X", loffset);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ",");   
	sprintf(strTemp,"0x%02X", moffset);     
	strcat(strInpt, strTemp);
	strcat(strInpt, ")");
	SetCommand(handle, strInpt, strOupt);  
			
	strcpy(strInpt, "MCU_SET_MON_INFO(RXPOWER,T,0x0000,0x0000)"); 	
	SetCommand(handle, strInpt, strOupt);   
	
	if(DWDM_ONU_DS4830_Update_Base0(handle)<0)
	{
		return -1;   
	}

	return 0; 	
}

//-------------------------------------------------------------------------------------------------------------------
int DWDM_ONU_DS4830_Set_TemperMode(int handle, int mode)	   //02为公式模式，00为外部模式 
{
	int  error=0;  
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "mcu_set_table(base,0,0x04,0x%02x)", mode); 
	error = SetCommand(handle, strInpt, strOupt);  					   
	if(error!=0)
	{
		return -1;
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(handle)<0)
	{
		return -1; 
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Get_Temper(int handle, double* temper)
{
	int            error=0;
	int            u16temp=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "SFP_GET_MONITOR(temperature)", strOupt);
	if (error != 0) 
	{
		return -1; 
	}
	sscanf(strOupt, "0x%x", &u16temp);
			
	*temper=((short)u16temp)/256.0;	
	
	return 0;   
}

int DWDM_ONU_DS4830_SET_Temp_Cal(int handle, INT16U wSlop, INT16U wOffset)
{
	int   error=0;   
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	
	sprintf(strInpt, "mcu_set_table(base,0,0x04,0x02,0x00,0x%x,0x%x,0x%x,0x%x)", (BYTE)(wSlop & 0xFF), (BYTE)((wSlop >> 8) & 0xFF), (BYTE)(wOffset & 0xFF), (BYTE)((wOffset >> 8) & 0xFF));
	error=SetCommand (handle, strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Monitor_RxPower(int handle, double* RxPower)
{
	int            error=0;
	int            u16temp=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "SFP_GET_MONITOR(RXPOWER)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	sscanf(strOupt,"0x%x",&u16temp);  
	
	if(u16temp==0)
	{
		*RxPower=-100;
	}
	else
	{
		*RxPower=10*log10(u16temp*0.1E-3);
	}
	
	return 0; 	 
}

int DWDM_ONU_DS4830_Monitor_TxPower(int handle, double* TxPower)
{
	int            error=0;
	int            u16temp=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "SFP_GET_MONITOR(TXPOWER)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	sscanf(strOupt,"0x%x",&u16temp);  
	
	if(u16temp==0)
	{
		*TxPower=-100;
	}
	else
	{
		*TxPower=10*log10(u16temp*0.1E-3);
	}
	
	return 0; 	
}

int DWDM_ONU_DS4830_Monitor_IBias(int handle, double* IBias, float Bias_unit)
{
	int            error=0;
	int            u16temp=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "SFP_GET_MONITOR(txbias)", strOupt);
	if(error!=0) 
	{	
		return -1; 
	}
	sscanf(strOupt,"0x%x",&u16temp);  
	
 	*IBias=u16temp*(Bias_unit*1.E-3);
	
	return 0; 		
}

int DWDM_ONU_DS4830_Set_CaseTemper_Cal(int handle, double sSlop,double sOffset)
{
	int            error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(base,3,0,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)",0xff,0,(BYTE)(((short)(sSlop*30))&0xFF), (BYTE)((((short)(sSlop*30))>>8) & 0xFF),(BYTE)(((short)sOffset) & 0xFF),(BYTE)((((short)sOffset) >> 8) & 0xFF));
	error=SetCommand (handle, strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}

	error = SetCommand (handle, "MCU_SET_MON_INFO(temperature, T, 1, 0)", strOupt);        
	if(error!=0)
	{
		return -1;
	}
	
	return 0;   
}  

int DWDM_ONU_DS4830_Save_Reg(int handle)
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error = SetCommand (handle, "DRV_SAVE_REG()", strOupt); 
	if (error<0) 
	{
		return -1; //MCU access error! 
	}
	
	return 0;   
}   

int DWDM_ONU_DS4830_Set_APD_Protect(int handle, unsigned short APD_Protect) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(BASE,0,0x26,0x%x,0x%x)", (BYTE)(APD_Protect & 0xFF), (BYTE)((APD_Protect >> 8)& 0xFF)); 
	
	error=SetCommand (handle, strInpt, strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;	 
}

int DWDM_ONU_DS4830_Set_APD_Protect2(int handle, unsigned short APD_Protect) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(BASE,0,0x28,0x%x,0x%x)", (BYTE)(APD_Protect & 0xFF), (BYTE)((APD_Protect >> 8)& 0xFF));
	
	error=SetCommand (handle, strInpt, strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;	 
} 

int DWDM_ONU_DS4830_Read_APC_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset)
{
	int  lslop, mslop, loffset, moffset, ladc, madc;   
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);

	sprintf(strInpt, "MCU_GET_TABLE(LUK,0,0x%02X, 6)", address);
	SetCommand(handle, strInpt, strOupt);    
	sscanf(strOupt, "0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,", &ladc, &madc, &lslop, &mslop, &loffset, &moffset);     
	
	*value = ladc | (madc<<8);
	*wSlop = lslop | (mslop<<8);
	*wOffset = loffset | (moffset<<8);
	
	return 0;
}			  

int DWDM_ONU_DS4830_Set_Tx_Soft_Disable(int handle, int disable_flag)  
{
	int         error=0;
	BYTE 		control = 0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	strcpy(strInpt,"I2C_READ(0xA2, 110, 1)");
	error=SetCommand (handle, strInpt, strOupt);				
	if (error!=0) 
	{	
		return -1; 
	}
	
	sscanf(strOupt, "0x%c", &control); 
	
	if (disable_flag)
	{
		control = control | 0x40;   //0x40 = 01000000		  bit6 软件disable标志位
	}
	else
	{
		control = control & 0xBF;   //0xBF = 10111111
	}
	
	sprintf(strInpt,"I2C_WRITE(0xA2,110,%d)",control);
	error=SetCommand (handle, strInpt, strOupt);				
	if (error!=0) 
	{	
		return -1; 
	}
	
	return 0;  	
}	  

int DWDM_ONU_DS4830_Set_Burst(int handle, BOOL BURST_FLAG)  
{
	//设置突发发光，BURST_FLAG = TRUE 突发发光，BURST_FLAG = FALSE 连续发光
	int		error=0;
	BYTE 	control = 0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	if (BURST_FLAG)
	{
		strcpy(strInpt,"MCU_I2C_WRITE(0XA2,0XA1,0X00)");
		error=SetCommand (handle, strInpt, strOupt);				
		if (error!=0) 
		{	
			return -1; 
		}
		
		strcpy(strInpt,"MCU_I2C_WRITE(0xA2,0XB7,0X58)");
		error=SetCommand (handle, strInpt, strOupt);				
		if (error!=0) 
		{	
			return -1; 
		}
	}
	else
	{
		strcpy(strInpt,"MCU_I2C_WRITE(0XA2,0XA1,0X20)");
		error=SetCommand (handle, strInpt, strOupt);				
		if (error!=0) 
		{	
			return -1; 
		}
		
		strcpy(strInpt,"MCU_I2C_WRITE(0xA2,0XB7,0X48)");
		error=SetCommand (handle, strInpt, strOupt);				
		if (error!=0) 
		{	
			return -1; 
		}
	}
	
	strcpy(strInpt,"mcu_update_flash(driver,0)");
	error=SetCommand (handle, strInpt, strOupt);				
	if (error!=0) 
	{	
		return -1; 
	}	 
	
	return 0;  	
}

int DWDM_ONU_DS4830_Set_TEC_Auto(int handle) 
{
	int error=0;							   //已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,104,0x41)", strOupt);	
	if(error!=0)			  
	{
		return -1;
	}
	
	return 0;   
}

int DWDM_ONU_DS4830_Set_TEC_Manual(int handle) 
{
	int error=0;							   //已改
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	error=SetCommand (handle, "mcu_set_table(base,0,104,0x4D)", strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0; 
}

int DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(int handle, int DAC)
{
	char mode;
	int  step, old_dac, DACMax, DACMin; 
	
	char strInpt[256];																			 
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);

	SetCommand (handle, "MCU_GET_ADJUST(7)", strOupt);
	sscanf(strOupt, "%c,0x%x,0x%x,0x%x,,0x%x", &mode, &step, &old_dac, &DACMax,&DACMin);			

	sprintf(strInpt, "MCU_SET_ADJUST(7,M,0x%02X,0x%03X,0x%03X,0x%03X)", step, DAC, DACMax, DACMin);       
	SetCommand(handle, strInpt, strOupt);    
	if(0 != strcmp(strOupt, "OK"))
	{
		MessagePopup ("ERROR", "输入命令出错!"); 
		return -1;
	}
	return 0; 
}

int DWDM_ONU_DS4830_Write_TEC_Luk(int handle, int channelindex, BOOL HIGH_FLAG, unsigned short* DAC, double* Temper) 
{
	short          sSlop[6];
	short          sOffset[6]; 
	double         TempTemper;
	int            error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	int  LUTIndex;
	int  straddress;
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);

	sSlop[0]=(DAC[1]-DAC[0])*30./(Temper[1]-Temper[0]);
	sOffset[0]=(DAC[0]-sSlop[0]*(Temper[0]+40.)/30.)*1;
	
	if (HIGH_FLAG)
	{
		straddress = 6;	
		TempTemper = 0xFF;
	}
	else
	{
		straddress = 0;
		TempTemper = Temper[1]+50;
	}
	
	LUTIndex = channelindex%4+2;
			

	sprintf(strInpt,"MCU_SET_TABLE(LUK,%d,%d,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)",    \
																		 LUTIndex, straddress,\
																		 (((BYTE)(TempTemper))& 0xFF),00,\
																		 (BYTE)((sSlop[0]) & 0xFF), (BYTE)((sSlop[0] >> 8) & 0xFF), \
																		 (BYTE)((sOffset[0]) & 0xFF), (BYTE)((sOffset[0] >> 8) & 0xFF));
	error=SetCommand (handle, strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
				
	return 0;
}

int DWDM_ONU_DS4830_Write_MOD_LUK6(int handle, int channelindex, unsigned short MOD_DAC) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(LUK, 6, 0x%x, 0x%x, 0x%x)", channelindex*2+2, (BYTE)(MOD_DAC & 0xFF), (BYTE)((MOD_DAC >> 8)& 0xFF));
	
	error=SetCommand (handle, strInpt, strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;
}

int DWDM_ONU_DS4830_Write_APC_LUK6(int handle, int channelindex, unsigned short APC_DAC) 
{
	int error=0;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256);
	
	sprintf(strInpt, "MCU_SET_TABLE(LUK, 6, 0x%x, 0x%x, 0x%x)", channelindex*2, (BYTE)(APC_DAC & 0xFF), (BYTE)((APC_DAC >> 8)& 0xFF));
	
	error=SetCommand (handle, strInpt, strOupt);	
	if(error!=0)
	{
		return -1;
	}
	
	return 0;
}
