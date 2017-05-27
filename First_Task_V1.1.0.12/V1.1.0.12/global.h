/****************************************************************************
 *
 * File:                global.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         全局变量定义文件
 *
 * Time:                2013-07-20
 *
 * version:				V1.4.0.19
 * 
 * Update Description:  new document
版本历史
===================================
更新时间：2013-5-23
更新版本：V1.4.0.19
更新人：  Roger
更新内容：
1、补充错误代码-441~-447

===================================
更新时间：2013-5-23
更新版本：V1.4.0.18
更新人：  Roger
更新内容：
1、补充错误代码
ERR_UX3328_SUM_UPDATE       =-439,  //ux3328更新A2校验位异常
ERR_UX3328_SUM_CHECK        =-440,  //ux3328检查A2校验位异常

===================================
更新时间：2013-1-21
更新版本：V1.4.0.17
更新人：  Eric.Yao
更新内容：
1、补充错误代码
ERR_TES_TE			        =-438,	//高温TE过大 

===================================
更新时间：2012-12-20
更新版本：V1.4.0.16
更新人：  Eric.Yao
更新内容：
1、补充错误代码
ERR_TES_SE_MOD80_DAC		=-436,  //高温80度调制电流大于预设值

===================================
更新时间：2012-7-13
更新版本：V1.4.0.15
更新人：  Roger
更新内容：
1、add error code -433

===================================
更新时间：2012-5-31
更新版本：V1.4.0.14
更新人：  Roger
更新内容：
1、add error code -432 -118 -119

===================================
更新时间：2012-4-17
更新版本：V1.4.0.13
更新人：  Roger
更新内容：
1、新增产线ID：PRODUCTLINE_HJ

V1.4.0.12
1 add error code -430

V1.4.0.11
新增DRIVERCHIP_UX3328和CHIEFCHIP_UX3328

V1.4.0.10
1 add error code -324 -325 -326 -327 -328 -329 -429
2 Add EVB11

V1.4.0.9
新增产品类型 PN_TYPE_XFP
新增驱动芯片 DRIVERCHIP_GN2010
新增限放芯片 AMPLIFIERCHIP_SY88933V,AMPLIFIERCHIP_SY88903AL
****************************************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_ 

#include "CH341A_DLL.h"  
#include <windows.h> 
#include <visatype.h> 
#include <formatio.h>  
#include <utility.h> 
#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define PN_SFF					0
#define PN_SFP					1
#define PN_ONU					2 
#define PN_SDH  				3
#define PN_GBE  				4
#define PN_ONU_DDMI				5
#define PN_GPON_OLT_SFP			6		

//为了保持程序的延续性，暂时保留了#define的定义，后续不再添加
enum ePN_TYPE
{  
PN_TYPE_SFF,
PN_TYPE_SFP,
PN_TYPE_ONU,
PN_TYPE_SDH,
PN_TYPE_GBE,  
PN_TYPE_ONU_DDMI,
PN_TYPE_GPON_OLT_SFP,
PN_TYPE_GPON_ONT,
PN_TYPE_XFP,
PN_TYPE_10G_EPON_ONU,
PN_TYPE_10G_EPON_OLT,
PN_TYPE_SFP_PLUS
};
		
//for c class or i class
#define PN_c 	0 
#define PN_i 	1

//for RSSI
/* V5.0版本的Firmware，当RSSIMode为不同值时的处理情况
RSSIMode=0，V2.2a方式，有APD保护，无管脚RSSI中断，轮询ADC采样1次不平均，无温度补偿四阶拟合收端光功率；  
RSSIMode=1，V3.5a方式，无APD保护，有管脚RSSI中断，中断ADC采样1次不平均，无温度补偿四阶拟合收端光功率； 
RSSIMode=2，V3.5b方式，无APD保护，有管脚RSSI中断，中断ADC采样1次不平均，有温度补偿分段拟合收端光功率； 
RSSIMode=3，V4.3a方式，有APD保护，有管脚RSSI中断，中断ADC采样4次要平均，有温度补偿分段拟合收端光功率；
*/
#define RSSI_TYPE_DDMI		0	//RSSIMode=0
#define RSSI_TYPE_RSSI1		1   //RSSIMode=1
#define RSSI_TYPE_RSSI2		2   //RSSIMode=2
#define RSSI_TYPE_RSSI3		3   //RSSIMode=3

