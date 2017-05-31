/****************************************************************************
 *
 * File:                UX3328S.h
 *
 * Author:              Superxon(Eric Yao)
 *
 * Description:         基本功能代码
 *
 * Time:                2014-02-18
 *
 * version:				V1.0.0.0
 * 
版本历史
V1.0.0.0
新文件

使用说明
ux3328上电后默认选择表table3，所有非table3的读写操作完成后，必须选回表table3
****************************************************************************/

#ifndef _UX3328S_H_
#define _UX3328S_H_

#include "ch341A_dll.h"

#pragma pack (1) /* set maximum alignment to 1 按一个字节对齐*/  

//0x81h
struct UX3328S_TX_CTRL1
{
INT8U Fstclk1_2:1;
INT8U Ftshdnen:1;  
INT8U Inpolsw:1;
INT8U ModLutEn:1;  	
INT8U CL_OLn:1;
INT8U BiasLutEn:1;
INT8U SD_C:1;
INT8U Lpow:1;
};
//0x82h
struct UX3328S_TX_CTRL2
{
INT8U ApcFsst:1;
INT8U MDM:2;
INT8U VdcDC:5;  	
};

//0x85h 
struct UX3328S_TX_CTRL3
{
INT8U Imt:5;
INT8U MDC:2;  
INT8U Bsmaxprotect:1;
};

//0x88h 
struct UX3328S_TX_CTRL4
{
INT8U Apcscaleset:3;
INT8U Apcscale_en:1;  
INT8U MOD_half:1;
INT8U BIAS_half:1;  	
INT8U LD_BW:1;
INT8U TXD0_sleep1:1;
};

//0x89h
struct UX3328S_TX_CTRL5
{
INT8U CMOS_SEL:1;
INT8U BEN_pch:1;  
INT8U Starten:1;
INT8U Apcrange:2; 
INT8U Txsdset:2;
INT8U Burst0_cm1:1;
};

//0x8Ah
struct UX3328S_TX_CTRL6
{
INT8U FaultSet:7;
INT8U Reserved:1;  
};

//0x8Dh
struct UX3328S_TX_CTRL7
{
INT8U APCSCALE:3;
INT8U Reserved:5;  
};

//0x8Fh
struct UX3328S_TX_CTRL8
{
INT8U P1_SET_8:1;
INT8U P1_SET_9:1;  
INT8U P1_NMU_0:1;
INT8U Reserved:2; 
INT8U SCL0_DCL1:1;
INT8U P1_HALF:1; 
INT8U TX_PD_CTRL:1;  
};

//0x91h
struct UX3328S_TX_CTRL9
{
INT8U Reserved0:3;
INT8U Io_on0_off1:1;  
INT8U Modfasten:1;
INT8U ModMxProtect:1;  	
INT8U Reserved1:2;
};

//0x93h
struct  DCDC_CTRL1
{
INT8U MAXDUTY:3;
INT8U SW_OCP:1;
INT8U SW_OVP:1;
INT8U reserved:3;
};

//0x94h
struct  DCDC_CTRL2
{
INT8U OVP_REF:3;
INT8U DCDC_LPOW:1;
INT8U DA0_PWM1:1;
INT8U DA_LPOW:1;  
INT8U DAI_PNS:1;
INT8U APD_CTRL:1;
};

//0x98h
struct  DCDC_CTRL3
{
INT8U Reserved:2;
INT8U FSEL:1;
INT8U IREF_DC_DC_SEL:1;
INT8U ADJ_FIX_SEL:1;  
INT8U reserved:3;
};

//0x99h
struct  RX_CTRL1
{
INT8U hysel:1;
INT8U Lstat_c:1;
INT8U Los_latch_en:1;
INT8U Los_latch:4;  
INT8U reserved:1;
};

//0x9Bh
struct  RX_CTRL2
{
INT8U std:1;
INT8U Irop:2;
INT8U Ctrl_rssi:1;
INT8U Dapol:1;  
INT8U Squ:1;
INT8U Lopol:1;
INT8U Rx_limscope:1;
};   

