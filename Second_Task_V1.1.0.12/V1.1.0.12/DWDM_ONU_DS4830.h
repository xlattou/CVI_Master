#ifndef _DWDM_ONU_DS4830_H_
#define _DWDM_ONU_DS4830_H_ 

#pragma pack (1)                                    // set maximum alignment to 1 

#define IS_RXRSSI_EXT_FLAG   (0x01 << 0)            // RSSIչ��=1��ʾRSSIչ�� ��PLA�жϣ�=0��ʾ��RSSI ��IO�ж�
#define IS_RXRSSI_INT_FLAG   (0x01 << 1)            // RSSI =1ȡ��, =0����
#define IS_RXRSSI_OUT_FLAG   (0x01 << 2)            // TRIGGER OUT չ����־��=1չ��=0��չ��
#define IS_RXRSSI_DELAY_FLAG (0x01 << 3)            // TRIGGER OUT �ӳ������=1�ӳ٣�=0���ӳ�

#define IS_RXTEMP_COMP_FLAG  (0x01 << 4)            // �ն˼���¶Ȳ�����=1��ʾ�¶Ȳ�����=0��ʾ���²�
#define IS_RXTRIG_FLAG       (0x01 << 5)            // �ն˼�����ж������Ĳ�����=1��ʾ�����ж������Ĳ�����=0��ʾ��DDMI����
#define IS_MULT_SEGMENT_FLAG (0x01 << 6)            // �ն˼������㷨��=1��ʾ�ֶ���ϣ�����ֻ��һ�ε�PIN����=0��ʾ4�����
#define IS_RXADC_POST_FLAG   (0x01 << 7)            // �ն˼��ADC���ƣ�=0��ʾ��Խ��ADCԽС��=1��ʾ��Խ��ADCԽ��

#define IS_APD_PROT_FLAG     (0x01 << 8)            // APD������=1��APD ����
#define IS_BEN_HIGH_ACT_FALG (0x01 << 9)            // BEN��Ч��ƽ��=1��BEN����Ч
#define IS_BEN_TRIGGER_FALG  (0x01 << 10)           // ��ѡ��BEN��������Txpower,  = 1,ben ����
#define IS_RXPWRNODIAG_FLAG  (0x01 << 11)           // =0����ʾ��Ҫ��RxPWR���м�ظ澯��=1����ʾ����RxPWR���м�ظ澯
#define IS_LOS_OR_SD_FLAG    (0x01 << 13)           // ��ģ��֧��LOS����SD, =1֧��LOS��=0֧��SD
#define IS_SUPPORT_SAVE_FLAG (0x01 << 14)           // �Ƿ�֧��ģ��ʡ��, =1֧�֣� =0��֧��
#define IS_DRV_LOS_FLAG      (0x01 << 15)           // оƬ�ź�״̬��־��=1DRV ���LOS
#define IS_A0HPAGE_PROT_FLAG (0x01 << 16)           // A0д������=1��A0д����

#define IS_FAULT_IN_FLAG     (0x01 << 17)           // FAULT_IN ��ѡȡ����͸����־��=1 ��ȡ��, =0 ͸��
#define IS_SOFT_DIS_FLAG     (0x01 << 19)           // SOFT_DIS��ѡ��־��=1 ��SOFT DISABLE ʹ��
#define IS_LA_EN_FLAG1       (0x01 << 20)           // LA_EN ��ѡ��־��=00�̶����0�� =10�̶����1 ��=11 ͸��
#define IS_LA_EN_FLAG2       (0x01 << 21) 
#define IS_LOS_OUT_FLAG      (0x01 << 22)           // LOS_OUT��ѡ��־��=1��ȡ����=0͸��

#define IS_DRV_TMEP_FLAG     (0x01 << 23)           // �¶Ȳ�����=0 MCU�¶ȣ�=1 DRV�¶�
#define IS_DRV_RXPWR_FLAG    (0x01 << 24)           // �ն˹��ʲ���, =0 MCU������ =1 DRV����

#define IS_SLECT_HW_LSBCAIL  (0x01 << 25)           // ��ģ���LSB����, =1֧�ֻ�Ϊ���ȣ�=0֧�����˾���

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
extern int DWDM_ONU_DS4830_GET_FirmwareVer(int handle, char *FirmwareVer); 		//��ȡfirmware�汾  

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
extern int DWDM_ONU_DS4830_Set_TemperMode(int handle, int mode);	   //02Ϊ��ʽģʽ��00Ϊ�ⲿģʽ 

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

extern int DWDM_ONU_DS4830_Set_Burst(int handle, BOOL BURST_FLAG);	//����ͻ�����⣬BURST_FLAG = TRUE ͻ�����⣬BURST_FLAG = FALSE ��������

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

int DWDM_ONU_DS4830_SET_LaserTemp_Model(int handle,int model);		//1---�¶ȣ�0---PID DAC
int DWDM_ONU_DS4830_Clear_FineTuning(int handle);		//���΢��ֵ 
int DWDM_ONU_DS4830_Set_Wavelength_Range(int handle,int start,int end);		//���ò�����Χ 
int DWDM_ONU_DS4830_Set_TecTempe(int handle);

#endif	   