//laser driver type
#define DiverType_MAX3738  0
#define DiverType_MAX3656  1
#define DiverType_VSC7965  2
#define DiverType_VSC7967  3

//为了保持程序的延续性，暂时保留了#define的定义，后续不再添加
enum eDriverChip
{  
DRIVERCHIP_MAX3738,
DRIVERCHIP_MAX3656,
DRIVERCHIP_VSC7965,
DRIVERCHIP_VSC7967,
DRIVERCHIP_UX2222,  
DRIVERCHIP_UX2228,
DRIVERCHIP_GN2010,
DRIVERCHIP_NT22L33,
DRIVERCHIP_NT25L90,
DRIVERCHIP_UX3328,
DRIVERCHIP_UX3328S, 
DRIVERCHIP_GN1157
};

//======================并行测试相关定义======================//
#define CHANNEL_MAX 8 			//并行测试最大通道数

//并行测试树形控件图标,注意与控件的图标列表相关
#define TREE_IMAGE_OK     0		//并行测试树形控件图标
#define TREE_IMAGE_WARING 1		//并行测试树形控件图标
#define TREE_IMAGE_ERROR  2		//并行测试树形控件图标

//测试设备标签格式定义
#define  LAB_PM         "光功率计 %d %d %s"
#define  LAB_ATT_MAI    "主光路衰减器 %d %s %d"
#define  LAB_ATT_REF    "参考光路衰减器 %d %s"
#define  LAB_ATT_OLT    "OLT衰减器 %d %s"
#define  LAB_SEVB       "测试板 %d %s"
#define  LAB_BERT       "误码仪 %d %s"
#define  LAB_BERT_OLT   "OLT误码仪 %d %s"
#define  LAB_SPECTRUM   "光谱仪 %d %s %s"
#define  LAB_DCA        "示波器 %d %s %s"
#define  LAB_FIBER      "光纤跳线 %s"
#define  LAB_SW         "光开关 %d %d %d %s"
#define  LAB_SW_2        "光开关(波长计) %d %d %d %s"  
#define  LAB_CLOCK      "时钟切换板 %d %s %d"

//线程锁
int ThreadLock;               //用与光路切换
int ThreadLock_ModuleSN;      //用于扫描模块SN
int Thread_PSS_PowerPeter;	  //用于PSS多通道光功率计切换  
int ThreadLock_JW8504;        //用于多通道衰减器  
int ThreadLock_AG86120;       //用与光路切换,波长计  


//信息框颜色组
static int COLOR_CHAN[CHANNEL_MAX]={VAL_GREEN,   VAL_YELLOW,   VAL_CYAN,   VAL_MAGENTA,
							        VAL_DK_GREEN,VAL_DK_YELLOW,VAL_DK_CYAN,VAL_DK_MAGENTA};
//======================并行测试相关定义======================//

/***************仪器相关的变量*************************/
//for evb
//int USB_Handle;		//用于调测装置evb的设备句柄，可能会包括而evb1、evb5等 
//int EVB5_Handle;
int EVB6_Handle; 
int EVB8_Handle;
int inst_EVB16;
int inst_EVB22;
int INST_EVB[CHANNEL_MAX];
int inst_SEVB0027_4001;

//for power meter  
ViSession inst_OLP18C;
ViSession inst_HP8153A; 
ViSession inst_MT9810; 
ViSession inst_PMSII;  
ViSession instr_3700C;			/*3700C句柄**Eric.Yao*/  
ViSession instr_6810B; 			/*6810B句柄**Eric.Yao*/ 
ViSession INST_PM[CHANNEL_MAX];

