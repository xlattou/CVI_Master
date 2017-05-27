/****************************************************************************
 *
 * File:                EVB5.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         EVB5头文件
 *
 * Time:                2011-06-09
 *
 * version:				V1.2.0.4
 * 
版本历史 
V1.2.0.4
1.
	INT8U   reserved56to73[18];		// F0H[56~73], 18byte 
改为
	INT16U  ADC15_SC1_MON;			// F0H[56~57], 2byte
	INT16U  ADC15_SC2_MON;			// F0H[58~59], 2byte
	float   Iapd_compensate;		// F0H[60~63], 4byte
	float   IT_compensate;			// F0H[64~67], 4byte
	float   IR_compensate;			// F0H[68~71], 4byte
	struct  sTxSD_Detect_DEF sTxSD_Detect;  //F0H[72], 1byte
	INT8U   reserved73[1];			// F0H[73], 1byte
2. 增加函数	
EVB5_SET_TxSD_Detect
EVB5_READ_TxSD
EVB5_CLEAR_TxSD
EVB5_READ_FirmwareVersion

V1.2.0.3
1、添加函数EVB5_SET_VAPD_10V
2、sBERT码型定义PRBS21改为PRBS23

V1.2.0.2
1. 增加函数EVB5_SBERT_Close
1. 增加函数EVB5_SBERT_Open  

V1.2.0.1
增加模块电压拉偏设置函数

V1.2.0.0 
增加函数
MEGA88_Programming_Enable
MEGA88_Programming_Disable
EVB5_GET_SPI_MEGA88

V1.1.0.6 
1. 添加定义
#define EVB5_BEN_MODE_L1L2		7
2. enum emum_SBERT_Pattern中的元素SBERT_P_PRBS23改为SBERT_P_PRBS21

V1.1.0.5 
添加定义
#define EVB5_BEN_MODE_L2L3		6

V1.1.0.4
码型和速率枚举变量名前加e
SBERT_Pattern -> eSBERT_Pattern
SBERT_Rate    -> eSBERT_Rate

V1.1.0.3
硬件版本升级到D版本，添加短路检查功能

V1.1.0.2 
更新部分变量定义位置

V1.1.0.1
1.EVB5 C版本定义文件，添加LIV功能函数
2.更新部分ADC DAC定义，具体请参看Jack EVB5底层程序更新说明：EVB5C Tester, V2.0.0.0, 2009/Apr/02
****************************************************************************/
#ifndef _EVB5_H_
#define _EVB5_H_

#include "CH341A_DLL.h"
#include <windows.h>  

#define	EVB5_Addr  0x84
#define SBERT_Addr 0x80

#pragma pack (1) /* set maximum alignment to 1 */

//////////////////////////////////////////////////////////////
/*DAC*/
/////////////////////////////////////////////////////////////////

union uDAC										  
{	INT8U  DAC_LUT[12][128];
	INT16U DAC_ARR[4][192];
};
union uDAC myDAC;
//////////////////////////////////////////////////////////////
/*F0*/
/////////////////////////////////////////////////////////////////
struct KB_DEF
{
  float K;	     
  float B; 
};
   
struct sInput_DEF
{
INT8U TOSA_FAIL:1;	// ROSA, P1.4
INT8U ONU_RxSD:1;	// ONU,  P1.5
INT8U ONU_TxFAULT:1;// ONU,  P1.6
INT8U ONU_TxSD:1;	// ONU,  P0.4
INT8U OLT_RxLOS:1;	// OLT,  P1.2
INT8U OLT_TxFAULT:1;// ONU,  P0.5
INT8U ROSA_LOS:1;	// ROSA, X5
INT8U MOD_DEF0:1;	// SFP,  X7
};

struct sOutput_DEF
{
INT8U SHDN_VTR:1;	// TRx,  P0.7
INT8U SHDN_VOSA:1;	// ROSA, P0.6
INT8U VPD_MON_EN:1; // ROSA, P0.0
INT8U ONU_TxBEN:1;  // ONU,  P1.7, PWM输出或pulse或level
INT8U OLT_Trig:1;	// OLT,  P2.0
INT8U OLT_RxRST:1;	// OLT,  P1.3
INT8U OLT_TxDIS:1;  // OLT,  P0.3
INT8U ONU_TxDIS:1;	// ONU， P0.5
};

