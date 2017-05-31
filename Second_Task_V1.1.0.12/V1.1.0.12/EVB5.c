#include "EVB5.h"
#include <ansi_c.h>
#include <utility.h>
#include <userint.h>

unsigned char sBERT_TIMER_SET[5];  
unsigned char EVB5_Version;   

// for tiny13 flash
int MAX_BYTENUMBER_EEPROM = 64; 	 	//最多处理64 byte， Limited by TINY13 
int MAX_PAGENUMBER = 1024/(16*2);		//1k byte 除以 每页包含的Byte个数 所得的最大页数 =32  
int MAX_ONEPAGEDATANUMBER = 16*2;	//每页包含的Byte个数 =16*2   

int EVB5_Init (int *EVB5Handle, char *SN)
{
	int  error, iIndex;
	int  lpdwNumDevices; 
	char myUSBSN[30];    
	int	 I2Cslave_found;
	int  reg_add;
	int  SPI_Rate, SCK_PHASE, SCK_POLARITY;  
	union uF0_EVB5 local_evb5; 
	
	EVB5_Addr=0x84;
	SBERT_Addr=0x80;
	
	I2Cslave_found =0;
	
	//set I2C rate
	SetF320I2CRate_DLL(0);//F320 I2C速率; 0=50KHz, 1=100KHz 
	
	error = GetF320Numbers(&lpdwNumDevices); 
	if(error==0 && lpdwNumDevices>0)
	{
		for (iIndex=0x800; iIndex<lpdwNumDevices+0x800; iIndex++) 
		{
			error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
			if (error!=-1) //USB host found
			{   
				*EVB5Handle = iIndex;
				
				error = I2C_SLAVE_SEARCH_DLL(iIndex, EVB5_Addr);  
				if (error==0) //I2C EVB found  
				{
					//read F320 version
					error = GET_Version_F320_DLL(*EVB5Handle-0x800, &EVB5_Version);
						
					//read F320 SN
					memset (myUSBSN, 0, sizeof(myUSBSN));
					error = GetF320SerialNumber(*EVB5Handle-0x800, myUSBSN);
					
					//read eeprom information from EVB_addr
					error = I2C_BYTEs_READ_DLL (*EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
					
					//compare SN
					error = strcmp (local_evb5.sStrF0.vendorSN, myUSBSN);
					if (error && (*EVB5Handle-0x800)>=(lpdwNumDevices-1)) {MessagePopup("提示","F320的序列号和EVB5的序列号不一致");return -1;}
					
					error = strcmp (local_evb5.sStrF0.vendorSN, SN);
					if (error && (*EVB5Handle-0x800)>=(lpdwNumDevices-1)) {MessagePopup("提示","选用的EVB5序列号和EVB5实际序列号不一致");return -1;}
					
					if (error==0) {I2Cslave_found=1;break;}
				}
			}
		}
	}
	
	if (I2Cslave_found==0) 
	{
		return -1;
	}
	
	error = (local_evb5.sStrF0.Core_Voltage/10000.0>3.6 || local_evb5.sStrF0.Core_Voltage/10000.0<3.0); 
	if (error<0) {MessagePopup("提示","EVB5电压检测失败！");return -1;}
	
	//Set Password
	//error = I2C_4BYTEs_WRITE_DLL (EVB5Handle, EVB_addr, 123, 'J', 'a', 'c', 'k', 0.1);//写0级密码
	local_evb5.sStrF0.Password = 0x6B63614A; //"Jack" 
	reg_add =(int)(&local_evb5.sStrF0.Password) - (int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0, 0.1);			

	//table0
	local_evb5.sStrF0.LUTIndex=0;
	reg_add = (int)(&local_evb5.sStrF0.LUTIndex)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);			

	//Set mode
	local_evb5.sStrF0.FirstByte = 0x00; //auto 
	reg_add =(int)(&local_evb5.sStrF0.FirstByte) - (int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	
	if (EVB5_Version<7)//老版本，可能是EVB5B 
	{
		;
	}
	else
	{
		SPI_Rate = 119;   //速率，计算公式SCK = 12000/( ioArray[0]+1)K，如ioArray[0] = 119，则SPI速率为100K
		SCK_PHASE = 0;    //相位，0：在SCK周期的第一个边沿采样数据，1：在SCK周期的第二个边沿采样数据
		SCK_POLARITY = 0x00; //极性，0：SCK在空闲状态时处于低电平，1：SCK在空闲状态时处于高电平
		                     //bit7自动设置CS：为1就自动设置CS，在发起SPI操作时自动置低，结束后自动置高
		error = F320_SPI_Init_DLL(*EVB5Handle-0x800, SPI_Rate, SCK_PHASE, SCK_POLARITY);		
	}
	
	//保持OSA端开电，否则评估版工作异常
	error = EVB5_SET_SHDN_VOSA (*EVB5Handle, 1);
	if (error) return -1;
	
	return 0;
}

int EVB5_Init_Ex (int *EVB5Handle)
{
	int  error, iIndex;
	int  lpdwNumDevices; 
	char myUSBSN[30];    
	int	 I2Cslave_found;
	int  reg_add, Version; 
	int  SPI_Rate, SCK_PHASE, SCK_POLARITY;  
	union uF0_EVB5 local_evb5; 
	
	EVB5_Addr=0x84;
	SBERT_Addr=0x80;
	
	I2Cslave_found =0;
	
	//set I2C rate
	SetF320I2CRate_DLL(0);//F320 I2C速率; 0=50KHz, 1=100KHz 
	
	error = GetF320Numbers(&lpdwNumDevices); 
	if(error==0 && lpdwNumDevices>0)
	{
		for (iIndex=0x800; iIndex<lpdwNumDevices+0x800; iIndex++) 
		{
			error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
			if (error!=-1) //USB host found
			{   
				*EVB5Handle = iIndex;
				
				error = I2C_SLAVE_SEARCH_DLL(iIndex, EVB5_Addr);  
				if (error==0) //I2C EVB found  
				{
					//read F320 version
					error = GET_Version_F320_DLL(*EVB5Handle-0x800, &EVB5_Version);
					
					//read F320 SN
					memset (myUSBSN, 0, sizeof(myUSBSN));
					error = GetF320SerialNumber(*EVB5Handle-0x800, myUSBSN);
					
					//read eeprom information from EVB_addr
					error = I2C_BYTEs_READ_DLL (*EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
					
					//compare SN
					error = strcmp (local_evb5.sStrF0.vendorSN, myUSBSN);
					if (error) {MessagePopup("提示","F320的序列号和EVB5的序列号不一致");return -1;}
					
					I2Cslave_found=1;
					
					break;
				}
			}
		}
	}
	
	if (I2Cslave_found==0) 
	{
		return -1;
	}
	
	error = (local_evb5.sStrF0.Core_Voltage/10000.0>3.6 || local_evb5.sStrF0.Core_Voltage/10000.0<3.0); 
	if (error<0) {MessagePopup("提示","EVB5电压检测失败！");return -1;}
	
	//Set Password
	//error = I2C_4BYTEs_WRITE_DLL (EVB5Handle, EVB_addr, 123, 'J', 'a', 'c', 'k', 0.1);//写0级密码
	local_evb5.sStrF0.Password = 0x6B63614A; //"Jack" 
	reg_add =(int)(&local_evb5.sStrF0.Password) - (int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0, 0.1);			

	//table0
	local_evb5.sStrF0.LUTIndex=0;
	reg_add = (int)(&local_evb5.sStrF0.LUTIndex)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);			

	//Set mode
	local_evb5.sStrF0.FirstByte = 0x00; //auto 
	reg_add =(int)(&local_evb5.sStrF0.FirstByte) - (int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (*EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	
	if (EVB5_Version<7)//老版本，可能是EVB5B 
	{
		;
	}
	else
	{
		SPI_Rate = 119;   //速率，计算公式SCK = 12000/( ioArray[0]+1)K，如ioArray[0] = 119，则SPI速率为100K
		SCK_PHASE = 0;    //相位，0：在SCK周期的第一个边沿采样数据，1：在SCK周期的第二个边沿采样数据
		SCK_POLARITY = 0x00; //极性，0：SCK在空闲状态时处于低电平，1：SCK在空闲状态时处于高电平
		                     //bit7自动设置CS：为1就自动设置CS，在发起SPI操作时自动置低，结束后自动置高
		error = F320_SPI_Init_DLL(*EVB5Handle-0x800, SPI_Rate, SCK_PHASE, SCK_POLARITY);		
	}
	
	//保持OSA端开电，否则评估版工作异常
	error = EVB5_SET_SHDN_VOSA (*EVB5Handle, 1);
	if (error) return -1;
	
	error = EVB5_READ_FirmwareVersion (*EVB5Handle, &Version);
	if (error) return -1;
	
	if (Version<36) {MessagePopup("提示","EVB5固件版本已升级到V3.6，请立即更新");} 
	
	return 0;
}

int EVB5_SBERT_Init (int EVB5Handle, int sBERT_Rate, int sBERT_PRBS)
//return -1 when USB error, otherwise return 0  
//sBERT_RATE=0 means 1.244G, otherwise 2.488G
//sBERT_PRBS=0 means PRBS7,  otherwise PRBS23
{int error, i;
 char str[256], str1[256];
 int reg_add, reg_dat;
 double T_wait=0.001;
 int I2Chost_found = 0;
 int I2Cslave_found = 0;
 int iIndex;

	error = I2C_SLAVE_SEARCH_DLL(EVB5Handle, SBERT_Addr);
	if (error) 
	{	return -1; 
	}
	else
	{	error = I2C_BYTEs_READ_DLL (EVB5Handle, SBERT_Addr, 0, 128, sBERT.pStr80); 
		if (error) 
		{	return -1;
		}
	}

	sBERT.sStr80.RESET = 0x80;
	reg_add = (int)(&sBERT.sStr80.RESET)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait); 
	if (error) 
	{	return -1;
	}
	Delay(0.1);

	sBERT.sStr80.MODE_CTRL_0 = 0xC8; 
	reg_add = (int)(&sBERT.sStr80.MODE_CTRL_0)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.RX_DATA_CTRL = 0x88 | (0<<6) | (0<<5);
	reg_add = (int)(&sBERT.sStr80.RX_DATA_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.TX_DATA_CTRL = 0x08 | (0<<6) | (0<<5); 
	reg_add = (int)(&sBERT.sStr80.TX_DATA_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.RX_SDU_CTRL = 0x8C;
	reg_add = (int)(&sBERT.sStr80.RX_SDU_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.RX_ASD_CTRL = 0x80; 
	reg_add = (int)(&sBERT.sStr80.RX_ASD_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.TX_SDU_CTRL = 0x8C;
	reg_add = (int)(&sBERT.sStr80.TX_SDU_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.TX_ASD_CTRL = 0x80;
	reg_add = (int)(&sBERT.sStr80.TX_ASD_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.RX_FIFO_CTRL = 0x20;
	reg_add = (int)(&sBERT.sStr80.RX_FIFO_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.TX_FIFO_CTRL = 0x20;
	reg_add = (int)(&sBERT.sStr80.TX_FIFO_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.RX_FT_MODE = 0x40;
	reg_add = (int)(&sBERT.sStr80.RX_FT_MODE)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	sBERT.sStr80.TX_FT_MODE = 0x40;
	reg_add = (int)(&sBERT.sStr80.TX_FT_MODE)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	error = GET_Version_F320_DLL(EVB5Handle-0x800, &EVB5_Version);  	
	if (error) return -1; 
	
	if (sBERT_Rate==SBERT_R_R1244Mbps)//1.25G
	{		sBERT.sStr80.CMU_RATE_CTRL = 0xE0;
			reg_add = (int)(&sBERT.sStr80.CMU_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
			sBERT.sStr80.RX_RATE_CTRL = 0xA0;
			reg_add = (int)(&sBERT.sStr80.RX_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
			sBERT.sStr80.TX_RATE_CTRL = 0xA0;
			reg_add = (int)(&sBERT.sStr80.TX_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
//			error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 24, 0, T_wait); 
//			error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 25, 0, T_wait); 
			if (EVB5_Version<7)//EVB5B
			{	error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 24, 0, T_wait); 
				error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 25, 0, T_wait); 
			}
			else			 //EVB5C
			{   error = SET_FS_F320_DLL(EVB5Handle-0x800, 0);
			}
	}
	else //2.488G
	{		sBERT.sStr80.CMU_RATE_CTRL = 0x60;
			reg_add = (int)(&sBERT.sStr80.CMU_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
			sBERT.sStr80.RX_RATE_CTRL = 0xC0;
			reg_add = (int)(&sBERT.sStr80.RX_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
			sBERT.sStr80.TX_RATE_CTRL = 0xC0;
			reg_add = (int)(&sBERT.sStr80.TX_RATE_CTRL)-(int)(&sBERT.sStr80.FirstByte);
			error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
			if (error) return -1;
	
//			error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 24, 0xFF, T_wait); 
//			error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 25, 0x0F, T_wait); 
			if (EVB5_Version<7)//EVB5B
			{	error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 24, 0xFF, T_wait); 
				error = I2C_BYTE_WRITE_DLL (EVB5Handle, EVB5_Addr, 25, 0x0F, T_wait); 
			}
			else			 //EVB5C
			{   error = SET_FS_F320_DLL(EVB5Handle-0x800, 1);
			}
	}
	
	sBERT.sStr80.REFCLK_CTRL=0x00;
	reg_add = (int)(&sBERT.sStr80.REFCLK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.RXCLK_CTRL=0xC0;
	reg_add = (int)(&sBERT.sStr80.RXCLK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.RXIN_CTRL=0x04 | (3<<5);
	reg_add = (int)(&sBERT.sStr80.RXIN_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.TXIN_CTRL=0x04 | (3<<5);
	reg_add = (int)(&sBERT.sStr80.TXIN_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.RX_OUT_CTRL=0x01 | (0<<1) | (0<<3);
	reg_add = (int)(&sBERT.sStr80.RX_OUT_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.TX_OUT_CTRL=0x01 | (0<<1) | (0<<3);
	reg_add = (int)(&sBERT.sStr80.TX_OUT_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.PATGEN_CTRL=(0x10 | sBERT_PRBS);
	reg_add = (int)(&sBERT.sStr80.PATGEN_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.PATCHK_CTRL =((0)<<6) | (0<<5) | (0<<4)| sBERT_PRBS; 
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	return 0; 
}

int EVB5_SBERT_Start (int EVB5Handle, int sBERT_Rate, double sBERT_Time)
//return -1 when USB error, otherwise return 0  
//sBERT_RATE=0 means 1.244G, otherwise 2.488G{int error;
//sBERT_Time is single run time;
{int reg_add;
 double T_wait=0.001, t_max, t1;
 int error;

	if (sBERT_Rate==SBERT_R_R1244Mbps)
		t1 = sBERT_Time/16*1.25E9;
	else
		t1 = sBERT_Time/16*2.48832E9;
	
	sBERT_TIMER_SET[4] = (unsigned char )(  t1 / pow(2, 8*4) );
	sBERT_TIMER_SET[3] = (unsigned char )( (t1 - sBERT_TIMER_SET[4]*pow(2, 8*4)) / pow(2, 8*3));
	sBERT_TIMER_SET[2] = (unsigned char )( (t1 - sBERT_TIMER_SET[4]*pow(2, 8*4) - sBERT_TIMER_SET[3]*pow(2, 8*3)) / pow(2, 8*2));
	sBERT_TIMER_SET[1] = (unsigned char )( (t1 - sBERT_TIMER_SET[4]*pow(2, 8*4) - sBERT_TIMER_SET[3]*pow(2, 8*3) - sBERT_TIMER_SET[2]*pow(2, 8*2)) / pow(2, 8*1));
	sBERT_TIMER_SET[0] = (unsigned char )( (t1 - sBERT_TIMER_SET[4]*pow(2, 8*4) - sBERT_TIMER_SET[3]*pow(2, 8*3) - sBERT_TIMER_SET[2]*pow(2, 8*2) - sBERT_TIMER_SET[1]*pow(2, 8*1)) / pow(2, 8*0));

	sBERT.sStr80.PATCHK_TIMER_SET4=sBERT_TIMER_SET[4];
	sBERT.sStr80.PATCHK_TIMER_SET3=sBERT_TIMER_SET[3];
	sBERT.sStr80.PATCHK_TIMER_SET2=sBERT_TIMER_SET[2];
	sBERT.sStr80.PATCHK_TIMER_SET1=sBERT_TIMER_SET[1];
	sBERT.sStr80.PATCHK_TIMER_SET0=sBERT_TIMER_SET[0];

	reg_add = (int)(&sBERT.sStr80.PATCHK_TIMER_SET4)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 5, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.PATCHK_CTRL &= (~0x20);  
	sBERT.sStr80.PATCHK_CTRL &= (~0x10);  
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.PATCHK_CTRL |= (0x10);   
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	Delay(0.1);

	sBERT.sStr80.PATCHK_CTRL &= (~0x10); 
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	sBERT.sStr80.PATCHK_CTRL |= 0x10;   
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	Delay(0.1);

	return 0;  
}

int EVB5_SBERT_Read (int EVB5Handle, int sBERT_Rate, int sBERT_PRBS, int *sBERT_Status_Code, unsigned char *sBERT_Status_String, double *sBERT_ERR_Number, double *sBERT_ERR_Ratio, double *sBERT_ElapsedTime)
//return -1 when USB error, otherwise return 0
//sBERT_RATE=0 means 1.244G, otherwise 2.488G
//*sBERT_ERR_Number is error bit number
//*sBERT_ERR_Ratio  is error bit ratio
//*sBERT_ElapsedTime is the time Elapsed
//*sBERT_Status_Code is the status, *sBERT_Status_String is to show the event which length is less than 30 bytes 
//=0;  "Completed, no error;"
//=1;  "testing, no error;"
//=-1; "Completed, with error;"
//=-2; "testing, sync error;" 
//=-3; "testing, with error;"
//=-4; "stoped, sync error;" 
//=-5; "unknown error;" 
{	int reg_add; 
    char str[256], PC_CurrentTimeStr[256];
	double TIMER_START, TIMER_STOP;
	int error, error1;
	double T_wait=0.001;
	double sBERT_ErrorRatio;
	
	struct sPATCHK_STATUS_DEF
	{
		INT8U  BUSY:1;    //LSB
		INT8U  SYNCERR:1;
		INT8U  ECF:1;
		INT8U  FAIL:1;
		INT8U  SYNC:1;	
		INT8U  Reserved1:3;	//MSB
	};
	union uPATCHK_STATUS_DEF
	{ struct sPATCHK_STATUS_DEF  sPATCHK_STATUS;
	  INT8U  aPATCHK_STATUS;
	};
	union uPATCHK_STATUS_DEF uPATCHK_STATUS;
	static union uPATCHK_STATUS_DEF old_uPATCHK_STATUS;

	error = I2C_BYTEs_READ_DLL (EVB5Handle, SBERT_Addr, 0, 128, sBERT.pStr80);
	if (error)
	{	return -1; 
	}
	
	if (sBERT_Rate==SBERT_R_R1244Mbps)//1.25G
	{	if (	(sBERT.sStr80.CMU_RATE_CTRL != 0xE0) 
			|| 	(sBERT.sStr80.RX_RATE_CTRL != 0xA0) 
			||	(sBERT.sStr80.TX_RATE_CTRL != 0xA0))
		{   strcpy(sBERT_Status_String, "Initial configuration error; error code -1");
			*sBERT_Status_Code = -99;	
		}
	}
	else //2.488G
	{	if (	(sBERT.sStr80.CMU_RATE_CTRL != 0x60) 
			||	(sBERT.sStr80.RX_RATE_CTRL != 0xC0) 
			||	(sBERT.sStr80.TX_RATE_CTRL != 0xC0))
		{   strcpy(sBERT_Status_String, "Initial configuration error; error code -2");
			*sBERT_Status_Code = -99;	
		}
	}

	if (	(sBERT.sStr80.RX_OUT_CTRL != (0x01 | (0<<1) | (0<<3))) 
		|| 	(sBERT.sStr80.TX_OUT_CTRL != (0x01 | (0<<1) | (0<<3))) 
		|| 	(sBERT.sStr80.PATGEN_CTRL != (0x10 | sBERT_PRBS)) )
		{   strcpy(sBERT_Status_String, "Initial configuration error; error code -3");
			*sBERT_Status_Code = -99;	
		}
	
	if (*sBERT_Status_Code == -99)
		return 0;

	uPATCHK_STATUS.aPATCHK_STATUS = sBERT.sStr80.PATCHK_STATUS;

	if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.ECF==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==0))
	{	strcpy(sBERT_Status_String, "Completed, no error;");
		*sBERT_Status_Code = 0; 
	}
	else if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.ECF==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==1))
	{	strcpy(sBERT_Status_String, "testing, no error;");
		*sBERT_Status_Code = 1; 
	}
	else if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==0))
	{	strcpy(sBERT_Status_String, "Completed, with error;");
		*sBERT_Status_Code = -1; 
	}
	else if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.ECF==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==1))
	{	strcpy(sBERT_Status_String, "testing, sync error;"); 
		*sBERT_Status_Code = -2;
	}
	else if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.ECF==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==1))
	{	strcpy(sBERT_Status_String, "testing, with error;");
		*sBERT_Status_Code = -3;
	}
	else if (  (uPATCHK_STATUS.sPATCHK_STATUS.SYNC==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.FAIL==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.ECF==0) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.SYNCERR==1) &&
		  (uPATCHK_STATUS.sPATCHK_STATUS.BUSY==0))
	{	strcpy(sBERT_Status_String, "stoped, sync error;"); 
		*sBERT_Status_Code = -4; 
	}
	else
	{	strcpy(sBERT_Status_String, "unknown error;"); 
		*sBERT_Status_Code = -5; 
	}

	TIMER_START = sBERT_TIMER_SET[4] * pow(2, 8*4) 
				+ sBERT_TIMER_SET[3] * pow(2, 8*3) 
				+ sBERT_TIMER_SET[2] * pow(2, 8*2) 
				+ sBERT_TIMER_SET[1] * pow(2, 8*1)
				+ sBERT_TIMER_SET[0] * pow(2, 8*0); 
	TIMER_STOP  = sBERT.sStr80.PATCHK_TIMER_GET4 * pow(2, 8*4) 
				+ sBERT.sStr80.PATCHK_TIMER_GET3 * pow(2, 8*3) 
				+ sBERT.sStr80.PATCHK_TIMER_GET2 * pow(2, 8*2) 
				+ sBERT.sStr80.PATCHK_TIMER_GET1 * pow(2, 8*1)
				+ sBERT.sStr80.PATCHK_TIMER_GET0 * pow(2, 8*0); 
	if (sBERT_Rate==SBERT_R_R1244Mbps)
		*sBERT_ElapsedTime = 16*(TIMER_START-TIMER_STOP) / 1.25E9;
	else
		*sBERT_ElapsedTime = 16*(TIMER_START-TIMER_STOP) / 2.48832E9;

	*sBERT_ERR_Number =   sBERT.sStr80.ERRCNT3 * pow(2, 8*3) 
						+ sBERT.sStr80.ERRCNT2 * pow(2, 8*2) 
						+ sBERT.sStr80.ERRCNT1 * pow(2, 8*1) 
						+ sBERT.sStr80.ERRCNT0 * pow(2, 8*0);

	*sBERT_ERR_Ratio = *sBERT_ERR_Number / (16*(TIMER_START-TIMER_STOP));
			
	return 0;
}

int EVB5_SBERT_End (int EVB5Handle)
//return -1 when USB error, otherwise return 0  
{int error;
 int reg_add;
 double T_wait=0.001; 
 
	sBERT.sStr80.PATCHK_CTRL &= (~0x10);   
	reg_add = (int)(&sBERT.sStr80.PATCHK_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
				
	return 0;
}

int EVB5_SBERT_Close (int inst_SBERT)
{
	int 	error, reg_add;
	double 	T_wait=0.001;
	
	sBERT.sStr80.RX_DATA_CTRL = 0x08;
	reg_add = (int)(&sBERT.sStr80.RX_DATA_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (inst_SBERT, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;

	return 0;
}

int EVB5_SBERT_Open (int inst_SBERT)
{
	int 	error, reg_add;
	double 	T_wait=0.001;
	
	sBERT.sStr80.RX_DATA_CTRL = 0x88 | (0<<6) | (0<<5);   
	reg_add = (int)(&sBERT.sStr80.RX_DATA_CTRL)-(int)(&sBERT.sStr80.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (inst_SBERT, SBERT_Addr, reg_add, 1, sBERT.pStr80, T_wait);
	if (error) return -1;
	
	return 0;
}

int EVB5_GET_SN (int EVB5Handle, char SN[16])
{
	int error;
	union uF0_EVB5 local_evb5;

	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	strcpy (SN, local_evb5.sStrF0.vendorSN);
	
	return 0;
}

int EVB5_READ_TxI (int EVB5Handle, double *TxI)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*TxI = (double)(local_evb5.sStrF0.ADC_IT_MON*10E-2);	//1LSB=0.01mA
			
	return 0;	
}

int EVB5_READ_RxI (int EVB5Handle, double *RxI)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*RxI = (double)(local_evb5.sStrF0.ADC_IR_MON*10E-2);	//1LSB=0.01mA
			
	return 0;	
}
int EVB5_READ_TxIRxI (int EVB5Handle, double *TxI,double *RxI)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	*TxI = (double)(local_evb5.sStrF0.ADC_IT_MON*10E-2);	//1LSB=0.01mA   
	*RxI = (double)(local_evb5.sStrF0.ADC_IR_MON*10E-2);	//1LSB=0.01mA
			
	return 0;	
}

int EVB5_READ_ONU_RxSD (int EVB5Handle, BYTE *SD)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*SD = local_evb5.sStrF0.sInput.ONU_RxSD;	 
			
	return 0;	
}

int EVB5_READ_ONU_TxSD (int EVB5Handle, BYTE *SD)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*SD = local_evb5.sStrF0.sInput.ONU_TxSD;	
			
	return 0;	
}

int EVB5_READ_ONU_TxFAULT (int EVB5Handle, BYTE *TxFAULT)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*TxFAULT = local_evb5.sStrF0.sInput.ONU_TxFAULT;	
			
	return 0;	
}

int EVB5_SET_SHDN_VTR(int EVB5Handle, unsigned char SHUTValue)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.SHDN_VTR = SHUTValue; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
			
	return 0;
}

int EVB5_SET_BEN_Mode(int EVB5Handle, unsigned char BEN_Mode)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.BEN_mode = BEN_Mode; 

	reg_add = (int)(&local_evb5.sStrF0.BEN_mode)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0; 	
}

int EVB5_SET_BEN_Level(int EVB5Handle, unsigned char BEN_Level)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.ONU_TxBEN = BEN_Level; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0;
}

int EVB5_SET_BEN_DELAY(int EVB5Handle, INT8U N1_Delay, INT8U N2_Delay, INT8U N3_Delay)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	local_evb5.sStrF0.delay1 = N1_Delay; 
	local_evb5.sStrF0.delay2 = N2_Delay; 
	local_evb5.sStrF0.delay3 = N3_Delay; 

	reg_add = (int)(&local_evb5.sStrF0.delay1)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 3, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0;
}

int EVB5_SET_BEN_PulsePolarity(int EVB5Handle, unsigned char BEN_PulsePolarity)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.BEN_polarity = BEN_PulsePolarity; 

	reg_add = (int)(&local_evb5.sStrF0.BEN_polarity)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0;	
}

int EVB5_WRITE_TINY13_2BYTEs(int EVB5Handle, BYTE Addr1, BYTE Data1, BYTE Addr2, BYTE Data2)
{
	int error;

	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, Addr1, Data1);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error");  
		return -1;
	}
	Delay(0.004);

	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, Addr2, Data2);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error");  
		return -1;
	}
	Delay(0.004);	

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1); //复位等待 	
	
	return 0;
}