//for DCA
ViSession inst_CSA8000;
ViSession inst_Ag86100; 

//for BERT
//int	GBERT_USBHandle;
//int	GBERT_ErrorCode;
//int	SBERT_USBHandle; 
ViSession inst_Ag86130A; 
ViSession inst_E5020;
ViSession inst_TG1B1A; 
//int	inst_B2BERT; 
//int	inst_SBERT2500; 
//int inst_SBERT10000;
//int inst_GBERT10000;
#define  SBERT_CAL   0.6
int INST_BERT[CHANNEL_MAX]; 
int INST_BERT_OLT[CHANNEL_MAX];

//for ATT  
ViSession inst_HP8156A;
ViSession inst_OLA55M;
ViSession inst_HP8157A; 
ViSession inst_FVA3100;  
//int EVOACOM; 
//int	inst_GVPM;  
int INST_ATT[CHANNEL_MAX];
int INST_ATT_OLT[CHANNEL_MAX];
int INST_ATT_MAIN[CHANNEL_MAX]; //双路衰减器中的主光路衰减器
int INST_ATT_REF[CHANNEL_MAX];  //双路衰减器中的参考光路衰减器

int INST_ATT_ONU[CHANNEL_MAX];

//for 光谱仪
ViSession inst_Ag86142;
ViSession Inst_AQ633X;
ViSession Inst_MS9740; 
ViSession Inst_AQ637X;
ViSession Inst_AG86120B;  

#define PowMeter_TYPE_NONE 	    -1
#define PowMeter_TYPE_OLP18C 	0
#define PowMeter_TYPE_HP8153A 	1  
#define PowMeter_TYPE_MT9810 	2
#define PowMeter_TYPE_PMSII 	3 
#define PowMeter_TYPE_EVBPD 	4 
#define PowMeter_TYPE_6810B		5		/*新增光功率计**Eric.Yao*/
#define PowMeter_TYPE_3700C		6		/*新增激光控制器**Eric.Yao*/
#define PowMeter_TYPE_LIV20		7		/*新增LIV2.0**Eric.Yao*/ 
#define PowerMeter_TYPE_PSS_OPM 8		

#define DCA_TYPE_NONE 			-1
#define DCA_TYPE_CSA8000		0
#define DCA_TYPE_Ag86100 		1 
#define DCA_TYPE_SEVB22 		2 
 
#define BERT_TYPE_NONE			-1   
#define BERT_TYPE_GBERT			0
#define BERT_TYPE_Ag86130 		1  
#define BERT_TYPE_OptoBERT 		2  
#define BERT_TYPE_SBERT 		3  
#define BERT_TYPE_E5020			4  
#define BERT_TYPE_B2BERT		5  
#define BERT_TYPE_SBERT2500     6
#define BERT_TYPE_TG1B1A		7
#define BERT_TYPE_SBERT10000    8  
#define BERT_TYPE_GBERT10000    9 

#define ATT_TYPE_NONE   		-1  
#define ATT_TYPE_HP8156A   		0
#define ATT_TYPE_OLA55M	   		1
#define ATT_TYPE_HP8157A		2
#define ATT_TYPE_FVA3100		3  
#define ATT_TYPE_EVOA			4
#define ATT_TYPE_GVPM			5
#define ATT_TYPE_JW8504			6

#define LASER_TYPE_FP			0
#define LASER_TYPE_DFB			1

#define SEVB_TYPE_NONE	   		-1  
#define SEVB_TYPE_SEVB1			1
#define SEVB_TYPE_SEVB2			2
#define SEVB_TYPE_SEVB3			3   
#define SEVB_TYPE_SEVB4			4 
#define SEVB_TYPE_SEVB5			5
#define SEVB_TYPE_SEVB6			6 
#define SEVB_TYPE_SEVB11		11
#define SEVB_TYPE_SEVB12		12
#define SEVB_TYPE_EVB27     	27 
#define SEVB_TYPE_EVB27_4001	2701 
#define SEVB_TYPE_B2BERT		101  

