/****************************************************************************
 *
 * File:                nt25l90.h
 *
 * Author:              Superxon(Roger Li)
 *
 * Description:         Nt25L90功能代码
 *
 * Time:                2011-07-02
 *
 * Version:				V1.0.0.0

NT25L90工作模式说明
用户模式：上电，NT25L90的寄存器读取NVM的值到RAM寄存器，然后从mega88 table1（EEPROM）对应寄存器地址取值更新NT25L90的RAM寄存器
工厂模式：上电，NT25L90的寄存器读取NVM的值到RAM寄存器

mega88的table1和table2
table1： 对应nt25l90的EEPORM值，用户模式下NT25L90上电后最终RAM寄存器的值
table2： 对应Nt25L90的RAM寄存器的值，可读可写

NVM保存命令：发出NVM保存命令后，将当前RAM寄存器中的值写入NVM，NVM写次数不能超过8次
****************************************************************************/

#ifndef _NT25L90_H_
#define _NT25L90_H_

#include <windows.h> 
#include "CH341A_DLL.h" 

#pragma pack (1)  

//=========================================================================//
struct sTRxConfig_DEF_Nt25L90
{
	INT8U isTxBurstHighActive_flag:1;	/* bit0=1, BEN high Tx on; =0, BEN low Tx on */
	INT8U isTxTriggeredByINT_flag:1;	/* bit1=1, TxPWR start sample is triggered by BEN interrupt; =0, cycling sampled in main-loop */
	INT8U isTxSampledByTXMON_flag:1;	/* bit2=1, TxPWR ADC is from mega88 TXMON ADC; =0, TxPWR ADC is from 796x Aout ADC */ 
	INT8U isRxSampledByRXMON_flag:1;	/* bit3=1, RxPWR ADC is from mega88 RXMON ADC; =0, RxPWR ADC is from 796x Aout ADC */
	INT8U isRxMultiSegmentFit_flag:1;	/* bit4=1, RxPWR multi-segment fit for APD; =0, single-segment fit for PIN */
	INT8U isVapdTunedByPWM_flag:1;		/* bit5=1, Vapd is tuned by PWM; =0, Vapd is not tuned by PWM */ 
	INT8U isA0Protected_flag:1;			/* bit6=1, A0 protected as ZTE wanted; =0, A0 not protected */ 
	INT8U isTxPowerLeveling:1;			/* bit7=1, TxPowerLeveling supported; =0, not supported */		
};