struct sFPGA_R7_SC_DEF
{
INT8U SC_GND:1;  // SC_GND， P1.4
INT8U SC_VCC:1;  // SC_VCC,  P1.5
INT8U SC_VAPD:1; // SC_VAPD, P1.6
INT8U BELL:1;    // BELL,    P4.2
INT8U reserved:4;  // 
};

struct sTxSD_Detect_DEF
{
INT8U ENABLE:1;  // Enable Test, 1: Enable, 0: Disable, Self Clear
INT8U Error :1;  // Test Result, 1: Error,  0: No Error
INT8U reserved:6;  // 
};

struct strF0_EVB5
{                       
	INT8U	FirstByte;				// F0H[0], 1byte
	INT8U	FirmwareVersionSuffix;	// F0H[1], 1byte, 
	INT8U	FirmwareVersion;		// F0H[2], 1byte 										
	INT8U	CPLDVersion;			// F0H[3], 1byte,
	INT16U	Core_Voltage;			// F0H[4~5], 2byte 							
	INT16S	Core_Temperature;		// F0H[6~7], 2byte
	struct	sOutput_DEF sOutput;	// F0H[8], 1byte, 
	struct	sInput_DEF  sInput;		// F0H[9], 1byte, 
	INT16U  ADC0_VPD_MON;			// F0H[10~11], 2byte，SEL_MON=0时采Vapd_MON，否则采VapdM_MON                  
	INT16U  ADC1_IPD_MON;			// F0H[12~13], 2byte，APD电流经AD8651转出来的电压                             
	INT16U  ADC2_Vx_MON;			// F0H[14~15], 2byte，SEL_MON=0时采+3.3V_TRx_MON，否则采+5V_MON               
	INT16U  ADC3_ITRx_MON;			// F0H[16~17], 2byte，SEL_MON=0时采IT_MON，否则采IR_MON                       
	INT16U  ADC4_LD_MON;			// F0H[18~19], 2byte，TOSA-LD前向电压                                         
	INT16U  DAC0_SET_VPD;			// F0H[20~21], 2byte，3482或3571的boost-up电压调节                            
	INT16U  DAC1_SET_VTR;			// F0H[22~23], 2byte，VTR电压调节                                             
	INT16U  ADC14_PD_MON;			// F0H[24~25], 2byte，PD管电流电压                                            
	INT16U	ADC15_MON;				// F0H[26~27], 2byte，未用 
	INT16U  ADC_VPD_MON;			// F0H[28~29], 2byte
	INT16U  ADC_IPD_MON;			// F0H[30~31], 2byte
	INT16U  ADC_IR_MON;				// F0H[32~33], 2byte
	INT16U  ADC_IT_MON;				// F0H[34~35], 2byte
	INT16U  ADC_VPDM_MON;			// F0H[36~37], 2byte
	INT16U  ADC_VTRx_MON;			// F0H[38~39], 2byte
	INT16U  ADC_VBUS_MON;			// F0H[40~41], 2byte
	INT16U  ADC_LD_MON;				// F0H[42~43], 2byte
	INT16U  ADC_PD_MON;				// F0H[44~45], 2byte
	INT16U  ADC0_VPDM_MON;			// F0H[46~47], 2byte
	INT16U  DAC_ADN8810_SET;	    // F0H[48~49], 2byte
	INT16U  ADC15_SC_MON;			// F0H[50~51], 2byte
	INT16U  ADC_SC_MON;				// F0H[52~53], 2byte
	struct  sFPGA_R7_SC_DEF sFPGA_R7_SC;// F0H[54],1byte
	INT8U   Vapd_3571LT3482_sel;	// F0H[55], 1byte; =0, 3571, Vapd正比于DAC
	INT16U  ADC15_SC1_MON;			// F0H[56~57], 2byte
	INT16U  ADC15_SC2_MON;			// F0H[58~59], 2byte
	float   Iapd_compensate;		// F0H[60~63], 4byte
	float   IT_compensate;			// F0H[64~67], 4byte
	float   IR_compensate;			// F0H[68~71], 4byte
	struct  sTxSD_Detect_DEF sTxSD_Detect;  //F0H[72], 1byte
	INT8U   FindVbrfromVapdExpect_flag;		//F0H[73], 1byte
	INT8U	BEN_mode;				// F0H[74],    1byte 
	INT8U   RSSI_mode;				// F0H[75],    1byte 
	INT16U  Tune_VPD_MON;			// F0H[76~77], 2byte
	INT8U   VapdAdj_loop;			// F0H[78],    1byte 
	INT8U   VapdAdj_en;				// F0H[79],    1byte
	float	Vapd_expect;			// F0H[80~83], 4byte 
	INT16U  Tune_IPD_MON;			// F0H[84~85], 2byte
	INT8U   IapdAdj_loop;			// F0H[86],    1byte 
	INT8U   IapdAdj_en;				// F0H[87],    1byte
	float	Iapd_expectMIN;			// F0H[88~91], 4byte 
	float	Iapd_expectMAX;			// F0H[92~95], 4byte 
	INT16U	Vapd20V_loop;			// F0H[96~97], 2byte 
	INT16U	Vapd20V_DAC;			// F0H[98~99], 2byte 
	INT8U   Vapd20V_en;				// F0H[100],   1byte
	INT8U	Vbr_3V4V_sel;			// F0H[101],   1byte, =0 Vbr-3V; =1 Vbr-4V;   	
	INT8U	Vbr_3V_en;				// F0H[102],   1byte
	INT8U	Vbr_3V_status;			// F0H[103],   1byte
	INT16U	ADC_Vbr_MON;			// F0H[104~105],2byte
	INT8U   Vapd_30V_en;			// F0H[106],   1byte
	INT8U   Vapd_30V_status;		// F0H[107],   1byte
	INT16U	ADC_V30V_MON;			// F0H[108~109],2byte
	INT16U	myPWMDAT0;				// F0H[110~111],2byte
	INT16U  Tune_VTR_MON;			// F0H[112~113], 2byte
	INT8U   VTRAdj_loop;			// 82H[114],    1byte 
	INT8U   VTRAdj_en;				// 82H[115],    1byte
	float	VTR_expect;				// 82H[116~119], 4byte 
	INT8U   delay1;					// F0H[120],    1byte
	INT8U   delay2;					// F0H[121],    1byte
	INT8U   delay3;					// F0H[122],    1byte
	INT32U  Password;  				// F0H[123~126]   										
	INT8U   LUTIndex;				// F0H[127] 
	INT8U	I2CSlaveAdd2;			// F0H[128], 1byte
	INT8U	I2CSlaveAdd0;			// F0H[129], 1byte
	INT8U	I2CSlaveAdd1;			// F0H[130], 1byte
	INT8U	CheckSumPNSN;			// F0H[131], 1byte
	INT8U   vendorPN[16];			// F0H[132~147], 16byte
	INT8U   vendorSN[16];			// F0H[148~163], 16byte
	INT8U	reserved164to171[8];	// F0H[164~171], 8byte           
	struct	KB_DEF RxVapd_MON_KB;	// F0H[172~179], 8byte           
	struct	KB_DEF VAPD_MON_KB;		// F0H[180~187], 8byte           
	INT8U	reserved188to227[40];	// F0H[188~227], 40byte           
	INT16U	RSSI_PW;				// F0H[228~229], 2byte 
	INT8U	RSSI_polarity;			// F0H[230], 1byte 
	INT8U	Reserved231;			// F0H[231], 1byte 
	float	Vapd_expectMIN;			// F0H[232~235], 4byte 
	float	Vapd_expectMAX;			// F0H[236~239], 4byte 
	float	Vapd_DeltaMAX;			// F0H[240~243], 4byte,
	INT16U	Vapd20V_loop_ini;		// F0H[244~245], 2byte 
	INT16U	Vapd20V_DAC_ini;		// F0H[246~247], 2byte 
	INT8U   Vapd20V_en_ini;			// F0H[248],   1byte
	INT8U	BEN_polarity;			// F0H[249], 1byte 
	INT16U	BEN_PW;					// F0H[250~251], 2byte 
	INT8U	Reserved252to255[4];	// F0H[252~255], 4byte 
};