int EVB5_WRITE_TINY13_BYTE(int EVB5Handle, BYTE Addr, BYTE Data)
{
	int error;
		
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, Addr, Data);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error");  
		return -1;
	}
	Delay(0.004);

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1); //复位等待 
	
	return 0;	
}

int EVB5_WRITE_TINY13_PAGE(int EVB5Handle, BYTE *rom_value)
{
	int error;
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Write_EEPROM(EVB5Handle, rom_value);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write eeprom error");  
		return -1;
	}

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.3);  
	
	return 0;		
}

int EVB5_READ_TINY13_PAGE(int EVB5Handle, BYTE *rom_value)
{
	int error;
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Read_EEPROM(EVB5Handle, rom_value);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 read eeprom error");  
		return -1;
	}

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);  
	
	return 0;	
}

int EVB5_READ_TINY13_Temper(int EVB5Handle, float *Temper)
{
	int error, count;
	BYTE NUM_EEPROM_ADD, NUM_EEPROM_DAT, rom_value[64];
	signed short  temperature_short; 
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	NUM_EEPROM_ADD = 0x0D; //AMUXA 
	NUM_EEPROM_DAT = 0x00;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);

	NUM_EEPROM_ADD = 0x0E; //AMUXB 
	NUM_EEPROM_DAT = 0x80;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);
	
	NUM_EEPROM_ADD = 0x28; //Control_Temperature_Flag 
	NUM_EEPROM_DAT = 0x03;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);	

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1); //复位等待 
	
	count=0;
	do
	{
		error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
		if (error)
		{   
			MessagePopup ("error", "Tiny13 programming enable error");   
			return -1;
		}						 

		error = TINY13_Read_EEPROM(EVB5Handle, rom_value);
		if (error)
		{   
			MessagePopup ("error", "Tiny13 read eeprom error");  
			return -1;
		}

		error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
		if (error)
		{   
			MessagePopup ("error", "Tiny13 programming disable error"); 
			return -1;
		}
		
		if (rom_value[0x28] != 0x01)
		{
			count++;
			Delay(0.1);
		}
		else
		{	
			break;
		}
	}
	while (count<3); 
	
	Delay(0.1);  
	
	//090226 必须先赋值到一个带符号的变量后，再强制转换数据类型
	temperature_short = (rom_value[0x24]<<8) + rom_value[0x25]; 
	*Temper = (double)temperature_short;
		
	return 0;
}

