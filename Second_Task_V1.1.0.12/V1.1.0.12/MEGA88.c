#include "MEGA88.h"
#include "CH341A_DLL.h" 
#include <userint.h>
#include <utility.h>
#include <ansi_c.h>

int MEGA88_Password1(int USB_I2CHandle)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 0x7B, 0, 0, 0, 0, 0.1);
	if (error<0) {MessagePopup ("ERROR", "NO Acknowledge from target !");  return -1;}
	
	return 0;
}

int MEGA88_Password2(int USB_I2CHandle)
{
	int error;
	
	error = I2C_4BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 0x7B, 'S', 'O', 'E', 'B', 0.2);
	if (error<0) {MessagePopup ("ERROR", "NO Acknowledge from target !");  return -1;}
	
	return 0;
}

int MEGA88_SET_Table(int USB_I2CHandle, BYTE Table)
{
	int error, reg_add; 
	
	union uA2 localA2; 

	localA2.sStrA2.LUTIndex = Table; 
	
	reg_add = 127;
	error = I2C_BYTE_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, localA2.sStrA2.LUTIndex, 0.1);	
	if (error<0) {MessagePopup ("ERROR", "MEGA88 set table error!"); return -1;}   
		
	return 0;
}

//从手动模式到自动模式，最长需idle (256+96+128)*3.9ms=1.872秒等待pageA0/A2[]单元存储到EEPROM,
//因此保险起见，当设置到自动模式时，统一等待2s
int MEGA88_SET_Mode_Auto(int USB_I2CHandle)
{
	int error, reg_add;
	union uA2 localA2;
	
	localA2.sStrA2.mode=0;
		
	reg_add = 122; 
	error = I2C_BYTE_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, localA2.sStrA2.mode, 2);
	if (error<0) return -1;

	return 0;
}

int MEGA88_SET_Mode_Manual(int USB_I2CHandle)
{
	int error, reg_add;
	union uA2 localA2;
	
	localA2.sStrA2.mode=1;
		
	reg_add = 122; 
	error = I2C_BYTE_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, localA2.sStrA2.mode, 0.1);
	if (error<0) return -1;

	return 0;
}