struct strLIV
{                       						 	
INT8U   FirstByte;                      // 82H[0],       程序更新控制位，=E，表示擦除第0页；=W，表示存储校准数据至Flash中       
INT8U	FirmwareVersionSuffix;			// 82H[1], 1byte,
INT8U	FirmwareVersion;				// 82H[2], 1byte 
INT8U	CPLDVersion;					// 82H[3], 1byte,
INT16U	Core_Voltage;					// 82H[4~5], 2byte
INT16S	Core_Temperature;				// 82H[6~7], 2byte
struct	sOutput_DEF sOutput;			// 82H[8], 1byte,
struct	sInput_DEF  sInput;				// 82H[9], 1byte, 
INT16U  ADCSample_Count;               	// 82H[10~11],   ADC采集所在数据点		
INT8U   Enable_Sample;          		// 82H[12],      采集使能命令，=1，表示使能ADC采集，采集完成后自动清零
INT16U	Run_Count;						// 82H[13~14], 
INT8U	Handle_Busy;					// 82H[15],		 采集及存储完成标志，=1，采集或存储忙中，=0，采集及存储完成
INT8U	Reserved[12];					// 82H[16~27],  12byte,预留
INT8U   Run_Monitor;					// 82H[28],
INT8U   Realtime_ADCEnable;				// 82H[29],	   	 实时采样使能开关	
INT16U	Realtime_ADCDate;				// 82H[30~31],   2byte,实时采样ADC数据
float  	Cali_850nm_K;					// 82H[32~35],	 校准系数值(850nm、1310nm、1490nm的校准K、B值) 
float  	Cali_850nm_B;					// 82H[36~39],
float  	Cali_1310nm_K;					// 82H[40~43],
float  	Cali_1310nm_B;					// 82H[44~47],
float  	Cali_1490nm_K;					// 82H[48~51],
float  	Cali_1490nm_B;					// 82H[52~55],
INT8U	Reserved1[67];					// 82H[56~122],  67byte,预留
INT32U  PassWord;						// 82H[123~126], 密码输入区
INT8U   PageIndex;                      // 82H[127],     页表选择
INT8U	Reserved2[4];					// 82H[128~131], 128byte,预留
INT8U   vendorPN[16];					// 82H[132~147], 16byte
INT8U   vendorSN[16];					// 82H[148~163], 16byte
INT8U	Reserved3[92];					// 82H[164~255], 128byte,预留
};

