#ifndef _DWDM_ONU_DS4830_H_
#define _DWDM_ONU_DS4830_H_ 

#pragma pack (1)                                    // set maximum alignment to 1 

#define IS_RXRSSI_EXT_FLAG   (0x01 << 0)            // RSSI展宽，=1表示RSSI展宽 进PLA中断，=0表示是RSSI 进IO中断
#define IS_RXRSSI_INT_FLAG   (0x01 << 1)            // RSSI =1取反, =0正常
#define IS_RXRSSI_OUT_FLAG   (0x01 << 2)            // TRIGGER OUT 展开标志，=1展宽，=0不展宽
#define IS_RXRSSI_DELAY_FLAG (0x01 << 3)            // TRIGGER OUT 延迟输出，=1延迟，=0不延迟

#define IS_RXTEMP_COMP_FLAG  (0x01 << 4)            // 收端监控温度补偿，=1表示温度补偿，=0表示不温补
#define IS_RXTRIG_FLAG       (0x01 << 5)            // 收端监控是中断引发的采样，=1表示是是中断引发的采样，=0表示是DDMI采样
#define IS_MULT_SEGMENT_FLAG (0x01 << 6)            // 收端监控拟合算法，=1表示分段拟合（隐含只分一段的PIN）；=0表示4阶拟合
#define IS_RXADC_POST_FLAG   (0x01 << 7)            // 收端监控ADC趋势，=0表示光越大ADC越小，=1表示光越大ADC越大

#define IS_APD_PROT_FLAG     (0x01 << 8)            // APD保护，=1则APD 保护
#define IS_BEN_HIGH_ACT_FALG (0x01 << 9)            // BEN有效电平，=1就BEN高有效
#define IS_BEN_TRIGGER_FALG  (0x01 << 10)           // 可选的BEN触发采样Txpower,  = 1,ben 采样
#define IS_RXPWRNODIAG_FLAG  (0x01 << 11)           // =0：表示需要对RxPWR进行监控告警，=1：表示不对RxPWR进行监控告警
#define IS_LOS_OR_SD_FLAG    (0x01 << 13)           // 该模块支持LOS还是SD, =1支持LOS，=0支持SD
#define IS_SUPPORT_SAVE_FLAG (0x01 << 14)           // 是否支持模块省电, =1支持， =0不支持
#define IS_DRV_LOS_FLAG      (0x01 << 15)           // 芯片信号状态标志，=1DRV 输出LOS
#define IS_A0HPAGE_PROT_FLAG (0x01 << 16)           // A0写保护，=1则A0写保护

#define IS_FAULT_IN_FLAG     (0x01 << 17)           // FAULT_IN 可选取反或透传标志，=1 则取反, =0 透传
#define IS_SOFT_DIS_FLAG     (0x01 << 19)           // SOFT_DIS可选标志，=1 则SOFT DISABLE 使能
#define IS_LA_EN_FLAG1       (0x01 << 20)           // LA_EN 可选标志，=00固定输出0， =10固定输出1 ，=11 透传
#define IS_LA_EN_FLAG2       (0x01 << 21) 
#define IS_LOS_OUT_FLAG      (0x01 << 22)           // LOS_OUT可选标志，=1则取反，=0透传

#define IS_DRV_TMEP_FLAG     (0x01 << 23)           // 温度采样，=0 MCU温度，=1 DRV温度
#define IS_DRV_RXPWR_FLAG    (0x01 << 24)           // 收端功率采样, =0 MCU采样， =1 DRV采样

#define IS_SLECT_HW_LSBCAIL  (0x01 << 25)           // 该模块的LSB精度, =1支持华为精度，=0支持中兴精度

// Define Bits for [110-111]

#define STA_DATA_NOT_READY      (0x01 << 0)         // Indicates transceiver has achieved power up and A/D data is ready 
#define STA_LOS                 (0x01 << 1)         // Indicates Optical Loss of Signal
#define STA_TXFAULT             (0x01 << 2)         // 
#define STA_SW_RATESEL          (0x01 << 3)         // 
#define STA_RATESEL_STA         (0x01 << 4)         // 
#define STA_SW_TXDIS            (0x01 << 6)         // Optional read/write bit that allows software disable of laser.
#define STA_TX_DISABLE          (0x01 << 7)         // Digital state of the TX Disable Input Pin 

#define ALARM_H					(0x01 << 3)
#define WARN_H					(0x01 << 2)
#define ALARM_L					(0x01 << 1)			 
#define WARN_L					(0x01 << 0)

#define TEMPER_LOW					0
#define TEMPER_ROOM					1
#define TEMPER_HIGH					2


