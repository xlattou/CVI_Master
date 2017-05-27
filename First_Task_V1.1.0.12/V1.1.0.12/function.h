/****************************************************************************
 *
 * File:                function.h
 *
 * Author:              Roger Li
 *
 * Description:         自动调测程序基本功能代码
 *
 * Time:                2009-11－25
 *
 * version:				v1.2.0.0
 *
****************************************************************************/
#ifndef _FUNCTION_H_
#define _FUNCTION_H_ 

#include "cvidll.h" 
#include "gbert.h"
#include "csa8000.h"
#include "ag86100.h"
#include "DWDM ONU Parallel ATE.h"  
#include "COFFSW.h"
#include "fsw.h"
#include "SEVB0027_4001.h"  
#include "JHFSW.h" 
#include "AG86120B.h"  
												 												    
#define	SOFTVER  "V1.1.0.12"
#define SOFTDATE "2017.05.06"

#define DATETIME_FORMATSTRING "%Y%m%d%H%M%S"  

#define WAVELENGTHCHANNEL 4

#define TunWLOffset		0.055		//波长调试时的中心波长偏移量，单位nm；
#define TunWLOffset2	0.075		//波长调试时的中心波长偏移量，单位nm；微调时候使用；
#define TunWLAccuracy	0.005		//波长调试的精度，单位nm；
#define Lambda10		0.12		//100%-10%波长lambda判定量，单位nm；


int panSN;  


char g_ConfigFileName[MAX_PATHNAME_LEN]; //for config file
char g_InstFileName[MAX_PATHNAME_LEN]; //for Instrument Add file  

static int gCtrl_BOX[CHANNEL_MAX] = {PAN_MAIN_TEXTBOX_CHAN0, PAN_MAIN_TEXTBOX_CHAN1, PAN_MAIN_TEXTBOX_CHAN2, PAN_MAIN_TEXTBOX_CHAN3, 
							 		 PAN_MAIN_TEXTBOX_CHAN4, PAN_MAIN_TEXTBOX_CHAN5, PAN_MAIN_TEXTBOX_CHAN6, PAN_MAIN_TEXTBOX_CHAN7}; 
static int gCtrl_BIN[CHANNEL_MAX] = {PAN_MAIN_BIN_CHAN0, PAN_MAIN_BIN_CHAN1, PAN_MAIN_BIN_CHAN2, PAN_MAIN_BIN_CHAN3, 
							 		 PAN_MAIN_BIN_CHAN4, PAN_MAIN_BIN_CHAN5, PAN_MAIN_BIN_CHAN6, PAN_MAIN_BIN_CHAN7}; 
static int gCtrl_LED[CHANNEL_MAX] = {PAN_MAIN_LED_CHAN0, PAN_MAIN_LED_CHAN1, PAN_MAIN_LED_CHAN2, PAN_MAIN_LED_CHAN3, 
							 		 PAN_MAIN_LED_CHAN4, PAN_MAIN_LED_CHAN5, PAN_MAIN_LED_CHAN6, PAN_MAIN_LED_CHAN7}; 

static int gCtrl_StartBut[CHANNEL_MAX]={PAN_MAIN_But_Star_Ch0,PAN_MAIN_But_Star_Ch1,PAN_MAIN_But_Star_Ch2,PAN_MAIN_But_Star_Ch3};

BOOL	Channel_Sel_Flag[CHANNEL_MAX];
BOOL 	WaitFlag[CHANNEL_MAX];

//定义功能配置Config结构体
struct struConfig 
{
int		PNIndex;
char	PN[50];
char	BOM[50];
char    EED[10];
BOOL	DT_OSA_WL_Test;
BOOL	DT_Tun_RX;  
BOOL	DT_Tuning;
BOOL	DT_Test_RX;     
BOOL	DT_Test;
BOOL	DT_QATest;
BOOL	DT_OQA;		/***添加OQA测试选项**Eric.Yao***/
BOOL	DT_Agin_Front;
BOOL	DT_Agin_Back;
BOOL	DT_Tun_Reliability;
BOOL	DT_Test_Reliability;
BOOL	DT_Test_Upstream;   
char	DT_Flag[30];
BOOL  	Temper_Low;
BOOL	Temper_Room;
BOOL	Temper_High;
char	Temper_Flag[10];
BOOL    Sel_T_AOP; 
BOOL    Sel_T_ER;
BOOL    Sel_T_TxDis;
BOOL    Sel_T_Eye;
BOOL	Sel_T_Mask;
int     Sel_T_Mask_Real;	//0=测门限值， 1= 测真实值 
BOOL    Sel_T_Spectrum;
BOOL 	Sel_R_Sens;
BOOL 	Sel_R_Over;
BOOL 	Sel_R_SDHys;
BOOL 	Sel_Calibrate; 
BOOL 	Sel_Calibrate_Test; 
BOOL 	Sel_EE_P;		//EEPROM 写保护检查  
BOOL 	Sel_TxSD; 		//发端SD测试
BOOL    Sel_T_Wavelength;
BOOL    Sel_T_Upstream;  

//具体值可选
BOOL Sel_R_Sens_Detail; 
BOOL Sel_RxSD_Detail;

int		PN_TYPE;    //SFF=0, SFP=1
char    batch[50];  
char    firstsn[50];
char    lastsn[50];
BOOL check_sn; 	    //根据工单判定是否检查SN范围
enum eServerType servertype;
enum eCUSTOMER   CUSTOMER;

//高级功能配置项   
BOOL isNPI;	        	//NPI测试标志位：0，不是NPI，1是NPI
BOOL isONUtoOLT;		//启用ONU传输测试功能
BOOL isQAUpdateSN;		//QA测试写SN到EEPROM  
BOOL isScanModuleSN;	//测试工序是否扫描模块SN标志位，用于多通道测试工序进行重测或维修测试时，不需要再次扫描模块SN
BOOL isQANoCheckBOM;
int  isSDRealFlag;			//测试具体的SDA和SDD指标 

double AOP_AginDelta;
double WL_AginDelta;
double WLMeterOffset;
//产品功能配置
//int  isTSSIflag;		//启用TSSI功能测试，目前只有SFRM0050的固件支持

//占空比配置
int Duty_Cycle_Val;
								 
} my_struConfig;
				  