struct str80 
{                       
INT8U	FirstByte;						// 8EH[0], 1byte //CMU_STATUS；
INT8U	RX_STATUS;						// 8EH[1], 1byte
INT8U   TX_STATUS;						// 8EH[2], 1byte
INT8U   RESERVED1[5];					// 8EH[3~7], 5byte
INT8U   FIFO_STATUS;					// 8EH[8], 1byte 
INT8U   RESERVED2[7];					// 8EH[9~15], 6byte
INT8U   RX_SDU_CTRL;					// 8EH[16], 1byte
INT8U   RX_ASD_CTRL;					// 8EH[17], 1byte
INT8U	TX_SDU_CTRL;					// 8EH[18], 1byte
INT8U	TX_ASD_CTRL;					// 8EH[19], 1byte
INT8U   RESERVED3[3];					// 8EH[20~22], 3byte
INT8U	MODE_CTRL_0;					// 8EH[23], 1byte
INT8U   RESERVED4[5];					// 8EH[24~28], 5byte
INT8U	RX_FIFO_CTRL;					// 8EH[29], 1byte
INT8U	TX_FIFO_CTRL;					// 8EH[30], 1byte
INT8U	RX_FT_MODE;						// 8EH[31], 1byte
INT8U	TX_FT_MODE;						// 8EH[32], 1byte
INT8U	RX_DATA_CTRL; 					// 8EH[33], 1byte
INT8U	TX_DATA_CTRL; 					// 8EH[34], 1byte
INT8U	CMU_RATE_CTRL;					// 8EH[35], 1byte
INT8U	RX_RATE_CTRL; 					// 8EH[36], 1byte
INT8U	TX_RATE_CTRL; 					// 8EH[37], 1byte
INT8U	REFCLK_CTRL;					// 8EH[38], 1byte
INT8U   RESERVED5;						// 8EH[39], 1byte
INT8U	RXIN_CTRL;						// 8EH[40], 1byte
INT8U	RX_OUT_CTRL; 					// 8EH[41], 1byte
INT8U	TXIN_CTRL;						// 8EH[42], 1byte
INT8U	TX_OUT_CTRL;					// 8EH[43], 1byte
INT8U	RXCLK_CTRL;						// 8EH[44], 1byte
INT8U   RESERVED6[13];					// 8EH[45~57], 13byte
INT8U	PATCHK_UD7;						// 8EH[58], 1byte
INT8U	PATCHK_UD6;						// 8EH[59], 1byte
INT8U	PATCHK_UD5;						// 8EH[60], 1byte
INT8U	PATCHK_UD4;						// 8EH[61], 1byte
INT8U	PATCHK_UD3;						// 8EH[62], 1byte
INT8U	PATCHK_UD2;						// 8EH[63], 1byte
INT8U	PATCHK_UD1;						// 8EH[64], 1byte
INT8U	PATCHK_UD0;						// 8EH[65], 1byte
INT8U	PATCHK_TIMER_SET4;				// 8EH[66], 1byte
INT8U	PATCHK_TIMER_SET3;				// 8EH[67], 1byte
INT8U	PATCHK_TIMER_SET2;				// 8EH[68], 1byte
INT8U	PATCHK_TIMER_SET1;				// 8EH[69], 1byte
INT8U	PATCHK_TIMER_SET0;				// 8EH[70], 1byte
INT8U	PATCHK_TIMER_GET4;				// 8EH[71], 1byte
INT8U	PATCHK_TIMER_GET3;				// 8EH[72], 1byte
INT8U	PATCHK_TIMER_GET2;				// 8EH[73], 1byte
INT8U	PATCHK_TIMER_GET1;				// 8EH[74], 1byte
INT8U	PATCHK_TIMER_GET0;				// 8EH[75], 1byte
INT8U   RESERVED7[2];					// 8EH[76~77], 1byte
INT8U	ERRCNT3;						// 8EH[78], 1byte
INT8U	ERRCNT2;						// 8EH[79], 1byte
INT8U	ERRCNT1; 						// 8EH[80], 1byte
INT8U	ERRCNT0;						// 8EH[81], 1byte
INT8U	PATCHK_CTRL;					// 8EH[82], 1byte
INT8U   RESERVED8;						// 8EH[83], 1byte
INT8U	PATCHK_STATUS;					// 8EH[84], 1byte 
INT8U   RESERVED9[5];					// 8EH[85~89], 5byte
INT8U	PATGEN_UD7;						// 8EH[90], 1byte
INT8U	PATGEN_UD6;						// 8EH[91], 1byte
INT8U	PATGEN_UD5;						// 8EH[92], 1byte
INT8U	PATGEN_UD4;						// 8EH[93], 1byte
INT8U	PATGEN_UD3;						// 8EH[94], 1byte
INT8U	PATGEN_UD2;						// 8EH[95], 1byte
INT8U	PATGEN_UD1;						// 8EH[96], 1byte
INT8U	PATGEN_UD0;						// 8EH[97], 1byte
INT8U	PATGEN_CTRL;					// 8EH[98], 1byte
INT8U   RESERVED10[18];					// 8EH[99~116], 18byte
INT8U	CHIP_ID;						// 8EH[117], 1byte
INT8U   RESERVED11[9];					// 8EH[118~126], 9byte
INT8U	RESET;							// 8EH[127], 1byte 
INT8U	Reserved128to255[128];			// A6H[128~255], 128byte 
};

