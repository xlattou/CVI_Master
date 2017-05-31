/****************************************************************************
 *
 * File:                UX3328.h
 *
 * Author:              Superxon(Roger Li)
 *
 * Description:         基本功能代码
 *
 * Time:                2011-08-22
 *
 * version:				V1.0.0.0
 * 
版本历史
V1.0.0.0
新文件

使用说明
ux3328上电后默认选择表table3，所有非table3的读写操作完成后，必须选回表table3
****************************************************************************/

#ifndef _UX3328_H_
#define _UX3328_H_

#include "ch341A_dll.h"

#pragma pack (1) /* set maximum alignment to 1 按一个字节对齐*/  

//0x81h
struct TX_CTRL0
{
INT8U ApcScale:3;
INT8U TxSDset:2;
INT8U ApcRange:2;
INT8U Starten:1;
};
//0x82h
struct TX_CTRL1
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
//0x83h
struct TX_CTRL2
{
INT8U ApcFsst:1;
INT8U MDM:2;
INT8U VdcDC:5;  	
};

//0x86h 
struct TX_CTRL3
{
INT8U Imt:5;
INT8U MDC:2;  
INT8U Bsmaxprotect:1;
};

//0x89h 
struct TX_CTRL4
{
INT8U ApdLutEn:1;
INT8U CMOS_SEL:1;  
INT8U BEN_pch:1;
INT8U TX_TEST2:4;  	
INT8U TX_TEST1:1;
};

//0x8Ah
struct TX_CTRL5
{
INT8U DAI_PNS:1;
INT8U TX_TEST4:1;  
INT8U TX_TEST3:1;
INT8U FaultSet:5;  	
};

//0x8Bh
struct TEMP_RX_CTRL
{
INT8U hysel:1;
INT8U Losel:1;  
INT8U Squ:1;
INT8U Lopol:1; 
INT8U TX_TEST7:1;
INT8U TX_TEST6:1;
INT8U TX_TEST5:1; 
INT8U DA_LPOW:1;
};

//0xB2h
struct DDM_CTRL
{
INT8U AMUX_ADDR:4;
INT8U SD_AMUX_C:1;
INT8U DEBUG_EN:1;
INT8U EXT:1;
INT8U IN:1;
};

//0xBBh
struct  SECURITY_SEL
{
INT8U A0_LOWER:2;
INT8U A0_UPPER:1;
INT8U A2_TAB0:1;
INT8U Reserved:4;
};

struct TABLE3
{
INT8U	EEPROMIdenfier0;			// A2 [128]
struct 	TX_CTRL0 TX_CTRL0;			// A2 [129]
struct 	TX_CTRL1 TX_CTRL1;			// A2 [130]
struct 	TX_CTRL2 TX_CTRL2;			// A2 [131]
INT8U   BiasMax;            		// A2 [132] 
INT8U  	Iapcset;					// A2 [133]  
struct 	TX_CTRL3 TX_CTRL3;			// A2 [134]
INT8U  	bias_dac;					// A2 [135]
INT8U  	mod_dac;					// A2 [136]
struct 	TX_CTRL4 TX_CTRL4;			// A2 [137] 
struct 	TX_CTRL5 TX_CTRL5;			// A2 [138] 
struct 	TEMP_RX_CTRL TEMP_RX_CTRL;	// A2 [139]  
INT8U	RxAlarmDAC;		            // A2 [140]
INT8U	Rx_Ctrl;                    // A2 [141]  
INT16U  TEMP_SLOPE;					// A2 [142-143]
INT16S  TEMP_OFFSET;				// A2 [144-145]
INT16U  VCC_SLOPE;					// A2 [146-147]
INT16S  VCC_OFFSET;					// A2 [148-149]
INT16U  BIAS_SLOPE;					// A2 [150-151]
INT16S  BIAS_OFFSET;				// A2 [152-153]
INT16U  TX_SLOPE;					// A2 [154-155]
INT16S  TX_OFFSET;					// A2 [156-157]
INT16U  RX_SLOPE2;					// A2 [158-159]
INT16S  RX_OFFSET2;					// A2 [160-161]
INT16U  RX_SLOPE1;					// A2 [162-163]
INT16S  RX_OFFSET1;					// A2 [164-165]
INT16U  RX_SLOPE0;					// A2 [166-167]
INT16S  RX_OFFSET0;					// A2 [168-169]
INT16U	RX_COMP1;					// A2 [170-171]  
INT16U	RX_COMP0;					// A2 [172-173]   
INT16U  INTER_TEST6;				// A2 [174-175]
INT8U	apd_dac;					// A2 [176]
INT8U	SLAVE_ADDRESS;				// A2 [177] 
struct 	DDM_CTRL DDM_CTRL;			// A2 [178] 
INT32U  PW1;						// A2 [179-182] 
INT32U  PW2;						// A2 [183-186] 
struct  SECURITY_SEL SECURITY_SEL;	// A2 [187]
//INT8U	Reserved188to193[6];		// A2 [188-193] 
INT8U   BIASCT_R;					// A2 [188] 
INT8U   LDD_Fault;					// A2 [189] 
INT8U   RX_Status;					// A2 [190] 
INT8U   INIT_Status;				// A2 [191]
INT8U   SYS_Status;					// A2 [192]
INT8U   EEPROM_Flag;				// A2 [193]
INT8U	BIAS_LUT;					// A2 [194]
INT8U	MOD_LUT;					// A2 [195] 
INT8U 	DEBUG_DATA;					// A2 [196] 
INT8U 	APD_LUT;					// A2 [197] 
INT8U 	TEMP_DATA;					// A2 [198]  
//INT8U	Reserved199to202[4];		// A2 [199-202] 
INT8U   EEPROM_Fail;				// A2 [199]
INT16U  RX_ADC_Value;				// A2 [200-201]
INT8U   EEPROMchksum;				// A2 [202]
INT8U	EEPROMIdenfier1;			// A2 [203]
INT8U	Reserved204to255[52];		// A2 [204-255] 
};  