//定义调测参数结构体 
struct struDBConfig	  
{
char    BarCode[50];
int     SNLength;  
char    FirmwareVer[50]; 
char 	SFRM_Number[50];	/***增加项目号**Eric.Yao***/
char	FEP_Number[50];
double  TemperMin;
double 	TemperMax;
double  TxPowMin;
double  TxPowMax;
double  TxErMin;
double  TxErMax;
double 	TxTjMax;
double	TxPPjMax;
double 	TxRiseMax;
double	TxFallMax;
double	TxDisPowMax;
double	TrackingErrorMax;
double	MaskMargin;
char 	MaskName[50];
double	PathPenaltyMax;
double	PathPenaltyTime;
//收端参数
double	RxAmpMin;
double	RxAmpMax;
double 	RxTjMax;
double	RxPPjMax;
double 	RxRiseMax;
double	RxFallMax;
double  Over;			//过载测试值
double	OverTime;
double  CenterSens;				//中间光误码测试参数
double	CenterSensTime;
double  Sens; 			//灵敏度测试值 
double	SensTime;
double  SDA;
double  SDD; 
double  SDHysMin;
double  SDHysMax;
//光谱参数
double	PeakWavelengthMin[WAVELENGTHCHANNEL];
double	PeakWavelengthMax[WAVELENGTHCHANNEL];
double	Sigma;
double	BandWidth;
double	SMSR;
//电压电流
double	TxVMax;
double	TxIMax;
double	RxVMax;
double	RxIMax;
//功能配置参数
float  TxCalMin;	//发端校准门限下限
float  TxCalMax;	//发端校准门限上限

double TEMin;
double TEMax;

int     TxWavelength;
int 	RxWavelength;
int     WavelengthChannel[WAVELENGTHCHANNEL];

float 	APD_Kl;
float	APD_Kh;

float	LOS_Kl;
float	LOS_Bl;
float	LOS_Kh;
float	LOS_Bh;

float	Temper_K;
float	Temper_B;
float	Temper_Kl;
float	Temper_Bl;

double	TxOffPowerMin;		//关断深度最小值
double	TxOffPowerMax;		//关断深度最大值 
double	WavelengthDelta;	//99%与1%的差值； 

} my_struDBConfig;

//定义VSC7965结构体 
struct  struDBConfig_VSC7965	  
{
double	Ratio_L;	//消光比补偿系数，低温到常温部分
double	Ratio_H;	//消光比补偿系数，常温到高温部分
double	Ratio_H_Ex;	//消光比补偿系数，高温到扩展高温部分
char BurstFlag[50];				//BurstON状态的电平状态
int  UpdatefOVERDFlag;
int  VSC7965_FLASH[26];			//寄存器配置
} my_struDBConfig_VSC7965;

//定义固件结构体
struct struFirmware
{
char SFRMNum[50];					//firmware项目编号
char Firmware_Ver[50];				//firmware版本编号
int  NT25L90_REG_EEPROM[14];		//NT25L90 对应寄存器 60h~6Dh 14byte mega88 table1部分
int  NT25L90_REG_RAM[14];			//NT25L90 对应寄存器 60h~6Dh 14byte mega88 table2部分
unsigned char MEGA88_EEPROM[512]; 
unsigned char TINY13_FLASH[1024];	//调试的firmware 
int  VSC7965_FLASH[26]; 
} my_struFirmware;

//OSA 波长测试
struct struDBConfig_OSATEST	  
{
 char	DAC_APC[50];
 char	DAC_MOD[50];
 char   WAVELENTH_RANGE[50];
 char   str_WaveRecorder[200];
 float  WaveLenth_Max;
 float  WaveLenth_Min;
 float Tec_Temper_Max;
 float Tec_Temper_Min;
 float Temper_Min;
 float Temper_Max;
} struDBConfig_OSATEST;  