#pragma pack () /* set maximum alignment to 1 */  

union uF0_EVB5										  
{ 
struct strF0_EVB5  sStrF0;
struct strLIV      LIV;
INT8U  pStrF0[256];
INT16U wData[128];  
};

union uF0_EVB5 EVB5;

union u80
{ struct str80  sStr80;
  INT8U  pStr80[256];
};
union u80 sBERT; 

//BERT pattern
enum enum_SBERT_Pattern 
{   
	SBERT_P_PRBS7,
	SBERT_P_PRBS23
} eSBERT_Pattern; 

//BERT rate 
enum enum_SBERT_Application 
{  
	SBERT_R_R1244Mbps,
	SBERT_R_R2488Mbps
} eSBERT_Rate;

//GBE 不需要设置BURST状态
struct  struEVB5	  
{
int  BURST_ON;
int  BURST_OFF;
} my_struEVB5;

#define EVB5_BURST_H		1	//定义burst高电平的值
#define EVB5_BURST_L		0	//定义burst低电平的值

#define EVB5_BEN_MODE_LEVEL		0	//定义BEN固定电平模式=
#define EVB5_BEN_MODE_PULSE		1	//定义BEN单脉冲模式
#define EVB5_BEN_MODE_CLOCK		2	//定义BEN时钟模式
#define EVB5_BEN_MODE_N1N2N3	3   //ONU_TxBEN with OLT_Trig, N1N2N3
#define EVB5_BEN_MODE_N1N3		4   //ONU_TxBEN with OLT_Trig, N1N3
#define EVB5_BEN_MODE_N2		5   //ONU_TxBEN with OLT_Trig, N2
#define EVB5_BEN_MODE_L2L3		6   //ONU_TxBEN with OLT_Trig, L2L3
#define EVB5_BEN_MODE_L1L2		7   //ONU_TxBEN with OLT_Trig, L1L2    