int EVB5_READ_TINY13_Ibias(int EVB5Handle, double *Ibias)
{
	int error, count;
	BYTE NUM_EEPROM_ADD, NUM_EEPROM_DAT, rom_value[64];   
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	NUM_EEPROM_ADD = 0x0D; //AMUXA 
	NUM_EEPROM_DAT = 0x01;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);

	NUM_EEPROM_ADD = 0x0E; //AMUXB 
	NUM_EEPROM_DAT = 0x00;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);
	
	NUM_EEPROM_ADD = 0x28; //Control_Temperature_Flag 
	NUM_EEPROM_DAT = 0x05;
	error = TINY13_Write_EEPROM_BYTE(EVB5Handle, NUM_EEPROM_ADD, NUM_EEPROM_DAT);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 write byte error"); 
		return -1;
	}						 
	Delay(0.004);	

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1); //复位等待 
	
	count=0;
	do
	{
		error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
		if (error)
		{   
			MessagePopup ("error", "Tiny13 programming enable error");   
			return -1;
		}						 

		error = TINY13_Read_EEPROM(EVB5Handle, rom_value);
		if (error)
		{   
			MessagePopup ("error", "Tiny13 read eeprom error");  
			return -1;
		}

		error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
		if (error)
		{   
			MessagePopup ("error", "Tiny13 programming disable error"); 
			return -1;
		}
		
		if (rom_value[0x28] != 0x01)
		{
			count++;
			Delay(0.1);
		}
		else
		{	
			break;
		}
	}
	while (count<3); 

	Delay(0.1);  
	
	*Ibias = (double)(((rom_value[0x26]<<8) + rom_value[0x27])/1023.*1.1*0.1*1E3); 
		
	return 0;	
}