//定义消光比补偿结构体 
struct struDBConfig_ERCompens	  
{
enum eDriverChip DriverChip;  
enum eChiefChip	 ChiefChip;
char    DS1856_TYPE[50];
char	Rb_Add[50];
char	Rm_Add[50];
double	Rs;	//与DS1856串联的电阻
double	Rp; //与DS1856并联的电阻
float	Im10_33;	//消光比补偿系数
float	Im60_33;	//消光比补偿系数
float	Im82_60;	//消光比补偿系数
float	AOP_Slope;			//光功率补偿因子，用于计算发调试消光比
float	AOP_Offset;			//光功率补偿因子，用于计算发调试消光比   
float   ER_Slope;			//消光比补偿因子，用于计算发调试消光比   
float	ER_Offset;			//消光比补偿因子，用于计算发调试消光比   
float   se;   
float   CurrentLimit_80;	//高温80度调制电流门限
float   IBias_Max;
float	IBias_MaxDAC;		//TWDM ONU 最大电流限制用DAC;
double	Ib10_33;			//Bias补偿系数
double	Ib60_33;			//Bias补偿系数
double	Ib82_60;			//Bias补偿系数
double  Ib105_82;			//Bias补偿系数
double  Ib030_10;			//Bias补偿系数

} my_struDBConfig_ERCompens;  

//定义监控参数结构体 
struct struDBConfig_Monitor	  
{
float	Temper_Prec;
float 	Vcc_Prec;
float	I_Bias_Prec;
float	Tx_Pow_Prec;
float	Rx_Pow_Prec;
int		CaliNumber;
float	CaliPoint[30];
int		CaliNumber_Test;
float	CaliPoint_Test[30];
float	RxPow_Mid;
} my_struDBConfig_Monitor;
// 定义高级功能配置校准结构体信息
struct struSupFunCfgCali
{
	double Wave_DAC[4];    // 校验时放置输入的DAC
	double Temperature[2]; // 存放温度
	double WaveLen[2];     // 存放波长
	float DAC_Wave_k[2];   // 拟合直线的斜率k，先是dac-wave后是dac-tempe
	float DAC_Wave_b[2];   // 拟合直线的b，先是dac-wave后是dac-tempe
	int Model_power_on[8]; // 模块上电开关标志位
} my_struSupFunCfgCali;

double m_MyDACData[2];     // 保存手动校准的DAC数值
double m_MyWaveData[2];    // 保存新被校模块的波长
double m_MyTemperature[2]; // 保存基准温度
float  m_MyWaveTempePara[2];
float  m_MyWaveDacPara[2];

//定义EEPROM结构体 
struct struDBConfig_E2	  
{
int		E2_Flag;  	//A0,A2标志 0：A0[0-127], 1:A2[0-127]
char    E2_str[128][5];
int		A0[128];
int		A2[128];
int		A2_Table0[128];
} my_struDBConfig_E2;

//定义设备配置结构体
struct struInstrument 
{
//测试通道配置
int     Channel;
int     ChannelEn;
//for power meter
int 	PM_TYPE;    //1=HP8153A; 0=OLP18C, 2=MT9810; -1=no power  
int     PM;
char	SN_PM[30];    
//for bert
int 	BERT_TYPE_ONU;  //0=GBERT, 1=Ag86130, -1=no BERT  
char	BERT_ONU[30]; 
//for bert olt
int     BERT_TYPE_OLT;
char	BERT_OLT[30]; 
//for att
int 	ATT_TYPE_ONU;  	//0=8156, 1=OLC65, 2=8157, -1=no att 3=FVA3100 
char	GVPM_ONU[30]; 
int     ATT_COM;  
//for att
int 	ATT_TYPE_OLT;  	//0=8156, 1=OLC65, 2=8157, -1=no att 3=FVA3100 
char	GVPM_OLT[30]; 
//for evb
int 	SEVB_TYPE;	//1=SEVB1, 2=SEVB2...
char	SEVB[30]; 
char	Fiber[30];
//for DCA
int 	DCA_TYPE;  	//0=CSA8000, 1=Ag86100, -1=no BERT    
char	DCA[30];
char	SN_DCA[30]; 
//for 86142
int     SPECTRUM_TYPE;
char	SPECTRUM[30];
char	SN_SPECTRUM[30];
//光开关
int 	SW_TYPE;   //0=FSW, -1=不选光开关    
int     SW;		   //光开关端口
int     SW_CHAN;   //光开关使用通道
char    SW_SN[30]; //光开关序列号，10G光开关使用
int		SW_Handle;  

//光开关
int 	SW_TYPE_2;   //0=FSW, -1=不选光开关    
int     SW_2;		   //光开关端口
int     SW_CHAN_2;   //光开关使用通道
char    SW_SN_2[30]; //光开关序列号，10G光开关使用
int		SW_Handle_2; 

//时钟切换
int 	CLOCK_TYPE;//0=SEVB0027-4001, -1=不选时钟切换
char	CLOCK[30]; //时钟切换板
int     CLOCK_CHAN;//时钟切换通道

int     SBert_Rate;
int     SBert_PRBS;
int     GBert_Rate;
int     GBert_PRBS;
}INSTR[CHANNEL_MAX];