#define TINY13_MODE_MANUAL	1	//定义TINY13人工模式
#define TINY13_MODE_AUTO	0	//定义TINY13自动模式  

//for SBERT
extern int EVB5_SBERT_Init (int EVB5Handle, int sBERT_Rate, int sBERT_PRBS); 
extern int EVB5_SBERT_Start (int EVB5Handle, int sBERT_Rate, double sBERT_Time); 
extern int EVB5_SBERT_Read (int EVB5Handle, int sBERT_Rate, int sBERT_PRBS, int *sBERT_Status_Code, char *sBERT_Status_String, double *sBERT_ERR_Number, double *sBERT_ERR_Ratio, double *sBERT_ElapsedTime);
extern int EVB5_SBERT_End (int EVB5Handle);
extern int EVB5_SBERT_Close (int inst_SBERT);
extern int EVB5_SBERT_Open (int inst_SBERT);   

//20090116 add by roger
//-----------------内部函数--------------------// 
int TINY13_Programming_Disable(int EVB5Handle);
int TINY13_Programming_Enable(int EVB5Handle);
int TINY13_Read_EEPROM(int EVB5Handle, unsigned char *EEPROM_Array);
int TINY13_Write_EEPROM(int EVB5Handle, unsigned char *EEPROM_Array);
int TINY13_Write_EEPROM_BYTE(int EVB5Handle, unsigned char NUM_EEPROM_ADD, unsigned char NUM_EEPROM_DAT);	//单字节写
int TINY13_Chip_Erase(int EVB5Handle);
int TINY13_OnePage_WRITE_FLASH(int EVB5Handle, int CurrentPageNumner, unsigned char *Data_Array);
int TINY13_OnePage_READ_FLASH(int EVB5Handle, int CurrentPageNumner, unsigned char *Data_Array);
int TINY13_Check_Signature_Byte(int EVB5Handle); //读出并检查MEGA88的标志字，返回错误代码   
int TINY13_Write_Fuse_bits(int EVB5Handle, unsigned char FUSE, unsigned char FUSE_HIGH); //设置三个熔丝位 
int TINY13_Read_Fuse_bits(int EVB5Handle, unsigned char *FUSE, unsigned char *FUSE_HIGH, unsigned char *FUSE_Extended); //回读三个熔丝位  
int MEGA88_Programming_Enable (int inst_EVB5);
int MEGA88_Programming_Disable(int inst_EVB5);
//-----------------内部函数--------------------//

