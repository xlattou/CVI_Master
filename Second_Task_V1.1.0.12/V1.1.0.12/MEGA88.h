/****************************************************************************
 *
 * File:                MEGA88.h
 *
 * Author:              Superxon(Roger Li)
 *
 * Description:         MEGA88基本功能代码, 统一了原有的   MEGA88_GBE和MEGA88_ONU文件
 *
 * Time:                2011-11-18
 *
 * Version:				V4.0.0.0
版本历史
V4.0.0.0
新增函数
MEGA88_SET_A2TemperKBl
MEGA88_SET_A2TemperKBh
MEGA88_SET_A2TemperKB_Index
MEGA88_SET_A2Temper
MEGA88_GET_Temper_Index

V3.0.0.2 
新增函数MEGA88_GET_BIASDAC
新增函数MEGA88_SET_BIASPRELOAD   

V3.0.0.1
1. add functions MEGA88_SET_LUT_APD

V3.0.0.0
1. add functions MEGA88_SET_LUT_APC_PL for V7 firmware later powerleveling
1. add functions MEGA88_SET_LUT_MOD_PL for V7 firmware later powerleveling

V2.0.0.2
add function MEGA88_GET_FirmwareVersion ()
 
V2.0.0.1
函数MEGA88_SET_Mode_Manual和MEGA88_SET_Mode_Auto删除对0x76地址的写入代码，避免对PowerLeveling产品产生影响
****************************************************************************/

#ifndef _MEGA88_H_
#define _MEGA88_H_ 

#include <windows.h> 
#include "CH341A_DLL.h" 
#include "global.h"

#pragma pack (1) /* set maximum alignment to 1 */  

struct sTRxConfig_DEF
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

struct strA2table1
{
INT8U   FirstByte;				   // A2H[0], 1byte
INT8U   Reserved1to127[127];       // A2H[1~127], 127byte
//FLASH
INT8U   fMODSETDAC;                // A2H[128]  0x00      R/W
INT8U   fBIASMAXM;                 // A2H[129]  0x01      R/W
INT8U   fBIASMAXL;                 // A2H[130]  0x02      R/W
INT8U   fBIASPRELOADM;             // A2H[131]  0x03      R/W
INT8U   fBIASPRELOADL;             // A2H[132]  0x04      R/W
INT8U   fAPCSETDAC;                // A2H[133]  0x05      R/W
INT8U   fCPDAC;                    // A2H[134]  0x06      R/W
INT8U   fLOSTHADAC;                // A2H[135]  0x07      R/W
INT8U   fLOSTHDDAC;                // A2H[136]  0x08      R/W
INT8U   fBIASDACM;                 // A2H[137]  0x09      R
INT8U   fBIASDACL_CHIPID;          // A2H[138]  0x0A      R
INT8U   fAPCRESSELM;               // A2H[139]  0x0B      R/W
INT8U   fAPCRESSELL_CAPSEL;        // A2H[140]  0x0C      R/W
INT8U   fAMUXA;                    // A2H[141]  0x0D      R/W
INT8U   fAMUXB;                    // A2H[142]  0x0E      R/W
INT8U   fBURSTCTRL;                // A2H[143]  0x0F      R/W
INT8U   fMNSTA;                    // A2H[144]  0x10      R
INT8U   fFAULT;                    // A2H[145]  0x11      R
INT8U   fMODEA;                    // A2H[146]  0x12      R/W
INT8U   fMODEB;                    // A2H[147]  0x13      R/W
INT8U   fMODEC;                    // A2H[148]  0x14      R/W
INT8U   fMODED;                    // A2H[149]  0x15      R/W
INT8U   fOVERA;                    // A2H[150]  0x16      R/W
INT8U   fOVERB;                    // A2H[151]  0x17      R/W
INT8U   fOVERC;                    // A2H[152]  0x18      R/W
INT8U   fOVERD;                    // A2H[153]  0x19      R/W
INT8U   APDPWM;               	   // A2H[154]  0x1A	  R/W
INT8U   Reserved155;               // A2H[155]  0x1B	  R/W
INT8U   Reserved156;               // A2H[156]  0x1C	  R/W
INT8U   RxInnerGain;               // A2H[157]  0x1E	  R/W
struct  sTRxConfig_DEF sTRxConfig; // A2H[158]  0x1F      R/W
INT8U   Reserved159to255[97];      // A2H[159~255] 
};										  
										  