//0x9Eh
struct  SYS_CTRL
{
INT8U Rx_respond_to_sleeppin:1;
INT8U Rx_respond_to_softsleep:1;
INT8U Tx_respond_to_sleeppin:1;
INT8U Tx_respond_to_softsleep:1;  
INT8U Apd_respond_to_sleeppin:1;
INT8U Apd_respond_to_softsleep:1;
INT8U Tx_disable_set:1;
INT8U ovflag:1;
};

//0x9Fh
struct  UX3328S_SECURITY_SEL
{
INT8U A0_LOWER:2;
INT8U A0_UPPER:1;
INT8U A2_TAB0:1;
INT8U Status_control_level:1;
INT8U Power_levelling:2;
INT8U Counter_level:1;
};

//0xCAh
struct UX3328S_DDM_CTRL
{
INT8U AMUX_ADDR:4;
INT8U SD_AMUX_C:1;
INT8U DEBUG_EN:1;
INT8U IN:1;
INT8U ctw_en:1;
};

//0xD6h
struct TX_STATUS1
{
INT8U BIASCT_R:2;
INT8U reserved:6;
};

//0xD8h
struct  TX_STATUS2
{
INT8U Fault:7;
INT8U txsdalarm:1;
};

//0xDAh
struct  TX_STATUS3
{
INT8U IMOD_DAC:2;
INT8U reserved:6;
};

//0xDEh
struct  TX_STATUS4
{
INT8U BiasTlut:2;
INT8U reserved:6;
};

//0xE0h
struct  TX_STATUS5
{
INT8U Imod:2;
INT8U reserved:6;
};

//0xE2h
struct  DAO_STATUS
{
INT8U APD_DAC:2;
INT8U reserved:6;
};

//0xE3h
struct  DCDC_STATUS
{
INT8U SS_FINISH:1;
INT8U VPWM_ABLE:1; 
INT8U OCP_OUT:1;	 
INT8U OVP_OUT:1; 
INT8U reserved:4;
};

//0xE4h
struct  RX_STATUS
{
INT8U rop_state:2;
INT8U Reserved:6; 
};

//0xE6h
struct  TEMP_VALUE
{
INT8U TEMP_DATA:1;
INT8U Reserved:7; 
};

//0xEAh
struct  DEBUG_VALUE
{
INT8U Debug_data:1;
INT8U Reserved:7; 
};

//0xEBh
struct  SYS_STATUS
{
INT8U Security:2;
INT8U Reserved:6; 
};

//0xECh
struct  INIT_STATE
{
INT8U dsfail0:1;
INT8U dsfail1:1;   
INT8U dsfail2:1;
INT8U dsfail3:1;   
INT8U EEPROM_FAIL0:1;
INT8U EEPROM_FAIL1:1;
INT8U EEPROM_FAIL2:1;
INT8U EEPROM_FAIL3:1;
};

//0xEDh
struct  EEPROM_FLAG
{
INT8U EEPROMflag0:1;
INT8U EEPROMflag1:1;   
INT8U EEPROMflag2:1;
INT8U EEPROMflag3:1;   
INT8U EEPROMflag4:1;
INT8U EEPROMflag5:1;
INT8U EEPROMflag6:1;
INT8U EEPROMflag7:1;
};

//0xEEh
struct  EEPROM_FAIL_FLAG
{
INT8U eerxfailflag0:1;
INT8U eerxfailflag1:1;   
INT8U eerxfailflag2:1;
INT8U eerxfailflag3:1;   
INT8U eerxfailflag4:1;
INT8U eerxfailflag5:1;
INT8U eerxfailflag6:1;
INT8U eerxfailflag7:1;
};