//-----------------外部函数--------------------//
//EVB5对TINY13的操作
extern int EVB5_WRITE_TINY13_2BYTEs(int EVB5Handle, BYTE Addr1, BYTE Data1, BYTE Addr2, BYTE Data2);//Tiny13 EEPROM 双字节写 
extern int EVB5_WRITE_TINY13_BYTE(int EVB5Handle, BYTE Addr, BYTE Data);	//Tiny13 EEPROM 单字节写 
extern int EVB5_READ_TINY13_PAGE(int EVB5Handle, BYTE *rom_value);			//Tiny13 EEPROM 页度
extern int EVB5_READ_TINY13_Temper(int EVB5Handle, double *Temper);			//Tiny13 温度读取
extern int EVB5_READ_TINY13_Ibias(int EVB5Handle, double *Ibias);			//Tiny13 偏置电流读取 
extern int EVB5_WRITE_TINY13_Firmware(int EVB5Handle, BYTE *FirmwareData);	//Tiny13 更新Firmware
extern int EVB5_READ_TINY13_Firmware(int EVB5Handle, BYTE *FirmwareData);   //Tiny13 读取Firmware
extern int EVB5_WRITE_TINY13_FuseBit(int EVB5Handle);						//Tiny13 设置熔丝位
extern int EVB5_READ_TINY13_FuseBit(int EVB5Handle);						//Tiny13 读取熔丝位 
extern int EVB5_WRITE_TINY13_PAGE(int EVB5Handle, BYTE *rom_value);			//Tiny13 EEPROM 页写 
	