int TINY13_Programming_Enable(int EVB5Handle) //进入编程模式，返回错误代码
{unsigned char temp_char;
 unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 unsigned char MISO_Buffer[256];	
 int error;
 
	error = SetF320_SPI_NSS_DLL(EVB5Handle-0x800, 1);
	Delay(0.001);	//RESET must be given a positive pulse of at least two CPU clock cycles duration after SCK has been set to “0”.,  
    error = SetF320_SPI_NSS_DLL(EVB5Handle-0x800, 0);

    Delay(0.02);	//Wait for at least 20 ms and enable serial programming by sending the Programming Enable serial instruction to pin MOSI.

	MOSI_Buffer[0]=0xAC;
	MOSI_Buffer[1]=0x53;
	MOSI_Buffer[2]=0xFF;
	MOSI_Buffer[3]=0x00;
	error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

	temp_char = MISO_Buffer[2];   

	if (temp_char != 0x53) //sync error, try again
	{	
		error = SetF320_SPI_NSS_DLL(EVB5Handle-0x800, 1);
		Delay(0.001);	//RESET must be given a positive pulse of at least two CPU clock cycles duration after SCK has been set to “0”.,  
	    error = SetF320_SPI_NSS_DLL(EVB5Handle-0x800, 0);

    	Delay(0.02);	//Wait for at least 20 ms and enable serial programming by sending the Programming Enable serial instruction to pin MOSI.
	
		MOSI_Buffer[0]=0xAC;
		MOSI_Buffer[1]=0x53;
		MOSI_Buffer[2]=0xFF;
		MOSI_Buffer[3]=0x00;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);
		
		temp_char = MISO_Buffer[2]; 
		if (temp_char != 0x53)
			return -1; 	
	}	

	return 0;
}