#define SPECTRUM_TYPE_NONE		-1
#define SPECTRUM_TYPE_AG8614X	0
#define SPECTRUM_TYPE_AQ633X 	1
#define SPECTRUM_TYPE_MS9740 	2
#define SPECTRUM_TYPE_AQ637X 	3
#define SPECTRUM_TYPE_AG86120B 	4

#define FIBER_TYPE_NONE	   		-1  
#define FIBER_TYPE_FIBER	   	0

//光开关
#define SW_TYPE_NONE	   		-1  
#define SW_TYPE_FSW			    0      //电子34所光开关
#define SW_TYPE_COFFSW		    1      //绵阳超光光开关
#define SW_TYPE_10G		    	2      //自制10G光开关
#define SW_TYPE_JHFSW			3	   //上海佳慧光开关
#define SW_TYPE_COFFSW02		4      //绵阳超光光开关

//时钟切换
#define CLOCK_TYPE_NONE	   		 	-1  
#define CLOCK_TYPE_SEVB0027_4001	0
/***************结束 仪器相关的变量********************/

//定义收端校准参数结构体
struct struRxCal 
{
BOOL 	RxCal_Flag;
char	RxCal_FileName[MAX_PATHNAME_LEN]; 
double 	RxCal_Power_In;
double 	RxCal_Power_Min; 
double 	RxCal_Power_Max;
double 	RxCal_Power; 
double 	RxCal_Power_Array[4];
//并行测试校准数据定义
BOOL flag[CHANNEL_MAX];
float power_in[CHANNEL_MAX];
float power_min[CHANNEL_MAX];
float power_max[CHANNEL_MAX];
float power[CHANNEL_MAX];
float power_array[CHANNEL_MAX*4];
};

//定义发端校准参数结构体
struct struTxCal 
{
BOOL 	TxCal_Flag;
char	TxCal_Power_FileName[MAX_PATHNAME_LEN]; 
char	TxCal_Power_FileName_Min[MAX_PATHNAME_LEN]; 
double 	TxCal_Power_ST;
double 	TxCal_Power_Min; 
double 	TxCal_Power_Max;
double 	TxCal_Power; 
double 	TxCal_Power_Array[4]; 
float   TxCal_Er_Slope;			//示波器ER校准因子
float   TxCal_Er_Offset;  		//示波器ER校准因子
BOOL flag[CHANNEL_MAX];
float power_st[CHANNEL_MAX];
float power_min[CHANNEL_MAX];
float power_max[CHANNEL_MAX];
float power[CHANNEL_MAX];      //校准值
float power_array[CHANNEL_MAX*4];

float minpower_st[CHANNEL_MAX];
float minpower_min[CHANNEL_MAX];
float minpower_max[CHANNEL_MAX];
float minpower[CHANNEL_MAX];      //校准值
float minpower_array[CHANNEL_MAX*4];
};

struct struTxCheck 
{
char  sn[50];	//点检模块序列号
float Power_Min;//点检模块系统读数下限
float Power_Max;//点检模块系统读数上限
float Power_Sys;//点检模块系统读数
char sn_array[CHANNEL_MAX][50];
float power_min[CHANNEL_MAX];
float power_max[CHANNEL_MAX];
float power_sys[CHANNEL_MAX];
};

/****************** Visa ***********************************/
extern BOOL Visa_Write(ViSession vi, ViBuf buf); 
extern BOOL Visa_OPC(ViSession vi, int Timeout);  //Timeout s
extern BOOL Visa_Query(ViSession vi, ViBuf buf); 
/******************end for Visa ****************************/