//定义调测结果结构体 
struct struTestData	  
{
char	PN[50];				   	//PN
char    Ver[10];			   	//产品版本
char	TestDate[50];          	//日期
char	ModuleSN[50];		   	//模块的序列号
char	OpticsSN[50];		   	//BOSA的序列号
float   Temperatrue;			//模块温度
double  AmbientTemperatrue;     //环境温度
int 	ErrorCode; 				//错误代码
char	Status[10];				//测试结果PASS FAIL
double  AOP[WAVELENGTHCHANNEL];	//光功率
double  ER[WAVELENGTHCHANNEL];	//消光比
double  Over;					//过载测试值
double  OverTime;
double  CenterSens;				//中间光误码测试参数  
double	CenterSensTime;   

double  Sens; 					//灵敏度测试值 
double  SensTime;
int 	ucRb[WAVELENGTHCHANNEL];					//Rb的数字电位器设置值
int 	ucRm[WAVELENGTHCHANNEL];//Rm的数字电位器设置值  
double  dRb;					//Rb的电阻值kohm  
double	dRm;					//Rm的电阻值kohm
double  TxOffPower[WAVELENGTHCHANNEL];				//TxDis光功率 
double  SDA;
double  SDD; 
double  SDHys; 
int 	ucRsd;					//Rsd的数字电位器设置
double	dRsd;					//Rsd的电阻值kohm
double  RxI[WAVELENGTHCHANNEL];
double  RxV;					//实际记录的是调试过程中测试的Se   
double  RxTf;					//实际记录的是发端关断厕所的发端监控值
double	TxI[WAVELENGTHCHANNEL];
double	TxV;  					//实际记录的是调试成功后的Ibias电流 
double  TxPPj;
double	TxRSj;
double  TxTj;
double 	TxRise;
double	TxFall;
double  TxPCTCROss;
//光谱数据
double	PeakWL_DutyRatio01[WAVELENGTHCHANNEL];
double	PeakWL_DutyRatio10[WAVELENGTHCHANNEL]; 
double	PeakWL_DutyRatio99[WAVELENGTHCHANNEL];
double  Peakwavelength[WAVELENGTHCHANNEL];
BOOL	WL_ReTun_FLAG;
double	Sigma;
double	Bandwidth;
double  SMSR;
int     ErCoverFlag;		//消光比重调标志 
int     AOPCoverFlag;		//光功率重调标志  
int     RxPow_num;
double	RxPow_mon[30];
int     FiberTime; 
float   RxAmp;				//电口幅度，但实际存储的是发端校准时TxDisable状态下的ADC值
double  RxTj;				//电口抖动，实际存储的是调试后重新插拔光纤的功率值
float   TE;					//trackingerror
double	ReturnLoss;			//回损
//APD调试结果
float	Vapd_D_Temper;		//APD调试温度
float	Vapd_D_Target;		//待调电压
float	Vapd_D_Reality; 	//实调电压
float	Rapd1;				//电阻1,单位kohm, 当用DS1856时存放对应的电阻值
float	Rapd2;				//电阻2,单位kohm, 当用DS1856时存放对应的step	 
double	Vapd_DAC;
		
INT8U   FirmwareVer; 			//从模块读取的Firmware版本
double  pathpenalty;			//通道代价，实际保存MaskMargin 
double  pathpenaltytime;		//通道代价，实际保存Tec Current 
char    curve[150000];		//大小可能不确定，定义一个默认的值
int     curvesize;

float   Vcc;  
int     TecDAC[WAVELENGTHCHANNEL];				//TEC DAC设置值
double  TecCoreTemper[WAVELENGTHCHANNEL];		//调试TEC时的内核温度

int		TxOffDepthDAC[WAVELENGTHCHANNEL];		//关断深度Bias
double	TxOffDepthTemp[WAVELENGTHCHANNEL];		//关断深度内核温度 
char    APD_Info[4000];

float	A2_temperatrue[WAVELENGTHCHANNEL];
float	A2_Vcc[WAVELENGTHCHANNEL];
float	A2_Ibias[WAVELENGTHCHANNEL];
double	A2_CaseTemperatrue;
float	A2_TxPower[WAVELENGTHCHANNEL];

double	ApcCoreTemper[WAVELENGTHCHANNEL];		//APC内核温度  
double	ModCoreTemper[WAVELENGTHCHANNEL];		//MOD内核温度  
double	TecCurrent[WAVELENGTHCHANNEL]; 
double	LDTemper[WAVELENGTHCHANNEL]; 

BOOL	RetuningAopFlag; 

//Tun WL 设置PID次数纪录 Or Tun TxOff Power设置Bias次数； 
int PointCount; 					

//耗时统计参数
int Time_Start;
int	Time_End;
};