int TINY13_Programming_Disable(int EVB5Handle) //退出编程模式
{	
	return SetF320_SPI_NSS_DLL(EVB5Handle-0x800, 1);
}

int TINY13_Read_EEPROM(int EVB5Handle, unsigned char *EEPROM_Array)
{
	int i;
	unsigned char MOSI_Buffer[256+1]; //PC下发到F320通过SPI MOSI出去的数组
	unsigned char MISO_Buffer[256+1];	
	int error;
	#define SPI_RDEE   0xB1 

	//页读，F320 V4.0支持
	MISO_Buffer[0] = 0;//（由于Tiny13只有64字节的EEPROM，所以只需要一次就可读完）
	MISO_Buffer[1] = MAX_BYTENUMBER_EEPROM;//指示一次读64字节
    error = F320ExecuteCmd_DLL(EVB5Handle-0x800, SPI_RDEE, 2, MAX_BYTENUMBER_EEPROM+2, MISO_Buffer);//返回64+2字节，其中的前两个字节分别是index和nums
    memcpy(EEPROM_Array, MISO_Buffer+2, MAX_BYTENUMBER_EEPROM);//丢弃前两字节

	return 0;
}

int TINY13_Write_EEPROM(int EVB5Handle, unsigned char *EEPROM_Array)
{
	int i, error;
 	unsigned char MOSI_Buffer[256+1]; //PC下发到F320通过SPI MOSI出去的数组
 	unsigned char MISO_Buffer[256+1];	
	#define SPI_WREE   0xB0

	//页写，F320 V4.0支持
	MOSI_Buffer[0] = 0;//（由于Tiny13只有64字节的EEPROM，所以只需要一次就可写完）
	MOSI_Buffer[1] = MAX_BYTENUMBER_EEPROM;//指示一次写64字节
    memcpy(MOSI_Buffer+2, EEPROM_Array, MAX_BYTENUMBER_EEPROM); //追加64字节数据
    F320ExecuteCmd_DLL(EVB5Handle-0x800, SPI_WREE, 2+MAX_BYTENUMBER_EEPROM, 0, MOSI_Buffer);//两个字节的index和nums加上64字节数据

	return 0;
}

int TINY13_Write_EEPROM_BYTE(int EVB5Handle, unsigned char NUM_EEPROM_ADD, unsigned char NUM_EEPROM_DAT)
{
	int i, error;
	unsigned char MOSI_Buffer[256+1]; //PC下发到F320通过SPI MOSI出去的数组
	unsigned char MISO_Buffer[256+1];	
	#define SPI_WREE   0xB0

	MOSI_Buffer[0]=0xC0;
	MOSI_Buffer[1]=0x00;
	MOSI_Buffer[2]=NUM_EEPROM_ADD&0xFF;
	MOSI_Buffer[3]=NUM_EEPROM_DAT;
	error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

	return error;
}

int EVB5_READ_Temper(int EVB5Handle, double *Temper)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*Temper = local_evb5.sStrF0.Core_Temperature/256.;   
	
	return 0;
}

int EVB5_WRITE_TINY13_Firmware(int EVB5Handle, BYTE *FirmwareData)
{
	int i, error;
	unsigned int Page_Array[128];	//有效页标志，如果该页包含有效数据，则等于1，否则=0;  
 	int CurrentPageNumner, CurrentDataNumber; 
	BYTE FirmwareData_r[1024];  
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Chip_Erase(EVB5Handle);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 erase flash error");  
		return -1;
	}
	Delay(0.2); //mega88的64byte的擦除时间是4ms，那么全片FLASH擦完，要耗时32*4ms=0.128s  
	
	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);  
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}
	
	//分析有效页标志，如果该页包含有效数据，则等于1，否则=0; 注意一页有248个数据
	memset(Page_Array, 0, sizeof(Page_Array)); //初始化所有有效页标志为无效；

	for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)
	{	for (i=0; i<MAX_ONEPAGEDATANUMBER; i++)
		{	CurrentDataNumber= CurrentPageNumner*MAX_ONEPAGEDATANUMBER+i;
			if (FirmwareData[CurrentDataNumber]!=0xFF)	//当前data数据不为0xff就说明该页包含合法数据
			{Page_Array[CurrentPageNumner]=1;			//置该页标志为有效 
			 break;
			}
		}
	}
			
	for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)  
	{ 	
		if (Page_Array[CurrentPageNumner]) 
		{	
			error = TINY13_OnePage_WRITE_FLASH(EVB5Handle, CurrentPageNumner, FirmwareData); //页写FLASH
			if (error)
			{ 	
			  return -1;
			}
		}
	}

	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1); 
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)  
	{ 	
		error = TINY13_OnePage_READ_FLASH(EVB5Handle, CurrentPageNumner, FirmwareData_r); //页读FLASH
		if (error)
		{ 
			MessagePopup ("error", "Tiny13 read flash error");	
		  	return -1; 
		}
	}	 
	
	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);
	
	for (i=0; i<1024; i++)
	{
		if (FirmwareData_r[i] != FirmwareData[i]) 
		{   
			MessagePopup ("error", "Tiny13 flash check error"); 
			return -1;
		}
	}
	
	return 0;
}

int TINY13_Chip_Erase(int EVB5Handle)	   //擦除FLASH、lock位、EEPROM(如果EESAVE==1)
{
	unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 	unsigned char MISO_Buffer[256];	
 	int error;
 
	MOSI_Buffer[0]=0xAC;
	MOSI_Buffer[1]=0x80;
	MOSI_Buffer[2]=0x00;
	MOSI_Buffer[3]=0x80;
	error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

	return error;
}

int TINY13_OnePage_WRITE_FLASH(int EVB5Handle, int CurrentPageNumner, unsigned char *Data_Array) 
{
	int error, i;
	int CurrentDataNumber; 
 	int device_addr, rom_startaddress, NoOfDataBytes;
 	unsigned char PageAddr_MSB, PageAddr_LSB;
 	unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 	unsigned char MISO_Buffer[256];
	#define SPI_LDPG   0xA5 

	//页写，F320 V4.0支持
	MOSI_Buffer[0] = 0;//Load Page 页号无效，可以为任意值
	MOSI_Buffer[1] = MAX_ONEPAGEDATANUMBER;//Tiny13 一页是32字节
	memcpy (MOSI_Buffer+2, Data_Array+CurrentPageNumner*MAX_ONEPAGEDATANUMBER, MAX_ONEPAGEDATANUMBER);//追加32字节数据
	error = F320ExecuteCmd_DLL(EVB5Handle-0x800, SPI_LDPG, MAX_ONEPAGEDATANUMBER+2, 0, MOSI_Buffer);//两个字节的index和nums加上32字节数据

	PageAddr_MSB = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER / 2) >> 8;
	PageAddr_LSB = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER / 2) & 0xFF;

	MOSI_Buffer[0]=0x4C;	// Write Program Memory Page 
	MOSI_Buffer[1]=PageAddr_MSB;
	MOSI_Buffer[2]=PageAddr_LSB;
	MOSI_Buffer[3]=0x00;
	error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

	Delay(0.1);	//如果不使用查询的方式，那么在操作下一页数据之前应等待至少tWD_FLASH 的时间，4.5 ms。 
	
	return 0;
}