enum eChiefChip
{  
CHIEFCHIP_ADUC7020,
CHIEFCHIP_DS1856,
CHIEFCHIP_TINY13,
CHIEFCHIP_MEGA88,
CHIEFCHIP_ADUC7023,
CHIEFCHIP_UX3328,
CHIEFCHIP_UX3328S,
CHIEFCHIP_DS4830,
CHIEFCHIP_ATTINYDS4830   /***增加ATTINYDS4830芯片类型**Eric.Yao**20130704***/
};

enum eAmplifierChip
{  
AMPLIFIERCHIP_DEFAULT,
AMPLIFIERCHIP_SY88933V,
AMPLIFIERCHIP_SY88903AL
};

enum eMCU_CMD_TYPE
{  
MCU_CMD_DEFAULT,		//默认访问接口，可自己通过I2C地址访问
MCU_CMD_SUPERMASTER		//只能通过supermaster接口访问
};

int ProductLineID;						//产线ID 

#define PRODUCTLINE_CD	0		//表示成都
#define PRODUCTLINE_FZ	1		//表示福州
#define PRODUCTLINE_HJ	2		//表示成都黄甲 

enum eATE_ERR_CODE
{  
ERR_BOSA_TEST_IOP   		=-101,
ERR_BOSA_TEST_IO    		=-102,
ERR_BOSA_TEST_SENS  		=-103,
ERR_BOSA_TEST_GAIN  		=-104,
ERR_BOSA_TEST_VBR			=-105,
ERR_BOSA_TEST_IDARK			=-106, 
ERR_BOSA_TUNE_VAPD  		=-107, 
ERR_BOSA_TEST_SE       		=-108,
ERR_BOSA_START_BERT			=-109,
ERR_BOSA_READ_ERRBIT		=-110,
ERR_BOSA_APD_VBR			=-111,
ERR_BOSA_FQC_APD			=-112,
ERR_BOSA_FQC_LIV			=-113,
ERR_BOSA_FQC_ROSA			=-114,
ERR_BOSA_20Fiberlink		=-115, 
ERR_BOSA_TRACKERR_LOW	    =-116,
ERR_BOSA_TRACKERR_HIGH      =-117,
ERR_BOSA_TEST_POCMP			=-118,
ERR_BOSA_TEST_ITH			=-119, 
ERR_BOSA_TEST_VFIO			=-120, 
ERR_BOSA_TEST_IMIO			=-121, 
ERR_BOSA_TEST_POIO			=-122, 
ERR_BOSA_TEST_VFPO			=-123, 
ERR_BOSA_TEST_IOPO			=-124, 
ERR_BOSA_TEST_IMPO			=-125, 
ERR_BOSA_TEST_SEPO			=-126,
ERR_BOSA_TEST_TEMPER		=-127,
ERR_BOSA_NO_DEFINE			=-199, 
ERR_TRA_APD_EN				=-201, 
ERR_TRA_APD_VBR				=-202, 
ERR_TRA_APD_VBR_TARGET		=-203, 
ERR_TRA_APD_VBR_TEMPER		=-204, 
ERR_TRA_APD_VBR_COUNT		=-205, 
ERR_TRA_PASSWORD			=-207, 
ERR_TRA_I2C					=-208, 
ERR_TRA_OVER				=-212,   
ERR_TRA_SENS				=-213,   
ERR_TRA_RXCAL				=-214,  
ERR_TRA_SET_APD_PROTECT		=-215, 
ERR_TRA_DOWN_ONU			=-216,  
ERR_TRA_DOWN_GBE			=-217, 
ERR_TRA_DOWN_SDH			=-218, 
ERR_TRA_APD_WELD			=-220,
ERR_TRA_APD_5uA				=-221, 
ERR_TRA_TX_WELD				=-222, 
ERR_TRA_NO_DEFINE			=-299,  
ERR_TUN_READ_INNERSN		=-301,
ERR_TUN_RAPD   		        =-302,
ERR_TUN_TEMPER 		        =-303,
ERR_TUN_STATION		        =-304,
ERR_TUN_SDA					=-305,
ERR_TUN_OVER			    =-306, 
ERR_TUN_SENS_START		    =-307, 
ERR_TUN_AOP	        		=-308,
ERR_TUN_P0					=-309, 
ERR_TUN_P1					=-310, 
ERR_TUN_DAC_255or0			=-311,
ERR_TUN_ER					=-312,
ERR_TUN_O_EYE			    =-313,
ERR_TUN_O_MASK			    =-314,
ERR_TUN_CURRENT		        =-315,
ERR_TUN_SPECTRUM		    =-316,
ERR_TUN_SENS			    =-317,
ERR_TUN_CAL_TXPOWER			=-318, 
ERR_TUN_CAL_RXPOWER			=-319,
ERR_TUN_SAVE_DATA			=-320,
ERR_TUN_LD_SE_LOW			=-321, 
ERR_TUN_CLEAR_TXFAIL        =-322, 
ERR_TUN_CAL_RX_MONOTONY     =-323, 
ERR_TUN_BIASCURR            =-324, 
ERR_TUN_TXCROSS             =-325,
ERR_TUN_TEST_RXRESPON		=-326,
ERR_TUN_TEST_LOS			=-327,
ERR_TUN_TXRMSJ			    =-328,
ERR_TUN_APD                 =-329,
ERR_TUN_EAVOL               =-330,
ERR_TUN_EATEMP              =-331,
ERR_TUN_BIAS_MAX            =-332, 
ERR_TUN_Wavelenth           =-333,
ERR_TUN_TxOFFPower          =-334,  
ERR_TUN_NO_DEFINE			=-399,
ERR_TES_READ_INNERSN		=-401, 
ERR_TES_TEMPER 		        =-402, 
ERR_TES_AOP					=-403, 
ERR_TES_ER					=-404, 
ERR_TES_ER_LUT				=-405, 
ERR_TES_SD					=-406, 
ERR_TES_O_EYE			    =-407,
ERR_TES_SPECTRUM		    =-408, 
ERR_TES_OVER			    =-409, 
ERR_TES_STATION		        =-410,  
ERR_TES_TXDIS			    =-411, 
ERR_TES_O_MASK			    =-412,    
ERR_TES_INPUT_SN			=-413, 
ERR_TES_INIT			    =-414, 
ERR_TES_CAL_TXPOWER			=-415, 
ERR_TES_CAL_RXPOWER			=-416, 
ERR_TES_CURRENT             =-417,
ERR_TES_SAVE_EEPROM			=-418, 
ERR_TES_REPEAT				=-419, 
ERR_TES_STABILITY			=-420,
ERR_TES_FIBER_LINK			=-421, 
ERR_TES_SAVE_DATA			=-422, 
ERR_TES_SENS_START		    =-423, 
ERR_TES_SENS			    =-424, 
ERR_TES_ONU_TXSD			=-425,  
ERR_TES_CLEAR_TXFAIL        =-426,  
ERR_TES_CAL_RX_28dBm        =-427, 
ERR_TES_FIBLINKandSPECTRUM	=-428, 
ERR_TES_CAL_TEMPERATURE		=-429,
ERR_TES_AOP_MAX				=-430,
ERR_TES_CAL_RX_POWER2		=-431, 	//相同功率点测试3次，最大差异超出2dB
ERR_TES_CAL_RX_POW			=-432, 	//设置收端监控功率点异常
ERR_TES_AOP_TC				=-433,	//TC前后AOP差异过大
ERR_TES_SE_MIN				=-434,	//SE小于最小值
ERR_TES_SE_HALFROOM			=-435,	//高温SE小于常温的一半 
ERR_TES_SE_MOD80_DAC		=-436,  //高温80度调制电流大于预设值
ERR_TES_TXDIS_FAULT	        =-437,  //测试TxDis时 出现TxFault异常
ERR_TES_TE			        =-438,	//高温TE过大 
ERR_UX3328_SUM_UPDATE       =-439,  //ux3328更新A2校验位异常
ERR_UX3328_SUM_CHECK        =-440,  //ux3328检查A2校验位异常
ERR_UX3328_E2_CHECK         =-441,  //ux3328上电检查EEPROM标志位异常
ERR_TSSI         			=-442,  //TSSI检测异常 
ERR_TES_SDA		            =-443,  //测试SDA或LOSD异常
ERR_TES_SDD	                =-444,  //测试SDD或LOSA异常
ERR_TES_SDA_P			    =-445,	//测试指定功率点下SDA异常(通常-28dBm)
ERR_TES_NO_DEFINE			=-499, 
ERR_TES_TxOffPower			=-500, 
ERR_TES_Wavelenth			=-501,
ERR_TES_SetIbiasMax			=-502,
ERR_TUN_SetBurnin			=-503,
ERR_SYS_LINK_FAIL			=-701, 
ERR_SYS_LOST				=-702,
ERR_SYS_LOST_UP				=-703, 
ERR_SYS_LOST_DOWN			=-704, 
ERR_SYS_FIBER_LINK			=-705, 
ERR_SYS_NO_DEFINE			=-799, 
ERR_EEP_MON_TX				=-801,
ERR_EEP_MON_RX				=-802,
ERR_EEP_SHORT_VCC			=-803, 
ERR_EEP_SHORT_VAPD			=-804, 
ERR_EEP_SHORT_GND			=-805, 
ERR_EEP_MON_BIAS			=-806,  
ERR_EEP_MON_VCC				=-807,  
ERR_EEP_MON_LOS				=-808, 
ERR_EEP_MON_SD				=-809, 
ERR_EEP_MON_TEMPER			=-810, 
ERR_EEP_TX_FAULT			=-811,   
ERR_EEP_TX_DISABLE			=-812,   
ERR_EEP_NO_DEFINE			=-899, 
ERR_K3_CHECK_MAC   			=-901,
ERR_K3_CHECK_IAD_MAC  		=-902,
ERR_K3_DOWN_FIRMWARE  		=-903,
ERR_K3_PONSYS_INIT   		=-904,
ERR_K3_CHECK_TXPOW   		=-905,
ERR_K3_UPDATE_MAC   		=-906,
ERR_K3_MODULE_SN   			=-907, 
ERR_K3_WRITE_IADMAC   		=-908, 
ERR_K3_LOGIN       			=-909,
ERR_K3_DOWN_IMG       		=-910,
ERR_K3_FEED_VOL       		=-911,
ERR_K3_TEST_VOL       		=-912,
ERR_K3_GET_VERSION     	 	=-913,  
ERR_K3_CHECK_HARVER      	=-914, 
ERR_K3_CHECK_SOFVER      	=-915, 
ERR_K3_CHECK_HARLOGO     	=-916,
ERR_K3_SET_PROTOCOL  	 	=-917,
ERR_K3_TEST_VOICE    	 	=-918,
ERR_K3_CHECK_STATUS  	 	=-919,
ERR_K3_ONLINE               =-920,
ERR_K3_GET_ID               =-921,
ERR_K3_SET_AU               =-922,
ERR_K3_CHECK_ONLINE         =-923,
ERR_K3_STREAM_TEST          =-924,
ERR_K3_NO_DEFINE   			=-999, 
ERR_NO_DEFINE				=-1
} eATE_ERR_CODE;