struct  struCal_OLT 
{
int 	flag[CHANNEL_MAX];						//校准标志
char	FileName[MAX_PATHNAME_LEN]; 
float 	Power_In[CHANNEL_MAX];					//测试功率点=OLT灵敏度 + OLT灵敏度测试余量 	
float 	Power_Min[CHANNEL_MAX]; 					//校准值最小值 
float 	Power_Max[CHANNEL_MAX];					//校准值最大值 
float 	Power[CHANNEL_MAX]; 						//校准值
float 	Array[CHANNEL_MAX*5];  	
float   time[CHANNEL_MAX];						//测试时间
float  	OLT_sens[CHANNEL_MAX];		//OLT灵敏度
float  	OLT_Reserves[CHANNEL_MAX];	//OLT灵敏度测试余量 
float  	OLT_sens_min[CHANNEL_MAX];	//OLT灵敏度下限
float  	OLT_sens_max[CHANNEL_MAX];	//OLT灵敏度上限
float  	OLT_sens_in[CHANNEL_MAX];	//OLT灵敏度测试开始点
float  	OLT_sens_arr[CHANNEL_MAX*5];
char	OLT_sens_file[MAX_PATHNAME_LEN];
};

struct struRxCal sRxCal;		//收端光路校准结构
struct struTxCal my_struTxCal;	//发端光路校准结构
struct struTxCheck sTxCheck;	//发端光路点检结构体

struct struCal_OLT my_struCal_OLT;	//ONU传输性能结构体

struct struAg86100 my_struAg86100;
struct struCSA8000 my_struCSA8000;

int DB_Initial(void);		        //初始化数据库 
int DB_Get_BOM(int panel, int control);
int DB_Get_EED_Ver(int panel, int control); 
int DB_Get_EEPROM(void);
int DB_Read_Spec_ATE_Item(void);
int DB_Get_PN_TYPE(void);
int DB_Get_ConfigInfo(void); 
//int DB_Get_Spec_ERCompens(void);
int DB_Get_Config_Monitor_Info (void);
int DB_Read_Spec_Monitor_Ex(int Flag);
int DB_Close(void); 
int DB_save_txcali(int channel);		//保存发端光路校准结果
int DB_get_txchecklimit(int channel);	//显示发端点检模块门限配置
int DB_save_txcheckup(int channel, char sn[], double val);	//保存发端光路点检结果  
int DB_Get_Spec_CSA8000(void);
int DB_Get_Spec_Ag86100(void);
int DB_Save_Results_ATE(struct struTestData data, struct struProcessLOG prolog); 
int DB_Seach_Optics_Data_Vbr(char sn[], double *Vbr, double *Iop, double *Vapd); 
int DB_Update_Optics_Data(struct struTestData data);
int DB_Search_Tracking_BareBOM (char SN[], char bom[], char batch[]);
int DB_get_cali(enum enum_CALIBRATION_TYPE cali_type, int channel);
int DB_save_rxcali(enum enum_CALIBRATION_TYPE cali_type, int channel); 
int DB_get_room_aop(char SN[], float *val);
int DB_Read_Spec_TxCal (void);
int DB_Get_AutoDT_Spec_UX3328(void);
int DB_Get_AutoDT_Spec_NT25L90(void);
int DB_Get_AutoDT_Spec_VSC796x(void);  
int DB_GeT_Spec_Image_Tiny13(void);
int DB_Get_Spec_Image_Mega88(void);
int DB_Get_Tuning_AOP (char *OpticsSN, float *aop);
int DB_GET_Wavelength(void);
int DB_GET_UX3328S_BENpch (const char pn[], const char bom[]);
int DB_READ_AutoDT_Spec_ERCompens (void);
int DB_Read_Spec_Tracking_Ex1(void) 
;
BOOL DB_GET_Spec_ERCompens_Ex(void);
int DB_Read_Spec_TxCal (void);
int DB_Read_pch (char pn[], char bom[], int panel, int control_pch); 
int DB_Read_Order (char order[]);
int DB_Get_Room_TEC (char *OpticsSN, int *ROOM_TEC, double *ROOM_Temper);
int DB_Get_Room_TxOffDepth (char *OpticsSN, int *ROOM_DAC, double *ROOM_Temper);
int DB_Get_Room_Vapd_DAC (char *OpticsSN, int *ROOM_TEC, double *ROOM_Temper);
int DB_Get_AginFront_TxPower (char *OpticsSN, double *val);
int DB_Get_AginFront_WaveLength (char *OpticsSN, double *val); 

int DBOA_Init(void);
int DBOA_Close(void);
int DBOA_Read_pch (char pn[], char bom[], int panel, int control_pch);

int DBOA_Read_Order (char order[]);	
BOOL DB_GET_Firmware_Ver(void);

// 功能单元函数
int GetConfig(void);
int SetConfig(void);
int GetConfig_Inst(void);
int SetConfig_Inst(void);
void InsertTree(int panel, int control, struct struInstrument *localInst, int collapsed);	//collapsed是否显示子项目
void Update_Config_Ver(int panel, BOOL updateVer);

void readconfigfilename(void); 
int SetCaliConfigFile(void); //更新参数到配置文件
int GetCaliConfigFile(void); //读取配置参数
int Initial(int panel, int panMain, int *error_channel, int ini_Flag);
int quittest(void); 
int SET_EVB_SHDN(int channel, unsigned char SHUTValue);
int Get_Power(int channel, double *Power);
int SaveDataBase(struct struTestData data, struct struProcessLOG prolog); 