int TINY13_OnePage_READ_FLASH(int EVB5Handle, int CurrentPageNumner, unsigned char *Data_Array) 
{
	int error, i;
 	int CurrentDataNumber; 
 	int device_addr, rom_startaddress, NoOfDataBytes;
 	unsigned char WordAddr_MSB, WordAddr_LSB;
 	unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 	unsigned char MISO_Buffer[256];
 	#define SPI_RDPG   0xA6 

	//页读，F320 V4.0支持
	MISO_Buffer[0] = CurrentPageNumner; //页地址；（还好只有128页）
    MISO_Buffer[1] = MAX_ONEPAGEDATANUMBER;
    F320ExecuteCmd_DLL(EVB5Handle-0x800, SPI_RDPG, 2, MAX_ONEPAGEDATANUMBER+2, MISO_Buffer);//返回两个字节的index和nums加上32字节数据

    memcpy(Data_Array+CurrentPageNumner*MAX_ONEPAGEDATANUMBER, MISO_Buffer+2, MAX_ONEPAGEDATANUMBER);//丢弃前两个字节

	return 0;
}

int TINY13_Check_Signature_Byte(int EVB5Handle) //读出并检查TINY13的标志字，返回错误代码
{unsigned char temp_char;
 unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 unsigned char MISO_Buffer[256];	
 int error;

		MOSI_Buffer[0]=0x30;
		MOSI_Buffer[1]=0x00;
		MOSI_Buffer[2]=0x00;
		MOSI_Buffer[3]=0xFF;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

		temp_char = MISO_Buffer[3];   
		if (temp_char != 0x1E) return -1;
		
		MOSI_Buffer[0]=0x30;
		MOSI_Buffer[1]=0x00;
		MOSI_Buffer[2]=0x01;
		MOSI_Buffer[3]=0xFF;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

		temp_char = MISO_Buffer[3];   
		if (temp_char != 0x90) return -1;
		
		
		MOSI_Buffer[0]=0x30;
		MOSI_Buffer[1]=0x00;
		MOSI_Buffer[2]=0x02;
		MOSI_Buffer[3]=0xFF;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

		temp_char = MISO_Buffer[3];   
		if (temp_char != 0x07) return -1;

	return 0;
}

int TINY13_Write_Fuse_bits(int EVB5Handle, unsigned char FUSE, unsigned char FUSE_HIGH) //设置三个熔丝位
{unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 unsigned char MISO_Buffer[256];	
 int error;
 
		MOSI_Buffer[0]=0xAC;
		MOSI_Buffer[1]=0xA0;
		MOSI_Buffer[2]=0x00;
		MOSI_Buffer[3]=FUSE;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);

		MOSI_Buffer[0]=0xAC;
		MOSI_Buffer[1]=0xA8;
		MOSI_Buffer[2]=0x00;
		MOSI_Buffer[3]=FUSE_HIGH;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);
	
	return 0;
}

int TINY13_Read_Fuse_bits(int EVB5Handle, unsigned char *FUSE, unsigned char *FUSE_HIGH, unsigned char *FUSE_Extended) //回读三个熔丝位
{int error;
 unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 unsigned char MISO_Buffer[256];	

		MOSI_Buffer[0]=0x50;
		MOSI_Buffer[1]=0x00;
		MOSI_Buffer[2]=0x00;
		MOSI_Buffer[3]=0xFF;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);
		if (error)
			return -1;
		*FUSE = MISO_Buffer[3];   

		MOSI_Buffer[0]=0x58;
		MOSI_Buffer[1]=0x08;
		MOSI_Buffer[2]=0x00;
		MOSI_Buffer[3]=0xFF;
		error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 4, MOSI_Buffer, MISO_Buffer);
		if (error)
			return -1;
		*FUSE_HIGH = MISO_Buffer[3];   
	
	return 0;
}

int EVB5_WRITE_TINY13_FuseBit(int EVB5Handle)
{
	int error;
	unsigned char FUSE, FUSE_HIGH, FUSE_Extended;  
	
	error = TINY13_Programming_Enable(EVB5Handle);		//进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	error = TINY13_Check_Signature_Byte(EVB5Handle);	//校验MCU标志位    
	if (error)
	{   
		MessagePopup ("error", "Tiny13 check id error"); 
		return -1;
	}
	
	FUSE_HIGH = 0xFB;	// f = 9.6MHz with start-up time of 64ms
	FUSE = 0x3A; 		// set brown-out detection level at Vcc = 2.7v, EEPROM preserved
	error = TINY13_Write_Fuse_bits(EVB5Handle, FUSE, FUSE_HIGH);
	if (error)
	{   
		MessagePopup ("error", "Tiny13 set fusebit error");  
		return -1;
	}
	Delay(0.1); //mega88的64byte的擦除时间是9ms，此处尚不确定写fuse需要耗时多少  
	
	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);  
	
	return 0;
}

int EVB5_READ_TINY13_FuseBit(int EVB5Handle)
{
	int error;
	unsigned char FUSE, FUSE_HIGH, FUSE_Extended;  
	
	error = TINY13_Programming_Enable(EVB5Handle);		//进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}
	
	error = TINY13_Read_Fuse_bits(EVB5Handle, &FUSE, &FUSE_HIGH, &FUSE_Extended);//读取熔丝位
	if (error)
	{   
		MessagePopup ("error", "Tiny13 read fusebit error");  
		return -1;
	}
	
	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);  
	
	if (FUSE_HIGH!=0xFB || FUSE!=0x3A)
	{
		MessagePopup ("error", "Tiny13 check fusebit error");  
		return -1;
	}
	return 0;
}

int EVB5_READ_TINY13_Firmware(int EVB5Handle, BYTE *FirmwareData) 
{
	int error, CurrentPageNumner;
	
	error = TINY13_Programming_Enable(EVB5Handle);   //进入编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming enable error");   
		return -1;
	}						 
	
	for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)  
	{ 	
		error = TINY13_OnePage_READ_FLASH(EVB5Handle, CurrentPageNumner, FirmwareData); //页读FLASH
		if (error)
		{ 
			MessagePopup ("error", "Tiny13 read flash error");	
		  	return -1; 
		}
	}	 
	
	error = TINY13_Programming_Disable(EVB5Handle);  //退出编程模式
	if (error)
	{   
		MessagePopup ("error", "Tiny13 programming disable error"); 
		return -1;
	}
	Delay(0.1);
	
	return 0;
}

int EVB5_READ_OLT_TxFAULT (int EVB5Handle, BYTE *TxFAULT)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*TxFAULT = local_evb5.sStrF0.sInput.OLT_TxFAULT;	
			
	return 0;	
}

int EVB5_READ_OLT_RxLOS (int EVB5Handle, BYTE *RxLOS)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;				
	
	*RxLOS = local_evb5.sStrF0.sInput.OLT_RxLOS;	
			
	return 0;	
}

int EVB5_SET_OLT_TxDIS (int EVB5Handle, BYTE TxDIS)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.OLT_TxDIS = TxDIS; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
			
	return 0;
}

int EVB5_SET_SHDN_VOSA (int EVB5Handle, unsigned char SHUTValue)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.SHDN_VOSA = SHUTValue; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
			
	return 0;
}

int EVB5_SET_ADN8810 (int EVB5Handle, float If)
{
	float Rsn=3.3; 
	int error, reg_add; 
	union uF0_EVB5 local_evb5;
	unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
	unsigned char MISO_Buffer[256];	
	
	//If = (EVB.sStrF0.DAC_ADN8810_SET/1000.0/Rsn*(Rsn/15.0E3+0.1)) *1000.0;//mA  
		
	local_evb5.sStrF0.DAC_ADN8810_SET = (INT16U)(If*Rsn/(Rsn/15.0E3+0.1)); 
		
	MOSI_Buffer[0]=local_evb5.sStrF0.DAC_ADN8810_SET >>8;
	MOSI_Buffer[1]=local_evb5.sStrF0.DAC_ADN8810_SET & 0xFF;
	error = SPI_BYTEs_READWRITE_F320_DLL (EVB5Handle-0x800, 2, MOSI_Buffer, MISO_Buffer);
	if (error) return -1; 
				
	reg_add = (int)(&local_evb5.sStrF0.DAC_ADN8810_SET)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 2, local_evb5.pStrF0, 0.1); 
	if (error) return -1; 
	
	return 0;
}