union uA2table1							  			  
{ 
	struct strA2table1 sStr;				  
	INT8U  pStr[256];						  
};										  
										  
//union uA2table1 A2table1, A2table2; 

struct M8_RxCaliData_DEF
{
	INT16U K;
	INT16U B;
};

struct strA2table3
{
	INT8U	FirstByte;				   			// A2H[0], 1byte
	INT8U   Reserved1to127[127];       			// A2H[1~127], 127byte
	INT16U  EE_TEMP_K;               			// A2H[128~129], 2byte R/W
	INT16U	EE_TEMP_B;                			// A2H[130~131], 2byte R/W 
	INT16U	EE_VCC_K;                 			// A2H[132~133], 2byte R/W 
	INT16U 	EE_VCC_B;             				// A2H[134~135], 2byte R/W 
	INT16U  EE_BIAS_K;             				// A2H[136~137], 2byte R/W 
	INT16U  EE_BIAS_B;                			// A2H[138~139], 2byte R/W 
	INT16U  EE_TXPWR_K;               			// A2H[140~141], 2byte R/W 
	INT16U  EE_TXPWR_B;                			// A2H[142~143], 2byte R/W 
	INT16U  EE_RXPWR_K;                			// A2H[144~145], 2byte R/W 
	INT16U  EE_RXPWR_B;                 		// A2H[146~147], 2byte R/W 
	INT16U 	TEMP1_MON_K;          				// A2H[148~149], 2byte R/W 
	INT16U  TEMP1_MON_B;              			// A2H[150~151], 2byte R/W 
	INT16U  T3_VCCT_BASE;						// A2H[152~153], 2byte R/W  
	INT8U   Reserved154to157[4];       			// A2H[154~157], 4byte  
	INT8U   RxCaliPo;							// A2H[158],     1byte R/W  
	INT8U   RxCaliNo;							// A2H[159],     1byte R/W 
	INT16U  RX_PWR_ADC[16];						// A2H[160~191], 32byte	//原始ADC值及其对应的1LSB=0.1uW的光功率值，共16组
	struct  M8_RxCaliData_DEF RxCaliData[16]; 	// A2H[192~255], 64byte	//收端校准系数，共16组  
};										  

union uA2table3							  			  
{ 
    struct strA2table3 sStr;
    INT8U  pStr[256];						  
};										  

struct status_110def_SFRM0050
{
	INT8U  Data_Ready:1;
	INT8U  LOS:1;
	INT8U  TX_Fault:1;
	INT8U  Soft_Rate_Select:1;
	INT8U  Rx_Rate_Select_State:1;
	INT8U  Reserved1:1;
	INT8U  Soft_Tx_Dis:1;	
	INT8U  Tx_Dis:1;
};

struct alarm_113_SFRM0050
{
	INT8U  Reserved0:3;
	INT8U  Tx_P_Min_Low:1;
	INT8U  Reserved1:1;
	INT8U  Tx_P_Max_High:1;
	INT8U  Rx_P_Low:1;
	INT8U  Rx_P_Hgh:1;
};
 
struct SFRM0050
{
	INT8U	reserved8[128];					  	//pStrA2[128~255]  
	INT8U	LUTIndex;							//pStrA2[127] 
	INT32U	Password;  							//pStrA2[123~126]   										
	INT8U   mode;                      			//pStrA2[122]
	INT8U   version;                   			//pStrA2[121]

	INT8U   BEN_i;                				//pStrA2[120]
	INT8U	reserved_2_l;                   	//pStrA2[119]      																																	
	INT8U   reserved_2_m;						//pStrA2[118]
	struct	FLAG_113DEF FLAG_WARN_117;   	  	//pStrA2[117]                 																														
	struct	FLAG_112DEF FLAG_WARN_116;    	  	//pStrA2[116]       																																
	INT8U   reserved_1_l;						//pStrA2[115]
	INT8U   reserved_1_m;						//pStrA2[114]
	struct	alarm_113_SFRM0050 FLAG_ALARM_113;	//pStrA2[113]																															
	struct	FLAG_112DEF FLAG_ALARM_112 ;      	//pStrA2[112]     //Optional Alarm and Warning Flag Bits                                       																																							