struct UX3328S_TABLE3
{
INT8U	EEPROMIdenfier0;							// A2 [128]
struct 	UX3328S_TX_CTRL1 UX3328S_TX_CTRL1;			// A2 [129]
struct 	UX3328S_TX_CTRL2 UX3328S_TX_CTRL2;			// A2 [130]
INT8U   BiasMax;            						// A2 [131] 
INT8U  	Iapcset;									// A2 [132]  
struct 	UX3328S_TX_CTRL3 UX3328S_TX_CTRL3;			// A2 [133]
INT8U  	bias_dac;									// A2 [134]
INT8U  	Imodc;										// A2 [135]
struct 	UX3328S_TX_CTRL4 UX3328S_TX_CTRL4;			// A2 [136] 
struct 	UX3328S_TX_CTRL5 UX3328S_TX_CTRL5;			// A2 [137]
struct 	UX3328S_TX_CTRL6 UX3328S_TX_CTRL6;			// A2 [138]
INT8U   reserved1;									// A2 [139]
INT8U   COUNTER;									// A2 [140]
struct 	UX3328S_TX_CTRL7 UX3328S_TX_CTRL7;			// A2 [141]
INT8U   P1_SET;										// A2 [142]
struct 	UX3328S_TX_CTRL8 UX3328S_TX_CTRL8;			// A2 [143]
INT8U   Imodmax;									// A2 [144]
struct 	UX3328S_TX_CTRL9 UX3328S_TX_CTRL9;			// A2 [145]
INT8U   reserved2;									// A2 [146]
struct  DCDC_CTRL1  DCDC_CTRL1;						// A2 [147]
struct  DCDC_CTRL2  DCDC_CTRL2;						// A2 [148]
INT16U  DCDC_threshold;								// A2 [149-150]
INT8U   APD_DAC;									// A2 [151]
struct  DCDC_CTRL3  DCDC_CTRL3;						// A2 [152]
struct  RX_CTRL1   	RX_CTRL1;						// A2 [153]
INT8U   RXAlarmDAC;									// A2 [154]
struct  RX_CTRL2   RX_CTRL2;						// A2 [155]
INT8U   reserved3;									// A2 [156]
INT8U   reserved4;									// A2 [157]
struct  SYS_CTRL   SYS_CTRL;						// A2 [158]
struct  UX3328S_SECURITY_SEL UX3328S_SECURITY_SEL;  // A2 [159]
INT16U  TX_PD;										// A2 [160-161]  
INT8U   TIMER;										// A2 [162]
INT8U   reserved5;									// A2 [163]
INT8U   TEMP_COMP;									// A2 [164]
INT16U  TEMP_SLOPE1;								// A2 [165-166]
INT16S  TEMP_OFFSET1;								// A2 [167-168]
INT16U  TEMP_SLOPE0;								// A2 [169-170]
INT16S  TEMP_OFFSET0;								// A2 [171-172] 
INT16U  VCC_SLOPE;									// A2 [173-174]
INT16S  VCC_OFFSET;									// A2 [175-176]  
INT16U  BIAS_SLOPE;									// A2 [177-178]
INT16S  BIAS_OFFSET;								// A2 [179-180]   
INT16U  TX_SLOPE;									// A2 [181-182]
INT16S  TX_OFFSET;									// A2 [183-184] 
INT16U  RX_SLOPE2;									// A2 [185-186]
INT16S  RX_OFFSET2;									// A2 [187-188]
INT16U  RX_SLOPE1;									// A2 [189-190]
INT16S  RX_OFFSET1;									// A2 [191-192]  
INT16U  RX_SLOPE0;									// A2 [193-194]
INT16S  RX_OFFSET0;									// A2 [195-196]   
INT16U	RX_COMP1;									// A2 [197-198]  
INT16U	RX_COMP0;									// A2 [199-200]   
INT8U	SLAVE_ADDRESS;								// A2 [201]   
struct 	UX3328S_DDM_CTRL UX3328S_DDM_CTRL;			// A2 [202] 
INT8U   reserved6;									// A2 [203]  
INT32U  PW1;										// A2 [204-207] 
INT32U  PW2;										// A2 [208-211]
INT8U   EEPROMchksum;								// A2 [212]
INT8U	EEPROMIdenfier1;							// A2 [213]
INT8U   BIASCT_R;									// A2 [214]
struct  TX_STATUS1	TX_STATUS1;						// A2 [215]
struct  TX_STATUS2	TX_STATUS2;						// A2 [216]
INT8U   IMOD_DAC_Dis;								// A2 [217]
struct  TX_STATUS3	TX_STATUS3;						// A2 [218]
INT8U   reserved7;									// A2 [219]
INT8U   reserved8;									// A2 [220]
INT8U	BiasTlut;									// A2 [221]
struct  TX_STATUS4	TX_STATUS4;						// A2 [222]
INT8U   IMOD_LUT;									// A2 [223]
struct  TX_STATUS5	TX_STATUS5;						// A2 [224]
INT8U   APD_LUT;									// A2 [225]
struct  DAO_STATUS   DAO_STATUS;					// A2 [226]
struct  DCDC_STATUS  DCDC_STATUS;					// A2 [227]
struct  RX_STATUS   RX_Status;						// A2 [228]
INT8U 	TEMP_DATA;									// A2 [229]
INT8U 	TEMP_VALUE;									// A2 [230]
INT16U  RX_ADC_Value;								// A2 [231-232]
INT16U  DEBUG_DATA;									// A2 [233-234]
struct  SYS_STATUS   SYS_Status;					// A2 [235]
struct  INIT_STATE   INIT_Status;					// A2 [236]
struct  EEPROM_FLAG  EEPROM_Flag;					// A2 [237]
struct  EEPROM_FAIL_FLAG   EEPROM_Fail;				// A2 [238]
INT8U	reserved238to255[17];						// A2 [239-255]   
};  