enum eRSSI_TIME
{  
RSSI_TIME_N1N3,
RSSI_TIME_N2, 
RSSI_TIME_L2L3, 
RSSI_TIME_L1L2   
};

/*
客户定义变量，0表示标准品
1表示是AL，现在主要用于区分是否是AL产品，innersn是否需要特殊处理，其他产品暂时全部定义为标准品
*/
enum eCUSTOMER
{  
CUSTOMER_STANDARD, 	//标准产品    
CUSTOMER_AL,		//ALCATEL定制产品
CUSTOMER_AP,		//Alphion定制产品
CUSTOMER_GW,
CUSTOMER_01,		//01 定制产品
CUSTOMER_OP			//OP定制产品
};

//**************光路校准点检定义**************//
int phCaliT;  		//发端光路校准界面
int phCaliLimit;  	//光路校准门限配置界面
int phCheckT;		//发端光路点检界面  
int phCali;			//光路校准界面，包括发端和收端部分

#define CALIBRATION_MAX 120 //120分钟，2小时
#define CALI_OSA_MAX 240	//240分钟，4小时 

enum enum_CALIBRATION_TYPE
{
CALI_TYPE_RX,					//收端光路校准，GPON OLT ATE使用情况下是收端参考光路校准  	
CALI_TYPE_TX,
CALI_TYPE_TX_CHECKUP,
CALI_TYPE_RX_MAINLOCK,			//收端主光路锁定模式校准，GPON OLT ATE使用
CALI_TYPE_RX_MAINFREE,			//收端主光路普通模式校准，GPON OLT ATE使用
CALI_TYPE_RX_OLT,				//OLT收端光路校准，EPON ONU ATE使用  
CALI_TYPE_LIV_AOP,
CALI_TYPE_LIV_ITH
}; 