	INT8U	PowerLeveling;                    	//pStrA2[111]     //Reserved for SFF-8079        																						
	struct	status_110def_SFRM0050 status_110;	//pStrA2[110]     //Optional Status/Control Bits              																						
	INT8U   tx_power_min_hold_l;				//pStrA2[109]
	INT8U   tx_power_min_hold_m;				//pStrA2[108]
	INT8U   tx_power_max_hold_l;				//pStrA2[107]
	INT8U   tx_power_max_hold_m;				//pStrA2[106]
												
	INT16U	rx_Power;                 		    //pStrA2[104~105] //Measured RX input power.           																						
	INT16U	tx_Power;                 		    //pStrA2[102~103] //Measured TX output power.                                                  																						
	INT16U	tx_Bias;                  		    //pStrA2[100~101] //Internally measured TX Bias Current                                        																									
	INT16U	vcc;                      		    //pStrA2[98~99]   //Internally measured supply voltage in transceiver                          																																									
	INT16S	temperature;              		    //pStrA2[96~97]   //Internally measured module temperature     	
	
	INT8U	cc_Ext;                   		    //pStrA2[95]   	//Check code for the Extended ID Fields           																									
	INT8U	reserved2[3];             		    //pStrA2[92~94]	//Reserved                  																								
	INT16S	voltageOffset;            		    //pStrA2[90~91]	//Offset of Supply Voltage Linear Calibartion      																																											
	INT16U	voltageSlope;             		    //pStrA2[88~89]	//Slope of Supply Voltage Linear Calibartion        																																														
	INT16S	tempOffset;               		    //pStrA2[86~87]	//Offset of Temperature Linear Calibartion                                    																																				
	INT16U	tempSlope;                		    //pStrA2[84~85]	//Slope of Temperature Linear Calibartion                                     																																				
	INT16S	tx_PwrOffset;             		    //pStrA2[82~83]	//Offset of Transmitter Coupled Output Power Linear Calibartion               																																						
	INT16U	tx_PwrSlope;              		    //pStrA2[80~81]	//Slope of Transmitter Coupled Output Power Linear Calibartion                																																						
	INT16S	tx_IOffset;               		    //pStrA2[78~79]	//Offset of Laser Bias Current Linear Calibartion                             																																						
	INT16U	tx_ISlope;                		    //pStrA2[76~77]	//Slope of Laser Bias Current Linear Calibartion           																																													

	float	rx_PWR0;                  		    //pStrA2[72~75]	//Polynomial Fit Coefficient of Order 0 for Rx Optical Power Calibartion.     																 
	float	rx_PWR1;                  		    //pStrA2[68~71]	//Polynomial Fit Coefficient of Order 1 for Rx Optical Power Calibartion.																																							
	float	rx_PWR2;                  		    //pStrA2[64~67]	//Polynomial Fit Coefficient of Order 2 for Rx Optical Power Calibartion.     																																									
	float	rx_PWR3;                  		    //pStrA2[60~63]	//Polynomial Fit Coefficient of Order 3 for Rx Optical Power Calibartion.																																											
	float	rx_PWR4;	              		    //pStrA2[56~59]	//Polynomial Fit Coefficient of Order 4 for Rx Optical Power Calibartion     																																																													
	INT8U	reserved1[6]; 						//pStrA2[50-55]
	INT16U	txPowerMinLowAlarm;	    			//pStrA2[48~49]	//TX Power min high Alarm  
	INT8U	reserved0[6];						//pStrA2[42-47]
	INT16U	txPowerMaxHighAlarm;	  		  	//pStrA2[40~41]	//TX Power max high Alarm     																													