struct strNt25l90A2table1
{
	INT8U   FirstByte;				   
	INT8U   Reserved1to127[127];       
	//FLASH
	//NT25L90 RAM
	INT8U   fTXSET0;	                // A2H[128]  0x00      R/W	   NT25L90 reg[60h] 
	INT8U   fTXSET1; 	                // A2H[129]  0x01      R/W	   NT25L90 reg[61h] 
	INT8U   fIMODSET;	                // A2H[130]  0x02      R/W	   NT25L90 reg[62h] 
	INT8U   fTCSTART;	                // A2H[131]  0x03      R/W	   NT25L90 reg[63h] 
	INT8U   fTSLOPE;	                // A2H[132]  0x04      R/W	   NT25L90 reg[64h] 
	INT8U   fSPARE;	                    // A2H[133]  0x05      R/W	   NT25L90 reg[65h] 
	INT8U   fAPCSET;	                // A2H[134]  0x06      R/W	   NT25L90 reg[66h] 
	INT8U   fBIASINIT;	                // A2H[135]  0x07      R/W	   NT25L90 reg[67h] 
	INT8U   fAPCCTRL0;	                // A2H[136]  0x08      R/W	   NT25L90 reg[68h] 
	INT8U   fMDMAX;	                    // A2H[137]  0x09      R/W     NT25L90 reg[69h] 
	INT8U   fBIASMAX;	                // A2H[138]  0x0A      R/W     NT25L90 reg[6Ah] 
	INT8U   fRXSET0;	                // A2H[139]  0x0B      R/W	   NT25L90 reg[6Bh] 
	INT8U   fRXSET1;	                // A2H[140]  0x0C      R/W	   NT25L90 reg[6Ch] 
	INT8U   fLOSLEVEL;	                // A2H[141]  0x0D      R/W	   NT25L90 reg[6Dh] 
	INT8U   fCONTROL0;	                // A2H[142]  0x14      R/W	   NT25L90 reg[B0h]
	INT8U   fBIASDAC_MSB;	            // A2H[143]  0x15      R/W	   NT25L90 reg[B1h]
	INT8U   fBIASDAC_LSB;	            // A2H[144]  0x16      R/W	   NT25L90 reg[B2h]
	INT8U   fWATCHDOG;	                // A2H[145]  0x17      R/W	   NT25L90 reg[B3h]
	INT8U   fSTATUS0;	                // A2H[146]  0x18      R/W	   NT25L90 reg[B4h]
	INT8U   fSTATUS1;	                // A2H[147]  0x19      R	   NT25L90 reg[B5h]
	INT8U   fCONTROL1;	                // A2H[148]  0x1A      R/W	   NT25L90 reg[B6h]
	INT8U   Reserved149to154[6];
	INT8U   APDPWM;               	    // A2H[155]  0x1B      R/W
	INT8U   PowerLevel_offset;	        // A2H[156]  0x1C      R/W
	INT8U   FactoryFlag;               	// A2H[157]  0x1D      R/W
	struct  sTRxConfig_DEF_Nt25L90 sTRxConfig; 	// A2H[158]  0x1E      R/W
	//only table2 have extend NT25L90 registers
    INT8U   fTEMP_MSB;					// A2H[159]  0x1F      R
    INT8U   fTEMP_LSB;					// A2H[160]  0x20      R
    INT8U   fTX_POWER_MSB;				// A2H[161]  0x21      R
    INT8U   fTX_POWER_LSB;				// A2H[162]  0x22      R
    INT8U   fTX_BIAS_MSB;				// A2H[163]  0x23      R
    INT8U   fTX_BIAS_LSB;				// A2H[164]  0x24      R
    INT8U   fTX_MOD_MSB;				// A2H[165]  0x25      R
    INT8U   fTX_MOD_LSB;				// A2H[166]  0x26      R
    INT8U   fRX_POWER_MSB;				// A2H[167]  0x27      R
    INT8U   fRX_POWER_LSB;				// A2H[168]  0x28      R
	INT8U   fVDD_TX_MSB;	            // A2H[169]  0x0E      R
	INT8U   fVDD_TX_LSB;	            // A2H[170]  0x0F      R
	INT8U   fVDD_RX_MSB;	            // A2H[171]  0x10      R  
	INT8U   fVDD_RX_LSB;	            // A2H[172]  0x11      R  
	INT8U   fVDD_DIG_MSB;	            // A2H[173]  0x12      R
	INT8U   fVDD_DIG_LSB;	            // A2H[174]  0x13      R
    INT8U   fUSER_NVM_PASS;				// A2H[175]  0x29      R/W
    INT8U   fUSER_NVM_PRGM;   			// A2H[176]  0x2A      R/W
	INT8U   Reserved176to255[80];       
};										  
										  
union uNt25l90A2table1							  			  
{ 
	struct strNt25l90A2table1 sStr;				  
	INT8U  pStr[256];						  
};										  

#pragma pack ()   

union uNt25l90A2table1 Nt25l90A2table1,Nt25l90A2table2; 				  

#define T3_TEMP_MON_G           0x00    /* 80H - 81H - temperature monitor calibration gain */
#define T3_TEMP_MON_OFS         0x02    /* 82H - 83H - temperature monitor calibration offset */
#define T3_VCC_MON_G            0x04    /* 84H - 85H - supply monitor calibration gain */
#define T3_VCC_MON_OFS          0x06    /* 86H - 87H - supply monitor calibration offset */
#define T3_TX_BIAS_G            0x08    /* 88H - 89H - Tx bias monitor calibration gain */
#define T3_TX_BIAS_OFS          0x0A    /* 8AH - 8BH - Tx bias monitor calibration offset */
#define T3_TX_PWR_G             0x0C    /* 8CH - 8DH - Tx power monitor calibration gain */
#define T3_TX_PWR_OFS           0x0E    /* 8EH - 8FH - Tx power monitor calibration offset */
#define T3_RX_PWR_G             0x10    /* 90H - 91H - Rx power monitor calibration gain */
#define T3_RX_PWR_OFS           0x12    /* 92H - 93H - Rx power monitor calibration offset */
//#define T3_TEMP1_MON_G          0x14    /* 94H - 95H - core temperature monitor calibration gain */
//#define T3_TEMP1_MON_OFS        0x16    /* 96H - 97H - core temperature monitor calibration offset */