static ViString CALI_TYPE[] = {
"RX_CALI", 
"TX_CALI",
"TX_CHECKUP",
"RX_CALI_MAINLOCK", 
"RX_CALI_MAINFREE",
"RX_CALI_OLT",
"LIV_AOP",
"LIV_ITH"
};
//**************光路校准点检定义**************//

//======================程序界面定义======================// 
int showphID;   //0=主程序界面，1=高级功能界面，2=校准配置界面, 3=变更用户， 4=标准模块点检门限界面, 5=参数配置界面 

#define License_Flag_INIT    -1  //开启程序时的初始值
#define License_Flag_panMain  0
#define License_Flag_panADV   1
#define License_Flag_panCALI  2 
#define License_Flag_USER  	  3
#define License_Flag_panPara  5 
#define License_Flag_panInst  6  //设备地址配置界面
//======================程序界面定义======================// 

//nt25L90寄存器定义
#define NT25L90_REG_NUMBER  21
#define NT25L90_MON_NUMBER  16

static ViString NT25L90_REG_NAME[NT25L90_REG_NUMBER] = {
"TXSET0",
"TXSET1",
"IMODSET",
"TCSTART",
"TSLOPE",
"SPARE",
"APCSET",
"BIASINIT",
"APCCTRL0",
"MDMAX",
"BIASMAX", 
"RXSET0", 
"RXSET1", 
"LOSLEVEL", 
"CONTROL0", 
"BIASDAC MSB", 
"BIASDAC LSB", 
"WATCHDOG", 
"STATUS0", 
"STATUS1", 
"CONTROL1" 
};

static ViString NT25L90_MON_NAME[NT25L90_MON_NUMBER] = {
"TEMP MSB",
"TEMP LSB",
"TX_POWER MSB",
"TX_POWER LSB",
"TX_BIAS MSB",
"TX_BIAS LSB",
"TX_MOD MSB",
"TX_MOD LSB",
"RX_POWER MSB",
"RX_POWER LSB", 
"VDD_TX MSB", 
"VDD_TX LSB", 
"VDD_RX MSB", 
"VDD_RX LSB", 
"VDD_DIG MSB", 
"VDD_DIG LSB" 
};

void Update_RESULTSInfo(int panel, int control, unsigned char *resultstr, BOOL bInsert);  
void Insert_Info(int panel, int control, unsigned char *resultstr);	 //插入信息到指定控件，区别函数Update_RESULTSInfo不带参数bInsert 
void Update_Info(int panel, int control, unsigned char *resultstr);	 //更新信息到指定控件，区别函数Update_RESULTSInfo不带参数bInsert 
void set_led_status(int panel, int control, int status_led, int status_cursor);

#ifdef __cplusplus
    }
#endif
	
#endif  	