/*
mode_B2BERT:工作模式设置，用于区分B2BERT的设置方式
-1：设备初始化话设置模式，需要对B2BERT重新配置
0：校准收端功率模式，使用B2BERT时只需要设置ONU0发光
1：校准收端功率模式，使用B2BERT时只需要设置ONU1发光  
*/
int Init_BERT (int channel); 
int BERT_Stop (int channel);
int BERT_Start(int channel, double testtime, int isTestErrbitFlag);
int BERT_Read (int channel, unsigned int *ErrorCount);
int Read_Error_Bit(const int channel, unsigned int *ErrorCount, int TestFlag, double testtime, int InfoFlag, char ErrInfo[],char ErrInfo_Ex[]);

int Set_Att(int channel, double val); 
int Set_Att_SD(int channel, double AttValue);
int Set_Power_Wavelength(int channel, int Wavelength);
int Set_Power_Unit(int channel, int Unit);
int SET_EVB_TxDis(int channel, unsigned char TxDis);
int Read_Tx_Fail(int channel, BYTE *Fail);
int Read_AOP(int channel, double *AOP);
int Read_AOP_Ex(int channel, double *AOP);
int Read_ER(int channel, double *ER);
int READ_DCA_PJ(int DCAType, double *PJ);  
int READ_DCA_RJ(int DCAType, double *RJ); 
int READ_DCA_Rise(int DCAType, double *Rise);
int READ_DCA_Fall(int DCAType, double *Fall); 
int READ_DCA_PCTCROss(int DCAType, double *PCTCROss);   
int READ_DCA_MaskHits(int DCAType, int WaveForms); 
int Set_DCA_Clear(int DCAType); 
char *revstring(char *str, int length); 


/***光开关控制部分**Eric.Yao**20131223***/ 
int Fiber_SW_Init(int FSW_TYPE, int COMIndex, int *FSW_10G_Handle, char FSW_SN[30]);
int Fiber_SW_Close(int FSW_TYPE, int COMIndex, int FSW_10G_Handle);
int Fiber_SW_SetChannel(int FSW_TYPE, int COMIndex, int FSW_10G_Handle, int channel);
int set10GSwitch(int FSW_10G_Handle, int channel);

//调测功能单元函数
int Init_Test(int channel, int panel, struct struTestData *data);//配置模块进入调测状态 
int Init_Test2(int channel, int panel, struct struTestData *data);
int GetBosaSN(int channel, struct struTestData *data);	        //获取模块ID
int	CalTemperature (int channel);		//温度校准
int testTemperature(int channel, int panel, struct struTestData *data); 	//获取调测温度
int checktestbed(int channel, int *val);						//检查测试板上是否正常接入模块，1：模块已接入，0：模块没有接入
int tuningAOP(int channel, struct struTestData *data);
int tuningAOP_AginBack(int channel, struct struTestData *data);
int tuningER(int channel, struct struTestData *data);
int tuningER_Beforehand(int channel, struct struTestData *data);	
             
int tuningAOP_ER(int channel, struct struTestData *data); 
int tuningSD(int channel, struct struTestData *data); 
int tuningSD_SDA(int channel, struct struTestData *data);
int tuningSD_SDD(int channel, struct struTestData *data);
int testFixOver(int channel, struct struTestData *data,struct struProcessLOG *prolog);
int testFixOver_Start(int channel, int *StartTime, char ErrInfo[],char ErrInfo_Ex[]);
int testFixOver_End(int channel, int StartTime, struct struTestData *data, char ErrInfo[],char ErrInfo_Ex[]);
int testCenterSens(int channel, struct struTestData *data,struct struProcessLOG *prolog);
BOOL testFixSens (int channel, int panel);
int testDetailSens_OLT (int channel, int panel);
int testFixSens_Start(int channel, int *StartTime, char ErrInfo[],char ErrInfo_Ex[]);
int testFixSens_End(int channel, int StartTime, struct struTestData *data, char ErrInfo[],char ErrInfo_Ex[]);
int	testDetailSens (int channel, struct struTestData *data);
int testDetailSens_Extra (int channel, struct struTestData *data, struct struProcessLOG *prolog);
int testCurrent(int channel, struct struTestData *data);
int RecordMonitor(int channel,int index, struct struTestData *data);					//嵌套与波长测试功能，减少通道切换时的等待耗时；
int teststation(int channel, struct struTestData *data, struct struProcessLOG prolog);
int calTxPower(int channel, struct struTestData *data); 
int short_circuit_check (int channel, struct struTestData *data);
int calRxPower (int channel, struct struTestData *data, int panel);
int testTxDis(int channel, struct struTestData *data); 
int caltestTxPower(int channel, struct struTestData *data);
int testFixSD(int channel, struct struTestData *data); 
int testDetailSD(int channel, struct struTestData *data);
int testER(int channel, struct struTestData *data,struct struProcessLOG *prolog);
int testOEye(int channel, struct struTestData *data);
int testSpectrum(int channel, struct struTestData *data);
int testAOP(int channel, struct struTestData *data);