struct UX3328S_LUT
{
INT8U   DAC[84];
INT8U   Reserved188to255[44];
};

union UX3328S_Table										  
{ 
struct UX3328S_TABLE3	UX3328S_TABLE3;
struct UX3328S_LUT 		UX3328S_LUT_APC;
struct UX3328S_LUT 		UX3328S_LUT_MOD;
struct UX3328S_LUT 		UX3328S_LUT_APD;
};

struct UX3328S 
{ 
INT16S	tempHighAlarm;				// A2 [0-1]
INT8U   Reserved2to122[121];		// A2 [2-122] 
INT32U  Password;  					// A2 [123~126]   										
INT8U   TableIndex;					// A2 [127] 
union UX3328S_Table uTable;			// A2 [128-255]
};

union uUX3328S										  
{ 
struct UX3328S  sStr;
INT8U  		   pStr[256];
};

#pragma pack ()  /*取消自定义字节对齐方式*/

#define UX3328S_AD_NUM  		84    //温度采样点数，
#define UX3328S_INDEXTEMP_COMP  101    //索引温度补偿系数

static unsigned char UX3328S_AD[84]=
{
0x41,0x43,0x45,0x47,0x49,0x4B,0x4D,0x4F,0x51,0x53,0x55,0x57,0x59,0x5B,0x5D,0x5F,
0x61,0x63,0x65,0x67,0x69,0x6B,0x6D,0x6F,0x71,0x73,0x75,0x77,0x79,0x7B,0x7D,0x7F,
0x81,0x83,0x85,0x87,0x89,0x8B,0x8D,0x8F,0x91,0x93,0x95,0x97,0x99,0x9B,0x9D,0x9F,
0xA1,0xA3,0xA5,0xA7,0xA9,0xAB,0xAD,0xAF,0xB1,0xB3,0xB5,0xB7,0xB9,0xBB,0xBD,0xBF,
0xC1,0xC3,0xC5,0xC7,0xC9,0xCB,0xCD,0xCF,0xD1,0xD3,0xD5,0xD7,0xD9,0xDB,0xDD,0xDF,
0xE1,0xE3,0xE5,0xE7
};	  

#define UX3328S_TABLE3_NUM      128
#define UX3328S_TEMP_MAX  200 