int EVB5_GET_ADN8810(int EVB5Handle, float *If, float *Vf, float *Im)
{
	int error;
	float Rsn=3.3; 
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	
	
	*If = local_evb5.sStrF0.DAC_ADN8810_SET/Rsn*(Rsn/15.0E3+0.1);//mA  
	*Vf = local_evb5.sStrF0.ADC_LD_MON/1000.;
	*Im = (local_evb5.sStrF0.ADC_PD_MON/1000.)/1000.; 
	
	return 0;
}

int EVB5_SET_VAPD(int EVB5Handle, const double Vapd)
{
int i, error, reg_add;    
union uF0_EVB5 local_evb5;

	local_evb5.sStrF0.Vapd_expect=Vapd;
	reg_add = (int)(&local_evb5.sStrF0.Vapd_expect)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0, 0.1);  
	if (error<0) return -1; 
	
	local_evb5.sStrF0.VapdAdj_en=1;
	reg_add = (int)(&local_evb5.sStrF0.VapdAdj_en)-(int)(&local_evb5.sStrF0.FirstByte);  
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error<0) return -1; 
	
	//reg_add = (int)(&EVB.sStrF0.Tune_VPD_MON)-(int)(&EVB.sStrF0.FirstByte); ???是否需要计算
	i=0;
	do
	{
		Delay(0.1); 
	
		error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
		if (error<0) return -1; //MCU access error!

		i++;
		
	}while (local_evb5.sStrF0.VapdAdj_en==1 && i<100);
	
	
	if (local_evb5.sStrF0.VapdAdj_loop >= 100)
	{MessagePopup ("error", "EVB5 Set Vapd fail"); return -1;}
	return 0;
}

int EVB5_SET_MODE (int EVB5Handle, const char Mode)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.FirstByte = Mode; 

	reg_add = (int)(&local_evb5.sStrF0.FirstByte)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0;
}

int EVB5_SET_VPD_MOD_EN (int EVB5Handle, const char Mode)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.VPD_MON_EN = Mode; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
	
	return 0;
}

int EVB5_READ_IPD_MON (int EVB5Handle, double *Iapd)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	
	
	*Iapd = local_evb5.sStrF0.ADC_IPD_MON/100.;   
	
	return 0;
}

int EVB5_READ_VPD_MON (int EVB5Handle, double *Vapd)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	//设置到电压档
	error = EVB5_SET_VPD_MOD_EN (EVB5Handle, 1);
	if (error<0) return -1;	
	Delay(0.5);
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	
	
	*Vapd = local_evb5.sStrF0.ADC_VPD_MON/1000.;   
	
	//设置到电流档
	error = EVB5_SET_VPD_MOD_EN (EVB5Handle, 0);
	if (error<0) return -1;	
	Delay(1);  
	
	return 0;
}

int EVB5_SET_Vbr_3V(int EVB5Handle, float *Vbr)
{
int i, error, reg_add;   
union uF0_EVB5 local_evb5;

	reg_add = (int)(&local_evb5.sStrF0.Vbr_3V_en)-(int)(&local_evb5.sStrF0.FirstByte);
	local_evb5.sStrF0.Vbr_3V_en = 1;
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Vbr_3V_en fail"); return -1;}  
	
	i=0;*Vbr=0;
	reg_add = (int)(&local_evb5.sStrF0.Vbr_3V_en)-(int)(&local_evb5.sStrF0.FirstByte);
	do
	{
		Delay(0.1); 

		error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0);
		if (error<0) return -1; //EVB access error!
		
		i++;							 
	}while (local_evb5.sStrF0.Vbr_3V_en==1 && i<100);
	
	if (i>=100) {MessagePopup ("error", "EVB5 set Vbr_3V fail"); return -1;}  
	
	*Vbr = local_evb5.sStrF0.ADC_Vbr_MON/1000.;
		
	return 0;
}

int EVB5_SET_VAPD_30V(int EVB5Handle)
{
	int i, error, reg_add;   
	union uF0_EVB5 local_evb5;

	local_evb5.sStrF0.Vapd_30V_en = 1;
	reg_add = (int)(&local_evb5.sStrF0.Vapd_30V_en)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Vbr_3V_en fail"); return -1;}  
	
	i=0;
	reg_add = (int)(&local_evb5.sStrF0.Vapd_30V_en)-(int)(&local_evb5.sStrF0.FirstByte);
	do
	{
		Delay(0.1); 

		error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0);
		if (error<0) return -1; //EVB access error!
		
		i++;							 
	}while (local_evb5.sStrF0.Vapd_30V_en==1 && i<100);
	
	if (i>=100) {MessagePopup ("error", "EVB5 set 30V fail"); return -1;}  
			
	return 0;
}

int EVB5_SET_IAPD_EXPECT (int EVB5Handle, const float Iapd_expectMIN, const float Iapd_expectMAX)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	local_evb5.sStrF0.Iapd_expectMIN = Iapd_expectMIN; 
	local_evb5.sStrF0.Iapd_expectMAX = Iapd_expectMAX;  
	
	reg_add = (int)(&local_evb5.sStrF0.Iapd_expectMIN)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 8, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Iapd_expect fail"); return -1;} 
	
	return 0;
}

int EVB5_SET_IAPD(int EVB5Handle, float *Vapd, float *Iapd)
{
	int i, error, reg_add; 
	union uF0_EVB5 local_evb5;
	
	double temp;

	reg_add = (int)(&local_evb5.sStrF0.IapdAdj_en)-(int)(&local_evb5.sStrF0.FirstByte);
	local_evb5.sStrF0.IapdAdj_en = 1;
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Vbr_3V_en fail"); return -1;}  
	
	i=0;*Vapd=0;
	reg_add = (int)(&local_evb5.sStrF0.Tune_IPD_MON)-(int)(&local_evb5.sStrF0.FirstByte);
	do
	{
		Delay(0.1); 

		error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, reg_add, 4, local_evb5.pStrF0);
		if (error<0) return -1; //EVB access error!
		
		i++;							 
	}while (local_evb5.sStrF0.IapdAdj_en==1 && i<100);
	
	if (i>=100) {MessagePopup ("error", "EVB5 set Iapd_expect fail"); return -1;}  
	
	*Vapd=0;
	error = EVB5_READ_VPD_MON (EVB5Handle, &temp);
	if (error<0) {MessagePopup ("error", "EVB5 read Vapd fail"); return -1;}  
	*Vapd=temp;

	*Iapd=0;
	error = EVB5_READ_IPD_MON (EVB5Handle, &temp);
	if (error<0) {MessagePopup ("error", "EVB5 read Iapd fail"); return -1;}  
	*Iapd=temp;
	
	return 0;
}

int EVB5_READ_VAPDM_MON (int EVB5Handle, double *Vapd)
{
	int error;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	
	
	*Vapd = local_evb5.sStrF0.ADC_VPDM_MON/1000.;   
	
	return 0;
}

int EVB5_SET_CLOCK(int EVB5Handle, double Clock)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	local_evb5.sStrF0.myPWMDAT0 = 41.78/2./Clock;
	reg_add = (int)(&local_evb5.sStrF0.myPWMDAT0)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 2, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Clock fail"); return -1;} 

	return 0;
}

int EVB5_SET_BEN_PW(int EVB5Handle, int BEN_PW)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	local_evb5.sStrF0.BEN_PW = BEN_PW;
	
	reg_add = (int)(&local_evb5.sStrF0.BEN_PW)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 2, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set BEN_PW fail"); return -1;} 
	
	return 0; 
}

int EVB5_GET_R7_SC (int EVB5Handle, int *ErrorCode, char *ErrorStr)
{
	int error; 
	union uF0_EVB5 local_evb5;

	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0); 
	if (error<0) return -1;
	
	*ErrorCode=0;
	strcpy (ErrorStr, "short test success");   
	
	if (local_evb5.sStrF0.sFPGA_R7_SC.SC_GND)
	{
		*ErrorCode=-1;
		strcpy (ErrorStr, "GND short test error");
		return 0;
	}
	
	if (local_evb5.sStrF0.sFPGA_R7_SC.SC_VAPD)
	{
		*ErrorCode=-2;
		strcpy (ErrorStr, "VAPD short test error");
		return 0;
	}
	
	if (local_evb5.sStrF0.sFPGA_R7_SC.SC_VCC)
	{
		*ErrorCode=-3;
		strcpy (ErrorStr, "VCC short test error");
		return 0;
	}
	
	return 0;
}