int MEGA88_SET_TRxConfigFlag(int USB_I2CHandle, struct sTRxConfig_DEF TRxConfigFlag)
{
	int error, reg_add; 
	union uA2table1 localA2table1;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table1.sStr.sTRxConfig = TRxConfigFlag; 
	reg_add =(int)(&localA2table1.sStr.sTRxConfig) - (int)(&localA2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set ben_active error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_SoftTxDis(int USB_I2CHandle, BYTE SoftTxDis)
{
	int error, reg_add; 
	union uA2 localA2;
	
	reg_add = 110;
	error = I2C_BYTE_READ_DLL (USB_I2CHandle, 0xA2, reg_add, (unsigned char *)(&localA2.sStrA2.status_110)); 
	if (error<0) return -1;
	
	localA2.sStrA2.status_110.Soft_Tx_Dis = SoftTxDis;
	
	reg_add = 110;
	error = I2C_BYTE_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, *(unsigned char *)(&localA2.sStrA2.status_110), 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set SoftTxDis error!"); return -1;}  
	
	return 0; 
}

int MEGA88_SET_DAC_MOD(int USB_I2CHandle, INT8U DAC)
{
	int error, reg_add;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;
	
	localA2table2.sStr.fMODSETDAC = DAC; 
	reg_add =(int)(&localA2table2.sStr.fMODSETDAC) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set MOD DAC error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_DAC_APC(int USB_I2CHandle, INT8U DAC)
{
	int error, reg_add;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;
	
	localA2table2.sStr.fAPCSETDAC = DAC; 
	reg_add =(int)(&localA2table2.sStr.fAPCSETDAC) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set APC DAC error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_VSCtoFLASH(int USB_I2CHandle, int VSC[26])
{
	int error, reg_add;
	union uA2table1 localA2table1;
	int i;
	
	error = MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	reg_add =(int)(&localA2table1.sStr.fMODSETDAC) - (int)(&localA2table1.sStr.FirstByte);
	for (i=0; i<26; i++)
	{
		localA2table1.pStr[reg_add+i]=VSC[i];
	}
	
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 26, localA2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set VSC to FLASH error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_TX_PWR(int USB_I2CHandle, INT16S gain, INT16S offset)
{
	int error, reg_add;
	unsigned char myARR[256];  

	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	myARR[128+T3_TX_PWR_G]     = gain >> 8;
	myARR[128+T3_TX_PWR_G+1]   = gain & 0xFF;
	myARR[128+T3_TX_PWR_OFS]   = offset >> 8;
	myARR[128+T3_TX_PWR_OFS+1] = offset & 0xFF;
	
	reg_add = 128 + T3_TX_PWR_G;  
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 4, myARR, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set TX_PWR error!"); return -1;} 
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_RX_PWR(int USB_I2CHandle, INT16S gain, INT16S offset)
{
	int error, reg_add;
	unsigned char myARR[256];  

	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	myARR[128+T3_RX_PWR_G]     = gain >> 8;
	myARR[128+T3_RX_PWR_G+1]   = gain & 0xFF;
	myARR[128+T3_RX_PWR_OFS]   = offset >> 8;
	myARR[128+T3_RX_PWR_OFS+1] = offset & 0xFF;
	
	reg_add = 128 + T3_RX_PWR_G;  
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 4, myARR, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set RX_PWR error!"); return -1;} 
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_RX_MON(int USB_I2CHandle, INT16U	*ADC, double *Rx_Mon)
{
	int error, i;
	unsigned char myARR[256];  
	union uA2 localA2;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;

	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	} 

	*ADC= localA2.sStrA2.rx_Power;
	*Rx_Mon = 10. * log10(localA2.sStrA2.rx_Power * 0.1 / 1000.);
		
	return 0;
}

int MEGA88_GET_TX_MON(int USB_I2CHandle, INT16U	*ADC, double *Tx_Mon)
{
	int error, i;
	unsigned char myARR[256];  
	union uA2 localA2;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	} 

	*ADC = localA2.sStrA2.tx_Power;
	*Tx_Mon = 10. * log10(localA2.sStrA2.tx_Power * 0.1 / 1000.);
		
	return 0;
}

int MEGA88_SET_BURSTCTRL (int USB_I2CHandle, BYTE BURST)
{
	int error, reg_add;
	union uA2table1 localA2table2;
	union uA2table1 localA2table1;
	
	//写RAM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;
	
	localA2table2.sStr.fBURSTCTRL = BURST;
	reg_add =(int)(&localA2table2.sStr.fBURSTCTRL) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set burst mode error!"); return -1;}
	
	//为保险起见 同时写EEPROM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table1.sStr.fBURSTCTRL = BURST;
	reg_add =(int)(&localA2table1.sStr.fBURSTCTRL) - (int)(&localA2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set burst mode error!"); return -1;}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_fOVERD (int USB_I2CHandle, BYTE fOVERD)
{
	int error, reg_add;
	union uA2table1 localA2table2;
	union uA2table1 localA2table1;
	
	//写RAM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;
	
	localA2table2.sStr.fOVERD = fOVERD;
	reg_add =(int)(&localA2table2.sStr.fOVERD) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set burst mode error!"); return -1;}
	
	//为保险起见 同时写EEPROM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table1.sStr.fOVERD = fOVERD;
	reg_add =(int)(&localA2table1.sStr.fOVERD) - (int)(&localA2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set burst mode error!"); return -1;}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_DAC_APC(int USB_I2CHandle, INT8U *DAC)
{
	int error, i;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;

	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, localA2table2.pStr); 
	if (error<0) return -1;
	
	*DAC = localA2table2.sStr.fAPCSETDAC;  
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_DAC_MOD(int USB_I2CHandle, INT8U *DAC)
{
	int error, i;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;

	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, localA2table2.pStr); 
	if (error<0) return -1;
	
	*DAC = localA2table2.sStr.fMODSETDAC;  
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_LUT_MOD(int USB_I2CHandle, BYTE MOD[81])
{
	int error, i;
	unsigned char myARR[256]; 
	
	for (i=0; i<81; i++)
	{
		myARR[128+i] = MOD[i];
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 4);
	if (error) return -1;
	
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set MOD LUT error!"); return -1;} 
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<81; i++)
	{
		if (myARR[128+i] != MOD[i])
		{MessagePopup ("ERROR", "MEGA88 set MOD LUT error!");break;}
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_LUT_APC(int USB_I2CHandle, BYTE APC[81])
{
	int error, i;
	unsigned char myARR[256]; 
	
	for (i=0; i<81; i++)
	{
		myARR[128+i] = APC[i];
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 5);
	if (error) return -1;
	
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set APC LUT error!"); return -1;} 
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<81; i++)
	{
		if (myARR[128+i] != APC[i])
		{MessagePopup ("ERROR", "MEGA88 set APC LUT error!");break;}
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_Temperatrue(int USB_I2CHandle, double *Temperatrue)
{
	unsigned char myARR[256]; 
	int error, i;
	float slope, offset;
	union uA2 localA2;
		
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	} 
	
	slope = localA2.sStrA2.tempSlope/256.;
	offset= localA2.sStrA2.tempOffset;
	*Temperatrue = (slope * localA2.sStrA2.temperature + offset) * (1/256.);
	
	return 0;
}

int MEGA88_GET_Ibias(int USB_I2CHandle, double *Ibias)
{
	unsigned char myARR[256]; 
	int error, i;
	float slope, offset;
	union uA2 localA2;
		
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	} 
	
	slope = localA2.sStrA2.tx_ISlope/256.;
	offset= localA2.sStrA2.tx_IOffset;
	*Ibias = (slope * localA2.sStrA2.tx_Bias + offset) * (2.E-3); 
				   
	return 0;
}

int MEGA88_GET_TX_PWR (int USB_I2CHandle, INT16S *gain, INT16S *offset)
{
	unsigned char myARR[256];  
	int error;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	*gain  = (myARR[128+T3_TX_PWR_G] << 8) + myARR[128+T3_TX_PWR_G+1];
	*offset= (myARR[128+T3_TX_PWR_OFS]<<8) + myARR[128+T3_TX_PWR_OFS+1];

	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_RX_PWR (int USB_I2CHandle, INT16S *gain, INT16S *offset)
{
	unsigned char myARR[256];  
	int error;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	*gain  = (myARR[128+T3_RX_PWR_G] << 8) + myARR[128+T3_RX_PWR_G+1];
	*offset= (myARR[128+T3_RX_PWR_OFS]<<8) + myARR[128+T3_RX_PWR_OFS+1];

	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_RX_GAIN(int USB_I2CHandle, INT8U gain)
{
	int error, reg_add;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table2.sStr.RxInnerGain = gain; 
	reg_add =(int)(&localA2table2.sStr.RxInnerGain) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set MOD DAC error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_FirmwareVer(int USB_I2CHandle, char *firmwarever)
{
	int error, i;
	unsigned char myARR[256]; 
	union uA2 localA2;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	}

	sprintf(firmwarever, "%c.%c", localA2.sStrA2.version+0x30, 0+0x30);

	return 0;
}

int MEGA88_SET_RX_PWR_CaliPo(int USB_I2CHandle, INT8U RxCaliPo)
{
	int error, reg_add;  
	union uA2table3 localM8_A2table3;

	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	localM8_A2table3.sStr.RxCaliPo = RxCaliPo; 
	reg_add =(int)(&localM8_A2table3.sStr.RxCaliPo) - (int)(&localM8_A2table3.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localM8_A2table3.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set RxCaliPo error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_RX_PWR_CaliNo(int USB_I2CHandle, INT8U RxCaliNo)
{
	int error, reg_add;
	union uA2table3 localM8_A2table3;

	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	localM8_A2table3.sStr.RxCaliNo = RxCaliNo; 
	reg_add =(int)(&localM8_A2table3.sStr.RxCaliNo) - (int)(&localM8_A2table3.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, localM8_A2table3.pStr, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set RxCaliNo error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_RX_PWR_MultiSegmentFit (int USB_I2CHandle, INT8U RxCaliNo, INT16U *RX_PWR_ADC, struct M8_RxCaliData_DEF *RxCaliData)
{
	int error, reg_add, i, reg_add_adc, reg_add_gain;
	unsigned char myARR[256], myARR_r[256]; 
	union uA2table3 localM8_A2table3;

	if (RxCaliNo>16)
	{MessagePopup ("ERROR", "MEGA88不能支持超过16个点的收端分段拟合"); return -1;} 
	
	memset (myARR, 0, 256);
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	reg_add_adc  = (int)(&localM8_A2table3.sStr.RX_PWR_ADC) - (int)(&localM8_A2table3.sStr.FirstByte); 
	reg_add_gain = (int)(&localM8_A2table3.sStr.RxCaliData) - (int)(&localM8_A2table3.sStr.FirstByte);
	for	(i=0; i<RxCaliNo-1; i++)
	{
		myARR[reg_add_adc+i*2]  = RX_PWR_ADC[i] >> 8; 
		myARR[reg_add_adc+i*2+1]= RX_PWR_ADC[i] & 0xFF;  
		
		myARR[reg_add_gain+i*4]  = RxCaliData[i].K >> 8; 
		myARR[reg_add_gain+i*4+1]= RxCaliData[i].K & 0xFF;  
		myARR[reg_add_gain+i*4+2]= RxCaliData[i].B >> 8; 
		myARR[reg_add_gain+i*4+3]= RxCaliData[i].B & 0xFF;  
	}
	
	//补充最后一个校准点
	myARR[reg_add_adc+i*2]  = RX_PWR_ADC[i] >> 8; 
	myARR[reg_add_adc+i*2+1]= RX_PWR_ADC[i] & 0xFF;  

	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add_adc, 96, myARR, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set RxCaliNo error!"); return -1;} 
	
	//回读检查
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR_r); 
	if (error<0) return -1;
	
	for	(i=reg_add_adc; i<256; i++)
	{
		if (myARR[i] != myARR_r[i]) 
		{MessagePopup ("ERROR", "MEGA88 check MultiSegmentFit RxCaliData error!"); return -1;}
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_FirmwareVersion (int USB_I2CHandle, INT8U *version)
{
	int error, i;
	unsigned char myARR[256]; 
	union uA2 localA2;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<256; i++)
	{
		localA2.pStrA2[i] = myARR[255-i];
	}

	*version = localA2.sStrA2.version;

	return 0;
}

int MEGA88_SET_LUT_APC_PL(int USB_I2CHandle, BYTE DAC_APC, int powerlevel)
{
	int error;
	unsigned char myARR[256]; 

	//table5
	error =  MEGA88_SET_Table (USB_I2CHandle, 5);
	if (error) return -1;
	
	if (powerlevel==0)
	{
		myARR[253] = DAC_APC;
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 253, 1, myARR, 0.05); 
		if (error) {MessagePopup ("Error", "powerlevel APC set error");return -1;} 
	}
	else if (powerlevel==1)
	{
		myARR[254] = DAC_APC;
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 254, 1, myARR, 0.05); 
		if (error) {MessagePopup ("Error", "powerlevel APC set error");return -1;} 
	}
	else if (powerlevel==2)
	{
		myARR[255] = DAC_APC;
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 255, 1, myARR, 0.05); 
		if (error) {MessagePopup ("Error", "powerlevel APC set error");return -1;}
	}
	else 
	{  
		MessagePopup ("Error", "APC PL的设置值错误");
		return -1;
	}

	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_LUT_MOD_PL(int USB_I2CHandle, BYTE DAC_MOD[81], int powerlevel)
{
	int error, i;
	unsigned char myARR[256]; 

	if (powerlevel==0)
	{
		for (i=0; i<81; i++)
		{
			myARR[128+i] = DAC_MOD[i];
		}
	
		//for table4
		error =  MEGA88_SET_Table (USB_I2CHandle, 4);
		if (error) return -1;
	
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
		if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<81; i++)
		{
			if (myARR[128+i] != DAC_MOD[i])
			{MessagePopup ("Error", "powerlevel MOD set error"); return -1;}
		}
	}
	else if (powerlevel==1)
	{
		for (i=0; i<81; i++)
		{
			myARR[128+i] = DAC_MOD[i];
		}
	
		//for table5
		error =  MEGA88_SET_Table (USB_I2CHandle, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
		if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<81; i++)
		{
			if (myARR[128+i] != DAC_MOD[i])
			{MessagePopup ("Error", "powerlevel MOD set error"); return -1;}
		}
	}
	else if (powerlevel==2)
	{
		//for table4
		for (i=0; i<40; i++)
		{
			myARR[209+i] = DAC_MOD[i];
		}
	
		error =  MEGA88_SET_Table (USB_I2CHandle, 4);
		if (error) return -1;
	
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 209, 40, myARR, 0.2);
		if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 209, 40, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<40; i++)
		{
			if (myARR[209+i] != DAC_MOD[i])
			{MessagePopup ("Error", "powerlevel MOD set error"); return -1;}
		}
		
		//for table5
		for (i=0; i<41; i++)
		{
			myARR[209+i] = DAC_MOD[i+40];
		}
	
		error =  MEGA88_SET_Table (USB_I2CHandle, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 209, 41, myARR, 0.2);
		if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 209, 41, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<41; i++)
		{
			if (myARR[209+i] != DAC_MOD[i+40])
			{MessagePopup ("Error", "powerlevel MOD set error"); return -1;}
		}
	}
	else 
	{  
		MessagePopup ("Error", "MOD PL的设置值错误");
		return -1;
	}

	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_LUT_APD(int USB_I2CHandle, BYTE APD[81])
{
	int error, i;
	unsigned char myARR[256]; 
	
	for (i=0; i<81; i++)
	{
		myARR[128+i] = APD[i];
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 6);
	if (error) return -1;
	
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set APD LUT error!"); return -1;} 
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;
	
	for (i=0; i<81; i++)
	{
		if (myARR[128+i] != APD[i])
		{MessagePopup ("ERROR", "MEGA88 set APD LUT error!");break;}
	}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_TRxConfigFlag (int USB_I2CHandle, struct sTRxConfig_DEF *TRxConfigFlag)
{
	int error;
	union uA2table1 localA2table1;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, localA2table1.pStr); 
	if (error<0) return -1;
	
	*TRxConfigFlag = localA2table1.sStr.sTRxConfig; 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_LUT_MOD_PL(int USB_I2CHandle, BYTE DAC_MOD[81], int powerlevel)
{
	int error, i;
	unsigned char myARR[256]; 

	if (powerlevel==0)
	{
		//for table4
		error =  MEGA88_SET_Table (USB_I2CHandle, 4);
		if (error) return -1;
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<81; i++)
		{		  
			DAC_MOD[i] = myARR[128+i];
		}
	}
	else if (powerlevel==1)
	{
		//for table5
		error =  MEGA88_SET_Table (USB_I2CHandle, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, 128, 81, myARR, 0.2);
		if (error) {MessagePopup ("Error", "powerlevel MOD set error"); return -1;} 
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<81; i++)
		{		  
			DAC_MOD[i] = myARR[128+i];
		}
	}
	else if (powerlevel==2)
	{
		//for table4
		error =  MEGA88_SET_Table (USB_I2CHandle, 4);
		if (error) return -1;
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 209, 40, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<40; i++)
		{		  
			DAC_MOD[i] = myARR[209+i];
		}
		
		//for table5
		error =  MEGA88_SET_Table (USB_I2CHandle, 5);
		if (error) return -1;
	
		error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 209, 41, myARR); 
		if (error<0) return -1;
	
		for (i=0; i<41; i++)
		{		  
			DAC_MOD[i+40] = myARR[209+i];
		}
	}
	else 
	{  
		MessagePopup ("Error", "MOD PL的设置值错误");
		return -1;
	}

	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_BIASDAC(int USB_I2CHandle, INT8U *DACM, INT8U *DACL)
{
	int error;
	union uA2table1 localA2table2;

	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;

	error = I2C_BYTEs_READ_DLL (USB_I2CHandle, 0xA2, 0, 256, localA2table2.pStr); 
	if (error<0) {MessagePopup ("ERROR", "MEGA88 get bias dac error!"); return -1;}   
	
	*DACM = localA2table2.sStr.fBIASDACM;  
	*DACL = localA2table2.sStr.fBIASDACL_CHIPID; 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_BIASPRELOAD(int USB_I2CHandle, INT8U DACM, INT8U DACL)
{
	int error, reg_add; 
	union uA2table1 localA2table1;
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table1.sStr.fBIASPRELOADM = DACM; 
	localA2table1.sStr.fBIASPRELOADL = DACL; 
	
	reg_add =(int)(&localA2table1.sStr.fBIASPRELOADM) - (int)(&localA2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 2, localA2table1.pStr, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set bias preload dac error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;  	
}

int MEGA88_SET_APC_DAC120(int USB_I2CHandle, INT8U DAC)
{
	int error, reg_add=0xFA; 
	unsigned char myARR[256];   
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 5);
	if (error) return -1;
	
	myARR[reg_add] = DAC; 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 1, myARR, 0.2);
	if (error) {MessagePopup ("ERROR", "MEGA88 set memery error!"); return -1;} 
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;  	
}

int MEGA88_SET_A2TemperCompens (int USB_I2CHandle, float k, float b)
{
	int error, reg_add;
	unsigned char myARR[256];  
	
	error = MEGA88_SET_Table (USB_I2CHandle, 3);
	if (error) return -1;
	
	reg_add = T3_TEMP1_MON_OFS+0x80;
	myARR[reg_add] 	 = (int)(b*256) >>8; 
	myARR[reg_add+1] = (int)(b*256) & 0xFF;
	
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 2, myARR, 0.2);
	if (error) {MessagePopup ("Error", "MEGA88 set a2 temperature compens error!"); return -1;} 
	
	error = MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0; 	
}

int MEGA88_SET_fLOS (int USB_I2CHandle, BYTE THADAC, BYTE THDDAC)
{
	int error, reg_add;
	union uA2table1 localA2table2;
	union uA2table1 localA2table1;
	
	//写RAM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 2);
	if (error) return -1;
	
	localA2table2.sStr.fLOSTHADAC = THADAC;
	localA2table2.sStr.fLOSTHDDAC = THDDAC;
	
	reg_add =(int)(&localA2table2.sStr.fLOSTHADAC) - (int)(&localA2table2.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 2, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set fLOSTHADAC error!"); return -1;}
	
	//为保险起见 同时写EEPROM  
	error =  MEGA88_SET_Table (USB_I2CHandle, 1);
	if (error) return -1;
	
	localA2table1.sStr.fLOSTHADAC = THADAC;
	localA2table1.sStr.fLOSTHDDAC = THDDAC;

	reg_add =(int)(&localA2table1.sStr.fLOSTHADAC) - (int)(&localA2table1.sStr.FirstByte); 
	error = I2C_BYTEs_WRITE_DLL (USB_I2CHandle, 0xA2, reg_add, 2, localA2table1.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set fLOSTHADAC error!"); return -1;}
	
	error =  MEGA88_SET_Table (USB_I2CHandle, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_A2TemperKBl (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	union uA2table1 localA2table2;
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T6_A2_TEMP_K1;
	
	localA2table2.pStr[reg_add]   = gain >> 8;
	localA2table2.pStr[reg_add+1] = gain & 0xFF;
	localA2table2.pStr[reg_add+2] = offset >> 8;
	localA2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}
	  
int MEGA88_SET_A2TemperKBh (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	union uA2table1 localA2table2;
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T6_A2_TEMP_K2;
	
	localA2table2.pStr[reg_add]   = gain >> 8;
	localA2table2.pStr[reg_add+1] = gain & 0xFF;
	localA2table2.pStr[reg_add+2] = offset >> 8;
	localA2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_A2TemperKB_Index (int inst_I2C, float k, float b)
{
	signed short gain, offset;
	int error, reg_add; 
	union uA2table1 localA2table2;
	
	gain = (signed short int)(k * 256.);
	offset = (signed short int)(b * 256.); 
	
	reg_add = 128+T3_TEMP_MON_G;
	
	localA2table2.pStr[reg_add]   = gain >> 8;
	localA2table2.pStr[reg_add+1] = gain & 0xFF;
	localA2table2.pStr[reg_add+2] = offset >> 8;
	localA2table2.pStr[reg_add+3] = offset & 0xFF;

	error =  MEGA88_SET_Table (inst_I2C, 3);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 4, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_SET_A2Temper (int inst_I2C, float val)
{
	signed short t0;
	int error, reg_add; 
	union uA2table1 localA2table2;
	
	t0 = (signed short int)(val * 256.);

	reg_add = 128+T6_A2_TEMP_T0;
	
	localA2table2.pStr[reg_add]   = t0 >> 8;
	localA2table2.pStr[reg_add+1] = t0 & 0xFF;

	error =  MEGA88_SET_Table (inst_I2C, 6);
	if (error) return -1;

	error = I2C_BYTEs_WRITE_DLL (inst_I2C, 0xA2, reg_add, 2, localA2table2.pStr, 0.1);
	if (error) {MessagePopup ("ERROR", "MEGA88 set nt25l90 register to eeprom error!"); return -1;} 
	
	error =  MEGA88_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	return 0;
}

int MEGA88_GET_Temper_Index (int inst_I2C, float *val)
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

int Mega88_Get_fOVERD(int inst_I2C, BYTE *val)
{
	int error;
	union uA2table1 localA2table1;
	
	//读EEPROM  
	error =  MEGA88_SET_Table (inst_I2C, 1);
	if (error) return -1;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, localA2table1.pStr); 
	if (error<0) {MessagePopup ("ERROR", "MEGA88 get bias dac error!"); return -1;}   
	
	error =  MEGA88_SET_Table (inst_I2C, 0);
	if (error) return -1;
	
	*val = localA2table1.sStr.fOVERD;
		
	return 0;
}

int MEGA88_GET_FirmwareVer2 (int USB_I2CHandle, char *version)
{
	int error;
	INT8U	firmversion;
	
	error = MEGA88_GET_FirmwareVersion (USB_I2CHandle, &firmversion); 
	if (error)
	{
		strcpy(version, "");
		return -1;	
	}
	sprintf(version, "%d.0", firmversion); 

	return 0;
}

int MEGA88GetSFRM50TemperIndex (int inst_I2C, float *val)
{
	int error, i;
	unsigned char myARR[256]; 
	signed short int temp;
	union uA2table0 localSFRM0050_A2;
	
	error = I2C_BYTEs_READ_DLL (inst_I2C, 0xA2, 0, 256, myARR); 
	if (error<0) return -1;

	for (i=0; i<256; i++)
	{
		localSFRM0050_A2.pStr[i] = myARR[255-i];
	} 
	
	temp = (signed short int) (((0x00FF & localSFRM0050_A2.sStr.reserved_2_m) << 8 ) + (0x00FF & localSFRM0050_A2.sStr.reserved_2_l));  
	*val = (float) (temp/256.);

	return 0;
}