	INT16U	rxPowerLowWarning;      			//pStrA2[38~39]	//Reserved         																														
	INT16U	rxPowerHighWarning;	    			//pStrA2[36~37]	//RX Power High Warning  																												
	INT16U	rxPowerLowAlarm;	    			//pStrA2[34~35]	//RX Power Low Alarm  																											
	INT16U	rxPowerHighAlarm;	    			//pStrA2[32~33]	//RX Power High Alarm 																											
	INT16U	txPowerLowWarning;	    			//pStrA2[30~31]	//TX Power Low Warning   																									
	INT16U	txPowerHighWarning;	    			//pStrA2[28~29]	//TX Power High Warning   																												
	INT16U	txPowerLowAlarm;	    			//pStrA2[26~27]	//TX Power Low Alarm     																													
	INT16U	txPowerHighAlarm;	    			//pStrA2[24~25]	//TX Power High Alarm   																														
	INT16U	biasLowWarning;	        			//pStrA2[22~23]	//Bias Low Warning     																									
	INT16U	biasHighWarning;	      		    //pStrA2[20~21]	//Bias High Warning   																													
	INT16U	biasLowAlarm;	          		    //pStrA2[18~19]	//Bias Low Alarm        																										
	INT16U	biasHighAlarm;	          		    //pStrA2[16~17]	//Bias High Alarm       																										
	INT16U	voltageLowWarning;	      		    //pStrA2[14~15]	//Voltage Low Warning   																									
	INT16U	voltageHighWarning;	      		    //pStrA2[12~13]	//Voltage High Warning   																									
	INT16U	voltageLowAlarm;	      		    //pStrA2[10~11]	//Voltage Low Alarm 				
	INT16U	voltageHighAlarm;	      		    //pStrA2[8~9]  	//Voltage High Alarm  							
	INT16S	tempLowWarning;           		    //pStrA2[6~7]  	//Temperature Low Warning  																											
	INT16S	tempHighWarning;          		    //pStrA2[4~5]  	//Temperature High Warning  																																																				
	INT16S	tempLowAlarm;	          		    //pStrA2[2~3]  	//Temperature Low Alarm  																																																			
	INT16S	tempHighAlarm;	          		    //pStrA2[0~1]  	//Temperature High Alarm 																													 
};

union uA2table0							  			  
{ 
    struct SFRM0050 sStr;
    INT8U  pStr[256];						  
};										  

#pragma pack () /* set maximum alignment to 1 */  

//union uA2table3 M8_A2table3; 
//union uA2table0 SFRM0050_A2;

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
#define T3_TEMP1_MON_G          0x14    /* 94H - 95H - core temperature monitor calibration gain */
#define T3_TEMP1_MON_OFS        0x16    /* 96H - 97H - core temperature monitor calibration offset */

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