int MEGA88_Programming_Enable(int inst_EVB5) //进入编程模式，返回错误代码
{
	unsigned char temp_char;
 	unsigned char MOSI_Buffer[256]; //PC下发到F320通过SPI MOSI出去的数组
 	unsigned char MISO_Buffer[256];	
 	int error;
 
	error = SetF320_SPI_NSS_DLL(inst_EVB5-0x800, 1);
	Delay(0.001);	//RESET must be given a positive pulse of at least two CPU clock cycles duration after SCK has been set to “0”.,  
    error = SetF320_SPI_NSS_DLL(inst_EVB5-0x800, 0);

    Delay(0.02);	//Wait for at least 20 ms and enable serial programming by sending the Programming Enable serial instruction to pin MOSI.

	MOSI_Buffer[0]=0xAC;
	MOSI_Buffer[1]=0x53;
	MOSI_Buffer[2]=0xFF;
	MOSI_Buffer[3]=0x00;
	error = SPI_BYTEs_READWRITE_F320_DLL (inst_EVB5-0x800, 4, MOSI_Buffer, MISO_Buffer);

	temp_char = MISO_Buffer[2];   

	if (temp_char != 0x53) //sync error, try again
	{	
		return -1; 	
	}	

	return 0;
}

int MEGA88_Programming_Disable(int inst_EVB5) //退出编程模式
{	
	return SetF320_SPI_NSS_DLL(inst_EVB5-0x800, 1);
}

//读取SPI总线设备 返回0，表示SPI上没有连接设备mega88， -1表示SPI上有连接正常的设备mega88
int EVB5_GET_SPI_MEGA88 (int inst_EVB5)
{
	int error;
	
	error = MEGA88_Programming_Enable (inst_EVB5);
	if (error)
	{
		return 0;	
	}
	else
	{
		MEGA88_Programming_Disable (inst_EVB5); 
		return -1;
	}
	
	return 0;
}

int EVB5_SET_TRIG_LEVEL (int EVB5Handle, int tigger_level)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;

	error = I2C_BYTEs_READ_DLL (EVB5Handle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	
	
	local_evb5.sStrF0.sOutput.OLT_Trig = tigger_level;
	
	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (EVB5Handle, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error<0) {MessagePopup ("error", "EVB5 set Trig level fail"); return -1;} 
	
	return 0;
}

int EVB5_SET_ONU_TxDIS (int inst_EVB5, BYTE TxDIS)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	local_evb5.sStrF0.sOutput.ONU_TxDIS = TxDIS; 

	reg_add = (int)(&local_evb5.sStrF0.sOutput)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1);
	if (error) return -1;
			
	return 0;
}
int EVB5_GET_ONU_TxDIS (int inst_EVB5, BYTE *TxDIS)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1;	

	*TxDIS = local_evb5.sStrF0.sOutput.ONU_TxDIS; 

	return 0;
}
int EVB5_SET_VTR (int inst_EVB5, float VTR)
{
	int error, reg_add, i; 
	union uF0_EVB5 local_evb5;
	
	local_evb5.sStrF0.VTR_expect = VTR;
	reg_add = (int)(&local_evb5.sStrF0.VTR_expect)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 4, local_evb5.pStrF0, 0.1); 
	if (error) return -1; 
	
	local_evb5.sStrF0.VTRAdj_en = 1; 
	reg_add = (int)(&local_evb5.sStrF0.VTRAdj_en)-(int)(&local_evb5.sStrF0.FirstByte);
	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.1); 
	if (error) return -1; 

	i=0;
	do
	{
		Delay(0.1); 

		error = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0);
		if (error<0) return -1; //EVB access error!
		
		i++;							 
	}while (local_evb5.sStrF0.VTRAdj_en==1 && i<100);
	
	if (i>=100) {MessagePopup ("error", "EVB5 set VTR_expect fail"); return -1;}  
	
	return 0;
}

int EVB5_GET_VTR (int inst_EVB5, float *VTR)
{
	int error, reg_add, i; 
	union uF0_EVB5 local_evb5;
	
	reg_add = (int)(&local_evb5.sStrF0.VTR_expect)-(int)(&local_evb5.sStrF0.FirstByte);     
	error = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0);
	if (error<0) 
	{
		return -1; 
	}
	
	*VTR = local_evb5.sStrF0.VTR_expect;  
	
	return 0;
}

int EVB5_SET_VAPD_10V(int inst_EVB5)
{
	int error, reg_add;  
	union uF0_EVB5 local_evb5;
	
	//需要验证是否必要
	error = EVB5_SET_MODE(inst_EVB5, 0x4d); //设置到手动模式 ‘M’
	if (error) return -1;
		
	local_evb5.sStrF0.DAC0_SET_VPD=0xFFF;
	reg_add = (int)(&local_evb5.sStrF0.DAC0_SET_VPD)-(int)(&local_evb5.sStrF0.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 2, local_evb5.pStrF0, 0.1);  
	if (error<0) return -1; 
	
	error = EVB5_SET_MODE(inst_EVB5, 0);	//设置到自动模式
	if (error) return -1;
	
	return 0;
}

int sevb5_get_mod_def0 (int inst, int *val)
{
	int error;
	union uF0_EVB5 local_evb5;
	
	error = I2C_BYTEs_READ_DLL (inst, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (error<0) return -1; //EVB access error!
	
	*val = local_evb5.sStrF0.sInput.MOD_DEF0;
		 
	return 0;
}

int EVB5_SET_TxSD_Detect (int inst_EVB5)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	reg_add = (int)(&local_evb5.sStrF0.sTxSD_Detect)-(int)(&local_evb5.sStrF0.FirstByte); 
	
	error   = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0); 
	if (error) return -1; 
	
	local_evb5.sStrF0.sTxSD_Detect.TxSD_ENABLE = 1;
	
	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.00);//写RAM，可以不等待
	if (error) return -1; 
	
	return 0; 
}

int EVB5_READ_TxSD (int inst_EVB5, BYTE *TxSD_Error)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	reg_add = (int)(&local_evb5.sStrF0.sTxSD_Detect)-(int)(&local_evb5.sStrF0.FirstByte);
	
	error   = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0); 
	if (error) return -1; 
	
	*TxSD_Error = local_evb5.sStrF0.sTxSD_Detect.TxSD_ERROR;
		
	return 0; 
}

int EVB5_CLEAR_TxSD (int inst_EVB5)
{
	int error, reg_add;
	union uF0_EVB5 local_evb5;
	
	reg_add = (int)(&local_evb5.sStrF0.sTxSD_Detect)-(int)(&local_evb5.sStrF0.FirstByte); 
	
	error   = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0); 
	if (error) return -1; 
	
	local_evb5.sStrF0.sTxSD_Detect.TxSD_ERROR = 0;

	error = I2C_BYTEs_WRITE_DLL (inst_EVB5, EVB5_Addr, reg_add, 1, local_evb5.pStrF0, 0.00);//写RAM，可以不等待 
	if (error) return -1; 
	
	return 0; 
}

int EVB5_READ_FirmwareVersion (int inst_EVB5, int *version)
{
	int error;
	union uF0_EVB5 local_evb5;
		
	error = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, 0, 256, local_evb5.pStrF0);   
	if (error) return -1; 
	
	*version = local_evb5.sStrF0.FirmwareVersion*10+local_evb5.sStrF0.FirmwareVersionSuffix; 
		
	return 0;
}

int EVB5_Get_MOD_DEF0(int inst_EVB5, int *val)
{
	int err;
	union uF0_EVB5 local_evb5;
	
	err = I2C_BYTEs_READ_DLL (inst_EVB5, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (err<0) return -1;	
	
	*val= local_evb5.sStrF0.sInput.MOD_DEF0;
	
	return 0;
}

int EVB5_READ_Reign_ADC(int instHandle, int *ADC)
{
	int err;
	union uF0_EVB5 local_evb5;
	
	err = I2C_BYTEs_READ_DLL (instHandle, EVB5_Addr, 0, 256, local_evb5.pStrF0);
	if (err<0) return -1;				
	
	*ADC = local_evb5.sStrF0.Reign_ADC; 
	return 0;
}