static ViString UX3328S_TABLE3_NAME[UX3328S_TABLE3_NUM] = {
"EEPROMIdentifier0",
"Tx_CTRL1",
"Tx_CTRL2",
"BiasMax",
"Iapcset",
"Tx_CTRL3",
"Bias_DAC",
"Imodc",
"TX_CTRL4",
"TX_CTRL5",
"TX_CTRL6",
"reserved1",
"COUNTER",
"TX_CTRL7",
"P1_SET",
"TX_CTRL8",
"IModMax",
"TX_CTRL9",
"reserved2",
"DCDC_CTRL1",
"DCDC_CTRL2",
"DCDC_threshold",
"DCDC_threshold",
"APD_DAC",
"DCDC_CTRL3",
"RX_CTRL1",
"RXAlarmDAC",
"RX_CTRL2",
"reserved3",
"reserved4",
"SYS_CTRL",
"SECURITY_SEL",
"TX_PD",
"TX_PD",
"TIMER",
"reserved5",
"TEMP_COMP",
"Temp_Slope1",
"Temp_Slope1",
"Temp_Offset1",
"Temp_Offset1",
"Temp_Slope0",
"Temp_Slope0",
"Temp_Offset0",
"Temp_Offset0",  
"Vcc_Slope",
"Vcc_Slope",
"Vcc_Offset",
"Vcc_Offset",	 
"Bias_Slope",
"Bias_Slope",
"Bias_Offset",
"Bias_Offset", 
"Tx_Slope",
"Tx_Slope",
"Tx_Offset",
"Tx_Offset",
"Rx_Slope2",
"Rx_Slope2",
"Rx_Offset2",
"Rx_Offset2",  
"Rx_Slope1",
"Rx_Slope1",
"Rx_Offset1",
"Rx_Offset1",
"Rx_Slope0",
"Rx_Slope0",
"Rx_Offset0",
"Rx_Offset0",
"RX_COMP1",
"RX_COMP1",
"RX_COMP0",
"RX_COMP0",
"Slave_Address",  
"DDM_CTRL",
"reserved6",  
"PW1",
"PW1",
"PW1",
"PW1",
"PW2",
"PW2",
"PW2",
"PW2",
"EEPROMchksum",
"EEPROMIdentifier1",  
"BIASCT_R",  
"TX_STATUS1",
"TX_STATUS2",
"IMOD_DAC_Dis",
"TX_STATUS3",
"reserved7", 
"reserved8", 
"Biastlut",
"TX_STATUS4",
"IMOD_LUT",
"TX_STATUS5",
"APD_LUT",
"DAO_STATUS", 
"DCDC_STATUS",  
"RX_Status",   
"TEMP_DATA", 
"RX_ADC_Value", 
"Debug_data",   
"SYS_Status",	 
"INIT_Status",
"EEPROM_Flag",		
"EEPROM_FAIL_FLAG", 
"reserved238to255",  
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",  
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255",
"reserved238to255"
};