struct LUT
{
INT8U   DAC[60];
INT8U   Reserved188to255[68];
};

union Table										  
{ 
struct TABLE3	TABLE3;
struct LUT 		LUT_APC;
struct LUT 		LUT_MOD;
struct LUT 		LUT_APD;
};

struct UX3328 
{ 
INT16S	tempHighAlarm;				// A2 [0-1]
INT8U   Reserved2to122[121];		// A2 [2-122] 
INT32U  Password;  					// A2 [123~126]   										
INT8U   TableIndex;					// A2 [127] 
union Table uTable;					// A2 [128-255]
};

union uUX3328										  
{ 
struct UX3328  sStr;
INT8U  		   pStr[256];
};

#pragma pack ()  /*取消自定义字节对齐方式*/

#define UX3328_AD_NUM  		60    //温度采样点数，
#define UX3328_INDEXTEMP_COMP 99.2526 //索引温度补偿系数

static unsigned char UX3328_AD[60]={0x4C,0x4F,0x52,0x55,0x58,0x5B,0x5E,0x61,0x64,0x67,0x6A,0x6D,0x70,0x73,0x76,0x79,0x7C,0x7F,0x82,0x85,0x88,0x8B,0x8E,0x91,0x94,0x97,0x9A,0x9D,0xA0,0xA3,0xA6,0xA9,0xAC,0xAF,0xB2,
0xB5,0xB7,0xB9,0xBB,0xBD,0xBF,0xC1,0xC3,0xC5,0xC7,0xC9,0xCB,0xCD,0xCF,0xD1,0xD3,0xD5,0xD7,0xD9,0xDB,0xDD,0xDF,0xE1,0xE3,0xE5};

#define UX3328_TABLE3_NUM      76
#define UX3328_TEMP_MAX  200 

static ViString UX3328_TABLE3_NAME[UX3328_TABLE3_NUM] = {
"EEPROMIdentifier0",
"Tx_CTRL0",
"Tx_CTRL1",
"Tx_CTRL2",
"BiasMax",
"Iapcset",
"TX_CTRL3",
"Biaspre",
"Imodc",
"TX_CTRL4",
"TX_CTRL5",
"TEMP_RX_CTRL",
"RxAlarmDAC",
"Rx_CTRL",
"Temp_Slope",
"Temp_Slope",
"Temp_Offset",
"Temp_Offset",
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
"INTER_TEST",
"INTER_TEST",
"APD_DACC",
"Slave_Address",
"DDM_CTRL",
"PW1",
"PW1",
"PW1",
"PW1",
"PW2",
"PW2",
"PW2",
"PW2",
"Security_sel",
"BIAS_CTRL",
"LDD_Fault",
"Rx_STATUS",
"INIT_STATE",
"SYS_STAUS",
"EEPROM_FLAG",
"Biastlut",
"Imod",
"Debug_data",
"APD_DAC",
"Temp_Data",
"EEPROM_FAIL_FLAG",
"RX_ADC_VALUE",
"RX_ADC_VALUE",
"EEPROMchksum",
"EEPROMIdentifier1"
};