//table6 A2 temperature calibration parameters
#define T6_A2_TEMP_T0           81//=209-128
#define T6_A2_TEMP_K1           83//=211-128
#define T6_A2_TEMP_B1           85//=213-128
#define T6_A2_TEMP_K2           87//=215-128
#define T6_A2_TEMP_B2           89//=217-128

#define M8_RX_CALI_PO			0x1E
#define M8_RX_CALI_NO			0x1F 
#define M8_RX_PWR_ADC0			0x20
#define M8_RX_PWR_K0			0x40
#define M8_RX_PWR_B0			0x42 

int MEGA88NT25_Password1(int inst_I2C);
int MEGA88NT25_Password2(int inst_I2C);
int MEGA88NT25_SET_Table (int inst_I2C, BYTE Table); 
int MEGA88NT25_SET_TRxConfigFlag (int inst_I2C, struct sTRxConfig_DEF_Nt25L90 TRxConfigFlag);
int MEGA88NT25_GET_FactoryFlag (int inst_I2C, int *flag);
int MEGA88NT25_SET_FactoryFlag (int inst_I2C, int flag);
int MEGA88NT25_SET_Reg_EEPROM (int inst_I2C, int reg[14]); 
int MEGA88NT25_GET_Reg_RAM (int inst_I2C, INT8U reg[14]);
int MEGA88NT25_SET_Reg_RAM (int inst_I2C, int reg[14]);  
int MEGA88NT25_GET_NVM_count (int inst_I2C, int *num);		//读取NVM已写入次数

int MEGA88NT25_Save_NVM (int inst_I2C);
int MEGA88NT25_SET_Mode_Auto(int inst_I2C);
int MEGA88NT25_SET_Mode_Manual(int inst_I2C);
int MEGA88NT25_SET_LOS_RAM (int inst_I2C, int DAC);
int MEGA88NT25_SET_LOS_EEPROM (int inst_I2C, int DAC); 
int MEGA88NT25_SET_APC_RAM (int inst_I2C, int DAC); 
int MEGA88NT25_SET_MOD_RAM (int inst_I2C, int DAC); 
int MEGA88NT25_GET_MOD_RAM (int inst_I2C, int *DAC);
int MEGA88NT25_SET_LUT_APC (int inst_I2C, BYTE DAC);
int MEGA88NT25_SET_LUT_MOD (int inst_I2C, BYTE DAC[81]); 
int MEGA88NT25_SET_A2TemperKBl (int inst_I2C, float k, float b);	//设置A2温度监控校准系数低温段 
int MEGA88NT25_SET_A2TemperKBh (int inst_I2C, float k, float b);	//设置A2温度监控校准系数高温段
int MEGA88NT25_SET_A2TemperKB_Mon (int inst_I2C, float k, float b); //设置内部温度监控校准系数
int MEGA88NT25_SET_A2Temper (int inst_I2C, float val);  	//设置温度分界点
int MEGA88NT25_GET_A2Temper_Mon (int inst_I2C, float *val);	//读取内部温度监控值
int MEGA88NT25_GET_Temper_Index (int inst_I2C, float *val);	//读取LUT温度监控值 

int MEGA88NT25_SET_fAPCCTRL0 (int inst_I2C, BYTE Data); 
int MEGA88NT25_SET_fBIASDAC_MSB (int inst_I2C, BYTE MSB);
int MEGA88NT25_SET_fBIASDAC_LSB (int inst_I2C, BYTE LSB);

#endif




