int ux3328s_get_table (int inst_I2C, int *Table);
int ux3328s_select_table (int inst_I2C, int Table);
int ux3328s_ENTRY_PW2 (int inst_I2C);	//输入level 2级密码 
int ux3328s_ENTRY_PW1 (int inst_I2C);    //输入level 1级密码
int ux3328s_SET_PW (int inst_I2C); 		//设置level 1级密码 0 0 0 0 和设置level 2级密码'S', 'O', 'E', 'B' 
int ux3328s_set_mode (int inst_I2C, int mode);	//设置LUT模式， 0=disable， 1=enable， 包括bias、mod、apd三个查找表
int ux3328s_get_bias_dac (int inst_I2C, INT8U *val);
int ux3328s_get_biaslut_dac (int inst_I2C, INT8U *val); 
int ux3328s_set_bias_dac (int inst_I2C, INT8U val);
int ux3328s_get_mod_dac (int inst_I2C, INT8U *val);
int ux3328s_get_modlut_dac (int inst_I2C, INT8U *val); 
int ux3328s_set_mod_dac (int inst_I2C, INT8U val); 
int ux3328s_get_apd_dac (int inst_I2C, INT8U *val);
int ux3328s_get_apdlut_dac (int inst_I2C, INT8U *val); 
int ux3328s_set_apd_dac (int inst_I2C, INT8U val);
int ux3328s_set_calibration_temper (int inst_I2C, int temper_comp, float slope0, short int offset0, float slope1, short int offset1); 
int ux3328s_get_calibration_temper (int inst_I2C, int *temper_comp, float *slope0, short int *offset0, float *slope1, short int *offset1);
int ux3328s_set_calibration_vcc (int inst_I2C, float slope, short int offset);
int ux3328s_get_calibration_vcc (int inst_I2C, float *slope, short int *offset);
int ux3328s_set_calibration_tx (int inst_I2C, float tx_power, float *slope, short int *offset); 
int ux3328s_get_calibration_tx (int inst_I2C, float *slope, short int *offset); 
int ux3328s_set_calibration_bias(int inst_I2C, float slope, short int offset);
int ux3328s_get_calibration_bias(int inst_I2C, float *slope, short int *offset); 
int ux3328s_set_calibration_rx_clear (int inst_I2C);//清空收端校准系数
int ux3328s_set_calibration_rx (int inst_I2C, float rx_power, float *slope, short int *offset, BOOL Flag);  
int ux3328s_set_calibration_rx_multi(int inst_I2C, double rx_power[4], unsigned short int ADC[4]); 
int ux3328s_get_calibration_mode(int inst_I2C, INT8U val);	//设置监控量校准方式 2内部校准，1外部校准
int ux3328s_set_checksum_A2_Table(int inst_I2C);
int ux3328s_set_checksum_A2(int inst_I2C);
int ux3328s_set_checksum_A0(int inst_I2C);
int ux3328s_set_default(int inst_I2C);		//初始化配置 
int ux3328s_set_table3(int inst_I2C, int reg[60]);			//初始化table3配置, 写入128~187字节配置数据 
int ux3328s_set_calibration_A2_T (int inst_I2C, float slope, short int offset);   	//设置A2 温度校准系数
int ux3328s_set_calibration_A2_V (int inst_I2C, float slope, short int offset);		//设置A2 电压校准系数 
int ux3328s_set_calibration_A2_Tx_I (int inst_I2C, float slope, short int offset);	//设置A2 偏置电流校准系数  
int ux3328s_set_calibration_A2_Tx_PWR (int inst_I2C, float slope, short int offset); //设置A2 发端功率校准系数  
int ux3328s_set_calibration_A2_Rx_PWR (int inst_I2C, float Rx_PWR4, float Rx_PWR3, float Rx_PWR2, float Rx_PWR1, float Rx_PWR0); //设置A2 收端功率校准系数  
int ux3328s_get_temper (int inst_I2C, float *val);	//读取校准索引温度 
int ux3328s_get_lut (int inst_I2C, INT8U LUT[105]); 
int ux3328s_set_lut (int inst_I2C, INT8U LUT[105]); 
int ux3328s_fit_mod_cClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD);
int ux3328s_fit_mod_iClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD, double Imod030_10, double Imod82_105); 
int ux3328s_set_apc_dac (int inst_I2C, INT8U val);
int ux3328s_get_apc_dac (int inst_I2C, INT8U *val);
int ux3328s_get_table3(int inst_I2C, INT8U table_data[]);
int ux3328s_set_table3_ex(int inst_I2C, INT8U table_data[]);	//初始化table3配置, 写入128~187字节配置数据,与函数ux3328_set_table3不同的是不写密码区，SLAVE_ADDRESS字节需要做bit0和bit1清零  
		  
int ux3328s_fit_apd (int inst_I2C, double kl, double kh, INT8U APD_DAC);
int ux3328s_set_backup_A2_Table (int inst_I2C, int table);
int ux3328s_check_eepflag (int inst_I2C, BOOL flag);
int ux3328s_write_backup_A2_Table (int inst_I2C);
int ux3328s_set_table3_backup(int inst_I2C, INT8U table_data[]);
int ux3328s_set_lut_ex (int inst_I2C, INT8U LUT[60], int table);
int ux3328s_check_A2flag(int inst_I2C, BOOL flag);

int ux3328s_set_table3_default (int inst_I2C);

int ux3328s_fit_APC_cClass (int inst_I2C, double Ibias10_33, double Ibias33_60, INT16U DAC_APC);
int ux3328s_set_FactoryMode(int InstI2C);
int ux3328s_ENTRY_PW0(int inst_I2C);
int ux3328s_set_UserMode(int InstI2C);
int ux3328s_set_los_dac(int inst_I2C, INT8U val);
int ux3328s_get_los_dac(int inst_I2C, int *val) ;
int ux3328s_get_biasct_r_dac (int inst_I2C, INT16U *val);

int ux3328s_set_los_RX_CTRL2(int inst_I2C, INT8U val);

int ux3328s_get_laser_tempe (int inst_I2C, double *temp);
int ux3328s_set_TX_CTRL2(int inst_I2C, INT8U val);
int ux3328s_get_Tec_Current (int inst_I2C, double *Current);
int ux3328s_get_LDtempe_TecCurr (int inst_I2C,double *temp, double *Current);
int ux3328s_get_PID_LOOKFLAG (int inst_I2C, int *val);
int ux3328s_Get_TX_CTRL2 (int inst_I2C, int *val);

#endif  