//#define DTN7354					0xA8

typedef struct RxCaliData_DEF
{
	unsigned short ADC;	     
	unsigned short PWR; 
}RxCaliData_DEF;

typedef union FLOATUNBYTE
{
	float         doubletemp;	     
	unsigned char bytetemp[4]; 
}UN_FLOATUNBYTE;


#pragma pack () /* set maximum alignment to default */

//--------------------------------------------------------------------------------------------------------------------------------

extern int DWDM_ONU_DS4830_Enter_Password(int handle); 
extern int DWDM_ONU_DS4830_GET_FirmwareVer(int handle, char *FirmwareVer); 		//读取firmware版本  

//-----------------------------------------------------------------------------------------------------------------------------------------

extern int DWDM_ONU_DS4830_Set_BIAS_Auto(int handle); 
extern int DWDM_ONU_DS4830_Set_BIAS_Manual(int handle); 
extern int DWDM_ONU_DS4830_Set_BIAS(int handle, unsigned short APC_DAC);
extern int DWDM_ONU_DS4830_Read_BIAS(int handle, unsigned char* APC_DAC); 	  
extern int DWDM_ONU_DS4830_Write_BIAS_Luk(int handle, int channelindex,int address, int value, INT16U wSlop, INT16U wOffset);
extern int DWDM_ONU_DS4830_Read_BIAS_Luk(int handle, int channelindex,int address, int *value, INT16U *wSlop, INT16U *wOffset);

extern int DWDM_ONU_DS4830_Set_MOD_Auto(int handle); 
extern int DWDM_ONU_DS4830_Set_MOD_Manual(int handle);  
extern int DWDM_ONU_DS4830_Set_MOD(int handle, unsigned short MOD_set);  
extern int DWDM_ONU_DS4830_Read_MOD(int handle, unsigned short* MOD_read); 

extern int DWDM_ONU_DS4830_Read_Txpower_ADC(int handle, unsigned short* ADC); 
extern int DWDM_ONU_DS4830_Write_Txpower_Cal(int handle, unsigned short* ADC,double* AOP_in, float Tx_unit);  	 

extern int DWDM_ONU_DS4830_Set_TEC_Auto(int handle); 
extern int DWDM_ONU_DS4830_Set_TEC_Manual(int handle);  
//--------------------------------------------------------------------------------------------------------------------------------

extern int DWDM_ONU_DS4830_Update_Base0(int handle);
extern int DWDM_ONU_DS4830_Update_Base1(int handle);    
extern int DWDM_ONU_DS4830_Update_Base2(int handle);    
extern int DWDM_ONU_DS4830_Update_Base3(int handle);    
extern int DWDM_ONU_DS4830_Update_LUK0(int handle);    
extern int DWDM_ONU_DS4830_Update_LUK1(int handle);    
extern int DWDM_ONU_DS4830_Update_LUK2(int handle);
extern int DWDM_ONU_DS4830_Update_LUK3(int handle);
extern int DWDM_ONU_DS4830_Update_LUK4(int handle);
extern int DWDM_ONU_DS4830_Update_LUK5(int handle);
extern int DWDM_ONU_DS4830_Update_LUK6(int handle);
extern int DWDM_ONU_DS4830_Update_LUK7(int handle); 
extern int DWDM_ONU_DS4830_Update_LUK8(int handle); 
extern int DWDM_ONU_DS4830_Update_LUK9(int handle); 
extern int DWDM_ONU_DS4830_Update_LUK10(int handle);
extern int DWDM_ONU_DS4830_Update_DRIVER1(int handle); 
extern int DWDM_ONU_DS4830_Update_DRIVER0(int handle);

//-------------------------------------------------------------------------------------------------------------------------------------

extern int DWDM_ONU_DS4830_Set_APD_Auto(int handle);  
extern int DWDM_ONU_DS4830_Set_APD_Manual(int handle); 
extern int DWDM_ONU_DS4830_Set_APD(int handle, unsigned int APD_set); 
extern int DWDM_ONU_DS4830_Read_APD(int handle, unsigned short* APD_set); 
extern int DWDM_ONU_DS4830_Write_APD_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset);
extern int DWDM_ONU_DS4830_Read_APD_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset);
extern int DWDM_ONU_DS4830_Write_MOD_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset); 
extern int DWDM_ONU_DS4830_Read_MOD_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset);