//extern int MEGA88_Password0(int USB_I2CHandle);				//0级密码为上电后的默认值可以写pageA0[0..255]和pageA2[123..255] 0xFF 0xFF 0xFF 0xFF 
extern int MEGA88_Password1(int USB_I2CHandle); 				//1级密码 可以写pageA0[0..255]和pageA2[0..95,123..255]  默认值0, 0, 0, 0， 
extern int MEGA88_Password2(int USB_I2CHandle); 				//2级密码 可以写所有table的存储空间 					默认值'S', 'O', 'E', 'B' 
extern int MEGA88_SET_Table(int USB_I2CHandle, BYTE Table);  
extern int MEGA88_SET_Mode_Auto(int USB_I2CHandle);
extern int MEGA88_SET_Mode_Manual(int USB_I2CHandle);
extern int MEGA88_SET_TRxConfigFlag(int USB_I2CHandle, struct sTRxConfig_DEF TRxConfigFlag); 
extern int MEGA88_SET_SoftTxDis(int USB_I2CHandle, BYTE SoftTxDis); 
extern int MEGA88_SET_DAC_MOD(int USB_I2CHandle, INT8U DAC); 
extern int MEGA88_SET_DAC_APC(int USB_I2CHandle, INT8U DAC);
extern int MEGA88_SET_VSCtoFLASH(int USB_I2CHandle, int VSC[26]);					//保存寄存器值到mega的EEPROM中
extern int MEGA88_SET_TX_PWR (int USB_I2CHandle, INT16S gain, INT16S offset); 		//gain=slope*256, offset = intercept 
extern int MEGA88_SET_RX_PWR (int USB_I2CHandle, INT16S gain, INT16S offset); 		//gain=slope*256, offset = intercept 
extern int MEGA88_GET_RX_MON (int USB_I2CHandle, INT16U	*ADC, double *Rx_Mon);		//读取Rx端采样值, ADC是采样值，Rx_Mon是换算后的实际值，单位dBm 
extern int MEGA88_GET_TX_MON (int USB_I2CHandle, INT16U	*ADC, double *Tx_Mon);		//读取Tx端采样值, ADC是采样值，Tx_Mon是换算后的实际值，单位dBm  
extern int MEGA88_SET_BURSTCTRL (int USB_I2CHandle, BYTE BURST);					//设置BURST模式寄存器  0：突发模式，0x70：连续模式	
extern int MEGA88_GET_DAC_MOD(int USB_I2CHandle, INT8U *DAC); 
extern int MEGA88_GET_DAC_APC(int USB_I2CHandle, INT8U *DAC);
extern int MEGA88_SET_LUT_MOD(int USB_I2CHandle, BYTE MOD[81]); 
extern int MEGA88_SET_LUT_APC(int USB_I2CHandle, BYTE APC[81]); 
extern int MEGA88_GET_Temperatrue(int USB_I2CHandle, double *Temperatrue);
extern int MEGA88_GET_Ibias(int USB_I2CHandle, double *Ibias); 
extern int MEGA88_GET_TX_PWR (int USB_I2CHandle, INT16S *gain, INT16S *offset); 	//gain=slope*256, offset = intercept 
extern int MEGA88_GET_RX_PWR (int USB_I2CHandle, INT16S *gain, INT16S *offset); 	//gain=slope*256, offset = intercept 
extern int MEGA88_SET_RX_GAIN(int USB_I2CHandle, INT8U gain); 
extern int MEGA88_SET_RX_PWR_CaliPo (int USB_I2CHandle, INT8U RxCaliPo);
extern int MEGA88_SET_RX_PWR_CaliNo (int USB_I2CHandle, INT8U RxCaliNo);			//分段收端校准点个数
extern int MEGA88_SET_RX_PWR_MultiSegmentFit (int USB_I2CHandle, INT8U RxCaliNo, INT16U *RX_PWR_ADC, struct M8_RxCaliData_DEF *RxCaliData); //分段收端校准参数写入
extern int MEGA88_GET_FirmwareVersion (int USB_I2CHandle, INT8U *version);			//get firmware version int
extern int MEGA88_GET_FirmwareVer(int USB_I2CHandle, char *firmwarever);			//get firmware version string
int MEGA88_GET_FirmwareVer2 (int USB_I2CHandle, char *version);						/***获取新的版本，支持大于9的版本**Eric.Yao***/
int MEGA88_SET_LUT_APC_PL(int USB_I2CHandle, BYTE DAC_APC, int powerlevel);  //set APC lut for V7 later   powerlevel can be set to 0、1、2
int MEGA88_SET_LUT_MOD_PL(int USB_I2CHandle, BYTE DAC_MOD[81], int powerlevel);//set APC lut for V7 later powerlevel can be set to 0、1、2
int MEGA88_SET_LUT_APD(int USB_I2CHandle, BYTE APD[81]);  
int MEGA88_GET_TRxConfigFlag (int USB_I2CHandle, struct sTRxConfig_DEF *TRxConfigFlag);
int MEGA88_GET_LUT_MOD_PL(int USB_I2CHandle, BYTE DAC_MOD[81], int powerlevel);
int MEGA88_GET_BIASDAC(int USB_I2CHandle, INT8U *DACM, INT8U *DACL);				
int MEGA88_SET_BIASPRELOAD(int USB_I2CHandle, INT8U DACM, INT8U DACL);
int MEGA88_SET_APC_DAC120(int USB_I2CHandle, INT8U DAC); 
int MEGA88_SET_fOVERD (int USB_I2CHandle, BYTE fOVERD);								//设置fOVERD寄存器  30：测试模式，0：工作模式	
int MEGA88_SET_fLOS (int USB_I2CHandle, BYTE THADAC, BYTE THDDAC);					//THADAC=R7, THDDAC=R8
int Mega88_Get_fOVERD(int inst_I2C, BYTE *val); 

//适用版本V9.0 
int MEGA88_SET_A2TemperCompens (int USB_I2CHandle, float k, float b); 	//设置A2的温度补偿系数，补偿系数只有一段，适用版本V9.0 

//适用版本V11
int MEGA88_SET_A2TemperKBl (int inst_I2C, float k, float b);	//设置A2温度监控校准系数低温段 
int MEGA88_SET_A2TemperKBh (int inst_I2C, float k, float b);	//设置A2温度监控校准系数高温段
int MEGA88_SET_A2TemperKB_Index(int inst_I2C, float k, float b);//设置内部温度监控校准系数
int MEGA88_SET_A2Temper (int inst_I2C, float val);  			//设置温度分界点
int MEGA88_GET_Temper_Index (int inst_I2C, float *val);			//读取LUT温度监控值 

//SFRM0050使用函数
int MEGA88GetSFRM50TemperIndex (int inst_I2C, float *val); 		//读取SFRM0050项目的LUT温度监控值,与SFRM0017的位置不一样
int MEGA88GetSFRM(int inst_I2C, int *SFRM);						//读取SFRM项目编号

#endif