int ux3328_select_table (int inst_I2C, int Table);
int ux3328_ENTRY_PW2 (int inst_I2C);	//输入level 2级密码 
int ux3328_ENTRY_PW1 (int inst_I2C);    //输入level 1级密码
int ux3328_ENTRY_PW0(int inst_I2C);     //输入level 0级密码 FF FF FF FF   
int ux3328_SET_PW (int inst_I2C); 		//设置level 1级密码 0 0 0 0 和设置level 2级密码'S', 'O', 'E', 'B' 
int ux3328_set_mode (int inst_I2C, int mode);	//设置LUT模式， 0=disable， 1=enable， 包括bias、mod、apd三个查找表
int ux3328_get_bias_dac (int inst_I2C, INT8U *val);
int ux3328_get_biaslut_dac (int inst_I2C, INT8U *val); 
int ux3328_set_bias_dac (int inst_I2C, INT8U val);
int ux3328_get_mod_dac (int inst_I2C, INT8U *val);
int ux3328_get_modlut_dac (int inst_I2C, INT8U *val); 
int ux3328_set_mod_dac (int inst_I2C, INT8U val); 
int ux3328_get_apd_dac (int inst_I2C, INT8U *val);
int ux3328_get_apdlut_dac (int inst_I2C, INT8U *val); 
int ux3328_set_apd_dac (int inst_I2C, INT8U val);
int ux3328_set_calibration_temper (int inst_I2C, float slope, short int offset); 
int ux3328_get_calibration_temper (int inst_I2C, float *slope, short int *offset);
int ux3328_set_calibration_vcc (int inst_I2C, float slope, short int offset);
int ux3328_get_calibration_vcc (int inst_I2C, float *slope, short int *offset);
int ux3328_set_calibration_tx (int inst_I2C, float tx_power, float *slope, short int *offset); 
int ux3328_get_calibration_tx (int inst_I2C, float *slope, short int *offset); 
int ux3328_set_calibration_bias(int inst_I2C, float slope, short int offset);
int ux3328_get_calibration_bias(int inst_I2C, float *slope, short int *offset); 
int ux3328_set_calibration_rx_clear (int inst_I2C);//清空收端校准系数
int ux3328_set_calibration_rx(int inst_I2C, float rx_power, float *slope, short int *offset);  
int ux3328_set_calibration_rx_multi(int inst_I2C, double rx_power[4], unsigned short int ADC[4]); 
int ux3328_get_calibration_mode(int inst_I2C, INT8U val);	//设置监控量校准方式 2内部校准，1外部校准
int ux3328_set_checksum_A2_Table(int inst_I2C);
int ux3328_set_checksum_A2(int inst_I2C);
int ux3328_set_checksum_A0(int inst_I2C);
int ux3328_set_default(int inst_I2C);		//初始化配置 
int ux3328_set_table3(int inst_I2C, int reg[60]);			//初始化table3配置, 写入128~187字节配置数据 
int ux3328_set_calibration_A2_T (int inst_I2C, float slope, short int offset);   	//设置A2 温度校准系数
int ux3328_set_calibration_A2_V (int inst_I2C, float slope, short int offset);		//设置A2 电压校准系数 
int ux3328_set_calibration_A2_Tx_I (int inst_I2C, float slope, short int offset);	//设置A2 偏置电流校准系数  
int ux3328_set_calibration_A2_Tx_PWR (int inst_I2C, float slope, short int offset); //设置A2 发端功率校准系数  
int ux3328_set_calibration_A2_Rx_PWR (int inst_I2C, float Rx_PWR4, float Rx_PWR3, float Rx_PWR2, float Rx_PWR1, float Rx_PWR0); //设置A2 收端功率校准系数  
int ux3328_get_temper (int inst_I2C, float *val);	//读取校准索引温度 
int ux3328_get_lut (int inst_I2C, INT8U LUT[60]); 
int ux3328_set_lut (int inst_I2C, INT8U LUT[60]); 
int ux3328_fit_mod_cClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD);
int ux3328_fit_mod_iClass (int inst_I2C, double Imod10_33, double Imod33_60, double Imod60_82, INT8U DAC_MOD, double Imod030_10, double Imod82_105); 
int ux3328_set_apc_dac (int inst_I2C, INT8U val);
int ux3328_get_apc_dac (int inst_I2C, INT8U *val);
int ux3328_get_table3(int inst_I2C, INT8U table_data[]);
int ux3328_set_table3_ex(int inst_I2C, INT8U table_data[]);	//初始化table3配置, 写入128~187字节配置数据,与函数ux3328_set_table3不同的是不写密码区，SLAVE_ADDRESS字节需要做bit0和bit1清零  

int ux3328_fit_apd (int inst_I2C, double kl, double kh, INT8U APD_DAC);
int ux3328_set_backup_A2_Table(int inst_I2C, int table);
int ux3328_check_eepflag(int inst_I2C, BOOL flag);
int ux3328_write_backup_A2_Table (int inst_I2C);
int ux3328_set_table3_backup(int inst_I2C, INT8U table_data[]);
int ux3328_set_lut_ex (int inst_I2C, INT8U LUT[60], int table);
int ux3328_check_A2flag(int inst_I2C, BOOL flag);

int ux3328_set_FactoryMode(int InstI2C);
int un3328_set_UserMode(int InstI2C);
int ux3328_set_los_dac(int inst_I2C, INT8U val);

int ux3328_get_biasct_r_dac (int inst_I2C, INT8U *val);
int ux3328_fit_APC_cClass (int inst_I2C, double Ibias10_33, double Ibias33_60, INT8U DAC_APC);


#endif  