extern int DWDM_ONU_DS4830_Set_LOS_Auto(int handle);  
extern int DWDM_ONU_DS4830_Set_LOS_Manual(int handle); 
extern int DWDM_ONU_DS4830_Set_LOS(int handle, unsigned short LOS);
extern int DWDM_ONU_DS4830_Read_LOS(int handle, unsigned short *LOS);
extern int DWDM_ONU_DS4830_Write_LOS_Luk(int handle, int address, int value, INT16U wSlop, INT16U wOffset);
extern int DWDM_ONU_DS4830_Read_LOS_Luk(int handle, int address, int *value, INT16U *wSlop, INT16U *wOffset);
extern int DWDM_ONU_DS4830_Set_TemperMode(int handle, int mode);	   //02为公式模式，00为外部模式 

extern int DWDM_ONU_DS4830_Read_Rx_ADC(int handle, unsigned short* ADC);
extern int DWDM_ONU_DS4830_Write_Rx_Cal(int handle, unsigned short* ADC_Array,float* PWR_Array_P,int RxCaliNo); 

//-------------------------------------------------------------------------------------------------------------------
extern int DWDM_ONU_DS4830_Get_CoreTemper(int handle, double* temper);
extern int DWDM_ONU_DS4830_Get_CoreTemper_Ex(int handle, double* temper); 
extern int DWDM_ONU_DS4830_Get_Temper(int handle, double* temper); 
extern int DWDM_ONU_DS4830_SET_Temp_Cal(int handle, INT16U wSlop, INT16U wOffset);

extern int DWDM_ONU_DS4830_Monitor_RxPower(int handle, double* RxPower);  								  
extern int DWDM_ONU_DS4830_Monitor_TxPower(int handle, double* TxPower); 
extern int DWDM_ONU_DS4830_Monitor_IBias(int handle, double* IBias, float Bias_unit); 

extern int DWDM_ONU_DS4830_Set_CaseTemper_Cal(int handle, double sSlop,double sOffset); 
extern int DWDM_ONU_DS4830_Write_APC_LUK6(int handle, int channelindex, unsigned short APC_DAC); 
extern int DWDM_ONU_DS4830_Read_APC_LUK6(int handle, int channelindex, int *APC_DAC);
extern int DWDM_ONU_DS4830_Write_MOD_LUK6(int handle, int channelindex, unsigned short MOD_DAC);  
extern int DWDM_ONU_DS4830_Read_Mod_LUK6(int handle, int channelindex, int *APC_DAC);
extern int DWDM_ONU_DS4830_Save_Reg(int handle); 

extern int DWDM_ONU_DS4830_Set_APD_Protect(int handle, unsigned short APD_Protect);
extern int DWDM_ONU_DS4830_Set_APD_Protect2(int handle, unsigned short APD_Protect);
 
extern int DWDM_ONU_DS4830_Set_Tx_Soft_Disable(int handle, int disable_flag);

extern int DWDM_ONU_DS4830_Set_Burst(int handle, BOOL BURST_FLAG);	//设置突发发光，BURST_FLAG = TRUE 突发发光，BURST_FLAG = FALSE 连续发光

int DWDM_ONU_DS4830_Set_APD_DAC(int handle, int add, unsigned char APD_DAC);
int DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(int handle, int DAC);
int DWDM_ONU_DS4830_Get_TEC_DAC7_Adjust7(int handle, int *DAC);
int DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7_Ex(int handle, int DAC);


int DWDM_ONU_DS4830_Write_TEC_Luk(int handle, int channelindex, BOOL HIGH_FLAG, unsigned short* DAC, double* Temper);   
int DWDM_ONU_DS4830_Write_TEC_Luk_Ex(int handle, int channelindex,  unsigned short* DAC, double* Temper); 				//Tunning Room WriteLuk K=0,
int DWDM_ONU_DS4830_Get_TEC_Luk(int handle, int channelindex,  unsigned short* DAC, double* Temper); 

int DWDM_ONU_DS4830_Write_TxOffDepth_Luk(int handle, int channelindex, BOOL HIGH_FLAG, unsigned short* DAC, double* Temper);	   
int DWDM_ONU_DS4830_Write_TxOffDepth_Luk_EX(int handle, int channelindex, unsigned short* DAC, double* Temper);

int DWDM_ONU_DS4830_SET_Vcc_Cal(int handle);

int DWDM_ONU_DS4830_SET_LaserTemp_Model(int handle,int model);		//1---温度，0---PID DAC
int DWDM_ONU_DS4830_Clear_FineTuning(int handle);		//清楚微调值 
int DWDM_ONU_DS4830_Set_Wavelength_Range(int handle,int start,int end);		//设置波长范围 
int DWDM_ONU_DS4830_Set_TecTempe(int handle);

#endif	   