//EVB5控制代码
//20090107 更新函数 添加SN参数，返回值改为int
extern int EVB5_Init (int *EVB5Handle, char *SN);
extern int EVB5_Init_Ex (int *EVB5Handle); 
extern int EVB5_GET_SN (int EVB5Handle, char SN[16]);
extern int EVB5_SET_SHDN_VTR(int EVB5Handle, unsigned char SHUTValue);	//1:ON 0:OFF 
extern int EVB5_SET_BEN_Mode(int EVB5Handle, unsigned char BEN_Mode);	//0:level 1:pulse 2:clock  
extern int EVB5_SET_BEN_Level(int EVB5Handle, unsigned char BEN_Level);	//0:low 1:high 
extern int EVB5_SET_BEN_PulsePolarity(int EVB5Handle, unsigned char BEN_PulsePolarity);	//0:Normal 1:Invert 
extern int EVB5_SET_BEN_DELAY(int EVB5Handle, INT8U N1_Delay, INT8U N2_Delay, INT8U N3_Delay);	//设置BEN delay    
extern int EVB5_READ_TxI (int EVB5Handle, double *TxI); 
extern int EVB5_READ_RxI (int EVB5Handle, double *RxI); 
extern int EVB5_READ_ONU_RxSD (int EVB5Handle, BYTE *SD); 
extern int EVB5_READ_ONU_TxSD (int EVB5Handle, BYTE *SD);
extern int EVB5_READ_ONU_TxFAULT (int EVB5Handle, BYTE *TxFAULT); 
extern int EVB5_READ_Temper(int EVB5Handle, double *Temper);
extern int EVB5_READ_OLT_TxFAULT (int EVB5Handle, BYTE *TxFAULT);
extern int EVB5_READ_OLT_RxLOS (int EVB5Handle, BYTE *RxLOS); 
extern int EVB5_SET_OLT_TxDIS (int EVB5Handle, BYTE TxDIS); 
extern int EVB5_SET_SHDN_VOSA (int EVB5Handle, unsigned char SHUTValue);
extern int EVB5_SET_ADN8810 (int EVB5Handle, float If);							//If输出电流mA
extern int EVB5_GET_ADN8810 (int EVB5Handle, float *If, float *Vf, float *Im); 
extern int EVB5_SET_VAPD (int EVB5Handle, const double Vapd);
extern int EVB5_SET_MODE (int EVB5Handle, const char Mode);
extern int EVB5_READ_IPD_MON (int EVB5Handle, double *Iapd);					//OSA端Iapd电流 单位uA
extern int EVB5_READ_VPD_MON (int EVB5Handle, double *Vapd);					//OSA端Vapd电压 单位V
extern int EVB5_SET_Vbr_3V(int EVB5Handle, float *Vbr);							//Vbr-3V测试函数，通过EVB5的MCU完成 
extern int EVB5_SET_VAPD_30V(int EVB5Handle);									//设置30V电压值
extern int EVB5_SET_VPD_MOD_EN (int EVB5Handle, const char Mode); 				//设置OSA端Vapd电压采集模式，或电流采集模式
extern int EVB5_SET_IAPD_EXPECT (int EVB5Handle, const float Iapd_expectMIN, const float Iapd_expectMAX);
extern int EVB5_SET_IAPD(int EVB5Handle, float *Vapd, float *Iapd);				//根据输入的Iapd，得到Vapd
extern int EVB5_READ_VAPDM_MON (int EVB5Handle, double *Vapd);					//TR端Vapd电压 单位V 
extern int EVB5_SET_CLOCK (int EVB5Handle, double Clock);						//设置发送时钟频率， 单位MHz, 由于四舍五入的原因，实际的发送频率有一定误差 
extern int EVB5_SET_BEN_PW(int EVB5Handle, int BEN_PW);							//设置单脉冲宽度，单位uS t = 1.6 + EVB.sStrF0.RSSI_PW * 0.26; 
extern int EVB5_GET_R7_SC (int EVB5Handle, int *ErrorCode, char *ErrorStr);		//短路测试函数 
extern int EVB5_GET_SPI_MEGA88 (int inst_EVB5);									//读取SPI总线设备 返回0，表示SPI上没有连接设备mega88， -1表示SPI上有连接正常的设备mega88
extern int EVB5_SET_TRIG_LEVEL (int EVB5Handle, int tigger_level);				//RSSI trigger 电平设置
extern int EVB5_SET_ONU_TxDIS (int inst_EVB5, BYTE TxDIS); 
extern int EVB5_SET_VTR (int inst_EVB5, float VTR);								//设置模块Vcc电压
extern int EVB5_SET_VAPD_10V (int inst_EVB5);									//设置APD电压10V，由于EVB5电气参数不一致，部分EVB不能达到10V，只能设置到FFF，电压最小值
extern int EVB5_SET_TxSD_Detect (int inst_EVB5);								//设置TxSD判决使能
extern int EVB5_READ_TxSD (int inst_EVB5, BYTE *TxSD_Error);					//读取TxSD状态
extern int EVB5_CLEAR_TxSD (int inst_EVB5);										//清除TxSD状态
extern int EVB5_READ_FirmwareVersion (int inst_EVB5, int *version);				//读取Firmware版本，格式FirmwareVersion*10+FirmwareVersionSuffix
//-----------------外部函数--------------------// 
int sevb5_get_mod_def0 (int inst, int *val);			//读取 MOD_DEF0信号，0:SFP 在评估板 1：SFP不在评估板

//LIV2.0版本功能代码
#define EVB5_LIV_Addr   0x82
#define LIV_GET_STATUS  0xB2 
#define LIV_START   	0xB3
#define LIV_SAMPLES_MAX 500
#define LIV_DELAY       200  //设置LD电流后，等待电流稳定的时间，以及发起功率采样后，等待PD稳定的时间，发起背光和电压采样后等待时间

typedef struct LIV
{
double Vf[LIV_SAMPLES_MAX];	//前向电压数组 V	     
double Im[LIV_SAMPLES_MAX];  //背光电流数组 uA
double Pout[LIV_SAMPLES_MAX];//输出功率数组 uW
double If[LIV_SAMPLES_MAX];  //驱动电流数组 mA
}LIVDATA;

LIVDATA liv_data;

typedef struct LIV_CONFG
{
float k_850;
float b_850;
float k_1310;
float b_1310;
float k_1490;
float b_1490;
} LIVCONFG;

LIVCONFG liv_config;
	
int sevb5_get_liv_status(int inst, int *busybit, int *errbit, int *run_cnt);
int sevb5_set_liv_start(int inst, float Istart, float Istep, float Istop, int wait);
int sevb5_get_liv_data(int inst, int sample_cnt, int wavelength);
int sevb5_init_liv(int inst);
int sevb5_set_liv_point(int inst, float Itest); //设置单点电流

#endif