int testAOP_AginFront(int channel, struct struTestData *data);
int testAOP_AginBlack(int channel, struct struTestData *data);
int testWaveLength_AginFront(int channel, struct struTestData *data);   
int testWaveLength_AginBlack(int channel, struct struTestData *data);   
	
int testTE(int channel, struct struTestData *data);
int caltestRxPower(int channel, struct struTestData *data, int panel);
int GetModuleSN(int channel, struct struTestData *data, int panel);	//扫描输入SN
int ReadModuleSN(int channel, struct struTestData *data, int panel);//读取模块SN 
int SaveEEPROM(int channel, struct struTestData *data);             //保存EEPROM 
int Init_TxSD_Detect(int channel);
int InitTSSIDetect(int channel);
int test_TxSD_Detect(int channel);
int TestTSSIDetect(int channel, int iTestStartTime);
int testTxSD(int channel);
int scanModduleSN(int panel, int channel, char *sn);
int testSD(int channel);
int testIbias(int channel, struct struTestData *data);

int GetEnableChannel(int *channel);
void SetMainControl(int panel); 						    
int checkfunction(void);

int SET_DAC_APC (const int channel, int DAC);
int SET_DAC_MOD (const int channel, int DAC);
int Read_Ibias (const int channel, double *Ibias);

BOOL UpdateTiny13Firmware(const int channel, struct struTestData *data);
int get_mega88_temper_lutindex (int evb, struct struTestData *data);
int fit_LUT_MEGA88 (double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data);
int fit_LUT_MEGA88_VSC7967 (double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data);
int fit_LUT_MEGA88_NT25L90 (double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data);
BOOL fit_LUT_TINY13(double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_APC, BYTE DAC_MOD, BYTE *LUT_VSC);

int tuningSD_Mega88 (const int channel, struct struTestData *data);
int tuningSD_Mega88_VSC7967 (const int channel, struct struTestData *data);
int tuningSD_Mega88_NT25L90 (const int channel, struct struTestData *data);
int tuningSD_Mega88_NT25L90_TSSI (const int channel, struct struTestData *data);
int tuningSD_UX3328 (const int channel, struct struTestData *data);
int tuningSD_UX3328S (const int channel, struct struTestData *data);

int Read_RX_SD(const int channel, const int SD_Target, BYTE *SD);

int calTxPower1(const int channel);				//针对VSC7965产品 需要设置评估板burst触发信号输出 
int calTxPower2(const int channel); 			//针对VSC7967产品 不需要设置评估板burst触发信号输出
int calTxPower_ux3328 (const int channel); 		//针对ux3328产品 
int calTxPower_ux3328s (const int channel,struct struTestData *data); 	//针对ux3328s产品 

int SET_EVB5_MOD_BEN(const int channel);  
int SET_EVB5_MOD_Level(const int channel);

BOOL Read_Error_Bit_Ex(int channel, unsigned int *ErrorCount, int TestFlag);

/******************外推法测误码率1.0e-12数量级SENS部分函数***********************************/
BOOL Find_1E5_Phase(int channel, double *power5, double *ber_5);	  //获取1.0e-5数量级的SENS
BOOL Get_Extra_Data(int channel, double power, double ber_5, double *ber_array, double *optics_array);   //获取1.0e-7，1.0e-8，1.0e-9，1.0e-10 这4个数量级的SENS
BOOL Fit_Extra_Data(double ber_array[], double optics_array[], double *sens_12);   //根据获取的数据进行拟合得到k，b  并求出1.0e-12数量级的SENS
int CheckData(double ber_arr[], double optics_arr[], double *Ber, double * Optics, int *count);   
void Get_loglog_Arr(double BER_val[],int Data_Num,double *Result);
double Get_loglog_val(double BER_val);
int BERT_Get_BER(int channel, double *ber);
/******************外推法测误码率1.0e-12数量级SENS所需函数***********************************/

int test_olt_errbit_start (int channel, int *starttime, char ErrInfo[]);	//配置onu信号发送测试  
int test_olt_errbit_end (int channel, int starttime, char ErrInfo[]);		//检查onu信号发送结果 
int SET_ATT_ONU (int channel, double AttValue);
int SET_ATT_OLT (int channel, double AttValue);
BOOL Read_RX_SD_Ex(int evb, const int SD_Target, BYTE *SD);

BOOL Read_Error_Bit_OLT(int channel, unsigned int *ErrorCount, int TestFlag, double testtime, int InfoFlag, int messageflag, char ErrInfo[]);

BOOL testOEye_Mask_Threshold(const int channel, struct struTestData *data);
int testOEye_Mask(const int channel, struct struTestData *data);
int Get_Real_Maskmagin(int channel, int MaskMargin_init,int step,int waveform,int *real_margin);
BOOL Save_DCA_Eye(int DCAType, char *filename);
BOOL GetEyePicture(char *FilePath, struct struTestData *data);
BOOL Set_DCA_Run(int DCAType);
int Get_EYE(const int channel, int WaveForms, int MaskMargin);
int Get_EYE_Clear(const int channel, int WaveForms, int MaskMargin);
BOOL Set_DCA_Stop(int DCAType);
BOOL Set_MaskMargin(int DCAType, int MaskMargin);
BOOL Get_DCA_MaskCount(int DCAType, int *totalcount);
BOOL Set_DCA_AutoSet(int DCAType);
BOOL Get_DCA_Waveform(int DCAType, int *wavecount);
BOOL GET_MaskHits(int DCAType, double MaskMargin, int WaveForms);

int CalibrateRxPower_MEGA88 (int channel, struct struTestData *data, int panel);
int CalibrateRxPower_UX3328 (int channel, struct struTestData *data, int panel);
int CalibrateRxPower_UX3328S (int channel, struct struTestData *data, int panel);

int UpdateBurstMode(const int channel);
int UpdateWorkMode(const int channel);

int UX3328_UpdateA2CheckSum(const int channel, struct struTestData *data, int panel, char Info[]);
int UX3328S_UpdateA2CheckSum(const int channel, struct struTestData *data, int panel, char Info[]);

int save_EEPROM (const int channel, struct struTestData *data);
int save_EEPROM_file (char *filename, unsigned char *rom_value_arr);

int GET_DAC_MOD (int channel, int *DAC);
int ee_protect_check (int channel);

int testWavelength(int channel, struct struTestData *data);
int testWavelength_Tun(int channel, struct struTestData *data); 
int Read_Wavelength(int channel, double *wavelength);

//DWDM ONU相关代码
int DWDM_ONU_Sel_Wavelength_Channel (int handle, INT16U WavelengthChannel);
int tuningWavelength(int channel, struct struTestData *data);  
int tuningWavelength01(int channel, struct struTestData *data);  
int tuningWavelength02(int channel, struct struTestData *data);  
int tuningWavelength_HightLow(int channel, struct struTestData *data);
int tuningWavelength_FineTuning(int channel, struct struTestData *data,int index);		//波长微调  

int TunVapd (int channel, struct struTestData *data);
int TunVapd_EX (int channel, struct struTestData *data);
int TunVapd_HighLow (int channel, struct struTestData *data);			   //海源所提供方法_高低温

int DWDM_ONU_Save_APD_Luk(int channel, unsigned short DAC_APD, double temper);                             //保存APD查找表
int DWDM_ONU_Save_LOS_Luk(int channel, unsigned short DAC_BIAS, double temper);                            //保存LOS查找表
int DWDM_ONU_Save_Bias_Luk(int channel,int channelindex, unsigned short DAC, double temper);               //保存Bias查找表  
int tuningSD_DWDM_ONU (const int channel, struct struTestData *data);

int tuningTxOff_Depth(int channel, struct struTestData *data);
int tuningTxOff_Depth_Ex(int channel, struct struTestData *data);
int DWDM_ONU_Select_Channelindex(int channel,int channelindex);
int testTxOff_Depth(int channel, struct struTestData *data);

int DWDM_ONU_Read_LaserTemperatrue(int channel, struct struTestData *data);
int DB_Save_Results_Monitor (struct struTestData data);
int DWDM_ONU_Read_Tec_Current(int channel, struct struTestData *data);
int Get_8472_Monitor (int inst, float *Vcc,float *Temperatrue,float *Ibias,float *TxPWR,double *TECtemp, double *TECCurrent);
int DB_Get_TunTxOffPower_Spec (double *min ,double *max);
int DWDM_ONU_Set_Ibias_Max(int inst ); 
int DWDM_ONU_Set_Ibias_Max_Ex(int inst,int *Imax_dac  ); 
int DWDM_ONU_Ibias_Max_Enable(int inst);
int DWDM_ONU_Ibias_Max_Disable(int inst);
int DWDM_ONU_Set_LOS_Squelch (int inst);

int DB_READ_AUTODT_Spec_OSA_TEST(void);
int DB_READ_AUTODT_Spec_OSA_TEST_EX(void); 
int GetBosaSN_OSA_WL_TEST(int panle,int channel, struct struTestData *data);
int TEST_OSA_EX(int channel,struct struTestData *data);      
int TEST_OSA(int channel);
int SaveDataBase_1( struct struProcessLOG prolog);

int DWDM_ONU_Set_Burnin_Enable(int inst);
int DWDM_ONU_Set_Burnin_OFF (int channel);
int DWDM_ONU_Set_Burnin_ON (int channel);

int Get_TecPID_LookState(int channel);
int Get_TecPID_LookState_Ex(int channel);  

float f(float x,float a,float b,float c,float d); //X三次方函数  
float xpoint(float x1,float x2,float a,float b,float c,float d); //求弦与x轴交点坐标  
float root(float x1,float x2,float a,float b,float c,float d); //求根函数  
int Solving_Root(double *ratio,int order,double *y,double *x);

int CheckBarcode(char *SN);
int CheckErrorCode(char *PN,char *SN);
int DB_GET_Errorcode(char *PN,char *SN);
/**************************计算校准OSA的模拟直线*******************************/
int CaculOSALine(double waveLen_Y[], double tempe_X[],float linepara[]);
//
#endif
