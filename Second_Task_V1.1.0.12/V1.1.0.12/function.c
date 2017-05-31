#include <userint.h>
#include "cvi_db.h"
#include "combobox.h"
#include "inifile.h"
#include "function.h" 
#include "DWDM ONU Parallel ATE.h"  
#include "toolbox.h" 
#include "evb5.h"
#include "pmsii.h"
#include "ag86142.h"
#include "aq633x.h"
#include <analysis.h>
#include "labdll.h" 
#include "ms9740.h"
#include "ux3328.h"
#include "ux3328s.h"  
#include "MEGA88.h"
#include "nt25l90.h"
#include "DWDM_ONU_DS4830.h"
#include "Supermaster.h"
#include "MS97xx.h"  
#include "AQ637x.h"  
#include "JW8504.h"
#include "pss.h"

//for database
int hdbc = 0;      /* Handle to database connection    */ 
int hdbc_oa = 0;   /* Handle to oa database connection */ 
int DBConfigStat;  /* Status code                      */ 

#define RETURN_LOSS_MAX  -30.0
#define TE_MAX           2.0

void ShowDataBaseError (void)
{
    MessagePopup("Database Error",DBErrorMessage());
}

void readconfigfilename (void)
{
	//��ȡ���⹦�����ò����ļ�����
	GetProjectDir (g_ConfigFileName);
	strcat (g_ConfigFileName,"\\data\\ATE_Function.ini");
	
	//��ȡ������ַ�����ļ�
	GetProjectDir (g_InstFileName);
	strcat (g_InstFileName,"\\data\\ATE_Instrument.ini");
	
	GetProjectDir (my_struTxCal.TxCal_Power_FileName);
	strcat (my_struTxCal.TxCal_Power_FileName,"\\data\\ATE_TxPowCal.txt");
	
	//����С��У׼ϵ��
	GetProjectDir (my_struTxCal.TxCal_Power_FileName_Min);
	strcat (my_struTxCal.TxCal_Power_FileName_Min,"\\data\\ATE_TxPowCal_Min.txt");
	
	GetProjectDir (sRxCal.RxCal_FileName);
	strcat (sRxCal.RxCal_FileName,"\\data\\ATE_RxPowCal.txt");
	
	//��ȡOLT������У׼����
	GetProjectDir (my_struCal_OLT.OLT_sens_file);
	strcat (my_struCal_OLT.OLT_sens_file,"\\data\\ATE_OLT_Sens.txt");
	
	//��ȡOLT���ն˹�·У׼����
	GetProjectDir (my_struCal_OLT.FileName);
	strcat (my_struCal_OLT.FileName,"\\data\\ATE_Cal_OLT.txt");
}

int Initial(int panel, int panMain, int *error_channel, int ini_Flag)
{
	int error = 0;
	int status, progresshandle, dca_no_init, spectrum_no_init;
	int sw_no_init,sw_no_init_2, clock_no_init;
	int channel;
	char buf[1024];
	struct struInstrument localInst;
	char Info[50]; 
	int Version;
	char tempbuf[256];
	
	DisplayPanel (panel);   //display initial panel
	
	SetCtrlVal (panMain, PAN_MAIN_LED, 0);
	SetCtrlAttribute (panMain, PAN_MAIN_LED, ATTR_OFF_COLOR, VAL_YELLOW);

	//get software version
	strcpy (my_struProcessLOG.SoftVersion, SOFTVER); 
	//get computername
	status = MyDLLGetComputerName(my_struProcessLOG.StationID);
	if(!status) 
	{
		MessagePopup ("Error", "Read Computer Name error !"); 
		error=-1;
		goto Error; 
	} 
	
	readconfigfilename();
	
	//���ݿ�����
	errChk(DB_Initial());
	
	//��ȡ���������ļ�
	errChk(GetConfig()); 

	//��ȡ�豸�����ļ�
	errChk(GetConfig_Inst());	  
	
	//����������ؼ�����
	SetMainControl(panMain);

	//��ȡУ׼�����ļ�
	error=GetCaliConfigFile();
	if(error<0)
	{
		MessagePopup("��ʾ","��ȡУ׼��Ϣʧ�ܣ�");
	}	
		
	/***�û�Ȩ�޲�ΪOQAʱ��my_struConfig.DT_OQA����ΪFalse���û�Ȩ�޲���OQA��������������ļ��л�ȡmy_struConfig.DT_OQAΪTRUE**Eric.Yao***/  
	if (0 != stricmp (my_struLicense.power, "OQA"))
	{
		my_struConfig.DT_OQA = FALSE;		 
	}
	
	if (ini_Flag)
	{
		//��ʼ���豸����
		progresshandle = CreateProgressDialog ("�豸����״̬", "�豸���ý���", 1, VAL_NO_INNER_MARKERS, ""); 
	
		dca_no_init = 1 ;
		spectrum_no_init = 1;
		sw_no_init=1;
		sw_no_init_2=1; 
		clock_no_init=1;
		for (channel=0;channel<CHANNEL_MAX;channel++)
		{
			UpdateProgressDialog (progresshandle, (int)(100.*channel/CHANNEL_MAX), 0); 
		
			if (INSTR[channel].ChannelEn)
			{
				INSTR[channel].SBert_Rate = SBERT_R_R2488Mbps;
				INSTR[channel].SBert_PRBS = SBERT_P_PRBS23;
				if (my_struConfig.DT_Test_Upstream) 
				{
					INSTR[channel].GBert_Rate = GBERT_R1250Mbps;
					INSTR[channel].GBert_PRBS = GBERT_PRBS23;
				}
				else
				{
					INSTR[channel].GBert_Rate = GBERT_R2488Mbps;
					INSTR[channel].GBert_PRBS = GBERT_PRBS23;
				}
			
				//��ʼ��EVB
				if ((SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE) || (SEVB_TYPE_EVB27==INSTR[channel].SEVB_TYPE))
				{
					if((EVB5_Init(&INST_EVB[channel], INSTR[channel].SEVB))<0)
					{
						MessagePopup("��ʾ","EVB5��ʼ��ʧ�ܣ�");  
						error=-1;      
						goto  Error;
					}
				
					error = I2C_SLAVE_SEARCH_DLL(INST_EVB[channel], SBERT_Addr);   
					if (error==0)
					{
						//���÷����ź�
						errChk(EVB5_SBERT_Init(INST_EVB[channel], SBERT_R_R1244Mbps, INSTR[channel].SBert_PRBS));
					}	 

			
					
					//����Clock����Ҫ���ڷ���У׼	
					errChk(EVB5_SET_CLOCK (INST_EVB[channel], 1));
	
					//����BEN_PW����Ҫ���ڲ���ONU_TxSD	
					errChk(EVB5_SET_BEN_PW (INST_EVB[channel], 0));   
				
					//����BEN_mode
					errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
				
					//����BEN_level, �͵�ƽ����
					errChk(EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON));
			
					if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip || CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 
					{
						errChk(I2C_HOST_RESET_F320_DLL (INST_EVB[channel]-0x800, (256-24*1E3/3./40), 0)); 
					}
					
					//��ȡEVB5�汾
					error = EVB5_READ_FirmwareVersion (INST_EVB[channel], &Version);
					if (error) 
					{
						error=-1; 
						goto Error; 
					}
	
					if (Version!=47) 
					{
						sprintf(tempbuf,"ͨ��%d EVB5�̼��汾��V4.7������������",channel);
						MessagePopup("��ʾ",tempbuf);
						error=-1; 
						goto Error; 
					} 
					
					//����ģ���ѹ3.3V
					error = EVB5_SET_VTR(INST_EVB[channel],3.3);
					if (error) 
					{
						error=-1; 
						goto Error; 
					}
	
					//�رյ�Դ
					errChk(EVB5_SET_SHDN_VTR(INST_EVB[channel], 0));
				}
				else 
				{
					sprintf (buf, "ͨ��%d ���԰��ͺ����ô���    ", channel);
					MessagePopup("Error", buf); 
					error=-1; 
					goto Error; 
				} 
				
				error=RegisterI2cCallBackFunction();
				if(error)
				{
					MessagePopup("Error", "RegisterI2cCallBackFunction���������쳣"); 
					goto Error;
				}
				Delay(1);
				SetSuperDev(0xA2);
 		
				//��ʼ�����ʼ�
				if(PowMeter_TYPE_PMSII == INSTR[channel].PM_TYPE)
				{
			     	error = PMSII_Init_Port(&INST_PM[channel], INSTR[channel].PM, my_struDBConfig.TxWavelength);
					if (error<0) 
					{
						sprintf (buf, "ͨ��%d �⹦�ʼƳ�ʼ���쳣    ", channel);
						MessagePopup("Error", buf); 
						goto Error;
					} 
				}
				else if(PowMeter_TYPE_NONE == INSTR[channel].PM_TYPE) 
				{
					;
				}
				else if(PowerMeter_TYPE_PSS_OPM ==INSTR[channel].PM_TYPE)
				{
				   error=PSS_init_port(&INST_PM[channel], INSTR[channel].PM);
				   if(error<0)
				   { 
					   MessagePopup("Error","�⹦�ʼƳ�ʼ���쳣��") ;
						goto Error;
				   }
				  if (1480==my_struDBConfig.TxWavelength)  my_struDBConfig.TxWavelength=1490;   
				   error =PSS_Set_WaveLength(INST_PM[channel],channel+1, my_struDBConfig.TxWavelength);
				   if(error<0)
				   {
				     MessagePopup("Error","��������ʧ�ܣ�");
					 goto Error;
				   }
				}
				else 
				{
					sprintf (buf, "ͨ��%d �⹦�ʼ��ͺ����ô���    ", channel);
					MessagePopup("Error", buf); 
					error=-1; 
					goto Error;
				}
			
				//for ATT main
				if(INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_NONE)
				{
					;
				}
				else if (INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_GVPM)  
				{
					errChk(GVPM_Init(&INST_ATT_ONU[channel], INSTR[channel].GVPM_ONU, my_struDBConfig.RxWavelength, 10));
			
					errChk(GVPM_SET_LockEnable (INST_ATT_ONU[channel], 1));	
		
					errChk(GVPM_SET_LockAttenuate (INST_ATT_ONU[channel], -10, my_struDBConfig.RxWavelength, 1));
	
				}
				else if(INSTR[channel].ATT_TYPE_ONU==ATT_TYPE_JW8504)
				{    if (1480==my_struDBConfig.RxWavelength)  my_struDBConfig.RxWavelength=1490; 
				   error=JW8504_Init_Port(&INST_ATT_ONU[channel],INSTR[channel].ATT_COM);
				   if (error) {MessagePopup("Error", "Initial JW8504 error!");goto Error;} 
				   
				   error=JW8504_Set(INST_ATT_ONU[channel],channel,my_struDBConfig.RxWavelength,0);
				   if (error) {MessagePopup("Error", "Initial JW8504 error!");goto Error;}
				   
				   if(JW8504_enable(INST_ATT_ONU[channel],channel, 1)<0) return -1; 
				}
				else 
				{
					sprintf (buf, "ͨ��%d ˥�����ͺ����ô���    ", channel);
					MessagePopup("Error", buf); 
					error=-1; 
					goto Error;
				}

				//for BERT
				errChk(Init_BERT (channel));
			
				//for bert to olt
				if (INSTR[channel].BERT_TYPE_OLT == BERT_TYPE_GBERT) 
				{
					if (!GBERT_Init(&INST_BERT_OLT[channel], INSTR[channel].BERT_OLT, INSTR[channel].GBert_PRBS, INSTR[channel].GBert_Rate, 0)) 
					{
						MessagePopup("Error", "Initial GBERT error!");
						error=-1;
						goto Error;
					} 
				}
				else if (INSTR[channel].BERT_TYPE_OLT == BERT_TYPE_SBERT) 
				{
					//ע�ⷵ��ֵ��int�ͣ�0��ʾOK
					errChk(EVB5_Init(&INST_BERT_OLT[channel], INSTR[channel].BERT_OLT)); 
				
					errChk(EVB5_SBERT_Init(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS));   
				}
				else if (INSTR[channel].BERT_TYPE_OLT == BERT_TYPE_NONE)  
				{
					;
				}
				else 
				{
					error=-1;           
					MessagePopup("Error", "Can not find this BERT  !");
					goto Error;
				}
				/**************************************BERT******************************/
	
				//for ONU to OLT att
				if (INSTR[channel].ATT_TYPE_OLT == ATT_TYPE_GVPM)  
				{
					errChk(GVPM_Init(&INST_ATT_OLT[channel], INSTR[channel].GVPM_OLT, my_struDBConfig.TxWavelength, 10));
			
					errChk(GVPM_SET_LockEnable(INST_ATT_OLT[channel], 1));
				
					errChk(GVPM_SET_LockAttenuate(INST_ATT_OLT[channel], my_struCal_OLT.Power_In[channel]+my_struCal_OLT.Power[channel], my_struDBConfig.TxWavelength, 0));
				}
				else if (INSTR[channel].ATT_TYPE_OLT == ATT_TYPE_NONE)  
				{
					;
				}
				else 
				{
					error=-1;          
					MessagePopup("Error", "Can not find this ATT type");
					goto Error;  
				} 
				/**************************************ATT******************************/  
			
				my_struTxCal.TxCal_Er_Slope = 1.;
				my_struTxCal.TxCal_Er_Offset = 0.;
				//for DCA
				if (INSTR[channel].DCA_TYPE==DCA_TYPE_CSA8000 && dca_no_init) 
				{
					if (!CSA8000_Init(&inst_CSA8000, INSTR[channel].DCA)) 
					{
						MessagePopup("Error", "Initial CSA8000 error!");
						error=-1;      
						goto Error;
					}  
		
					if (!CSA8000_GET_MainSN(inst_CSA8000, my_struCSA8000.MainSN)) 
					{
						MessagePopup("Error", "��ȡCSA8000�������кŴ���");
						error=-1;      
						goto Error;
					} 

					errChk(DB_Get_Spec_CSA8000());
		
					if (!CSA8000_Set_O(inst_CSA8000, my_struCSA8000.Channel_O, my_struCSA8000.X_Scale, my_struCSA8000.X_Position, my_struCSA8000.Y_Scale, my_struCSA8000.Y_Position, my_struCSA8000.Y_Offset, my_struCSA8000.Wavelength, my_struCSA8000.Filter, my_struCSA8000.MaskName)) 
					{
						MessagePopup("Error", "CSA8000 Config for Optics Channel error!");
						error=-1;      
						goto Error;
					} 
				
					dca_no_init = 0;
				}
				else if (INSTR[channel].DCA_TYPE==DCA_TYPE_Ag86100 && dca_no_init)
				{
					if (!Ag86100_Init(&inst_Ag86100, INSTR[channel].DCA)) 
					{
						MessagePopup("Error", "Initial Ag86100 error!");
						error=-1;      
						goto Error;
					} 
		
					if (!Ag86100_GET_MainSN(inst_Ag86100, my_struAg86100.MainSN)) 
					{
						MessagePopup("Error", "��ȡAg86100�������кŴ���");
						error=-1;      
						goto Error;
					} 
		
					errChk(DB_Get_Spec_Ag86100()); 
		
					if (!Ag86100_SET_O(inst_Ag86100, my_struAg86100.ERFactor, my_struAg86100.X_Scale, my_struAg86100.X_Position, my_struAg86100.Y_Scale, my_struAg86100.Y_Offset, my_struAg86100.Filter, my_struAg86100.Wavelength, my_struAg86100.MaskName, my_struAg86100.MaskMargin, my_struAg86100.Channel_O)) 
					{
						MessagePopup("Error", "Initial Ag86100 error!");
						error=-1;      
						goto Error;
					}
				
					dca_no_init = 0;
				}
				else if (INSTR[channel].DCA_TYPE==DCA_TYPE_NONE || 0==dca_no_init)
				{
					;
				}	
				else 
				{
					sprintf (buf, "ͨ��%d ʾ�����ͺ����ô���    ", channel); 
					MessagePopup("Error", buf);
					error=-1;
					goto Error;
				}
			
				//SPECTRUM
				if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AG8614X && spectrum_no_init)
				{
					if (!Ag86142_Init(&inst_Ag86142, INSTR[channel].SPECTRUM)) 
					{
						MessagePopup("Error", "Initial Spectrum error!");
						error=-1;      
						goto Error;
					} 
		
					if (!Ag86142_Config(LASER_TYPE_DFB, inst_Ag86142, my_struDBConfig.TxWavelength-1.0, my_struDBConfig.TxWavelength+1.0, 0.2)) 
					{
						MessagePopup("Error", "Config Spectrum error!");
						error=-1;      
						goto Error;
					}
				
					spectrum_no_init=0;
				}
				else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AQ633X && spectrum_no_init)
				{
					errChk(AQ633X_Init (&Inst_AQ633X, INSTR[channel].SPECTRUM)); 
					if (my_struConfig.DT_OSA_WL_Test)
					{
						my_struDBConfig.PeakWavelengthMin[0]=struDBConfig_OSATEST.WaveLenth_Min;
						my_struDBConfig.PeakWavelengthMax[0]=struDBConfig_OSATEST.WaveLenth_Min;
					}
					errChk(AQ633X_Config (Inst_AQ633X, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMin[0]+my_struDBConfig.PeakWavelengthMax[0])/2.0, 2.0, 0.2)); 
				
					spectrum_no_init=0;
				}
				else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AQ637X && spectrum_no_init)
				{
					errChk(AQ637X_Init (&Inst_AQ637X, INSTR[channel].SPECTRUM)); 
					if (my_struConfig.DT_OSA_WL_Test)
					{
						my_struDBConfig.PeakWavelengthMin[0]=struDBConfig_OSATEST.WaveLenth_Min;
						my_struDBConfig.PeakWavelengthMax[0]=struDBConfig_OSATEST.WaveLenth_Min;
					}
					errChk(AQ637X_Config (Inst_AQ637X, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMin[0]+my_struDBConfig.PeakWavelengthMax[0])/2.0, 2.0, 0.03)); 
				
					spectrum_no_init=0;
				}
				else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_MS9740 && spectrum_no_init)
				{
					errChk(MS9740_Init (&Inst_MS9740, INSTR[channel].SPECTRUM)); 
					if (my_struConfig.DT_OSA_WL_Test)
					{
						my_struDBConfig.PeakWavelengthMin[0]=struDBConfig_OSATEST.WaveLenth_Min;
						my_struDBConfig.PeakWavelengthMax[0]=struDBConfig_OSATEST.WaveLenth_Min;
					}
					errChk(MS9740_Config (Inst_MS9740, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMin[0]+my_struDBConfig.PeakWavelengthMax[0])/2.0, 2.0, 0.03)); 
				
					spectrum_no_init=0;
				}
				else if(INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AG86120B || 0==spectrum_no_init)
				{
					errChk(Ag86120B_Init (&Inst_AG86120B, INSTR[channel].SPECTRUM)); 
				}
				else if(INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_NONE || 0==spectrum_no_init)
				{
					;
				}
				else 
				{
					sprintf (buf, "ͨ��%d �������ͺ����ô���    ", channel); 
					MessagePopup("Error", buf);
					error=-1;
					goto Error;
				} 

				// for sw
				if (((INSTR[channel].SW_TYPE == SW_TYPE_FSW) || (SW_TYPE_COFFSW == INSTR[channel].SW_TYPE) || (SW_TYPE_10G == INSTR[channel].SW_TYPE) || (SW_TYPE_JHFSW == INSTR[channel].SW_TYPE) || (SW_TYPE_COFFSW02 == INSTR[channel].SW_TYPE)) && sw_no_init)
				{
					errChk(Fiber_SW_Init(INSTR[channel].SW_TYPE, INSTR[channel].SW, &INSTR[channel].SW_Handle, INSTR[channel].SW_SN));
					
					sw_no_init=0;
				}
				else if ((SW_TYPE_10G == INSTR[channel].SW_TYPE) || (0 == sw_no_init))
				{
					INSTR[channel].SW_Handle = INSTR[channel-1].SW_Handle;
				}
				else if ((SW_TYPE_NONE == INSTR[channel].SW_TYPE) || (0==sw_no_init))
				{
					;
				}
				else 
				{
					sprintf (buf, "ͨ��%d �⿪���ͺ����ô���    ", channel); 
					MessagePopup("Error", buf);
					error=-1;
					goto Error;
				} 
				
				// for sw 2
				if (((INSTR[channel].SW_TYPE_2 == SW_TYPE_FSW) || (SW_TYPE_COFFSW == INSTR[channel].SW_TYPE_2) || (SW_TYPE_10G == INSTR[channel].SW_TYPE_2) || (SW_TYPE_JHFSW == INSTR[channel].SW_TYPE_2) || (SW_TYPE_COFFSW02 == INSTR[channel].SW_TYPE_2)) && sw_no_init_2)
				{
					errChk(Fiber_SW_Init(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, &INSTR[channel].SW_Handle_2, INSTR[channel].SW_SN_2));
					
					sw_no_init_2=0;
				}
				else if ((SW_TYPE_10G == INSTR[channel].SW_TYPE_2) || (0 == sw_no_init_2))
				{
					INSTR[channel].SW_Handle_2 = INSTR[channel-1].SW_Handle_2;
				}
				else if ((SW_TYPE_NONE == INSTR[channel].SW_TYPE_2) || (0==sw_no_init_2))
				{
					;
				}
				else 
				{
					sprintf (buf, "ͨ��%d �⿪���ͺ����ô���    ", channel); 
					MessagePopup("Error", buf);
					error=-1;
					goto Error;
				} 
		
				// for clock Ŀǰֻ֧��һ��clock���԰壬��ѡ8ͨ��
				if (INSTR[channel].CLOCK_TYPE == CLOCK_TYPE_SEVB0027_4001 && clock_no_init)
				{
					errChk(SEVB0027_4001_Init(&inst_SEVB0027_4001, INSTR[channel].CLOCK));
				
					clock_no_init=0;
				}
				else if(CLOCK_TYPE_NONE == INSTR[channel].CLOCK_TYPE || 0==clock_no_init)   /***û��ʱ�Ӱ壬�򲻳�ʼ��**Eric.Yao**20130904***/
				{
					;
				}
				else 
				{
					sprintf (buf, "ͨ��%d ʱ���л����ͺ����ô���    ", channel); 
					MessagePopup("Error", buf);
					error=-1;
					goto Error;
				} 
				
				//Set SW Channel
				if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
				{
					if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
					{
						error=-1;
						goto Error;
					}
				}
				if (SW_TYPE_NONE != INSTR[channel].SW_TYPE_2)     
				{
					if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
					{
						error=-1;
						goto Error;
					}
				}
				
				// for fiber
				if (!my_struConfig.isNPI)
				{
					if((MyDLL_FiberManage_Init (hdbc, INSTR[channel].Fiber))<0)
					{
						sprintf (buf, "ͨ��%d ���߲���ʹ�ã�    ", channel); 
						MessagePopup("Error", buf);
						error=-1;
						goto Error;
					}
				}
			} //if (INSTR[channel].ChannelEn)
		} //for (channel=0;channel<CHANNEL_MAX;channel++) 
	}

	SetCtrlVal (panMain, PAN_MAIN_LED, 1);
	HidePanel (panel);
	*error_channel = 0;
//	channel = 0;
	errChk(GetEnableChannel(&channel));  
	if (ini_Flag) 
	{
		DiscardProgressDialog (progresshandle);
	}  
	
	strcpy (Info, "ϵͳ��ʼ�����ɹ�");
	Insert_Info(panMain, gCtrl_BOX[channel], Info); 
	my_struSupFunCfgCali.Model_power_on[0] = -1;
	return 0;
Error:
	
	if (ini_Flag) 
	{
		DiscardProgressDialog (progresshandle);
	}
		
	HidePanel (panel); 
	
	*error_channel = channel;
	
	sprintf (Info, "ϵͳ��ʼ����ʧ�� ͨ��%d�����쳣", channel);
	Insert_Info(panMain, gCtrl_BOX[channel], Info);
	
	return -1; 
}

int DB_Initial (void)
{
	int err;
		
	err = MyDLL_DB_Init (&hdbc); 
	if (err) 
	{
		MessagePopup("��ʾ","���ݿ��ʼ��ʧ�ܣ�");
		return -1;
	}

	return 0;
}

int DB_Close (void)
{
int resCode = 0;   /* Result code                      */ 
int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	if (hdbc>0)
	{
		resCode = DBDisconnect (hdbc);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		
		hdbc=0;
	}
	
	return 0;
}

int quittest (void)
{
	int error;
	int channel;
	
	if (!my_struConfig.isNPI)
	{
		for (channel=0; channel<CHANNEL_MAX;channel++)
		{
			if (INSTR[channel].ChannelEn)
			{
				errChk(MyDLL_FiberManage_End (hdbc, INSTR[channel].Fiber));
			}
		}
	}

	errChk(DB_Close());
	
Error:
	return error;
}

int SetConfig_Inst (void)
{
	IniText g_myInifile = 0;
	int error, i;
	char Section_Name[256]; 

	//����ini�ļ����
	g_myInifile = Ini_New (0); 
	
	for (i=0; i<CHANNEL_MAX; i++)
	{
		sprintf (Section_Name, "Channel %d", i);
		
		errChk(Ini_PutInt (g_myInifile, Section_Name, "CHANNEL_EN", INSTR[i].ChannelEn));  
		errChk(Ini_PutInt (g_myInifile, Section_Name, "CHANNEL", INSTR[i].Channel));
		
		//for power meter
		errChk(Ini_PutInt (g_myInifile, Section_Name, "PM_TYPE", INSTR[i].PM_TYPE));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "PM_COM", INSTR[i].PM));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SN_PM", INSTR[i].SN_PM));
	
		//for evb
		errChk(Ini_PutInt (g_myInifile, Section_Name, "EVB_TYPE", INSTR[i].SEVB_TYPE));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SEVB", INSTR[i].SEVB));
		
		//for att 
		errChk(Ini_PutInt (g_myInifile, Section_Name, "ATT_TYPE_ONU", INSTR[i].ATT_TYPE_ONU));
		errChk(Ini_PutString (g_myInifile, Section_Name, "GVPM_ONU", INSTR[i].GVPM_ONU));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "ATT_COM", INSTR[i].ATT_COM));
	
		//for bert
		errChk(Ini_PutInt (g_myInifile, Section_Name, "BERT_TYPE_ONU", INSTR[i].BERT_TYPE_ONU));
		errChk(Ini_PutString (g_myInifile, Section_Name, "BERT_ONU", INSTR[i].BERT_ONU));
		
		//for att olt
		errChk(Ini_PutInt (g_myInifile, Section_Name, "ATT_TYPE_OLT", INSTR[i].ATT_TYPE_OLT));
		errChk(Ini_PutString (g_myInifile, Section_Name, "GVPM_OLT", INSTR[i].GVPM_OLT));	 
	
		//for bert olt
		errChk(Ini_PutInt (g_myInifile, Section_Name, "BERT_TYPE_OLT", INSTR[i].BERT_TYPE_OLT));
		errChk(Ini_PutString (g_myInifile, Section_Name, "BERT_OLT", INSTR[i].BERT_OLT));
		
		//for spectrum
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SPECTRUM_TYPE", INSTR[i].SPECTRUM_TYPE));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SPECTRUM", INSTR[i].SPECTRUM));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SN_SPECTRUM", INSTR[i].SN_SPECTRUM));
	
		//for DCA
		errChk(Ini_PutInt (g_myInifile, Section_Name, "DCA_TYPE", INSTR[i].DCA_TYPE));
		errChk(Ini_PutString (g_myInifile, Section_Name, "DCA", INSTR[i].DCA));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SN_DCA", INSTR[i].SN_DCA));
	
		//for fiber
		errChk(Ini_PutString (g_myInifile, Section_Name, "Fiber", INSTR[i].Fiber));
		
		// for sw
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_TYPE", INSTR[i].SW_TYPE));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_COM", INSTR[i].SW));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_CHAN", INSTR[i].SW_CHAN));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SW_SN", INSTR[i].SW_SN));
		
		// for sw2
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_TYPE_2", INSTR[i].SW_TYPE_2));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_COM_2", INSTR[i].SW_2));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "SW_CHAN_2", INSTR[i].SW_CHAN_2));
		errChk(Ini_PutString (g_myInifile, Section_Name, "SW_SN_2", INSTR[i].SW_SN_2));
		
		// for clock
		errChk(Ini_PutInt (g_myInifile, Section_Name, "CLOCK_TYPE", INSTR[i].CLOCK_TYPE));
		errChk(Ini_PutString (g_myInifile, Section_Name, "CLOCK", INSTR[i].CLOCK));
		errChk(Ini_PutInt (g_myInifile, Section_Name, "CLOCK_CHAN", INSTR[i].CLOCK_CHAN));
	}
	
	errChk(Ini_WriteToFile (g_myInifile, g_InstFileName));

Error:

	//�ͷ�ini�ļ����
	Ini_Dispose (g_myInifile);
	
	return error;
}

int GetConfig_Inst (void)
{
	IniText g_myInifile = 0;
	int error=0, i;
	char Section_Name[256]; 

	//����ini�ļ����
	g_myInifile = Ini_New (0); 
	
	//ָ��ini�ļ�����ȡini�ļ���Ϣ
	Ini_ReadFromFile (g_myInifile, g_InstFileName); 
	
	for (i=0; i<CHANNEL_MAX; i++)
	{
		sprintf (Section_Name, "Channel %d", i);
	
		errChk(Ini_GetInt (g_myInifile, Section_Name, "CHANNEL_EN", &INSTR[i].ChannelEn)); 
		errChk(Ini_GetInt (g_myInifile, Section_Name, "CHANNEL", &INSTR[i].Channel));
		
		//for power meter
		errChk(Ini_GetInt (g_myInifile, Section_Name, "PM_TYPE", &INSTR[i].PM_TYPE));   
		errChk(Ini_GetInt (g_myInifile, Section_Name, "PM_COM", &INSTR[i].PM));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SN_PM", INSTR[i].SN_PM, 30));
	
		//for evb
		errChk(Ini_GetInt (g_myInifile, Section_Name, "EVB_TYPE", &INSTR[i].SEVB_TYPE));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SEVB", INSTR[i].SEVB, 30));
	
		//for att
		errChk(Ini_GetInt (g_myInifile, Section_Name, "ATT_TYPE_ONU", &INSTR[i].ATT_TYPE_ONU));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "GVPM_ONU", INSTR[i].GVPM_ONU, 30));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "ATT_COM", &INSTR[i].ATT_COM));  
		//for bert
		errChk(Ini_GetInt (g_myInifile, Section_Name, "BERT_TYPE_ONU", &INSTR[i].BERT_TYPE_ONU));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "BERT_ONU", INSTR[i].BERT_ONU, 30));
		
		//for att
		errChk(Ini_GetInt (g_myInifile, Section_Name, "ATT_TYPE_OLT", &INSTR[i].ATT_TYPE_OLT));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "GVPM_OLT", INSTR[i].GVPM_OLT, 30));
		//for bert OLT
		errChk(Ini_GetInt (g_myInifile, Section_Name, "BERT_TYPE_OLT", &INSTR[i].BERT_TYPE_OLT));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "BERT_OLT", INSTR[i].BERT_OLT, 30));
	
		//for spectrum
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SPECTRUM_TYPE", &INSTR[i].SPECTRUM_TYPE));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SPECTRUM", INSTR[i].SPECTRUM, 30));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SN_SPECTRUM", INSTR[i].SN_SPECTRUM, 30));
	
		//for DCA
		errChk(Ini_GetInt (g_myInifile, Section_Name, "DCA_TYPE", &INSTR[i].DCA_TYPE));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "DCA", INSTR[i].DCA, 30));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SN_DCA", INSTR[i].SN_DCA, 30));
			
		//for fiber
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "Fiber", INSTR[i].Fiber, 30));
		
		// for sw
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_TYPE", &INSTR[i].SW_TYPE));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_COM", &INSTR[i].SW));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_CHAN", &INSTR[i].SW_CHAN));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SW_SN", INSTR[i].SW_SN, 30)); 
		
		// for sw 2
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_TYPE_2", &INSTR[i].SW_TYPE_2));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_COM_2", &INSTR[i].SW_2));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "SW_CHAN_2", &INSTR[i].SW_CHAN_2));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "SW_SN_2", INSTR[i].SW_SN_2, 30)); 
		
		// for clock
		errChk(Ini_GetInt (g_myInifile, Section_Name, "CLOCK_TYPE", &INSTR[i].CLOCK_TYPE));
		errChk(Ini_GetStringIntoBuffer (g_myInifile, Section_Name, "CLOCK", INSTR[i].CLOCK, 30));
		errChk(Ini_GetInt (g_myInifile, Section_Name, "CLOCK_CHAN", &INSTR[i].CLOCK_CHAN));
	}
	
Error:

	//�ͷ�ini�ļ����
	Ini_Dispose (g_myInifile);
	if(error<0)
	{
		MessagePopup("��ʾ","��ȡ�豸������Ϣʧ�ܣ�");
	}
	return error;
}

void InsertTree(int panel, int control, struct struInstrument *localInst, int collapsed)
{
	char label[256];
	int ParentIndex, cnt, ChildIndex;
	
	//��ȡͨ����index
	sprintf (label, "ͨ��%d", localInst->Channel);
	GetTreeItemFromLabel (panel, control, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, label, &ParentIndex);
	if (ParentIndex>=0)
	{   //ɾ��ͨ����Ӧ���豸���������
		GetNumTreeItems (panel, control, VAL_CHILD, ParentIndex, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, &cnt);
		DeleteListItem (panel, control, ParentIndex+1, cnt);
	}
	else
	{
		//��ӺϷ���ͨ��
		ParentIndex=InsertTreeItem (panel, control, VAL_SIBLING, 0, VAL_LAST, label, "", NULL, 0); 
	}
	
	//����ͨ��ͼ��
	if (localInst->ChannelEn)
	{
		SetTreeItemAttribute (panel, control, ParentIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_OK); 
		SetTreeItemAttribute (panel, control, ParentIndex, ATTR_COLLAPSED_IMAGE_INDEX, TREE_IMAGE_OK); 	
	}
	else
	{
		SetTreeItemAttribute (panel, control, ParentIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
		SetTreeItemAttribute (panel, control, ParentIndex, ATTR_COLLAPSED_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	
	//����豸
	sprintf (label, LAB_PM,localInst->PM_TYPE,localInst->PM,localInst->SN_PM);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (PowMeter_TYPE_NONE == localInst->PM_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_SEVB,localInst->SEVB_TYPE, localInst->SEVB);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (SEVB_TYPE_NONE == localInst->SEVB_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_ATT_MAI,localInst->ATT_TYPE_ONU, localInst->GVPM_ONU,localInst->ATT_COM);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (ATT_TYPE_NONE == localInst->ATT_TYPE_ONU)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}  

	sprintf (label, LAB_BERT,localInst->BERT_TYPE_ONU, localInst->BERT_ONU);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (BERT_TYPE_NONE == localInst->BERT_TYPE_ONU)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_ATT_OLT,localInst->ATT_TYPE_OLT, localInst->GVPM_OLT);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (ATT_TYPE_NONE == localInst->ATT_TYPE_OLT)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}  

	sprintf (label, LAB_BERT_OLT,localInst->BERT_TYPE_OLT, localInst->BERT_OLT);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (BERT_TYPE_NONE == localInst->BERT_TYPE_OLT)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_SPECTRUM,localInst->SPECTRUM_TYPE, localInst->SPECTRUM,localInst->SN_SPECTRUM);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (SPECTRUM_TYPE_NONE == localInst->SPECTRUM_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_DCA,localInst->DCA_TYPE, localInst->DCA,localInst->SN_DCA);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (DCA_TYPE_NONE == localInst->DCA_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	sprintf (label, LAB_FIBER,localInst->Fiber);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	
	//�⿪��
	sprintf (label, LAB_SW,localInst->SW_TYPE,localInst->SW,localInst->SW_CHAN, localInst->SW_SN); 
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (SW_TYPE_NONE == localInst->SW_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	//�⿪��2
	sprintf (label, LAB_SW_2,localInst->SW_TYPE_2,localInst->SW_2,localInst->SW_CHAN_2, localInst->SW_SN_2); 
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (SW_TYPE_NONE == localInst->SW_TYPE_2)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}

	
	//ʱ���л���
	sprintf (label, LAB_CLOCK,localInst->CLOCK_TYPE,localInst->CLOCK,localInst->CLOCK_CHAN);
	ChildIndex=InsertTreeItem (panel, control, VAL_CHILD, ParentIndex, VAL_LAST, label, "", NULL, 0);
	if (CLOCK_TYPE_NONE == localInst->CLOCK_TYPE)
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_ERROR); 
	}
	else
	{
		SetTreeItemAttribute (panel, control, ChildIndex, ATTR_IMAGE_INDEX, TREE_IMAGE_WARING); 
	}
	
	//������SortTreeItemsǰ������ParentIndexֵ��ؼ���ʵ��λ�ò�һ��
	SetTreeItemAttribute (panel, control, ParentIndex, ATTR_COLLAPSED, collapsed); 
	
	//����������
	SortTreeItems (panel, control, ParentIndex, 0, 0, 0, 0, 0);
}

int DB_Read_pch (char pn[], char bom[], int panel, int control_pch)
{   
	int  error, num;
	char buf[2056], str[256] = "";
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	char Versin_str[256];
	char materiel_code_str[256];
	char xlmfw[128], first[50], last[50]; 
	
	//��ȡ��������
	sprintf (buf, "SELECT distinct pch_tc, version,materiel_code,csxlmfw FROM sgd_scdd_trx where state ='�´�' and partnumber like '%s%%'  order by pch_tc", pn);	 /***���κ�����bom��������**Eric.Yao***/
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	
	resCode = DBBindColChar (hstmt, 1, 256, str, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 2, 256, Versin_str, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 3, 256, materiel_code_str, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	resCode = DBBindColChar (hstmt, 4, 128, xlmfw, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	
    num=0;
	Combo_DeleteComboItem (panel, control_pch, 0, -1);
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
	{
		sprintf(str,"%s;%s;%s",str,Versin_str,materiel_code_str);   
		Combo_InsertComboItem (panel, control_pch, num, str);   
		num++;
	}     
	
	SetCtrlVal (panel, control_pch, str);   
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) 
	{ShowDataBaseError(); return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	return 0;
}
int DBOA_Read_pch (char pn[], char bom[], int panel, int control_pch)
{   
	int  error, num;
	char buf[2056], str[256] = "";
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	error = DBOA_Init ();
	if (error) return -1;

	//��ȡ��������
	sprintf (buf, "SELECT distinct pch_tc FROM sgd_scdd_trx where state ='release' and cpxh='%s' and barebom = 'PA%s' order by pch_tc", pn, bom);	 /***���κ�����bom��������**Eric.Yao***/
	
	hstmt = DBActivateSQL (hdbc_oa, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	
	resCode = DBBindColChar (hstmt, 1, 256, str, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
    num=0;
	Combo_DeleteComboItem (panel, control_pch, 0, -1);
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
	{
		Combo_InsertComboItem (panel, control_pch, num, str);   
		num++;
	}     
	
	SetCtrlVal (panel, control_pch, str);   
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) 
	{ShowDataBaseError(); return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	error = DBOA_Close ();
	if (error) return -1;
	   
	return 0;
}
int DBOA_Init (void)
{
	int err;
	
	err = MyDLL_DBOA_Init (&hdbc_oa);
	if (err) return -1;
	
	return 0;
}

int DBOA_Close (void)
{
int resCode = 0;   /* Result code                      */ 
int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	if (hdbc_oa>0)
	{
		resCode = DBDisconnect (hdbc_oa);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		
		hdbc_oa=0;
	}
	
	return 0;
}

int DBOA_Read_Order (char order[])
{   
int  error, num;
char buf[2056], xlmfw[128], first[50], last[50];
int resCode = 0;   /* Result code                      */ 
int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	error = DBOA_Init ();
	if (error) return -1;

	sprintf (buf, "SELECT csxlmfw FROM sgd_scdd_trx where pch_tc='%s'", order);
	hstmt = DBActivateSQL (hdbc_oa, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	
	resCode = DBBindColChar (hstmt, 1, 128, xlmfw, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
    num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) 
	{ShowDataBaseError(); return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	error = DBOA_Close ();
	if (error) return -1;

	if (num != 1) {return -1;} 
	
	//��ȡ���к�
	Scan(xlmfw, "%s>%s[xt45]%s[xt45]", first, last);
	
	MyDLL_TRIM(first); 
	MyDLL_TRIM(last);
	
	Scan(first, "%s>%s[w14]", first);
	Scan(last, "%s>%s[w14]", last);     
	
	strcpy(my_struConfig.firstsn, first);
	strcpy(my_struConfig.lastsn, last);

	return 0;
}
int DB_Read_Order (char order[])
{   
	int  error, num;
	char buf[2056], xlmfw[128], first[50], last[50];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	sprintf (buf, "SELECT csxlmfw FROM sgd_scdd_trx where pch_tc='%s'", order);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	
	resCode = DBBindColChar (hstmt, 1, 128, xlmfw, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

    num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) 
	{ShowDataBaseError(); return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (num != 1) {return -1;} 
	
	//��ȡ���к�
	Scan(xlmfw, "%s>%s[xt45]%s[xt45]", first, last);
	
	MyDLL_TRIM(first); 
	MyDLL_TRIM(last);
	
	Scan(first, "%s>%s[w14]", first);
	Scan(last, "%s>%s[w14]", last);     
	
	strcpy(my_struConfig.firstsn, first);
	strcpy(my_struConfig.lastsn, last);

	return 0;
}

int DB_Get_BOM(int panel, int control)
{
	int num;
	char buf[1024],bom[50];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	Fmt (buf, "SELECT Version from AutoDT_Spec_BarCode where PN='%s' ORDER BY Version ASC", my_struConfig.PN);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColChar (hstmt, 1, 50, bom, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
	ClearListCtrl (panel, control); 
	num=0; 
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
		InsertListItem (panel, control, num, bom, num); 
		num++;
	}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_BarCode�в����ҵ���Ӧ�����ݣ�"); return -1;}
	
	return 0;
}
/*
BOOL DB_Get_Barcode_OSA(char *PN)		//OSA����������
{
	char buf[1024];
	char barcode[20]="", sn_F[10]="";
	int num, myCount;
	int resCode = 0;   // Result code                      // 
	int hstmt = 0;	   // Handle to SQL statement          // 
   	
	Fmt (buf, "SELECT Barcode, SNLength FROM AutoDT_Spec_BarCode where PN='%s'", PN);  
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt<=0) {ShowDataBaseError(); return FALSE;}  	
	
	resCode = DBBindColChar (hstmt, 1, 10, my_struConfig.Barcode, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	resCode = DBBindColInt (hstmt, 2, &my_struConfig.SNLength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
    
    num=0;							 
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return FALSE;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}

	if (num!=1) {MessagePopup("Error", "AutoDT_Spec_BarCode���ñ���û�л��ж����˲�Ʒ�ļ�¼��     ");return FALSE;}   

	return TRUE;
}
*/
int DB_Get_PN_TYPE (void)
{
	char buf[1024];
	char myTYPE[50];
	int	 num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	Fmt (buf, "SELECT PN_TYPE from AutoDT_Spec_Tracking WHERE PartNumber='%s' and BarBOM='%s'", my_struConfig.PN, my_struConfig.BOM); 
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColChar (hstmt, 1, 50, myTYPE, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
    	num++;
    }      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_Tracking�в����ҵ���Ӧ�����ݣ�"); return -1;} 

 	if (stricmp (myTYPE, "ONU") ==0 || stricmp (myTYPE, "DWDM_ONU") ==0) 
	{
		my_struConfig.PN_TYPE=PN_TYPE_ONU;
	} 
	else if (0==stricmp (myTYPE, "ONU_DDMI"))
	{
		my_struConfig.PN_TYPE=PN_TYPE_ONU_DDMI;
	}
	else 
	{
		MessagePopup ("��ʾ", "���ݿ�AutoDT_Spec_Tracking�ж����PN_TYPE�д���");
		return -1;
	} 
	
	return 0;
}

int DB_Get_EED_Ver (int panel, int control)
{
char buf[256];
char myVer[10];
int	num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	Fmt (buf, "SELECT DISTINCT(Version) FROM AutoDT_Spec_E2_Data where PartNumber='%s' ORDER BY Version ASC", my_struConfig.PN);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColChar (hstmt, 1, 10, myVer, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
	ClearListCtrl (panel, control);
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
    	InsertListItem (panel, control, num, myVer, num); 
    	num++;
    }      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError(); return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ���Ӧ�����ݣ�"); return -1;} 

	return 0;
}

int DB_Read_Spec_ATE_Item (void)  
{								  
	char buf[1024];
	int	 num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	Fmt (buf, "SELECT T_AOP,T_ER,T_Dis,T_Eye,T_Mask,T_Spectrum,R_Sens,R_OverLoad,R_SDHys,Calibrate,Calibrate_Test,EEPROM_Protect,T_SD,T_Wavelength,ONU_TO_OLT FROM AutoDT_Spec_ATE_Item WHERE PN='%s' AND DT_Flag='%s' AND Temper_Flag='%s'",my_struConfig.PN, my_struConfig.DT_Flag, my_struConfig.Temper_Flag);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColInt (hstmt, 1, &my_struConfig.Sel_T_AOP, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 2, &my_struConfig.Sel_T_ER, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 3, &my_struConfig.Sel_T_TxDis, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 4, &my_struConfig.Sel_T_Eye, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 5, &my_struConfig.Sel_T_Mask, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 6, &my_struConfig.Sel_T_Spectrum, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 7, &my_struConfig.Sel_R_Sens, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 8, &my_struConfig.Sel_R_Over, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 9, &my_struConfig.Sel_R_SDHys, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 10, &my_struConfig.Sel_Calibrate, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 11, &my_struConfig.Sel_Calibrate_Test, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 12, &my_struConfig.Sel_EE_P, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 13, &my_struConfig.Sel_TxSD, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 14, &my_struConfig.Sel_T_Wavelength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 15, &my_struConfig.Sel_T_Upstream, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (num<1)
	{MessagePopup ("ERROR", "��ȡ���ݿ�AutoDT_Spec_ATE_Item�еļ�¼ʧ��"); return -1;}  
	
	return 0;
}

int GetConfig(void)
{
IniText g_myInifile = 0;
int error;

	//����ini�ļ����
	g_myInifile = Ini_New (0); 
	
	//ָ��ini�ļ�����ȡini�ļ���Ϣ
	Ini_ReadFromFile (g_myInifile, g_ConfigFileName); 
	
	//��ȡָ��Section��Item��ItemValue�ַ���
	errChk(Ini_GetInt (g_myInifile, "Configuration Set", "PNIndex", &my_struConfig.PNIndex));
	errChk(Ini_GetStringIntoBuffer (g_myInifile, "Configuration Set", "PN", my_struConfig.PN, 50));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_Tuning", &my_struConfig.DT_Tuning));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_Test_RX", &my_struConfig.DT_Test_RX));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_Test", &my_struConfig.DT_Test));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_QA", &my_struConfig.DT_QATest));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_OQA", &my_struConfig.DT_OQA));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_Tun_RX", &my_struConfig.DT_Tun_RX)); 
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_Test_Upstream", &my_struConfig.DT_Test_Upstream));  
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Fun_OSA_WL_Test", &my_struConfig.DT_OSA_WL_Test));     
	
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Agin_Front", &my_struConfig.DT_Agin_Front));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Agin_Back", &my_struConfig.DT_Agin_Back));
	
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Tun_Reliability", &my_struConfig.DT_Tun_Reliability));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Test_Reliability", &my_struConfig.DT_Test_Reliability));
	
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Temper_Low", &my_struConfig.Temper_Low));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Temper_Room", &my_struConfig.Temper_Room));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Temper_High", &my_struConfig.Temper_High));

	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Sel_R_Sens_Detail", &my_struConfig.Sel_R_Sens_Detail));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "Sel_RxSD_Detail", &my_struConfig.Sel_RxSD_Detail)); 
	
	//�߼�����������
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "NPI", &my_struConfig.isNPI));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "ONUtoOLT", &my_struConfig.isONUtoOLT));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "QAUpdateSN", &my_struConfig.isQAUpdateSN));
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "ScanModuleSN", &my_struConfig.isScanModuleSN));       
	errChk(Ini_GetBoolean (g_myInifile, "Configuration Set", "QANoCheckBOM", &my_struConfig.isQANoCheckBOM)); 
	
	errChk(Ini_GetDouble (g_myInifile, "Configuration Set", "AOP_AginDelta", &my_struConfig.AOP_AginDelta));  
	errChk(Ini_GetDouble (g_myInifile, "Configuration Set", "WL_AginDelta", &my_struConfig.WL_AginDelta)); 
	errChk(Ini_GetDouble (g_myInifile, "Configuration Set", "WLMeterOffset", &my_struConfig.WLMeterOffset));  
	
Error:

	//�ͷ�ini�ļ����
	Ini_Dispose (g_myInifile);
	if(error<0)
	{
		MessagePopup("��ʾ","��ȡ����������Ϣʧ�ܣ�");  
	}
	return error;
}

int SetConfig(void)
{
IniText g_myInifile = 0;
int error;

	//����ini�ļ����
	g_myInifile = Ini_New (0); 
	
	//д��ָ��Section��Item��ItemValue�ַ���
	errChk(Ini_PutInt (g_myInifile, "Configuration Set", "PNIndex", my_struConfig.PNIndex));
	errChk(Ini_PutString (g_myInifile, "Configuration Set", "PN", my_struConfig.PN));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_Tuning", my_struConfig.DT_Tuning));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_Tun_RX", my_struConfig.DT_Tun_RX));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_Test_RX", my_struConfig.DT_Test_RX));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_Test", my_struConfig.DT_Test));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_QA", my_struConfig.DT_QATest));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_OQA", my_struConfig.DT_OQA));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_Test_Upstream", my_struConfig.DT_Test_Upstream));  
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Fun_OSA_WL_Test", my_struConfig.DT_OSA_WL_Test));   
	
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Agin_Front", my_struConfig.DT_Agin_Front));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Agin_Back", my_struConfig.DT_Agin_Back));
	
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Tun_Reliability", my_struConfig.DT_Tun_Reliability));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Test_Reliability",my_struConfig.DT_Test_Reliability));

	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Temper_Low", my_struConfig.Temper_Low));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Temper_Room", my_struConfig.Temper_Room));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Temper_High", my_struConfig.Temper_High));
	
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Sel_RxSD_Detail", my_struConfig.Sel_RxSD_Detail)); 
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "Sel_R_Sens_Detail", my_struConfig.Sel_R_Sens_Detail));
	
	//�߼�����������
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "NPI", my_struConfig.isNPI));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "ONUtoOLT", my_struConfig.isONUtoOLT));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "QAUpdateSN", my_struConfig.isQAUpdateSN));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "ScanModuleSN", my_struConfig.isScanModuleSN));
	errChk(Ini_PutBoolean (g_myInifile, "Configuration Set", "QANoCheckBOM", my_struConfig.isQANoCheckBOM));
	
	errChk(Ini_PutDouble (g_myInifile, "Configuration Set", "AOP_AginDelta", my_struConfig.AOP_AginDelta));
	errChk(Ini_PutDouble (g_myInifile, "Configuration Set", "WL_AginDelta", my_struConfig.WL_AginDelta));  
	errChk(Ini_PutDouble (g_myInifile, "Configuration Set", "WLMeterOffset", my_struConfig.WLMeterOffset)); 
	//my_struConfig.isScanModuleSN
	errChk(Ini_WriteToFile (g_myInifile, g_ConfigFileName));
	
Error:
	
	//�ͷ�ini�ļ����
	Ini_Dispose (g_myInifile);
	
	return error;
}

int DB_Get_ConfigInfo (void)
{
	char buf[2048];
	char spec_flag[256];
	int  num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	if     (0==stricmp(my_struConfig.DT_Flag, "TUNING")) {strcpy (spec_flag, "TUNING");}
	else if(0==stricmp(my_struConfig.DT_Flag, "TUN_RX")) {strcpy (spec_flag, "TUN_RX");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST"))   {strcpy (spec_flag, "TEST");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_RX")){strcpy (spec_flag, "TEST");} 		
	else if(0==stricmp(my_struConfig.DT_Flag, "QATEST")) {strcpy (spec_flag, "QATEST");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "OQA")) 	 {strcpy (spec_flag, "QATEST");}      /***OQA����������QA����һ��**Eric.Yao***/
	else if(0==stricmp(my_struConfig.DT_Flag, "AGIN_FRONT"))  {strcpy (spec_flag, "AGIN_FRONT");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "AGIN_BACK")) 	 {strcpy (spec_flag, "AGIN_BACK");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TUN_Reliability")) {strcpy (spec_flag, "TUN_Reliability");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_Reliability"))   {strcpy (spec_flag, "TEST_Reliability");}
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_Upstream"))   {strcpy (spec_flag, "TEST_Upstream");}   
	else							 					 {strcpy (spec_flag, "");}

if (!my_struConfig.DT_OSA_WL_Test)
{
	Fmt (buf, "SELECT TemperMin, TemperMax, TxPowMin, TxPowMax, TxErMin, TxErMax, TxTjMax, TxPPjMax, TxRiseMax, TxFallMax, TxDisPowMax, TrackingErrorMax,\
		PathPenaltyMax, PathPenaltyTime, RxAmpMin, RxAmpMax, RxTjMax, RxPPjMax, RxRiseMax, RxFallmax, OverLoad, OverLoadTime, Sens,SensTime, SDAssert, SDDeassert,\
		SDHysMin, SDHysMax, PeakWavelengthMin, PeakWavelengthMax, Sigma, BandWidth, SMSR, TxVMax, TxIMax, RxVMax, RxIMax, TE_Min, TE_Max,Center_Sens,Center_Sens_Time,\
		PeakWavelengthMin02, PeakWavelengthMax02, PeakWavelengthMin03, PeakWavelengthMax03,PeakWavelengthMin04, PeakWavelengthMax04, WAVELENGTHCHANNEL01, WAVELENGTHCHANNEL02,\
		WAVELENGTHCHANNEL03, WAVELENGTHCHANNEL04, TXOFFPOWER_MIN,TXOFFPOWER_MAX,WAVELENGTH_DELTA \
		FROM AutoDT_Spec_ATE WHERE PartNumber='%s' and Version='%s' and DT_Flag='%s' and Temper_Flag='%s'", my_struConfig.PN, my_struConfig.BOM,spec_flag,my_struConfig.Temper_Flag); 
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	resCode = DBBindColDouble (hstmt, 1, &my_struDBConfig.TemperMin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 2, &my_struDBConfig.TemperMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 3, &my_struDBConfig.TxPowMin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 4, &my_struDBConfig.TxPowMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 5, &my_struDBConfig.TxErMin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 6, &my_struDBConfig.TxErMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 7, &my_struDBConfig.TxTjMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 8, &my_struDBConfig.TxPPjMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 9, &my_struDBConfig.TxRiseMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 10, &my_struDBConfig.TxFallMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 11, &my_struDBConfig.TxDisPowMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 12, &my_struDBConfig.TrackingErrorMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 13, &my_struDBConfig.PathPenaltyMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 14, &my_struDBConfig.PathPenaltyTime, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}   
	resCode = DBBindColDouble (hstmt, 15, &my_struDBConfig.RxAmpMin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 16, &my_struDBConfig.RxAmpMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 17, &my_struDBConfig.RxTjMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 18, &my_struDBConfig.RxPPjMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 19, &my_struDBConfig.RxRiseMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 20, &my_struDBConfig.RxFallMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 21, &my_struDBConfig.Over, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 22, &my_struDBConfig.OverTime, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 23, &my_struDBConfig.Sens, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 24, &my_struDBConfig.SensTime, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 25, &my_struDBConfig.SDA, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 26, &my_struDBConfig.SDD, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 27, &my_struDBConfig.SDHysMin, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 28, &my_struDBConfig.SDHysMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
   	resCode = DBBindColDouble (hstmt, 29, &my_struDBConfig.PeakWavelengthMin[0], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 30, &my_struDBConfig.PeakWavelengthMax[0], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 31, &my_struDBConfig.Sigma, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 32, &my_struDBConfig.BandWidth, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 33, &my_struDBConfig.SMSR, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}   
   	resCode = DBBindColDouble (hstmt, 34, &my_struDBConfig.TxVMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 35, &my_struDBConfig.TxIMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 36, &my_struDBConfig.RxVMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 37, &my_struDBConfig.RxIMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	
   	resCode = DBBindColDouble (hstmt, 38, &my_struDBConfig.TEMin, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 39, &my_struDBConfig.TEMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColDouble (hstmt, 40, &my_struDBConfig.CenterSens, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 41, &my_struDBConfig.CenterSensTime, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}    
   	resCode = DBBindColDouble (hstmt, 42, &my_struDBConfig.PeakWavelengthMin[1], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 43, &my_struDBConfig.PeakWavelengthMax[1], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 44, &my_struDBConfig.PeakWavelengthMin[2], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 45, &my_struDBConfig.PeakWavelengthMax[2], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 46, &my_struDBConfig.PeakWavelengthMin[3], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 47, &my_struDBConfig.PeakWavelengthMax[3], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 	 				  
	resCode = DBBindColInt (hstmt, 48, &my_struDBConfig.WavelengthChannel[0], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColInt (hstmt, 49, &my_struDBConfig.WavelengthChannel[1], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColInt (hstmt, 50, &my_struDBConfig.WavelengthChannel[2], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColInt (hstmt, 51, &my_struDBConfig.WavelengthChannel[3], &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 52, &my_struDBConfig.TxOffPowerMin, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 53, &my_struDBConfig.TxOffPowerMax, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
   	resCode = DBBindColDouble (hstmt, 54, &my_struDBConfig.WavelengthDelta, &DBConfigStat); 
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

    num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
    	num++;
    }      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	sprintf (buf, "���ݿ�AutoDT_Spec_ATE�в����ҵ�DT_Flag=%s��Temper_Flag=%s�����ݻ��߶�Ӧ�����ݼ�¼����һ����", spec_flag, my_struConfig.Temper_Flag);
	if (num!=1) {MessagePopup("��ʾ",buf); return -1;}
}
	memset (my_struDBConfig.BarCode, 0, 10);
	my_struDBConfig.SNLength=0;
	
	Fmt (buf, "SELECT BarCode,SNLength from AutoDT_Spec_BarCode where PN='%s' and Version='%s'", my_struConfig.PN, my_struConfig.BOM);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColChar (hstmt, 1, 50, my_struDBConfig.BarCode, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColInt (hstmt, 2, &	my_struDBConfig.SNLength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
   
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num!=1) {MessagePopup("Error","Get Barcode fail  !"); return -1;} 
	
	MyDLLCheckSN (my_struDBConfig.BarCode);   

	return 0;
}

int DB_READ_AutoDT_Spec_ERCompens (void)
{
	char buf[1024], DriverChip[50], ERCompensChip[50];
	int  count;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */
	
	sprintf (buf, "SELECT ERCompensChip,Rb_Add,Rm_Add,DriverChip,LD_SE,CurrentLimit_80,Im10_33,Im60_33,Im82_60,IBias_MAX, Ib030_10, Ib10_33, Ib60_33, Ib82_60, Ib105_82,IBias_MIN FROM AutoDT_Spec_ERCompens WHERE PartNumber='%s' AND Version='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
    
	resCode = DBBindColChar (hstmt, 1, 50, ERCompensChip, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 2, 50, my_struDBConfig_ERCompens.Rb_Add, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 3, 50, my_struDBConfig_ERCompens.Rm_Add, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 4, 50, DriverChip, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 5, &my_struDBConfig_ERCompens.se, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 6, &my_struDBConfig_ERCompens.CurrentLimit_80, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 7, &my_struDBConfig_ERCompens.Im10_33, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 8, &my_struDBConfig_ERCompens.Im60_33, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 9, &my_struDBConfig_ERCompens.Im82_60, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 10, &my_struDBConfig_ERCompens.IBias_Max, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColDouble (hstmt, 11, &my_struDBConfig_ERCompens.Ib030_10, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}   
	resCode = DBBindColDouble (hstmt, 12, &my_struDBConfig_ERCompens.Ib10_33, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 13, &my_struDBConfig_ERCompens.Ib60_33, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 14, &my_struDBConfig_ERCompens.Ib82_60, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColDouble (hstmt, 15, &my_struDBConfig_ERCompens.Ib105_82, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColFloat (hstmt, 16, &my_struDBConfig_ERCompens.IBias_MaxDAC, &DBConfigStat);			//��IBias_Min���ã�
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) {MessagePopup("��ʾ","���ݿ���AutoDT_Spec_ERCompens�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����"); return -1;}
	
	//stricmp �Ƚ��ַ������Դ�Сд
	if      (stricmp (DriverChip, "VSC7965")==0) my_struDBConfig_ERCompens.DriverChip=DRIVERCHIP_VSC7965;
	else if (stricmp (DriverChip, "VSC7967")==0) my_struDBConfig_ERCompens.DriverChip=DRIVERCHIP_VSC7967;
	else if (stricmp (DriverChip, "NT25L90")==0) my_struDBConfig_ERCompens.DriverChip=DRIVERCHIP_NT25L90; 
	else if (stricmp (DriverChip, "UX3328")==0)  my_struDBConfig_ERCompens.DriverChip=DRIVERCHIP_UX3328;
	else if (stricmp (DriverChip, "UX3328S")==0)  my_struDBConfig_ERCompens.DriverChip=DRIVERCHIP_UX3328S;
	else    {MessagePopup ("Error", "DriverChip type is not know !"); return -1;}

	//stricmp �Ƚ��ַ������Դ�Сд
	if      (stricmp (ERCompensChip, "MEGA88")==0) my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_MEGA88;
	else if (stricmp (ERCompensChip, "TINY13")==0) my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_TINY13;
	else if (stricmp (ERCompensChip, "UX3328")==0) my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_UX3328;
	else if (stricmp (ERCompensChip, "UX3328S")==0) my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_UX3328S;
	else if (stricmp (ERCompensChip, "DS4830")==0) my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_DS4830; 
	else    {MessagePopup ("Error", "ERCompensChip type is not know !"); return -1;}
	
	return 0;
}

BOOL DB_GET_Spec_ERCompens_Ex(void)
{
	char buf[256];
	int	 num;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */
	
	Fmt (buf, "SELECT AOP_Slope,AOP_Offset,ER_Slope,ER_Offset FROM AutoDT_Spec_ERCompens_Ex WHERE PartNumber='%s' AND BarBOM='%s'", my_struConfig.PN, my_struConfig.BOM); 

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
    resCode = DBBindColFloat (hstmt, 1, &my_struDBConfig_ERCompens.AOP_Slope, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
    resCode = DBBindColFloat (hstmt, 2, &my_struDBConfig_ERCompens.AOP_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}	
    resCode = DBBindColFloat (hstmt, 3, &my_struDBConfig_ERCompens.ER_Slope, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}	
	resCode = DBBindColFloat (hstmt, 4, &my_struDBConfig_ERCompens.ER_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}	
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();  return FALSE;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	
	if (num!=1) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_ERCompens_Ex�в����ҵ����ݻ������ݼ�¼����һ����"); return FALSE;} 
	
	return TRUE;
}

int DB_Get_EEPROM (void)
{
int count, i, num;  
char buf[2048], buf_addr[2048], myEED_str[128][5];   
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	if (CUSTOMER_STANDARD == my_struConfig.CUSTOMER || CUSTOMER_AP == my_struConfig.CUSTOMER)
	{
		//����ַ�������
		memset (buf_addr, 0, sizeof(buf_addr));
		strcpy (buf_addr, "PartNumber");
	    for (i=0;i <128; i++)
	    {
	    	sprintf (buf, ",Address%d", i);
			strcat (buf_addr, buf);
	    }

		//for A0
		Fmt (buf, "SELECT %s FROM AutoDT_Spec_E2_Data WHERE PartNumber='%s' AND Version='%s' AND A2=0", buf_addr, my_struConfig.PN, my_struConfig.EED);

		hstmt = DBActivateSQL (hdbc, buf);
	    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	    for (i=0;i <128; i++)
	    {
		    resCode = DBBindColChar (hstmt, 2+i, 5, myEED_str[i], &DBConfigStat, "");
		    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		}

	    num=0;
	    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
	    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();return -1;}
    
	    resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ�A0�����ݻ������ݼ�¼����һ����"); return -1;} 

		for (i=0; i<128; i++)
	  	{sscanf (myEED_str[i], "%x", &my_struDBConfig_E2.A0[i]);}  

		//for A2
		Fmt (buf, "SELECT %s FROM AutoDT_Spec_E2_Data WHERE PartNumber='%s' AND Version='%s' AND A2=1", buf_addr, my_struConfig.PN, my_struConfig.EED);

		hstmt = DBActivateSQL (hdbc, buf);
	    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	    for (i=0;i <128; i++)
	    {
		    resCode = DBBindColChar (hstmt, 2+i, 5, myEED_str[i], &DBConfigStat, "");
		    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		}

	    num=0;
	    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
	    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();return -1;}
    
	    resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ�A2�����ݻ������ݼ�¼����һ����"); return -1;} 

		for (i=0; i<128; i++)
	  	{sscanf (myEED_str[i], "%x", &my_struDBConfig_E2.A2[i]);}  
	}  
	else if (CUSTOMER_AL == my_struConfig.CUSTOMER || CUSTOMER_01 == my_struConfig.CUSTOMER)
	{
		//����ַ�������
		memset (buf_addr, 0, sizeof(buf_addr));
		strcpy (buf_addr, "PartNumber");
	    for (i=0;i <128; i++)
	    {
	    	sprintf (buf, ",Address%d", i);
			strcat (buf_addr, buf);
	    }

		//for A0
		Fmt (buf, "SELECT %s FROM AutoDT_Spec_E2_Data WHERE PartNumber='%s' AND Version='%s' AND A2=0 AND table_flag='LOWER'", buf_addr, my_struConfig.PN,my_struConfig.EED);

		hstmt = DBActivateSQL (hdbc, buf);
	    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	    for (i=0;i <128; i++)
	    {
		    resCode = DBBindColChar (hstmt, 2+i, 5, myEED_str[i], &DBConfigStat, "");
		    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		}

	    num=0;
	    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
	    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();return -1;}
    
	    resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ�A0�����ݻ������ݼ�¼����һ����"); return -1;} 

		for (i=0; i<128; i++)
	  	{sscanf (myEED_str[i], "%x", &my_struDBConfig_E2.A0[i]);}  

		//for A2
		Fmt (buf, "SELECT %s FROM AutoDT_Spec_E2_Data WHERE PartNumber='%s' AND Version='%s' AND A2=1 AND table_flag='LOWER'", buf_addr, my_struConfig.PN, my_struConfig.EED);

		hstmt = DBActivateSQL (hdbc, buf);
	    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	    for (i=0;i <128; i++)
	    {
		    resCode = DBBindColChar (hstmt, 2+i, 5, myEED_str[i], &DBConfigStat, "");
		    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		}

	    num=0;
	    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
	    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();return -1;}
    
	    resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ�A2�����ݻ������ݼ�¼����һ����"); return -1;} 

		for (i=0; i<128; i++)
	  	{sscanf (myEED_str[i], "%x", &my_struDBConfig_E2.A2[i]);}  
		
		//for A2 table0
		Fmt (buf, "SELECT %s FROM AutoDT_Spec_E2_Data WHERE PartNumber='%s' AND Version='%s' AND A2=1 AND table_flag='TABLE0'", buf_addr, my_struConfig.PN,my_struConfig.EED);

		hstmt = DBActivateSQL (hdbc, buf);
	    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
	    for (i=0;i <128; i++)
	    {
		    resCode = DBBindColChar (hstmt, 2+i, 5, myEED_str[i], &DBConfigStat, "");
		    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		}

	    num=0;
	    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
	    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();return -1;}
    
	    resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		if (num==0) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_E2_Data�в����ҵ�A2 table0�����ݻ������ݼ�¼����һ����"); return -1;} 

		for (i=0; i<128; i++)
	  	{sscanf (myEED_str[i], "%x", &my_struDBConfig_E2.A2_Table0[i]);}  
	}
	else
	{
		MessagePopup("��ʾ","�ͻ����Ʋ�Ʒ�����쳣"); 
		return -1;	
	}
	
	return 0;
}

int DB_Read_Spec_Monitor_Ex(int Flag)  
{
	char buf[1024]; 
	int  num, i;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	//��ѯУ׼������
	Fmt (buf, "SELECT CalibrateNumber,CaliPoint1,CaliPoint2,CaliPoint3,CaliPoint4,CaliPoint5,CaliPoint6,CaliPoint7,CaliPoint8,CaliPoint9,CaliPoint10,CaliPoint11,CaliPoint12,CaliPoint13,CaliPoint14,CaliPoint15,CaliPoint16,CaliPoint17,CaliPoint18,CaliPoint19,CaliPoint20,CaliPoint21,CaliPoint22,CaliPoint23,CaliPoint24,CaliPoint25,CaliPoint26,CaliPoint27,CaliPoint28,CaliPoint29,CaliPoint30 FROM AutoDT_Spec_Monitor_Ex WHERE PartNumber='%s' AND BarBOM='%s' AND CalibrateFlag=0", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColInt (hstmt, 1, &my_struDBConfig_Monitor.CaliNumber, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	for (i=0; i<30; i++)
	{
		resCode = DBBindColFloat (hstmt, i+2, &my_struDBConfig_Monitor.CaliPoint[i], &DBConfigStat);
	    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	}
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (num!=1)
	{MessagePopup ("ERROR", "��ȡ���ݿ�AutoDT_Spec_Monitor_Ex�еļ�¼ʧ��"); return -1;}  
	
	//��ѯУ׼���Ե�����
	Fmt (buf, "SELECT CalibrateNumber,CaliPoint1,CaliPoint2,CaliPoint3,CaliPoint4,CaliPoint5,CaliPoint6,CaliPoint7,CaliPoint8,CaliPoint9,CaliPoint10,CaliPoint11,CaliPoint12,CaliPoint13,CaliPoint14,CaliPoint15,CaliPoint16,CaliPoint17,CaliPoint18,CaliPoint19,CaliPoint20,CaliPoint21,CaliPoint22,CaliPoint23,CaliPoint24,CaliPoint25,CaliPoint26,CaliPoint27,CaliPoint28,CaliPoint29,CaliPoint30 FROM AutoDT_Spec_Monitor_Ex WHERE PartNumber='%s' AND BarBOM='%s' AND CalibrateFlag=%d", my_struConfig.PN, my_struConfig.BOM, Flag);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColInt (hstmt, 1, &my_struDBConfig_Monitor.CaliNumber_Test, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	for (i=0; i<30; i++)
	{
		resCode = DBBindColFloat (hstmt, i+2, &my_struDBConfig_Monitor.CaliPoint_Test[i], &DBConfigStat);
	    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	}
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (num!=1)
	{MessagePopup ("ERROR", "��ȡ���ݿ�AutoDT_Spec_Monitor_Ex�еļ�¼ʧ��"); return -1;}  
	
	return 0;
}


int DB_Get_Config_Monitor_Info (void) 
{
	int  num;
	char buf[1024], spec_flag[30];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	if     (0==stricmp(my_struConfig.DT_Flag, "TUNING")) {strcpy (spec_flag, "TUNING");}
	else if(0==stricmp(my_struConfig.DT_Flag, "TUN_RX")) {strcpy (spec_flag, "TUN_RX");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST"))   {strcpy (spec_flag, "TEST");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_RX")){strcpy (spec_flag, "TEST");} 		
	else if(0==stricmp(my_struConfig.DT_Flag, "QATEST")) {strcpy (spec_flag, "QATEST");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "AGIN_FRONT"))  {strcpy (spec_flag, "AGIN_FRONT");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "AGIN_BACK")) 	 {strcpy (spec_flag, "AGIN_BACK");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TUN_Reliability")) {strcpy (spec_flag, "TUN_Reliability");} 
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_Reliability"))   {strcpy (spec_flag, "TEST_Reliability");}
	else if(0==stricmp(my_struConfig.DT_Flag, "TEST_Upstream"))   {strcpy (spec_flag, "TEST_Upstream");}  
	else							 					 {strcpy (spec_flag, "");}
	
	Fmt (buf, "SELECT Temper_Prec,Vcc_Prec,I_Bias_Prec,Tx_Pow_Prec,Rx_Pow_Prec FROM AutoDT_Spec_Monitor WHERE PartNumber='%s' AND Version='%s' AND DT_Flag='%s' AND Temper_Flag='%s'", my_struConfig.PN, my_struConfig.BOM, spec_flag, my_struConfig.Temper_Flag);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColFloat (hstmt, 1, &my_struDBConfig_Monitor.Temper_Prec, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 2, &my_struDBConfig_Monitor.Vcc_Prec, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 3, &my_struDBConfig_Monitor.I_Bias_Prec, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 4, &my_struDBConfig_Monitor.Tx_Pow_Prec, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 5, &my_struDBConfig_Monitor.Rx_Pow_Prec, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
    num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;} 
	
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError(); return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	Fmt (buf, "���ݿ�AutoDT_Spec_Monitor�в����ҵ�PartNumber='%s' AND Version='%s' AND DT_Flag='%s' AND Temper_Flag='%s'�����ݻ������ݼ�¼����һ��", my_struConfig.PN, my_struConfig.BOM, spec_flag, my_struConfig.Temper_Flag);
	if (num!=1) {MessagePopup("��ʾ", buf); return -1;} 
	
	return 0;
}

int DB_Read_Spec_Tracking_Ex1(void)
{
	char buf[1024]; 
	int	num;	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement 		   */
					   
	sprintf (buf, "SELECT LOS_KL, LOS_BL, LOS_KH, LOS_BH, A2Temper_K, A2Temper_B, A2Temper_Kl, A2Temper_Bl FROM AutoDT_Spec_Tracking_Ex1 WHERE PartNumber='%s' AND BOM='%s'", my_struConfig.PN, my_struConfig.BOM);
																														
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColFloat (hstmt, 1, &my_struDBConfig.LOS_Kl, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
	resCode = DBBindColFloat (hstmt, 2, &my_struDBConfig.LOS_Bl, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
	resCode = DBBindColFloat (hstmt, 3, &my_struDBConfig.LOS_Kh, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 4, &my_struDBConfig.LOS_Bh, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	resCode = DBBindColFloat (hstmt, 5, &my_struDBConfig.Temper_K, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}	
	resCode = DBBindColFloat (hstmt, 6, &my_struDBConfig.Temper_B, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}	
	resCode = DBBindColFloat (hstmt, 7, &my_struDBConfig.Temper_Kl, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	resCode = DBBindColFloat (hstmt, 8, &my_struDBConfig.Temper_Bl, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (num!=1)
	{MessagePopup ("��ʾ", "��ȡ���ݿ�AutoDT_Spec_Tracking_Ex1�еļ�¼ʧ�ܣ�"); return -1;}  	 
 
	return 0;    
}


int SetCaliConfigFile(void)
{
	int status;
	
	status = ArrayToFile (my_struTxCal.TxCal_Power_FileName, my_struTxCal.power_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_SEP_BY_TAB, 10, VAL_BINARY, VAL_TRUNCATE);
	if (status != 0) {MessagePopup("��ʾ", "д�뷢�˹⹦��У׼�ļ�����");  return -1;} 
	
	status = ArrayToFile (my_struTxCal.TxCal_Power_FileName_Min, my_struTxCal.minpower_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_SEP_BY_TAB, 10, VAL_BINARY, VAL_TRUNCATE);
	if (status != 0) {MessagePopup("��ʾ", "д�뷢�˹⹦��У׼�ļ�����");  return -1;} 

	status = ArrayToFile (sRxCal.RxCal_FileName, sRxCal.power_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_SEP_BY_TAB, 10, VAL_BINARY, VAL_TRUNCATE);
	if (status != 0) {MessagePopup("��ʾ", "д���ն�У׼�ļ�����");  return -1;} 
	
	status = ArrayToFile (my_struCal_OLT.OLT_sens_file, my_struCal_OLT.OLT_sens_arr, VAL_FLOAT, 5*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_SEP_BY_TAB, 10, VAL_ASCII, VAL_TRUNCATE);
	if (status != 0) {MessagePopup("��ʾ", "д��OLT������У׼�ļ�����");  return -1;} 

	status = ArrayToFile (my_struCal_OLT.FileName, my_struCal_OLT.Array, VAL_FLOAT, 5*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_SEP_BY_TAB, 10, VAL_BINARY, VAL_TRUNCATE);
	if (status != 0) {MessagePopup("��ʾ", "д��OLT�ն˹�·У׼�ļ�����");  return -1;} 

	return 0;
}

int GetCaliConfigFile(void)
{
	int status;
	int i;
	
	//��ȡ����У׼����
	status = FileToArray (my_struTxCal.TxCal_Power_FileName, my_struTxCal.power_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_BINARY);
	if (status != 0) {MessagePopup("��ʾ", "�򿪷��˹⹦��У׼�ļ�����"); return -1;} 
	for (i=0;i<CHANNEL_MAX;i++)
	{
		my_struTxCal.power_st[i]  = my_struTxCal.power_array[i*4+0];
		my_struTxCal.power_min[i] = my_struTxCal.power_array[i*4+1]; 
		my_struTxCal.power_max[i] = my_struTxCal.power_array[i*4+2]; 
		my_struTxCal.power[i] 	  = my_struTxCal.power_array[i*4+3];
	}
	
	//��ȡ����У׼����_С��
	status = FileToArray (my_struTxCal.TxCal_Power_FileName_Min, my_struTxCal.minpower_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_BINARY);
	if (status != 0) {MessagePopup("��ʾ", "�򿪷��˹⹦��У׼�ļ�����"); return -1;} 
	for (i=0;i<CHANNEL_MAX;i++)
	{
		my_struTxCal.minpower_st[i]  = my_struTxCal.minpower_array[i*4+0];
		my_struTxCal.minpower_min[i] = my_struTxCal.minpower_array[i*4+1]; 
		my_struTxCal.minpower_max[i] = my_struTxCal.minpower_array[i*4+2]; 
		my_struTxCal.minpower[i] 	  = my_struTxCal.minpower_array[i*4+3];
	}
	
	//��ȡ�ն�У׼����
	status = FileToArray (sRxCal.RxCal_FileName, sRxCal.power_array, VAL_FLOAT, 4*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_BINARY);
	if (status != 0) {MessagePopup("��ʾ", "���ն˲ο���·У׼�ļ�����"); return -1;} 
	for (i=0;i<CHANNEL_MAX;i++)  
	{
		sRxCal.power_in[i]  = sRxCal.power_array[i*4+0];
		sRxCal.power_min[i] = sRxCal.power_array[i*4+1]; 
		sRxCal.power_max[i] = sRxCal.power_array[i*4+2]; 
		sRxCal.power[i] 	= sRxCal.power_array[i*4+3];
	}
	
	status = FileToArray (my_struCal_OLT.OLT_sens_file, my_struCal_OLT.OLT_sens_arr, VAL_FLOAT, 5*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_ASCII);
	if (status != 0) {MessagePopup("��ʾ", "��OLT������У׼�ļ�����"); return -1;}  
	for (i=0;i<CHANNEL_MAX;i++) 
	{
		my_struCal_OLT.OLT_sens[i]  	= my_struCal_OLT.OLT_sens_arr[i*5+0];
		my_struCal_OLT.OLT_Reserves[i] 	= my_struCal_OLT.OLT_sens_arr[i*5+1]; 
		my_struCal_OLT.OLT_sens_min[i] 	= my_struCal_OLT.OLT_sens_arr[i*5+2]; 
		my_struCal_OLT.OLT_sens_max[i] 	= my_struCal_OLT.OLT_sens_arr[i*5+3];
		my_struCal_OLT.OLT_sens_in[i] 	= my_struCal_OLT.OLT_sens_arr[i*5+4]; 
	}
	
	status = FileToArray (my_struCal_OLT.FileName, my_struCal_OLT.Array, VAL_FLOAT, 5*CHANNEL_MAX, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_BINARY);
	if (status != 0) {MessagePopup("��ʾ", "��OLT�ն˹�·У׼�ļ�����"); return -1;}
	for (i=0;i<CHANNEL_MAX;i++) 
	{
		my_struCal_OLT.Power_In[i]  = my_struCal_OLT.Array[i*5+0];
		my_struCal_OLT.Power_Min[i] = my_struCal_OLT.Array[i*5+1]; 
		my_struCal_OLT.Power_Max[i] = my_struCal_OLT.Array[i*5+2]; 
		my_struCal_OLT.Power[i] 	= my_struCal_OLT.Array[i*5+3];
		my_struCal_OLT.time[i]      = my_struCal_OLT.Array[i*5+4];
	}
	
	return 0;
}

void Update_Config_Ver(int panel, BOOL updateVer)
{
	char tempStr[50];
	int  status, err, index;
		
	GetCtrlIndex (panel, PAN_CONF_RING_PN, &my_struConfig.PNIndex);
	GetLabelFromIndex (panel, PAN_CONF_RING_PN, my_struConfig.PNIndex, my_struConfig.PN);
	
	if (updateVer)
	{
		DB_Get_BOM (panel, PAN_CONF_RING_BOM); 
	}
	
	GetCtrlIndex (panel, PAN_CONF_RING_BOM, &index);  
	if(index>=0)
	{
		GetLabelFromIndex (panel, PAN_CONF_RING_BOM, index, my_struConfig.BOM);
	}
	else
	{
		strcpy (my_struConfig.BOM, "");
	}
 
	GetCtrlVal (panel, PAN_CONF_RAD_TUNING, &my_struConfig.DT_Tuning);
	GetCtrlVal (panel, PAN_CONF_RAD_TUN_RX, &my_struConfig.DT_Tun_RX); 
	GetCtrlVal (panel, PAN_CONF_RAD_TEST_RX, &my_struConfig.DT_Test_RX); 
	GetCtrlVal (panel, PAN_CONF_RAD_TEST, &my_struConfig.DT_Test); 
	GetCtrlVal (panel, PAN_CONF_RAD_QA, &my_struConfig.DT_QATest); 
	GetCtrlVal (panel, PAN_CONF_RAD_OQA, &my_struConfig.DT_OQA);
	GetCtrlVal (panel, PAN_CONF_RAD_LOW, &my_struConfig.Temper_Low); 
	GetCtrlVal (panel, PAN_CONF_RAD_ROOM, &my_struConfig.Temper_Room); 
	GetCtrlVal (panel, PAN_CONF_RAD_HIGH, &my_struConfig.Temper_High);
	
	
	GetCtrlVal (panel, PAN_CONF_RAD_Agin_Front, &my_struConfig.DT_Agin_Front);
	GetCtrlVal (panel, PAN_CONF_RAD_Agin_Back, &my_struConfig.DT_Agin_Back);
	
	GetCtrlVal (panel, PAN_CONF_RAD_TUNING_Reliabilit, &my_struConfig.DT_Tun_Reliability); 
	GetCtrlVal (panel, PAN_CONF_RAD_TEST_Reliability , &my_struConfig.DT_Test_Reliability); 
	
	GetCtrlVal (panel, PAN_CONF_RAD_Upstream_Test , &my_struConfig.DT_Test_Upstream); 
	GetCtrlVal (panel, PAN_CONF_RADIOBUTTON_OSATEST, &my_struConfig.DT_OSA_WL_Test);   
	//get version info according PN, temper_sel, ATE_sel
	/***����ȫ����Ϊ��д**Eric.Yao***/
	if     (my_struConfig.DT_Tuning) {strcpy (my_struConfig.DT_Flag, "TUNING");}	 
	else if(my_struConfig.DT_Tun_RX) {strcpy (my_struConfig.DT_Flag, "TUN_RX");} 
	else if(my_struConfig.DT_Test)   {strcpy (my_struConfig.DT_Flag, "TEST");} 
	else if(my_struConfig.DT_Test_RX){strcpy (my_struConfig.DT_Flag, "TEST_RX");} 		
	else if(my_struConfig.DT_QATest) {strcpy (my_struConfig.DT_Flag, "QATEST");} 
	else if(my_struConfig.DT_OQA) 	 {strcpy (my_struConfig.DT_Flag, "OQA");}  
	else if(my_struConfig.DT_Agin_Front)	{strcpy (my_struConfig.DT_Flag, "AGIN_FRONT");}
	else if(my_struConfig.DT_Agin_Back)		{strcpy (my_struConfig.DT_Flag, "AGIN_BACK");}  
	else if(my_struConfig.DT_Tun_Reliability)	{strcpy (my_struConfig.DT_Flag, "TUN_Reliability");}
	else if(my_struConfig.DT_Test_Reliability)	{strcpy (my_struConfig.DT_Flag, "TEST_Reliability");} 
	else if(my_struConfig.DT_Test_Upstream)	{strcpy (my_struConfig.DT_Flag, "TEST_Upstream");}   
	else if(my_struConfig.DT_OSA_WL_Test)	{strcpy (my_struConfig.DT_Flag, "OSA_WL_TEST");}  
	else							 {strcpy (my_struConfig.DT_Flag, "");}
	
	if     (my_struConfig.Temper_Low) {strcpy (my_struConfig.Temper_Flag, "LOW");}
	else if(my_struConfig.Temper_Room){strcpy (my_struConfig.Temper_Flag, "ROOM");} 
	else if(my_struConfig.Temper_High){strcpy (my_struConfig.Temper_Flag, "HIGH");} 
	else							  {strcpy (my_struConfig.Temper_Flag, "");}
	
	//OSA��������
	if(my_struConfig.DT_OSA_WL_Test)
	{
		SetCtrlAttribute (panel, PAN_CONF_BUT_OK, ATTR_DIMMED, 0);	           
		return ;
	}
	
	//��ȡ��Ʒ����
	err = DB_Get_PN_TYPE ();
	if (err) return ;
			
	//��ȡEED�汾
	err = DB_Get_EED_Ver (panel, PAN_CONF_RING_EED);
	if (err) return;
	
	//��ѯ������Ŀ����
	err = DB_Read_Spec_ATE_Item ();
	if(err) 
	{   //���ý����ȷ�ϰ�ť�������ȡ�����쳣�󣬳�����ʹ��
		SetCtrlAttribute (panel, PAN_CONF_BUT_OK, ATTR_DIMMED, 1); 
		return ;  
	}
	else
	{
		SetCtrlAttribute (panel, PAN_CONF_BUT_OK, ATTR_DIMMED, 0);	
	}
	
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_AOP, my_struConfig.Sel_T_AOP); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_ER, my_struConfig.Sel_T_ER); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_DIS, my_struConfig.Sel_T_TxDis); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_EYE, my_struConfig.Sel_T_Eye); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_MASK, my_struConfig.Sel_T_Mask); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_T_SPECTRUM, my_struConfig.Sel_T_Spectrum);
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_R_SENS, my_struConfig.Sel_R_Sens); 
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_R_OVER, my_struConfig.Sel_R_Over);
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_R_SDHYS, my_struConfig.Sel_R_SDHys);
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_CALI, my_struConfig.Sel_Calibrate);
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_CALI_TEST, my_struConfig.Sel_Calibrate_Test);
	SetCtrlVal (panel, PAN_CONF_CHE_BOX_EE_PROTECT, my_struConfig.Sel_EE_P); 
	SetCtrlVal (panel, PAN_CONF_CHE_TXSD, my_struConfig.Sel_TxSD);
	SetCtrlVal (panel, PAN_CONF_CHE_SENS_DETail, my_struConfig.Sel_R_Sens_Detail);
	SetCtrlVal (panel, PAN_CONF_CHE_SD_DETail, my_struConfig.Sel_RxSD_Detail);
	SetCtrlVal (panel, PAN_CONF_CHE_Wavelength,my_struConfig.Sel_T_Wavelength);
	
	ProcessDrawEvents();
}

int DB_save_txcali(int channel)
{
	char buf[2048];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	char stationid[256];
	
	sprintf (stationid, "%s-%d",my_struProcessLOG.StationID, channel);
	
	if (SERVER_ORACLE == my_struConfig.servertype)  	 
	{ 
		sprintf (buf, "INSERT INTO calibration (id, pcname,loss,lossmin,lossmax,flag) VALUES (s_id.nextval, '%s',%f,%f,%f,'%s')", stationid,my_struTxCal.power[channel],my_struTxCal.power_min[channel],my_struTxCal.power_max[channel],CALI_TYPE[CALI_TYPE_TX]); 
	}
	else
	{
		sprintf (buf, "INSERT INTO calibration (pcname,loss,lossmin,lossmax,flag) VALUES ('%s',%f,%f,%f,'%s')", stationid,my_struTxCal.power[channel],my_struTxCal.power_min[channel],my_struTxCal.power_max[channel],CALI_TYPE[CALI_TYPE_TX]); 
	}
	resCode = DBImmediateSQL (hdbc, buf); 
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	
	return 0;
}

int SET_EVB_SHDN(int channel, unsigned char SHUTValue)
{
	char buf[128];
	
	if (EVB5_SET_SHDN_VTR(INST_EVB[channel], SHUTValue)<0) 
	{
		sprintf(buf,"����ͨ��%d EVB��Դ�쳣    " ,channel);
		MessagePopup("��ʾ", buf);
		return -1;
	}	
	
	return 0;
}

static void GetLock (int tmpthreadLock, int *obtainedLock)
{
	double timeout = 300;  //timeout 300s, 5 min
    double startTime = Timer();
	
    do
    {
        // NOTE - If needed, you can process messages, etc, here.
        CmtTryToGetLock(tmpthreadLock, obtainedLock);
    } while (!*obtainedLock && Timer() - startTime < timeout);
}

int Get_Power(int channel, double *Power)
{
	int error ,obtainedLock = 0;  
	
	switch (INSTR[channel].PM_TYPE)
	{
		case PowMeter_TYPE_PMSII:
			if (PMSII_Get_Power(INST_PM[channel], Power)<0) return -1; 
			break;
		case PowMeter_TYPE_NONE:
			;
			break; 
		case PowerMeter_TYPE_PSS_OPM:									
		   	GetLock(Thread_PSS_PowerPeter,&obtainedLock);
		   	error=PSS_Get_Power(INST_PM[channel],channel+1,Power) ;
		   	CmtReleaseLock(Thread_PSS_PowerPeter);
		   	if(error<0)return -1;
		  	break;	  
		default:
			return -1;
	}
	return 0;
}

int DB_get_txchecklimit(int channel)
{
	char buf[1024];
	int  cnt;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
		
	sprintf (buf, "select val_min,val_max from calibration_spec where sn='%s'", sTxCheck.sn_array[channel]);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColFloat (hstmt, 1, &sTxCheck.power_min[channel], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColFloat (hstmt, 2, &sTxCheck.power_max[channel], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	cnt=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
		cnt++;
    }      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (cnt!=1)
	{
		MessagePopup("��ʾ", "���ݿ���û�д�ŵ�ǰ��׼ģ����ؼ�¼    ");
		return -1;
	}
	
	return 0;
}

int DB_save_txcheckup(int channel, char sn[], double val)
{
	char buf[2048];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	char stationid[256];
	
	sprintf (stationid, "%s-%d",my_struProcessLOG.StationID, channel);
	
	if (SERVER_ORACLE == my_struConfig.servertype)      
	{
		sprintf (buf, "INSERT INTO calibration (id, pcname,modulesn,sysval,flag,lossmin,lossmax) VALUES (s_id.nextval, '%s','%s',%f,'%s',%f,%f)", stationid,sn,val,CALI_TYPE[CALI_TYPE_TX_CHECKUP],sTxCheck.Power_Min,sTxCheck.Power_Max); 
	}
	else
	{
		sprintf (buf, "INSERT INTO calibration (pcname,modulesn,sysval,flag,lossmin,lossmax) VALUES ('%s','%s',%f,'%s',%f,%f)", stationid,sn,val,CALI_TYPE[CALI_TYPE_TX_CHECKUP],sTxCheck.Power_Min,sTxCheck.Power_Max); 
	}
		
	resCode = DBImmediateSQL (hdbc, buf); 
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	
	return 0;
}

int	Init_BERT (int channel)
{
	int error;
	
//	char B2BERT_PATTFILE[MAX_PATHNAME_LEN];
//	unsigned char B2BERT_PATT[B2_USERPATT_MAX_BYTENUMBER]; 
	
	if (INSTR[channel].BERT_TYPE_ONU == BERT_TYPE_SBERT) 
	{
		//ע�ⷵ��ֵ��int�ͣ�0��ʾOK
		error = EVB5_Init(&INST_BERT[channel], INSTR[channel].BERT_ONU); 
		if (error) 
		{
			MessagePopup("Error", "SBERT��ʼ��ʧ�ܣ�"); 
			goto Error;
		} 
			
		if (strcmp(INSTR[channel].BERT_ONU, INSTR[channel].SEVB)!=0) //SBERT��EVB5ʹ����ͬ�Ĳ��԰壬�����ٴγ�ʼ��
		{
			error = EVB5_SBERT_Init(INST_BERT[channel], INSTR[channel].GBert_Rate, INSTR[channel].GBert_PRBS);
			if (error) 
			{
				MessagePopup("��ʾ","SBERT����ʧ�ܣ�"); 
				goto Error;
			} 

			//����BEN_mode
			if (EVB5_SET_BEN_Mode (INST_BERT[channel], EVB5_BEN_MODE_LEVEL)<0) 
			{
				MessagePopup ("Error", "EVB5 Set level mode error !"); 
				goto Error;
			}   

			//����BEN_level, �ߵ�ƽ����
			if (EVB5_SET_BEN_Level(INST_BERT[channel], 1)<0)
			{
				MessagePopup ("Error", "EVB5 Set low level mode error !"); 
				goto Error; 
			} 

			//Power on
			errChk(EVB5_SET_SHDN_VTR(INST_BERT[channel], 1));
		}
	}
	else if (INSTR[channel].BERT_TYPE_ONU == BERT_TYPE_GBERT) 
	{
		if (!GBERT_Init(&INST_BERT[channel], INSTR[channel].BERT_ONU, INSTR[channel].GBert_PRBS, INSTR[channel].GBert_Rate, 0)) 
		{
			MessagePopup("Error", "Initial GBERT error!");
			goto Error;
		} 
	}
	else if (INSTR[channel].BERT_TYPE_ONU == BERT_TYPE_NONE) 
	{
		;
	}
	else 
	{
		MessagePopup("Error", "Can not find this BERT!");
		goto Error;
	} 
	
	return 0;
Error:
	return -1;	
}

int Set_Att(int channel, double val)
{
	switch (INSTR[channel].ATT_TYPE_ONU)
		{
		case ATT_TYPE_NONE:
			//û��˥����
			break;
		case ATT_TYPE_GVPM:
			val = 0-val; 
			if (val<0)  {MessagePopup ("ERROR", "GVPM Set Value error!"); return -1;} 
			if (val>40) {if(GVPM_SET_OutputEnable(INST_ATT[channel], 0)<0)   return -1;}	//����GVPM���ù⹦�ʲ�����ȫ�ضϣ�����������ǿ��ԭ�򣩵�Ӱ��     
			else  		{if(GVPM_SET_Attenuate (INST_ATT[channel], val, 1310)<0) return -1;}  	
			Delay(0.3);
			break; 			
		default:
			return -1;
		}
	
	return 0;
}

int Set_Att_SD(int channel, double AttValue)
{
	switch (INSTR[channel].ATT_TYPE_ONU)
		{
		case ATT_TYPE_NONE:
			//û��˥����
			break;
		case ATT_TYPE_GVPM:
			AttValue = 0-AttValue; 
		//	if (AttValue<0)  {MessagePopup ("ERROR", "GOAM Set Value error!"); return -1;} 
		//	if (AttValue>40) {if(GVPM_SET_OutputEnable(INST_ATT[channel], 0)<0) return -1;}		//����GVPM���ù⹦�ʲ�����ȫ�ضϣ�����������ǿ��ԭ�򣩵�Ӱ��     
		//	else  			 {if(GVPM_SET_LockAttenuate (INST_ATT[channel], 0-AttValue, my_struDBConfig.RxWavelength, 1)<0) return -1;}  
				  			 {if(GVPM_SET_LockAttenuate (INST_ATT[channel], 0-AttValue, my_struDBConfig.RxWavelength, 1)<0) return -1;}    
		//	Delay(0.3); 
			break; 				
		default:
			{MessagePopup ("ERROR", "ATT Type error!"); return -1;}     
			break;			
		}
	return 0;
}

int DB_Get_Spec_CSA8000(void)
{   
	char buf[256];
	int	 num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	Fmt (buf, "SELECT X_Scale,X_Position,Y_Scale,Y_Position,Y_Offset,Filter,Wavelength,MaskMargin,MaskName,Channel_O,ER_Slope,ER_Offset from AutoDT_Spec_CSA8000 where PN='%s' AND MainSN='%s'", my_struConfig.PN, my_struCSA8000.MainSN); 
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColFloat (hstmt, 1, &my_struCSA8000.X_Scale, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColFloat (hstmt, 2, &my_struCSA8000.X_Position, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 3, &my_struCSA8000.Y_Scale, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 4, &my_struCSA8000.Y_Position, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 5, &my_struCSA8000.Y_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColChar (hstmt, 6, 50, my_struCSA8000.Filter, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColInt (hstmt, 7, &my_struCSA8000.Wavelength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColInt (hstmt, 8, &my_struCSA8000.MaskMargin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColChar (hstmt, 9, 50, my_struCSA8000.MaskName, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColInt (hstmt, 10, &my_struCSA8000.Channel_O, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColFloat (hstmt, 11, &my_struTxCal.TxCal_Er_Slope, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 12, &my_struTxCal.TxCal_Er_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num!=1) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_CSA8000�в����ҵ����ݻ������ݼ�¼����һ����"); return -1;} 
	
	return 0;
}

int DB_Get_Spec_Ag86100(void)
{   
	char buf[256];
	int	 num;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	Fmt (buf, "SELECT X_Scale,Y_Scale,Y_Offset,ErFactor,Filter,Wavelength,MaskMargin,MaskName,ER_Slope,ER_Offset,X_Position,Channel_O from AutoDT_Spec_Ag86100 where PN='%s' AND MainSN='%s'", my_struConfig.PN, my_struAg86100.MainSN); 
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColFloat (hstmt, 1, &my_struAg86100.X_Scale, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColFloat (hstmt, 2, &my_struAg86100.Y_Scale, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 3, &my_struAg86100.Y_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
	resCode = DBBindColFloat (hstmt, 4, &my_struAg86100.ERFactor, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColInt (hstmt, 5, &my_struAg86100.Filter, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColInt (hstmt, 6, &my_struAg86100.Wavelength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColInt (hstmt, 7, &my_struAg86100.MaskMargin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColChar (hstmt, 8, 50, my_struAg86100.MaskName, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColFloat (hstmt, 9, &my_struTxCal.TxCal_Er_Slope, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 10, &my_struTxCal.TxCal_Er_Offset, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColFloat (hstmt, 11, &my_struAg86100.X_Position, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
    resCode = DBBindColInt (hstmt, 12, &my_struAg86100.Channel_O, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0; 
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num!=1) {MessagePopup("��ʾ","���ݿ�AutoDT_Spec_Ag86100�в����ҵ����ݻ������ݼ�¼����һ����"); return -1;} 
	
	return 0;
}

int SaveDataBase(struct struTestData data, struct struProcessLOG prolog)
{
	int error=0;
	
	errChk(DB_Save_Results_ATE (data, prolog)); 
	
	//����APD��������
//	if (data.Vapd_D_Flag)
//	{
//		errChk(DB_Update_Optics_Data(data));
//	}
	
	//ֻ�е�У׼��>0ʱ���ż�¼У׼���Խ����ÿ�ο�ʼ����ʱУ׼�㽫���ָ���0��
	//������ն�У׼���Կ�ʼǰ����У׼�㽫������ֵ�����ݿ��оͲ�����У׼��¼
	if (data.RxPow_num>0)
	{
		errChk(DB_Save_Results_Monitor (data));
	}

Error:
	return error;
}

int DB_Save_Results_ATE(struct struTestData data, struct struProcessLOG prolog)
{
	char buf[5000],temp_buf[100];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	char  filepath[1000] = "";
	char  filename[1000] = ""; 
	int   filehandle = 0;  

	if (SERVER_ORACLE == my_struConfig.servertype)
	{
		sprintf (buf, "begin sp_add_results_ate_ex04 ('%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',%f,%f,%d,'%s',%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d ,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,'%s','%s','%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s','%s','%s','%s',%d ); end;", 
			       data.PN,
				   data.Ver, 
				   data.TestDate, 
				   data.ModuleSN, 
				   data.OpticsSN,
				   data.Temperatrue,
				   data.AmbientTemperatrue,
				   data.ErrorCode,
				   data.Status, 
				   
				   data.AOP[0],
				   data.AOP[1],
				   data.AOP[2],
				   data.AOP[3], 
				   data.ER[0], 
				   data.ER[1], 
				   data.ER[2],
				   data.ER[3],
				   data.TxTj, 
				   data.TxPPj, 
				   
				   data.TxRise,
				   data.TxFall,
				   data.TxPCTCROss,
				   data.TxOffPower[0],
				   data.TxOffPower[1],
				   data.TxOffPower[2],
				   data.TxOffPower[3],
				   data.TE, 
				   data.ErCoverFlag,
				   data.ucRb[0],
				   
				   data.ucRb[1],
				   data.ucRb[2],
				   data.ucRb[3],
				   data.ucRm[0],
				   data.ucRm[1],
				   data.ucRm[2],
				   data.ucRm[3], 
				   data.RxAmp,
				   data.RxTj,
				   0.,
				   
				   data.Vapd_DAC,
				   data.Vapd_D_Temper,
				   data.Over,
				   data.OverTime,
				   data.Sens,
				   data.SensTime,
				   data.SDA,
				   data.SDD,
				   data.SDHys,	   
				   data.ucRsd,  
				   
				   data.pathpenalty,
				   data.pathpenaltytime,
				   data.PeakWL_DutyRatio01[0],
				   data.PeakWL_DutyRatio99[0],
				   data.PeakWL_DutyRatio01[1],
				   data.PeakWL_DutyRatio99[1],
				   data.PeakWL_DutyRatio01[2],
				   data.PeakWL_DutyRatio99[2],
				   data.PeakWL_DutyRatio01[3],
				   data.PeakWL_DutyRatio99[3],
				   
				   data.Peakwavelength[0],
				   data.Peakwavelength[1],  
				   data.Peakwavelength[2],  
				   data.Peakwavelength[3],  
				   data.Sigma,
				   data.Bandwidth,
				   data.SMSR,
				   data.TxV,
				   data.TxI[0],
				   data.RxV,
				   
				   data.RxI[0],
				   data.Vcc,
				   data.A2_temperatrue[0],
				   data.A2_Vcc[0],
				   data.A2_Ibias[0],
				   data.A2_CaseTemperatrue,
				   data.TxOffDepthDAC[0],
				   data.TxOffDepthDAC[1],
				   data.TxOffDepthDAC[2],
				   data.TxOffDepthDAC[3],
				   
				   data.TecDAC[0],
				   data.TecDAC[1],
				   data.TecDAC[2],
				   data.TecDAC[3],
				   
				   data.ApcCoreTemper[0],
				   data.ApcCoreTemper[1],
				   data.ApcCoreTemper[2],
				   data.ApcCoreTemper[3],
				   data.ModCoreTemper[0],
				   data.ModCoreTemper[1],
				   data.ModCoreTemper[2],
				   data.ModCoreTemper[3],
				   data.TecCoreTemper[0],
				   data.TecCoreTemper[1],
				   data.TecCoreTemper[2],
				   data.TecCoreTemper[3],
				   data.TxOffDepthTemp[0],
				   data.TxOffDepthTemp[1],
				   data.TxOffDepthTemp[2],
				   data.TxOffDepthTemp[3],
				   
				   data.PeakWL_DutyRatio10[0],
				   data.PeakWL_DutyRatio10[1],
				   data.PeakWL_DutyRatio10[2],
				   data.PeakWL_DutyRatio10[3],
				   data.A2_TxPower[0], 
				   
				   data.A2_TxPower[1], 
				   data.A2_TxPower[2], 
				   data.A2_TxPower[3], 
				   data.TxI[1],
				   data.TxI[2],
				   data.TxI[3],
				   data.RxI[1],
				   data.RxI[2],
				   data.RxI[3],
				   data.A2_temperatrue[1],
				   data.A2_temperatrue[2], 
				   data.A2_temperatrue[3], 
				   data.A2_Vcc[1],
				   data.A2_Vcc[2],
				   data.A2_Vcc[3],
				   data.A2_Ibias[1],
				   data.A2_Ibias[2],
				   data.A2_Ibias[3],
				   
				   data.LDTemper[0],
				   data.LDTemper[1], 
				   data.LDTemper[2], 
				   data.LDTemper[3], 
				   data.TecCurrent[0],
				   data.TecCurrent[1], 
				   data.TecCurrent[2], 
				   data.TecCurrent[3],
				   

				   prolog.PN, 
				   prolog.SN, 
				   prolog.Operator, 
				   prolog.Log_Action, 
				   prolog.Action_Time, 
				   prolog.Parameter,
				   prolog.P_Value,
				   prolog.Comments, 
				   prolog.SoftVersion, 
				   prolog.StationID, 
				   prolog.RunTime); 
	}
	else
	{
		sprintf (buf, "EXEC sp_add_autodt_results_ate_ex04 '%s','%s','%s','%s','%s',%f,%f,%d,'%s',%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d", 
			       data.PN, data.Ver, data.TestDate, data.ModuleSN, data.OpticsSN,data.Temperatrue,data.AmbientTemperatrue,data.ErrorCode,data.Status, 
				   data.AOP, data.ER,data.TxTj, data.TxPPj, data.TxRise,data.TxFall,data.TxPCTCROss,data.TxOffPower,data.TE,data.ErCoverFlag,data.ucRb,data.ucRm,
				   data.RxAmp,data.RxTj,0.,0.,0.,data.Over,data.OverTime,data.Sens,data.SensTime,data.SDA,data.SDD,data.SDHys,data.ucRsd,0.0,0.0,data.PeakWL_DutyRatio01[0],data.PeakWL_DutyRatio99[0],data.PeakWL_DutyRatio01[1],data.PeakWL_DutyRatio99[1],data.PeakWL_DutyRatio01[2],data.PeakWL_DutyRatio99[2],data.PeakWL_DutyRatio01[3],data.PeakWL_DutyRatio99[3],data.Sigma,data.Bandwidth,data.SMSR,
				   data.TxV,data.TxI,data.RxV,data.RxI,prolog.PN, prolog.SN, prolog.Operator, prolog.Log_Action, prolog.Action_Time, 
				   prolog.Parameter, prolog.P_Value,prolog.Comments, prolog.SoftVersion, prolog.StationID, prolog.RunTime); 
	}
	
	//���ر���洢�������
	GetDir (filepath); 	
	strcat (filepath,"\\data\\");
	
	sprintf(temp_buf,"%s.txt",data.OpticsSN);   
	strcat (filepath,temp_buf); 
	filehandle = OpenFile (filepath, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII); 			
	WriteFile (filehandle, buf, strlen(buf));
	CloseFile (filehandle);	
	//���ر���洢������� 
	
	resCode = DBImmediateSQL (hdbc, buf); 
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}  
	

	
	
	return 0;
}

int Init_Test(int channel, int panel, struct struTestData *data)
{
	BYTE	device_addr, Rb_Addr, Rm_Addr, Rb, Rm, Fail; 	
	int		error, count, status;
	int     DAC0, DACmin, DACmax;
	char    FirmwareVer[5], buf[256];
	float   Ibias;
	INT16U  DAC_r; 
	union utestdata
	{
	struct struTestData sStr;
	unsigned char pStr[sizeof(struct struTestData)];
	} uTestData;

	//ͨ����������union��������ֵ0���ٴ������Խ���洢�ṹ��
	memset (uTestData.pStr, 0, sizeof (uTestData.pStr));
	*data = uTestData.sStr; 

	//������ʹ�����
	if (!my_struConfig.isNPI)
	{
		errChk(MyDLL_FiberManage_Run (hdbc, INSTR[channel].Fiber, &data->FiberTime));
	}

	//����EVB5����ģʽ�� 
	if (my_struConfig.Sel_TxSD) //��Ҫ����TxSD�Ĳ�Ʒ����Ҫ��TxSD_Detect���Խ�����������BURST_ON״̬
	{
		;
	}
	else
	{
		if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) return -1; 
	}

	//�ȴ��ϵ���ȶ�
	Delay(2.);
	
	/*********************Resultstable������ֵ**************************/
	strcpy (data->PN, my_struConfig.PN);     	//PN 
	strcpy (data->Ver, my_struConfig.BOM);  	//Version
	MyDLLGetSystemDate (data->TestDate);		//testdate 
	//ModuleSN�� �ڲ��Ժ�����ֵ
	//OpticsSN, �ڲ��Ժ�����ֵ
	//����ģ���¶ȣ���testteperature��ֵ
	//�����¶�
	if     (my_struConfig.Temper_Low) {data->AmbientTemperatrue=0;}
	else if(my_struConfig.Temper_Room){data->AmbientTemperatrue=25;} 
	else if(my_struConfig.Temper_High){data->AmbientTemperatrue=70;} 
	else							  {data->AmbientTemperatrue=999;}
	/*********************Resultstable������ֵ*********************/
	
	if (my_struConfig.Sel_T_AOP)
	{
		errChk(Set_Power_Wavelength(channel, 1310));  
		errChk(Set_Power_Unit(channel, PMSII_MODE_WATT_DBM));  
	}
	
	errChk(BERT_Stop (channel));
	
Error:
	return error;
}

Init_Test2(int channel, int panel, struct struTestData *data)
{
//	BYTE	device_addr, Rb_Addr, Rm_Addr, Rb, Rm; 
	INT8U   FirmwareVer;
	int		error, count,table;
//	INT16U  DAC;
	char    buf[256];
//	BOOL Status;
	double temp_AOP;
	int	temp_val,LookFlag;
	

	//ͨ����������union��������ֵ0���ٴ������Խ���洢�ṹ��
	memset (data, 0, sizeof (data));

	//������ʹ�����
#if 1	
	if (!my_struConfig.isNPI)
	{
		error = MyDLL_FiberManage_Run (hdbc, INSTR[channel].Fiber, &data->FiberTime);   
		if (error) 
		{
			return -1;
		}
	}
#endif	
	//����EVB5����ģʽ�� 
	if (my_struConfig.Sel_TxSD) //��Ҫ����TxSD�Ĳ�Ʒ����Ҫ��TxSD_Detect���Խ�����������BURST_ON״̬
	{
		;
	}
	else
	{
		if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) 
		{
			return -1; 
		}
		//����ռ�ձ�100%
		if (EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
		{
			return -1; 
		}
		if (!my_struConfig.DT_OSA_WL_Test) 
		{
			//���÷����ź�
			if(EVB5_SBERT_Init(INST_EVB[channel], SBERT_R_R1244Mbps, INSTR[channel].SBert_PRBS)<0)
			{
				MessagePopup("Error", "Sbert Init error!"); 
				return -1; 
			}
		}
																								  
	}

	//�ȴ�������ȶ�
//	Delay(1.);

	/*********************Resultstable������ֵ**************************/
	strcpy (data->PN, my_struConfig.PN);     	//PN 
	strcpy (data->Ver, my_struConfig.BOM);  	//Version
	MyDLLGetSystemDate (data->TestDate);		//testdate
	
	//ModuleSN���ڲ��Ժ�����ֵ
	//OpticsSN���ڲ��Ժ�����ֵ
	//����ģ���¶ȣ���testteperature��ֵ
	//�����¶�
	if     (my_struConfig.Temper_Low) {data->AmbientTemperatrue=0;}
	else if(my_struConfig.Temper_Room){data->AmbientTemperatrue=25;} 
	else if(my_struConfig.Temper_High){data->AmbientTemperatrue=70;} 
	else							  {data->AmbientTemperatrue=999;}
	/*********************Resultstable������ֵ*********************/
	
	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip || CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 
	{
		error = I2C_HOST_RESET_F320_DLL (INST_EVB[channel]-0x800, (256-24*1E3/3./40), 0); 
		if (error) 
		{
			MessagePopup ("Error", "EVB5s set i2c rate to 40k error"); 
			return -1;
		}  
	}
	//Set PowMeter
	if((PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
	{
		error = Set_Power_Wavelength(channel, my_struDBConfig.TxWavelength);  
		if (error) 
		{
			MessagePopup("Error", "Set Power wavelength error!"); 
			return -1;
		}
	}
	//set att
	if (INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE)
	{
		error = SET_ATT_ONU(channel, -10);
		if (error) 
		{
			MessagePopup("Error", "Set Att error!");
			return -1; 
		}
	}							   

	if(CHIEFCHIP_UX3328  == my_struDBConfig_ERCompens.ChiefChip) 
	{
		//���²����3328�Ĵ���  
		if (!my_struConfig.Temper_Low)
		{
			error = ux3328_set_FactoryMode (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}   
			
			//���EEPROM��־λ�Ƿ�����
			error = ux3328_check_eepflag (INST_EVB[channel], my_struConfig.DT_Tuning);		 
			if (error) 
			{
				if (my_struConfig.DT_Tuning)  
				{
					error = ux3328_write_backup_A2_Table (INST_EVB[channel]);
					if (error) 	
					{
						MessagePopup("Error", "д���ݵ�A2����ʧ��   ");
						return -1;
					}
				}
				else
				{
					MessagePopup("Error", "���A2[0xBF]��־λ����   ");
					data->ErrorCode=ERR_UX3328_E2_CHECK;	
					return -1;
				}
			}

			error = un3328_set_UserMode (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328�����û�ģʽ����   "); 
				return -1;
			}   
		}
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 
	{
		//���²����3328�Ĵ���  
		if (!my_struConfig.Temper_Low)
		{
			error = ux3328s_set_FactoryMode (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			
			//���EEPROM��־λ�Ƿ�����
			error = ux3328s_check_eepflag (INST_EVB[channel], my_struConfig.DT_Tuning);		 
			if (error) 
			{
				if (my_struConfig.DT_Tuning)  
				{
					error = ux3328s_write_backup_A2_Table (INST_EVB[channel]);
					if (error) 	
					{
						MessagePopup("Error", "д���ݵ�A2����ʧ��   ");
						return -1;
					}
				}
				else
				{
					MessagePopup("Error", "���A2[0xBF]��־λ����   ");
					data->ErrorCode=ERR_UX3328_E2_CHECK;	
					return -1;
				}
			}

			error = ux3328s_set_UserMode (INST_EVB[channel]);
			if (error)
			{
				MessagePopup("����", "3328�����û�ģʽ����   ");
				return -1;
			}   
		}
	}
	else if (CHIEFCHIP_DS4830  == my_struDBConfig_ERCompens.ChiefChip)  
	{
		if(my_struConfig.DT_Test_Upstream )
		{
			//���ò���ͨ��
			if(DWDM_ONU_Select_Channelindex(channel,3)<0) 
			{
				return -1;
			}
			;	//�����������
			
		}
		else if(my_struConfig.DT_Agin_Back || my_struConfig.DT_Agin_Front)  
		{
			//��ȡPID������־λ
			count=0;
			do
			{
				ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
				Delay(0.3);
				count++;
			}while(0==LookFlag && count <4);
			if(count>=4)
			{
				MessagePopup("����", "ģ��PID����ʧ��"); 
				return -1;
			}
			
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			Delay(0.2);
			error=DWDM_ONU_DS4830_GET_FirmwareVer(INST_EVB[channel],buf);
			if (error) 
			{
				MessagePopup("����", "��ȡDS4830�汾����   "); 
				return -1;
			} 
			if((!my_struConfig.DT_Tun_Reliability || !my_struConfig.DT_Test_Reliability) && !my_struConfig.DT_OSA_WL_Test)				  //�ɿ��Ե���ʱ�������ư汾
			{
				if(strcmp(buf,my_struDBConfig.FirmwareVer)!=0)
				{
					MessagePopup("����", "Firmware�汾������   "); 
					return -1;	
				}
			}
			
			//ȥ��Burnin״̬�����⹤��ȡ��Burn-in���ܣ����ճ����ϻ�ִ�У�2017-02-20  wenyao.xi;
			error = DWDM_ONU_Set_Burnin_OFF(channel);
			if (error<0) {MessagePopup ("ERROR", "ȥ��Burnin״̬ʧ��"); return -1;}
		}
		else
		{
			//��ȡPID������־λ
			count=0;
			do
			{
				ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
				Delay(0.3);
				count++;
			}while(0==LookFlag && count <4);
			if(count>=4)
			{
				MessagePopup("����", "ģ��PID����ʧ��"); 
				return -1;
			}
			
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			Delay(0.2);
			error=DWDM_ONU_DS4830_GET_FirmwareVer(INST_EVB[channel],buf);
			if (error) 
			{
				MessagePopup("����", "��ȡDS4830�汾����   "); 
				return -1;
			} 
			if((!my_struConfig.DT_Tun_Reliability || !my_struConfig.DT_Test_Reliability) && !my_struConfig.DT_OSA_WL_Test)				  //�ɿ��Ե���ʱ�������ư汾
			{
				if(strcmp(buf,my_struDBConfig.FirmwareVer)!=0)
				{
					MessagePopup("����", "Firmware�汾������   "); 
					return -1;	
				}
			}
			
			//ȥ��Burnin״̬�����⹤��ȡ��Burn-in���ܣ����ճ����ϻ�ִ�У�2017-02-20  wenyao.xi;
			error = DWDM_ONU_Set_Burnin_OFF(channel);
			if (error<0) {MessagePopup ("ERROR", "ȥ��Burnin״̬ʧ��"); return -1;}
		
			
			if (my_struConfig.DT_Tuning || my_struConfig.DT_Tun_Reliability)  
			{
/*				//Set LOS Luk
				if(DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], 0, 35+50, my_struDBConfig.LOS_Kl*512,my_struDBConfig.LOS_Bl))	  //LOW 
				{
					MessagePopup("����", "дLOS LUKʧ��   "); 
					return -1;	
				}
				if(DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], 6, 255  , my_struDBConfig.LOS_Kh*512,my_struDBConfig.LOS_Bh))	  //High 
				{
					MessagePopup("����", "дLOS LUKʧ��   "); 
					return -1;	
				}
				if(DWDM_ONU_DS4830_Set_LOS_Auto(INST_EVB[channel]))
				{
					MessagePopup("����", "ʹ��LOS LUK Aģʽʧ��   "); 
					return -1;	
				}
*///������Թ���			
				//Set LOS ΪSDģʽ
			/*	error = ux3328s_select_table (INST_EVB[channel], 3);
				if (error) return -1;
				if(ux3328s_set_los_RX_CTRL2(INST_EVB[channel], 0x80))
				{
					MessagePopup("����", "����SDģʽʧ��   "); 
					return -1;
				}   */  //�뾲�빦����ͬ
				
				//ʹ��LOS���빦�ܣ�
				error = DWDM_ONU_Set_LOS_Squelch(INST_EVB[channel]);
				if (error<0) {MessagePopup ("ERROR", "ʹ��LOS���빦��ʧ��"); return -1;}
				

/*				
				//Cal Temprature  ������ ģ��ϵ��			   
				my_struDBConfig.Temper_K =0.929  ; 
				my_struDBConfig.Temper_B =-57 ; 
				//Cal Temprature
				if(DWDM_ONU_DS4830_SET_Temp_Cal(INST_EVB[channel], my_struDBConfig.Temper_K*256,my_struDBConfig.Temper_B))	   		//д��ʼУ׼ֵ
				{
					MessagePopup("����", "дTemp_Calʧ��   "); 
					return -1;	
				}
			
				//Cal Vcc
				if(DWDM_ONU_DS4830_SET_Vcc_Cal(INST_EVB[channel]))
				{
					MessagePopup("����", "Vcc У׼ʧ��   "); 
					return -1;	
				}
				//Cal Bias
				if(ux3328s_get_table(INST_EVB[channel],&table))
				{
					MessagePopup("����", "��ȡTable ʧ��   "); 
					return -1;
				}
				if(ux3328s_select_table(INST_EVB[channel],3))
				{
					MessagePopup("����", "����Table 3ʧ��   "); 
					return -1;
				}
				if(ux3328s_set_calibration_bias(INST_EVB[channel],0.5625,0x3000))
				{
					MessagePopup("����", "Bias У׼ʧ��   "); 
					return -1;	
				}
				if(ux3328s_select_table(INST_EVB[channel],table))
				{
					MessagePopup("����", "����Table 3ʧ��   "); 
					return -1;
				}
				
				//Set Laser temperature model
				if(DWDM_ONU_DS4830_SET_LaserTemp_Model(INST_EVB[channel],1))
				{
					MessagePopup("����", "����Laser temperature modelʧ��   "); 
					return -1;
				}
				//Clear Fine Tuning  val
				if(DWDM_ONU_DS4830_Clear_FineTuning(INST_EVB[channel]))
				{
					MessagePopup("����", "����Clear Fine Tuning Valʧ��   "); 
					return -1;
				}
				
				if(DWDM_ONU_DS4830_Set_Wavelength_Range(INST_EVB[channel],my_struDBConfig.WavelengthChannel[3],my_struDBConfig.WavelengthChannel[0]))
				{
					MessagePopup("����", "����Set Fine Wavelength Rangeʧ��   "); 
					return -1;
				}
				//���ò���ͨ��
				if(DWDM_ONU_Select_Channelindex(channel,0))
				{
					MessagePopup("����", "����Set Fine Wavelength Rangeʧ��   "); 
					return -1;
				}
				
				if(DWDM_ONU_DS4830_Set_TecTempe(INST_EVB[channel]))
				{
					MessagePopup("����", "����Set Fine Wavelength Rangeʧ��   "); 
					return -1;
				}
				//����
				if(DWDM_ONU_DS4830_Update_LUK10(INST_EVB[channel]))   
				{
					MessagePopup("����", "����LUK10 ʧ��   "); 
					return -1;	
				}
			
				if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel]))   
				{
					MessagePopup("����", "����BASE0 ʧ��   "); 
					return -1;	
				}
*///������Թ���
			}
			else
			{
				// Get Table3 0x82		 ��ȡBias����������Χ����¼
				error = ux3328s_select_table (INST_EVB[channel], 3);
				if (error) 
				{
					return -1;
				}
				if(ux3328s_Get_TX_CTRL2(INST_EVB[channel], &temp_val))
				{
					MessagePopup("����", "��Table 3 0X82 ʧ��   "); 
					return -1;
				}
				data->pathpenaltytime =(double)temp_val;
				
				if(my_struConfig.Temper_High || my_struConfig.Temper_Low)
				{
					error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
					if(error)
					{
						MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
					  	return -1;
					} 
					if(DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100)<0)
					{
						return -1; 
					}
				}
			}
		}
	}
	else 
	{
		MessagePopup ("��ʾ", "���ݿ�AutoDT_Spec_Tracking�ж����PN_TYPE�д���"); 
		return -1;
	}
	
/*	error = EVB5_GET_VTR (INST_EVB[channel], &data->Vcc);
	if (error) 	
	{
		MessagePopup("Error", "��ȡVccʧ��   ");
		return -1;
	}
*/	
	return 0; 
}


int Set_Power_Wavelength(int channel, int Wavelength)
{
	switch (INSTR[channel].PM_TYPE)  
		{
		case PowMeter_TYPE_PMSII:
			if (Wavelength==1490) 
			{
				Wavelength=1480;
			}
			if (PMSII_Set_Wavlength(INST_PM[channel], Wavelength)<0) 
			{
				return -1; 
			}
			break;
		case PowerMeter_TYPE_PSS_OPM:
		case PowMeter_TYPE_NONE:
			break; 
		default:
			return -1;
		}
	return 0;
}

int Set_Power_Unit(int channel, int Unit)
{
	switch (INSTR[channel].PM_TYPE)  
		{
		case PowMeter_TYPE_PMSII:
			if (PMSII_Set_Mode (INST_PM[channel], Unit)<0) 
			{
				return -1; 
			}
			break;
		case PowerMeter_TYPE_PSS_OPM: 
		case PowMeter_TYPE_NONE:
			break; 
		default:
			return -1;
		}
	return 0;
}

int SET_EVB_TxDis(int channel, unsigned char TxDis)
{
	switch (INSTR[channel].SEVB_TYPE)
	{
		case SEVB_TYPE_SEVB5:
		case SEVB_TYPE_EVB27:
			if (EVB5_SET_OLT_TxDIS(INST_EVB[channel], TxDis)<0) return -1;
			break; 			
		default:

			MessagePopup("Error", "Can not find this EVB type");

			return -1;
	}
	return 0;	
}

int Read_Tx_Fail(int channel, BYTE *Fail)
{
	BYTE temp;

	switch (INSTR[channel].SEVB_TYPE) 
	{
		case SEVB_TYPE_SEVB5: 
		case SEVB_TYPE_EVB27:	
			if (EVB5_READ_OLT_TxFAULT(INST_EVB[channel], &temp)<0) 
			{
				return -1;     
			}
			break;			
		default:
		{
			MessagePopup("Error", "Can not find this EVB type");
			return -1;
		}
	}
	
	*Fail= temp>0 ? 1:0;
	
	return 0;
}

int BERT_Stop (int channel)
{
	int error;
	
	switch (INSTR[channel].BERT_TYPE_ONU) 
	{
		case BERT_TYPE_GBERT:
			
			if (!GBERT_Stop(INST_BERT[channel])) 
			{
				MessagePopup("Error", "GBERT Set Stop Error!"); 
				return -1;
			} 
			break; 
			
		case BERT_TYPE_SBERT:
			
			if (EVB5_SBERT_End(INST_BERT[channel])<0) 
			{
				MessagePopup("Error", "SBERT Set Stop Error!!"); 
				return -1;
			}
			break; 
			
		default:
			
			MessagePopup("Error", "can not find this BERT Type!"); 
			return -1;
	}
	return 0;
}

int GetBosaSN(int channel, struct struTestData *data)
{
int	 error;
char substr, str[25]="", myBareBOM[50]="", batch[50]="";

	errChk(MyDLLGetInnerSN (INST_EVB[channel], my_struConfig.PN_TYPE, data->OpticsSN));

	if (!my_struConfig.isQANoCheckBOM) 
	{
		error = DB_Search_Tracking_BareBOM(data->OpticsSN, myBareBOM, batch);
		if (error) 
		{
			return -1;
		}

		if (strcmp (myBareBOM, my_struConfig.BOM) != 0)
		{
			MessagePopup ("��ʾ", "��ƷBOM���ʧ��");
			return -1;
		} 
	}
	
	if (!my_struConfig.isNPI) 
	{
		if (0 != stricmp (batch, my_struConfig.batch))
		{
			MessagePopup ("��ʾ", "�������μ��ʧ��");
			return -1;
		}   
	}
	
Error:
	
	return error;
}

int checktestbed(int channel, int *val)
{
int	 error; 
int  mod0;

	*val = 0; 

	errChk(EVB5_Get_MOD_DEF0 (INST_EVB[channel], &mod0));
	
	*val = !mod0;

Error:
	
	return error;
}
int	CalTemperature (int channel)   
{
	double tempnow, d_offset;
	INT16S slope, offset;   

	int error;
	double TCal_Target=31.35;
	my_struDBConfig.Temper_K=0.968;
	
	error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
	if (error) 
	{
		MessagePopup("����", "3328���ù���ģʽ����   "); 
		return -1;
	}
	Delay(0.2);
	error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &tempnow);
	if (error)
	{
		return -1;        
	}
	
	offset = TCal_Target - my_struDBConfig.Temper_K * (tempnow+50); 

	//Cal Temprature
	if(DWDM_ONU_DS4830_SET_Temp_Cal(INST_EVB[channel], my_struDBConfig.Temper_K*256,offset))	   
	{
		MessagePopup("����", "дTemp_Calʧ��   "); 
		return -1;	
	}
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel]))   
	{
		MessagePopup("����", "����BASE0 ʧ��   "); 
		return -1;	
	}
	return 0;
}
int testTemperature(int channel, int panel, struct struTestData *data)
{
	int cnt=0;
	
	char Info[256];
	
	do
	{
		if (MyDLL_8472_GetTemperatrue(INST_EVB[channel], &data->Temperatrue)<0) 
		{
			return -1;
		}
		
//		data->Temperatrue=data->Temperatrue+5;
		
		sprintf (Info, "ģ���¶�=%.2f�� ", data->Temperatrue);     
		Update_Info(panel, gCtrl_BOX[channel], Info);
	
		//�жϷ�Χ
		if (data->Temperatrue<my_struDBConfig.TemperMax && data->Temperatrue>my_struDBConfig.TemperMin) 
		{
			break;
		}
	
		Delay(1);
	
		cnt++;
	
	} while (cnt<90);
	
	if (cnt>=90) 
	{
		return -1;
	}
	
	return 0;
}

int Read_AOP(int channel, double *AOP)
{
	double Power,tempPower;
	int index;
	tempPower=0;
	
	for(index=0;index<10;index++)
	{
		if (Get_Power(channel, &Power)<0) 
		{
			return -1;
		}   
		if((fabs(tempPower-Power)<0.2) && (index>0))
		{
		 	break;
		} 
		tempPower=Power;
		Delay(0.2);
	}	  	
	
	*AOP=Power+my_struTxCal.power[channel];
	
  	return 0;
}
int Read_AOP_Ex(int channel, double *AOP)			//�ض����ʱʹ��
{
	double Power;

	if (Get_Power(channel, &Power)<0) 
	{
		return -1;
	}
	
	*AOP=Power+my_struTxCal.minpower[channel];
//	*AOP=Power+my_struTxCal.power[channel]; 
  	return 0;
}

int tuningAOP_ER(int channel, struct struTestData *data)
{
/*	double	AOPavg;      
	double	ERavg,Pavg,temp_Ibias;
	BYTE 	LUT_APC[81], LUT_MOD[81], biasdac_m, biasdac_l; 
	INT8U	 UX3328_BiasCT_R;
	INT16U	 UX3328S_BiasCT_R;
	int		DB_DACmin, DB_DACmax;
	
	double   ptrAOP=0.0;
	double   ptrER=0.0; 
	double   temp_min=0.0; 
	double   temp_max=0.0;
	double   temp_AOP=0;
	double   temperature=0;
	double   P0=0; 
	double   I0=0;
	int      count=0;
	int      count2, conut2OK; //ER���Զ���ѭ��
	int      setDAC=0;
	int      DACMax=0xff; 
	int      DACMin=0; 
	int      DAC0=0;
	int      DAC1=0;
	int      error=0;
	
	double   AOP_s1=0.0;
	double   Bias_s1=0.0;
	int      R5_s1=0;
	int      R0_s1=0; 
	
	double   AOP_s2=0.0;
	double   Bias_s2=0.0;
	int      R5_s2=0;
	int      R0_s2=0; 
	double   SE2=0.0;   
	
	double   Bias_s4=0.0;
	int      R5_s4=0;
	int      R0_s4=0; 
	char     buf[256];
	
	const int MOD_DAC_MAX = 0xA0;
	
	int      index;
	
	BOOL	tun2, tun_add, tun_dec;

	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328_set_mode (INST_EVB[channel], 1));
	}
	
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328s_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328s_set_mode (INST_EVB[channel], 1));
	}
	
	//reset APC and MOD
	errChk(SET_DAC_APC(channel, 0)); 
	
	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ        
	{
		errChk(SET_DAC_MOD(channel, 0)); 
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ        
	{
		errChk(SET_DAC_MOD(channel, 0)); 
	}
	else
	{
		errChk(SET_DAC_MOD(channel, 0));      
	}
	
	error = EVB5_SBERT_Init(INST_EVB[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS);
	if (error){MessagePopup("��ʾ","EVB5����BERT����ģʽʧ�ܣ�"); goto Error;}

	//��Ҫ���У׼ϵ��
	AOPavg=(my_struDBConfig.TxPowMax+my_struDBConfig.TxPowMin)/2.0;
	AOPavg = AOPavg*my_struDBConfig_ERCompens.AOP_Slope+my_struDBConfig_ERCompens.AOP_Offset;
	//��Ҫ���У׼ϵ�� 
	ERavg=(my_struDBConfig.TxErMax+my_struDBConfig.TxErMin)/2.0;
	ERavg = ERavg*my_struDBConfig_ERCompens.ER_Slope+my_struDBConfig_ERCompens.ER_Offset; 
	
	ptrAOP=AOPavg;			 
	ptrER=ERavg;
	
	//step 1
	
	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip || CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 	 
	{
		//Ux3328�ڵ��⹦��֮ǰ�������ݿ����õľ���ֵ����MOD**Eric.Yao**20140604//
		sscanf (my_struDBConfig_ERCompens.Rm_Add, "(0x%02x)(0x%02x)(0x%02x)", &setDAC, &DB_DACmin, &DB_DACmax);          
		R0_s1=setDAC;       
	}
	else
	{
		R0_s1=0x00; 
	}
	
	errChk(SET_DAC_MOD(channel, R0_s1));	           		              
	temp_min=ptrAOP-0.2;	  //�⹦�ʵ��Է�Χ����Ϊ����ֵ��0.3**xuzhi**20141121//
	temp_max=ptrAOP+0.2;
	
	sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &setDAC, &DACMin, &DACMax); 	

//	errChk(SET_DAC_APC(setDAC));
//	data->ucRb=setDAC;
//	Delay(5);
//	Read_AOP(&temp_AOP);    
//	data->AOP=temp_AOP;
	

	count=0;
	tun2=FALSE;
	tun_dec=FALSE;
	tun_add=FALSE;	//���Է����ʼ��
	
	DAC0 = setDAC;
	do
	{
		errChk (SET_DAC_APC(channel, DAC0));

		//Ϊ�˱�֤��һ�ζ�ȡ�⹦��׼ȷ�����ӵȴ�ʱ��
		if (count==0) 
		{
			Delay(3);
		}
		else  
		{
			Delay(0.5); /***�⹦�ʵ��Թ����еȴ�ʱ����1.5s�޸�Ϊ0.5s����֤����Ч��**Eric.Yao**20140417//
		}
		
		Read_AOP(channel, &temp_AOP);
		data->AOP[0]=temp_AOP;
		
		if (temp_AOP>=temp_min&&temp_AOP<=temp_max) 
		{
			break;
		}

		DAC1 = pow(10,AOPavg/10)/pow(10,temp_AOP/10)*DAC0;         

		if (DAC1==DAC0)
		{
			if(temp_AOP<AOPavg) 
			{
				tun_add=TRUE;  
				if (tun_add && tun_dec)	
				{
					tun2=TRUE;
					break;
				}//�����Է�����ʱ�˳���ǰ����ѭ��
		 
				DAC1=DAC0+10;
			} 
			else					 
			{
				tun_dec=TRUE;
				if (tun_add && tun_dec)	
				{
					tun2=TRUE;
					break;
				}//�����Է�����ʱ�˳���ǰ����ѭ��	
				DAC1=DAC0-10;
			} 
		}

		//�������ܵ��Գ�����ʹ�ö��ַ�����
		if (DAC1>DACMax || DAC1<DACMin) 
		{
			tun2=TRUE;
			break;
		}  

		DAC0=DAC1;
		setDAC = DAC0;
		count++;

	}while (count<10);
	
	if (count>=10) 
	{
		tun2=TRUE;
	} 
	
	
	if (tun2)    
	{
		sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &setDAC, &DACMin, &DACMax); 	

		errChk(SET_DAC_APC(channel, setDAC));
		data->ucRb=setDAC;
	//	Delay(5);
		Delay(3); 
		Read_AOP(channel, &temp_AOP);    
		data->AOP[0]=temp_AOP;
			
  	 	while(count<10)		                
		{
			if (temp_AOP>=temp_min&&temp_AOP<=temp_max) 
			{
				break;
			}
			else if((DACMax-DACMin)<2) 
			{
				MessagePopup("Error", "����AOPʧ�ܣ�");
				data->ErrorCode=ERR_TUN_AOP;
				error = -1;
				goto Error;
			} 
			else
			{
				if(temp_AOP>ptrAOP) 
				{
					DACMax=setDAC;
					setDAC=(DACMax+DACMin)/2;
				}
				else					 
				{
					DACMin=setDAC;
					setDAC=(DACMax+DACMin)/2;
				}
			}
			
			errChk(SET_DAC_APC(channel, setDAC));
			data->ucRb=setDAC;
			Delay(1.5);
			Read_AOP(channel, &temp_AOP);    
			data->AOP=temp_AOP;
			count++;
		}
	  
		if (count >= 10) 
		{
			MessagePopup ("Error", "����AOP��ʱ!");
			error = -1;
			goto Error;
		} 
	}
	
	
	R5_s1=setDAC;
	R0_s1=0;
	errChk(SET_DAC_MOD(channel, R0_s1));	
//	Delay(3);
	Delay(0.5);     
			
	Read_AOP(channel, &AOP_s1);
	errChk(Read_Ibias(channel, &Bias_s1));
	
	//step 2
	R5_s2=R5_s1>0x80? R5_s1-30:R5_s1+30;
	
	errChk(SET_DAC_APC(channel, R5_s2)); 
	
	R0_s2=R0_s1;
//	Delay(3); 
	Delay(0.5);     
	
	Read_AOP(channel, &AOP_s2);
	errChk(Read_Ibias(channel, &Bias_s2)); 
	
	SE2=(pow(10,AOP_s2/10)-pow(10,AOP_s1/10))/(Bias_s2-Bias_s1); 
	
	data->RxV=SE2; 
	
	//step 3  
	P0=(2*(pow(10,AOP_s1/10)))/((pow(10,ptrER/10))+1);
	I0=((P0-pow(10,AOP_s1/10))/SE2)+Bias_s1;
	
	if(I0<3)
	{
		MessagePopup ("Error", "����I0<3mA!");
		error = -1;
		goto Error;		
	}
	
	//step 4  
	R5_s4=R5_s1;
	
	errChk(SET_DAC_APC(channel, R5_s4)); 
	
	Delay(0.5); 
	
	for (index=0; index<WAVELENGTHCHANNEL; index++)
	{
		conut2OK=0;
		count2=0;
		do
		{
			sscanf (my_struDBConfig_ERCompens.Rm_Add, "(0x%02x)(0x%02x)(0x%02x)", &setDAC, &DB_DACmin, &DB_DACmax);    
		
			DACMax=DB_DACmax;
			DACMin=DB_DACmin;
	
			if (2 > count2)
			{
				temp_min=I0-0.2;
				temp_max=I0+0.2;
			}
			else if (2 <= count2 && count2 <4)
			{
				temp_min=I0-0.8;
				temp_max=I0;
			}
			else
			{
				temp_min=I0;
				temp_max=I0+0.8;  
			}
	
			count=0;
		    while(count<10)		                
			{
				errChk(SET_DAC_MOD(channel, setDAC)); 
		
			//	Delay(0.5);
				Delay(0.3);
			
				errChk(Read_Ibias(channel, &Bias_s4));
		
				if (Bias_s4>=temp_min&&Bias_s4<=temp_max)
				{
					R0_s4=setDAC;
				
					conut2OK=1;
					break;
				}
				else if((DACMax-DACMin)<2) 
				{
					break;
				} 
				else
				{
					if(Bias_s4<I0) 
					{
						DACMax=setDAC;
						setDAC=(DACMax+DACMin)/2;
					}
					else					 
					{
						DACMin=setDAC;
						setDAC=(DACMax+DACMin)/2;
					}
				}
				count++;
			}
	
			if (conut2OK) break;  //���Գɹ����˳�ѭ��
		
			count2++;
			Delay(1); 	
		} while (count2<6);			//130523 change 5 to 6
	
		data->RxAmp=count2;
		
		if (count2 >= 6) 
		{
			MessagePopup ("Error", "����ER��ʱ!");
			data->ErrorCode=ERR_TUN_ER;
			error = -1;
			goto Error;
		} 
	
		data->ucRm[index]=R0_s4+4; 		 // R0_s4Ϊ���Խ��
		data->ucRb=R5_s1;			 // R5_s1Ϊ���Խ��
	
		if (R0_s4>DB_DACmax)
		{
			MessagePopup ("Error", "MOD DAC������������");
			data->ErrorCode=ERR_TUN_DAC_255or0; 
			error = -1;
			goto Error;
		}    
	
		if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 
		{
			//error = GET_DAC_BIASCT_R(&DAC_BiasCT_R); //��ȡBiasCT_R������дBias���ұ� 
			if (ux3328_get_biasct_r_dac(INST_EVB[channel], &UX3328_BiasCT_R)<0) 
			{
				return -1; 
			}
		
			//mod���ұ�
			error = ux3328_fit_mod_cClass (INST_EVB[channel], my_struDBConfig_ERCompens.Im10_33, my_struDBConfig_ERCompens.Im60_33, my_struDBConfig_ERCompens.Im82_60, data->ucRm[index]);
			if (error) goto Error; 
		
			//Bias���ұ�
			error = ux3328_fit_APC_cClass (INST_EVB[channel], my_struDBConfig_ERCompens.Ib10_33, my_struDBConfig_ERCompens.Ib60_33, UX3328_BiasCT_R);   
			if (error) 
			{
				goto Error;  
			}
		
			//�л����Զ�ģʽ
			error = ux3328_set_mode (INST_EVB[channel], 1);
			if (error) goto Error;
		
			errChk(Read_Ibias (channel, &temp_Ibias));
		
			data->TxV = temp_Ibias; 
		
			if(temp_Ibias<5)
			{
				MessagePopup ("Error", "���I_Bias<5mA!");
				error = -1;   
				goto Error;					
			}
		}
		else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 
		{
		
			//error = GET_DAC_BIASCT_R(&DAC_BiasCT_R); //��ȡBiasCT_R������дBias���ұ�   UX3328S_BiasCT_R
		//	if (ux3328s_get_biasct_r_dac(INST_EVB[channel], &UX3328S_BiasCT_R)<0) return -1; 
			//mod���ұ�
			error = ux3328s_fit_mod_cClass (INST_EVB[channel], my_struDBConfig_ERCompens.Im10_33, my_struDBConfig_ERCompens.Im60_33, my_struDBConfig_ERCompens.Im82_60, data->ucRm[index]);
			if (error) goto Error; 
		
			//Bias���ұ�
		//	error = ux3328s_fit_APC_cClass (INST_EVB[channel], my_struDBConfig_ERCompens.Ib10_33, my_struDBConfig_ERCompens.Ib60_33, UX3328S_BiasCT_R);   
		//	if (error) 
		//	{
		//		goto Error;  
		//	}
		
			//�л����Զ�ģʽ
			error = ux3328s_set_mode (INST_EVB[channel], 1);
			if (error) goto Error;
		
			errChk(Read_Ibias (channel, &temp_Ibias));
		
			data->TxV = temp_Ibias; 
		
			if(temp_Ibias<5)
			{
				MessagePopup ("Error", "���I_Bias<5mA!");
				error = -1;   
				goto Error;					
			}
		}
		else 
		{
			MessagePopup ("��ʾ", "���ݿ�AutoDT_Spec_Tracking�ж����PN_TYPE�д���"); 
			error = -1;   
			goto Error;
		} 
	}
	
	Read_AOP(channel, &data->AOP); 
	/***ȡ���⹦������+0.5**xuzhi**20141121//
	if (data->AOP>my_struDBConfig.TxPowMax || data->AOP<my_struDBConfig.TxPowMin) 
	{
		MessagePopup ("Error", "����ȵ��Խ�����⹦�ʼ��ʧ�ܣ�"); 
		error = -1;
		goto Error;
	}   
	
	//�õ�Pavg����λmW������ERֵ�����뱣֤��ʱ��PL��ȷ
	Pavg = pow(10, data->AOP/10); 
	data->ER[index]=10*log10((2*Pavg-P0)/P0);
	
	//���Ibias�����Ƿ񳬳��趨ֵ
	if (data->TxV>my_struDBConfig_ERCompens.IBias_Max)
	{
		sprintf (buf, "Ibias���������������ֵ%.2f    ", my_struDBConfig_ERCompens.IBias_Max);
		MessagePopup ("��ʾ", buf);
		data->ErrorCode=ERR_TUN_BIAS_MAX;
		error = -1;   
		goto Error;					
	}
	  
Error:
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
		ux3328s_set_UserMode(INST_EVB[channel]);
	} 
	return error;  */
	return 0;
}

int DB_Seach_Optics_Data_Vbr(char sn[], double *Vbr, double *Iop, double *Vapd)
{
	char buf[1024];
	int  count, temp_T_Flag ;
	double temp_Vbr; 
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	*Vbr=0;
	Fmt (buf, "SELECT APD_T_Vbr,APD_T_Iop,APD_T_Flag,APD_D_Vapd from Optics_Data where SN='%s'", sn);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	
	resCode = DBBindColDouble (hstmt, 1, &temp_Vbr, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 2, Iop, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 3, &temp_T_Flag, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 4, Vapd, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
   
    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) 
	{ShowDataBaseError(); return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	*Vbr=temp_Vbr;
	
	if (count != 1) {return -1;} 
	
	if (*Iop<5)
	{
		MessagePopup ("��ʾ", "���ݿ�Optics_Data��APD_T_Iop��¼���ʧ�ܣ�"); 
		return -1;
	}  
	
	if (temp_T_Flag != 1) return -1; 

	return 0;
}

int DB_Update_Optics_Data(struct struTestData data)
{
	char buf[2048];
	int resCode;
	
	if (SERVER_ORACLE == my_struConfig.servertype)
	{
		sprintf (buf, "UPDATE Optics_Data SET Vapd_D_Date = to_date('%s','yyyy-mm-dd hh24:mi:ss') ,Vapd_D_Temper = %.2f,Vapd_D_Target = %.2f,Vapd_D_Reality = %.2f , Rapd1 = %.2f ,	Rapd2 = %.2f ,	Vapd_D_Flag = 1 WHERE   SN = '%s'",
			 data.TestDate,data.Vapd_D_Temper,data.Vapd_D_Target,data.Vapd_D_Reality,data.Rapd1,data.Rapd2,data.OpticsSN); 
	}
	else
	{
		sprintf (buf, "UPDATE Optics_Data SET Vapd_D_Date = '%s' ,Vapd_D_Temper = %.2f ,Vapd_D_Target = %.2f,Vapd_D_Reality = %.2f ,Rapd1 = %.2f ,Rapd2 = %.2f ,Vapd_D_Flag = 1 WHERE  SN = '%s'",
			data.TestDate,data.Vapd_D_Temper,data.Vapd_D_Target,data.Vapd_D_Reality,data.Rapd1,data.Rapd2,data.OpticsSN); 
	}
	
	resCode = DBImmediateSQL (hdbc, buf); 
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}  

	return 0;
}

int tuningSD (int channel, struct struTestData *data)
{	 
	int error;

	error = tuningSD_DWDM_ONU (channel, data);    
	
	return error;
}

int tuningSD_SDA (int channel, struct struTestData *data)
{	 
int	error=0;

	
Error:
	
	return error;
}

int tuningSD_SDD(int channel, struct struTestData *data)
{	 
int	error=0;

Error:
	
	return error;
}

int BERT_Start (int channel, double testtime, int isTestErrbitFlag)
{
	int Lock, ErrorCount; 
	int error, count, sBERT_Status_Code;
	char 	sBERT_Status_String[500];
	double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime; 
	unsigned int InstantErrNum; 
	double  InstantErrTime, InstantErrRate;

	switch (INSTR[channel].BERT_TYPE_ONU) 
	{
		case BERT_TYPE_GBERT:
			
			if (!GBERT_Start(INST_BERT[channel], my_struDBConfig.SensTime))
			{
				MessagePopup("Error", "Reset GBERT error!"); 
				return -1;
			}
			
			break; 
			
		case BERT_TYPE_SBERT:
			
			//���ȳ�ʼ��SBERT����֤��������
			error = EVB5_SBERT_Init(INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS);
			if (error) 
			{
				MessagePopup("��ʾ","SBERT����ʧ�ܣ�"); 
				return -1;
			} 
			
			Delay (1);
			
			if (EVB5_SBERT_Start(INST_BERT[channel], INSTR[channel].SBert_Rate, my_struDBConfig.SensTime)<0) 
			{
				MessagePopup("Error", "SBERT start test error!"); 
				return -1;
			}
			
			break; 
				
		default:
			
			MessagePopup("Error", "can not find this BERT Type!"); 
			return -1;
	}
	return 0;
}

int testFixOver(int channel, struct struTestData *data,struct struProcessLOG *prolog)
{
	int error=0;

	double 	temp_over;
	BYTE 	SD;
	int     timer_star, err_count, timer;
	char ErrInfo[50] = "";
	

	temp_over = my_struDBConfig.Over+sRxCal.power[channel];
	errChk (SET_ATT_ONU (channel, temp_over));	  	//���ù��ع��� 
	
	errChk (Read_RX_SD (channel, 0, &SD));   			//���SD�ź�   0��ģ��los, 1:û��los  
	
	if(SD==1) //0��ģ��los		//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_over=temp_over+2.0;     	  
			errChk (SET_ATT_ONU (channel, temp_over)); 
			
			errChk (Read_RX_SD (channel, 1, &SD));
			
		} while ((SD==0)&&(temp_over<0));
	
		temp_over = my_struDBConfig.Over+sRxCal.power[channel];
		errChk (SET_ATT_ONU (channel, temp_over));		//�������������ȹ���
		
		errChk (Read_RX_SD(channel, 1, &SD)); 
		if(SD == 0)	
		{
			error = -1;
			goto Error;							//���������LOS�źţ�����
		}
	}
	
	errChk (BERT_Start (channel, my_struDBConfig.OverTime, 1));
	
	timer_star=Timer(); 
	do
	{
		//��ȡ����
		Delay(1); 
		
		errChk (Read_Error_Bit(channel, &err_count, 0, my_struDBConfig.OverTime, 0, ErrInfo,prolog->Comments));           
		
		timer = Timer()- timer_star; 
	} while (timer<my_struDBConfig.OverTime && err_count==0);

	errChk (BERT_Stop (channel)); 		 
	
	//�жϲ��Խ��,����ֵ 
	if (timer<my_struDBConfig.OverTime || err_count>0) 
	{
		data->Over=0; 
		error = -1;
		goto Error;
	}

	data->Over=my_struDBConfig.Over;
	data->OverTime=timer;  
	
Error:
	
	return error;
}

int testFixOver_Start(int channel, int *StartTime, char ErrInfo[],char ErrInfo_Ex[])
{
	int	error=0;

	BYTE 	SD;
	double 	temp_sens;
	int	err_count, status;
	int OptoBERT_Lock, OptoBERT_ErrorCount;
	
	//�����������
    strcpy(ErrInfo, "");

	temp_sens = my_struDBConfig.Over+sRxCal.power[channel]; 
	
	SET_ATT_ONU(channel, temp_sens);	  	//���������ȹ���       
	Read_RX_SD(channel, 0, &SD);   								//���SD�ź�   0��ģ��los, 1:û��los  
	if(SD == 1) //0��ģ��los							//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;     
			SET_ATT_ONU(channel, temp_sens); 
			Read_RX_SD(channel, 1, &SD);
		} while ((SD==0)&&(temp_sens<0));
	
		temp_sens = my_struDBConfig.Over+sRxCal.power[channel]; 

		SET_ATT_ONU(channel, temp_sens);	//�������������ȹ���
		
		Read_RX_SD(channel, 1, &SD); 
		if(SD == 0)
		{
			error = -1;
			goto Error; 						//���������LOS�źţ�����
		}
	}
	
	errChk (BERT_Start (channel, my_struDBConfig.OverTime, 1));
	
	*StartTime=Timer(); 
	
	Delay(0.3); 
	
	errChk (Read_Error_Bit(channel, &err_count, 0, my_struDBConfig.OverTime, 0, ErrInfo,ErrInfo_Ex));
	
	if (err_count!=0) 
	{
		error = -1;
	//	MessagePopup("Error", "Sens test Start fail!"); 
		goto Error;
	} 
	
Error:
	
	return error;
}

int testFixOver_End(int channel, int StartTime, struct struTestData *data, char ErrInfo[],char ErrInfo_Ex[])
{
	double 	temp_sens;
	int	timer_star, timer, err_count, error;

	int count;

	count = 0;
	timer_star=StartTime; 
	do
	{
		//��ȡ����
		Delay(1); 
		errChk(Read_Error_Bit(channel, &err_count, 1, 0, count, ErrInfo,ErrInfo_Ex));
		timer = Timer()- timer_star; 
		count++;   
	} while (timer<my_struDBConfig.OverTime && err_count==0);
	
	errChk(BERT_Stop (channel));

	//�жϲ��Խ��,����ֵ 
	if (timer<my_struDBConfig.OverTime || err_count>0) 
	{
		data->Sens=0; 
		error = -1;
		goto Error;
	}

	data->Over=my_struDBConfig.Over;
	data->OverTime=timer;
	
Error:
	
	if (BERT_Stop (channel) < 0)
	{
		return -1;
	}
	
	return error;
}

int testCenterSens(int channel, struct struTestData *data,struct struProcessLOG *prolog)
{
	int error=0;

	double 	temp_CenterSens;
	BYTE 	SD;
	int     timer_star, err_count, timer;
	char ErrInfo[50] = "";

	temp_CenterSens = my_struDBConfig.Over+sRxCal.power[channel];
	errChk (SET_ATT_ONU (channel, temp_CenterSens));	  	//���ù��ع��� 
	
	errChk (Read_RX_SD (channel, 1, &SD));   			//���SD�ź�   0��ģ��los, 1:û��los  
	
	if(SD==0) //0��ģ��los		//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_CenterSens=temp_CenterSens+2.0;     	  
			errChk (SET_ATT_ONU (channel, temp_CenterSens)); 
			
			errChk (Read_RX_SD (channel, 1, &SD));
			
		} while ((SD==0)&&(temp_CenterSens<0));
	
		temp_CenterSens = my_struDBConfig.CenterSens+sRxCal.power[channel];
		errChk (SET_ATT_ONU (channel, temp_CenterSens));		//�������������ȹ���
		
		errChk (Read_RX_SD(channel, 1, &SD)); 
		if(SD == 0)	
		{
			error = -1;
			goto Error;							//���������LOS�źţ�����
		}
	}
	
	errChk (BERT_Start (channel, my_struDBConfig.CenterSensTime, 1));
	
	timer_star=Timer(); 
	do
	{
		//��ȡ����
		Delay(1); 
		
		errChk (Read_Error_Bit(channel, &err_count, 0, my_struDBConfig.CenterSensTime, 0, ErrInfo,prolog->Comments));           
		
		timer = Timer()- timer_star; 
	} while (timer<my_struDBConfig.CenterSensTime && err_count==0);

	errChk (BERT_Stop (channel)); 		 
	
	//�жϲ��Խ��,����ֵ 
	if (timer<my_struDBConfig.CenterSensTime || err_count>0) 
	{
		data->Over=0; 
		error = -1;
		goto Error;
	}

	data->CenterSens=my_struDBConfig.CenterSens;
	data->CenterSensTime=timer;  
	
Error:
	
	return error;
}

int BERT_Read(int channel, unsigned int *ErrorCount)
{
	int 	Lock, error, sBERT_Status_Code; 
	char 	sBERT_Status_String[500];
	double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime; 
	unsigned int InstantErrNum; 
	double  InstantErrTime, InstantErrRate;
	
	switch (INSTR[channel].BERT_TYPE_ONU)
	{
		case BERT_TYPE_NONE:
			break; 
			
		default: 
			MessagePopup("Error", "can not find this BERT Type!"); 
		    return -1;
	}
	return 0;
}

int testFixSens_Start(int channel, int *StartTime, char ErrInfo[],char ErrInfo_Ex[])
{
	int	error=0;

	BYTE 	SD;
	double 	temp_sens;
	int	err_count, status;
	int OptoBERT_Lock, OptoBERT_ErrorCount;
	
	//�����������
    strcpy(ErrInfo, "");

	temp_sens = my_struDBConfig.Sens+sRxCal.power[channel]; 
	
	SET_ATT_ONU(channel, temp_sens);	  	//���������ȹ���       
	Read_RX_SD(channel, 0, &SD);   								//���SD�ź�   0��ģ��los, 1:û��los  
	if(SD == 1) //0��ģ��los							//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;     
			SET_ATT_ONU(channel, temp_sens); 
			Read_RX_SD(channel, 1, &SD);
		} while ((SD==0)&&(temp_sens<0));
	
		temp_sens = my_struDBConfig.Sens+sRxCal.power[channel]; 

		SET_ATT_ONU(channel, temp_sens);	//�������������ȹ���
		
		Read_RX_SD(channel, 1, &SD); 
		if(SD == 0)
		{
			error = -1;
			goto Error; 						//���������LOS�źţ�����
		}
	}
	
	errChk (BERT_Start (channel, my_struDBConfig.SensTime, 1));
	
	*StartTime=Timer(); 
	
	Delay(0.3); 
	
	errChk (Read_Error_Bit(channel, &err_count, 0, my_struDBConfig.SensTime, 0, ErrInfo,ErrInfo_Ex));
	
	if (err_count!=0) 
	{
		error = -1;
	//	MessagePopup("Error", "Sens test Start fail!"); 
		goto Error;
	} 
	
Error:
	
	return error;
}

int testFixSens_End(int channel, int StartTime, struct struTestData *data, char ErrInfo[],char ErrInfo_Ex[])
{
	double 	temp_sens;
	int	timer_star, timer, err_count, error;

	int count;

	count = 0;
	timer_star=StartTime; 
	do
	{
		//��ȡ����
		Delay(1); 
		errChk(Read_Error_Bit(channel, &err_count, 1, 0, count, ErrInfo,ErrInfo_Ex));
		timer = Timer()- timer_star; 
		count++;   
	} while (timer<my_struDBConfig.SensTime && err_count==0);
	
	errChk(BERT_Stop (channel));

	//�жϲ��Խ��,����ֵ 
	if (timer<my_struDBConfig.SensTime || err_count>0) 
	{
		data->Sens=0; 
		error = -1;
		goto Error;
	}

	data->Sens=my_struDBConfig.Sens;
	data->SensTime=timer;
	
Error:
	
	if (BERT_Stop (channel) < 0)
	{
		return -1;
	}
	
	return error;
}

int	testDetailSens (int channel, struct struTestData *data)
{
	int  error, err_count;
	int count;
	BYTE SD;
	double 	temp_sens, stepdown=0.5, stepup=0.2, timer_star, timer;
	
	//���ó�ʼ�⹦��, �����ݿ��еĲ���ָ��Ϊ��ʼ��
	temp_sens = my_struDBConfig.Sens+sRxCal.power[channel]; 
	
	SET_ATT_ONU(channel, temp_sens);	  	//���������ȹ���       

//���SD�źţ����û�м�⵽SD�����ӹ⹦��
	Read_RX_SD(channel, 0, &SD);   				//���SD�ź�   0��ģ��los, 1:û��los  
	if(SD == 1) //0��ģ��los		//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;     
			SET_ATT_ONU(channel, temp_sens);
			Read_RX_SD(channel, 1, &SD);
		} while (SD==1 && temp_sens<0);
	
		temp_sens = my_struDBConfig.Sens+sRxCal.power[channel]; 

		SET_ATT_ONU(channel, temp_sens);	//�������������ȹ��� 
		Read_RX_SD(channel, 1, &SD); 
		if(SD == 1)	 					   				//���������LOS�źţ�����
		{
			MessagePopup("��ʾ", "�������ȹ��ʵ�SD�źż��ʧ��");
			goto Error;
		}
	}

//BERT��ʼ���� 
	error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
	if (error) 
	{
		goto Error; 
	}
	
//��ȡ����״̬������������룬ֱ���˳� 
	Delay(0.3); 
	if (!Read_Error_Bit_Ex(channel, &err_count, 0)) 
	{
		//��������������ڼ��һ��
		error = BERT_Stop (channel); 
		if (error) 
		{
			goto Error;
		}
		
		Delay(0.3); 
	
		error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
		if (error) 
		{
			goto Error; 
		}
		
		Delay(0.5);
		
		if (!Read_Error_Bit_Ex(channel, &err_count, 0))
		{MessagePopup("��ʾ", "�����ȿ�ʼ����ʱ��������"); goto Error;}
	}
	
	//��ʱ�Ĺ⹦��=my_struRxCal_OLT.RxCal_Power_Max�����ܳ������� 
	if (err_count>0)
	{
		goto Error;  
	}

//���ù⹦�ʵ��պò��������� 
	count=0;
	do 
	{
		temp_sens = temp_sens - stepdown;  
		SET_ATT_ONU(channel, temp_sens);//���������ȹ���  
	
		error = BERT_Stop (channel); 
		if (error) 
		{
			goto Error;
		}
		
		Delay(0.3); 
	
		error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
		if (error)
		{
			goto Error; 
		}

		Delay(0.5);
		
		Read_Error_Bit_Ex(channel, &err_count, 0);    
		if(0==err_count)
		{
			count=0;
		}
		else
		{
			count++;
		}
	} while((err_count==0 || count<3));     
	
//����˥����ֱ��û������Ϊֹ 
	do 
	{
	 	temp_sens = temp_sens + stepup;  
		
		SET_ATT_ONU(channel, temp_sens);
	
		error = BERT_Stop (channel); 
		if (error) 
		{
			goto Error;
		}
		
		Delay(0.3); 
	
		error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
		if (error) 
		{
			goto Error; 
		}

		Delay(0.5);
		
		Read_Error_Bit_Ex(channel, &err_count, 0);    
	
	} while (err_count != 0);
	
//�����㹻ʱ��û������ 
	error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
	if (error) 
	{
		goto Error; 
	}
	
	do
	{
		timer_star=Timer(); 
		do
		{
			//��ȡ����
			Delay(1); 
			Read_Error_Bit_Ex(channel, &err_count, 1);
			timer = Timer()- timer_star; 
		} while (timer<my_struDBConfig.SensTime && err_count==0);
	
		if(err_count != 0)
	    {
		    temp_sens = temp_sens + stepup;  
			SET_ATT_ONU(channel, temp_sens);
		
			Delay(0.5); 
			
			error = BERT_Stop (channel); 
			if (error)
			{
				goto Error;
			}
		
			Delay(0.3); 
	
			error = BERT_Start (channel, my_struDBConfig.SensTime, 1);
			if (error) 
			{
				goto Error; 
			}
	   	}
	}while (err_count>0); 
	
//�ر�BRET
	error = BERT_Stop (channel); 
	if (error) 
	{
		return -1;
	}
	
	data->Sens = temp_sens-sRxCal.power[channel]; 
	data->SensTime=timer;
	
	if (data->Sens>my_struDBConfig.Sens) 
	{
		return -1;
	}
	
	return 0;
	
Error:
	error = BERT_Stop (channel);

	return -1;
}

int testDetailSens_Extra (int channel, struct struTestData *data, struct struProcessLOG *prolog)
{
	BOOL status;   
	int  phase7,timer_star,timer;   
	double power, o_delta, sens, optics, ber;   
	double ber_array[10], optics_array[10]; 
	char Inf[512];

	timer_star=Timer();
	ProcessDrawEvents (); 
	//��1.0e-5~1.0e-4~�Ĺ�																			
	status = Find_1E5_Phase(channel, &power, &ber);
	if (!status)
	{
		return -1;  
	}	
	ProcessDrawEvents (); 
	
	//��ȡ����
	status = Get_Extra_Data(channel, power, ber, ber_array, optics_array); 
	if (!status)	
	{
		return -1;
	}	
	ProcessDrawEvents (); 
	
	//�������  
	status = Fit_Extra_Data(ber_array, optics_array, &sens);  
	if (!status)	
	{
		return -1; 
	}
	timer = Timer()- timer_star;
	ProcessDrawEvents (); 
	
	data->Sens = sens;
	data->SensTime = timer;    
	sprintf(Inf, "  (%.16f, %.16f)(%.16f, %.16f)(%.16f, %.16f)(%.16f, %.16f)(%.2f)  ", ber_array[0], optics_array[0], ber_array[1], optics_array[1], ber_array[2], optics_array[2], ber_array[3], optics_array[3], data->Sens );
	strcat (prolog->Comments, Inf);
	if (my_struDBConfig.Sens < data->Sens)
	{	
		return -1;
	}	
	
	return 0;
}

BOOL Find_1E5_Phase(int channel, double *power5, double *ber_5)
{
	BOOL status;
	int i, phase_7[10], maxIndex, minIndex, k; 
	double power, step, ber_temp, ber_7[10], maxValue, minValue, ber_temp3;
	
	//���ó�ʼ�⹦��, �����ݿ��еĲ���ָ��-4.5Ϊ��ʼ��          
	power = my_struDBConfig.Sens -4.5 + sRxCal.power[channel];
	step = 0.3;
	i = 0;
	k = 0;
	do
	{
		i++;       

		if(k>1)
		{
			MessagePopup("Error", "�����ʲ��ȶ���    ");
			return FALSE;
		}
		
		if (SET_ATT_ONU(channel, power)<0)	
		{
			return FALSE;
		}
		if (BERT_Start(channel, 5, 0))	
		{
			return FALSE; 
		}
		
		Delay(1);
		
		if (BERT_Get_BER(channel, &ber_temp))
		{
			return FALSE; 
		}
		
		if (ber_temp < 1.0e-5)
		{
			power = power - step;
			continue;
		}
		else if (ber_temp > 1.0e-3)
		{
			power = power + step;
			continue;  
		}	 
		
		Delay(2);
		
		if (BERT_Get_BER(channel, &ber_temp3))
		{
			return FALSE;
		}
		
		if((ber_temp/ber_temp3)>1.3 || (ber_temp/ber_temp3)<0.7)
		{
			power = power -0.2;
			k++;
			continue;
		}
		
		k=0; 
		
		ProcessDrawEvents ();

		if (i>15) 
		{
			break;
		}
		
	} while (ber_temp>1.0e-3 || ber_temp<1.0e-5);
	
	if (i>15)   
	{
		MessagePopup("Error", "�޷���ȡ��������1.0e-5~1.0e-4�ĵ�    ");  
		return FALSE;
	}

	*power5 = power - sRxCal.power[channel];
	*ber_5  = ber_temp;
	return TRUE;   
}

BOOL Get_Extra_Data(int channel, double power, double ber_5, double *ber_array, double *optics_array)
{
	BOOL status;
	int i=0, number=4, class=0, j, num, k; 
	int time_wait[5]={20,20,20,30,120}, timewait=0, time_delay[4]={1, 1, 2, 3};
	double ber_temp, step,step_aop, rxpower, ber[10], sum_ber, ber_temp3;
	double ber_max_num, ber_min_num;
	char Inf[512];
	
	step_aop = 0.5;

	ber_array[0] = ber_5;
	optics_array[0] = power;
	
	power = power + sRxCal.power[channel];
	power = power + step_aop;
	i=1;
	k=0;
	do
	{
		if(k>1)                                            
		{
			MessagePopup("Error", "�����ʲ��ȶ���    ");
			return FALSE;
		}
		if (SET_ATT_ONU(channel, power)<0)	
		{	
			return FALSE;
		}	
		if (BERT_Start(channel, time_wait[i], 1))
		{
			return FALSE;
		}	
		Delay(time_delay[i]);
		if (BERT_Get_BER(channel, &ber_temp))	
		{
			return FALSE;
		}	
		Delay(2);
		if (BERT_Get_BER(channel, &ber_temp3))
		{
			return FALSE;
		}
		if((ber_temp/ber_temp3)>1.3 || (ber_temp/ber_temp3)<0.7)
		{
			Delay(1);
			if (BERT_Get_BER(channel, &ber_temp))
			{
				return FALSE; 
			}	
			if((ber_temp/ber_temp3)>1.3 || (ber_temp/ber_temp3)<0.7) 
			{
				power=power - 0.2;
				k++;
				continue;
			}	
		}
		k=0;
		if(BERT_Stop(channel))  
		{
			return FALSE; 
		}
		ber_array[i] = ber_temp;
		optics_array[i] = power - sRxCal.power[channel];

		power += step_aop;
		i++;
		ProcessDrawEvents (); 
	}while(i<number);
	
	return TRUE;
}

BOOL Fit_Extra_Data(double ber_array[], double optics_array[], double *sens_12)
{
	int error=0, count=0;
	double BERArray[10], OpticsA[10], Log_Array[10];
	double output[10], slope, intercept, mean, magnitude, sens;   
	
	//Ӧ�����������жϣ�����3�����������У�
	error = CheckData(ber_array, optics_array, BERArray, OpticsA, &count); 
	if(-1 == error)   //����ȫ��������һ�������� ,�˳�
	{
		MessagePopup("Error","���Եõ���BERȫ������һ��������\n���ݲ�����");
		return FALSE;
	}
	ProcessDrawEvents (); 
	Get_loglog_Arr(BERArray, count, Log_Array);
	LinFit(OpticsA, Log_Array, count, output, &slope, &intercept, &mean); 
	sens = (Get_loglog_val(1.0e-12) - intercept) / slope; 
	*sens_12 = sens;
	
	return TRUE; 	
}

int CheckData(double ber_arr[], double optics_arr[], double *Ber, double * Optics, int *count)
{
	int i,j;
	double data_temp, value;
	short int flag[8]={0};  
	
	j=0;
	for (i=0; i<4; i++)
	{
		if ((ber_arr[i] > 0) && (ber_arr[i] < 1))
		{	
			data_temp = ber_arr[i] / ber_arr[i+1];
			if (data_temp > 2)
			{
				Ber[j] = ber_arr[i];
				Optics[j] = optics_arr[i];
				j++;
			}
		}
	}
	*count = j;
	
	for (i=1; i<j; i++)
	{
		value = Ber[0]/Ber[i];
	
		if (value >=1.0E4)
			flag[4]=1;
		else if (value >= 1.0E3)
			flag[3]=1;
		else if (value >= 100.0)
			flag[2]=1;
		else if (value >= 10.0)
			flag[1]=1;
		else
			flag[0]=1;
	}
	
	if ((1 == flag[4])||(1 == flag[3])||(1 == flag[2]))
		return 0;
	else if (1 == flag[1])
		return 2;
	else 
		return -1;
}

void Get_loglog_Arr(double BER_val[],int Data_Num,double *Result)
{
	int i;
	double temp;
	for(i =0;i<Data_Num;i++ )
	{
		
		*(Result+i) = Get_loglog_val(BER_val[i]);
	}
}

//����Log��-Log��x����
double Get_loglog_val(double BER_val)
{
	double temp;
	if(BER_val<0)
	{return -1.0;}//�ж��Ƿ���
	temp = log10(BER_val);
	if(temp>=0)   //���temp>=0����˵�� BER_val>1���϶�����
	{return -1.0;}
	temp = log10(0 - (temp));//�˴����Բ����ж�����������Ϊ��BER_val<1ʱ��log(BER_val)�Ǹ�����
	return temp;
}

int BERT_Get_BER(int channel, double *ber)
{
	int error, errorcode;
	char 	sBERT_Status_String[500]; 
	int 	sBERT_Status_Code;	  
	double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime; 
	
	if (INSTR[channel].BERT_TYPE_ONU==BERT_TYPE_GBERT)
	{
		if(!GBERT_Check_Ratio(INST_BERT[channel], &sBERT_ERR_Ratio, &errorcode))
		{
			MessagePopup("Error", " GBERT Read error!"); 
			return -1;	
		}	
	}
	else if(INSTR[channel].BERT_TYPE_ONU==BERT_TYPE_SBERT)
	{
		//���ȳ�ʼ��SBERT����֤��������
		//error = EVB5_SBERT_Init(SBERT_USBHandle, my_struInstrument.SBert_Rate, SBERT_P_PRBS7);
		//if (error) {MessagePopup("��ʾ","SBERT����ʧ�ܣ�"); return -1;} 
		
		Delay (1);
		
		error = EVB5_SBERT_Read(INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
		//����������С��5����Ϊû�����룬��ֹ����
		/*
		if ((sBERT_Status_Code==-3 || sBERT_Status_Code==-1) && sBERT_ERR_Number<5) 
		{
			sBERT_Status_Code=1;
			sBERT_ERR_Number =0;
		}
		if (error<0 || sBERT_Status_Code<0) 
		*/
		if (error<0) 
		{
			return -1;
		} 
	}
	else
	{
		MessagePopup ("ERROR", "BERT Type error!"); 
		return -1; 
	}
	
	*ber = sBERT_ERR_Ratio;
	return 0; 
}

int testCurrent(int channel, struct struTestData *data)
{
	float temp_Ibias;
	int index,error;
	
	error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
	if (error) 
	{
		MessagePopup("����", "3328���ù���ģʽ����   "); 
		return -1;
	}
	Delay(0.2);
	
	for(index=0;index<WAVELENGTHCHANNEL;index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0)
		{
			return -1; 
		}  
		Delay(2);
	
		//ͬʱ��ȡTxI ,RxI  
		if (EVB5_READ_TxIRxI(INST_EVB[channel], &data->TxI[index],&data->RxI[index])<0) 
		{
			return -1;
		}
		
		if (data->TxI[index]>my_struDBConfig.TxIMax || data->RxI[index]>my_struDBConfig.RxIMax) 
		{
			return -1;  
		}

		//Read Vcc,Ibias,Txpower
		if (Get_8472_Monitor(INST_EVB[channel],&data->A2_Vcc[index], &data->A2_temperatrue[index],&data->A2_Ibias[index],&data->A2_TxPower[index],&data->LDTemper[index],&data->TecCurrent[index])<0) 
		{
			return -1;
		}
		
		//Read LD Tempe && Tec Current   ��������Get_8472_Monitor��������
//		if(ux3328s_get_LDtempe_TecCurr (INST_EVB[channel],  &data->LDTemper[index],&data->TecCurrent[index])<0)
//		{
//			return -1;
//		}
		
		//Read Mod DAC
		if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
		{
			return -1;  
		}
	
		//Read APC DAC
		if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
		{
			return -1;  
		}
	}

	return 0;
}

int RecordMonitor(int channel,int index, struct struTestData *data)
{   
	float temp_Ibias;
	int error;
	
	//ͬʱ��ȡTxI ,RxI  
	if (EVB5_READ_TxIRxI(INST_EVB[channel], &data->TxI[index],&data->RxI[index])<0) 
	{
		return -1;
	}
		
	if (data->TxI[index]>my_struDBConfig.TxIMax || data->RxI[index]>my_struDBConfig.RxIMax) 
	{
		return -1;  
	}

	//Read Vcc,Ibias,Txpower
	if (Get_8472_Monitor(INST_EVB[channel],&data->A2_Vcc[index], &data->A2_temperatrue[index],&data->A2_Ibias[index],&data->A2_TxPower[index],&data->LDTemper[index],&data->TecCurrent[index])<0) 
	{
		return -1;
	}
	
	return 0;
}


int teststation(int channel, struct struTestData *data, struct struProcessLOG prolog)
{
	int  retParam, errcode, resCode, hstmt;
	char *errmsg;
	
	/* Prepare a statement which calls the stored procedure */
    resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_STORED_PROC);
	 if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
    hstmt = DBPrepareSQL (hdbc, "sp_workstation_check");
    if (hstmt <= 0) {ShowDataBaseError();  return -1;} 
	 
	resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_UNKNOWN);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
	if (SERVER_ORACLE == my_struConfig.servertype)
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "V_return", DB_PARAM_OUTPUT, -1);  
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "V_pn", DB_PARAM_INPUT, my_struConfig.PN, strlen(my_struConfig.PN)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_bom", DB_PARAM_INPUT, my_struConfig.BOM, strlen(my_struConfig.BOM)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		resCode = DBCreateParamChar (hstmt, "V_station", DB_PARAM_INPUT, prolog.Log_Action, strlen(prolog.Log_Action)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_sn", DB_PARAM_INPUT, data->OpticsSN, strlen(data->OpticsSN)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_errmsg ", DB_PARAM_OUTPUT, "", 500); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "V_errcode", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	else
	{	 
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "", DB_PARAM_RETURN_VALUE, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "@pn", DB_PARAM_INPUT, my_struConfig.PN, strlen(my_struConfig.PN)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@bom", DB_PARAM_INPUT, my_struConfig.BOM, strlen(my_struConfig.BOM)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		resCode = DBCreateParamChar (hstmt, "@station", DB_PARAM_INPUT, prolog.Log_Action, strlen(prolog.Log_Action)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@sn", DB_PARAM_INPUT, data->OpticsSN, strlen(data->OpticsSN)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@errmsg ", DB_PARAM_OUTPUT, "", 500); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "@errcode", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	
    /* Execute the statement */
    resCode = DBExecutePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;}  
	
    /* Close the statement.  Output parameters are invalid until the statement is closed */
    resCode = DBClosePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
    /* Examine the parameter values */
	resCode = DBGetParamInt (hstmt, 1, &retParam); 
	if (resCode < 0) {ShowDataBaseError();  return -1;}
    resCode = DBGetParamChar(hstmt, 6, &errmsg, "");		//�˴���Ҫ�Է��ص��ַ����ո�������
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
    resCode = DBGetParamInt(hstmt, 7, &errcode);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 

    /* Discard the statement */
    resCode = DBDiscardSQLStatement(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
	if (errcode)
	{
		if ((ERR_EEP_SHORT_VCC == errcode) || (ERR_EEP_SHORT_VAPD == errcode) || (ERR_EEP_SHORT_GND == errcode))		 /***��·�������**Eric.Yao***/
		{
			data->ErrorCode = errcode;
			//ɾ��������Ϣ�еĿո�
			MyDLL_TRIM (errmsg);
			MessagePopup("��ʾ", errmsg);
			return -1;
		}
		
		if (my_struConfig.DT_Tuning)
		{
			if (ERR_TES_AOP_MAX == errcode)	    /***���Թ����³��ֹ⹦��ƫ����Ϊ������OK**Eric.Yao***/
			{
				return 0;
			}
			else
			{
				//ɾ��������Ϣ�еĿո�
				MyDLL_TRIM (errmsg);
				MessagePopup("��ʾ", errmsg);	  
			}
		}
		else
		{
			if (ERR_TES_AOP_MAX == errcode)	   /***������ݷ��صĴ��������-430,�����ô������**Eric.Yao***/
			{
				MyDLL_TRIM (errmsg);
				MessagePopup("��ʾ", errmsg);
				data->ErrorCode = ERR_TES_AOP_MAX;    
			}
			else
			{
				//ɾ��������Ϣ�еĿո�
				MyDLL_TRIM (errmsg);
				MessagePopup("��ʾ", errmsg);	  
			}
		}
		return -1;
	}

	return 0;
}

int DB_Search_Tracking_BareBOM (char SN[], char bom[], char batch[])
{
	int  errcode, resCode, hstmt;
	char *errmsg, *mybom, *pn, *mybatch;
	
	/* Prepare a statement which calls the stored procedure */
    resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_STORED_PROC);
	 if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
    hstmt = DBPrepareSQL (hdbc, "sp_get_tracking_bom");
    if (hstmt <= 0) {ShowDataBaseError();  return -1;} 
	 
	resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_UNKNOWN);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
	if (SERVER_ORACLE == my_struConfig.servertype)
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "V_return", DB_PARAM_OUTPUT, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "V_sn", DB_PARAM_INPUT, SN, strlen(SN)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_bom", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_errmsg ", DB_PARAM_OUTPUT, "", 500); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "V_errcode", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_pn", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_batch", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	else
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "", DB_PARAM_RETURN_VALUE, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "@sn", DB_PARAM_INPUT, SN, strlen(SN)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@bom", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@errmsg ", DB_PARAM_OUTPUT, "", 500); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "@errcode", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@pn", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@batch", DB_PARAM_OUTPUT, "", 50); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	
    /* Execute the statement */
    resCode = DBExecutePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;}  
	
    /* Close the statement.  Output parameters are invalid until the statement is closed */
    resCode = DBClosePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
    /* Examine the parameter values */
	resCode = DBGetParamChar(hstmt, 3, &mybom, "");			
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	resCode = DBGetParamChar(hstmt, 4, &errmsg, "");		//�˴���Ҫ�Է��ص��ַ����ո�������
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	resCode = DBGetParamInt(hstmt, 5, &errcode);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	resCode = DBGetParamChar(hstmt, 6, &pn, "");			
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	resCode = DBGetParamChar(hstmt, 7, &mybatch, "");			
	if (resCode < 0) {ShowDataBaseError();  return -1;} 

    /* Discard the statement */
    resCode = DBDiscardSQLStatement(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
	if (errcode)
	{
		//ɾ��������Ϣ�еĿո�
		MyDLL_TRIM (errmsg);
		MessagePopup("��ʾ", errmsg); 
		return -1;
	}
	else
	{
		MyDLL_TRIM (mybom); 
		strcpy (bom, mybom);
		
		MyDLL_TRIM (pn); 
		MyDLL_TRIM (mybatch); 
		strcpy (batch, mybatch); 
	}
	
	return 0;
}

int calTxPower(int channel, struct struTestData *data)
{
	int  error = 0;
	
	if (DiverType_VSC7965 == my_struDBConfig_ERCompens.DriverChip)
	{
		errChk (calTxPower1 (channel));
	}
	else if (DiverType_VSC7967 == my_struDBConfig_ERCompens.DriverChip)
	{
		errChk (calTxPower2 (channel)); 
	}
	else if (DRIVERCHIP_NT25L90 == my_struDBConfig_ERCompens.DriverChip)
	{
		errChk (calTxPower2 (channel)); 
	}
	else if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		 errChk (calTxPower_ux3328 (channel));   
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
		 errChk (calTxPower_ux3328s (channel,data));   
	}
	else
	{
		error = -1;
		goto Error;
	}

Error:
	
	return error;
}

int DB_get_cali(enum enum_CALIBRATION_TYPE cali_type, int channel)
{
/*
(@pcname nvarchar(50), 
@timein smalldatetime,
@flag nvarchar(50),
@timeout int OUTPUT)
*/
	int  retParam, calitime;
	char timebuf[30], buf[1024];
	int  errcode, resCode, hstmt;
	char stationid[256];
	int status;
/*	
	status = MyDLLGetComputerName(my_struProcessLOG.StationID);
	if(!status) 
	{
		MessagePopup ("Error", "Read Computer Name error !"); 
		return -1; 
	}
*/	
	sprintf (stationid, "%s-%d",my_struProcessLOG.StationID, channel);
	
	MyDLL_GetDateTime(timebuf);
	
	/* Prepare a statement which calls the stored procedure */
    resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_STORED_PROC);
	 if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
    hstmt = DBPrepareSQL (hdbc, "p_get_calitime");
    if (hstmt <= 0) {ShowDataBaseError();  return -1;} 
	 
	resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_UNKNOWN);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
	if (SERVER_ORACLE == my_struConfig.servertype)
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "V_return", DB_PARAM_OUTPUT, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "V_pcname", DB_PARAM_INPUT, stationid, strlen(stationid)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "V_timein", DB_PARAM_INPUT, timebuf, strlen(timebuf)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		resCode = DBCreateParamChar (hstmt, "V_flag", DB_PARAM_INPUT, CALI_TYPE[cali_type], strlen(CALI_TYPE[cali_type])+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "V_timeout", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	else
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "", DB_PARAM_RETURN_VALUE, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
	    resCode = DBCreateParamChar (hstmt, "@pcname", DB_PARAM_INPUT, stationid, strlen(stationid)+1);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamChar (hstmt, "@timein", DB_PARAM_INPUT, timebuf, strlen(timebuf)+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		resCode = DBCreateParamChar (hstmt, "@flag", DB_PARAM_INPUT, CALI_TYPE[cali_type], strlen(CALI_TYPE[cali_type])+1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	    resCode = DBCreateParamInt (hstmt, "@timeout", DB_PARAM_OUTPUT, -1); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	
    /* Execute the statement */
    resCode = DBExecutePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;}  
	
    /* Close the statement.  Output parameters are invalid until the statement is closed */
    resCode = DBClosePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
    /* Examine the parameter values */
	resCode = DBGetParamInt (hstmt, 1, &retParam); 
	if (resCode < 0) {ShowDataBaseError();  return -1;}
	resCode = DBGetParamInt(hstmt, 5, &calitime);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 

    /* Discard the statement */
    resCode = DBDiscardSQLStatement(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 

	if (retParam)
	{
		sprintf (buf, "ִ��p_get_calitime�������%d  stationid=%s;timebuf=%s;CALI_TYPE[cali_type]=%s  ",retParam,stationid,timebuf,CALI_TYPE[cali_type]);
		MessagePopup("��ʾ", buf); 
		
		return -1;
	}
	
	if (calitime>=CALIBRATION_MAX)
	{
		switch (cali_type)
		{
			case CALI_TYPE_TX_CHECKUP:
				strcpy (buf, "����λ���˹�·���ʱ���ѳ���120���ӣ������µ��    ");
				break;
			case CALI_TYPE_TX:
				strcpy (buf, "����λ���˹�·У׼ʱ���ѳ���120���ӣ�������У׼    ");
				break;
			case CALI_TYPE_RX:
				strcpy (buf, "����λ�ն˲ο���·У׼ʱ���ѳ���120���ӣ�������У׼    ");
				break;
			case CALI_TYPE_RX_MAINLOCK:
				strcpy (buf, "����λ�ն�����·����ģʽУ׼ʱ���ѳ���120���ӣ�������У׼    ");
				break;
			case CALI_TYPE_RX_MAINFREE:
				strcpy (buf, "����λ�ն�����·��ͨģʽУ׼ʱ���ѳ���120���ӣ�������У׼    ");
				break;
			default:
				strcpy (buf, "��·У׼�����쳣 ");
				break;
		}
		
		MessagePopup("��ʾ", buf); 
	}
	
	return 0;
}

int short_circuit_check (int channel, struct struTestData *data)
{
	int  error, errorcode;
	char buf[50]="";
	
	errorcode=0;
	error = EVB5_GET_R7_SC (INST_EVB[channel], &errorcode, buf);
	if (error)
	{
		return -1;	
	}
	
	switch (errorcode)
	{
		case 0: //no error
			;
			break;
			
		case -1: //gnd
			MessagePopup("��ʾ", buf);
			data->ErrorCode=ERR_EEP_SHORT_GND;  
			return -1;
			
			break;
		
		case -2: //Vapd
			MessagePopup("��ʾ", buf);
			data->ErrorCode=ERR_EEP_SHORT_VAPD;  
			return -1;
			
			break;
			
		case -3: //Vcc
			MessagePopup("��ʾ", buf);
			data->ErrorCode=ERR_EEP_SHORT_VCC; 
			return -1;
			
			break;
			
		default:
			data->ErrorCode=ERR_EEP_NO_DEFINE; 
			return -1; 
			break;
	}
	
	return 0;
}

int DB_save_rxcali(enum enum_CALIBRATION_TYPE cali_type, int channel)
{
	char buf[2048];
	float fLoss, fLoss_min,fLoss_max,fVal;
	int resCode;
	char stationid[256];
	
	sprintf (stationid, "%s-%d",my_struProcessLOG.StationID, channel);
	
	switch (cali_type)
	{
		case CALI_TYPE_RX:

			fLoss     = sRxCal.power[channel];
			fLoss_min = sRxCal.power_min[channel];
			fLoss_max = sRxCal.power_max[channel];
			fVal 	  = sRxCal.RxCal_Power_In-sRxCal.RxCal_Power; 
			
			break;
		case CALI_TYPE_RX_OLT:

			fLoss 	  = my_struCal_OLT.Power[channel];
			fLoss_min = my_struCal_OLT.Power_Min[channel];
			fLoss_max = my_struCal_OLT.Power_Max[channel];
			fVal 	  = my_struCal_OLT.Power_In[channel]-my_struCal_OLT.Power[channel];
			
			break;	
		default:
			MessagePopup ("��ʾ", "�����ն˹�·У׼ֵʱ������ʶ��У׼����   "); 
		    return -1;
	}
	
	if (SERVER_ORACLE == my_struConfig.servertype)  	 
	{ 
//		sprintf (buf, "INSERT INTO calibration (id, pcname,loss,lossmin,lossmax,flag)        VALUES (s_id.nextval, '%s',%f,%f,%f,'%s')", stationid,fLoss,fLoss_min,fLoss_max,CALI_TYPE[cali_type]); 
		sprintf (buf, "INSERT INTO calibration (id, pcname,loss,lossmin,lossmax,sysval,flag) VALUES (s_id.nextval, '%s',%f,%f,%f,%f,'%s')", stationid,fLoss,fLoss_min,fLoss_max,fVal,CALI_TYPE[cali_type]); 
	}
	else
	{
		sprintf (buf, "INSERT INTO calibration (pcname,loss,lossmin,lossmax,flag) VALUES ('%s',%f,%f,%f,'%s')", stationid,fLoss,fLoss_min,fLoss_max,CALI_TYPE[cali_type]); 
	}
	resCode = DBImmediateSQL (hdbc, buf); 
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;} 
	
	return 0;
}

int calRxPower (int channel, struct struTestData *data, int panel)
{
	int error=0;
	
	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip)
	{
		error = CalibrateRxPower_UX3328 (channel, data, panel);  
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip)
	{
		error = CalibrateRxPower_UX3328S (channel, data, panel);
	}
	else if (CHIEFCHIP_DS4830 == my_struDBConfig_ERCompens.ChiefChip)
	{
		error = CalibrateRxPower_UX3328S (channel, data, panel);
		if(error<0)
		{
			error = CalibrateRxPower_UX3328S (channel, data, panel); 
		}
	}
	else
	{
		MessagePopup ("Error", "ChiefChip type is not know !");
	}
	
	return error;
}

int testTxDis(int channel, struct struTestData *data)
{
	int count, error;
	float TxPow_mon;
	int Table,index;
	
	double TxOffPower_max,TxOffPower_min;
	double TxOffPower_max0=-31.;
	double TxOffPower_min0=-33.; 
	double TxOffPower_max1=-31.;
	double TxOffPower_min1=-32.;

	SET_ATT_ONU(channel, -60);
	//TxDis
	if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_OFF)<0) 
	{
		return -1; 
	}
/*	
	error = EVB5_SET_VTR (INST_EVB[channel], 3.5);
	if (error) 
	{
		goto Error;
	}
	
	Delay(1.0);   
*/
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		if(index<2)
		{
			TxOffPower_max=TxOffPower_max0;
			TxOffPower_min=TxOffPower_min0;	
		}
		else
		{
		   	TxOffPower_max=TxOffPower_max1;
			TxOffPower_min=TxOffPower_min1;	  
		}
		//���ò���ͨ��
		errChk(DWDM_ONU_Select_Channelindex(channel,index));
		Delay(3);
		if (Read_AOP(channel, &data->TxOffPower[index])<0) 
		{
			return FALSE; 
		}		
		//�жϹ⹦�� 
/*		count=0;
		do
		{
			if (Read_AOP(channel, &data->TxOffPower[index])<0) 
			{
				return FALSE; 
			}
		
			if (data->TxOffPower[index]>TxOffPower_max || data->TxOffPower[index]<TxOffPower_min)
			{
				Delay (0.5);
				count++;
			}
			else 
			{
				break;
			}
		
		} while (count<3);
*/

		if (data->TxOffPower[index]>TxOffPower_max || data->TxOffPower[index]<TxOffPower_min) 
		{
			return -1;
		}
/*	
		error = MyDLL_8472_GetTxPWR (INST_EVB[channel], &TxPow_mon);
		data->RxTf = TxPow_mon;	   //���淢�˼�ع⹦��ֵ��AutoDT_Results_ATE��RxTr��**Eric.Yao**20131119//  
	
		if (data->RxTf>my_struDBConfig.TxDisPowMax) 
		{
			return -1;
		}  */
	}
	//��TxOffPower���õ�-70dBm, ʹ��ʾ����
//	my_struTestData.TxOffPower=-70;
	
	//TxEn  for AOP test
	if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) 
	{
		return -1; 
	}
	
	if(!my_struConfig.Temper_Room)	   //130520 add ��ǿ˵���¹⹦�ʻָ���
	{
		Delay(2.5);
	}
	
	error = EVB5_SET_VTR (INST_EVB[channel], 3.3);
	if (error) 
	{
		goto Error;
	}
	
	SET_ATT_ONU(channel, -10);
	
	return 0;

Error:
	error = EVB5_SET_VTR (INST_EVB[channel], 3.3);
	if (error) 
	{
		MessagePopup ("Error", "����EVB5 VTR��3.3Vʧ��");
		return -1;
	}
	
	SET_ATT_ONU(channel, -10); 
		
	return -1;
}

int caltestTxPower(int channel, struct struTestData *data) 
{
double TxPow_meter, temp; //dBm
float  TxPow_mon;   //dBm
int  error;
	
	//���ò���ͨ��
	if(DWDM_ONU_Select_Channelindex(channel,0)<0) 
	{
		return -1;
	}
	Delay(3);
	errChk(Read_AOP(channel, &TxPow_meter)); 
	
	errChk(MyDLL_8472_GetTxPWR (INST_EVB[channel], &TxPow_mon));

	temp = fabs (TxPow_meter-TxPow_mon); 
	if (temp>my_struDBConfig_Monitor.Tx_Pow_Prec) 
	{
		error = -1;
		goto Error;
	}
	
	if(!my_struConfig.DT_Tun_Reliability)  
	{
		if (TxPow_mon < my_struDBConfig.TxCalMin || TxPow_mon > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
		{
			error = -1;  
			goto Error; 
		}
	}
	
Error:
	
	return error;
}

int testFixSD(int channel, struct struTestData *data)
{
	BYTE 	SD;
	double 	temp_sda, temp_sdd; 
	
	SET_ATT_ONU(channel, -20);//�����ù⹦�ʵ�-50dBm��ȷ����SDD״̬
	
	//����SDA 
	temp_sda = my_struDBConfig.SDA+sRxCal.power[channel];
	
	SET_ATT_ONU(channel, temp_sda);	  	//����SDA 
	Delay(1.5); 
	if (Read_RX_SD(channel, 1, &SD)<0)
	{
		return -1;  
	}
	
	if(1==SD) 
	{
		return -1; 
	}
	
	//����SDD  
	temp_sdd = my_struDBConfig.SDD+sRxCal.power[channel];

	SET_ATT_ONU(channel, temp_sdd);	  	//����SDD 
	Delay(1.5);
	if (Read_RX_SD(channel, 0, &SD)<0) 
	{
		return -1;  
	}
	
	if(0==SD)
	{
		return -1;
	}

	data->SDA=my_struDBConfig.SDA;
	data->SDD=my_struDBConfig.SDD;
	data->SDHys=my_struDBConfig.SDA-my_struDBConfig.SDD;
	
	return 0;
}

/*
���Է�����
1������SDA��SDD�����Ƿ�����Ҫ�󣬺ϸ�������ԣ����ϸ��˳�����
2��
*/
int testDetailSD(int channel, struct struTestData *data)
{
	int   error=0;
	float step_array[3]={2,0.5,0.1};
	double temp_sda, temp_sdd;
	BYTE  SD;
	int   step;
	
	//set low rx power
	SET_ATT_ONU(channel, -50);

	//����SDA 
	temp_sda = my_struDBConfig.SDA+sRxCal.power[channel];
	Set_Att_SD(channel, temp_sda);	  	//����SDA 
	
	if (Read_RX_SD(channel, 1, &SD)<0) 
	{
		return -1;   
	}
	
	if(0==SD) 
	{
		return -1;
	}
	
	//����SDD 
	temp_sdd = my_struDBConfig.SDD+sRxCal.power[channel];
	Set_Att_SD(channel, temp_sdd);	  	//����SDD 
	
	if (Read_RX_SD(channel, 0, &SD)<0)
	{
		return -1;
	}
	if(1==SD)
	{	
		return -1;
	}
	
	//test sda
	step = 0;
	temp_sda = my_struDBConfig.SDD+sRxCal.power[channel];   
	do
	{
		do
		{
			temp_sda += step_array[step];
			
			Set_Att_SD(channel, temp_sda);
		//	Delay(2.5);
			Read_RX_SD(channel, -1, &SD);  
			if (SD==1) 
			{
				break; 		//�õ���SDA��,��������ѭ��
			}
			
		} while (-15>temp_sda);		//���û�е���-15dBm����ѭ��
			
		if (-15<=temp_sda)    		//����-15dBm���쳣�˳�
		{
			error = -1;
			goto Error;
		}
		if (step==2)          		//�õ�����С��step
		{
			data->SDA = temp_sda-sRxCal.power[channel]; 
			break;
		}
		
		//�ָ�SDD״̬
		Set_Att_SD(channel, my_struDBConfig.SDD+sRxCal.power[channel]);
	
		temp_sda -= step_array[step];  
		Set_Att_SD(channel, temp_sdd);  
		Delay(2.5);        
		Read_RX_SD(channel, -1, &SD);  
		if (SD==1) 
		{
			temp_sda -= step_array[step];  
			Set_Att_SD(channel, temp_sdd); 
		//	Delay(2.5);        
			Read_RX_SD(channel, -1, &SD);  
			if (SD==1)
			{
				temp_sda -= step_array[step];  
				SET_ATT_ONU(channel, temp_sdd); 
			}
		}
	
		step++;
		
	}while (-15>temp_sda);
	
	//test sdd
	step = 0;
	temp_sdd = temp_sda;  
//	temp_sda = my_struDBConfig.SDA+my_struRxCal.RxCal_Power;    
	do
	{
		do
		{
			temp_sdd -= step_array[step];
			
			Set_Att_SD(channel, temp_sdd);
		//	Delay(2.0);
			Read_RX_SD(channel, -1, &SD);  
			if (0==SD) 
			{
				break; 		//�õ���SDD��,��������ѭ��
			}
			
		} while (-45<temp_sdd);		//���û�е���-45dBm����ѭ��
			
		if (-45>=temp_sdd)    		//С��-45dBm���쳣�˳�
		{
			error = -2;
			goto Error;
		}
		if (step==2)          		//�õ�����С��step
		{
			data->SDD = temp_sdd-sRxCal.power[channel]; 
			break;
		}
		
		//�ָ�SDA״̬
		Set_Att_SD(channel, my_struDBConfig.SDA+sRxCal.power[channel]);
	
		temp_sdd += step_array[step];  
		Set_Att_SD(channel, temp_sdd);  
	//	Delay(2.0);      
		Read_RX_SD(channel, -1, &SD);  
		if (0==SD) 
		{
			temp_sdd += step_array[step];  
			Set_Att_SD(channel, temp_sdd);  
		//	Delay(2.0);      
			Read_RX_SD(channel, -1, &SD);  
			if (0==SD) 
			{
				temp_sdd += step_array[step];  
				Set_Att_SD(channel, temp_sdd);   
			}
		}
	
		step++;
		
	}while (-45<temp_sdd);
	
	//�ж�sda�Ƿ�ϸ�
//	if ((data->SDA>my_struDBConfig.SDA) || (my_struTestData.SDA<(my_struDBConfig.SDA-3)))
//	{
//		error = -3;
//		goto Error;
//	}
	
	//�ж�sdd�Ƿ�ϸ�
//	if ((data->SDD<my_struDBConfig.SDD) || (my_struTestData.SDD>(my_struDBConfig.SDD+3)))
//	{
//		error = -4;
//		goto Error;
//	}
	
	//�ж�sda�Ƿ�ϸ�
	if (data->SDA>my_struDBConfig.SDA)
	{
		error = -3;
		goto Error;
	}
	
	//�ж�sdd�Ƿ�ϸ�
	if (data->SDD<my_struDBConfig.SDD)
	{
		error = -4;
		goto Error;
	}
	
	if (data->SDA-data->SDD<my_struDBConfig.SDHysMin)
	{
		error = -5;
		goto Error;             
	}
	
	if (data->SDA-data->SDD>my_struDBConfig.SDHysMax)
	{
		error = -6;
		goto Error;             
	}   
	
Error:	
	return error;
}
  
int testER(int channel, struct struTestData *data,struct struProcessLOG *prolog)
{
	char    Inf[1024];
	int 	error,Table;
	double	temp_ER;

	char buf[256]; 
	
	int     index;
	int		count;

	if (my_struConfig.DT_QATest)
	{		
		if (EVB5_SET_BEN_Mode(INST_EVB[channel], EVB5_BEN_MODE_CLOCK)<0) 
		{
			return -1;
		}
		Delay(0.3);
		if (EVB5_SET_BEN_Mode(INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
		{
			return -1; 
		}
		if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) 
		{
			return -1; 
		}   
	}
	
	for (index=0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ò���ͨ��
		errChk(DWDM_ONU_Select_Channelindex(channel,index));
		
		Delay(3);
		if (Read_ER(channel, &data->ER[index])<0) 
		{
			return -1;   
		}

		//�ж������
		if (data->ER[index]>my_struDBConfig.TxErMax || data->ER[index]<my_struDBConfig.TxErMin)
		{	 
			//AOP ReTuning,Tuning Er
			if((my_struConfig.Temper_High || my_struConfig.Temper_Low) && my_struConfig.DT_Test)
			{
			/*	DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index]);
				
				count=0;   
				do
				{   
					if( (data->ER[index] > (my_struDBConfig.TxErMin + 0.5)) && (data->AOP[index] < (my_struDBConfig.TxErMin + 1.0)) ) 
					{
						break;
					}
						
					if(data->ER[index] < (my_struDBConfig.TxErMin+ 0.5)) 
					{
						data->ucRm[index]= data->ucRm[index]+3;
					}
					else					 
					{
						data->ucRm[index]= data->ucRm[index]-3;  
					}
					
					DWDM_ONU_DS4830_Write_MOD_LUK6(INST_EVB[channel], index, data->ucRm[index]); 
					
					Delay(0.5);					
					if (Read_ER(channel, &data->ER[index])<0) 
					{
						return -1;
					}
					count++;
				}while (count<10);
				if(count>=10)
				{
					return -1; 
				}
				DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel]); 
				*/
				return -1;  
			} 
			else
			{
				return -1; 
			}
		}
	}
	
	return 0;
	
Error:
	
	return -1;
}

int Read_ER(int channel, double *ER)
{
	double Er;

    *ER=0;

	switch (INSTR[channel].DCA_TYPE)
	{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_Set_Clear(inst_CSA8000)) 
			{
				return -1; 
			}
			Delay(1);
			if (!CSA8000_GET_Er(inst_CSA8000, &Er))
			{
				return -1;
			}
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_Set_Clear(inst_Ag86100))
			{
				return -1; 
			}
			Delay(2);
			if (!Ag86100_GET_Er(inst_Ag86100, &Er))
			{
				return -1;
			}
			break;
		default:
			return -1;
	}
	
	*ER=Er*my_struTxCal.TxCal_Er_Slope+my_struTxCal.TxCal_Er_Offset; 
	
	return 0;
}

int testOEye(int channel, struct struTestData *data)
{
int count; 
int error;

	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)
	{
	   if (!Ag86100_Set_AutoScale(inst_Ag86100)) 
	   {
		   return FALSE;  
	   }
	}

	//������ͼ���ԣ�����ag86100��ֻ������ĸ�������
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)
	{
	   if (!Ag86100_SET_O_EYE(inst_Ag86100)) 
	   {
		   return -1;
	   }
	   Delay(2);
	}
	
	count=0;
	do
	{
		errChk(READ_DCA_PJ(INSTR[channel].DCA_TYPE, &data->TxPPj)); 
		errChk(READ_DCA_RJ(INSTR[channel].DCA_TYPE, &data->TxRSj)); 
		errChk(READ_DCA_Rise(INSTR[channel].DCA_TYPE, &data->TxRise)); 
		errChk(READ_DCA_Fall(INSTR[channel].DCA_TYPE, &data->TxFall)); 
		
		data->TxTj=data->TxPPj+data->TxRSj*14;
	
		if (data->TxPPj<=my_struDBConfig.TxPPjMax && data->TxTj<=my_struDBConfig.TxTjMax && data->TxRise<=my_struDBConfig.TxRiseMax && data->TxFall<=my_struDBConfig.TxFallMax) break;
		
		Delay(0.5);
		count++; 
	}while(count<3); 
	
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)
	{
	   if (!Ag86100_SET_O_ER(inst_Ag86100, my_struAg86100.X_Scale, my_struAg86100.X_Position, my_struAg86100.Y_Scale, my_struAg86100.Y_Offset,my_struAg86100.Channel_O)) 
	   {
		   return -1;  
	   }
	}	
	
	//���ǵ�86100ֻ����Ag86100_SET_O_ER��������ԣ���ΪAg86100ֻ������ĸ�������
	errChk(READ_DCA_PCTCROss(INSTR[channel].DCA_TYPE, & data->TxPCTCROss));  
	
	data->TxPPj      = data->TxPPj>1000  ? 1000:data->TxPPj;
	data->TxTj       = data->TxTj>1000   ? 1000:data->TxTj;
	data->TxRise     = data->TxRise>1000 ? 1000:data->TxRise;
	data->TxFall     = data->TxFall>1000 ? 1000:data->TxFall;
	data->TxPCTCROss = data->TxPCTCROss>1000 ? 1000:data->TxPCTCROss;    

	//�ж϶���
	if (data->TxPPj>my_struDBConfig.TxPPjMax || data->TxTj>my_struDBConfig.TxTjMax) 
	{
		return -1;
	}
	//�ж������½�ʱ��
	if (data->TxRise>my_struDBConfig.TxRiseMax || data->TxFall>my_struDBConfig.TxFallMax) 
	{
		return -1;
	}
	
Error:
	
	return error;   
}

int READ_DCA_PJ(int DCAType, double *PJ)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_GET_O_PPj(inst_CSA8000, PJ)) return -1;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_GET_O_PPj(inst_Ag86100, PJ)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int READ_DCA_RJ(int DCAType, double *RJ)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_GET_O_RSj(inst_CSA8000, RJ)) return -1;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_GET_O_RSj(inst_Ag86100, RJ)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int READ_DCA_Rise(int DCAType, double *Rise)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_GET_O_Rise(inst_CSA8000, Rise)) return -1;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_GET_O_Rise(inst_Ag86100, Rise)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int READ_DCA_Fall(int DCAType, double *Fall)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_GET_O_Fall(inst_CSA8000, Fall)) return -1;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_GET_O_Fall(inst_Ag86100, Fall)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int READ_DCA_PCTCROss(int DCAType, double *PCTCROss)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_GET_O_PCTCROss(inst_CSA8000, PCTCROss)) return -1;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_GET_O_CROSsing(inst_Ag86100, PCTCROss)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int READ_DCA_MaskHits(int DCAType, int WaveForms)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_SET_MaskMargin(inst_CSA8000, 15, "OC48")) return -1;
			if (!CSA8000_GET_MaskCount(inst_CSA8000, 1000)) return -1;
			
			if (!CSA8000_Set_O(inst_CSA8000, my_struCSA8000.Channel_O, my_struCSA8000.X_Scale, my_struCSA8000.X_Position, my_struCSA8000.Y_Scale, my_struCSA8000.Y_Position, my_struCSA8000.Y_Offset, my_struCSA8000.Wavelength, my_struCSA8000.Filter, my_struCSA8000.MaskName)) 
			{
				MessagePopup("Error", "CSA8000 Config for Optics Channel error!");
				return -1; 
			} 
			
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_SET_MaskMargin(inst_Ag86100, 15, "\"01xGbEthernet.msk\"")) return -1;
			if (!Ag86100_GET_MaskHits(inst_Ag86100, 200)) return -1;
			break;
		default:
			return -1;
		}
	return 0;
}

int testSpectrum(int channel, struct struTestData *data)
{
	int count, error;
	int index;
	double tmp_PeakWL;
	
	for (index=0;  index<4;index++)        
	{
		error=0;
		count=0;
		do
		{
			if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AG8614X) 
			{
				if (!Ag86142_Read(LASER_TYPE_DFB, inst_Ag86142, &tmp_PeakWL, &data->Sigma, &data->Bandwidth, &data->SMSR)) return -1;
			}
			else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AQ633X)
			{
				error = AQ633X_Read (Inst_AQ633X, LASER_TYPE_DFB, 30.0, &tmp_PeakWL, &data->Sigma, &data->Bandwidth, &data->SMSR);
				if (error) {MessagePopup("Error", "Read spectrum aq633x error"); return -1;}
			}
			else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_MS9740)
			{
				error = MS9740_Read (Inst_MS9740, LASER_TYPE_DFB, 1490.0, 30.0, &tmp_PeakWL, &data->Sigma, &data->Bandwidth, &data->SMSR);
				if (error) {MessagePopup("Error", "Read spectrum aq633x error"); return -1;}
			}
			else 
			{
				MessagePopup("Error", "Can not find this spectrum type");
				return -1;
			} 
		
			//------------�жϷ�Χ---------------//
			//forDFB	
			if (data->Bandwidth<=my_struDBConfig.BandWidth && data->SMSR>=my_struDBConfig.SMSR)
			{
				break;
			}
			//------------�жϷ�Χ---------------// 
			count++;
		} while(count<3);	
	
		if(count>=3) 
		{
			return -1; 	
		}
	}
	
Error:
	
	return error;
}

int testWavelength(int channel, struct struTestData *data)
{
	//�����ĸ�����
	//ÿ��������Ҫ����1%-99%��ռ�ձ������µĲ���
	int count0,count, error,Table;
	int index,LookFlag;
	float delta=0;
	float delta0=0;
	float delta1=0;
	double Center_Wavelegnth;
	double tempval;
	char tempbuf[256];
	
	double Wavelegnth,WL_Delata;

	data->WL_ReTun_FLAG=FALSE; 
	
	delta= my_struDBConfig.WavelengthDelta;
	for (index=3;index>=0;index-- )
	{	 
		//���ò���ͨ��
		Center_Wavelegnth=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0;
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0) 
		{
			return -1;
		}
		
		Delay(1); 
		error=RecordMonitor(channel,index, data);
		
		//����ռ�ձ�100%
		errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
		//Set TxDisable
		if(EVB5_SET_BEN_Level (INST_EVB[channel], 0),0)
		{
			return -1;
		}   
		
		error=0;   
		count = 0;
		WL_Delata=1;
		do
		{
			//������   
			errChk (Read_Wavelength(channel, &data->PeakWL_DutyRatio99[index])); 
			if(0==count)
			{
				Wavelegnth  = data->PeakWL_DutyRatio99[index];
			}
			if(count>0)
			{
				WL_Delata	= fabs(Wavelegnth - data->PeakWL_DutyRatio99[index]);	
				Wavelegnth  = data->PeakWL_DutyRatio99[index];   
			}
			Delay(1.0);
			count++;
		}while(count<10 && WL_Delata> 0.005);   
		
		error=0;  		
		//����ռ�ձ�10%  
		//100k hz
		errChk(EVB5_SET_CLOCK (INST_EVB[channel], 0.008));
		errChk(EVB5_SET_BEN_DELAY(INST_EVB[channel], 10, 10, 60));   
		//set ben to clock mode
		errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_N1N2N3));
		
		count0=0;
 		do
		{
			//set ben to LEVEL mode  
			if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
			{
				return -1; 
			}
			Delay(0.2);
			//set ben to LEVEL mode  
			if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_N1N2N3)<0)
			{
				return -1;
			}																												   
			
			error=0;
			count = 0;
			WL_Delata=1;
			do
			{
				//������   
				errChk (Read_Wavelength(channel, &data->PeakWL_DutyRatio10[index])); 
				if(0==count)
				{
					Wavelegnth  = data->PeakWL_DutyRatio10[index];
				}
				if(count>0)
				{
					WL_Delata	= fabs(Wavelegnth - data->PeakWL_DutyRatio10[index]);	
					Wavelegnth  = data->PeakWL_DutyRatio10[index];   
				}
				Delay(1.0);
				count++;
			}while(count<10 && WL_Delata> 0.005); 
			count0++;
		}while(fabs(data->PeakWL_DutyRatio10[index]-(Center_Wavelegnth+delta))>1.0 && count0<3);
		if((data->PeakWL_DutyRatio99[index]>(Center_Wavelegnth+delta)) || (data->PeakWL_DutyRatio99[index]<(Center_Wavelegnth-delta)))		  			//100%�����ж�
		{
			delta1= data->PeakWL_DutyRatio99[index]-Center_Wavelegnth;
			sprintf(tempbuf,"Ch%d Delat=%f; 100%%-����ֵ��%f-%f=%f>%f",(index+1),delta1,data->PeakWL_DutyRatio99[index],Center_Wavelegnth,delta1,delta); 
			MessagePopup("Error", tempbuf); 
			error=-1;
			goto Error; 	
		}
		else if((data->PeakWL_DutyRatio10[index]>(Center_Wavelegnth+delta)) || (data->PeakWL_DutyRatio10[index]<(Center_Wavelegnth-delta+0.025)))		//10%�����ж�    
		{
			delta0= data->PeakWL_DutyRatio99[index] - data->PeakWL_DutyRatio10[index];  
			if(delta0<0.135 && !my_struConfig.DT_QATest && !data->WL_ReTun_FLAG)
			{
				data->WL_ReTun_FLAG=TRUE;
				//���벨��΢��
				if(tuningWavelength_FineTuning(channel,data,index)<0)
				{
					MessagePopup("Error", "����΢��ʧ�ܣ�"); 
					error=-1;
					goto Error;
				}
				index++;
			}
			else if(delta0<0.135 && !my_struConfig.DT_QATest && data->WL_ReTun_FLAG)   
			{
				;
			}
			else
			{ 
				sprintf(tempbuf,"Ch%d ����=%f; 10%%����<����ֵ-55pm; %f<%f=%f-%f",(index+1),delta0,data->PeakWL_DutyRatio10[index],Center_Wavelegnth-delta+0.025,Center_Wavelegnth,delta-0.025); 
				MessagePopup("Error", tempbuf); 
				error=-1;
				goto Error; 
			}
			
		}  
		else
		{
			;
		}
			
			
		
	/*	if( 
			(data->PeakWL_DutyRatio99[index]>(Center_Wavelegnth+delta)) || (data->PeakWL_DutyRatio99[index]<(Center_Wavelegnth-delta))
		  ||(data->PeakWL_DutyRatio10[index]>(Center_Wavelegnth+delta)) || (data->PeakWL_DutyRatio10[index]<(Center_Wavelegnth-delta+0.025))  
		  )
		{
			delta1=data->PeakWL_DutyRatio99[index]-data->PeakWL_DutyRatio10[index]; 
			if((data->PeakWL_DutyRatio99[index]>(Center_Wavelegnth+delta))|| (data->PeakWL_DutyRatio99[index]<(Center_Wavelegnth-delta)))
			{
				delta0= data->PeakWL_DutyRatio99[index]-Center_Wavelegnth;
				sprintf(tempbuf,"Ch%d Delat=%f; 99-����ֵ��%f-%f=%f>%f",(index+1),delta1,data->PeakWL_DutyRatio99[index],Center_Wavelegnth,delta0,delta); 
				
			}
			else 
			{
				delta0= data->PeakWL_DutyRatio99[index] - data->PeakWL_DutyRatio10[index];
			//	sprintf(tempbuf,"Ch%d Delat=%f; ����ֵ-10:%f-%f=%f>%f-0.025",(index+1),delta1,Center_Wavelegnth,data->PeakWL_DutyRatio10[index],delta0,delta-0.025); 
				sprintf(tempbuf,"Ch%d 10%%����<����ֵ-55pm; %f<%f=%f-%f,%f",(index+1),data->PeakWL_DutyRatio10[index],Center_Wavelegnth-delta+0.025,Center_Wavelegnth,delta-0.025,delta0);  
			}
			MessagePopup("Error", tempbuf); 
			error=-1;
			goto Error; 
		}
	*/
	}
Error: 	
	//set ben to LEVEL mode  
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL); 
	//Set TxDisable
	if(EVB5_SET_BEN_Level (INST_EVB[channel], 0),0)
	{
		return -1;
	}
	
	return error;
}

int testWavelength_Tun(int channel, struct struTestData *data)				//���Թ���ֻ����10%�����Ƿ���ָ���ڣ�100%�ڳ��²��Թ�����ԣ�2017-02-20 wenyao.xi;
{
	//�����ĸ�����
	//ÿ��������Ҫ����1%-99%��ռ�ձ������µĲ���
	int count0,count, error,Table;
	int index,LookFlag;
	float delta=0;
	float delta0=0;
	float delta1=0;
	double Center_Wavelegnth;
	double tempval;
	char tempbuf[256];
	float dt=0.2;
	float dt1=0.2;
	
	double Wavelegnth,WL_Delata;

	delta= my_struDBConfig.WavelengthDelta;
	
	//Set TxEnable
	if(EVB5_SET_BEN_Level (INST_EVB[channel], 0)<0)
	{
		return -1;
	}
	Delay(dt);
	//100k hz
	if(EVB5_SET_CLOCK (INST_EVB[channel], 0.008)<0)
	{
		return -1;
	}
	Delay(dt);
	if(EVB5_SET_BEN_DELAY(INST_EVB[channel], 10, 10, 60)<0)
	{
		return -1;
	}	   

	//����ռ�ձ�10% 
	//set ben to clock mode
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_N1N2N3));  
	  	
	
	for (index=3;index>=0;index-- )
	{	 
		//���ò���ͨ��
		Center_Wavelegnth=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0;
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0) 
		{
			return -1;
		}  
		
		Delay(1);
		error=RecordMonitor(channel,index, data);
		
		count0=0;
		do
		{
			 //set ben to LEVEL mode  
			if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
			{
				return -1; 
			}
			Delay(0.2);
			//set ben to LEVEL mode  
			if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_N1N2N3)<0)
			{
				return -1;
			}
			
			error=0;   
			count = 0;
			WL_Delata=1;
			do
			{																										  
				//������   
				errChk (Read_Wavelength(channel, &data->PeakWL_DutyRatio10[index])); 
				if(0==count)
				{
					Wavelegnth  = data->PeakWL_DutyRatio10[index];
				}
				if(count>0)
				{
					WL_Delata	= fabs(Wavelegnth - data->PeakWL_DutyRatio10[index]);	
					Wavelegnth  = data->PeakWL_DutyRatio10[index];   
				}
				Delay(1.0);
				count++;
			}while(count<10 && WL_Delata> 0.005); 
			count0++;
		}while(fabs(data->PeakWL_DutyRatio10[index]-(Center_Wavelegnth+delta))>1.0 && count0<3);
		
		if((data->PeakWL_DutyRatio10[index]>(Center_Wavelegnth+delta)) || (data->PeakWL_DutyRatio10[index]<(Center_Wavelegnth-delta+0.025)))
		{
			if(data->PeakWL_DutyRatio10[index]>(Center_Wavelegnth+delta))
			{
				delta0= data->PeakWL_DutyRatio10[index] - Center_Wavelegnth;   
				sprintf(tempbuf,"Ch%d 10%%����>����ֵ+80pm; %f>%f=%f+%f,%f",(index+1),data->PeakWL_DutyRatio10[index],Center_Wavelegnth+delta,Center_Wavelegnth,delta,delta0); 
			}
			else
			{
				delta0= Center_Wavelegnth - data->PeakWL_DutyRatio10[index];   
				sprintf(tempbuf,"Ch%d 10%%����<����ֵ-55pm; %f<%f=%f-%f,%f",(index+1),data->PeakWL_DutyRatio10[index],Center_Wavelegnth-delta+0.025,Center_Wavelegnth,delta-0.025,delta0); 
			}	 
			MessagePopup("Error", tempbuf); 
			error=-1;
			goto Error; 
		}
	}
Error: 	
	//set ben to LEVEL mode  
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL); 
	
	return error;
}

int testAOP(int channel, struct struTestData *data)
{
	int error=0;
	int Table,index;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_AOP,temp_AOPavg,temp_AOPmin,temp_AOPmax;
	
	int count;   	
	
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0) 
		{
			return -1;
		}
		Delay(3);
		count=0;  
		do
		{
			if (Read_AOP(channel, &data->AOP[index])<0)
			{
				return -1;     
			}

			if (data->AOP[index]>my_struDBConfig.TxPowMax || data->AOP[index]<my_struDBConfig.TxPowMin)
			{
				Delay (0.5);
				count++;
			}
			else 
			{
				break;
			}
		
		} while (count<3); 
	
		//�жϹ⹦��
		if (data->AOP[index]>my_struDBConfig.TxPowMax || data->AOP[index]<my_struDBConfig.TxPowMin) 
		{   
			if (my_struConfig.Temper_Room) 
			{

				if (data->AOP[index]>my_struDBConfig.TxPowMax)						 /***AOP�����޴������-430**Eric.Yao***/
				{
				//	MessagePopup ("��ʾ", "AOP���ڲ���ָ�꣬�����½��г���AOP����");
					data->ErrorCode=ERR_TES_AOP_MAX;
				}
				else
		 		{
					data->ErrorCode=ERR_TES_AOP;
				}
				return -1; 
			}
			else
			{
				//AOP<Min && AOP > Min-0.8dBm,Tuning
		/*		if( (data->AOP[index]<my_struDBConfig.TxPowMin) && (data->AOP[index]>(my_struDBConfig.TxPowMin-2))  && !my_struConfig.DT_QATest)
				{
					DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index]);
					data->RetuningAopFlag=TRUE;
					count=0;   
					do
					{   
						if( (data->AOP[index] > (my_struDBConfig.TxPowMin + 0.3)) && (data->AOP[index] < (my_struDBConfig.TxPowMin + 1.5)) ) 
						{
							break;
						}
						
						if(data->AOP[index] < (my_struDBConfig.TxPowMin+ 0.5)) 
						{
							data->ucRb[index]= data->ucRb[index]+5;
						}
						else					 
						{
							data->ucRb[index]= data->ucRb[index]-5;  
						}
						
						DWDM_ONU_DS4830_Write_APC_LUK6(INST_EVB[channel], index, data->ucRb[index]); 
						
						Delay(0.5);					
						if (Read_AOP(channel, &data->AOP[index])<0)
						{
							return -1;
						}
						count++;
					}while (count<20);
					if(count>=20)
					{
						return -1; 
					}
					DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel]); 
				}
				else
		 		{
					return -1; 
				}
				*/
				return -1;
			}
		}	 

	}
	return 0;

}
int testAOP_Ex(int channel, struct struTestData *data)
{
	int error=0;
	int Table,index;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_AOP,temp_AOPavg,temp_AOPmin,temp_AOPmax;
	
	int count;  
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0) 
		{
			return -1;
		}
		Delay(3);
		if (my_struConfig.DT_QATest)
		{		
			if (EVB5_SET_BEN_Mode(INST_EVB[channel], EVB5_BEN_MODE_CLOCK)<0)
			{
				return -1;     
			}
			Delay(0.3);
			if (EVB5_SET_BEN_Mode(INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0) 
			{
				return -1;     
			}
			if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) 
			{
				return -1;     
			}
			Delay(2);  
		}
	
		Delay(1);
		count=0;  
		do
		{
			if (Read_AOP(channel, &data->AOP[index])<0)
			{
				return -1;     
			}
		
			if (data->AOP[index]<my_struDBConfig.TxPowMin)
			{
				//MessagePopup ("��ʾ", "�⹦�ʲ���ƫС�������²�ι��˽��в���    ");
				return -1;
			}
		
			if (data->AOP[index]>my_struDBConfig.TxPowMax || data->AOP[index]<my_struDBConfig.TxPowMin)
			{
				Delay (0.5);
				count++;
			}
			else 
			{
				break;
			}
		
		} while (count<3); 
	
		//�жϹ⹦��
		if (data->AOP[index]>my_struDBConfig.TxPowMax || data->AOP[index]<my_struDBConfig.TxPowMin) 
		{   
			if (my_struConfig.Temper_Room) 
			{

				if (data->AOP[index]>my_struDBConfig.TxPowMax)						 /***AOP�����޴������-430**Eric.Yao***/
				{
				//	MessagePopup ("��ʾ", "AOP���ڲ���ָ�꣬�����½��г���AOP����");
					data->ErrorCode=ERR_TES_AOP_MAX;
					return -1;
				}
				else
		 		{
					data->ErrorCode=ERR_TES_AOP;
				}
			}
			else
			{
				//AOP<Min && AOP > Min-0.8dBm,Tuning
				if( (data->AOP[index]<my_struDBConfig.TxPowMin) && (data->AOP[index]>(my_struDBConfig.TxPowMin-0.5)) && !my_struConfig.DT_QATest)
				{
					DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index]);
					
					count=0;   
					do
					{   
						if( (data->AOP[index] > (my_struDBConfig.TxPowMin + 0.2)) && (data->AOP[index] < (my_struDBConfig.TxPowMin + 0.4)) ) 
						{
							break;
						}
						
						if(data->AOP[index] < my_struDBConfig.TxPowMin) 
						{
							data->ucRb[index]= data->ucRb[index]+2;
						}
						else					 
						{
							data->ucRb[index]= data->ucRb[index]-2;  
						}
						
						DWDM_ONU_DS4830_Write_APC_LUK6(INST_EVB[channel], index, data->ucRb[index]); 
						
						Delay(1.5);					
						if (Read_AOP(channel, &temp_AOP)<0)
						{
							return -1;
						}
						count++;
					}while (count<10);
					if(count>=10)
					{
						return -1; 
					}
					DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel]); 
				}
				else
				{
					return -1;
				}
			}
		
			return -1;   
		}	 

	}
	return 0;

}
int testAOP_AginFront(int channel, struct struTestData *data)
{
	int error=0;
	int Table,index;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_AOP,temp_AOPavg,temp_AOPmin,temp_AOPmax;
	
	Delay(2);
	if (Read_AOP(channel, &data->AOP[0])<0)
	{
		return -1;     
	}

	if (data->AOP[0]>my_struDBConfig.TxPowMax || data->AOP[0]<my_struDBConfig.TxPowMin)
	{
		return -1;     
	}
	
	return 0;

}

int testAOP_AginBlack(int channel, struct struTestData *data)
{
	int error=0;
	int Table,index;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_AOP,temp_AOPavg,temp_AOPmin,temp_AOPmax;
	char buf[256];
	
	Delay(1);
	if (Read_AOP(channel, &data->AOP[0])<0)
	{
		return -1;     
	}
	
	if (data->AOP[0]>my_struDBConfig.TxPowMax || data->AOP[0]<my_struDBConfig.TxPowMin)
	{
		return -1;     
	}
		
	//�жϹ⹦��
	DB_Get_AginFront_TxPower(data->OpticsSN,&temp_AOP);
	if((temp_AOP-data->AOP[0] - my_struConfig.AOP_AginDelta) >0.)
	{
		sprintf(buf,"�ϻ���AOP����%.2fdBm >%.2fdBm",(temp_AOP-data->AOP[0]),my_struConfig.AOP_AginDelta);
		MessagePopup("��ʾ",buf);
		return -1; 
	}
	return 0;

}

int testWaveLength_AginFront(int channel, struct struTestData *data)
{
	int error=0;
	int Table,count;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_AOP,temp_AOPavg,temp_AOPmin,temp_AOPmax;
	float WL_Delata,Wavelegnth;   
	
	//�л���·
	if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
	{
		if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
		{
			return -1;
		}
	}
	
	error=0;   
	count = 0;
	WL_Delata=1;
	do
	{
		//������   
		if(Read_Wavelength(channel, &data->PeakWL_DutyRatio99[0])<0)
		{
			return -1;
		}
		if(0==count)
		{
			Wavelegnth  = data->PeakWL_DutyRatio99[0];
		}
		if(count>0)
		{
			WL_Delata	= fabs(Wavelegnth - data->PeakWL_DutyRatio99[0]);	
			Wavelegnth  = data->PeakWL_DutyRatio99[0];   
		}
		Delay(1.0);
		count++;
	}while(count<10 && WL_Delata> 0.005);  
	
	if(count>=10)
	{
		return -1;
	}
	if(100==data->PeakWL_DutyRatio99[0])
	{
		return -1;
	}
	return 0;

}

int testWaveLength_AginBlack(int channel, struct struTestData *data)
{
	int error=0;
	int Table,count;
	int TemPIDVal[4];
	int DAC0,DACmin,DACmax;
	double temp_WL;
	char buf[256];
	float WL_Delata,Wavelegnth;
	
	//�л���·
	if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
	{
		if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
		{
			return -1;
		}
	}
		
	count = 0;
	WL_Delata=1;
	do
	{
		//������   
		if (Read_Wavelength(channel, &data->PeakWL_DutyRatio99[0])<0)
		{
			return -1;
		}
		if(0==count)
		{
			Wavelegnth  = data->PeakWL_DutyRatio99[0];
		}
		if(count>0)
		{
			WL_Delata	= fabs(Wavelegnth - data->PeakWL_DutyRatio99[0]);	
			Wavelegnth  = data->PeakWL_DutyRatio99[0];   
		}
		Delay(1.0);
		count++;
	}while(count<10 && WL_Delata> 0.005); 
	
	if(count>=10)
	{
		return -1;
	}
		
	//�жϹ⹦��
	DB_Get_AginFront_WaveLength(data->OpticsSN,&temp_WL);
	if((temp_WL-data->PeakWL_DutyRatio99[0] - my_struConfig.WL_AginDelta) >0.)
	{
		sprintf(buf,"�ϻ��󲨳�Ư��%.3fdBm >%.3fnm",(temp_WL-data->PeakWL_DutyRatio99[0]),my_struConfig.WL_AginDelta);
		MessagePopup("��ʾ",buf);
		return -1; 
	}
	return 0;

}

int testTE(int channel, struct struTestData *data)
{
	float TE, TestAop, TunAop[4];
	int error,index;
	
    for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0) 
		{
			return -1;
		}
		Delay(3);
		TestAop = data->AOP[index];
		error = DB_Get_Tuning_AOP(data->OpticsSN, TunAop);
		if(error)
		{
			return -1;
		}

		TE = fabs(TunAop[index] - TestAop);
		data->TE = TE;

		if((double)TE < my_struDBConfig.TEMin|| (double)TE > my_struDBConfig.TEMax)
		{
			return -1;
		}
	 }
	return 0;
}

int DB_Get_Tuning_AOP (char *OpticsSN, float *aop)
{
	float TempAOP,TempAOP1,TempAOP2,TempAOP3;
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	
	sprintf (buf, "select TXAOP,TXAOP1,TXAOP2,TXAOP3 from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='TUNING' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColFloat (hstmt, 1, &TempAOP, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 2, &TempAOP1, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 3, &TempAOP2, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 4, &TempAOP3, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
																			   
	if (num!=1) {MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN��ǰһ�����ݣ�    "); return -2;} 
	
	aop[0] = TempAOP;
	aop[1] = TempAOP1; 
	aop[2] = TempAOP2; 
	aop[3] = TempAOP3; 
	return 0; 
}

int DB_get_room_aop(char SN[], float *val)
{
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	
	/* Prepare a statement which calls the stored procedure */
    resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_STORED_PROC);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
    hstmt = DBPrepareSQL (hdbc, "p_get_room_aop");
    if (hstmt <= 0) {ShowDataBaseError();  return -1;} 
	 
	resCode = DBSetAttributeDefault(hdbc, ATTR_DB_COMMAND_TYPE, DB_COMMAND_UNKNOWN);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	 
	if (SERVER_ORACLE == my_struConfig.servertype)
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "V_return", DB_PARAM_OUTPUT, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
		
		ParamLen =  (0==strlen(SN)) ? 0:strlen(SN)+1;     
	    resCode = DBCreateParamChar (hstmt, "V_sn", DB_PARAM_INPUT, SN, ParamLen);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		
	    resCode = DBCreateParamFloat (hstmt, "V_aop", DB_PARAM_OUTPUT, 0.); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	else
	{
	    /* Set the input parameter */
		resCode = DBCreateParamInt (hstmt, "", DB_PARAM_RETURN_VALUE, -1);
		if (resCode < 0) {ShowDataBaseError();  return -1;}
		
		ParamLen =  (0==strlen(SN)) ? 0:strlen(SN)+1;     
	    resCode = DBCreateParamChar (hstmt, "@sn", DB_PARAM_INPUT, SN, ParamLen);   //����Ҫ��ȡʵ�ʵ��ַ�����С��������������ַ��ռ䣬����ĳ�������ַ������Կո���
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
		
	    resCode = DBCreateParamFloat (hstmt, "@aop", DB_PARAM_OUTPUT,0); 
		if (resCode < 0) {ShowDataBaseError();  return -1;} 
	}
	
    /* Execute the statement */
    resCode = DBExecutePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;}  
	
    /* Close the statement.  Output parameters are invalid until the statement is closed */
    resCode = DBClosePreparedSQL(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
    /* Examine the parameter values */
	resCode = DBGetParamInt (hstmt, 1, &retParam); 
	if (resCode < 0) {ShowDataBaseError();  return -1;}
	resCode = DBGetParamFloat(hstmt, 3, val);
	if (resCode < 0) {ShowDataBaseError();  return -1;}
	
    /* Discard the statement */
    resCode = DBDiscardSQLStatement(hstmt);
	if (resCode < 0) {ShowDataBaseError();  return -1;} 
	
	if (retParam)
	{
		MessagePopup("��ʾ", "��ѯ���²��Խ���쳣    "); 
		return -1;
	}
	
	return 0;
}

int Set_DCA_Clear(int DCAType)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_Set_Clear(inst_CSA8000)) return -1; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_Set_Clear(inst_Ag86100)) return -1; 
			break;
		default:
			return -1;
		}
	return 0;
}

int caltestRxPower(int channel, struct struTestData *data, int panel)
{
	int    i, error, count;
	double dRxPow, RxPowMax, delta_Max=2, delta;
	float  RxPow_mon_DB[30];
	INT16U ADC; 
	char   Info[256];
	
	//�����Ե������ֵ���������ݿ�û�м�¼
	data->RxPow_num = my_struDBConfig_Monitor.CaliNumber_Test;

//-----------------����-18���ʣ��õ�ϵͳ���----------------------//
	SET_ATT_ONU(channel, -18+sRxCal.power[channel]);
//	Delay(0.3);  

	error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon_DB[0]);
	if (error)
	{
		return -1; 
	}	 
	
	delta = fabs (RxPow_mon_DB[0]+18);
	if (delta>delta_Max)  //�������ƫ���ظ�5�� 
	{
		count=0;
		do
		{
			SET_ATT_ONU (channel, -18+sRxCal.power[channel]);
	//		Delay(0.3); 

			error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon_DB[0]);
			if (error) 
			{
				return -1; 
			}
			
			delta = fabs (RxPow_mon_DB[0]+18); 
			if (delta<=delta_Max)
			{
				break;
			}
			count++;
			Delay(0.1);
		} while(count<5);
	}
	
	//��¼-18�Ĳ���ֵ�����Ե�����
	data->RxPow_mon[my_struDBConfig_Monitor.CaliNumber_Test] = RxPow_mon_DB[0];
	
	if (delta>delta_Max)
	{
		sprintf (Info, "-18dBm���ʵ���ֵϵͳ������%.2fdB", delta_Max);
		MessagePopup ("��ʾ", Info); 
		data->ErrorCode=ERR_TES_CAL_RX_28dBm; 
		return -1; 
	}
	delta = RxPow_mon_DB[0]+18;
//-----------------����-18���ʣ��õ�ϵͳ���----------------------//
	
	strcpy (Info, "���   У׼�㣨dBm��   ����ֵ��dBm��");
	Insert_Info(panel, gCtrl_BOX[channel], Info);

	RxPowMax=0;
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber_Test; i++) 
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint_Test[i]+sRxCal.power[channel]);	
//		Delay(0.3);
		
		error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon_DB[i]);
		if (error) 
		{
			return FALSE; 
		}
		data->RxPow_mon[i] = RxPow_mon_DB[i]; 
		if(data->RxPow_mon[i] > 1000) 
		{
			data->RxPow_mon[i]=999;  
		}
		dRxPow = fabs(RxPow_mon_DB[i]-my_struDBConfig_Monitor.CaliPoint_Test[i]-delta);
		
		if (dRxPow>my_struDBConfig_Monitor.Rx_Pow_Prec)  //����ֵ��������ָ�꣬�ظ�����5�Σ�ֱ��ͨ������
		{
			count=0;
			do
			{
				error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon_DB[i]);
				if (error)
				{
					return -1; 
				}
				data->RxPow_mon[i] = RxPow_mon_DB[i]; 
				
				dRxPow = fabs(RxPow_mon_DB[i]-my_struDBConfig_Monitor.CaliPoint_Test[i]-delta);
				if (dRxPow<=my_struDBConfig_Monitor.Rx_Pow_Prec) 
				{
					break;
				}
				count++;
				Delay(0.1);
			} while(count<5);
			if (count >=5 )
			{
				MessagePopup ("Error", "RxCal test Error  !");
				return -1; 
			}
		} 
		RxPowMax = dRxPow>RxPowMax ? dRxPow:RxPowMax; 
		
//		my_struTestData.RxPow_mon[i] = RxPow_mon_DB[i]; 
		
		sprintf (Info, "%02d:   %.2f   %.2f", i+1, my_struDBConfig_Monitor.CaliPoint_Test[i], RxPow_mon_DB[i]); 
		Insert_Info(panel, gCtrl_BOX[channel], Info);
	}
	
	//�����ݿ����ò�����������ָ��
	if (RxPowMax>my_struDBConfig_Monitor.Rx_Pow_Prec) 
	{
		return -1; 
	}
	
	return 0;
}

int GetModuleSN (int channel, struct struTestData *data, int panel)
{
int  index, status;
char prestr[30], preserial[20], laststr[30], lastserial[20], str[30], serial[20]; 
int  prenum, lastnum, num; 
char buf[1024];

//	if(my_struConfig.isScanModuleSN)	/***��Ҫɨ��ģ��SN**Eric.Yao**20130826***/
//	{
//		sprintf(buf,"������ͨ��%dģ��������", channel);
//		status = PromptPopup ("��ʾ", buf, data->ModuleSN, 30);
//		if (status<0) 
//		{
//			return -1;
//		}
//	}
//	else
//	{

		if (MyDLLReadModuleSN(INST_EVB[channel], data->ModuleSN) < 0)	   /***����ɨ��ģ��SN��ֱ�Ӵ�ģ���ڲ���ȡ**Eric.Yao**20130826***/
		{
			return -1;
		}
//	}
	
	//��SNת��Ϊ��д��������������ַ�ȥ��
	MyDLLCheckSN (data->ModuleSN);
	
	index=strlen (my_struDBConfig.BarCode);
	for(index=0;index<strlen (my_struDBConfig.BarCode); index++)
	{if (my_struDBConfig.BarCode[index]!=data->ModuleSN[index]) return -1;}

	if (my_struDBConfig.SNLength != strlen (data->ModuleSN))
	{
		Insert_Info(panel, gCtrl_BOX[channel], "ModuleSN���ȼ��ʧ��"); 
		return -1;
	} 
	
	if (my_struConfig.check_sn && (!my_struConfig.isNPI))    /***NPI�����ģ��SN��Χ**Eric.Yao**20130710***/
	{
		Scan(my_struConfig.firstsn, "%s>%s[w9]%s", prestr, preserial);     
		Scan(preserial, "%s>%i", &prenum);
	
		Scan(my_struConfig.lastsn, "%s>%s[w9]%s", laststr, lastserial);     
		Scan(lastserial, "%s>%i", &lastnum);
	
		Scan(data->ModuleSN, "%s>%s[w9]%s", str, serial);     
		Scan(serial, "%s>%i", &num);
				
		if (0!= strcmp(prestr, str) || 0!= strcmp(laststr, str))
		{
			Insert_Info(panel, gCtrl_BOX[channel], "���к�ǰ9λ��sgd_scdd_trx����Ĳ�һ��"); 
			return -1; 
		}  	
	
		if (num < prenum || num > lastnum)
		{
			Insert_Info(panel, gCtrl_BOX[channel], "���кŲ���sgd_scdd_trx��������кŷ�Χ��"); 
			return -1; 
		} 
	}

	return 0;
}

char *revstring(char *str, int length)
{
	char *start= str;
	char *end= str + length -1;
	char ch;
	
	if (str != NULL)
	{
		while(start < end)
		{
			ch = *start;
			*start++ = *end;
			*end-- = ch;
		}
	}	  
	return str;
}

int ReadModuleSN(int channel, struct struTestData *data, int panel)
{
	int index;
	
	MyDLLReadModuleSN (INST_EVB[channel], data->ModuleSN);
	
	index=strlen (my_struDBConfig.BarCode);
	for(index=0;index<strlen (my_struDBConfig.BarCode); index++)
	{if (my_struDBConfig.BarCode[index]!=data->ModuleSN[index]) return -1;}

	if (my_struDBConfig.SNLength != strlen (data->ModuleSN))
	{
		Insert_Info(panel, gCtrl_BOX[channel], "ModuleSN���ȼ��ʧ��"); 
		return -1;
	} 
	
	return 0;
}

int SaveEEPROM(int channel, struct struTestData *data)
{
int  error=0, count, rom_StartAddress;  
BYTE device_addr, rom_value, rom_value_arr_A0[256], CC_BASE, CC_EXT, rom_value_arr_A2[256]; 
BYTE rom_value_arr_A0_r[256], rom_value_arr_A2_r[256]; 
char data_str[20];
BOOL A0OK, A2OK;

	for(count=0; count<128; count++)
	{rom_value_arr_A0[count]=my_struDBConfig_E2.A0[count];}
	
	//set Vendor SN 
	for (count=0; count<16; count++)  
	{
  		if ( data->ModuleSN[count] != 0 ) 
		{
			rom_value_arr_A0[68+count]=data->ModuleSN[count]; 
		}
 		else
		{
			rom_value_arr_A0[68+count]=0x20 ;  //����0���������ÿո����
		}
	} 
	
	//set Vendor DATE 
	strcpy (data_str, DateStr ());
	rom_value_arr_A0[84]=data_str[8];   //year MSB   //locate from A0h[84]
	rom_value_arr_A0[85]=data_str[9];  //year LSB
	rom_value_arr_A0[86]=data_str[0];   //month MSB
	rom_value_arr_A0[87]=data_str[1];   //month LSB
	rom_value_arr_A0[88]=data_str[3];  //date MSB
	rom_value_arr_A0[89]=data_str[4];  //date LSB
	rom_value_arr_A0[90]=' ';  //space key, 0x20
	rom_value_arr_A0[91]=' ';  //space key, 0x20
			
	//re-calculate check-sum, A0h[0~62]
	CC_BASE=0;
	for (count=0; count<63; count++)
	{ 
		CC_BASE += rom_value_arr_A0[count]; 
	}
	rom_value_arr_A0[63] = CC_BASE;

	//re-calculate check-sum, A0h[64~94]
	CC_EXT=0;
	for (count=64; count<95; count++)
	{ 
		CC_EXT += rom_value_arr_A0[count]; 
	}
	rom_value_arr_A0[95] = CC_EXT;
	
	//Set Password
	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 
	{
		error = ux3328_set_FactoryMode (INST_EVB[channel]);
		if (error) 
		{
			MessagePopup("����", "3328���ù���ģʽ����   "); 
			return -1;
		}  
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) 
	{
		error = ux3328s_set_FactoryMode (INST_EVB[channel]);
		if (error)
		{
			MessagePopup("����", "3328S���ù���ģʽ����   "); 
			return -1;
		}  
	}
	else if (CHIEFCHIP_DS4830 == my_struDBConfig_ERCompens.ChiefChip) 
	{
		error = ux3328s_set_FactoryMode (INST_EVB[channel]);
		if (error)
		{
			MessagePopup("����", "3328S���ù���ģʽ����   "); 
			return -1;
		}  
	}
	//write A0h[] to Module flash
	device_addr=0xA0;
	rom_StartAddress=0;
	errChk(I2C_BYTEs_WRITE_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 128, rom_value_arr_A0, 0.2));   //121126�޸� ��дA0 96�ֽڸ�ΪдA0 128�ֽ�

	for(count=0; count<128; count++)
	{
		rom_value_arr_A2[count]=my_struDBConfig_E2.A2[count];
	}
	
	//re-calculate check-sum, A2h[0~94]
	CC_EXT=0;
	for (count=0; count<95; count++)
	{
		CC_EXT += rom_value_arr_A2[count]; 
	}
	rom_value_arr_A2[95] = CC_EXT;
	
	//write A2h[] to Module flash
	device_addr=0xA2;
	rom_StartAddress=0;
    errChk(I2C_BYTEs_WRITE_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 96, rom_value_arr_A2, 0.2));

	//��Ӽ�����
	//Power off
//	errChk(SET_EVB_SHDN(channel, 0));
	Delay(0.1);
	//Power on
	errChk(SET_EVB_SHDN(channel, 1));     
	Delay(1);
	
	device_addr=0xA0;
	rom_StartAddress=0;
	memset(rom_value_arr_A0_r,0,sizeof(rom_value_arr_A0_r));
	errChk(I2C_BYTEs_READ_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 128, rom_value_arr_A0_r));  
	
	A0OK=TRUE;
	for (count=0; count<128; count++)
	{
		if (rom_value_arr_A0[count] != rom_value_arr_A0_r[count]) 
		{
			A0OK=FALSE;
			break;
		}
	}
	
	device_addr=0xA2;
	rom_StartAddress=0;
	errChk(I2C_BYTEs_READ_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 96, rom_value_arr_A2_r));  
	
	A2OK=TRUE;
	for (count=0; count<96; count++)
	{
		if (rom_value_arr_A2[count] != rom_value_arr_A2_r[count])
		{
			A2OK=FALSE;
			break;
		}
	}
	
	if (!A0OK || !A2OK)
	{
//		if (!SFP_ENTRY_Password2(INST_EVB[channel])) 
//		{
//			error = -1;
//			goto Error;
//		} 
		
		//write A0h[] to Module flash
		device_addr=0xA0;
		rom_StartAddress=0;
		errChk(I2C_BYTEs_WRITE_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 128, rom_value_arr_A0, 0.2));
		
		//write A2h[] to Module flash
		device_addr=0xA2;
		rom_StartAddress=0;
		errChk(I2C_BYTEs_WRITE_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 96, rom_value_arr_A2, 0.2));
		
		//�ٴμ��
		//Power off
		errChk(SET_EVB_SHDN(channel, 0));
		Delay(0.1);
		//Power on
		errChk(SET_EVB_SHDN(channel, 1));     
		Delay(1);
		
		device_addr=0xA0;
		rom_StartAddress=0;
	    errChk(I2C_BYTEs_READ_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 128, rom_value_arr_A0_r));  
	
		A0OK=TRUE;
		for (count=0; count<128; count++)
		{
			if (rom_value_arr_A0[count] != rom_value_arr_A0_r[count]) 
			{
				A0OK=FALSE;
				break;
			}
		}
	
		device_addr=0xA2;
		rom_StartAddress=0;
	    errChk(I2C_BYTEs_READ_DLL (INST_EVB[channel], device_addr, rom_StartAddress, 96, rom_value_arr_A2_r));  
	
		A2OK=TRUE;
		for (count=0; count<96; count++)
		{
			if (rom_value_arr_A2[count] != rom_value_arr_A2_r[count])
			{
				A2OK=FALSE;
				break;
			}
		}
		
		if (!A0OK || !A2OK) 
		{
			return -1;
		}
	}
	
Error:
	
	return error;
}

int scanModduleSN(int panel, int channel, char *sn)
{
	//�������棬ɨ��SN
	int myConfirmID;
	char tmp[100] = "";  
	
	int cmd_OK = 0;
		
	//���ý��汳��ɫ      
	SetPanelAttribute (panel, ATTR_BACKCOLOR, COLOR_CHAN[channel]);	
	
	DisplayPanel(panel);  
	
	sprintf(tmp, "��ɨ��ͨ��%d��ģ��SN", channel);
	SetCtrlAttribute(panel, PAN_SN_STRING_MODULESN, ATTR_LABEL_TEXT, "");   
//	SetPanelAttribute (panel, ATTR_TITLE, tmp);
	
	SetCtrlVal(panel, PAN_SN_CHECKBOX, 0); 
	
//	SetActivePanel(panel);	    
	SetCtrlVal(panel, PAN_SN_STRING_MODULESN, "");	
	SetActiveCtrl (panel, PAN_SN_STRING_MODULESN);  

	while (1)
	{   
//		GetUserEvent (0, NULL, &myConfirmID); 
//		if (PAN_SN_CMD_OK == myConfirmID)
//		{
//			break;
//		}  
//		else
//		{																														  k
//			Delay(0.05);
//		}

//		GetCtrlVal(panel, PAN_SN_STRING_MODULESN, sn); 
//		if (my_struDBConfig.SNLength == strlen (sn)) 
//		{
//			break;
//		}  
//		else
//		{
//			Delay(0.05);
//		}
		
		GetCtrlVal(panel, PAN_SN_CHECKBOX, &cmd_OK); 
		if (cmd_OK) 
		{
			break;
		}  
		else
		{
			Delay(0.05);
		}
	}
	
	GetCtrlVal(panel, PAN_SN_STRING_MODULESN, sn);
	
	HidePanel(panel);       
	
	return 0;
}

int DB_Read_Spec_TxCal (void)
{
	char buf[256];
	int  num;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
    
	Fmt (buf, "select txpowmin,txpowmax from AUTODT_SPEC_ATE  where dt_flag='QATEST' and temper_flag='ROOM' and PartNumber='%s' and version='%s'",my_struConfig.PN, my_struConfig.BOM);  
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColFloat (hstmt, 1, &my_struDBConfig.TxCalMin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}	
	resCode = DBBindColFloat (hstmt, 2, &my_struDBConfig.TxCalMax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
	if (num!=1) 
	{
		MessagePopup ("��ʾ", "���ݿ�AUTODT_SPEC_ATE����û��dt_flag='QATEST'��temper_flag='ROOM'�ļ�¼    "); 
		return -1;
	} 	
	
	return 0;
}

int GetEnableChannel(int *channel)
{
	int error;
	int i;
	
	error=0;
	
	for (i=0; i<CHANNEL_MAX; i++)
	{
		if(INSTR[i].ChannelEn) 
		{
			*channel=i;
			break;
		}
	}
	
	if (i>=CHANNEL_MAX)
	{
		return -1;
	}
	
	return error;
}

void SetMainControl(int panel)
{
	int channel;
	struct struInstrument localInst;
	
	for (channel=0; channel<CHANNEL_MAX; channel++)
	{
		localInst = INSTR[channel]; 
		InsertTree (panel, PAN_MAIN_TREE, &localInst, 1); 
		
		//����״̬����������
		SetCtrlAttribute (panel, gCtrl_BOX[channel], ATTR_DIMMED, !INSTR[channel].ChannelEn);
		//������ɫ
		SetCtrlAttribute (panel, gCtrl_BOX[channel], ATTR_TEXT_BGCOLOR, COLOR_CHAN[channel]);
		
		//���ؿؼ�
		SetCtrlAttribute (panel, gCtrl_BIN[channel], ATTR_DIMMED, !INSTR[channel].ChannelEn);  
		
		//LED��
		SetCtrlAttribute (panel, gCtrl_LED[channel], ATTR_DIMMED, !INSTR[channel].ChannelEn);  
		SetCtrlAttribute (panel, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_GRAY);
		SetCtrlVal (panel, gCtrl_LED[channel], 0);
	}
}

int DB_Get_AutoDT_Spec_UX3328(void) 
{
	int error;
	char buf[1024];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	int count;
	char BurstFlag[50];
	
	sprintf (buf, "SELECT BurstON FROM AutoDT_Spec_UX3328 WHERE pn='%s' AND bom='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}   

	resCode = DBBindColChar (hstmt, 1, 50, BurstFlag, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      

    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError(); return -1;} 

    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) {MessagePopup("��ʾ","���ݿ���AutoDT_Spec_UX3328�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����"); return -1;}

	//��ȡburst��״̬
	if (stricmp(BurstFlag, "�ߵ�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_H;
		my_struEVB5.BURST_OFF=EVB5_BURST_L; 
	}
	else if (stricmp (BurstFlag, "�͵�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_L;
		my_struEVB5.BURST_OFF=EVB5_BURST_H; 
	}
	else
	{
		MessagePopup("��ʾ","���ݿ�AutoDT_Spec_UX3328��BurstON����ֵ����ȷ��\nֻ������Ϊ���͵�ƽ�������ǡ��ߵ�ƽ����"); 
		return -1;
	}
	
	return 0;
}

int DB_Get_AutoDT_Spec_NT25L90(void)
{
	int error;
	char buf[1024];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	int count;
	int i;

	sprintf (buf, "SELECT Ratio_L,Ratio_H,Ratio_H_Ex,BurstON,reg60H,reg61H,reg62H,reg63H,reg64H,reg65H,reg66H,reg67H,reg68H,reg69H,reg6AH,reg6BH,reg6CH,reg6DH FROM AutoDT_Spec_NT25L90 WHERE pn='%s' AND bom='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}   

	resCode = DBBindColDouble (hstmt, 1, &my_struDBConfig_VSC7965.Ratio_L, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 2, &my_struDBConfig_VSC7965.Ratio_H, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 3, &my_struDBConfig_VSC7965.Ratio_H_Ex, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 4, 50, my_struDBConfig_VSC7965.BurstFlag, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
 	for (i=0; i<14; i++)
	{	
		resCode = DBBindColInt (hstmt, i+5, &my_struFirmware.NT25L90_REG_EEPROM[i], &DBConfigStat);
    	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	}

    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      

    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError(); return -1;} 

    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) 
	{
		MessagePopup("��ʾ","���ݿ���AutoDT_Spec_NT25L90�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����");
		return -1;
	}

	//��ȡburst��״̬
	if (strcmp(my_struDBConfig_VSC7965.BurstFlag, "�ߵ�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_H;
		my_struEVB5.BURST_OFF=EVB5_BURST_L; 
	}
	else if (strcmp(my_struDBConfig_VSC7965.BurstFlag, "�͵�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_L;
		my_struEVB5.BURST_OFF=EVB5_BURST_H; 
	}
	else
	{
		MessagePopup("��ʾ","���ݿ�AutoDT_Spec_VSC7965��BurstON����ֵ����ȷ��\nֻ������Ϊ���͵�ƽ�������ǡ��ߵ�ƽ����"); 
		return -1;
	}
	
	//60H �ϵ�RAMTxDisΪ��
	for (i=0; i<14; i++) 
	{
		my_struFirmware.NT25L90_REG_RAM[i] = my_struFirmware.NT25L90_REG_EEPROM[i];
	}
	my_struFirmware.NT25L90_REG_RAM[0] = my_struFirmware.NT25L90_REG_EEPROM[0] | 0x01;

	return 0;
}

int DB_Get_AutoDT_Spec_VSC796x(void)
{
	int error;
	char buf[1024];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	int count;

	sprintf (buf, "SELECT Ratio_L,Ratio_H,Ratio_H_Ex,BurstON,R0,R1,R2,R3,R4,R5,R6,R7,R8,R11,R12,R13,R14,R15,R18,R19,R20,R21,R22,R23,R24,R25 FROM AutoDT_Spec_VSC7965 WHERE PartNumber='%s' AND Version='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}   

	resCode = DBBindColDouble (hstmt, 1, &my_struDBConfig_VSC7965.Ratio_L, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 2, &my_struDBConfig_VSC7965.Ratio_H, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 3, &my_struDBConfig_VSC7965.Ratio_H_Ex, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 4, 50, my_struDBConfig_VSC7965.BurstFlag, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 5, &my_struDBConfig_VSC7965.VSC7965_FLASH[0], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 6, &my_struDBConfig_VSC7965.VSC7965_FLASH[1], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 7, &my_struDBConfig_VSC7965.VSC7965_FLASH[2], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 8, &my_struDBConfig_VSC7965.VSC7965_FLASH[3], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 9, &my_struDBConfig_VSC7965.VSC7965_FLASH[4], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 10, &my_struDBConfig_VSC7965.VSC7965_FLASH[5], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 11, &my_struDBConfig_VSC7965.VSC7965_FLASH[6], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 12, &my_struDBConfig_VSC7965.VSC7965_FLASH[7], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 13, &my_struDBConfig_VSC7965.VSC7965_FLASH[8], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 14, &my_struDBConfig_VSC7965.VSC7965_FLASH[11], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 15, &my_struDBConfig_VSC7965.VSC7965_FLASH[12], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 16, &my_struDBConfig_VSC7965.VSC7965_FLASH[13], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 17, &my_struDBConfig_VSC7965.VSC7965_FLASH[14], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 18, &my_struDBConfig_VSC7965.VSC7965_FLASH[15], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 19, &my_struDBConfig_VSC7965.VSC7965_FLASH[18], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 20, &my_struDBConfig_VSC7965.VSC7965_FLASH[19], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 21, &my_struDBConfig_VSC7965.VSC7965_FLASH[20], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 22, &my_struDBConfig_VSC7965.VSC7965_FLASH[21], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 23, &my_struDBConfig_VSC7965.VSC7965_FLASH[22], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 24, &my_struDBConfig_VSC7965.VSC7965_FLASH[23], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 25, &my_struDBConfig_VSC7965.VSC7965_FLASH[24], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 26, &my_struDBConfig_VSC7965.VSC7965_FLASH[25], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      

    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError(); return -1;} 

    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) 
	{
		MessagePopup("��ʾ","���ݿ���AutoDT_Spec_VSC7965�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����");
		return -1;
	}

	//��ȡburst��״̬
	if (strcmp(my_struDBConfig_VSC7965.BurstFlag, "�ߵ�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_H;
		my_struEVB5.BURST_OFF=EVB5_BURST_L; 
	}
	else if (strcmp(my_struDBConfig_VSC7965.BurstFlag, "�͵�ƽ")==0)
	{
		my_struEVB5.BURST_ON =EVB5_BURST_L;
		my_struEVB5.BURST_OFF=EVB5_BURST_H; 
	}
	else
	{
		MessagePopup("��ʾ","���ݿ�AutoDT_Spec_VSC7965��BurstON����ֵ����ȷ��\nֻ������Ϊ���͵�ƽ�������ǡ��ߵ�ƽ����"); 
		return -1;
	}
	
	//��ȡfOVERDFlag����ֵ,ֻ��Vsc7967��Ч��Ĭ��ֵ0
	if (DRIVERCHIP_VSC7967==my_struDBConfig_ERCompens.DriverChip && 0==my_struDBConfig_VSC7965.VSC7965_FLASH[25])
	{
		my_struDBConfig_VSC7965.UpdatefOVERDFlag=1;
	}
	else
	{
		my_struDBConfig_VSC7965.UpdatefOVERDFlag=0;
	}

	return -1;
}

int DB_GeT_Spec_Image_Tiny13(void)
{
	char buf[500]; 
	int  count, i;
	unsigned char myArray_VSC[1024];   
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	Fmt (buf, "SELECT Firmware_Image from AutoDT_Spec_Image WHERE PartNumber='%s' AND Version='%s' AND Firmware_Ver='FLASH'", my_struConfig.PN, my_struConfig.BOM);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  	
   
	resCode = DBBindColBinary (hstmt, 1, 1024, myArray_VSC, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
    	for (i=0; i<1024; i++)
    	{my_struFirmware.TINY13_FLASH[i] = myArray_VSC[i];}
    	
    	count++;    
    }
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (count!=1)
	{MessagePopup ("ERROR", "��ȡ���ݿ��Firmware�ļ�ʧ�ܣ�"); return -1;}  	

	return 0;
}

int DB_Get_Spec_Image_Mega88(void)
{
	char buf[500]; 
	int  count, i;
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	Fmt (buf, "SELECT Firmware_Ver from AutoDT_Spec_Image WHERE PartNumber='%s' AND Version='%s' AND Firmware_Flag='FLASH'", my_struConfig.PN, my_struConfig.BOM);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  	
   
    resCode = DBBindColChar (hstmt, 1, 50, my_struFirmware.Firmware_Ver, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError();  return -1;}
    
    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}
	
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (count!=1)
	{MessagePopup ("ERROR", "��ȡ���ݿ��Firmware�汾ʧ�ܣ�"); return -1;}  	

	//��ȡEEPROM��Ϣ	
	Fmt (buf, "SELECT Firmware_Image,SFRM_NUMBER from AutoDT_Spec_Image WHERE PartNumber='%s' AND Version='%s' AND Firmware_Ver='%s' AND Firmware_Flag='EEPROM'", my_struConfig.PN, my_struConfig.BOM, my_struFirmware.Firmware_Ver);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  	
   
	resCode = DBBindColBinary (hstmt, 1, 512, my_struFirmware.MEGA88_EEPROM, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    resCode = DBBindColChar (hstmt, 2, 50, my_struFirmware.SFRMNum, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError();  return -1;}
    
    count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    {
    	count++;    
    }
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    if (count!=1)
	{MessagePopup ("ERROR", "��ȡ���ݿ��AutoDT_Spec_Image�ļ�ʧ�ܣ�"); return -1;}  

	//תΪ��д����ɾ��������ַ�������ո��
	MyDLLCheckSN (my_struFirmware.SFRMNum);
		
	return 0;
}

int checkfunction(void)
{
	int error;
	
	if (!my_struConfig.DT_Tuning && !my_struConfig.Sel_T_Upstream)
	{
		if(!my_struConfig.Sel_R_Sens) 
		{
			if (!ConfirmPopup ("��ʾ", "��ȷ�ϲ���Ҫѡ�������Ȳ��Թ��ܣ�")) return -1;
		}
	}

	//�ж�У׼����ѡ��
	if(my_struConfig.PN_TYPE==PN_TYPE_ONU_DDMI && my_struConfig.DT_Tuning && my_struConfig.Temper_Room)
	{
		if(!my_struConfig.Sel_Calibrate) 
		{if (!ConfirmPopup ("��ʾ", "��ȷ�ϲ���Ҫѡ��У׼���ܣ�")) return -1;}
	}

	if(my_struConfig.PN_TYPE==PN_TYPE_ONU_DDMI && my_struConfig.DT_Test && my_struConfig.Temper_Room)
	{
		if(!my_struConfig.Sel_Calibrate_Test) 
		{if (!ConfirmPopup ("��ʾ", "��ȷ�ϲ���Ҫѡ��У׼���Թ��ܣ�")) return -1;}
	}

	return 0;
}

int Init_TxSD_Detect(int channel)
{
	int error;
	
	error = EVB5_SET_TxSD_Detect (INST_EVB[channel]);	
	if (error) 
	{
		return -1;
	}
	
	if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_OFF)<0) 
	{
		return -1;
	}
	
	return 0;
}

int InitTSSIDetect(int channel)
{
	int error;
	
	//set ben to clock mode
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_CLOCK));
	
	//100k hz
	errChk(EVB5_SET_CLOCK (INST_EVB[channel], 0.1));
	
Error:	
	
	return error;
}

int test_TxSD_Detect(int channel)
{
	int  error; 
	BYTE TxSD_Error;
		
	//��ȡTxSD_Detect״̬
	errChk(EVB5_READ_TxSD (INST_EVB[channel], &TxSD_Error));
	
	if (TxSD_Error) return -1;  
	
	//����ONU��BURST.ON״̬
	errChk(EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON));
	
Error:	
	
	return error;
}

int testTxSD(int channel)
{
	int  error;
	BYTE SD;
	
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_PULSE)); 
	
	//---for TxSD=1---// 
	errChk(EVB5_SET_BEN_PulsePolarity (INST_EVB[channel], my_struEVB5.BURST_ON)); 
		
	errChk(EVB5_SET_BEN_Level (INST_EVB[channel], 1)); 
		
	errChk(EVB5_READ_ONU_TxSD (INST_EVB[channel], &SD)); 
		
	if (SD !=1) 
	{
		MessagePopup ("Error", "����ONU_TxSD=1ʧ��");
		error = -1;
		goto Error;
	} 
	//---for TxSD=1---// 
	
	//---for TxSD=0---//
	errChk(EVB5_SET_BEN_PulsePolarity (INST_EVB[channel], my_struEVB5.BURST_OFF)); 
		
	errChk(EVB5_SET_BEN_Level (INST_EVB[channel], 1)); 
		
	errChk(EVB5_READ_ONU_TxSD (INST_EVB[channel], &SD)); 
		
	if (SD !=0)
	{
		MessagePopup ("Error", "����ONU_TxSD=0ʧ��");
		error = -1; 
		goto Error;
	} 
	//---for TxSD=0---// 
	
Error:
	
	if (EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0) 
	{
		return -1;
	} 
	
	return error; 
}

int TestTSSIDetect(int channel, int iTestStartTime)
{
	int  error;
	BYTE SD; 
	int cnt;
	int timer;
	
	timer = Timer()- iTestStartTime; 
	
	errChk(EVB5_READ_ONU_TxSD (INST_EVB[channel], &SD)); 
		
	while (0==SD && timer<4)
	{
		
		Delay(1);
		
		errChk(EVB5_READ_ONU_TxSD (INST_EVB[channel], &SD)); 
		
		if (1==SD)
		{
			error=-1;
			goto Error;
		}

		timer = Timer()- iTestStartTime;  
	} 
	
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
	
	cnt=0;
	do
	{
		Delay(1);
		
		errChk(EVB5_READ_ONU_TxSD (INST_EVB[channel], &SD));
		
		if (1==SD)
		{   
			break;
		}
		
		cnt++;
		
	} while (5>cnt);
	
	if (5<=cnt)
	{
		return -1; 		
	}
	
Error:	
	
	error = EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);
	if (error) return -1;
		
	return error;
}

int testSD(int channel)
{
BYTE SD;

	//���SD�ź�   0��ģ��SDD, 1:û��SDA 
	if (Read_RX_SD(channel, 0, &SD)<0) 
	{
		return -1;   
	}
	
	if(0==SD)
	{
		return -1;
	}
	
	return 0;
}

int SET_DAC_APC (const int channel, int DAC)
{
	switch (my_struDBConfig_ERCompens.DriverChip)
	{
		case DRIVERCHIP_VSC7965:
			if (EVB5_WRITE_TINY13_BYTE(INST_EVB[channel], 0x05, DAC)<0) return -1; 
			break;
		case DRIVERCHIP_VSC7967:
			if (MEGA88_SET_DAC_APC(INST_EVB[channel], DAC)<0) return -1; 
			break;
		case DRIVERCHIP_NT25L90:
			if (MEGA88NT25_SET_APC_RAM(INST_EVB[channel], DAC)<0) return -1;
			break;
		case DRIVERCHIP_UX3328:
			if (ux3328_set_apc_dac(INST_EVB[channel], DAC)<0) return -1;
			break;
		case DRIVERCHIP_UX3328S:
			if (ux3328s_set_apc_dac(INST_EVB[channel], DAC)<0) return -1;
			break;
		default:
			MessagePopup("��ʾ","��ƷоƬ���ʹ���");
			return -1;
			break;
	}
	return 0; 
}

int SET_DAC_MOD (const int channel, int DAC)
{
	switch (my_struDBConfig_ERCompens.DriverChip)
	{
		case DRIVERCHIP_VSC7965:
			if (EVB5_WRITE_TINY13_BYTE(INST_EVB[channel], 0x00, DAC)<0) return -1; 
			break;
		case DRIVERCHIP_VSC7967:
			if (MEGA88_SET_DAC_MOD(INST_EVB[channel], DAC)<0) return -1; 
			break;
		case DRIVERCHIP_NT25L90:
			if (MEGA88NT25_SET_MOD_RAM(INST_EVB[channel], DAC)<0) return -1;
			break;
		case DRIVERCHIP_UX3328:
			if (ux3328_set_mod_dac(INST_EVB[channel], DAC)<0) return -1;
			break;
		case DRIVERCHIP_UX3328S:
			if (ux3328s_set_mod_dac(INST_EVB[channel], DAC)<0) return -1;
			break;
		default:
			MessagePopup("��ʾ","��ƷоƬ���ʹ���");
			return -1;
			break;
	}
	return 0; 
}

int Read_Ibias(const int channel, double *Ibias)
{
	int    error;
	float  temp;
	double Ibias_temp=0;	
	
	switch (my_struDBConfig_ERCompens.ChiefChip)
	{
		case CHIEFCHIP_TINY13:
			error = EVB5_READ_TINY13_Ibias (INST_EVB[channel], &Ibias_temp); 
			if (error) return -1;
	    	break;
			
		case CHIEFCHIP_MEGA88:
			error = MEGA88_GET_Ibias (INST_EVB[channel], &Ibias_temp); 
			if (error) return -1;
			break;

		case CHIEFCHIP_UX3328:
			error = MyDLL_8472_GetIbias (INST_EVB[channel], &temp); 
			if (error) return -1;
			Ibias_temp=temp;
			break;
		case CHIEFCHIP_UX3328S:
			error = MyDLL_8472_GetIbias (INST_EVB[channel], &temp); 
			if (error) return -1;
			Ibias_temp=temp;
			break;
		case CHIEFCHIP_DS4830:
			error = MyDLL_8472_GetIbias (INST_EVB[channel], &temp); 
			if (error) return -1;
			Ibias_temp=temp;
			break;	
		default:
			MessagePopup("��ʾ","��Ʒ���ʹ���");
			return -1;
			break;
	}

	*Ibias=Ibias_temp;

	return 0;
}

BOOL UpdateTiny13Firmware(const int channel, struct struTestData *data)
{
	unsigned char myArray[1024];
	int	index;
	
	//��ȡ�����¶�
	if (EVB5_READ_TINY13_Temper(INST_EVB[channel], &data->Temperatrue)<0)	
	{MessagePopup("Error", "��ȡTiny13�¶�ʧ�ܣ�");return FALSE;} 
	
	if(data->Temperatrue<0 || data->Temperatrue>60) 
	{MessagePopup("Error", "����LUT�¶ȴ���");return FALSE;}  

	//�õ�Appfirmware
	for (index=0;index<1024; index++)
	{myArray[index]=my_struFirmware.TINY13_FLASH[index];}
	
	//���lut 	
	if (!fit_LUT_TINY13(my_struDBConfig_VSC7965.Ratio_L, my_struDBConfig_VSC7965.Ratio_H, 115, -35, data->Temperatrue, data->ucRb[index], data->ucRm[index], myArray)) return FALSE;
	
	//����Ӧ��ģʽfirmware
	if (EVB5_WRITE_TINY13_Firmware(INST_EVB[channel], myArray)<0) 
	{MessagePopup("Error", "����Firmwareʧ�ܣ�");return FALSE;} 

	//-----------����Ϊ�Զ�ģʽģʽ--------------//
	if (EVB5_WRITE_TINY13_BYTE(INST_EVB[channel], 0x28, TINY13_MODE_AUTO)<0) 
	{MessagePopup("Error", "�����Զ�ģʽʧ�ܣ�");return FALSE;} 	
	//-----------����Ϊ�Զ�ģʽģʽ--------------// 	
	return TRUE;
}

int get_mega88_temper_lutindex (int evb, struct struTestData *data)
{
	int error;
	double temp=0.;
	float ftemp;

	switch (my_struDBConfig_ERCompens.DriverChip)
	{
		case DRIVERCHIP_VSC7967:
			
			if (data->FirmwareVer>10)
			{
				error = MEGA88_GET_Temper_Index (evb, &ftemp);
				if (error) return -1;
			}
			else
			{
				error = MEGA88_GET_Temperatrue (evb, &temp);
				if (error) return -1;
				
				ftemp = (float)temp;  
			}
		
			break;
			
		case DRIVERCHIP_NT25L90:
			
			if (0==stricmp(my_struFirmware.SFRMNum,"SFRM0050"))
			{
				error = MEGA88GetSFRM50TemperIndex (evb, &ftemp);
				if (error) return -1; 
			}
			else
			{
				error = MEGA88NT25_GET_Temper_Index (evb, &ftemp);
				if (error) return -1; 
			}
			
			break;
			
		default:
			
			MessagePopup ("��ʾ", "DriverChip type error");
			return -1;
	}
	
	data->Temperatrue = ftemp;
		
	return 0;
}

int fit_LUT_MEGA88(double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data)
{
	int error;
	
	switch (my_struDBConfig_ERCompens.DriverChip)
	{
		case DRIVERCHIP_VSC7967:
			
			error = fit_LUT_MEGA88_VSC7967 (Ratio_L, Ratio_H, TemperLimit_H, TemperLimit_L, Temper, DAC_MOD, LUT_arr, data);
			if (error) return -1;
		   
			break;
		case DRIVERCHIP_NT25L90:
			
			error = fit_LUT_MEGA88_NT25L90 (Ratio_L, Ratio_H, TemperLimit_H, TemperLimit_L, Temper, DAC_MOD, LUT_arr, data);
			if (error) return -1;
			
			break;
		default:
			MessagePopup("��ʾ","��ƷоƬ���ʹ���");
			return -1;
			break;
	}
	return 0; 		  
}

int fit_LUT_MEGA88_NT25L90 (double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data)
{
int	 DAC_MOD35;
int  fitNumber, i, j; 
double 	slope, offset, *slope_arr, *offset_arr, temper; 
double	LUT_in_arr[3], Temper_in_arr[3]; 
double  Imod35, Imod, Imod040, Imod120;

	fitNumber=3; //3��2�� 
  
	//���㵱ǰDAC��Ӧ��Imodֵ 
	Imod = 0.4102 * exp (0.0217*DAC_MOD);
		
	//����30C��Ӧ��DACֵ
	if (Temper>35 && Temper<85) Imod35=Imod+Ratio_H*(35-Temper);
	else if (Temper<35)		    Imod35=Imod+Ratio_L*(Temper-35); 
	else {MessagePopup ("��ʾ", "ER����¶ȳ������㷶Χ"); return -1;}    
	
	Temper_in_arr[0]=-40;
	Temper_in_arr[1]=35; 
	Temper_in_arr[2]=120;  
	
	Imod040 = Imod35-Ratio_L*(-40-35);   
	Imod120 = Imod35+Ratio_H*(120-35);   
		
  	LUT_in_arr[0]= (log(Imod040/0.4102))/0.0217; 
  	LUT_in_arr[1]= (log(Imod35/0.4102))/0.0217; 
	LUT_in_arr[2]= (log(Imod120/0.4102))/0.0217;   
	
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	offset_arr = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(LUT_in_arr[i+1]-LUT_in_arr[i])/(double)(Temper_in_arr[i+1]-Temper_in_arr[i]);
		offset_arr[i] =(double)(LUT_in_arr[i+1]-slope_arr[i]*Temper_in_arr[i+1]);
	}
	//������ұ�
	for (i=0; i<81; i++)
	{
		temper = i*2-40;
		
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper_in_arr[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper_in_arr[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��255��Χ��ֵ����
		if      ((temper*slope + offset)<0)   LUT_arr[i] = 0;
		else if ((temper*slope + offset)>255) LUT_arr[i] = 255;
		else                                  LUT_arr[i] = temper*slope + offset;
	}
		
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 
	
	//��ӵ����Լ��
	for(i=0;i<80;i++)
	{
		if ((LUT_arr[i+1]-LUT_arr[i]) < 0 ) 
		{MessagePopup ("Error", "ER���ұ����Լ��ʧ�ܣ�");return -1;}   
	}
	
	//����80����Ƶ���
//	Imod = 0.4102 * exp (0.0217*DAC_MOD); ;
	if (0.4102 * exp (0.0217*LUT_arr[60]) > my_struDBConfig_ERCompens.CurrentLimit_80)
	{
		MessagePopup ("��ʾ", "����80����Ƶ����쳣������AutoDT_Spec_ERCompens��CurrentLimit_80������ֵ�Ƿ���ȷ"); 
		data->ErrorCode = ERR_TUN_LD_SE_LOW;
		return -1;
	}
	
	return 0;
}

int fit_LUT_MEGA88_VSC7967 (double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_MOD, BYTE *LUT_arr, struct struTestData *data)
{
int	 DAC_MOD30;
int  fitNumber, i, j; 
double 	slope, offset, *slope_arr, *offset_arr, temper; 
double	LUT_in_arr[3], Temper_in_arr[3]; 

	fitNumber=3; //3������ 
	
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	offset_arr = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
  
	//����30C��Ӧ��DACֵ
	if (Temper>30) DAC_MOD30=DAC_MOD+Ratio_H*(30-Temper);
	else		   DAC_MOD30=DAC_MOD+Ratio_L*(30-Temper); 
	
	Temper_in_arr[0]=-40;
	Temper_in_arr[1]=30; 
  	Temper_in_arr[2]=120; 
	
  	LUT_in_arr[0]= DAC_MOD30+Ratio_L*(-40-30); 
  	LUT_in_arr[1]= DAC_MOD30;
 	LUT_in_arr[2]= DAC_MOD30+Ratio_H*(120-30);
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(LUT_in_arr[i+1]-LUT_in_arr[i])/(double)(Temper_in_arr[i+1]-Temper_in_arr[i]);
		offset_arr[i] =(double)(LUT_in_arr[i+1]-slope_arr[i]*Temper_in_arr[i+1]);
	}
	//������ұ�
	for (i=0; i<81; i++)
	{
		temper = i*2-40;
		
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper_in_arr[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper_in_arr[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<arraylenth-1; j++)
		
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��255��Χ��ֵ����
		if      ((temper*slope + offset)<0)   LUT_arr[i] = 0;
		else if ((temper*slope + offset)>255) LUT_arr[i] = 255;
		else                                  LUT_arr[i] = temper*slope + offset;
	}
		
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 
	
	//��ӵ����Լ��
	for(i=0;i<80;i++)
	{
		if ((LUT_arr[i+1]-LUT_arr[i]) < 0 ) 
		{MessagePopup ("Error", "ER���ұ����Լ��ʧ�ܣ�"); return -1;}   
	}
	
	//����80����Ƶ���
//	Imod = 100*LUT_arr[60]/255.;
	if (100*LUT_arr[60]/255. > my_struDBConfig_ERCompens.CurrentLimit_80)
	{
		MessagePopup ("��ʾ", "����80����Ƶ����쳣������AutoDT_Spec_ERCompens��CurrentLimit_80������ֵ�Ƿ���ȷ"); 
		data->ErrorCode = ERR_TUN_LD_SE_LOW;
		return -1;
	}  

	return 0;
}

BOOL fit_LUT_TINY13(double Ratio_L, double Ratio_H, int TemperLimit_H, int TemperLimit_L, double Temper, BYTE DAC_APC, BYTE DAC_MOD, BYTE *LUT_VSC)
{
int 	i, j, DAC_MOD40;
double 	slope, offset, *slope_arr, *offset_arr, temper; 
BYTE 	fitNumber;
double	Temper_in_arr[3], LUT_in_arr[3];
BYTE LUT_out_arr[80];

	fitNumber=3; //3������ 
	
	slope_arr  = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
	offset_arr = malloc ((fitNumber-1)*8); //�����ڴ�ռ�
  
	//����40C��Ӧ��DACֵ
	if (Temper>40) DAC_MOD40=DAC_MOD+Ratio_H*(40-Temper);
	else		   DAC_MOD40=DAC_MOD+Ratio_L*(40-Temper); 
	
	Temper_in_arr[0]=-40;
	Temper_in_arr[1]=40; 
  	Temper_in_arr[2]=120; 
  	LUT_in_arr[0]= DAC_MOD40+Ratio_L*(-40-40); 
  	LUT_in_arr[1]= DAC_MOD40;
 	LUT_in_arr[2]= DAC_MOD40+Ratio_H*(120-40);
  
	for (i=0; i<fitNumber-1; i++)  //����slope��offset
	{
		slope_arr[i]  =(double)(LUT_in_arr[i+1]-LUT_in_arr[i])/(double)(Temper_in_arr[i+1]-Temper_in_arr[i]);
		offset_arr[i] =(double)(LUT_in_arr[i+1]-slope_arr[i]*Temper_in_arr[i+1]);
	}
	//������ұ�
	for (i=0; i<80; i++)
	{
		temper = i*2-40;
		
		for (j=0; j<fitNumber-1; j++) //�õ���Ӧ�¶ȵ�slope��offset
		{
			if (temper>Temper_in_arr[fitNumber-1])
			{
				slope  = slope_arr[fitNumber-2];
				offset = offset_arr[fitNumber-2];
				break;
			}
			if (temper<=Temper_in_arr[j+1])
			{
				slope  = slope_arr[j];
				offset = offset_arr[j];
				break;
			}
		}//for (j=0; j<fitNumber-1; j++)
		
		if (temper>TemperLimit_H) temper=TemperLimit_H;
		if (temper<TemperLimit_L) temper=TemperLimit_L;
		
		//��LUT����ֵ����0��255��Χ��ֵ����
		if ((temper*slope + offset)<0)        LUT_out_arr[i] = 0;
		else if ((temper*slope + offset)>255) LUT_out_arr[i] = 255;
		else                                  LUT_out_arr[i] = temper*slope + offset;
	}
	
	if (slope_arr != NULL) free(slope_arr);
	if (offset_arr != NULL) free(offset_arr); 
	
	//��ӵ����Լ��
	for(i=0;i<79;i++)
	{
		if ((LUT_out_arr[i+1]-LUT_out_arr[i]) < 0 ) 
		{MessagePopup ("Error", "ER���ұ����Լ��ʧ�ܣ�");return FALSE;}   
	}
	
	for(i=0;i<80;i++)
	{
		LUT_VSC[i+864]=DAC_APC;                   
		LUT_VSC[i+944]=LUT_out_arr[i];      
	}
	return TRUE;
}

int tuningSD_DWDM_ONU (const int channel, struct struTestData *data)			   //LOS DAC�ɴ���С����
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 5, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	INT8U   DACini=0;	   //���ڸ�DAC���¸���ֵ   zhi.xu20140423
	double  temper;
		
	errChk(DWDM_ONU_DS4830_Set_LOS_Manual(INST_EVB[channel])); 
		
	DACmax=0xFF;
	DACmin=0x0;
	
	DAC=DACmax;
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=1) 
	{
		MessagePopup ("ERROR", "Rsd=max SDD test fail !"); 
		goto Error;
	}
	
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd);  
	DAC=DACmin;
	
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=0) 
	{
		MessagePopup ("ERROR", "Rsd=min SDA test fail !"); 
		goto Error;
	} 
	
	//��ʼ����SDA  
	DAC=DACmax;		 
	
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	
	Delay(0.5);
	
	Read_RX_SD(channel, -1, &SD);
	if(SD !=0) 
	{
		j=0;
		do
		{
			do
			{
				DACstep=DACstep_Arr[j];
				DAC=DAC-DACstep;
				DAC=DAC<DACmin ? DACmin:DAC;//��ֹ���
				
				DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
				
				Read_RX_SD(channel, 0, &SD);  
				if (SD==0) break; 		//�õ���SDD��,��������ѭ��
			} while (DAC>DACmin);		//���û�е���Rsd����󲽳�����ѭ��
				
			if (DAC==DACmin)   break;  
			if (DACstep==DACstep_Arr[2])    break;   //�õ�����С��step
			
			DAC=DAC+DACstep;
			DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC); 
			
			SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
		
			SET_ATT_ONU(channel, temp_sd);
		
			j++;
		}while (DAC>DACmin);
	}
	
	//���Խ�������SDD״̬�����SDA����
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel]; 
	SET_ATT_ONU(channel, temp_sd); 
	Read_RX_SD(channel, 1, &SD);
	sprintf (buf, "Rsd=%d SDA test fail", DAC);
	if(SD != 1) 
	{
		MessagePopup ("Error", buf);
		goto Error;
	}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
	data->ucRsd=DAC;    
		
	errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &temper));
	
	errChk (DWDM_ONU_Save_LOS_Luk(channel, DAC, temper));
	        
	
	//дLOS���ұ�
	
	errChk (DWDM_ONU_DS4830_Set_LOS_Auto(INST_EVB[channel]));

	return 0;
	
Error:
	
	return -1;
}

int tuningSD_DWDM_ONU_Ex (const int channel, struct struTestData *data)
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 5, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	INT8U   DACini=0;	   //���ڸ�DAC���¸���ֵ   zhi.xu20140423
	double  temper;
		
	errChk(DWDM_ONU_DS4830_Set_LOS_Manual(INST_EVB[channel])); 
		
	DACmax=0xFF;
	DACmin=0x0;
	
	DAC=DACmax;
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=1) 
	{
		MessagePopup ("ERROR", "Rsd=max SDD test fail !"); 
		goto Error;
	}
	
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd);  
	DAC=DACmin;
	
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=0) 
	{
		MessagePopup ("ERROR", "Rsd=min SDA test fail !"); 
		goto Error;
	} 
	
	//��ʼ����SDD  
	DAC=DACini;		 
	
	DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	
	Delay(0.5);
	
	Read_RX_SD(channel, -1, &SD);
	if(SD !=1) 
	{
		j=0;
		do
		{
			do
			{
				DACstep=DACstep_Arr[j];
				DAC=DAC+DACstep;
				DAC=DAC>DACmax ? DACmax:DAC;//��ֹ���
				
				DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC);
				
				Read_RX_SD(channel, 0, &SD);  
				if (SD==1) break; 		//�õ���SDD��,��������ѭ��
			} while (DAC<DACmax);		//���û�е���Rsd����󲽳�����ѭ��
				
			if (DAC==DACmax)   break;  
			if (DACstep==DACstep_Arr[2])    break;   //�õ�����С��step
			
			DAC=DAC-DACstep;
			DWDM_ONU_DS4830_Set_LOS (INST_EVB[channel], DAC); 
			
			SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
		
			SET_ATT_ONU(channel, temp_sd);
		
			j++;
		}while (DAC<DACmax);
	}
	
	//���Խ�������SDD״̬�����SDA����
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel]; 
	SET_ATT_ONU(channel, temp_sd); 
	Read_RX_SD(channel, 1, &SD);
	sprintf (buf, "Rsd=%d SDA test fail", DAC);
	if(SD != 0) 
	{
		MessagePopup ("Error", buf);
		goto Error;
	}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
	data->ucRsd=DAC;    
		
	errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &temper));
	
	errChk (DWDM_ONU_Save_LOS_Luk(channel, DAC, temper));
	        
	
	//дLOS���ұ�
	
	errChk (DWDM_ONU_DS4830_Set_LOS_Auto(INST_EVB[channel]));

	return 0;
	
Error:
	
	return -1;
}

int tuningSD_Mega88 (const int channel, struct struTestData *data) 
{
	int error=0;
	
	switch (my_struDBConfig_ERCompens.DriverChip)
	{
		case DRIVERCHIP_VSC7967:
			error = tuningSD_Mega88_VSC7967 (channel, data);  
			break;
		
		case DRIVERCHIP_NT25L90:
			error = tuningSD_Mega88_NT25L90 (channel, data); 
		
			break;
			
		default:  
			MessagePopup ("��ʾ", "Mega88��Ʒ��֧�ִ���DriverоƬ"); 
			error = -1;
	}
	
	return error;
}

int tuningSD_UX3328 (const int channel, struct struTestData *data)
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 5, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	INT8U   DACini=30;	   //���ڸ�DAC���¸���ֵ   zhi.xu20140423
		
	errChk(ux3328_set_FactoryMode(INST_EVB[channel])); 
		
	errChk(ux3328_set_mode (INST_EVB[channel], 1));
	
	DACmax=0xFF;
	DACmin=0x0;
	
	DAC=DACmax;
	ux3328_set_los_dac (INST_EVB[channel], DAC);
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=0) {MessagePopup ("ERROR", "Rsd=max SDD test fail !"); goto Error;}
	
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd);  
	DAC=DACmin;
	ux3328_set_los_dac (INST_EVB[channel], DAC);
	error = Read_RX_SD(channel, 1, &SD);
	if(error<0 || SD !=1) {MessagePopup ("ERROR", "Rsd=min SDA test fail !"); goto Error;} 
	
	//��ʼ����SDD
	if(0 == strcmp(my_struConfig.PN ,"SO013462-FSGE") && 0 == strcmp(my_struConfig.BOM, "5200272C"))		  
	{
		 DACini = 30;
	}
	else 
	{
		DACini = DACmin;
	}
	DAC=DACini;		 //��DAC���¸���ֵDACini�������DACmin��ʼ���˷�ʱ��  zhi.xu20140423
	ux3328_set_los_dac (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	
	Delay(0.5);
	
	Read_RX_SD(channel, -1, &SD);
	if(SD !=0) 
	{
		j=0;
		do
		{
			do
			{
				DACstep=DACstep_Arr[j];
				DAC=DAC+DACstep;
				DAC=DAC>DACmax ? DACmax:DAC;//��ֹ���
				ux3328_set_los_dac (INST_EVB[channel], DAC);
				Read_RX_SD(channel, 0, &SD);  
				if (SD==0) break; 		//�õ���SDD��,��������ѭ��
			} while (DAC<DACmax);		//���û�е���Rsd����󲽳�����ѭ��
				
			if (DAC==DACmax)   break;  
			if (DACstep==DACstep_Arr[2])    break;   //�õ�����С��step
			
			DAC=DAC-DACstep;
			ux3328_set_los_dac (INST_EVB[channel], DAC); 
			
			SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
		
			SET_ATT_ONU(channel, temp_sd);
		
			j++;
		}while (DAC<DACmax);
	}
	
	//���Խ�������SDD״̬�����SDA����
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel]; 
	SET_ATT_ONU(channel, temp_sd); 
	Read_RX_SD(channel, 1, &SD);
	sprintf (buf, "Rsd=%d SDA test fail", DAC);
	if(SD != 1) {MessagePopup ("Error", buf); goto Error;}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
	data->ucRsd=DAC; 

	errChk (un3328_set_UserMode(INST_EVB[channel]));

	return 0;
	
Error:
	
	un3328_set_UserMode(INST_EVB[channel]);
	
	return -1;
}

int tuningSD_UX3328S (const int channel, struct struTestData *data)
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 5, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	INT8U   DACini=30;	   //���ڸ�DAC���¸���ֵ   zhi.xu20140423
		
	errChk(ux3328s_set_FactoryMode(INST_EVB[channel])); 
		
	errChk(ux3328s_set_mode (INST_EVB[channel], 1));
	
	DACmax=0xFF;
	DACmin=0x0;
	
	DAC=DACmax;
	ux3328s_set_los_dac (INST_EVB[channel], DAC);
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	error = Read_RX_SD(channel, 0, &SD);
	if(error<0 || SD !=0) {MessagePopup ("ERROR", "Rsd=max SDD test fail !"); goto Error;}
	
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd);  
	DAC=DACmin;
	ux3328s_set_los_dac (INST_EVB[channel], DAC);
	error = Read_RX_SD(channel, 1, &SD);
	if(error<0 || SD !=1) {MessagePopup ("ERROR", "Rsd=min SDA test fail !"); goto Error;} 
	
	//��ʼ����SDD
	if(0 == strcmp(my_struConfig.PN ,"SO013462-FSGE") && 0 == strcmp(my_struConfig.BOM, "5200272C"))		  
	{
		 DACini = 30;
	}
	else 
	{
		DACini = DACmin;
	}
	DAC=DACini;		 //��DAC���¸���ֵDACini�������DACmin��ʼ���˷�ʱ��  zhi.xu20140423
	ux3328s_set_los_dac (INST_EVB[channel], DAC);
	
	temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	
	Delay(0.5);
	
	Read_RX_SD(channel, -1, &SD);
	if(SD !=0) 
	{
		j=0;
		do
		{
			do
			{
				DACstep=DACstep_Arr[j];
				DAC=DAC+DACstep;
				DAC=DAC>DACmax ? DACmax:DAC;//��ֹ���
				ux3328s_set_los_dac (INST_EVB[channel], DAC);
				Read_RX_SD(channel, 0, &SD);  
				if (SD==0) break; 		//�õ���SDD��,��������ѭ��
			} while (DAC<DACmax);		//���û�е���Rsd����󲽳�����ѭ��
				
			if (DAC==DACmax)   break;  
			if (DACstep==DACstep_Arr[2])    break;   //�õ�����С��step
			
			DAC=DAC-DACstep;
			ux3328s_set_los_dac (INST_EVB[channel], DAC); 
			
			SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
		
			SET_ATT_ONU(channel, temp_sd);
		
			j++;
		}while (DAC<DACmax);
	}
	
	//���Խ�������SDD״̬�����SDA����
	temp_sd=my_struDBConfig.SDA+sRxCal.power[channel]; 
	SET_ATT_ONU(channel, temp_sd); 
	Read_RX_SD(channel, 1, &SD);
	sprintf (buf, "Rsd=%d SDA test fail", DAC);
	if(SD != 1) {MessagePopup ("Error", buf); goto Error;}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
	data->ucRsd=DAC; 
	
	errChk (un3328_set_UserMode(INST_EVB[channel]));

	return 0;
	
Error:
	
	un3328_set_UserMode(INST_EVB[channel]);
	
	return -1;
}

int tuningSD_Mega88_VSC7967 (const int channel, struct struTestData *data)
{
	int i, sdaok, sddok, status, error;
	unsigned char R7[3]={0x18,0x3A,0x58}, R8[3]={0x30,0x60,0x90}, SD, losArray[6];
 	char FileName[MAX_PATHNAME_LEN];
 
	//R7��R8����ֵΪ(28,48)�����鱸ѡֵΪ(18,30)��(3A,60)��(58,90)

	//��ȡLOS�����ļ�, δʹ�ó�ʼ����ֵ
	GetProjectDir (FileName);
	strcat (FileName,"\\data\\Mega88LOS.txt");
	error = FileToArray (FileName, losArray, VAL_UNSIGNED_CHAR, 6, 3, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_ROWS, VAL_ASCII);
	if (error) 
	{
		MessagePopup("��ʾ", "��Mega88LOS.txt�ļ��쳣"); 
		return -1;
	} 
	
	for (i=0; i<3; i++)
	{
		R7[i]=losArray[2*i+0]; 
		R8[i]=losArray[2*i+1];	
	}
		
	sdaok=FALSE;
	sddok=FALSE;
	data->ucRsd = my_struFirmware.VSC7965_FLASH[7];
	for (i=0; i<4; i++)
	{
		if (i>0) 
		{//�޸�R7��R8ֵ�� ʵ����ֻ�޸������Σ����ع̼�ʱ�������˳�ֵ
			 error = MEGA88_SET_fLOS (INST_EVB[channel], R7[i-1], R8[i-1]);
			 if (error) 
			 {
				 MessagePopup ("��ʾ", "����Mega88 fLOS �Ĵ����쳣"); 
				 return -1;
			 } 
			 data->ucRsd = R7[i-1]; 
		}
		
		//���õ�-50dB����֤����SDA��׼ȷ��
		SET_ATT_ONU(channel, -50);   
		
		SET_ATT_ONU(channel, my_struDBConfig.SDA+sRxCal.power[channel]); 
		
		error = Read_RX_SD (channel, 1, &SD);
		if (error<0) 
		{
			MessagePopup ("��ʾ", "��ȡSD״̬�쳣"); 
			return -1;
		}
		
		if (SD==1) 
		{
			sdaok=TRUE;
		}
		else	   
		{
			sdaok=FALSE;
			continue;
		}
	
		SET_ATT_ONU(channel, my_struDBConfig.SDD+sRxCal.power[channel]); 
		
		error = Read_RX_SD (channel, 0, &SD);
		if (error<0) 
		{
			MessagePopup ("��ʾ", "��ȡSD״̬�쳣"); 
			return -1;
		}
		
		if (SD==0) 
		{
			sddok=TRUE;
		}
		else	   
		{
			sddok=FALSE;
			continue;
		}
		
		if (sdaok && sddok)
		{
			break;
		}
	}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
		
	if (sdaok && sddok) 
	{
		return 0; 
	}
	else
	{
		return -1;
	}
}

int tuningSD_Mega88_NT25L90 (const int channel, struct struTestData *data)
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 6, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	BOOL    bSDOK = TRUE;
	double 	temp_sda;
	double  temp_sdd;  
	
	/***����SDǰ�Ȳ��ԣ�Fail�Ž��е���**Eric.Yao***/	  
	SET_ATT_ONU(channel, -50);//�����ù⹦�ʵ�-50dBm��ȷ����SDD״̬
	
	temp_sda = my_struDBConfig.SDA+sRxCal.power[channel];
	
	SET_ATT_ONU (channel, temp_sda);	  	//����SDA 
	//����SDA
	if (Read_RX_SD(channel, 1, &SD)<0) 
	{
		return FALSE;
	}
	
	if(0==SD) 
	{
		bSDOK = FALSE;
	}
	
	if (bSDOK)
	{
		temp_sdd = my_struDBConfig.SDD+sRxCal.power[channel];
		
		SET_ATT_ONU(channel, temp_sdd);	//����SDD 
		
		//����SDD
		if (Read_RX_SD(channel, 0, &SD)<0) 
		{
			return FALSE;  
		}
		
		if(1==SD)
		{
			bSDOK = FALSE;
		}
	}
	
	if (bSDOK)
	{
		DAC = my_struFirmware.NT25L90_REG_EEPROM[13]; /***��AutoDT_Spec_NT25L90��0x6D��ֵ**Eric.Yao***/
	} 
	/**********************************************/

	if (!bSDOK) 
	{
		DACmax=0x3F;
		DACmin=0x5;
	
		DAC=DACmax;
		MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
		temp_sd = my_struDBConfig.SDD + sRxCal.power[channel];  
		SET_ATT_ONU(channel, temp_sd); 
		error = Read_RX_SD(channel, 0, &SD);
		if(error<0 || SD !=0) 
		{
			MessagePopup ("ERROR", "Rsd=max SDD test fail !"); 
			return -1;
		}
	
		temp_sd = my_struDBConfig.SDA + sRxCal.power[channel];  
		SET_ATT_ONU(channel, temp_sd);  
		DAC=DACmin;
		MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
		error = Read_RX_SD(channel, 1, &SD);
		if((error<0) || (SD !=1)) 
		{
			MessagePopup ("ERROR", "Rsd=min SDA test fail !"); 
			return -1;
		} 
	
		//��ʼ����SDD
		temp_sd=my_struDBConfig.SDD+sRxCal.power[channel];  
		SET_ATT_ONU(channel, temp_sd); 
		
		Delay(0.5);
	
		Read_RX_SD(channel, -1, &SD);
		if(SD !=0) 
		{
			j=0;
			do
			{
				do
				{
					DACstep=DACstep_Arr[j];
					DAC=DAC+DACstep;
					DAC=DAC>DACmax ? DACmax:DAC;//��ֹ���
					MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
					Read_RX_SD(channel, 0, &SD);  
					if (SD==0) 
					{
						break; 		//�õ���SDD��,��������ѭ��
					}
				} while (DAC<DACmax);		//���û�е���Rsd����󲽳�����ѭ��
					
				if (DAC==DACmax)  
				{
					break;  
				}
				
				if (DACstep==2) 
				{
					break;   //�õ�����С��step
				}
				
				DAC=DAC-DACstep;
				MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC); 
				
				SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
			
				SET_ATT_ONU(channel, temp_sd);
			
				j++;
			}while (DAC<DACmax);
		}
	
		//���Խ�������SDD״̬�����SDA����
		temp_sd = my_struDBConfig.SDA+sRxCal.power[channel]; 
		SET_ATT_ONU(channel, temp_sd); 
		Read_RX_SD(channel, 1, &SD);
		sprintf (buf, "Rsd=%d SDA test fail", DAC);
		if(SD != 1)
		{
			MessagePopup ("Error", buf); 
			return -1;
		}

		//����LOS���õ�eeprom
		error = MEGA88NT25_SET_LOS_EEPROM (INST_EVB[channel], DAC); 
		if (error) 
		{
			MessagePopup ("��ʾ", "����LOS����ֵ��EEPROM�쳣"); 
			return -1;
		}  
	}
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = my_struDBConfig.SDD;
	data->SDHys = my_struDBConfig.SDA-my_struDBConfig.SDD;
	data->ucRsd=DAC; 
	
	return 0; 
}

int tuningSD_Mega88_NT25L90_TSSI (const int channel, struct struTestData *data)
{
	INT8U 	DACmax, DACmin, DACstep, DACstep_Arr[3]={20, 6, 2}, SD;
	int     DAC, status, j, error;
	double 	temp_sd;
	char    buf[1024]; 
	double 	temp_sda;
	double  temp_sdd; 
	float   SDD_CAL;
	
	if (data->Sens<-29.5)
	{
		SDD_CAL=2.;
	}
	else
	{
		SDD_CAL=1.2;
	}
	
	DACmax=0x3F;
	DACmin=0x5;

	DAC=DACmax;
	MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
	temp_sd = my_struDBConfig.SDD + sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd); 
	error = Read_RX_SD(channel, 0, &SD);
	if((error<0) || (SD !=0)) 
	{
		MessagePopup ("ERROR", "Rsd=max SDD test fail !");
		return -1;
	}

	temp_sd = my_struDBConfig.SDA + sRxCal.power[channel];  
	SET_ATT_ONU(channel, temp_sd);  
	DAC=DACmin;
	MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
	error = Read_RX_SD(channel, 1, &SD);
	if((error<0) || (SD !=1))
	{
		MessagePopup ("ERROR", "Rsd=min SDA test fail !"); 
		return -1;
	} 

	//��ʼ����SDD,��������+1db��Ϊ����Ŀ��
	temp_sd=data->Sens+sRxCal.power[channel]+SDD_CAL;  
	SET_ATT_ONU(channel, temp_sd); 
	
	Delay(0.5);

	Read_RX_SD(channel, -1, &SD);
	if(SD !=0) 
	{
		j=0;
		do
		{
			do
			{
				DACstep=DACstep_Arr[j];
				DAC=DAC+DACstep;
				DAC=DAC>DACmax ? DACmax:DAC;//��ֹ���
				MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC);
				Read_RX_SD(channel, 0, &SD);  
				if (SD==0)
				{
					break; 		//�õ���SDD��,��������ѭ��
				}
			} while (DAC<DACmax);		//���û�е���Rsd����󲽳�����ѭ��
				
			if (DAC==DACmax) 
			{
				break;  
			}
			
			if (DACstep==2) 
			{
				break;   //�õ�����С��step
			}
			
			DAC=DAC-DACstep;
			MEGA88NT25_SET_LOS_RAM (INST_EVB[channel], DAC); 
			
			SET_ATT_ONU(channel, -15+sRxCal.power[channel]);
		
			SET_ATT_ONU(channel, temp_sd);
		
			j++;
		}while (DAC<DACmax);
	}

	//���Խ�������SDD״̬�����SDA����
	temp_sd = my_struDBConfig.SDA+sRxCal.power[channel]; 
	SET_ATT_ONU(channel, temp_sd); 
	Read_RX_SD(channel, 1, &SD);
	sprintf (buf, "Rsd=%d SDA test fail", DAC);
	if(SD != 1) 
	{
		MessagePopup ("Error", buf); 
		return -1;
	}

	//����LOS���õ�eeprom
	error = MEGA88NT25_SET_LOS_EEPROM (INST_EVB[channel], DAC); 
	if (error) 
	{
		MessagePopup ("��ʾ", "����LOS����ֵ��EEPROM�쳣"); 
		return -1;
	}  
	
	data->SDA = my_struDBConfig.SDA;
	data->SDD = data->Sens+SDD_CAL;
	data->SDHys = data->SDA-data->SDD;
	data->ucRsd=DAC; 
	
	return 0; 
}

int Read_RX_SD(const int channel, const int SD_Target, BYTE *SD)
{
	BYTE temp, tempSD;
	int	i;

	if (SD_Target == -1)
	{  
		//����Ҫ��ȡ�ȶ���״̬��ֻ��һ�� 
		if (EVB5_READ_ONU_RxSD(INST_EVB[channel], &tempSD)<0) 
		{
			return -1;
		}
	}
	else //Ϊ��ȷ��SD�Ĳ���׼ȷ,��Ҫ��⼸��
	{
		i=0;
		do 
		{
			if (EVB5_READ_ONU_RxSD(INST_EVB[channel], &tempSD)<0) 
			{
				return -1;
			}
			
			if (SD_Target == tempSD) 
			{
				break;
			}
			
			i++;
			
			Delay(0.1);  //0.5sʱ���������Ϊ0.1s ���ܷ�ͨ������ zhi.xu20140416
			
		}while (i<10);
	}
	
	*SD=tempSD; 

	return 0;
}

int Read_Error_Bit(const int channel, unsigned int *ErrorCount, int TestFlag, double testtime, int InfoFlag, char ErrInfo[],char ErrInfo_Ex[])
{
	int		Lock, error, sBERT_Status_Code, count; 
	char 	sBERT_Status_String[500], buf[256];
	double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime;  

	strcpy (ErrInfo, "unknown error");

	switch (INSTR[channel].BERT_TYPE_ONU)
	{
		case BERT_TYPE_GBERT:
		{	if (TestFlag==0)
			{
				if (!GBERT_Check_Start(INST_BERT[channel], ErrorCount))
				{
					return -1;
				}
			}
			else
			{
				if (!GBERT_Check(INST_BERT[channel], ErrorCount))
				{
					return -1;
				}
			}
			break;
		}	
		case BERT_TYPE_SBERT: 
		{	
			error = EVB5_SBERT_Read(INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
			//			if (TestFlag==0 && (error==-2 || error==-4))
			if (TestFlag==0 && (error ||  (sBERT_Status_Code<0))) 
			{
				count = 0;
				do
				{
					//��ʼ���ԣ�����ͬ�����ϵ����⣬����BERT��ʼ������
					if (EVB5_SBERT_Init (INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS)<0)
					{
						return -1;
					}
				
					Delay (1); 
					
					if (EVB5_SBERT_Start(INST_BERT[channel], INSTR[channel].SBert_Rate, testtime)<0) 
					{
					//	MessagePopup("Error", "SBERT start test error!");
						return -1;
					}

					Delay (0.3);
					error = EVB5_SBERT_Read(INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
					count++;	/***�����ۼ�**Eric.Yao***/ 
				} while ((error || (sBERT_Status_Code<0)) && (count<10));
			}
			
			//����������С��5����Ϊû�����룬��ֹ����
			if ((sBERT_Status_Code==-3 || sBERT_Status_Code==-1) && sBERT_ERR_Number<21) 
			{
				if (0==InfoFlag)
				{
					if (TestFlag)
					{
						sprintf (buf, "ONU��������С���� %d", (int)sBERT_ERR_Number); 
						strcat (ErrInfo_Ex, buf);			  
					}
					else
					{
						sprintf (buf, "ONU��ʼ����С���� %d", (int)sBERT_ERR_Number);
						strcat (ErrInfo_Ex, buf);			
					}
				}
				
				sBERT_Status_Code=1;
				sBERT_ERR_Number =0;
				error = 0 ;
			}
			
			if (error<0 || sBERT_Status_Code<0) 
			{
			//	MessagePopup("Error", sBERT_Status_String);
				*ErrorCount=99;
				strcpy (ErrInfo, sBERT_Status_String);
				if (TestFlag)	//TestFlag=1 ���Թ����г����쳣����ʾ����У׼ϵͳ
				{
				//	MessagePopup("��ʾ", "������У׼����ϵͳ����һ��");
				}	
				else			//TestFlag=0 ���Ըտ�ʼ�����쳣����ʾ�����˶��� 
				{
				//	MessagePopup("��ʾ", "�����������˶�������һ��");
				}
				
				return -1;
			} 
			*ErrorCount=sBERT_ERR_Number;
			break;
		}
		default:
		{
			MessagePopup("Error", "BERT type error!");
			return -1;
		} 
	}
	 
	return 0;
}

int calTxPower_ux3328 (const int channel)
{
	float tx_dBm, slope;
	short int offset;
	int   error;
	double temp;
	
	double dAOPavg;
	float TxPow_mon;
	double delta = 0.5;

	errChk(ux3328_set_FactoryMode(INST_EVB[channel]));
	
	//��ȡ��ǰ���˹���
	errChk (Read_AOP(channel, &temp));

	/***���ӷ��˹⹦�����޼��**Eric.Yao**20131102***/ 
	/***���������չ��0.5dB�������������ȵ��¹⹦��ƫ��**Eric.Yao20131203***/ 
	if ((temp < my_struDBConfig.TxPowMin - delta) || (temp > my_struDBConfig.TxPowMax + delta))	
	{
		error = -1; 
		goto Error;  		 	/***����У׼��ȡ�⹦�ʲ���������Ҫ������Ϊ���ϸ�Eric.Yao**20131104***/
	}
	
	dAOPavg = (my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax) / 2.;   /***��������ֵ**Eric.Yao**20131025***/                   
	
	if (temp < dAOPavg)  		//��С������ֵ
	{
		if (((temp + 0.5) >= my_struDBConfig.TxPowMin) && ((temp + 0.5) <= my_struDBConfig.TxPowMax)) 
		{
			temp = temp + 0.5;	/***У׼ʱ��С������ֵ��������0.5dB��⹦�ʲ��������ݿ����ޣ�У׼�⹦��+0.5dB**Eric.Yao**20130731***/ 
		}
		else
		{
			;					/***���������ֵ���߹�����0.5dB�󳬹����ޣ���У׼�⹦�ʲ���0.5dB**Eric.Yao**20130731***/
		}
	}
	else
	{
		;
	}
	
	tx_dBm = temp; 
	
	//д�뷢��У׼ϵ��   
	errChk (ux3328_set_calibration_tx (INST_EVB[channel], tx_dBm, &slope, &offset));

	//д����
	errChk (ux3328_set_backup_A2_Table (INST_EVB[channel], 3));
	if (error) goto Error; 
	
	errChk (MyDLL_8472_GetTxPWR (INST_EVB[channel], &TxPow_mon));

	if (TxPow_mon < my_struDBConfig.TxCalMin || TxPow_mon > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
	{
		error = -1;
		goto Error;        
	}

Error:	
	
	un3328_set_UserMode(INST_EVB[channel]);
	
	return error; 	
}

int calTxPower_ux3328s (const int channel,struct struTestData *data)
{
	float tx_dBm, slope;
	short int offset;
	int   error,Table;
	double temp;
	
	double dAOPavg;
	float TxPow_mon;
	double delta = 0.5;

	errChk(ux3328s_set_FactoryMode(INST_EVB[channel]));	   
//	Delay(0.5);
	//���ò���ͨ��
	errChk(DWDM_ONU_Select_Channelindex(channel,0)); 
	Delay(3);
	//��ȡ��ǰ���˹���
	errChk (Read_AOP(channel, &temp));

	/***���ӷ��˹⹦�����޼��**Eric.Yao**20131102***/ 
	/***���������չ��0.5dB�������������ȵ��¹⹦��ƫ��**Eric.Yao20131203***/ 
	if ((temp < my_struDBConfig.TxPowMin - delta) || (temp > my_struDBConfig.TxPowMax + delta))	
	{
		error = -1; 
		goto Error;  		 	/***����У׼��ȡ�⹦�ʲ���������Ҫ������Ϊ���ϸ�Eric.Yao**20131104***/
	}
	
	dAOPavg = (my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax) / 2.;   /***��������ֵ**Eric.Yao**20131025***/                   
/*	
	if (temp < dAOPavg)  		//��С������ֵ
	{
		if (((temp + 0.5) >= my_struDBConfig.TxPowMin) && ((temp + 0.5) <= my_struDBConfig.TxPowMax)) 
		{
			temp = temp + 0.5;	//У׼ʱ��С������ֵ��������0.5dB��⹦�ʲ��������ݿ����ޣ�У׼�⹦��+0.5dB**Eric.Yao**20130731// 
		}
		else
		{
			;					//���������ֵ���߹�����0.5dB�󳬹����ޣ���У׼�⹦�ʲ���0.5dB**Eric.Yao**20130731//
		}
	}
	else
	{
		;
	}
*/	
	tx_dBm = temp; 
	
	//д�뷢��У׼ϵ��   
	errChk (ux3328s_set_calibration_tx (INST_EVB[channel], tx_dBm, &slope, &offset));

	//д����
//	errChk (ux3328s_set_backup_A2_Table (INST_EVB[channel], 3));
//	if (error) 
//	{
//		goto Error; 
//	}
	
	errChk (MyDLL_8472_GetTxPWR (INST_EVB[channel], &TxPow_mon));

	if (TxPow_mon < my_struDBConfig.TxCalMin || TxPow_mon > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
	{
		
		//д�뷢��У׼ϵ��   
		errChk (ux3328s_set_calibration_tx (INST_EVB[channel], tx_dBm, &slope, &offset));

		//д����
//		errChk (ux3328s_set_backup_A2_Table (INST_EVB[channel], 3));
//		if (error) goto Error; 
	
		errChk (MyDLL_8472_GetTxPWR (INST_EVB[channel], &TxPow_mon));

		if(!my_struConfig.DT_Tun_Reliability)
		{
			if (TxPow_mon < my_struDBConfig.TxCalMin || TxPow_mon > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
			{
				error = -1;
				goto Error;        
			}
		}
	}

Error:	
	
	ux3328s_set_UserMode(INST_EVB[channel]);
	
	return error; 	
}



int calTxPower1(const int channel)
{
double TxPow_uW[2], TxPow_DB[2];
INT16U ADC[2];
INT8U  DAC_r;
int error, i;
signed short gain, offset;
double x[30], y[30], d[30];	//���֧��30������ 
double a, b, e;
int  TemperOffset_lsb;
unsigned char myARR[256];

	double dAOPavg;
	double delta = 0.5;

	//��У׼ϵ������Ϊ��ʼֵ 
	gain  =0x0100;
	offset=0x0000;

	errChk (MEGA88_SET_TX_PWR (INST_EVB[channel], gain, offset));

	//���õ��ֶ�ģʽ
	errChk (MEGA88_SET_Mode_Manual(INST_EVB[channel])); 

	//Ϊ�˷�ֹMOD����������Ӱ�죬��Ҫ��DAC_MOD���͵�30��
 	errChk (SET_DAC_MOD(channel, 30)); 
	
	errChk (MEGA88_GET_DAC_APC(INST_EVB[channel], &DAC_r));  
	
	DAC_r=(DAC_r-60<0) ? 10:(DAC_r-60);
	errChk (SET_DAC_APC(channel, DAC_r));
	
//------------��ʱģ�鹤����burstģʽ�£�����ͨ��evb�������źţ���ģ��APC��������--------//
	//����EVB��BENģʽ
	errChk (SET_EVB5_MOD_BEN (channel));
//------------��ʱģ�鹤����burstģʽ�£�����ͨ��evb�������źţ���ģ��APC��������--------// 
	errChk (MEGA88_GET_TX_MON(channel, &ADC[0], &TxPow_DB[0]));

	//����EVB��Levelģʽ
	errChk (SET_EVB5_MOD_Level (channel));

	Delay (1.5); 
	
	errChk (Read_AOP(channel, &TxPow_DB[0]));
	
	TxPow_uW[0] = pow (10.0, (TxPow_DB[0]/10)) * 10000.; //convert from dBm to 0.1uW  
	
	//���õ�LUTģʽ
	errChk (MEGA88_SET_Mode_Auto (INST_EVB[channel]));  
	
//------------��ʱģ�鹤����burstģʽ�£�����ͨ��evb�������źţ���ģ��APC��������--------//
	//����EVB��BENģʽ
	errChk (SET_EVB5_MOD_BEN (channel));
//------------��ʱģ�鹤����burstģʽ�£�����ͨ��evb�������źţ���ģ��APC��������--------// 

	errChk (MEGA88_GET_TX_MON(INST_EVB[channel], &ADC[1], &TxPow_DB[1]));

	//����EVB��Levelģʽ
	errChk (SET_EVB5_MOD_Level (channel));

	Delay (1.5); 

	errChk (Read_AOP(channel, &TxPow_DB[1]));  
	/***���ӷ��˹⹦�����޼��**Eric.Yao**20131102***/
	/***���������չ��0.5dB�������������ȵ��¹⹦��ƫ��**Eric.Yao20131203***/ 
	if ((TxPow_DB[1] < my_struDBConfig.TxPowMin - delta) || (TxPow_DB[1] > my_struDBConfig.TxPowMax + delta))	
	{
		error = -1;
		goto Error;		 /***����У׼��ȡ�⹦�ʲ���������Ҫ������Ϊ���ϸ�Eric.Yao**20131104***/
	}

	dAOPavg = (my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax) / 2.;   /***��������ֵ**Eric.Yao**20131025***/                   
	
	if (TxPow_DB[1] < dAOPavg)  		//��С������ֵ
	{
		if (((TxPow_DB[1] + 0.5) >= my_struDBConfig.TxPowMin) && ((TxPow_DB[1] + 0.5) <= my_struDBConfig.TxPowMax)) 
		{
			TxPow_DB[1] = TxPow_DB[1] + 0.5;	 /***У׼ʱ��С������ֵ��������0.5dB��⹦�ʲ��������ݿ����ޣ�У׼�⹦��+0.5dB**Eric.Yao**20130731***/ 
		}
		else
		{
			;								 /***���������ֵ���߹�����0.5dB�󳬹����ޣ���У׼�⹦�ʲ���0.5dB**Eric.Yao**20130731***/
		}
	}
	else
	{
		;
	}
	
	TxPow_uW[1] = pow (10.0, (TxPow_DB[1]/10)) * 10000.; //convert from dBm to 0.1uW  
	
	for (i=0; i<2; i++) 
	{
		x[i]=ADC[i];
		y[i]=TxPow_uW[i];
	}
	LinFit (x, y, 2, d, &a, &b, &e); 
	
	gain  = a*256.;
	offset= b;

	errChk (MEGA88_SET_TX_PWR (INST_EVB[channel], gain, offset));

	
//��Ӳ�������, ��������ǰ��Ĳ��Թ⹦��ֵ
	//����EVB��BENģʽ
	errChk (SET_EVB5_MOD_BEN (channel));

	//����EVB��Levelģʽ
	errChk (SET_EVB5_MOD_Level (channel));

	errChk (MEGA88_GET_TX_MON(INST_EVB[channel], &ADC[0], &TxPow_DB[0]));
	
	if (fabs(TxPow_DB[1]-TxPow_DB[0])>my_struDBConfig_Monitor.Tx_Pow_Prec)
	{
		return -1;
	}
	
	if (TxPow_DB[0] < my_struDBConfig.TxCalMin || TxPow_DB[0] > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
	{
		return -1;
	}

	return 0;
Error:
	//����EVB��Levelģʽ
	SET_EVB5_MOD_Level (channel);
	 
	return error; 	
}

int calTxPower2(const int channel)
{
	double TxPow_uW[2], TxPow_DB[2];
	INT16U ADC[2];
	INT8U  DAC_r;
	int error, i;
	signed short gain, offset;
	double x[30], y[30], d[30];	//���֧��30������ 
	double a, b, e;
	int  TemperOffset_lsb;
	unsigned char myARR[256]; 

	double dAOPavg;
	double delta = 0.5;

	//��У׼ϵ������Ϊ��ʼֵ 
	gain  =0x0100;
	offset=0x0000;

	errChk (MEGA88_SET_TX_PWR (INST_EVB[channel], gain, offset));

	//���õ��ֶ�ģʽ
	errChk (MEGA88_SET_Mode_Manual(INST_EVB[channel])); 

	//Ϊ�˷�ֹMOD����������Ӱ�죬��Ҫ��DAC_MOD���͵�30��
 	errChk (SET_DAC_MOD(channel, 30));    
	
	errChk (MEGA88_GET_DAC_APC(INST_EVB[channel], &DAC_r));    
	
	DAC_r=(DAC_r-60<0) ? 10:(DAC_r-60);
	errChk (SET_DAC_APC(channel, DAC_r));     

	Delay (1.5); 

	errChk (MEGA88_GET_TX_MON(INST_EVB[channel], &ADC[0], &TxPow_DB[0]));
	
	errChk (Read_AOP(channel, &TxPow_DB[0]));     
	TxPow_uW[0] = pow (10.0, (TxPow_DB[0]/10)) * 10000.; //convert from dBm to 0.1uW  
	
	//���õ�LUTģʽ
	errChk (MEGA88_SET_Mode_Auto (INST_EVB[channel]));     
	
	Delay (1.5); 
		
	errChk (MEGA88_GET_TX_MON(INST_EVB[channel], &ADC[1], &TxPow_DB[1]));
	
	errChk (Read_AOP(channel, &TxPow_DB[1])); 
	
	/***���ӷ��˹⹦�����޼��**Eric.Yao**20131102***/ 
	/***���������չ��0.5dB�������������ȵ��¹⹦��ƫ��**Eric.Yao20131203***/ 
	if ((TxPow_DB[1] < my_struDBConfig.TxPowMin - delta) || (TxPow_DB[1] > my_struDBConfig.TxPowMax + delta))	
	{
		error = -1;
		goto Error;	 /***����У׼��ȡ�⹦�ʲ���������Ҫ������Ϊ���ϸ�Eric.Yao**20131104***/
	}
	
	dAOPavg = (my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax) / 2.;   /***��������ֵ**Eric.Yao**20131025***/                   
	
	if (TxPow_DB[1] < dAOPavg)  		//��С������ֵ
	{
		if (((TxPow_DB[1] + 0.5) >= my_struDBConfig.TxPowMin) && ((TxPow_DB[1] + 0.5) <= my_struDBConfig.TxPowMax)) 
		{
			TxPow_DB[1] = TxPow_DB[1] + 0.5;	 /***У׼ʱ��С������ֵ��������0.5dB��⹦�ʲ��������ݿ����ޣ�У׼�⹦��+0.5dB**Eric.Yao**20130731***/ 
		}
		else
		{
			;								 /***���������ֵ���߹�����0.5dB�󳬹����ޣ���У׼�⹦�ʲ���0.5dB**Eric.Yao**20130731***/
		}
	}
	else
	{
		;
	}
	
	TxPow_uW[1] = pow (10.0, (TxPow_DB[1]/10)) * 10000.; //convert from dBm to 0.1uW  
	
	for (i=0; i<2; i++) 
	{
		x[i]=ADC[i];
		y[i]=TxPow_uW[i];
	}
	LinFit (x, y, 2, d, &a, &b, &e); 
	
	gain  = a*256.;
	offset= b;

	errChk (MEGA88_SET_TX_PWR (INST_EVB[channel], gain, offset));

	errChk (MEGA88_GET_TX_MON(INST_EVB[channel], &ADC[0], &TxPow_DB[0]));

	if (fabs(TxPow_DB[1]-TxPow_DB[0])>my_struDBConfig_Monitor.Tx_Pow_Prec) return FALSE;
	
	if (TxPow_DB[0] < my_struDBConfig.TxCalMin || TxPow_DB[0] > my_struDBConfig.TxCalMax)		/***���ӷ��˼������ֵ��飬���޲���QA�⹦�ʱ�׼**Eric.Yao**20131025***/
	{
		error = -1;
		goto Error;
	}

Error:
	
	return error;
}

int SET_EVB5_MOD_BEN(const int channel)
{
	if (EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_CLOCK)<0) 
	{
		return -1;
	} 
	
	return 0;
}

int SET_EVB5_MOD_Level(const int channel)
{
	if (EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0) 
	{
		return -1;
	} 
	
	if (EVB5_SET_BEN_Level (INST_EVB[channel], my_struEVB5.BURST_ON )<0) 
	{
		return -1;
	} 
	
	return 0;
}

BOOL Read_Error_Bit_Ex(int channel, unsigned int *ErrorCount, int TestFlag)
{
int		error, sBERT_Status_Code; 
char 	sBERT_Status_String[500];
double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime;  

	switch (INSTR[channel].BERT_TYPE_ONU)
	{
		case BERT_TYPE_GBERT:
			if (TestFlag==0)
			{
				if (!GBERT_Check_Start(INST_BERT[channel], ErrorCount))
				{
					return FALSE;    	
				}
			}
			else
			{
				if (!GBERT_Check(INST_BERT[channel], ErrorCount))
				{
					return FALSE;
				}
			}
			break;
		case BERT_TYPE_SBERT: 
			error = EVB5_SBERT_Read(INST_BERT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
			
			//����������С��5����Ϊû�����룬��ֹ����
			if ((sBERT_Status_Code==-3 || sBERT_Status_Code==-1) && sBERT_ERR_Number<5) 
			{
				sBERT_Status_Code=1;
				sBERT_ERR_Number =0;
			}
			if (error<0 || sBERT_Status_Code<0) 
			{
				*ErrorCount=99;
				return FALSE;
			} 
			*ErrorCount=sBERT_ERR_Number;
			break;
		default:
			{
				MessagePopup("Error", "BERT type error!");
				return FALSE;
			} 
	}
	return TRUE;
}

int test_olt_errbit_start (int channel, int *starttime, char ErrInfo[])
{
	double temp_sens;
	BYTE   SD;
	int    error, status;
	unsigned int err_count;	
	
	temp_sens = my_struDBConfig.Sens +my_struCal_OLT.Power[channel]; 
	
	error = SET_ATT_OLT (channel, temp_sens);	  	//���������ȹ��� 
	if (error) 
	{
		return -1;
	}
	
/*	Read_RX_SD_Ex (INST_BERT[channel], 1, &SD);   									//���SD�ź�   0��ģ��los, 1:û��los  
	if(SD == 0) //0��ģ��los							//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;     
			
			error = SET_ATT_OLT (channel, temp_sens); 
			if (error) 
			{
				return -1;
			}
			
			Read_RX_SD_Ex (INST_BERT[channel], 1, &SD);
		} while (SD==0 && temp_sens<0);
	
		temp_sens = my_struCal_OLT.Power_In[channel]+my_struCal_OLT.Power[channel];
		
		error = SET_ATT_OLT (channel, temp_sens);	  	//���������ȹ��� 
		if (error)
		{
			return -1;
		}
		
		Read_RX_SD_Ex (INST_BERT[channel], 1, &SD); 
		if(SD == 0)	
		{
			return -1; 						//���������LOS�źţ�����
		}
	}
*/	
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		//���ȳ�ʼ��SBERT����֤��������
		error = EVB5_SBERT_Init(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS);
		if (error)
		{
			MessagePopup("��ʾ","SBERT����ʧ�ܣ�");
			return -1;
		} 
		
		Delay (1); 
		
		if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, my_struCal_OLT.time[channel])<0) 
		{
			MessagePopup("Error", "SBERT start test error!"); 
			return -1;
		}
	}
	else if(INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_GBERT)
	{
		if (!GBERT_Start(INST_BERT_OLT[channel], 1000))
		{
			MessagePopup("Error", "Reset GBERT error!");
			return -1;
		} 
	}
	else 
	{
		MessagePopup("Error", "can not find this BERT Type!");
		return -1;
	} 

	*starttime=Timer(); 
	
	Delay(0.3); 
	
	status = Read_Error_Bit_OLT(channel, &err_count, 0, my_struDBConfig.SensTime, 0, 1, ErrInfo); 
	if (!status)
	{
		MessagePopup("Error", "Sens test Start fail!"); 
		return -1;
	}

	if (err_count!=0)
	{
		MessagePopup("Error", "Sens test Start fail!");
		return -1;
	} 

	return 0;
}

int test_olt_errbit_end (int channel, int starttime, char ErrInfo[])
{
double 	temp_sens;
int		timer_star, timer, err_count, status, count=0;

	timer_star=starttime; 
	do
	{
		//��ȡ����
		Delay(1); 
		
		status = Read_Error_Bit_OLT(channel, &err_count, 1, 0, count, 1, ErrInfo);
		if (!status) 
		{
		//	MessagePopup("Error", "Sens test fail!"); 
			return -1;
		}  
		
		timer = Timer()- timer_star; 
		
		count++;
		
	} while (timer<my_struDBConfig.SensTime && err_count==0);
	
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{					   
		if (EVB5_SBERT_End(INST_BERT_OLT[channel])<0) 
		{
			MessagePopup("Error", "SBERT Set Stop Error!!");
			return -1;
		}
	}
	else if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_GBERT)
	{
		if (!GBERT_Stop(INST_BERT[channel])) 
		{
			MessagePopup("Error", "GBERT Set Stop Error!"); 
			return -1;
		} 	
	}
	else 
	{
		MessagePopup("Error", "can not find this BERT Type!"); 
		return -1;
	}

	
	//�жϲ��Խ��,����ֵ 
	if (timer<my_struCal_OLT.time[channel] || err_count>0) 
	{
		return -1; 
	}

	return 0;
}

int SET_ATT_ONU (int channel, double AttValue)
{   
	int error;
	int obtainedlock;
	
	switch (INSTR[channel].ATT_TYPE_ONU)
	{
		case ATT_TYPE_GVPM:
			
			AttValue = 0-AttValue; 
			if (AttValue>40) 
			{
				error = GVPM_SET_OutputEnable(INST_ATT_ONU[channel], 0);
				if (error) 
				{
					return -1;
				} 
			}
			else
			{
				error = GVPM_SET_LockAttenuate(INST_ATT_ONU[channel], 0-AttValue, my_struDBConfig.RxWavelength, 1);
				if (error) 
				{
					MessagePopup ("Error", "GVPM Set lock Att error!"); 
					return -1;
				}  
			}
			
			break; 	
		case ATT_TYPE_JW8504:  
			GetLock(ThreadLock_JW8504,&obtainedlock);
			AttValue = 0-AttValue;
			if (AttValue<0)
			{
				MessagePopup ("Error", "JW8504 Set Value error!");
				return -1; 
			}
			else
			{
				if(JW8504_set_lock(INST_ATT_ONU[channel],channel,0-AttValue)<0)
				{
					return -1;
				}
			}
			CmtReleaseLock(ThreadLock_JW8504);
			
			break;
		default:
			
			MessagePopup ("ERROR", "ATT Type error!"); 
			return -1;
		    
			break;			
	}
	
	return 0;
}

int SET_ATT_OLT (int channel, double AttValue)
{   
	int error;
	
	switch (INSTR[channel].ATT_TYPE_OLT)
	{
		case ATT_TYPE_GVPM:
			
			//AttValue = 0-AttValue; 
			error = GVPM_SET_LockAttenuate(INST_ATT_OLT[channel], AttValue, my_struDBConfig.TxWavelength, 1);
			if (error) 
			{
				MessagePopup ("Error", "GVPM Set lock Att error!"); 
				return -1;
			}  
			
			break; 				
			
		default:
			
			MessagePopup ("ERROR", "ATT Type error!"); 
			return -1;
		    
			break;			
	}
	
	return 0;
}

BOOL Read_RX_SD_Ex(int evb, const int SD_Target, BYTE *SD)
{
BYTE temp, tempSD;
int	i;

	if (SD_Target == -1) EVB5_READ_ONU_RxSD(evb, &tempSD);  //����Ҫ��ȡ�ȶ���״̬��ֻ��һ�� 
	else //Ϊ��ȷ��SD�Ĳ���׼ȷ,��Ҫ��⼸��
	{
		i=0;
		do 
		{
			if (EVB5_READ_ONU_RxSD(evb, &tempSD)<0) return FALSE;
			
			if (SD_Target == tempSD) break;
			i++;Delay(0.5);
		}while (i<10);
	}
	
	*SD=tempSD; 

	return TRUE;
}

BOOL Read_Error_Bit_OLT(int channel, unsigned int *ErrorCount, int TestFlag, double testtime, int InfoFlag, int messageflag, char ErrInfo[])
{
int		Lock, error, sBERT_Status_Code, count; 
char 	sBERT_Status_String[500], buf[256];
double 	sBERT_ERR_Number, sBERT_ERR_Ratio, sBERT_ElapsedTime;  

	strcpy (ErrInfo, "unknown error");

	switch (INSTR[channel].BERT_TYPE_OLT)
	{
		case BERT_TYPE_GBERT: 
			if (TestFlag==0)
			{
				if (!GBERT_Check_Start(INST_BERT_OLT[channel], ErrorCount))
				{
					return FALSE;    	
				}
			}
			else
			{
				if (!GBERT_Check(INST_BERT_OLT[channel], ErrorCount))
				{
					return FALSE;
				}
			}
			break;
		case BERT_TYPE_SBERT: 
			
			error = EVB5_SBERT_Read(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
#if 0	  //	ȡ��������ʼ����   wenyao.xi 2015-11-19  
			if (TestFlag==0 && (error || (sBERT_Status_Code<0)))
			{
				count = 0;
				do
				{
					//��ʼ���ԣ�����ͬ�����ϵ����⣬����BERT��ʼ������
					if (EVB5_SBERT_Init (INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS)<0) 
					{
						return FALSE;
					}
				
					Delay (1); 
					
					if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, testtime)<0) 
					{
						MessagePopup("Error", "SBERT start test error!");
						return FALSE;
					}

					Delay (0.3);   
					error = EVB5_SBERT_Read(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS, &sBERT_Status_Code, sBERT_Status_String, &sBERT_ERR_Number, &sBERT_ERR_Ratio, &sBERT_ElapsedTime); 
					count++;	/***�����ۼ�**Eric.Yao***/           
				} while ((error || (sBERT_Status_Code<0)) && (count<10));
			}
#endif			
			//����������С��5����Ϊû�����룬��ֹ����
		//	if ((sBERT_Status_Code==-3 || sBERT_Status_Code==-1) && sBERT_ERR_Number<21) 
			if ((sBERT_Status_Code==-3 || sBERT_Status_Code==-1) && sBERT_ERR_Number<0) 
			{
				if (0==InfoFlag) 
				{
					if (TestFlag)
					{
						sprintf (buf, "OLT��������С���� %d", (int)sBERT_ERR_Number);
						strcat (my_struProcessLOG.Comments, buf);			
					}
					else
					{
						sprintf (buf, "OLT��ʼ����С���� %d", (int)sBERT_ERR_Number);
						strcat (my_struProcessLOG.Comments, buf); 			
					}
				}
				sBERT_Status_Code=1;
				sBERT_ERR_Number =0;
			}									
			if (error<0 || sBERT_Status_Code<0) 
			{
				*ErrorCount=99;
				strcpy (ErrInfo, sBERT_Status_String);
				if (messageflag)
				{
					MessagePopup("Error", sBERT_Status_String);
					if (TestFlag)	//TestFlag=1 ���Թ����г����쳣����ʾ����У׼ϵͳ
					{
						MessagePopup("��ʾ", "������У׼����ϵͳ����һ��");
					}	
					else			//TestFlag=0 ���Ըտ�ʼ�����쳣����ʾ�����˶��� 
					{
						MessagePopup("��ʾ", "�����������˶�������һ��");
					}
				}
				
				return FALSE;
			} 
			*ErrorCount=sBERT_ERR_Number;
			break;
		
		default:
			{
				MessagePopup("Error", "BERT type error!");
				return FALSE;
			} 
	}
	 
	return TRUE;
}

BOOL testOEye_Mask_Threshold(const int channel, struct struTestData *data)
{
	int maskmargin_threshold;
	
	char filename[256], str[1024], strip[1024];               
	
	switch (INSTR[channel].DCA_TYPE)
	{
		case DCA_TYPE_CSA8000:
			maskmargin_threshold = my_struCSA8000.MaskMargin;      
			break;
		case DCA_TYPE_Ag86100:
			maskmargin_threshold = my_struAg86100.MaskMargin;                    
			break;
		default:
			return -1;
			break;
	}

	if (!GET_MaskHits(INSTR[channel].DCA_TYPE, maskmargin_threshold, 200)) 
	{
		return -1;
	}
	
	data->pathpenalty = maskmargin_threshold;
	
	/***��¼��ͼ**Eric.Yao**20141111***/
	GetProjectDir (str); 
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)  
	{
		strcat (str, "\\data\\screen.jpg");  
	}
	else if (INSTR[channel].DCA_TYPE == DCA_TYPE_CSA8000)  
	{
		strcat (str, "\\data\\eyegraph.png");
	}
	else
	{
		MessagePopup("�澯��ʾ","����ͼʧ�ܣ�");
		return -1;
	}

	//save eye
	if(!Save_DCA_Eye (INSTR[channel].DCA_TYPE, str)) 
	{
		MessagePopup("�澯��ʾ","����ͼʧ�ܣ�");
		return -1; 
	}     
	else
	{
		GetEyePicture(str, data);
	}
	
	if (!Set_DCA_Run(INSTR[channel].DCA_TYPE)) 
	{
		return -1; 
	}
	
	return 0;   
}   

int testOEye_Mask(const int channel, struct struTestData *data)
{
	int error;
	int maskmargin;
	
	int maskmargin_init;
	
	char filename[256], str[1024], strip[1024];          
	
	switch (INSTR[channel].DCA_TYPE)
	{
		case DCA_TYPE_CSA8000:
			maskmargin_init = my_struCSA8000.MaskMargin;      
			break;
		case DCA_TYPE_Ag86100:
			maskmargin_init = my_struAg86100.MaskMargin;                    
			break;
		default:
			return -1;
			break;
	}
	
	error = Get_Real_Maskmagin(channel, maskmargin_init, 4, 1000, &maskmargin);
	if (error)
	{
		return -1;
	}
	data->pathpenalty = maskmargin;
	
	/***��¼��ͼ**Eric.Yao**20141111***/
	/***��¼��ͼ**Eric.Yao**20141111***/
	GetProjectDir (str); 
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)  
	{
		strcat (str, "\\data\\screen.jpg");  
	}
	else if (INSTR[channel].DCA_TYPE == DCA_TYPE_CSA8000)  
	{
		strcat (str, "\\data\\eyegraph.png");
	}
	else
	{
		MessagePopup("�澯��ʾ","����ͼʧ�ܣ�");
		return -1;
	}

	//save eye
	if(!Save_DCA_Eye (INSTR[channel].DCA_TYPE, str)) 
	{
		MessagePopup("�澯��ʾ","����ͼʧ�ܣ�");
		return -1; 
	}     
	else
	{
		GetEyePicture(str, data);
	}
	
	if (!Set_DCA_Run(INSTR[channel].DCA_TYPE)) 
	{
		return -1; 
	}
	
	if (data->pathpenalty < maskmargin_init)
	{
		return -1;
	}

	return 0;   
}

int Get_Real_Maskmagin(int channel, int MaskMargin_init,int step,int waveform,int *real_margin)
{
	char command_str[200], value_str[100];
    int count=0, flag, totalcount=0, MaskMargin, real;
			
	if (!Set_DCA_Run(INSTR[channel].DCA_TYPE))
	{
		return -1;    
	}
	
	flag = Get_EYE(channel, waveform, MaskMargin_init);
	if(1 == flag)
	{
		flag=Get_EYE_Clear(channel, waveform, MaskMargin_init); 
	}
	ProcessDrawEvents ();  
	if(0 == flag)
	{
		if (!Set_DCA_Stop(INSTR[channel].DCA_TYPE))
		{
			return -1;
		}
		
		count = 0;
		totalcount=0;
		MaskMargin=MaskMargin_init;
		do
		{
			MaskMargin=MaskMargin + step;
		
			if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin))
			{
				return -1;   
			}
			Delay(1); 
			if (!Get_DCA_MaskCount(INSTR[channel].DCA_TYPE, &totalcount)) 
			{
				return -1;        
			}
			
			count++;
			ProcessDrawEvents ();  
		}while((totalcount == 0) && (count<=20));
		
		if(count>20) 
		{
			MessagePopup ("ERROR", "ѭ��20�λ�û�е���!    ");  
			return -1; 
		}  
		 
		if (0 != totalcount)
		{
		//	count = 0;
			totalcount = 0;
			
			step = step / 2;
			MaskMargin=MaskMargin - step;
			if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin))
			{
				return -1;  
			}
			
			Delay(1); 
			if (!Get_DCA_MaskCount(INSTR[channel].DCA_TYPE, &totalcount)) 
			{
				return -1; 
			}
			ProcessDrawEvents ();  
			
			if (0 == totalcount)
			{
				step = step / 2;
				MaskMargin=MaskMargin + step;
			}
			else
			{
				step = step / 2;
				MaskMargin=MaskMargin - step; 
			}
			if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin)) 
			{
				return -1;
			}
			Delay(1); 
			if (!Get_DCA_MaskCount(INSTR[channel].DCA_TYPE, &totalcount)) 
			{
				return -1;  
			}
			
			if (0 == totalcount)
			{
				real = MaskMargin;
			}
			else
			{
				real = MaskMargin - 1;  
			}
		
			*real_margin = real;
			MaskMargin = real;
			
			if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin)) 
			{
				return -1;  
			}
			
			Delay(1); 
	    }
	}
	else
	{
	//	MessagePopup ("ERROR", "MaskMargin����ʼֵ���е���!    ");
		return -2;
	}	
	return 0;
}
 
BOOL Save_DCA_Eye(int DCAType, char *filename)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_Transfer_GPIB (inst_CSA8000, "C:\\MY DOCUMENTS\\testfist.png", filename)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
		//	if(!Ag86100_Set_AutoScale(inst_Ag86100)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL GetEyePicture(char *FilePath, struct struTestData *data)
{                
	int bitmapID, filehandle;
	char *buf;
	int  error, filezise;
	int cnt;

	int plotHandle;
                       
	error = GetFileInfo (FilePath, &filezise);
	if (error==0) 
	{
		return FALSE;
	}
	
	filehandle = OpenFile (FilePath, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_BINARY);
	if (filehandle<0) 
	{
		return FALSE;
	}
	
	buf=calloc (filezise, sizeof(char)); 
	if (buf==NULL) 
	{
		return FALSE;
	}
	
	error = ReadFile (filehandle, buf, filezise);
	if (error<0) 
	{
		return FALSE;
	}
	
	data->curvesize = filezise; 

	if (filezise>150000) MessagePopup("Error", "��ȡ��ͼ�ļ����󣬶�����ļ����Ȳ���"); 

	for(cnt = 0;cnt < filezise;cnt++)
	{
	    data->curve[cnt] = buf[cnt];    
	}
   	
	if (buf!=NULL) 
	{
		free (buf);
	}
	
	error = CloseFile (filehandle);
	if (error<0) 
	{
		return FALSE; 
	}
	
	return TRUE;   
}

BOOL Set_DCA_Run(int DCAType)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_SET_State_ON(inst_CSA8000)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_Set_Run(inst_Ag86100)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

int Get_EYE(const int channel, int WaveForms, int MaskMargin)   
{
    int i=0, rcount=0;
    char buf[128]="";
    int count=0, totalcount=0, wavecount=0;
	BOOL Status;
    
	if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin)) 
	{
		return -1; 
	}
	
	Delay(1); 
	//set AUTOSet EXECute
	if (!Set_DCA_AutoSet(INSTR[channel].DCA_TYPE))
	{
		return -1;
	}
	Delay(5);
	
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)
	{
		WaveForms = 200;
	}
	
	do
	{
		if (!Get_DCA_Waveform(INSTR[channel].DCA_TYPE, &wavecount))
		{
			return -1;     
		}
	
		if (!Get_DCA_MaskCount(INSTR[channel].DCA_TYPE, &totalcount)) 
		{
			return -1;  
		}
		if(totalcount!=0)   
		{
			return 1; 
		}
		
		if (wavecount > WaveForms)
		{
			break;
		}
											 
		Delay(0.5); 
		ProcessDrawEvents ();  
	
	} while (wavecount < WaveForms);		 //��ȡ����

    return 0;        
}

int Get_EYE_Clear(const int channel, int WaveForms, int MaskMargin)   
{
    int i=0, rcount=0;
    char buf[128]="";
    int count=0, totalcount=0, wavecount=0;
	BOOL Status;
    
	if (!Set_DCA_Clear(INSTR[channel].DCA_TYPE)) 
	{
		return -1;
	}
	
	if (!Set_MaskMargin(INSTR[channel].DCA_TYPE, MaskMargin)) 
	{
		return -1; 
	}
	Delay(1); 
	
	if (INSTR[channel].DCA_TYPE == DCA_TYPE_Ag86100)
	{
		WaveForms = 200;
	}
	
	do
	{
		if (!Get_DCA_Waveform(INSTR[channel].DCA_TYPE, &wavecount)) 
		{
			return -1; 
		}
	
		if (!Get_DCA_MaskCount(INSTR[channel].DCA_TYPE, &totalcount)) 
		{
			return -1;  
		}
		if(totalcount!=0) 
		{
			return 1;  
		}
		
		if (wavecount > WaveForms)
		{
			break;
		}
		
		Delay(0.5);
		
		ProcessDrawEvents ();  
	} while (wavecount < WaveForms);		 //��ȡ����

    return 0;        
}

BOOL Set_DCA_Stop(int DCAType)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_SET_State_OFF(inst_CSA8000)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_Set_Stop(inst_Ag86100)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL Set_MaskMargin(int DCAType, int MaskMargin)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_SET_MaskMargin(inst_CSA8000, MaskMargin, my_struCSA8000.MaskName)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_SET_MaskMargin(inst_Ag86100, MaskMargin, my_struAg86100.MaskName)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL Get_DCA_MaskCount(int DCAType, int *totalcount)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_GET_MaskCount_Ex(inst_CSA8000, totalcount)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_GET_MaskFSample(inst_Ag86100, totalcount)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL Set_DCA_AutoSet(int DCAType)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_SET_AUTOSet_EXECute(inst_CSA8000)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_Set_AutoScale(inst_Ag86100)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL Get_DCA_Waveform(int DCAType, int *wavecount)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if(!CSA8000_GET_WaveForms(inst_CSA8000, wavecount)) return FALSE; 
			break;
		case DCA_TYPE_Ag86100:
			if(!Ag86100_GET_WaveForms(inst_Ag86100, wavecount)) return FALSE; 
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

BOOL GET_MaskHits(int DCAType, double MaskMargin, int WaveForms)
{
	switch (DCAType)
		{
		case DCA_TYPE_CSA8000:
			if (!CSA8000_SET_MaskMargin(inst_CSA8000, MaskMargin, "ENET1250")) return FALSE;
			if (!CSA8000_GET_MaskCount(inst_CSA8000, 2000)) return FALSE;
			break;
		case DCA_TYPE_Ag86100:
			if (!Ag86100_SET_MaskMargin(inst_Ag86100, MaskMargin, "\"01xGbEthernet.msk\"")) return FALSE;
			if (!Ag86100_GET_MaskHits(inst_Ag86100, 200)) return FALSE;
			break;
		default:
			return FALSE;
			break;
		}
	return TRUE;
}

int CalibrateRxPower_MEGA88 (int channel, struct struTestData *data, int panel)
{
	signed short gain, offset; 
	int error, i, count;
	INT16U ADC, ADC_arr[16];
	double RxPow, RxPowMax, x[30], y[30], d[30], RxPow_mon_DB[30], x1[30], y1[30];	//���֧��30������ 
	double a, b, e;
	struct M8_RxCaliData_DEF RxCaliData[16];
		
	//������gain=0x0100��offset=0x0000
	gain  =0x0100;
	offset=0x0000;
	
	error = MEGA88_SET_RX_PWR (INST_EVB[channel], gain, offset);
	if (error)
	{
		return -1;
	}
	
	//����У׼�����Ϊ0
	error = MEGA88_SET_RX_PWR_CaliNo (INST_EVB[channel], 0);
	if (error) 
	{
		return -1;
	}

	//��ʼУ׼
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber; i++) 
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint[i]+sRxCal.power[channel]);
		
		Delay(0.5);
		
		error = MEGA88_GET_RX_MON (INST_EVB[channel], &ADC, &RxPow);
		if (error) 
		{
			return -1; 
		}
		
		x[i] = (double)ADC;
		y[i] = (double)(pow (10.0, (my_struDBConfig_Monitor.CaliPoint[i]/10.+4.)));		//1LSB=0.1uW 
	
		//�����ն˷ֶ����
		ADC_arr [i]= ADC;
	}
	
	LinFit (x, y, my_struDBConfig_Monitor.CaliNumber, d, &a, &b, &e); 
	
	gain  = a*256.;
	offset= b;
	
	error = MEGA88_SET_RX_PWR (INST_EVB[channel], gain, offset);
	if (error)
	{
		return -1;
	}

	//��ȡ�ֶ����ϵ��
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber-1; i++)  
	{
		x1[0] = x[i];
		y1[0] = y[i];
		
		x1[1] = x[i+1];
		y1[1] = y[i+1];
		
		LinFit (x1, y1, 2, d, &a, &b, &e);  
						
		RxCaliData[i].K = a*256.;
		RxCaliData[i].B = b;
	}
	
	error = MEGA88_SET_RX_PWR_CaliNo (INST_EVB[channel],my_struDBConfig_Monitor.CaliNumber-1);
	if (error)
	{
		return -1;
	}
	
	error = MEGA88_SET_RX_PWR_MultiSegmentFit (INST_EVB[channel], my_struDBConfig_Monitor.CaliNumber, ADC_arr, RxCaliData);
	if (error) 
	{
		return -1;
	}
	
	//��ʼ���У׼��
	RxPowMax=0; 
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber_Test; i++) 
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint_Test[i]+sRxCal.power[channel]);	
		Delay(0.3);
		
		error = MEGA88_GET_RX_MON (INST_EVB[channel], &ADC,  &RxPow_mon_DB[i]);  
		if (error)
		{
			return -1; 
		}
		RxPow = fabs(RxPow_mon_DB[i]-my_struDBConfig_Monitor.CaliPoint_Test[i]);
		
		//�����ݿ����ò�����������ָ�� 
		if (RxPow>my_struDBConfig_Monitor.Rx_Pow_Prec)  //����ֵ��������ָ�꣬�ظ�����5�Σ�ֱ��ͨ������
		{
			count=0;
			do
			{
				error = MEGA88_GET_RX_MON (INST_EVB[channel], &ADC,  &RxPow_mon_DB[i]);  
				if (error) 
				{
					return -1; 
				}
				RxPow = fabs(RxPow_mon_DB[i]-my_struDBConfig_Monitor.CaliPoint_Test[i]);
				if (RxPow<=my_struDBConfig_Monitor.Rx_Pow_Prec) 
				{
					break;
				}
				count++;
				Delay(0.1);
			} while(count<5);
			if (count >=5 )
			{
				MessagePopup ("Error", "RxCal test Error!"); 
				return -1;
			}
		} 
		RxPowMax = RxPow>RxPowMax ? RxPow:RxPowMax; //������������ֵ����
	}
	
	//�����ݿ����ò�����������ָ�� 
	if (RxPowMax>my_struDBConfig_Monitor.Rx_Pow_Prec) 
	{
		return -1;  
	}
	
	return 0;
}

int CalibrateRxPower_UX3328 (int channel, struct struTestData *data, int panel)
{
	int i, error, count;
	double RxPow=-99, RxPowMax=0, Rx_arr[4];
	float  RxPow_mon;
	char   Info[256]; 
	union uUX3328 UX3328;  
	unsigned short int ADC_arr[4];
	
	errChk(ux3328_set_FactoryMode(INST_EVB[channel])); 
	
	errChk (ux3328_select_table (INST_EVB[channel], 3));

	//���ux3328���ն�У׼ϵ��
	errChk (ux3328_set_calibration_rx_clear (INST_EVB[channel]));

	strcpy (Info, "���   У׼�㣨dBm��   ����ֵ��HEX��");
	Insert_Info(panel, gCtrl_BOX[channel], Info);     
	
	//��ʼУ׼ 
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber; i++)
	{
		SET_ATT_ONU(channel, RxPow+sRxCal.power[channel]);     
		
		Delay(1.5);
		
		errChk (I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, UX3328.pStr)); 

		ADC_arr[i] = UX3328.pStr[104]*256+UX3328.pStr[105]; 
		Rx_arr[i] =  (double)my_struDBConfig_Monitor.CaliPoint[i];
		
		sprintf (Info, "%02d:   %.2f   0x%03X", i+1, my_struDBConfig_Monitor.CaliPoint[i], ADC_arr[i]); 
		Insert_Info(panel, gCtrl_BOX[channel], Info);     
	}

	//��������Ĺ⹦�ʣ�����ն�У׼
	errChk (ux3328_set_calibration_rx_multi (INST_EVB[channel], Rx_arr, ADC_arr));

	errChk (ux3328_set_backup_A2_Table (INST_EVB[channel], 3));

	//�ȴ������㹻�ȶ�
	Delay (1);
	
	strcpy (Info, "���   У׼�㣨dBm��   ����ֵ��dBm��");
	Insert_Info(panel, gCtrl_BOX[channel], Info);

	//��ʼ���У׼��
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber_Test; i++)
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint_Test[i]+sRxCal.power[channel]);
		Delay(0.3);

		error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
		if (error<0) 
		{
			goto Error; 
		}
		data->RxPow_mon[i] = RxPow_mon;
		
		RxPow = fabs(RxPow_mon-my_struDBConfig_Monitor.CaliPoint_Test[i]); 
		
		if (RxPow>my_struDBConfig_Monitor.Rx_Pow_Prec)
		{
			count=0;
			do
			{
				error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
				if (error<0) 
				{
					goto Error; 
				}
				
				data->RxPow_mon[i] = RxPow_mon;
				
				RxPow = fabs(RxPow_mon-my_struDBConfig_Monitor.CaliPoint_Test[i]); 

				if (RxPow<=my_struDBConfig_Monitor.Rx_Pow_Prec)
				{
					break; 
				}
				
				count++;
				Delay(0.1);
				
			} while(count<5);
			
			if (count >=5 )
			{
				MessagePopup ("Error", "RxCal test Error!");
				data->RxPow_mon[i] = RxPow_mon;
				sprintf (Info, "%02d:   %.2f   %.2f", i+1, my_struDBConfig_Monitor.CaliPoint_Test[i], data->RxPow_mon[i]); 
				Insert_Info(panel, gCtrl_BOX[channel], Info);
				
				error = -1;  
				goto Error;
			}
		} 
		RxPowMax = RxPow>RxPowMax ? RxPow:RxPowMax; //������������ֵ����
		
		data->RxPow_mon[i] = RxPow_mon;
		sprintf (Info, "%02d:   %.2f   %.2f", i+1, my_struDBConfig_Monitor.CaliPoint_Test[i], data->RxPow_mon[i]); 
		Insert_Info(panel, gCtrl_BOX[channel], Info);
	}

	if (RxPowMax>my_struDBConfig_Monitor.Rx_Pow_Prec) 
	{
		error = -1;  
		goto Error;
	}
	
Error:	
	
	un3328_set_UserMode(INST_EVB[channel]);
	
	return error; 	
}

int CalibrateRxPower_UX3328S (int channel, struct struTestData *data, int panel)
{
	int i, error, count;
	double RxPow=-99, RxPowMax=0, Rx_arr[16];
	float  RxPow_mon;
	char   Info[256]; 
	union uUX3328S UX3328S;  
	unsigned short int ADC_arr[16];
	
	//Set TxDisable
	errChk(EVB5_SET_BEN_Level (INST_EVB[channel], 1));
		
	errChk(ux3328s_set_FactoryMode(INST_EVB[channel])); 
//	Delay(0.3);
//	errChk (ux3328s_select_table (INST_EVB[channel], 3));

	//���ux3328���ն�У׼ϵ��
	errChk (ux3328s_set_calibration_rx_clear (INST_EVB[channel]));

	strcpy (Info, "���   У׼�㣨dBm��   ����ֵ��HEX��");
	Insert_Info(panel, gCtrl_BOX[channel], Info);     
	
	//��ʼУ׼ 
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber; i++)
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint[i]+sRxCal.power[channel]);     
		
//		Delay(1.5);
		Delay(0.3); 
		
		errChk (I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, UX3328S.pStr)); 

		ADC_arr[i] = UX3328S.pStr[104]*256+UX3328S.pStr[105]; 
		Rx_arr[i] =  (double)my_struDBConfig_Monitor.CaliPoint[i];
		
		sprintf (Info, "%02d:   %.2f   0x%04X", i+1, my_struDBConfig_Monitor.CaliPoint[i], ADC_arr[i]); 
		Insert_Info(panel, gCtrl_BOX[channel], Info);     
	}

	//��������Ĺ⹦�ʣ�����ն�У׼
	errChk (ux3328s_set_calibration_rx_multi (INST_EVB[channel], Rx_arr, ADC_arr));
//	Delay(0.3);
//	errChk (ux3328s_set_backup_A2_Table (INST_EVB[channel], 3));

	//�ȴ������㹻�ȶ�
//	Delay (1);
	
	strcpy (Info, "���   У׼�㣨dBm��   ����ֵ��dBm��");
	Insert_Info(panel, gCtrl_BOX[channel], Info);

	//��ʼ���У׼��
	for (i=0; i<my_struDBConfig_Monitor.CaliNumber_Test; i++)
	{
		SET_ATT_ONU(channel, my_struDBConfig_Monitor.CaliPoint_Test[i]+sRxCal.power[channel]);
		Delay(0.3);

		error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
		if (error<0) 
		{
			goto Error; 
		}
		data->RxPow_mon[i] = RxPow_mon;
		
		RxPow = fabs(RxPow_mon-my_struDBConfig_Monitor.CaliPoint_Test[i]); 
		
		if (RxPow>my_struDBConfig_Monitor.Rx_Pow_Prec)
		{
			count=0;
			do
			{
				error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
				if (error<0) 
				{
					goto Error; 
				}
				
				data->RxPow_mon[i] = RxPow_mon;
				
				RxPow = fabs(RxPow_mon-my_struDBConfig_Monitor.CaliPoint_Test[i]); 

				if (RxPow<=my_struDBConfig_Monitor.Rx_Pow_Prec)
				{
					break; 
				}
				
				count++;
				Delay(0.1);
				
			} while(count<5);
			
			if (count >=5 )
			{
			//	MessagePopup ("Error", "RxCal test Error!");
				if(RxPow_mon<-60.)
				{
					RxPow_mon=-50.;
				}
				data->RxPow_mon[i] = RxPow_mon;
				sprintf (Info, "%02d:   %.2f   %.2f", i+1, my_struDBConfig_Monitor.CaliPoint_Test[i], data->RxPow_mon[i]); 
				Insert_Info(panel, gCtrl_BOX[channel], Info);
				
				error = -1;  
				goto Error;
			}
		} 
		RxPowMax = RxPow>RxPowMax ? RxPow:RxPowMax; //������������ֵ����
		
		data->RxPow_mon[i] = RxPow_mon;
		sprintf (Info, "%02d:   %.2f   %.2f", i+1, my_struDBConfig_Monitor.CaliPoint_Test[i], data->RxPow_mon[i]); 
		Insert_Info(panel, gCtrl_BOX[channel], Info);
	}

	if (RxPowMax>my_struDBConfig_Monitor.Rx_Pow_Prec) 
	{
		error = -1;  
		goto Error;
	}
	
Error:	
	
	un3328_set_UserMode(INST_EVB[channel]);
	//Set TxEnable
	EVB5_SET_BEN_Level (INST_EVB[channel], 0);
	return error; 	
}

int UpdateBurstMode(const int channel)
{
	if (my_struConfig.PN_TYPE == PN_TYPE_ONU) 
	{
		if (EVB5_WRITE_TINY13_BYTE(INST_EVB[channel], 0x0F, 0)<0) 
		{
			MessagePopup("Error", "����Burstģʽʧ�ܣ�");
			return -1;
		} 
	}
	else if (my_struConfig.PN_TYPE == PN_TYPE_ONU_DDMI) 
	{
		if (MEGA88_SET_BURSTCTRL(INST_EVB[channel], 0)<0) 
		{
			MessagePopup("Error", "����Burstģʽʧ�ܣ�");
			return -1;
		} 
	}
	else 
	{
		MessagePopup ("��ʾ", "���ݿ�AutoDT_Spec_Tracking�ж����PN_TYPE�д���");
		return -1;
	} 
	
	return 0;
}

int UpdateWorkMode(const int channel)
{
	if (MEGA88_SET_fOVERD(INST_EVB[channel], 0)<0) 
	{
		MessagePopup("Error", "����VSC7967����ģʽʧ�ܣ�");
		return -1;
	} 
	
	return 0;
}

int UX3328_UpdateA2CheckSum(const int channel, struct struTestData *data, int panel, char Info[])
{
	int  error;
	char UX3328_Info[256];
	
	errChk(ux3328_set_FactoryMode(INST_EVB[channel]));
	
	error = ux3328_set_calibration_A2_T (INST_EVB[channel], 1.0, 0.0);
	if (error) return -1;
		 
	error = ux3328_set_calibration_A2_V (INST_EVB[channel], 1.0, 0.0);
	if (error) return -1;
	
	error = ux3328_set_calibration_A2_Tx_I (INST_EVB[channel], 1.0, 0.0);
	if (error) return -1;
	
	error = ux3328_set_calibration_A2_Tx_PWR (INST_EVB[channel], 1.0, 0.0);
	if (error) return -1;
	
	error = ux3328_set_calibration_A2_Rx_PWR (INST_EVB[channel], 0, 0, 0, 1.0, 0.0);
	if (error) return -1;
	
	Insert_Info(panel, gCtrl_BOX[channel], "����A2У��λ..."); 
	error = ux3328_set_checksum_A2_Table (INST_EVB[channel]);
	if (error) 	
	{
		strcpy (UX3328_Info, "����A2У��λ��ʧ��");
		Insert_Info(panel, gCtrl_BOX[channel], UX3328_Info);  
		data->ErrorCode=ERR_UX3328_SUM_UPDATE;
		strcpy(Info, UX3328_Info);											/***ʧ����Ϣ����Info**Eric.Yao***/  
		goto Error;
	}
	else		
	{
		strcpy (UX3328_Info, "����A2У��λ���ɹ�");							/***�ɹ���Ϣ������Info**Eric.Yao***/   
		Insert_Info(panel, gCtrl_BOX[channel], UX3328_Info);
	}       
	Delay(2); 		
	//�ص�  ����
	error = EVB5_SET_SHDN_VTR(INST_EVB[channel], 0);
	if (error) 
	{
		goto Error; 
	}
	
	Delay(0.5);
	
	error = EVB5_SET_SHDN_VTR(INST_EVB[channel], 1); 
    if (error)
	{
		goto Error;
	}

	Delay(1.5);  

	//����Ƿ���ȷ
	Insert_Info(panel, gCtrl_BOX[channel], "���A2У׼��׼λBFh...");    
	error = ux3328_check_A2flag(INST_EVB[channel], FALSE);
	if (error) 
	{
		strcpy (UX3328_Info, "���A2У׼��־λBFh��ʧ��");
		Insert_Info(panel, gCtrl_BOX[channel], UX3328_Info);
		data->ErrorCode=ERR_UX3328_SUM_CHECK;
		strcpy(Info, UX3328_Info);										/***ʧ����Ϣ����Info**Eric.Yao***/    	
		goto Error; 
	}
	else	 
	{
		strcpy (UX3328_Info, "���A2У׼��־λBFh���ɹ�"); 				/***�ɹ���Ϣ������Info**Eric.Yao***/     
		Insert_Info(panel, gCtrl_BOX[channel], UX3328_Info);
	}       
	
Error:	
	
	un3328_set_UserMode(INST_EVB[channel]);
	
	return error; 	
}

int UX3328S_UpdateA2CheckSum(const int channel, struct struTestData *data, int panel, char Info[])
{
	int  error;
	char UX3328S_Info[256];
	
	errChk(ux3328s_set_FactoryMode(INST_EVB[channel]));
	
	Insert_Info(panel, gCtrl_BOX[channel], "����A2У��λ...");
	error = ux3328s_set_checksum_A2_Table (INST_EVB[channel]);
	if (error) 	
	{
		strcpy (UX3328S_Info, "����A2У��λ��ʧ��");
		Insert_Info(panel, gCtrl_BOX[channel], UX3328S_Info);
		data->ErrorCode=ERR_UX3328_SUM_UPDATE;
		strcpy(Info, UX3328S_Info);											/***ʧ����Ϣ����Info**Eric.Yao***/  
		goto Error;
	}
	else		
	{
		strcpy (UX3328S_Info, "����A2У��λ���ɹ�");							/***�ɹ���Ϣ������Info**Eric.Yao***/   
		Insert_Info(panel, gCtrl_BOX[channel], UX3328S_Info);
	}       
			
	//�ص�  ����
	error = EVB5_SET_SHDN_VTR(INST_EVB[channel], 0);
	if (error) 
	{
		goto Error; 
	}
	
	Delay(0.3);
	
	error = EVB5_SET_SHDN_VTR(INST_EVB[channel], 1); 
    if (error)
	{
		goto Error;
	}

	Delay(1);  

	//����Ƿ���ȷ
	Insert_Info(panel, gCtrl_BOX[channel], "���A2У׼��׼λECh...");
	error = ux3328s_check_A2flag(INST_EVB[channel], FALSE);
	if (error) 
	{
		strcpy (UX3328S_Info, "���A2У׼��־λECh��ʧ��");
		Insert_Info(panel, gCtrl_BOX[channel], UX3328S_Info);     
		data->ErrorCode=ERR_UX3328_SUM_CHECK;
		strcpy(Info, UX3328S_Info);										/***ʧ����Ϣ����Info**Eric.Yao***/    	
		goto Error; 
	}
	else	 
	{
		strcpy (UX3328S_Info, "���A2У׼��־λECh���ɹ�"); 				/***�ɹ���Ϣ������Info**Eric.Yao***/     
		Insert_Info(panel, gCtrl_BOX[channel], UX3328S_Info);
	}       
	
Error:	
	
	ux3328s_set_UserMode(INST_EVB[channel]);
	
	return error; 	
}

int save_EEPROM (const int channel, struct struTestData *data)
{
	int error;
	char  filepath[1000] = "";
	char  filename[1000] = ""; 
	
	char  temp[1000] = "";
	char  buf[1000] = "";  
	
	int   bufferLen;
	char 	*dateTimeBuffer;
	
	double 	currDateTime;  
		
	unsigned char  rom_value_arr[256];
	
	memset (rom_value_arr, 0, sizeof(rom_value_arr));
	
	GetCurrentDateTime (&currDateTime);   
	bufferLen      = FormatDateTimeString (currDateTime, DATETIME_FORMATSTRING, NULL, 0);
	dateTimeBuffer = malloc (bufferLen + 1);
	FormatDateTimeString (currDateTime, DATETIME_FORMATSTRING, dateTimeBuffer, bufferLen + 1 );
	
	//SaveExcelFile ��ʽSuperxon_IXX0852206_SOEB4366-PSGA_838450_3400_20110627.xls
	FormatDateTimeString (currDateTime, DATETIME_FORMATSTRING, dateTimeBuffer, bufferLen + 1 ); 
	
	error = ux3328_ENTRY_PW2 (INST_EVB[channel]);
	if (error)
	{
		return -1;
	}
		
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA0, 0, 256, rom_value_arr); 
	if (error<0)
	{
		MessagePopup ("ERROR", "No Acknowledge from target !");
		return -1;
	} 

	strcpy (buf, data->OpticsSN);
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " "); 
	strcat (buf, " A0 ");   
	strcat (buf,".txt");
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}
	
	error = ux3328_select_table (INST_EVB[channel], 0);
	if (error) 
	{
		return -1;
	}
	
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, rom_value_arr); 
	if (error<0)
	{
		MessagePopup ("ERROR", "No Acknowledge from target !"); 
		return -1;
	} 
	
	strcpy (buf, data->OpticsSN);     
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " "); 
	strcat (buf, " A2 Table 0 ");   
	strcat (buf,".txt");	
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}	
	
	error = ux3328_select_table (INST_EVB[channel], 3);
	if (error)
	{
		return -1;
	}
	
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, rom_value_arr); 
	if (error<0) 
	{
		MessagePopup ("ERROR", "No Acknowledge from target !"); 
		return -1;
	} 
	
	strcpy (buf, data->OpticsSN);     
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " "); 
	strcat (buf, " A2 Table 3 ");   
	strcat (buf,".txt");	
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}	 
	
	error = ux3328_select_table (INST_EVB[channel], 4);
	if (error) 
	{
		return -1;
	}
	
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, rom_value_arr); 
	if (error<0) 
	{
		MessagePopup ("ERROR", "No Acknowledge from target !"); 
		return -1;
	} 
	
	strcpy (buf, data->OpticsSN);     
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " ");  
	strcat (buf, " A2 Table 4 ");   
	strcat (buf,".txt");  
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}

	error = ux3328_select_table (INST_EVB[channel], 5);
	if (error) 
	{
		return -1;
	}
	
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, rom_value_arr); 
	if (error<0) 
	{
		MessagePopup ("ERROR", "No Acknowledge from target !"); 
		return -1;
	} 
	
	strcpy (buf, data->OpticsSN);
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " "); 
	strcat (buf, " A2 Table 5 ");   
	strcat (buf,".txt");
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}

	error = ux3328_select_table (INST_EVB[channel], 6);
	if (error) 
	{
		return -1;
	}
	
	error = I2C_BYTEs_READ_DLL (INST_EVB[channel], 0xA2, 0, 256, rom_value_arr); 
	if (error<0) 
	{
		MessagePopup ("ERROR", "No Acknowledge from target !");
		return -1;
	} 
	
	strcpy (buf, data->OpticsSN);
	strcat (buf, " ");
	strcat (buf, dateTimeBuffer);   
	strcat (buf, " "); 
	strcat (buf, " A2 Table 6 ");   
	strcat (buf,".txt");
	GetDir (filepath);       
	strcat (filepath,"\\data\\");
	strcat (filepath, buf);
	
	error = save_EEPROM_file (filepath, rom_value_arr);
	if (error)
	{
		return -1;
	}   

	return 0;
}

int save_EEPROM_file (char *filename, unsigned char *rom_value_arr)
{
	char  temp[1000] = "";
	char  buf[10000] = ""; 
	int   filehandle = 0;   
	int   index;
	
	for (index=0; index<256; index++)
	{
		sprintf (temp, "0x%02X[%03d]=0x%02X;\n", index, index, rom_value_arr[index]);
		strcat(buf, temp);
	}   

	filehandle = OpenFile (filename, VAL_READ_WRITE, VAL_APPEND, VAL_ASCII); 			
	WriteFile (filehandle, buf, strlen(buf));
	CloseFile (filehandle);
			
	return 0;  
}

int testIbias(int channel, struct struTestData *data)
{
	char    Inf[1024];
	int 	error; 
	double	temp_Ibias; 
	
	errChk(Read_Ibias (channel, &temp_Ibias)); 
		
	data->TxV =temp_Ibias; 
		
	//���Ibias�����Ƿ񳬳��趨ֵ
	if (data->TxV>my_struDBConfig_ERCompens.IBias_Max)
	{
		goto Error;					
	}
	/*****************************************************************/
		
	return 0;
	
Error:

	return -1;
}

int GET_DAC_MOD (int channel, int *DAC)
{
	BYTE rom_value[64];
	INT8U tmpDAC;
	
	switch (my_struDBConfig_ERCompens.DriverChip)
	{			
		case DRIVERCHIP_VSC7965:
			if (EVB5_READ_TINY13_PAGE(INST_EVB[channel], rom_value)<0) 
			{
				return -1;  
			}
			*DAC = rom_value[0];
			break;
		case DRIVERCHIP_VSC7967:
			if (MEGA88_GET_DAC_MOD(INST_EVB[channel], &tmpDAC)<0)
			{
				return -1;  
			}
			*DAC = tmpDAC;
			break;
		case DRIVERCHIP_NT25L90:
			if (MEGA88NT25_GET_MOD_RAM(INST_EVB[channel], DAC)<0)
			{
				return -1;  
			}
			break;
		case DRIVERCHIP_UX3328:
			if (ux3328_get_mod_dac(INST_EVB[channel], &tmpDAC)<0) 
			{
				return -1;  
			}
			*DAC = tmpDAC;               
			break;
		case DRIVERCHIP_UX3328S:
			if (ux3328s_get_mod_dac(INST_EVB[channel], &tmpDAC)<0)
			{
				return -1;  
			}
			*DAC = tmpDAC;               
			break;
		default:
			MessagePopup("��ʾ","��ƷоƬ���ʹ���");
			return -1;
			break;
	}
	return 0; 
}

int ee_protect_check (int channel)
{
int error;
struct  sTRxConfig_DEF myTRxConfig;
	
	error = MEGA88_GET_TRxConfigFlag (INST_EVB[channel], &myTRxConfig);
	if (error<0)
	{
		MessagePopup ("��ʾ", "��ȡMEGA88�Ĵ��������쳣");
		return -1;
	}
	
	if (myTRxConfig.isA0Protected_flag !=1)
	{
		MessagePopup ("��ʾ", "��ȡMEGA88 A0д������־λ�쳣");
		return -1;
	} 	
	
	return 0;
}		   

int DB_GET_Wavelength(void)
{
	int num;
	char buf[256];  

	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	int tx_wavelength = 0;
	int rx_wavelength = 0;

	sprintf (buf, "SELECT Tx_WaveLength, Rx_WaveLength from AutoDT_Spec_WaveLength where partnumber = '%s'", my_struConfig.PN);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    resCode = DBBindColInt (hstmt, 1, &tx_wavelength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
    resCode = DBBindColInt (hstmt, 2, &rx_wavelength, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
    
	num=0;       
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	if (num!=1) {MessagePopup("Error","���ݿ�AutoDT_Spec_WaveLength���Ҳ����������ݻ����ж�����������  !"); return -1;} 
	
	my_struDBConfig.TxWavelength = tx_wavelength;
	my_struDBConfig.RxWavelength = rx_wavelength;  
	
	return 0;
}

BOOL testFixSens (int channel, int panel)
{
BYTE 	SD;
double 	temp_sens;
int	timer_star, timer, err_count, error, status, count;
char    ErrInfo[256];

	SetCtrlVal (panel, PAN_SMC_SLIDE_TIME, 0.); 

	temp_sens = my_struCal_OLT.OLT_sens[channel]+my_struCal_OLT.Power[channel]; 
	
	error = SET_ATT_ONU (channel, temp_sens);	  	//���������ȹ��� 
	if (error) return FALSE;
	
	Read_RX_SD_Ex (INST_BERT[channel], 1, &SD);   						//���SD�ź�   0��ģ��los, 1:û��los  
	if(SD == 0) //0��ģ��los										//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;
			
			error = SET_ATT_ONU (channel, temp_sens); 
			if (error) return FALSE;
			
			Read_RX_SD_Ex (INST_BERT[channel], 1, &SD);
		} while ((SD==0)&&(temp_sens<0));
	
		temp_sens = my_struCal_OLT.Power_In[channel]+my_struCal_OLT.Power[channel]; 
		
		error = SET_ATT_ONU (channel, temp_sens);	  	//���������ȹ��� 
		if (error) return FALSE;
		
		Read_RX_SD_Ex (INST_BERT[channel], 1, &SD); 
	
		if(SD == 0)	return FALSE; 							//���������LOS�źţ�����
	}

	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		//���ȳ�ʼ��SBERT����֤��������
		error = EVB5_SBERT_Init(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS);
		if (error){MessagePopup("��ʾ","SBERT����ʧ�ܣ�"); return FALSE;} 

		Delay (1); 
		
		if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, my_struCal_OLT.time[channel])<0) 
		{MessagePopup("Error", "SBERT start test error!"); return FALSE;}
	}	
	else {MessagePopup("Error", "can not find this BERT Type!"); return FALSE;} 
	
	timer_star=Timer();
	count=0;
	do
	{
		//��ȡ����
		Delay(1); 
			
		status = Read_Error_Bit_OLT(channel, &err_count, 0, my_struCal_OLT.time[channel], count, 1, ErrInfo);
		if (!status) {MessagePopup("Error", "Sens test fail!"); return FALSE;}  
	
		timer = Timer()- timer_star;
		   
		SetCtrlVal (panel, PAN_SMC_SLIDE_TIME, timer/my_struCal_OLT.time[channel]*100.);
		ProcessDrawEvents();
		
		count++;
	
	} while (timer<my_struCal_OLT.time[channel] && err_count==0);
	
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		if (EVB5_SBERT_End(INST_BERT_OLT[channel])<0) 
		{MessagePopup("Error", "SBERT Set Stop Error!!"); return FALSE;}
	}
	else {MessagePopup("Error", "can not find this BERT Type!"); return FALSE;} 

	//�жϲ��Խ��,����ֵ 
	if (timer<my_struCal_OLT.time[channel] || err_count>0) {return FALSE;}

	return TRUE;
}

int testDetailSens_OLT (int channel, int panel)
{
	double 	temp_sens, stepdown=0.5, stepup=0.2, senstime=12; 
	int    	error, err_count, timer_star, timer, count;
	BYTE	sd;
	char    buf[256];
	
	SetCtrlVal (panel, PAN_CALOLT_SLIDE_TIME, 0.);
	SetCtrlAttribute (panel, PAN_CALOLT_SLIDE_TIME, ATTR_LABEL_TEXT, "���Խ���(0%)");
	
	my_struCal_OLT.OLT_sens[channel]=0;

	//���ȳ�ʼ��SBERT����֤��������
	error = EVB5_SBERT_Init(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, INSTR[channel].SBert_PRBS);
	if (error)
	{
		MessagePopup("��ʾ","SBERT����ʧ�ܣ�");
		goto Error;
	} 
	
	//���ó�ʼ�⹦��
	temp_sens = my_struCal_OLT.OLT_sens_in[channel]+my_struCal_OLT.Power[channel];
	SET_ATT_OLT (channel, temp_sens);
	SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
	
	//���SD�źţ����û�м�⵽SD�����ӹ⹦��
	Read_RX_SD_Ex (INST_BERT_OLT[channel], 1, &sd);
	if(sd == 0) //0��ģ��los		//�����LOS������⹦�ʣ�ֱ��û��LOS�źţ�step��2dB   
	{
		do
		{
			temp_sens=temp_sens+2.0;     
			SET_ATT_OLT (channel, temp_sens); 
			SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
			Read_RX_SD_Ex (INST_BERT_OLT[channel], 1, &sd);
		} while ((sd==0)&&(temp_sens<0));
	
		temp_sens = my_struCal_OLT.OLT_sens_max[channel]+my_struCal_OLT.Power[channel];  
		SET_ATT_OLT (channel, temp_sens);//�������������ȹ��� 
		SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
		Read_RX_SD_Ex (INST_BERT_OLT[channel], 1, &sd);
		if(sd == 0)	
		{
			goto Error; 							//���������LOS�źţ�����
		}
	}

	//BERT��ʼ����
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, senstime)<0) 
		{
			MessagePopup("Error", "SBERT start test error!"); 
			goto Error;
		}
	}	
	else 
	{
		MessagePopup("Error", "can not find this BERT Type!"); 
		goto Error;
	} 
	
	//��ȡ����״̬������������룬ֱ���˳�
	Delay(0.3); 
	if (!Read_Error_Bit_OLT(channel, &err_count, 0, 1, 0, 0, buf)) 
	{
		MessagePopup("Error", "Sens test Start fail!"); 
		goto Error;
	}
	//��ʱ�Ĺ⹦��=my_struRxCal_OLT.RxCal_Power_Max�����ܳ�������
	if (err_count>0) 
	{
		goto Error;  
	}
	
	//���ù⹦�ʵ�OLT�պò��������� 
	count=0;
	do  //��������ʱ����˥��ֵ,һֱ����������˳�ѭ��
	{
		temp_sens = temp_sens - stepdown;  
		SET_ATT_OLT (channel, temp_sens);//���������ȹ���  
		SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
		Delay(1); 
	
		if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
		{
			if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, senstime)<0) 
			{
				MessagePopup("Error", "SBERT start test error!"); 
				goto Error;
			}
		}	
		else 
		{
			MessagePopup("Error", "can not find this BERT Type!");
			goto Error;
		} 

		Delay(1);
		Read_Error_Bit_OLT(channel, &err_count, 0, 1, count, 0, buf);    

		count++;
		
	} while((err_count ==0));     

	count=0; 
	do  //����˥����ֱ��û������Ϊֹ
	{
	 	temp_sens = temp_sens + stepup;  
		SET_ATT_OLT (channel, temp_sens);
		SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
		Delay(1); 
	
		if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
		{
			if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, senstime)<0) 
			{
				MessagePopup("Error", "SBERT start test error!"); 
				goto Error;
			}
		}	
		else 
		{
			MessagePopup("Error", "can not find this BERT Type!"); 
			goto Error;
		} 

		Delay(1);    
		Read_Error_Bit_OLT(channel, &err_count, 0,1, count, 0, buf);
	
		count++;
		
	} while (err_count != 0);

	//�����㹻ʱ��û������
	do
	{
		if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
		{
			if (EVB5_SBERT_Start(INST_BERT_OLT[channel], INSTR[channel].SBert_Rate, senstime)<0) 
			{
				MessagePopup("Error", "SBERT start test error!");
				goto Error;
			}
		}	
		else 
		{
			MessagePopup("Error", "can not find this BERT Type!");
			goto Error;
		} 
	
		timer_star=Timer(); 
		count=0;
		do
		{
			//��ȡ����
			Delay(1); 
			Read_Error_Bit_OLT(channel, &err_count, 1, 1, count, 0, buf);
			timer = Timer()- timer_star; 
			
			SetCtrlVal (panel, PAN_CALOLT_SLIDE_TIME, timer/senstime*100.);
			sprintf (buf, "���Խ���(%.2f%%)", timer/senstime*100.);
			SetCtrlAttribute (panel, PAN_CALOLT_SLIDE_TIME, ATTR_LABEL_TEXT, buf); 
			ProcessDrawEvents();
			Delay(0.2); 
			count++;
			
		} while (timer<senstime && err_count==0);
	
		if(err_count != 0)
	    {
		    temp_sens = temp_sens + stepup;  
			SET_ATT_OLT (channel, temp_sens); 
			SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, temp_sens-my_struCal_OLT.Power[channel]); 
			Delay(1); 
	   	}
	}while (err_count>0); 
	
	//�ر�BRET
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		if (EVB5_SBERT_End(INST_BERT_OLT[channel])<0)
		{
			MessagePopup("Error", "SBERT Set Stop Error!!"); 
			return -1;
		}
	}
	else 
	{
		MessagePopup("Error", "can not find this BERT Type!"); 
		return -1;
	} 
	
	my_struCal_OLT.OLT_sens[channel]=temp_sens-my_struCal_OLT.Power[channel];
	
	return 0;
	
Error:
	if (INSTR[channel].BERT_TYPE_OLT==BERT_TYPE_SBERT)
	{
		if (EVB5_SBERT_End(INST_BERT_OLT[channel])<0)
		{
			MessagePopup("Error", "SBERT Set Stop Error!!");
			return -1;
		}
	}
	else 
	{
		MessagePopup("Error", "can not find this BERT Type!"); 
		return -1;
	} 
	
	return -1;
}

int DB_GET_UX3328S_BENpch (const char pn[], const char bom[]) 
{
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	
	int  	num, error, index;
	char    chipadd[50];
	char    chipval[2048],buf2[2048], *tempbuf, buf[2048];
	int regadd, regval;
	union uUX3328S UX3328S_r;


		memset (UX3328S_r.pStr, 0,sizeof(UX3328S_r.pStr));
		
		sprintf (buf, "select chipadd,regval from AUTODT_SPEC_CHIPREG where pn='%s' and bom='%s' and chipname='UX3328S'", pn, bom);
	
		hstmt = DBActivateSQL (hdbc, buf);
   		if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
    
    	resCode = DBBindColChar (hstmt, 1, 50, chipadd, &DBConfigStat, "");
    	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
		resCode = DBBindColChar (hstmt, 2, 2048, chipval, &DBConfigStat, "");
    	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
    
    	num=0; 
    	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) 
    	{num++;}      
    
    	if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;}
    
    	resCode = DBDeactivateSQL (hstmt);
		if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
		Fmt (buf, "���ݿ�AUTODT_SPEC_CHIPREG���в����ҵ�PN=%s��Ψһ����", pn);
		if (num!=1) {MessagePopup("��ʾ", buf); return -1;} 
	
		StringUpperCase(chipadd);
		StringUpperCase(chipval);
		
		strcpy(buf2, chipval);
		tempbuf=strtok(buf2,";");
		if (tempbuf) 
		{ 
		//regxx=yy
			sscanf(tempbuf, "REG%x=%x", &regadd, &regval); 
			UX3328S_r.pStr[128] = regval;
						
		}
		index=1;
		while (tempbuf)
		{
			tempbuf=strtok(NULL,";"); 
			if (tempbuf)
			{
				sscanf(tempbuf, "REG%x=%x", &regadd, &regval);  

				UX3328S_r.pStr[128+index] = regval; 
				index++;
			}
		}	  
		
		if(UX3328S_r.sStr.uTable.UX3328S_TABLE3.UX3328S_TX_CTRL5.BEN_pch)
		{
			my_struEVB5.BURST_ON =EVB5_BURST_H;
			my_struEVB5.BURST_OFF=EVB5_BURST_L; 
		}	
		else
		{
			my_struEVB5.BURST_ON =EVB5_BURST_L;
			my_struEVB5.BURST_OFF=EVB5_BURST_H; 
		}	
			
	return 0;
}
int Fiber_SW_Init(int FSW_TYPE, int COMIndex, int *FSW_10G_Handle, char FSW_SN[30])
{
	int error;
	
	switch (FSW_TYPE)
	{
		case SW_TYPE_FSW: 
			error = FSW_Init (COMIndex);	
			break;
			
		case SW_TYPE_COFFSW:
			error = COFFSW_Init (COMIndex);	
			break; 
			
		case SW_TYPE_10G:
			error = EVB5_Init (FSW_10G_Handle, FSW_SN);
			break; 
			
		case SW_TYPE_JHFSW:
			error = JHFSW_Init (COMIndex);	
			break; 
			
		case SW_TYPE_COFFSW02:
			error = COFFSW_Init (COMIndex);	
			break;
			
		default: 
			return -1;
	}
	
	return error; 
}

int Fiber_SW_Close(int FSW_TYPE, int COMIndex, int FSW_10G_Handle)
{
	int error;
	
	switch (FSW_TYPE)
	{
		case SW_TYPE_FSW: 
			error = FSW_Close (COMIndex);	
			break;
			
		case SW_TYPE_COFFSW:
			error = COFFSW_Close (COMIndex);	
			break;
			
		case SW_TYPE_10G:
				;
			break;
			
		case SW_TYPE_JHFSW: 
			error = JHFSW_Close (COMIndex);	
			break;
			
		case SW_TYPE_COFFSW02:
			error = COFFSW_Close (COMIndex);	
			break;
			
		default: 
			return -1;
	}
	
	return error; 	
}


int Fiber_SW_SetChannel(int FSW_TYPE, int COMIndex, int FSW_10G_Handle, int channel)
{
	int error;
	
	switch (FSW_TYPE)
	{
		case SW_TYPE_FSW: 
			error = FSW_SetChannel (COMIndex, channel);	
			break;
			
		case SW_TYPE_COFFSW:
			error = COFFSW_SetChannel (COMIndex, channel);	
			break; 
		
		case SW_TYPE_10G:  
			error = set10GSwitch(FSW_10G_Handle, channel);
			break;
			
		case SW_TYPE_JHFSW:
			error = JHFSW_SetChannel (COMIndex, channel);	
			break; 
			
		case SW_TYPE_COFFSW02:
			error = COFFSW02_SetChannel (COMIndex, channel);	
			break;
			
		default: 
			return -1;
	}
	
	return error; 
}

int display_FSW(int panel, int fsw_type)
{
	switch(fsw_type)
	{
		case SW_TYPE_NONE:
			
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, FALSE); 
			break;
					
		case SW_TYPE_FSW:
			
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, TRUE); 
			break;
					
		case SW_TYPE_COFFSW:
					
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, TRUE);
			break;
					
		case SW_TYPE_10G:
				
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, TRUE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, TRUE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, FALSE);
			break;
			
		case SW_TYPE_JHFSW:
					
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, TRUE);
			break;
			
		case SW_TYPE_COFFSW02:
					
			SetCtrlAttribute (panel, PAN_INST_RING_FSW, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_BUT_FSW,  ATTR_VISIBLE, FALSE);
			SetCtrlAttribute (panel, PAN_INST_NUM_SW,   ATTR_VISIBLE, TRUE);
			break;

		default:
			MessagePopup ("��ʾ", "�޷�֧�ָù⿪��!");
			return -1;
			break;	   
	}  
	
	return 0;   
}

int set10GSwitch(int FSW_10G_Handle, int channel)
{
	int error;
	BYTE tmp_channel;
	int count;
	
	channel = channel -1;
	
	if (channel >= 2)	  /***10G�⿪��ֻ֧��0��1ͨ��**Eric.Yao**20131224***/
	{
		return -1;
	}
	
	count = 0;
	do
	{
		error = EVB5_SET_ONU_TxDIS (FSW_10G_Handle, channel);
		if(error<0) { MessagePopup ("Error", "�⿪������ʧ�ܣ�"); return error;} 
	
		Delay(1.5);       
		
		error = EVB5_GET_ONU_TxDIS (FSW_10G_Handle, &tmp_channel);
		if(error<0) { MessagePopup ("Error", "�⿪������ʧ�ܣ�"); return error;} 
	}while (count<10 && (channel != tmp_channel));
	
	
	if (count >= 5)
	{
		return -1;
	}
	
//	error = EVB5_SET_Switch_Local (EVB5_Handle, 1) ;
//	if(error<0) { MessagePopup ("Error", "�⿪����������ʧ�ܣ�"); return error;} 
	
	
	return error;    
}

int DWDM_ONU_Sel_Wavelength_Channel (int handle, INT16U WavelengthChannel)
{
	int error;
	int reg_add;
	union uA2Table2 local_A2Table2;
	
	local_A2Table2.sStrA2Table2.ChNumSet[0] = WavelengthChannel>>8;
	local_A2Table2.sStrA2Table2.ChNumSet[1] = WavelengthChannel&0xff; 
	reg_add = (int)(&local_A2Table2.sStrA2Table2.ChNumSet)-(int)(&local_A2Table2.sStrA2Table2.FirstByte);  
	error = I2C_BYTEs_WRITE_DLL (handle, 0xA2, reg_add, 2, local_A2Table2.pStrA2Table2,0.1);
	if (error<0)
	{
		return -1;	
	}
	
	return 0;	
}

int tuningAOP(int channel, struct struTestData *data)
{
	int count, DAC0, DAC1,DAC2,DAC3, DACmin, DACmax, count_error, error; 
	double	temp_AOP, temp_AOPmin, temp_AOPmax, temp_AOPavg; 
	BYTE LUT_APC[81], biasdac_m, biasdac_l;
	int index;																										   
	int PID_Arry[4];
	int temp_val;
	
	PID_Arry[0]=0x860;
	PID_Arry[1]=0xA30;										   
	PID_Arry[2]=0xC00;
	PID_Arry[3]=0xD50;
	
	//�ر� Ibias_Max����
//	error = DWDM_ONU_Ibias_Max_Disable(INST_EVB[channel]);
	error = DWDM_ONU_Ibias_Max_Enable(INST_EVB[channel]); 
	if (error<0) {MessagePopup ("ERROR", "�ر�Ibias_Max���ƹ���ʧ��"); return -1;}
	
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));
	
	//Set BIAS
	error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
	if(error)
	{
		MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}  
	error=DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]);
	if(error)
	{
		MessagePopup ("Error", "�л�TEC�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	
	/**********************************����Ibias������Χ**************************/
	if(SET_DAC_APC(channel, 0x35)<0)
	{
		return -1;
	}

	if(SET_DAC_MOD(channel, 0x35)<0)
	{
		return -1;	
	}
	error = ux3328s_select_table (INST_EVB[channel], 3);
	if (error) return -1;
	//ux3328s_set_TX_CTRL2
	if(ux3328s_set_TX_CTRL2(INST_EVB[channel], 0xCF))
	{
		MessagePopup("����", "дTable 3 0X82=0X8Fʧ��   "); 
		return -1;
	}
	//Read AOP <1.5dBm
	if (Read_AOP(channel, &temp_AOP)<0)
	{
		return -1;    
	}
	if(temp_AOP>=1.5)
	{
		//ux3328s_set_TX_CTRL2
		if(ux3328s_set_TX_CTRL2(INST_EVB[channel], 0x8F))
		{
			MessagePopup("����", "дTable 3 0X82=0X8Fʧ��   "); 
		return -1;
	}
	temp_val=0x8F;
	data->pathpenaltytime =(double)temp_val;
	}
	else
	{
		temp_val=0xCF;
		data->pathpenaltytime =(double)temp_val;
	}
	/**********************************����Ibias������Χ**************************/	
	//Reset APC and MOD
	errChk(SET_DAC_APC(channel, 0));    

	errChk(SET_DAC_MOD(channel, 0x10)); 
	
	errChk(DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100));  
	
	for(index=WAVELENGTHCHANNEL-1;index>=0;index--)
	{
		//Set PID ����ֵ
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
		{
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
			{
				return -1;
			}
		}

		   
		if((WAVELENGTHCHANNEL-1) ==index)
		{
			temp_AOPavg=(my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax)/2.0;
			temp_AOPmin=temp_AOPavg - 0.2;  
			temp_AOPmax=temp_AOPavg + 0.2;			
			
			sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 
//			errChk(SET_DAC_APC (channel, DAC0));
		}	 
		else
		{   
			temp_AOPmin=my_struDBConfig.TxPowMin;  
			temp_AOPmax=my_struDBConfig.TxPowMax;
			temp_AOPavg=(temp_AOPmin + temp_AOPmax)/2.0; 			
		}
		   
		count=0;   
		do
		{
			if((3!=index) && (0==count))  
			{
				DAC0 = data->ucRb[index+1]-2;	
			}
			errChk(SET_DAC_APC (channel, DAC0)); 
			Delay(3);					
			errChk (Read_AOP(channel, &temp_AOP)); 
		
			if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
			{
				break;
			}
			else 
			{	 
				if((WAVELENGTHCHANNEL-1) >index)
				{
					if(0==count)
					{
						sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 
					//	DACmax=data->ucRb[index+1] + 0x10;
					//	DACmin=data->ucRb[index+1] - 0x20;
					//	DAC0=(DACmax-DACmin)/2;
					}			
					temp_AOPavg=(my_struDBConfig.TxPowMin + my_struDBConfig.TxPowMax)/2.0;
					temp_AOPmin=temp_AOPavg - 0.2;  
					temp_AOPmax=temp_AOPavg + 0.2;  
				}
				if((DACmax-DACmin)<2) 
				{
					if((DACmax<=255) && (temp_AOP>temp_AOPavg))
					{
						DACmin= DACmax-0x10;
						DAC0=(DACmax+DACmin)/2;
			
					}
					else
					{
						MessagePopup("Error", "DACmax-DACmin<2��");
						data->ErrorCode=ERR_TUN_AOP;
						error = -1;
						goto Error;
					}
				} 
				else
				{
					if(temp_AOP>temp_AOPavg) 
					{
						DACmax=DAC0;
						DAC0=(DACmax+DACmin)/2;
					}
					else					 
					{
						DACmin=DAC0;
						DAC0=(DACmax+DACmin)/2;
					}
				}
			}
			count++;
		}while (count<20);
	
		if (count >= 20) 
		{
			MessagePopup ("Error", "�⹦�ʵ��Գ�ʱ!");
			error = -1; 
			goto Error;
		} 
		//��¼�����¶ȼ�DAC
		errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->ApcCoreTemper[index]));
		data->AOP[index] =temp_AOP; 								 
		data->ucRb[index]=DAC0; 
		
		errChk (DWDM_ONU_DS4830_Write_APC_LUK6(INST_EVB[channel], index, DAC0));   
	}
	
	errChk (DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel]));
	
	if (temp_AOP>my_struDBConfig.TxPowMax || temp_AOP<my_struDBConfig.TxPowMin) 
	{
		error = -1; 
		goto Error;
	} 
	
Error:	
	return error;
}

int tuningAOP_Ex(int channel, struct struTestData *data)
{
	int count, DAC0, DAC1,DAC2,DAC3, DACmin, DACmax, count_error, error; 
	double	temp_AOP, temp_AOPmin, temp_AOPmax, temp_AOPavg; 
	BYTE LUT_APC[81], biasdac_m, biasdac_l;
	int index;
	int PID_Arry[4];
	
	PID_Arry[0]=0x860;
	PID_Arry[1]=0xA30;
	PID_Arry[2]=0xC00;
	PID_Arry[3]=0xD50;
	
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));
	
	//Set BIAS
	error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
	if(error)
	{
	//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
	if(error)
	{
	//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	
	error=DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]);
	if(error)
	{
	//	MessagePopup ("Error", "�л�TEC�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	//reset APC and MOD
	errChk(SET_DAC_APC(channel, 0));    
	Delay(0.5);
	errChk(SET_DAC_MOD(channel, 0x10)); 
	Delay(1);
		
	temp_AOPavg=(my_struDBConfig.TxPowMax+my_struDBConfig.TxPowMin)/2.0;
	temp_AOPmin=temp_AOPavg-0.3;	/***�⹦�ʵ��Է�Χ����Ϊ����ֵ��0.15**Eric.Yao**20140616***/     
	temp_AOPmax=temp_AOPavg+0.3;
	
	for(index=0;index<WAVELENGTHCHANNEL;index++)
	{
		//Set PID ����ֵ
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
		{
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
			{
				return -1;
			}
		}

		   
		if(0==index)
		{
			sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 
		}
		else
		{
			DAC0= data->ucRb[index-1];
			DACmin=data->ucRb[index-1]-5; 
			DACmax=data->ucRb[index-1]+0x30;
		}
		
		count=0;   
		do
		{
			errChk(SET_DAC_APC (channel, DAC0));	
			Delay(1.5);					
			errChk (Read_AOP(channel, &temp_AOP)); 
		
			if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
			{
				break;
			}
			else if((DACmax-DACmin)<2) 
			{
			//	MessagePopup("Error", "����AOPʧ�ܣ�");
				data->ErrorCode=ERR_TUN_AOP;
				error = -1;
				goto Error;
			} 
			else
			{
				if(temp_AOP>temp_AOPavg) 
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else					 
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
			}
			count++;
		}while (count<10);
	
		if (count >= 10) 
		{			  
		//	MessagePopup ("Error", "�⹦�ʵ��Գ�ʱ!");
			error = -1; 
			goto Error;
		} 
		//��¼�����¶ȼ�DAC
		errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->ApcCoreTemper[index]));
		data->AOP[index] =temp_AOP; 								 
		data->ucRb[index]=DAC0; 
		
		errChk (DWDM_ONU_DS4830_Write_APC_LUK6(INST_EVB[channel], index, DAC0));   
	}
	
	errChk (DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel]));
	
	if (temp_AOP>my_struDBConfig.TxPowMax || temp_AOP<my_struDBConfig.TxPowMin) 
	{
		error = -1; 
		goto Error;
	} 
	
Error:	
	return error;
}

int tuningAOP_AginBack(int channel, struct struTestData *data)
{   
	int count, DAC0, DAC1,DAC2,DAC3, DACmin, DACmax, count_error, error; 
	double	temp_AOP, temp_AOPmin, temp_AOPmax, temp_AOPavg; 
	BYTE LUT_APC[81], biasdac_m, biasdac_l;
	int index;
	int PID_Arry[4];
	int ImaxDAC;
	
	PID_Arry[0]=0x860;
	PID_Arry[1]=0xA30;
	PID_Arry[2]=0xC00;
	PID_Arry[3]=0xD50;
	
	
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));
	
	//�ر� Ibias_Max����
	error = DWDM_ONU_Ibias_Max_Disable(INST_EVB[channel]);
	if (error<0) {MessagePopup ("ERROR", "�ر�Ibias_Max���ƹ���ʧ��"); return -1;}
	
	error = ux3328s_select_table (INST_EVB[channel], 3);
	if (error) return -1;
	//ux3328s_set_TX_CTRL2
	if(ux3328s_set_TX_CTRL2(INST_EVB[channel], 0xCF))
	{
		MessagePopup("����", "дTable 3 0X82=0X8Fʧ��   "); 
		return -1;
	}
	Delay(0.2);
	//Set BIAS
	error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
	if(error)
	{
	//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
	if(error)
	{
	//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	
	error=DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]);
	if(error)
	{
	//	MessagePopup ("Error", "�л�TEC�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	//reset APC and MOD
	errChk(SET_DAC_APC(channel, 0x35));    
	Delay(0.5);
	errChk(SET_DAC_MOD(channel, 0x0)); 
	Delay(1);
		
	temp_AOPavg=(my_struDBConfig.TxPowMax+my_struDBConfig.TxPowMin)/2.0;
	temp_AOPmin=temp_AOPavg-0.3;	/***�⹦�ʵ��Է�Χ����Ϊ����ֵ��0.15**Eric.Yao**20140616***/     
	temp_AOPmax=temp_AOPavg+0.3;
	
	for(index=3;index<WAVELENGTHCHANNEL;index++)
	{
		//Set PID ����ֵ
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
		{
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],PID_Arry[index])<0)
			{
				return -1;
			}
		}

		   
		sscanf (my_struDBConfig_ERCompens.Rb_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 

		count=0;   
		do
		{
			errChk(SET_DAC_APC (channel, DAC0));	
			Delay(1.5);					
			errChk (Read_AOP(channel, &temp_AOP)); 
		
			if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
			{
				break;
			}
			else if((DACmax-DACmin)<2) 
			{
				if(DAC0>250)
				{
					break;
				}
			//	MessagePopup("Error", "����AOPʧ�ܣ�");
				data->ErrorCode=ERR_TUN_AOP;
				error = -1;
				goto Error;
			} 
			else
			{
				if(temp_AOP>temp_AOPavg) 
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else					 
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
			}
			count++;
		}while (count<10);
	
		if (count >= 10) 
		{
		//	MessagePopup ("Error", "�⹦�ʵ��Գ�ʱ!");
			error = -1; 
			goto Error;
		} 
		
		//��¼�����¶ȼ�DAC
		errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->ApcCoreTemper[index]));
		data->AOP[index] =temp_AOP; 								 
		data->ucRb[index]=DAC0; 

	}
	
    //��ȡA2 D6ֵ ��ΪImax����DAC
	error = DWDM_ONU_Set_Ibias_Max_Ex(INST_EVB[channel],&ImaxDAC);
	if (error) 
	{
		error = -1; 
	}  
	data->ucRm[3]=ImaxDAC;  	//�ϻ����� ucRm[3] ��¼ImaxDAC

Error:	
	return error;
}

int tuningER(int channel, struct struTestData *data)
{
	int count, DAC0, DAC1, DACmin, DACmax, error; 
	double	temp_ER, temp_ERmin, temp_ERmax, temp_ERavg, temp_Ibias, temp_delta, tempval; 
	BYTE LUT_MOD[81], biasdac_m, biasdac_l;
	char buf[256];
	int  index,Table;
	double temp_AOP;
	BOOL Tun2=FALSE;

	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328_set_mode (INST_EVB[channel], 0));
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328s_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328s_set_mode (INST_EVB[channel], 0));
	}
	
	//Save LUK   
	errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));		//����TEC �ֶ�ģʽ
	for(index=WAVELENGTHCHANNEL-1;index>=0;index--)
	{   
		Tun2=FALSE; 
		error=0;    
		//Set Tec
		if(data->TecDAC[index]<0x600)  data->TecDAC[index]=0x600;
		errChk(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], data->TecDAC[index]));
		
		//Set APC
		errChk(SET_DAC_APC (channel, data->ucRb[index])); 
		
		//Set BIAS
		error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
		error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
	
		if(3==index)
		{
			sscanf (my_struDBConfig_ERCompens.Rm_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 
		//	DACmin=data->ucRm[3]-5;
		//	DACmax=data->ucRm[3]+10;
		//	DAC0= data->ucRm[3]; 
		}
		else
		{
			DACmin=data->ucRm[index+1]-10;
			DACmax=data->ucRm[index+1]+5;
			DAC0=data->ucRm[index+1]-3;
		}
	
		temp_ERavg=(my_struDBConfig.TxErMax+my_struDBConfig.TxErMin)/2.0;
		if (my_struConfig.DT_Test)			/***���Թ���ָ�갴�ղ�������**Eri.Yao**20140526***/
		{
//			temp_ERmin = my_struDBConfig.TxErMin;
//			temp_ERmax = my_struDBConfig.TxErMax;
		}
		else
		{
		//	temp_ERmin=temp_ERavg-(my_struDBConfig.TxErMax-my_struDBConfig.TxErMin)/2.0;
		//	temp_ERmax=temp_ERavg+(my_struDBConfig.TxErMax-my_struDBConfig.TxErMin)/2.0;
			
			temp_ERmin=temp_ERavg-0.5;
			temp_ERmax=temp_ERavg+0.0;
		}   
	
		count=0;
		do
		{
			errChk(SET_DAC_MOD (channel, DAC0))
			Delay(1);
			errChk (Read_ER(channel, &temp_ER));
			if(temp_ER >999.)
			{
				MessagePopup ("Error", "�����ȡʧ��!"); 
				return -1;	
			}
			
			if((3!=index)&&(0==count))
			{
				temp_ERmin = my_struDBConfig.TxErMin;
				temp_ERmax = my_struDBConfig.TxErMax;
			}
			else
			{
				temp_ERmin=temp_ERavg-0.5;
				temp_ERmax=temp_ERavg+0.5;
			}
			
			if (temp_ER>=temp_ERmin && temp_ER<=temp_ERmax) 
			{
				break;
			}
			else
			{
				if((DACmax-DACmin)<2)
				{
					Tun2=TRUE;
					break;
				}
				if(temp_ER < temp_ERavg)
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else			
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2; 
				}
			}

			count++;
		}while (count<10);
		  
		if (count >= 10) 
		{
			MessagePopup ("Error", "����ȵ��Գ�ʱ!"); 
			error = -1;
			goto Error;
		} 
		
		//����΢��  
		if(Tun2)
		{	 
			count=0;
			do
			{
				if (temp_ER>=temp_ERmin && temp_ER<=temp_ERmax) 
				{
					break;
				}
				else 
				{
					if(temp_ER < temp_ERavg)
					{
						DAC0+=2;
					}
					else			
					{
						
						DAC0-=2;
					}  
					if(DAC0>255)   DAC0=255;
					if(DAC0<0)	   DAC0=0;
					errChk(SET_DAC_MOD (channel, DAC0))  
				//	Delay(1);	
					errChk (Read_ER(channel, &temp_ER));
				}	   
			}while(count<10);
			if(count>=10)
			{
				MessagePopup ("Error", "������ȵ���ʧ��!"); 
				error = -1;
				goto Error;	
			}
		}
	
		//��¼Mod�¶ȼ�DAC  	
		data->ER[index]=temp_ER; 
		data->ucRm[index]=DAC0;	   
		errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->ModCoreTemper[index]));
	
		if (temp_ER>my_struDBConfig.TxErMax || temp_ER<my_struDBConfig.TxErMin) 
		{
			error = -1;
			goto Error;
		}
	
//		if (DAC0>DACmax)
//		{
//			MessagePopup ("Error", "MOD DAC������������"); 
//			error = -1;
//			goto Error;
//		}  
	   	//Check AOP
//		errChk (Read_AOP(channel, &temp_AOP)); 	
//		if (temp_AOP>my_struDBConfig.TxCalMax || temp_AOP<my_struDBConfig.TxCalMin) 
//		{
		//	MessagePopup ("Error", "�⹦�ʼ��ʧ��!");   
//			error = -1; 
//			goto Error;
//		} 
		errChk (DWDM_ONU_DS4830_Write_MOD_LUK6(INST_EVB[channel], index, DAC0));	 
	}
	
	errChk (DWDM_ONU_DS4830_Update_LUK6(INST_EVB[channel])); 
	
	errChk(Read_Ibias (channel, &temp_Ibias)); 
	
	data->TxV =temp_Ibias; 
	
	if(temp_Ibias<5)
	{
	//	MessagePopup ("Error", "���I_Bias<5mA!");
		error = -1; 
		goto Error;					
	}
	
	//���Ibias�����Ƿ񳬳��趨ֵ
//	if (data->TxV>my_struDBConfig_ERCompens.IBias_Max)
//	{
//		sprintf (buf, "Ibias���������������ֵ%.2f    ", my_struDBConfig_ERCompens.IBias_Max);
//		MessagePopup ("��ʾ", buf);
//		data->ErrorCode=ERR_TUN_BIAS_MAX;  
//		error = -1; 
//		goto Error;					
//	}
/*	
	//Set Bias Auto Mode
	error=DWDM_ONU_DS4830_Set_BIAS_Auto(INST_EVB[channel]);
	if(error)
	{
	//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	//����TEC Auto��ģʽ 
	errChk(DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]));
*/	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
Error:
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningER_Beforehand(int channel, struct struTestData *data)					//Ԥ�ȵ��Ե�4ͨ��ER
{
	int count, DAC0, DAC1, DACmin, DACmax, error; 
	double	temp_ER, temp_ERmin, temp_ERmax, temp_ERavg, temp_Ibias, temp_delta, tempval; 
	BYTE LUT_MOD[81], biasdac_m, biasdac_l;
	char buf[256];
	int  index,Table;
	double temp_AOP;
	BOOL Tun2=FALSE;
	int Delaytime;

	if (CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328_set_mode (INST_EVB[channel], 0));
	}
	else if (CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip) 	  //�ֶ�ģʽ
	{
		errChk(ux3328s_set_FactoryMode(INST_EVB[channel])); 
		
		errChk(ux3328s_set_mode (INST_EVB[channel], 0));
	}
	
	//Save LUK   
	errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));		//����TEC �ֶ�ģʽ
	for(index=3;index<WAVELENGTHCHANNEL;index++)
	{   
		Tun2=FALSE; 
		error=0;    
		//Set Tec
//		if(data->TecDAC[index]<0x600)  data->TecDAC[index]=0x600;
		errChk(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], 0xD00));
		
		//Set APC
		errChk(SET_DAC_APC (channel, data->ucRb[index])); 
		
		//Set BIAS
		error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
		error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
	
		sscanf (my_struDBConfig_ERCompens.Rm_Add, "(0x%02x)(0x%02x)(0x%02x)", &DAC0, &DACmin, &DACmax); 
		 
		temp_ERavg=(my_struDBConfig.TxErMax+my_struDBConfig.TxErMin)/2.0;
		if (my_struConfig.DT_Test)			/***���Թ���ָ�갴�ղ�������**Eri.Yao**20140526***/
		{
			temp_ERmin = my_struDBConfig.TxErMin;
			temp_ERmax = my_struDBConfig.TxErMax;
		}
		else
		{
		//	temp_ERmin=temp_ERavg-(my_struDBConfig.TxErMax-my_struDBConfig.TxErMin)/2.0;
		//	temp_ERmax=temp_ERavg+(my_struDBConfig.TxErMax-my_struDBConfig.TxErMin)/2.0;
			
			temp_ERmin=temp_ERavg-0.5;
			temp_ERmax=temp_ERavg+0.5;
		}   
	
		count=0;
		do
		{
			errChk(SET_DAC_MOD (channel, DAC0))
	//		Delay(1);
			errChk (Read_ER(channel, &temp_ER));
		
			if (temp_ER>=temp_ERmin && temp_ER<=temp_ERmax) 
			{
				break;
			}
			else
			{
				if((DACmax-DACmin)<2)
				{
					Tun2=TRUE;
					break;
				}
				if(temp_ER < temp_ERavg)
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else			
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2; 
				}
			}

			count++;
		}while (count<20);
		  
		if (count >= 20) 
		{
		//	MessagePopup ("Error", "����ȵ��Գ�ʱ!"); 
		//	error = -1;
		//	goto Error;
			data->ucRm[index]=0x20;	 		   //���Գ�ʱ��Ĭ��ʹ��0x20Ϊ����MOD DAC   
		} 
		
		//����΢��  
		if(Tun2)
		{	 
			count=0;
			do
			{
				if (temp_ER>=temp_ERmin && temp_ER<=temp_ERmax) 
				{
					break;
				}
				else if(temp_ER < temp_ERavg)
				{
					errChk(SET_DAC_MOD (channel, DAC0))
				//	Delay(1);
					errChk (Read_ER(channel, &temp_ER));
					DAC0+=2;
					if(DAC0>256) DAC0=255;
				}
				else			
				{
					errChk(SET_DAC_MOD (channel, DAC0))
					errChk (Read_ER(channel, &temp_ER));
					DAC0-=2;
					if(DAC0<0) DAC0=0; 
				}
				count++;
			}while(count<30);
			if(count>=30)
			{
			//	MessagePopup ("Error", "����ȵ���ʧ��!"); 
			//	error = -1;
			//	goto Error;
				data->ucRm[index]=0x20;	 			  //���Գ�ʱ��Ĭ��ʹ��0x20Ϊ����MOD DAC
			}
		}
	
		//��¼Mod�¶ȼ�DAC  	
		data->ER[index]=temp_ER; 
		data->ucRm[index]=DAC0;	
		data->Sigma=(double)DAC0;
	}

Error:
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningWavelength(int channel, struct struTestData *data)
{
//	int count,count1, DAC,DAC0, DAC1,DACmin,DACmax, count_error, error; 
	


	double span;
	double deltaWL;
//	double	temp_Wavelegnth, temp_Wavelegnthmin, 
	 
//	double	temp_Wavelegnth_01,Wavelegnth_delta;
	int index;
	int count;
	int	error;
	char strTmp[1024]; 
	
	int	    ROOM_TEC[WAVELENGTHCHANNEL];
	double	ROOM_Temper[WAVELENGTHCHANNEL]; 
//	double temp_AOP;
	
	//���ұ����
	unsigned short LukDACArr[3]; 
	double LukTemperArr[3];
	
	int MaxCount = 15; 

	
//	int LookFlag=0;
	
//	double	Wavelegnth_Arry[WAVELENGTHCHANNEL];
	
	BOOL 	tun2;
	int 	DACminArry[WAVELENGTHCHANNEL];
	int 	DACmaxArry[WAVELENGTHCHANNEL];	
	double	WavelegnthMin[WAVELENGTHCHANNEL];
	double	WavelegnthMax[WAVELENGTHCHANNEL];
	double	temp_Wavelegnth;
	double	Wavelegnth0; 
	int		DAC,DAC0,DACmin,DACmax;
	double  Slope,Offset; 
	
	//New������ر���
	double	DAC_Array[WAVELENGTHCHANNEL];
	double	WL_Array[WAVELENGTHCHANNEL];
	double	newWL_Array[WAVELENGTHCHANNEL];
	double	Ratio_Array[WAVELENGTHCHANNEL];
	double	MeanSquaredError;
	double	TargetDAC_Array[WAVELENGTHCHANNEL];
	double	TargetWL[WAVELENGTHCHANNEL];
	BOOL	okFlag;
	int		UpFlag[WAVELENGTHCHANNEL]={1,1,1,1};
	int		order=3;
	
	 

	memset (strTmp, 0, sizeof(strTmp)); 
//	memset (DAC_Arr, 0, sizeof(DAC_Arr));
//	memset (Temper_Arr, 0, sizeof(Temper_Arr)); 
//	Wavelegnth_Arry[0]=0;
//	Wavelegnth_Arry[1]=0;
	
	if(!my_struConfig.Temper_Room)
	{
	 	DAC_Array[0]=0x800;
		DAC_Array[1]=0xA00;
		DAC_Array[2]=0xC00;
		DAC_Array[3]=0xD50;
	}						  
	else
	{
		//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
		errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
		DAC_Array[0]=ROOM_TEC[0]; 
		DAC_Array[1]=ROOM_TEC[1]; 
		DAC_Array[2]=ROOM_TEC[2]; 
		DAC_Array[3]=ROOM_TEC[3]; 
	}
	DACminArry[0]=0x700;
	DACminArry[1]=0x800;
	DACminArry[2]=0x900;
	DACminArry[3]=0xA00; 
	
	DACmaxArry[0]=0x900;
	DACmaxArry[1]=0xA00;
	DACmaxArry[2]=0xB00;
	DACmaxArry[3]=0xC00;

	strcat (data->APD_Info, "\nTun WL");  
	//����TEC�ֶ�ģʽ
	strcat (data->APD_Info, "\nSet TEC�ֶ�");   	
	if(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel])<0)
	{
		strcat (data->APD_Info, "ʧ��"); 
		return -1;   
	}
	
	//���ùض����ֵΪ�ֶ�ģʽ��DAC=0x100;
	strcat (data->APD_Info, "\nSet BIAS�ֶ�");   
	if(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel])<0)
	{
		strcat (data->APD_Info, "ʧ��");    
		return -1; 
	}
	
	strcat (data->APD_Info, "\nSet BIAS DAC=100");  
	if(DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100)<0)
	{
		strcat (data->APD_Info, "ʧ��");
	  	return -1;
	}

	//����ռ�ձ�100% 
	strcat (data->APD_Info, "\nSet 100%");  
	if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//New ����---4ͨ��ͬʱ����
	strcat (data->APD_Info, "\n4ͨ������"); 
	
	for (index=0;index<WAVELENGTHCHANNEL;index++)
	{
		TargetWL[index]=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset; 
		sprintf(strTmp,"\nTargetWL%d=%f��",index,TargetWL[index]);
		strcat (data->APD_Info, strTmp);
	}
	
	count=0;
	data->PointCount=0;
	okFlag=FALSE;
	do
	{
		for (index=0;index<WAVELENGTHCHANNEL;index++)
		{
			if(1==UpFlag[index])
			{
				data->PointCount++; 
				//Set APC &Mod
				if (!my_struConfig.DT_Tuning)
				{
					strcat (data->APD_Info, "\nRead LUK6 APC"); 
					if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					strcat (data->APD_Info, "\nRead LUK6 MOD");  
					if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
					strcat (data->APD_Info, strTmp);    
					if(SET_DAC_APC(channel, data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
			
					sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
					strcat (data->APD_Info, strTmp);  
					if(SET_DAC_MOD(channel, data->ucRm[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
				}
				else
				{
					sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
					strcat (data->APD_Info, strTmp);
					if(SET_DAC_APC(channel, data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[3]);   
					strcat (data->APD_Info, strTmp);
					if(SET_DAC_MOD(channel, data->ucRm[3])<0)  
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
				}
					  
				sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0)
				{
					strcat (data->APD_Info, "ʧ��\n��д����"); 
					error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
					if (error) 
					{
						strcat (data->APD_Info, "ʧ��");  
						return -1;
					}
					
					sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0) 
					{
						strcat (data->APD_Info, "ʧ��");  
						return -1;
					}
				}   
				
				//�ȴ�WL�ȶ�  
				strcat (data->APD_Info, "\nWait PID LOOK"); 
				if(Get_TecPID_LookState(channel)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;	
				}
	
				//��ȡWL
				strcat (data->APD_Info, "\nRead WL");
				if (Read_Wavelength(channel, &WL_Array [index])<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
				sprintf(strTmp,"WL_Array[%d]=%f",index,WL_Array [index]);   
				strcat (data->APD_Info, strTmp);
			}
			else
			{
				;
			}
		}
	
		if((fabs(WL_Array[0]-TargetWL[0])<TunWLAccuracy) && (fabs(WL_Array[1]-TargetWL[1])<TunWLAccuracy) && (fabs(WL_Array[2]-TargetWL[2])<TunWLAccuracy) && (fabs(WL_Array[3]-TargetWL[3])<TunWLAccuracy))
		{
			okFlag=TRUE;
			data->TecDAC[0]=DAC_Array[0]; 
			data->TecDAC[1]=DAC_Array[1]; 
			data->TecDAC[2]=DAC_Array[2]; 
			data->TecDAC[3]=DAC_Array[3]; 
			data->Peakwavelength[0] =WL_Array[0];
			data->Peakwavelength[1] =WL_Array[1]; 
			data->Peakwavelength[2] =WL_Array[2]; 
			data->Peakwavelength[3] =WL_Array[3]; 
		}
		else
		{
	  		PolyFit(DAC_Array,WL_Array,WAVELENGTHCHANNEL,3,newWL_Array,Ratio_Array,&MeanSquaredError);  
		
		 	error = Solving_Root(Ratio_Array,3,TargetWL,TargetDAC_Array);
			if(error<0)
			{
				return -1; 	
			}
			
			for (index=0;index<WAVELENGTHCHANNEL;index++)  
			{
				if(fabs(WL_Array[index]-TargetWL[index])>TunWLAccuracy)
				{
					UpFlag[index]=1;  
				
					if((0==index) && (WL_Array[index]-TargetWL[index])>0)
					{
						DAC_Array[index] = TargetDAC_Array[index]-0x05;
					}   
					else if((4==index) && (WL_Array[index]-TargetWL[index])<0)
					{
						DAC_Array[index] = TargetDAC_Array[index]+0x05;
					} 
					else
					{
						deltaWL= WL_Array[index]-TargetWL[index];
						if(fabs(deltaWL)< 0.015)
						{
							DAC_Array[index]= DAC_Array[index] - deltaWL/0.002;   
						}
						else
						{
							DAC_Array[index] = TargetDAC_Array[index];  
						}
					}
				}
				else
				{
					UpFlag[index]=0; 	
				}
			}  
		}   
		count++;
	}while(count<10 && okFlag==FALSE);
	
	if(count>=10)
	{	 
		strcat (data->APD_Info, "\n4ͨ������ʧ��\n��ͨ������");   
		
		for (index = (WAVELENGTHCHANNEL-1); index>=0; index--)					//����ͨ�����Ե������ȵ���4ͨ��
		{
			tun2=FALSE;
			//������ 
			sprintf(strTmp,"\nTun Ch%d",index+1);
			strcat (data->APD_Info, strTmp); 
		
			WavelegnthMin[index]=TargetWL[index] - TunWLAccuracy;				
			WavelegnthMax[index]=TargetWL[index] + TunWLAccuracy;		
			
			sprintf(strTmp,"\nWL_Min=%f;WL_Max=%f",WavelegnthMin[index],WavelegnthMax[index]);
			strcat (data->APD_Info, strTmp); 
		
			//Set APC &Mod
			if (!my_struConfig.DT_Tuning)
			{
				strcat (data->APD_Info, "\nRead LUK6 APC"); 
				if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				strcat (data->APD_Info, "\nRead LUK6 MOD");  
				if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);    
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
				strcat (data->APD_Info, strTmp);  
				if(SET_DAC_MOD(channel, data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
			else
			{
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[3]);   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_MOD(channel, data->ucRm[3])<0)  
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
		
			//����Tec DAC��С
			DAC0= DACminArry[index]; 
			sprintf(strTmp,"\nSet TEC DAC0=%d",DAC0);   
			strcat (data->APD_Info, strTmp);
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC0)<0)
			{
				strcat (data->APD_Info, "ʧ��\n��д����");  
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					strcat (data->APD_Info, "ʧ��");   
					return -1;
				}
				
				sprintf(strTmp,"\nReSet TEC DAC0=%d",DAC0);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC0)<0) 
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
			}
		
			//�ȴ�WL�ȶ�  
			strcat (data->APD_Info, "\nWait PID LOOK");  
			if(Get_TecPID_LookState(channel)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;	
			}
		
			//��ȡWL
			strcat (data->APD_Info, "\nRead WL");   
			if (Read_Wavelength(channel, &Wavelegnth0)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;
			}
			sprintf(strTmp,"=%f",Wavelegnth0);   
			strcat (data->APD_Info, strTmp);

			//����Tec DAC���
			DAC=DACmaxArry[index];
			count=0;
			do
			{
				sprintf(strTmp,"\nSet TEC DAC=%d",DAC);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0)
				{
					strcat (data->APD_Info, "ʧ��\n��д����");  
					error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
					if (error) 
					{
						strcat (data->APD_Info, "ʧ��");   
						return -1;
					}
				
					sprintf(strTmp,"\n\nReSet TEC DAC=%d",DAC);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0) 
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;
					}
				}
		
				//�ȴ�WL�ȶ�  
				strcat (data->APD_Info, "\nWait PID LOOK");  
				if(Get_TecPID_LookState(channel)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;	
				}
		
				//��ȡWL
				strcat (data->APD_Info, "\nRead WL");   
				if (Read_Wavelength(channel, &temp_Wavelegnth)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
				sprintf(strTmp,"=%f",temp_Wavelegnth);   
				strcat (data->APD_Info, strTmp);
			
				//�ж�
				if (temp_Wavelegnth>=WavelegnthMin[index] && temp_Wavelegnth<=WavelegnthMax[index]) 
				{  	
					strcat (data->APD_Info, "\nWL TunOK"); 
					break;
				}
				else
				{
					//����Slope Offset,�Ƶ�Ŀ��DAC
					Slope=(temp_Wavelegnth-Wavelegnth0) / (DAC-DAC0);
					Offset=temp_Wavelegnth-(Slope* DAC);
					
					sprintf(strTmp,"\nSlope=%f,Offset=%f",Slope,Offset);   
					strcat (data->APD_Info, strTmp);
					
					if(fabs(temp_Wavelegnth-TargetWL[index]) < fabs(Wavelegnth0-TargetWL[index]))
					{
						Wavelegnth0= temp_Wavelegnth;
						DAC0=DAC;
					}  	
					
					DAC=(TargetWL[index]-Offset)/Slope; 
					if(DAC0>0xF10) { DAC0=0xF10;}
					if(DAC0<0x500) { DAC0=0x500;}
				}

			}while(count<6);
			
			if(count>=6) 
			{
				tun2=TRUE;
			}
		
			if(tun2)
			{
				//���ַ�������
				DACmin = DAC-0x50;
				DACmax = DAC+0x50;	
				
				sprintf(strTmp,"\n���ַ�,DAC��Χ:%d~%d,Offset=%f",DACmin,DACmax);   
				strcat (data->APD_Info, strTmp);
			
				count=0;  
				span=2;
				DAC=(DACmin+DACmax)/2;
				do
				{
					sprintf(strTmp,"\nSet TEC DAC=%d",DAC);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0)
					{
						strcat (data->APD_Info, "ʧ��\n��д����");  
						error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
						if (error) 
						{
							strcat (data->APD_Info, "ʧ��");   
							return -1;
						}
				
						sprintf(strTmp,"\n\nReSet TEC DAC=%d",DAC);   
						strcat (data->APD_Info, strTmp);
						if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0) 
						{
							strcat (data->APD_Info, "ʧ��");     
							return -1;
						}
					}
		
					//�ȴ�WL�ȶ�  
					strcat (data->APD_Info, "\nWait PID LOOK");  
					if(Get_TecPID_LookState(channel)<0)
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;	
					}
		
					//��ȡWL
					strcat (data->APD_Info, "\nRead WL");   
					if (Read_Wavelength(channel, &temp_Wavelegnth)<0)
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;
					}
					sprintf(strTmp,"=%f",temp_Wavelegnth);   
					strcat (data->APD_Info, strTmp);
					
					if (temp_Wavelegnth>=WavelegnthMin[index] && temp_Wavelegnth<=WavelegnthMax[index])    
					{  		
						break;
					}
					else if((DACmax-DACmin)<2) 
					{
						sprintf(strTmp,"\nDACmax-DACmax<2,ʧ��",DACmax,DACmin);  
						strcat (data->APD_Info, strTmp); 
						error = -1;
						goto Error;
					} 
					else
					{
						if(temp_Wavelegnth>TargetWL[index]) 
						{
							DACmax=DAC0;
							DAC0=(DACmax+DACmin)/2;
							sprintf(strTmp,"\nDACmin=%d,DACmax=%d",DACmin,DACmax);  
							strcat (data->APD_Info, strTmp); 
						}
						else					 
						{
							DACmin=DAC0;
							DAC0=(DACmax+DACmin)/2;
							sprintf(strTmp,"\nDACmin=%d,DACmax=%d",DACmin,DACmax);  
							strcat (data->APD_Info, strTmp); 
						}
					}
					count++;
					if(fabs(temp_Wavelegnth-TargetWL[index])<1.0)
					{
						span=1;
					}
					else
					{
						;
					}
				}while (count<MaxCount);
	
				if (count >= MaxCount) 
				{
					sprintf(strTmp,"\nTun Conut>=%d,ʧ��",count);  
					strcat (data->APD_Info, strTmp); 
					error = -1; 
					goto Error;
				} 
			}
	
			//��¼�����¶ȼ�DAC
			errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));
			data->Peakwavelength[index] =temp_Wavelegnth; 
			data->TecDAC[index]=DAC;
			
			//�����¶�������²��ұ�
			if (my_struConfig.Temper_Room)
			{
				//LUKдֱ��
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=data->TecDAC[index];
				LukDACArr[2]=data->TecDAC[index]; 
				
				LukTemperArr[0] =-40.;  
				LukTemperArr[1] = data->TecCoreTemper[index];   
				LukTemperArr[2] =100;
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, LukDACArr, LukTemperArr)); 					//����дK=0;
			}
			else if(my_struConfig.Temper_High)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
			
				LukDACArr[0]=ROOM_TEC[index];
				LukDACArr[1]=data->TecDAC[index];
				
				LukTemperArr[0] = ROOM_Temper[index]; 
				LukTemperArr[1] = data->TecCoreTemper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, LukDACArr, LukTemperArr));  
			}
			else if(my_struConfig.Temper_Low)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
			
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=ROOM_TEC[index];  
			
				LukTemperArr[0] = data->TecCoreTemper[index];
				LukTemperArr[1] = ROOM_Temper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, LukDACArr, LukTemperArr));  
			}
			else
			{
				error = -1;
				goto Error;
			}   
		}
	}
	else
	{
		sprintf(strTmp,"\nPID Set����=%d",data->PointCount);   
		strcat (data->APD_Info, strTmp);
		for (index = (WAVELENGTHCHANNEL-1); index>=0; index--)
		{
			//��¼�����¶�
			errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));
			
			//�����¶�������²��ұ�
			if (my_struConfig.Temper_Room)
			{
				//LUKдֱ��
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=data->TecDAC[index];
				LukDACArr[2]=data->TecDAC[index]; 
				
				LukTemperArr[0] =-40.;  
				LukTemperArr[1] = data->TecCoreTemper[index];   
				LukTemperArr[2] =100;
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, LukDACArr, LukTemperArr)); 					//����дK=0;
			}
			else if(my_struConfig.Temper_High)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
			
				LukDACArr[0]=ROOM_TEC[index];
				LukDACArr[1]=data->TecDAC[index];
				
				LukTemperArr[0] = ROOM_Temper[index]; 
				LukTemperArr[1] = data->TecCoreTemper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, LukDACArr, LukTemperArr));  
			}
			else if(my_struConfig.Temper_Low)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
			
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=ROOM_TEC[index];  
			
				LukTemperArr[0] = data->TecCoreTemper[index];
				LukTemperArr[1] = ROOM_Temper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, LukDACArr, LukTemperArr));  
			}
			else
			{
				error = -1;
				goto Error;
			}
		}
	}

	//Save LUK  
	if(!my_struConfig.Temper_Room)
	{
		errChk(DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]));		//����TEC Auto��ģʽ
	}
	
	if(DWDM_ONU_DS4830_Update_LUK2(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK3(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK4(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK5(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
		
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningWavelength01(int channel, struct struTestData *data)
{
	double span;
	double deltaWL;

	int index;
	int count;
	int	error;
	char strTmp[1024]; 
	
	int	    ROOM_TEC[WAVELENGTHCHANNEL];
	double	ROOM_Temper[WAVELENGTHCHANNEL]; 

	//���ұ����
	unsigned short LukDACArr[3]; 
	double LukTemperArr[3];
	
	int MaxCount = 15; 

	BOOL 	tun2;
	int 	DACminArry[WAVELENGTHCHANNEL];
	int 	DACmaxArry[WAVELENGTHCHANNEL];	
	double	WavelegnthMin[WAVELENGTHCHANNEL];
	double	WavelegnthMax[WAVELENGTHCHANNEL];
	double	temp_Wavelegnth;
	double	Wavelegnth0; 
	int		DAC,DAC0,DACmin,DACmax;
	double  Slope,Offset; 
	
	//New������ر���
	double	DAC_Array[WAVELENGTHCHANNEL];
	double	WL_Array[WAVELENGTHCHANNEL];
	double	newWL_Array[WAVELENGTHCHANNEL];
	double	Ratio_Array[WAVELENGTHCHANNEL];
	double	MeanSquaredError;
	double	TargetDAC_Array[WAVELENGTHCHANNEL];
	double	TargetWL[WAVELENGTHCHANNEL];
	BOOL	okFlag;
	int		UpFlag[WAVELENGTHCHANNEL]={1,1,1,1};
	int		order=3;
	
	 
	data->PointCount=0;
	memset (strTmp, 0, sizeof(strTmp)); 

 	DAC_Array[0]=0x800;
	DAC_Array[1]=0xA00;
	DAC_Array[2]=0xC00;
	DAC_Array[3]=0xD50;
	
	DACminArry[0]=0x700;
	DACminArry[1]=0x800;
	DACminArry[2]=0x900;
	DACminArry[3]=0xA00; 
	
	DACmaxArry[0]=0x900;
	DACmaxArry[1]=0xA00;
	DACmaxArry[2]=0xB00;
	DACmaxArry[3]=0xC00;

	strcat (data->APD_Info, "\nTun WL");  
	//����TEC�ֶ�ģʽ
	strcat (data->APD_Info, "\nSet TEC�ֶ�");   	
	if(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel])<0)
	{
		strcat (data->APD_Info, "ʧ��"); 
		return -1;   
	}
	
	//���ùض����ֵΪ�ֶ�ģʽ��DAC=0x100;
	strcat (data->APD_Info, "\nSet BIAS�ֶ�");   
	if(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel])<0)
	{
		strcat (data->APD_Info, "ʧ��");    
		return -1; 
	}
	
	strcat (data->APD_Info, "\nSet BIAS DAC=100");  
	if(DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100)<0)
	{
		strcat (data->APD_Info, "ʧ��");
	  	return -1;
	}

	//����ռ�ձ�100% 
	strcat (data->APD_Info, "\nSet 100%");  
	if(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//New ����---4ͨ��ͬʱ����
	strcat (data->APD_Info, "\n4ͨ������01"); 
	
	for (index=0;index<WAVELENGTHCHANNEL;index++)
	{
		TargetWL[index]=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset; 
		sprintf(strTmp,"\nTargetWL%d=%f��",index,TargetWL[index]);
		strcat (data->APD_Info, strTmp);
	}
	
	for (index=0;index<WAVELENGTHCHANNEL;index++)
	{
		if(1==UpFlag[index])
		{
			//Set APC &Mod
			if (!my_struConfig.DT_Tuning)
			{
				strcat (data->APD_Info, "\nRead LUK6 APC"); 
				if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
		
				strcat (data->APD_Info, "\nRead LUK6 MOD");  
				if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);    
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
				strcat (data->APD_Info, strTmp);  
				if(SET_DAC_MOD(channel, data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
			else
			{
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
		
				sprintf(strTmp,"\n\nSet MOD DAC=0x28");   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_MOD(channel, 0x28)<0)  						//����Mod����ֵ0x28
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
			
			//Set PID
			sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
			strcat (data->APD_Info, strTmp);
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0)
			{
				strcat (data->APD_Info, "ʧ��\n��д����"); 
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					strcat (data->APD_Info, "ʧ��");  
					return -1;
				}
				
				sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0) 
				{
					strcat (data->APD_Info, "ʧ��");  
					return -1;
				}
			}  
			data->PointCount++;
				
			//�ȴ�WL�ȶ�  
			strcat (data->APD_Info, "\nWait PID LOOK"); 
			if(Get_TecPID_LookState(channel)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;	
			}
	
			//��ȡWL
			strcat (data->APD_Info, "\nRead WL");
			if (Read_Wavelength(channel, &WL_Array[index])<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;
			}
			sprintf(strTmp,"WL_Array[%d]=%f",index,WL_Array [index]);   
			strcat (data->APD_Info, strTmp);
		}
		else
		{
			;
		}
	}
	
	//������ݣ����ϵ��
	PolyFit(DAC_Array,WL_Array,WAVELENGTHCHANNEL,3,newWL_Array,Ratio_Array,&MeanSquaredError);  
		
	//�Ƶ�Ŀ��DAC
	error = Solving_Root(Ratio_Array,3,TargetWL,TargetDAC_Array);
	if(error<0)
	{
		return -1; 	
	}
			
	//��¼�����Ƶ�TEC DAC����ΪTunEr��׼��
	if (my_struConfig.Temper_Room)
	{
		data->TecDAC[0]=TargetDAC_Array[0]; 
		data->TecDAC[1]=TargetDAC_Array[1]; 
		data->TecDAC[2]=TargetDAC_Array[2]; 
		data->TecDAC[3]=TargetDAC_Array[3];
	}
	else
	{
		error = -1;
		goto Error;
	}
	
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningWavelength02(int channel, struct struTestData *data)
{
//	int count,count1, DAC,DAC0, DAC1,DACmin,DACmax, count_error, error; 
	


	double span;
	double deltaWL;

	int index;
	int count;
	int	error;
	char strTmp[1024]; 
	
	int	    ROOM_TEC[WAVELENGTHCHANNEL];
	double	ROOM_Temper[WAVELENGTHCHANNEL]; 

	
	//���ұ����
	unsigned short LukDACArr[3]; 
	double LukTemperArr[3];
	
	int MaxCount = 15;  
	
	BOOL 	tun2;
	int 	DACminArry[WAVELENGTHCHANNEL];
	int 	DACmaxArry[WAVELENGTHCHANNEL];	
	double	WavelegnthMin[WAVELENGTHCHANNEL];
	double	WavelegnthMax[WAVELENGTHCHANNEL];
	double	temp_Wavelegnth;
	double	Wavelegnth0; 
	int		DAC,DAC0,DACmin,DACmax;
	double  Slope,Offset; 
	
	//New������ر���
	double	DAC_Array[WAVELENGTHCHANNEL];
	double	WL_Array[WAVELENGTHCHANNEL];
	double	newWL_Array[WAVELENGTHCHANNEL];
	double	Ratio_Array[WAVELENGTHCHANNEL];
	double	MeanSquaredError;
	double	TargetDAC_Array[WAVELENGTHCHANNEL];
	double	TargetWL[WAVELENGTHCHANNEL];
	BOOL	okFlag;
	int		UpFlag[WAVELENGTHCHANNEL]={1,1,1,1};
	int		order=3;
	
	 

	memset (strTmp, 0, sizeof(strTmp)); 

	DACminArry[0]=0x700;
	DACminArry[1]=0x800;
	DACminArry[2]=0x900;
	DACminArry[3]=0xA00; 
	
	DACmaxArry[0]=0x900;
	DACmaxArry[1]=0xA00;
	DACmaxArry[2]=0xB00;
	DACmaxArry[3]=0xC00;

	DAC_Array[0]=data->TecDAC[0]; 
	DAC_Array[1]=data->TecDAC[1]; 
	DAC_Array[2]=data->TecDAC[2]; 
	DAC_Array[3]=data->TecDAC[3]; 
	
	//New ����---4ͨ��ͬʱ����
	strcat (data->APD_Info, "\n4ͨ������02"); 
	
	for (index=0;index<WAVELENGTHCHANNEL;index++)
	{
		TargetWL[index]=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset; 
		sprintf(strTmp,"\nTargetWL%d=%f��",index,TargetWL[index]);
		strcat (data->APD_Info, strTmp);
	}
	
	count=0;
	data->PointCount=0;
	okFlag=FALSE;
	do
	{
		for (index=0;index<WAVELENGTHCHANNEL;index++)
		{
			if(1==UpFlag[index])
			{
				data->PointCount++; 
				//Set APC &Mod
				if (!my_struConfig.DT_Tuning)
				{
					strcat (data->APD_Info, "\nRead LUK6 APC"); 
					if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					strcat (data->APD_Info, "\nRead LUK6 MOD");  
					if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
					strcat (data->APD_Info, strTmp);    
					if(SET_DAC_APC(channel, data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
			
					sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
					strcat (data->APD_Info, strTmp);  
					if(SET_DAC_MOD(channel, data->ucRm[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
				}
				else
				{
					sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
					strcat (data->APD_Info, strTmp);
					if(SET_DAC_APC(channel, data->ucRb[index])<0)
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
		
					sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
					strcat (data->APD_Info, strTmp);
					if(SET_DAC_MOD(channel, data->ucRm[index])<0)  				
					{
						strcat (data->APD_Info, "ʧ��");
						return -1; 
					}
				}
				
				//Set PID
				sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0)
				{
					strcat (data->APD_Info, "ʧ��\n��д����"); 
					error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
					if (error) 
					{
						strcat (data->APD_Info, "ʧ��");  
						return -1;
					}
					
					sprintf(strTmp,"\nSet DAC_Array[%d]=%f",index,DAC_Array[index]);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC_Array[index])<0) 
					{
						strcat (data->APD_Info, "ʧ��");  
						return -1;
					}
				}   
				
				//�ȴ�WL�ȶ�  
				strcat (data->APD_Info, "\nWait PID LOOK"); 
				if(Get_TecPID_LookState(channel)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;	
				}
	
				//��ȡWL
				strcat (data->APD_Info, "\nRead WL");
				if (Read_Wavelength(channel, &WL_Array [index])<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
				sprintf(strTmp,"WL_Array[%d]=%f",index,WL_Array [index]);   
				strcat (data->APD_Info, strTmp);
			}
			else
			{
				;
			}
		}
	
		if((fabs(WL_Array[0]-TargetWL[0])<TunWLAccuracy) && (fabs(WL_Array[1]-TargetWL[1])<TunWLAccuracy) && (fabs(WL_Array[2]-TargetWL[2])<TunWLAccuracy) && (fabs(WL_Array[3]-TargetWL[3])<TunWLAccuracy))
		{
			okFlag=TRUE;
			data->TecDAC[0]=DAC_Array[0]; 
			data->TecDAC[1]=DAC_Array[1]; 
			data->TecDAC[2]=DAC_Array[2]; 
			data->TecDAC[3]=DAC_Array[3]; 
			data->Peakwavelength[0] =WL_Array[0];
			data->Peakwavelength[1] =WL_Array[1]; 
			data->Peakwavelength[2] =WL_Array[2]; 
			data->Peakwavelength[3] =WL_Array[3]; 
		}
		else
		{
	  		PolyFit(DAC_Array,WL_Array,WAVELENGTHCHANNEL,3,newWL_Array,Ratio_Array,&MeanSquaredError);  
		
		 	error = Solving_Root(Ratio_Array,3,TargetWL,TargetDAC_Array);
			if(error<0)
			{
				return -1; 	
			}
			
			for (index=0;index<WAVELENGTHCHANNEL;index++)  
			{
				if(fabs(WL_Array[index]-TargetWL[index])>TunWLAccuracy)
				{
					UpFlag[index]=1;  
				
					if((0==index) && (WL_Array[index]-TargetWL[index])>0)
					{
						DAC_Array[index] = TargetDAC_Array[index]-0x05;
					}   
					else if((4==index) && (WL_Array[index]-TargetWL[index])<0)
					{
						DAC_Array[index] = TargetDAC_Array[index]+0x05;
					} 
					else
					{
						deltaWL= WL_Array[index]-TargetWL[index];
						if(fabs(deltaWL)< 0.015)
						{
							DAC_Array[index]= DAC_Array[index] - deltaWL/0.002;   
						}
						else
						{
							DAC_Array[index] = TargetDAC_Array[index];  
						}
					}
				}
				else
				{
					UpFlag[index]=0; 	
				}
			}  
		}   
		count++;
	}while(count<10 && okFlag==FALSE);
	
	if(count>=10)
	{	 
		strcat (data->APD_Info, "\n4ͨ������ʧ��\n��ͨ������");   
		
		for (index = (WAVELENGTHCHANNEL-1); index>=0; index--)					//����ͨ�����Ե������ȵ���4ͨ��
		{
			tun2=FALSE;
			//������ 
			sprintf(strTmp,"\nTun Ch%d",index+1);
			strcat (data->APD_Info, strTmp); 
		
			WavelegnthMin[index]=TargetWL[index] - TunWLAccuracy;				
			WavelegnthMax[index]=TargetWL[index] + TunWLAccuracy;		
			
			sprintf(strTmp,"\nWL_Min=%f;WL_Max=%f",WavelegnthMin[index],WavelegnthMax[index]);
			strcat (data->APD_Info, strTmp); 
		
			//Set APC &Mod
			if (!my_struConfig.DT_Tuning)
			{
				strcat (data->APD_Info, "\nRead LUK6 APC"); 
				if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				strcat (data->APD_Info, "\nRead LUK6 MOD");  
				if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);    
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
				strcat (data->APD_Info, strTmp);  
				if(SET_DAC_MOD(channel, data->ucRm[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
			else
			{
				sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_APC(channel, data->ucRb[index])<0)
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			
				sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[3]);   
				strcat (data->APD_Info, strTmp);
				if(SET_DAC_MOD(channel, data->ucRm[3])<0)  
				{
					strcat (data->APD_Info, "ʧ��");
					return -1; 
				}
			}
		
			//����Tec DAC��С
			DAC0= DACminArry[index]; 
			sprintf(strTmp,"\nSet TEC DAC0=%d",DAC0);   
			strcat (data->APD_Info, strTmp);
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC0)<0)
			{
				strcat (data->APD_Info, "ʧ��\n��д����");  
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					strcat (data->APD_Info, "ʧ��");   
					return -1;
				}
				
				sprintf(strTmp,"\nReSet TEC DAC0=%d",DAC0);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC0)<0) 
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
			}
		
			//�ȴ�WL�ȶ�  
			strcat (data->APD_Info, "\nWait PID LOOK");  
			if(Get_TecPID_LookState(channel)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;	
			}
		
			//��ȡWL
			strcat (data->APD_Info, "\nRead WL");   
			if (Read_Wavelength(channel, &Wavelegnth0)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;
			}
			sprintf(strTmp,"=%f",Wavelegnth0);   
			strcat (data->APD_Info, strTmp);

			//����Tec DAC���
			DAC=DACmaxArry[index];
			count=0;
			do
			{
				sprintf(strTmp,"\nSet TEC DAC=%d",DAC);   
				strcat (data->APD_Info, strTmp);
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0)
				{
					strcat (data->APD_Info, "ʧ��\n��д����");  
					error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
					if (error) 
					{
						strcat (data->APD_Info, "ʧ��");   
						return -1;
					}
				
					sprintf(strTmp,"\n\nReSet TEC DAC=%d",DAC);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0) 
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;
					}
				}
		
				//�ȴ�WL�ȶ�  
				strcat (data->APD_Info, "\nWait PID LOOK");  
				if(Get_TecPID_LookState(channel)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;	
				}
		
				//��ȡWL
				strcat (data->APD_Info, "\nRead WL");   
				if (Read_Wavelength(channel, &temp_Wavelegnth)<0)
				{
					strcat (data->APD_Info, "ʧ��");     
					return -1;
				}
				sprintf(strTmp,"=%f",temp_Wavelegnth);   
				strcat (data->APD_Info, strTmp);
			
				//�ж�
				if (temp_Wavelegnth>=WavelegnthMin[index] && temp_Wavelegnth<=WavelegnthMax[index]) 
				{  	
					strcat (data->APD_Info, "\nWL TunOK"); 
					break;
				}
				else
				{
					//����Slope Offset,�Ƶ�Ŀ��DAC
					Slope=(temp_Wavelegnth-Wavelegnth0) / (DAC-DAC0);
					Offset=temp_Wavelegnth-(Slope* DAC);
					
					sprintf(strTmp,"\nSlope=%f,Offset=%f",Slope,Offset);   
					strcat (data->APD_Info, strTmp);
					
					if(fabs(temp_Wavelegnth-TargetWL[index]) < fabs(Wavelegnth0-TargetWL[index]))
					{
						Wavelegnth0= temp_Wavelegnth;
						DAC0=DAC;
					}  	
					
					DAC=(TargetWL[index]-Offset)/Slope; 
					if(DAC0>0xF10) { DAC0=0xF10;}
					if(DAC0<0x500) { DAC0=0x500;}
				}

			}while(count<6);
			
			if(count>=6) 
			{
				tun2=TRUE;
			}
		
			if(tun2)
			{
				//���ַ�������
				DACmin = DAC-0x50;
				DACmax = DAC+0x50;	
				
				sprintf(strTmp,"\n���ַ�,DAC��Χ:%d~%d,Offset=%f",DACmin,DACmax);   
				strcat (data->APD_Info, strTmp);
			
				count=0;  
				span=2;
				DAC=(DACmin+DACmax)/2;
				do
				{
					sprintf(strTmp,"\nSet TEC DAC=%d",DAC);   
					strcat (data->APD_Info, strTmp);
					if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0)
					{
						strcat (data->APD_Info, "ʧ��\n��д����");  
						error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
						if (error) 
						{
							strcat (data->APD_Info, "ʧ��");   
							return -1;
						}
				
						sprintf(strTmp,"\n\nReSet TEC DAC=%d",DAC);   
						strcat (data->APD_Info, strTmp);
						if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0) 
						{
							strcat (data->APD_Info, "ʧ��");     
							return -1;
						}
					}
		
					//�ȴ�WL�ȶ�  
					strcat (data->APD_Info, "\nWait PID LOOK");  
					if(Get_TecPID_LookState(channel)<0)
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;	
					}
		
					//��ȡWL
					strcat (data->APD_Info, "\nRead WL");   
					if (Read_Wavelength(channel, &temp_Wavelegnth)<0)
					{
						strcat (data->APD_Info, "ʧ��");     
						return -1;
					}
					sprintf(strTmp,"=%f",temp_Wavelegnth);   
					strcat (data->APD_Info, strTmp);
					
					if (temp_Wavelegnth>=WavelegnthMin[index] && temp_Wavelegnth<=WavelegnthMax[index])    
					{  		
						break;
					}
					else if((DACmax-DACmin)<2) 
					{
						sprintf(strTmp,"\nDACmax-DACmax<2,ʧ��",DACmax,DACmin);  
						strcat (data->APD_Info, strTmp); 
						error = -1;
						goto Error;
					} 
					else
					{
						if(temp_Wavelegnth>TargetWL[index]) 
						{
							DACmax=DAC0;
							DAC0=(DACmax+DACmin)/2;
							sprintf(strTmp,"\nDACmin=%d,DACmax=%d",DACmin,DACmax);  
							strcat (data->APD_Info, strTmp); 
						}
						else					 
						{
							DACmin=DAC0;
							DAC0=(DACmax+DACmin)/2;
							sprintf(strTmp,"\nDACmin=%d,DACmax=%d",DACmin,DACmax);  
							strcat (data->APD_Info, strTmp); 
						}
					}
					count++;
					if(fabs(temp_Wavelegnth-TargetWL[index])<1.0)
					{
						span=1;
					}
					else
					{
						;
					}
				}while (count<MaxCount);
	
				if (count >= MaxCount) 
				{
					sprintf(strTmp,"\nTun Conut>=%d,ʧ��",count);  
					strcat (data->APD_Info, strTmp); 
					error = -1; 
					goto Error;
				} 
			}
	
			//��¼�����¶ȼ�DAC
			errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));
			data->Peakwavelength[index] =temp_Wavelegnth; 
			data->TecDAC[index]=DAC;
			
			//�����¶�������²��ұ�
			if (my_struConfig.Temper_Room)
			{
				//LUKдֱ��
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=data->TecDAC[index];
				LukDACArr[2]=data->TecDAC[index]; 
				
				LukTemperArr[0] =-40.;  
				LukTemperArr[1] = data->TecCoreTemper[index];   
				LukTemperArr[2] =100;
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, LukDACArr, LukTemperArr)); 					//����дK=0;
			}
			else if(my_struConfig.Temper_High)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
			
				LukDACArr[0]=ROOM_TEC[index];
				LukDACArr[1]=data->TecDAC[index];
				
				LukTemperArr[0] = ROOM_Temper[index]; 
				LukTemperArr[1] = data->TecCoreTemper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, LukDACArr, LukTemperArr));  
			}
			else if(my_struConfig.Temper_Low)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
			
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=ROOM_TEC[index];  
			
				LukTemperArr[0] = data->TecCoreTemper[index];
				LukTemperArr[1] = ROOM_Temper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, LukDACArr, LukTemperArr));  
			}
			else
			{
				error = -1;
				goto Error;
			}   
		}
	}
	else
	{
		sprintf(strTmp,"\nPID Set����=%d",data->PointCount);   
		strcat (data->APD_Info, strTmp);
		for (index = (WAVELENGTHCHANNEL-1); index>=0; index--)
		{
			//��¼�����¶ȼ�DAC
			errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));

			//�����¶�������²��ұ�
			if (my_struConfig.Temper_Room)
			{
				//LUKдֱ��
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=data->TecDAC[index];
				LukDACArr[2]=data->TecDAC[index]; 
				
				LukTemperArr[0] =-40.;  
				LukTemperArr[1] = data->TecCoreTemper[index];   
				LukTemperArr[2] =100;
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, LukDACArr, LukTemperArr)); 					//����дK=0;
			}
			else if(my_struConfig.Temper_High)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
			
				LukDACArr[0]=ROOM_TEC[index];
				LukDACArr[1]=data->TecDAC[index];
				
				LukTemperArr[0] = ROOM_Temper[index]; 
				LukTemperArr[1] = data->TecCoreTemper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, LukDACArr, LukTemperArr));  
			}
			else if(my_struConfig.Temper_Low)
			{
				//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
				errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
			
				LukDACArr[0]=data->TecDAC[index];
				LukDACArr[1]=ROOM_TEC[index];  
			
				LukTemperArr[0] = data->TecCoreTemper[index];
				LukTemperArr[1] = ROOM_Temper[index];
			
				//Set Tec LUK
				errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, LukDACArr, LukTemperArr));  
			}
			else
			{
				error = -1;
				goto Error;
			}
		}
	}

	//Save LUK  
	if(!my_struConfig.Temper_Room)
	{
		errChk(DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]));		//����TEC Auto��ģʽ
	}
	
	if(DWDM_ONU_DS4830_Update_LUK2(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK3(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK4(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK5(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
		
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningWavelength_FineTuning(int channel, struct struTestData *data,int index)		//����΢��
{
	double span;
	double deltaWL;

	int count;
	int	error;
	char strTmp[1024]; 
	
	int	    ROOM_TEC[WAVELENGTHCHANNEL];
	double	ROOM_Temper[WAVELENGTHCHANNEL]; 

	
	//���ұ����
	unsigned short LukDACArr[3]; 
	double LukTemperArr[3];
	
	int MaxCount = 15;  
	
	BOOL 	tun2;
//	int 	DACminArry[WAVELENGTHCHANNEL];
//	int 	DACmaxArry[WAVELENGTHCHANNEL];	
//	double	WavelegnthMin[WAVELENGTHCHANNEL];
//	double	WavelegnthMax[WAVELENGTHCHANNEL];
//	double	temp_Wavelegnth;
//	double	Wavelegnth0; 
	int		DAC,DAC0,DACmin,DACmax;
//	double  Slope,Offset; 
	
	//New������ر���
//	double	DAC_Array[WAVELENGTHCHANNEL];
	double	WL_Array[WAVELENGTHCHANNEL];
//	double	newWL_Array[WAVELENGTHCHANNEL];
//	double	Ratio_Array[WAVELENGTHCHANNEL];
//	double	MeanSquaredError;
//	double	TargetDAC_Array[WAVELENGTHCHANNEL];
	double	TargetWL[WAVELENGTHCHANNEL];
//	BOOL	okFlag;
//	int		UpFlag[WAVELENGTHCHANNEL]={1,1,1,1};
//	int		order=3;
	
	 

	memset (strTmp, 0, sizeof(strTmp)); 
	
	//����ռ�ձ�100%
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
	//Set TxDisable
	if(EVB5_SET_BEN_Level (INST_EVB[channel], 0),0)
	{
		return -1;
	}  
	
	//д����
	error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
	if (error) 
	{
		strcat (data->APD_Info, "ʧ��");  
		return -1;
	}
	
	//ͨ��΢��
	strcat (data->APD_Info, "\n����΢��"); 
	
	TargetWL[index]=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset2; 

	count=0;
	data->PointCount=0;


	//���ùض����ֵΪ�ֶ�ģʽ��DAC=0x100;
/*	strcat (data->APD_Info, "\nSet BIAS�ֶ�");   
	if(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel])<0)
	{
		strcat (data->APD_Info, "ʧ��");    
		return -1; 
	}

	strcat (data->APD_Info, "\nSet BIAS DAC=100");  
	if(DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100)<0)
	{
		strcat (data->APD_Info, "ʧ��");
	  	return -1;
	}
*/			
	//Read APC 
	strcat (data->APD_Info, "\nRead LUK6 APC"); 
	if(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index])<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//Read MOD
	strcat (data->APD_Info, "\nRead LUK6 MOD");  
	if(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index])<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//Set APC 
	sprintf(strTmp,"\n\nSet APC DAC=%d",data->ucRb[index]);   
	strcat (data->APD_Info, strTmp);    
	if(SET_DAC_APC(channel, data->ucRb[index])<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//Set Mod  
	sprintf(strTmp,"\n\nSet MOD DAC=%d",data->ucRm[index]);   
	strcat (data->APD_Info, strTmp);  
	if(SET_DAC_MOD(channel, data->ucRm[index])<0)
	{
		strcat (data->APD_Info, "ʧ��");
		return -1; 
	}
	
	//Get Tec DAC
	if(DWDM_ONU_DS4830_Get_TEC_DAC7_Adjust7(INST_EVB[channel], &DAC) <0)
	{
		return -1;  
	}
	count=0;
	do
	{	
		data->PointCount++;   			
		//Set PID
		sprintf(strTmp,"\nSet DAC[%d]=%d",index,DAC);   
		strcat (data->APD_Info, strTmp);
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0)
		{
			strcat (data->APD_Info, "ʧ��\n��д����"); 
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				strcat (data->APD_Info, "ʧ��");  
				return -1;
			}
					
			sprintf(strTmp,"\nSet DAC[%d]=%d",index,DAC);   
			strcat (data->APD_Info, strTmp);
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC)<0) 
			{
				strcat (data->APD_Info, "ʧ��");  
				return -1;
			}
		}   
				
		//�ȴ�WL�ȶ�  
		strcat (data->APD_Info, "\nWait PID LOOK"); 
		if(Get_TecPID_LookState(channel)<0)
		{
			strcat (data->APD_Info, "ʧ��");     
			return -1;	
		}
	
		//��ȡWL
		strcat (data->APD_Info, "\nRead WL");
		if (Read_Wavelength(channel, &WL_Array [index])<0)
		{
			strcat (data->APD_Info, "ʧ��");     
			return -1;
		}
		sprintf(strTmp,"WL_Array[%d]=%f",index,WL_Array [index]);   
		strcat (data->APD_Info, strTmp);

		if((fabs(WL_Array[index]-TargetWL[index])<TunWLAccuracy))
		{
			break;
		}
		else
		{
	  		if(WL_Array[index]>TargetWL[index])
			{
				DAC--;
				if(DAC<0x600)
				{
					DAC=0x600;
				}
			}
			else
			{
				DAC++;
				if(DAC>0xF00)
				{
					DAC=0xF00;
				}
			}
			
		}   
		count++;
	}while(count<20);
	
	if(count>=20)
	{	 
		error = -1;
		goto Error;
	}
	
	//��¼�����¶ȼ�DAC
	errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));
	data->Peakwavelength[index] =WL_Array[index]; 
	data->TecDAC[index]=DAC;
			
	//�����¶�������²��ұ�
	if (my_struConfig.Temper_Room)
	{
		//LUKдֱ��
		LukDACArr[0]=data->TecDAC[index];
		LukDACArr[1]=data->TecDAC[index];
		LukDACArr[2]=data->TecDAC[index]; 
				
		LukTemperArr[0] =-40.;  
		LukTemperArr[1] = data->TecCoreTemper[index];   
		LukTemperArr[2] =100;
			
		//Set Tec LUK
		errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, LukDACArr, LukTemperArr)); 					//����дK=0;
	}
	else if(my_struConfig.Temper_High)
	{
		//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
		errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
	
		LukDACArr[0]=ROOM_TEC[index];
		LukDACArr[1]=data->TecDAC[index];
			
		LukTemperArr[0] = ROOM_Temper[index]; 
		LukTemperArr[1] = data->TecCoreTemper[index];
		
		//Set Tec LUK
		errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, LukDACArr, LukTemperArr));  
	}
	else if(my_struConfig.Temper_Low)
	{
		//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
		errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
	
		LukDACArr[0]=data->TecDAC[index];
		LukDACArr[1]=ROOM_TEC[index];  
		
		LukTemperArr[0] = data->TecCoreTemper[index];
		LukTemperArr[1] = ROOM_Temper[index];
			
		//Set Tec LUK
		errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, LukDACArr, LukTemperArr));  
	}
	else
	{
		error = -1;
		goto Error;
	}   

	//Save LUK  
	errChk(DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]));		//����TEC Auto��ģʽ

	
	if(DWDM_ONU_DS4830_Update_LUK2(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK3(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK4(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK5(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
		
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}

int tuningWavelength_HightLow(int channel, struct struTestData *data)		//�ߵ��µ��Բ������������ó���DACΪ������С��Χ���ַ�����
{
	int count,DAC0,DAC1,DACmin,DACmax,count_error,error; 
	int DACminArry[4];
	int DACmaxArry[4];
	int DACPoint[3];
	double span;
	double	temp_Wavelegnth, temp_Wavelegnthmin, temp_Wavelegnthmax, temp_WavelegnthAvg; 
	double	temp_Wavelegnth_01,Wavelegnth_delta;
	int index;
	int ROOM_TEC[4];
	double ROOM_Temper[4]; 
	double temp_AOP;
	unsigned short DAC_Arr[3];
	double Temper_Arr[3];
	int MaxCount = 15;  
	
	double k1,k2,k3;
	double b1,b2,b3;
	double T1,T0;
	double Laser_Tempe;
	
	char tempbuf[256];
	int LookFlag;
	
	memset(tempbuf,0,sizeof(tempbuf));
	
	k1=0.0155;
	k2=0.0229;
	k3=0.0361;
	b1=-4.3177;
	b2=-25.631;
	b3=-72.231;
	DACPoint[0]=2841;
	DACPoint[1]=3506;
	DACPoint[2]=4096;   
	
	memset (DAC_Arr, 0, sizeof(DAC_Arr));
	memset (Temper_Arr, 0, sizeof(Temper_Arr)); 

	DACminArry[0]=0x600;
	DACminArry[1]=0xA00;
	DACminArry[2]=0xC00;
	DACminArry[3]=0xDD0;
	
	DACmaxArry[0]=0xA00;
	DACmaxArry[1]=0xC00;
	DACmaxArry[2]=0xDD0;
	DACmaxArry[3]=0xF10;

	//����TEC�ֶ�ģʽ
	errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));
	//���ùض����ֵΪ�ֶ�0x100;
	errChk(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]));
	error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
	if(error)
	{
	//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
	
	//����ռ�ձ�1% 
	//set ben to Level mode
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));  
	//100k hz
	errChk(EVB5_SET_CLOCK (INST_EVB[channel], 0.008));
	errChk(EVB5_SET_BEN_DELAY(INST_EVB[channel], 1, 1, 60));    
	Delay(1);
	
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ַ�������  
		if(my_struConfig.Temper_High)  
		{
		
			temp_WavelegnthAvg=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset;
		}
		else
		{
			temp_WavelegnthAvg=(my_struDBConfig.PeakWavelengthMax[index]+my_struDBConfig.PeakWavelengthMin[index])/2.0+TunWLOffset;
		}
		temp_Wavelegnthmin=temp_WavelegnthAvg - TunWLAccuracy;	
		temp_Wavelegnthmax=temp_WavelegnthAvg + TunWLAccuracy;
		
		//Set APC &Mod
		if (!my_struConfig.Temper_Room)
		{
			errChk(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index]));	
			errChk(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index]));
			
			errChk(SET_DAC_APC (channel, data->ucRb[index] ));	
			errChk(SET_DAC_MOD(channel, data->ucRm[index])); 
		}
		else
		{
			//Set Mod 0x2A
			errChk(SET_DAC_MOD(channel, 0x20));  
		}
		
		//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
		errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
		DAC0=ROOM_TEC[index];
		
		DACmax = DAC0 + 0x20;
		DACmin = DAC0 - 0x40 ;
		
			
		count=0;
		temp_Wavelegnth = 0.0;  
		span=2;    
		do
		{
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DAC0)<0)
			{
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					MessagePopup("����", "3328���ù���ģʽ����   "); 
					return -1;
				}
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], DACminArry[index])<0) 
				{
					return -1;
				}
			}
			
			//�ȴ������ȶ�
			if(Get_TecPID_LookState(channel)<0)
			{
				strcat (data->APD_Info, "ʧ��");     
				return -1;	
			}
		
			errChk (Read_Wavelength(channel, &temp_Wavelegnth)); 
//			if(temp_Wavelegnth<0)
//			{
//				errChk (MS9740_Config (Inst_MS9740, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMin[0]+my_struDBConfig.PeakWavelengthMax[4])/2.0,span, 0.03));
//				errChk (Read_Wavelength(channel, &temp_Wavelegnth)); 	
//			}
			if (temp_Wavelegnth>=temp_Wavelegnthmin && temp_Wavelegnth<=temp_Wavelegnthmax) 
			{  		
				break;
			}
			else if((DACmax-DACmin)<2) 
			{
			//	MessagePopup("Error", "���Բ���ʧ�ܣ�");
				data->ErrorCode=ERR_TUN_AOP;
				error = -1;
				goto Error;
			} 
			else
			{
				if(temp_Wavelegnth>temp_WavelegnthAvg) 
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else					 
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
			}
			count++;
			if(fabs(temp_Wavelegnth-temp_WavelegnthAvg)<1.0)
			{
				span=1;
			}
			else
			{
				;//	errChk (MS9740_Config (Inst_MS9740, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMin[index]+my_struDBConfig.PeakWavelengthMax[index])/2.0,span, 0.03));   
			}
		}while (count<MaxCount);
	
		if (count >= MaxCount) 
		{
		//	MessagePopup ("Error", "�⹦�ʵ��Գ�ʱ!");
			error = -1; 
			goto Error;
		} 

		//��¼�����¶ȼ�DAC
		errChk (DWDM_ONU_DS4830_Get_CoreTemper_Ex(INST_EVB[channel], &data->TecCoreTemper[index]));
		data->Peakwavelength[index] =temp_Wavelegnth; 
		data->TecDAC[index]=DAC0;
	
		//�����¶�ѡ�����ж��Ƿ�Щ���ұ�
		if (my_struConfig.Temper_Room)
		{
			//LUKдֱ��
			DAC_Arr[0]=data->TecDAC[index];
			Temper_Arr[0] =-40.;   
			DAC_Arr[1]=data->TecDAC[index];
			Temper_Arr[1] = data->TecCoreTemper[index]; 
			DAC_Arr[2]=data->TecDAC[index];
			Temper_Arr[2] =100;
			
			errChk (DWDM_ONU_DS4830_Write_TEC_Luk_Ex (INST_EVB[channel], index, DAC_Arr, Temper_Arr)); 					//����дK=0;
		}
		else if(my_struConfig.Temper_High)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
			errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));
			
			DAC_Arr[0]=ROOM_TEC[index];
			Temper_Arr[0] = ROOM_Temper[index];
			
			DAC_Arr[1]=data->TecDAC[index];
			Temper_Arr[1] = data->TecCoreTemper[index];
			
			//Check AOP
//			errChk (Read_AOP(channel, &temp_AOP)); 	
//			if (temp_AOP>my_struDBConfig.TxPowMax || temp_AOP<my_struDBConfig.TxPowMin) 
//			{
//				MessagePopup ("Error", "�⹦�ʼ��ʧ��!");   
//				error = -1; 
//				goto Error;
//			} 
//		
			errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, TRUE, DAC_Arr, Temper_Arr));  
		}
		else if(my_struConfig.Temper_Low)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
			errChk (DB_Get_Room_TEC(data->OpticsSN, ROOM_TEC, ROOM_Temper));   
			
			DAC_Arr[0]=data->TecDAC[index];
			Temper_Arr[0] = data->TecCoreTemper[index];
			
			DAC_Arr[1]=ROOM_TEC[index];
			Temper_Arr[1] = ROOM_Temper[index];
			
			//Set Tec LUK
			errChk (DWDM_ONU_DS4830_Write_TEC_Luk (INST_EVB[channel], index, FALSE, DAC_Arr, Temper_Arr)); 
		}
		else
		{
			error = -1;
			goto Error;
		}  
	}
	//Save LUK  
	if(!my_struConfig.Temper_Room)
	{
		errChk(DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]));		//����TEC Auto��ģʽ
	}
	
	if(DWDM_ONU_DS4830_Update_LUK2(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK3(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK4(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK5(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
		
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
//		ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
}  

int Read_Wavelength(int channel, double *wavelength)
{
	double Sigma;
	double BandWidth;
	double SMSR;
	int error;
	double TempWaveLenght[10]={0.0};
	double Temp_Sum=0.0;
	double Temp_power;
	double Temp_WL0,Temp_WL1;
	int count=0;
	BOOL Flag=FALSE;
		   
	//���Բ���   
	if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AG8614X) 
	{
		if (!Ag86142_Read(LASER_TYPE_DFB, inst_Ag86142, wavelength, &Sigma, &BandWidth, &SMSR)) 
		{
			return -1;
		}
	}
	else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AQ633X)
	{
		error = AQ633X_Read (Inst_AQ633X, LASER_TYPE_DFB, 30.0, wavelength, &Sigma, &BandWidth, &SMSR);
		if (error) 
		{
			MessagePopup("Error", "Read spectrum aq633x error"); 
			return -1;
		}
	}
	else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AQ637X)
	{
		do
		{
			error = AQ637X_Read (Inst_AQ637X, LASER_TYPE_DFB, 30.0,&TempWaveLenght[count], &Sigma, &BandWidth, &SMSR);
			if (error) 
			{
				MessagePopup("Error", "Read spectrum aq633x error"); 
				return -1;
			}
			Temp_Sum+=TempWaveLenght[count];
			count++;
		}while(count<10);
	    *wavelength=Temp_Sum/10;	
	}
	else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_MS9740)
	{
		error = MS9740_Read (Inst_MS9740, LASER_TYPE_DFB, (my_struDBConfig.PeakWavelengthMax[channel]+my_struDBConfig.PeakWavelengthMin[channel])/2.0, 30.0, wavelength, &Sigma, &BandWidth, &SMSR);
		if (error) 
		{
			MessagePopup("Error", "Read spectrum aq633x error"); 
			return -1;
		}
	}
	else if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_AG86120B)
	{
//		count=0;
//		Flag=FALSE;
//		do
//		{
			error = Ag86120B_Read (Inst_AG86120B, wavelength, &Temp_power); 
			if (error) 			   
			{
				MessagePopup("Error", "Read WaveLength Ag86120B error"); 
				return -1;
			}
			*wavelength= *wavelength + my_struConfig.WLMeterOffset;
/*			if(0==count%2)
			{
				Temp_WL0=*wavelength;
			}
			else
			{
				Temp_WL1=*wavelength;  
			}
			if(count>0)			  
			{
				Flag = (fabs(Temp_WL0-Temp_WL1) < 0.005);
			}
			Delay(0.5);
			count++;
		}while( !Flag && (count<5));
		if(count >=5)
		{
			return -1;	
		}
*/		
	}
	else 
	{
		MessagePopup("Error", "Can not find this spectrum type");
		return -1;
	} 
	
	return 0;
}

int DB_Get_Room_TEC (char *OpticsSN, int *ROOM_TEC, double *ROOM_Temper)
{
	double TempROOMTemper[4];
	int    TempROOMTec[4];
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	memset(&TempROOMTemper, 0, sizeof(TempROOMTemper));
	memset(&TempROOMTec, 0, sizeof(TempROOMTec));
	
	sprintf (buf, "select TECDAC0,TECDAC1,TECDAC2,TECDAC3,CORETEMP_TEC0,CORETEMP_TEC1,CORETEMP_TEC2,CORETEMP_TEC3 from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='TUNING' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColInt (hstmt, 1, &TempROOMTec[0], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 2, &TempROOMTec[1], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 3, &TempROOMTec[2], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 4, &TempROOMTec[3], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 5, &TempROOMTemper[0], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 6, &TempROOMTemper[1], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 7, &TempROOMTemper[2], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 8, &TempROOMTemper[3], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 
	for(i=0;i<4;i++)
	{
		ROOM_TEC[i] = TempROOMTec[i];
		ROOM_Temper[i] = TempROOMTemper[i]; 
	}
	
	return 0; 
}

int DB_Get_Room_TxOffDepth (char *OpticsSN, int *ROOM_DAC, double *ROOM_Temper)
{
	double TempROOMTemper[4];
	int    TempROOMDac[4];
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	memset(&TempROOMTemper, 0, sizeof(TempROOMTemper));
	memset(&TempROOMDac, 0, sizeof(TempROOMDac));
	
	sprintf (buf, "select TXOFFDEPTH_DAC0,TXOFFDEPTH_DAC1,TXOFFDEPTH_DAC2,TXOFFDEPTH_DAC3,CORETEMP_TXOFFDEPTH0,CORETEMP_TXOFFDEPTH1,CORETEMP_TXOFFDEPTH2,CORETEMP_TXOFFDEPTH3 from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='TUNING' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColInt (hstmt, 1, &TempROOMDac[0], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 2, &TempROOMDac[1], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 3, &TempROOMDac[2], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColInt (hstmt, 4, &TempROOMDac[3], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 5, &TempROOMTemper[0], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 6, &TempROOMTemper[1], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 7, &TempROOMTemper[2], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 8, &TempROOMTemper[3], &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 
	for(i=0;i<4;i++)
	{
		ROOM_DAC[i] = TempROOMDac[i];
		ROOM_Temper[i] = TempROOMTemper[i]; 
	}
	
	return 0; 
} 

int DB_Get_Room_Vapd_DAC (char *OpticsSN, int *ROOM_TEC, double *ROOM_Temper)
{
	double TempROOMTemper;
	int    TempROOMTec;
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	memset(&TempROOMTemper, 0, sizeof(TempROOMTemper));
	memset(&TempROOMTec, 0, sizeof(TempROOMTec));
	
	sprintf (buf, "select RxTr,RxTf from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='TUNING' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColDouble (hstmt, 1, &TempROOMTemper, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 2, &TempROOMTemper, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 

	*ROOM_TEC= TempROOMTec;
	*ROOM_Temper = TempROOMTemper; 

	
	return 0; 
}

int DB_Get_AginFront_TxPower (char *OpticsSN, double *val)
{
	double tempval;
	int    TempROOMTec;
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	sprintf (buf, "select TxAOP from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='AGINFRONT_TEST' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColDouble (hstmt, 1, &tempval, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 
	*val = tempval;
	return 0; 
}  

int DB_Get_AginFront_WaveLength (char *OpticsSN, double *val)
{
	double tempval;
	int    TempROOMTec;
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	sprintf (buf, "select peakwl_dutyratio99_0 from autodt_results_ate where ID=(select max(resultsid) from autodt_process_log where SN='%s' AND LOG_ACTION='AGINFRONT_TEST' AND P_VALUE='PASS')", OpticsSN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColDouble (hstmt, 1, &tempval, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 
	*val = tempval;
	return 0; 
} 

int DB_Get_TunTxOffPower_Spec (double *min ,double *max)
{
	double tempmin,tempmax;
	int    TempROOMTec;
	char buf[1024];  
	int num=0;
	
	int  retParam;
	int  ParamLen;
	int  resCode;
	int  hstmt;
	int i;
	
	sprintf (buf, "select txoffpower_min,txoffpower_max from autodt_spec_ate  where partnumber='%s' and dt_flag='TUNING'", my_struConfig.PN);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
	
	resCode = DBBindColDouble (hstmt, 1, &tempmin, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColDouble (hstmt, 2, &tempmax, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if (resCode != DB_SUCCESS && resCode != DB_EOF)
	{
		ShowDataBaseError();  
		return -1;
	}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS)
	{
		ShowDataBaseError(); 
		return -1;
	}
																			   
	if (num!=1) 
	{
		MessagePopup("��ʾ","���ݿ�Data�в����ҵ���ӦSN�ĳ��²����������ݣ�    "); 
		return -2;
	} 
	*min = tempmin;
	*max = tempmax;
	return 0; 
} 

int TunVapd (int channel, struct struTestData *data)
{
	double K, K1, K2, K3; 
	
	char tmp[256] = "";
	int error;
	int DAC30;
	int DAC_arr[3];
	int Temper_arr[3];
	
	float Slope_arr[2];
	float Offset_arr[2];
	INT16U wSlope;
	INT16U wOffset;
	
	double temper;
	
	float Vbr_Class1 = 3.;   
	int index;
	
	int fitNumber = 3;
	
	INT16U APD_Step  = 0x40;		 //0.5V
	INT16U APD_Start = 0x500; 
	INT16U APD_End   = 0x220;
	INT16U APD_Step_Min = 0x20;		//0.2V
	
	INT16U RX_ADC0 = 0;
	INT16U RX_ADC = 0; 
	INT16U RX_ADC_Min = 250;  

	INT16U temparry[1024];
	memset(temparry,0,sizeof(temparry));
	
	error = SET_ATT_ONU(channel, -27+sRxCal.power[channel]);   
	if (error<0)
	{
		goto Error;
	}
	
	//�ֶ�ģʽ
	strcpy (data->APD_Info, "TUN APD\n�ֶ�ģʽ"); 
	error = DWDM_ONU_DS4830_Set_APD_Manual (INST_EVB[channel]);	  //ֻд���ұ������л����ֶ�ģʽ
	if (error) 
	{
		strcat (data->APD_Info, "ʧ��"); 
		goto Error;
	}

	strcat (data->APD_Info, "\n�Զ���"); 

	DAC30 = APD_Start; 
	index=0;
	do
	{
		DAC30 = DAC30 - APD_Step;	 
		
		sprintf (tmp, "\nAPD=0x%02X", DAC30);
		strcat (data->APD_Info, tmp); 
		error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], DAC30);
		if (error)
		{
			strcat (data->APD_Info, "ʧ��"); 
		//	MessagePopup ("ERROR", "����APD DAC����"); 
			goto Error;      
		}
			
//		Delay (1.);
		RX_ADC = 0;
		error = DWDM_ONU_DS4830_Read_Rx_ADC (INST_EVB[channel], &RX_ADC);
		sprintf (tmp, "\nRxADC=0x%03X", RX_ADC);
		strcat (data->APD_Info, tmp);
		if (error)
		{
			strcat (data->APD_Info, "ʧ��");
		//	MessagePopup ("ERROR", "��ȡRX ADC����"); 
			goto Error;      
		}
	
		if ((fabs(RX_ADC0-RX_ADC) >= (RX_ADC_Min)) &&index>0)
		{	
//			if (APD_Step<=APD_Step_Min)
//			{
				sprintf (tmp, "\nRxADC=0x%03X>RxADCMIN=0x%03X,OK", RX_ADC, RX_ADC_Min);
				strcat (data->APD_Info, tmp);
				break;
//			}
//			else
//			{
//				DAC30 = DAC30 + APD_Step*2;
//				APD_Step = APD_Step/2;
//			}
		}
		temparry[index] =RX_ADC;
		index++;
		RX_ADC0=RX_ADC;
	}while (DAC30 > APD_End);     

	sprintf (tmp, "\nVbr DAC=0x%02X", DAC30);
	strcat (data->APD_Info, tmp); 
		
	DAC30 = DAC30 + Vbr_Class1/0.00797; 
	
	if (APD_End == DAC30)
	{
		DAC30 = DAC30 + Vbr_Class1/0.00797; 
	}
	sprintf (tmp, "\nAPD DAC=0x%02X", DAC30);
	strcat (data->APD_Info, tmp);  
	
	//��¼DAC && Temperature
	error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &temper);
	if (error)
	{
		goto Error;        
	}
	data->Vapd_DAC=(float)DAC30;
	data->Vapd_D_Temper=temper;
	
	error = DWDM_ONU_Save_APD_Luk(channel, DAC30, temper);
	if (error)
	{
		goto Error;        
	}
	
	return 0;       
	
Error:
	
	//�Զ�ģʽ
	error = DWDM_ONU_DS4830_Set_APD_Auto (INST_EVB[channel]);			  //������ʼ�����л����ֶ�ģʽ������Ҳ���ûظ����Զ�ģʽ
	if (error) 
	{
		return -1;
	}

	return -1; 
}

int TunVapd_EX (int channel, struct struTestData *data)			   //��Դ���ṩ����
{
	double K, K1, K2, K3; 
	
	char tmp[256] = "";
	int error;
	int DAC30;
	int DAC_arr[3];
	int Temper_arr[3];
	
	float Slope_arr[2];
	float Offset_arr[2];
	INT16U wSlope;
	INT16U wOffset;
	
	double temper;
	
	float Vbr_Class1 = 3.;   
	int index;
	
	int fitNumber = 3;
	
	INT16U APD_Step  = 0x10;
	INT16U APD_Start = 0x500; 
	INT16U APD_End   = 0x300;
	INT16U APD_Step_Min = 0x05;
	
	INT16U RX_ADC = 0;
	INT16U RX_ADC_Min = 0;  
	INT16U RX_ADC_Max = 0;    

	error = SET_ATT_ONU(channel, -20+sRxCal.power[channel]);   
	if (error<0)
	{
		goto Error;
	}
	
	//�ֶ�ģʽ
	strcpy (data->APD_Info, "�ֶ�ģʽ"); 
	error = DWDM_ONU_DS4830_Set_APD_Manual (INST_EVB[channel]);	  //ֻд���ұ������л����ֶ�ģʽ
	if (error) 
	{
		strcat (data->APD_Info, "ʧ��"); 
		goto Error;
	}
	
	error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], 0xFFF);
	if (error)
	{
		strcat (data->APD_Info, "ʧ��"); 
	//	MessagePopup ("ERROR", "����APD DAC����"); 
		goto Error;      
	}
		
	error = DWDM_ONU_DS4830_Read_Rx_ADC (INST_EVB[channel], &RX_ADC);
	if (error)
	{
		strcat (data->APD_Info, "ʧ��");
	//	MessagePopup ("ERROR", "��ȡRX ADC����"); 
		goto Error;      
	}
	RX_ADC_Max=RX_ADC+5;
	RX_ADC_Min=RX_ADC-5;
	
	error = SET_ATT_ONU(channel, -30+sRxCal.power[channel]);   
	if (error<0)
	{
		goto Error;
	}

	strcat (data->APD_Info, "\n�Զ���"); 

	DAC30 = APD_Start; 
	error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], DAC30);
	if (error)
	{
			strcat (data->APD_Info, "ʧ��"); 
		//	MessagePopup ("ERROR", "����APD DAC����"); 
			goto Error;      
	}
			
	Delay (1.);
	RX_ADC = 0;
	error = DWDM_ONU_DS4830_Read_Rx_ADC (INST_EVB[channel], &RX_ADC);
	sprintf (tmp, "\nRxADC=0x%03X", RX_ADC);
	strcat (data->APD_Info, tmp);
	if (error)
	{
		strcat (data->APD_Info, "ʧ��");
	//	MessagePopup ("ERROR", "��ȡRX ADC����"); 
		goto Error;      
	}
	do
	{
		if(RX_ADC>RX_ADC_Min && RX_ADC<RX_ADC_Max)
		{
			break;
		}
		else
		{
			if(RX_ADC>RX_ADC_Max)
			{
				DAC30 = DAC30 + APD_Step/2;
			}
			else
			{
				DAC30 = DAC30 - APD_Step;
			}
		}
			 
		
		sprintf (tmp, "\nAPD=0x%02X", DAC30);
		strcat (data->APD_Info, tmp); 
		error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], DAC30);
		if (error)
		{
			strcat (data->APD_Info, "ʧ��"); 
		//	MessagePopup ("ERROR", "����APD DAC����"); 
			goto Error;      
		}
			
		Delay (1.);
		RX_ADC = 0;
		error = DWDM_ONU_DS4830_Read_Rx_ADC (INST_EVB[channel], &RX_ADC);
		sprintf (tmp, "\nRxADC=0x%03X", RX_ADC);
		strcat (data->APD_Info, tmp);
		if (error)
		{
			strcat (data->APD_Info, "ʧ��");
		//	MessagePopup ("ERROR", "��ȡRX ADC����"); 
			goto Error;      
		}
		
		
	}while (DAC30 > APD_End);     

	sprintf (tmp, "\nVbr DAC=0x%02X", DAC30);
	strcat (data->APD_Info, tmp); 
		
	error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &temper);
	if (error)
	{
		goto Error;        
	}
	
	error = DWDM_ONU_Save_APD_Luk(channel, DAC30, temper);
	if (error)
	{
		goto Error;        
	}
	
	return 0;       
	
Error:
	
	//�Զ�ģʽ
	error = DWDM_ONU_DS4830_Set_APD_Auto (INST_EVB[channel]);			  //������ʼ�����л����ֶ�ģʽ������Ҳ���ûظ����Զ�ģʽ
	if (error) 
	{
		return -1;
	}

	return -1; 
}   

int TunVapd_HighLow (int channel, struct struTestData *data)			   //��Դ���ṩ����_�ߵ���
{
	double K, K1, K2, K3; 
	
	char tmp[256] = "";
	int error;
	int DAC30;
	int DAC_arr[3];
	int Temper_arr[3];
	
	float Slope_arr[2];
	float Offset_arr[2];
	INT16U wSlope;
	INT16U wOffset;
	
	double temper;
	
	float Vbr_Class1 = 3.;   
	int index;
	
	int fitNumber = 3;
	
	INT16U APD_Step  = 0x20;
	INT16U APD_Start = 0x500; 
	INT16U APD_End   = 0x190;
	INT16U APD_Step_Min = 0x10;
	
	float RxPow_mon = 0;
	float RX_Power_Min = 0;  
	float RX_Power_Max = 0;    

	error = SET_ATT_ONU(channel, -30+sRxCal.power[channel]);   
	if (error<0)
	{
		goto Error;
	}
	
	//�ֶ�ģʽ
	strcpy (data->APD_Info, "�ֶ�ģʽ"); 
	error = DWDM_ONU_DS4830_Set_APD_Manual (INST_EVB[channel]);	  //ֻд���ұ������л����ֶ�ģʽ
	if (error) 
	{
		strcat (data->APD_Info, "ʧ��"); 
		goto Error;
	}
	
	error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], 0xFFF);
	if (error)
	{
		strcat (data->APD_Info, "ʧ��"); 
	//	MessagePopup ("ERROR", "����APD DAC����"); 
		goto Error;      
	}
		
	if(!my_struConfig.Temper_Room)
	{
		RX_Power_Min=-30.-1.0;
		RX_Power_Max=-30.+1.0;
	}
	else
	{
		RX_Power_Min=-30.-0.5;
		RX_Power_Max=-30.+0.5;
	}

	DAC30 = APD_Start; 
	error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], DAC30);
	if (error)
	{
			strcat (data->APD_Info, "ʧ��"); 
	//		MessagePopup ("ERROR", "����APD DAC����"); 
			goto Error;      
	}   
	Delay (1.);
	
	error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
	if (error)
	{
		return -1; 
	}
	do
	{
		if(RxPow_mon>RX_Power_Min && RxPow_mon<RX_Power_Max)
		{
			break;
		}
		else
		{
			if(RxPow_mon>RX_Power_Max)
			{
				DAC30 = DAC30 + APD_Step/2;
			}
			else
			{
				DAC30 = DAC30 - APD_Step;
			}
		}
			 
		
		sprintf (tmp, "\nAPD=0x%02X", DAC30);
		strcat (data->APD_Info, tmp); 
		error = DWDM_ONU_DS4830_Set_APD (INST_EVB[channel], DAC30);
		if (error)
		{
			strcat (data->APD_Info, "ʧ��"); 
		//	MessagePopup ("ERROR", "����APD DAC����"); 
			goto Error;      
		}
			
		Delay (1.);
		error = MyDLL_8472_GetRxPWR (INST_EVB[channel], &RxPow_mon);
		if (error)
		{
			return -1; 
		}
		
		
	}while (DAC30 > APD_End);     

	sprintf (tmp, "\nVbr DAC=0x%02X", DAC30);
	strcat (data->APD_Info, tmp); 
	if(DAC30 <= APD_End)
	{
	//	MessagePopup ("ERROR", "����APD����Сֵ,Rx������>0.5dBm"); 
		goto Error;  
	}
	
	error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &temper);
	if (error)
	{
		goto Error;        
	}
	
	data->Vapd_DAC=(float)DAC30;
	data->Vapd_D_Temper=temper;
	
	error = DWDM_ONU_Save_APD_Luk(channel, DAC30, temper);
	if (error)
	{
		goto Error;        
	}
	
	return 0;       
	
Error:
	
	//�Զ�ģʽ
	error = DWDM_ONU_DS4830_Set_APD_Auto (INST_EVB[channel]);			  //������ʼ�����л����ֶ�ģʽ������Ҳ���ûظ����Զ�ģʽ
	if (error) 
	{
		return -1;
	}

	return -1; 
}   



int DWDM_ONU_Save_APD_Luk(int channel, unsigned short DAC_APD,double temper)                             //����APD���ұ� 
{
	float           f32Offset=0.0;
	int	            Fit_Temper_arr[166]={0};
	int    		    Temper_arr[5]={0}; 
	int             Status=0;
	unsigned short  DAC_arr[5]={0}; 
	unsigned short  Fit_LUT_arr[166]={0};
	unsigned short  Fit_LUT_arr_r[192]={0}; 
	double          t_read=0;
	double          t_check=0;
	int             count=0;
	
	double          Core_Temper_All[3]={0.0};  	
	unsigned short  DAC[6]={0}; 
	double          Offset[2]={0.0};
	double 			Slope, offset;
	INT16U 			wSlope, wOffset;
	
	double			Slope_arr[3];
	double			Offset_arr[3];
	int 			index;
	double			dTemper_arr[4];
	
	if(my_struConfig.Temper_Room)
	{
		if (temper<15||temper>95) 
		{
		//	MessagePopup ("��ʾ", "�¶�<15����>95���������Ȳ��ұ��쳣"); 
			return -1;
		} 

		dTemper_arr[0] = -28 + 50;
		dTemper_arr[1] =  42 + 50;
		dTemper_arr[2] =  98 + 50;	
			
		temper = temper+50;
			
		Slope_arr[0] = my_struDBConfig.APD_Kl;
		Slope_arr[1] = my_struDBConfig.APD_Kh;    
														  
		if(temper<dTemper_arr[1])
		{
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - temper) + DAC_APD;
			DAC[1] =  Slope_arr[0]*(dTemper_arr[1] - dTemper_arr[0]) + DAC[0];
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - dTemper_arr[1]) + DAC[1];  
		}
		else if(temper<dTemper_arr[2])
		{
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - temper) + DAC_APD;
			DAC[1] =  Slope_arr[1]*(dTemper_arr[1] - dTemper_arr[2]) + DAC[2];
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - dTemper_arr[1]) + DAC[1]; 
		}
				
		Slope_arr[0]=(DAC[1]-DAC[0])/(dTemper_arr[1]-dTemper_arr[0]);
		Offset_arr[0]=(DAC[0]-Slope_arr[0]*(dTemper_arr[0]));
		
		Slope_arr[1]=(DAC[2]-DAC[1])/(dTemper_arr[2]-dTemper_arr[1]);
		Offset_arr[1]=(DAC[1]-Slope_arr[1]*(dTemper_arr[1]));	

		Slope_arr[2]=0;
		Offset_arr[2]=DAC[2];
				
		dTemper_arr[3] = 255;
				
		for (index=0; index<3; index++)
		{
			wSlope = Slope_arr[index] * 512;
			wOffset = Offset_arr[index];
			Temper_arr[index+1] = dTemper_arr[index+1];
			if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], index*6, Temper_arr[index+1], wSlope, wOffset)<0)
			{
				return -1; 
			}
		}  
	}
	else if(my_struConfig.Temper_High) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
		
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_APD_Luk(INST_EVB[channel], 6, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
		
		Temper_arr[1] =  42 + 50;
		Temper_arr[2] =  98 + 50;

		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;

		temper = temper + 50; 
		DAC_arr[2] = DAC_APD;

		//����slope��offset   д���ұ�
		Slope = (DAC_arr[2] - DAC_arr[1]) * 512 / (temper - Temper_arr[1]); 
	    offset = (DAC_arr[2] - Slope * (temper) / 512) * 1;	
		
		DAC_arr[3] = Temper_arr[2] * (Slope / 512) + offset;  
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 6, Temper_arr[2], wSlope, wOffset)<0)
		{
			return -1;
		}
				
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 12, 0xFF, 0x00, DAC_arr[3])<0)
		{
			return -1; 	
		}
	}
	else if(my_struConfig.Temper_Low) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_APD_Luk(INST_EVB[channel], 0, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
				
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
	
		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;
	
		Temper_arr[0] = temper + 50; 
		DAC_arr[0] = DAC_APD;
			
		//����slope��offset   д���ұ�
		Slope = (DAC_arr[1] - DAC_arr[0]) * 512 / (Temper_arr[1] - Temper_arr[0]); 
	    offset = (DAC_arr[1] - Slope * (Temper_arr[1]) / 512) * 1;	
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 0, Temper_arr[1], wSlope, wOffset)<0)
		{
			return -1;
		}
	}
	
	if(DWDM_ONU_DS4830_Set_APD_Auto(INST_EVB[channel])<0)
	{
		return -1; 
	}
			
	if(DWDM_ONU_DS4830_Update_LUK0(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
	
	return 0;
}

int DWDM_ONU_Save_LOS_Luk(int channel, unsigned short DAC_LOS, double temper)                             //����LOS���ұ� 
{
	float           f32Offset=0.0;
	int	            Fit_Temper_arr[166]={0};
	int    		    Temper_arr[5]={0}; 
	int             Status=0;
	unsigned short  DAC_arr[5]={0}; 
	unsigned short  Fit_LUT_arr[166]={0};
	unsigned short  Fit_LUT_arr_r[192]={0}; 
	double          t_read=0;
	double          t_check=0;
	int             count=0;
	
	double          Core_Temper_All[3]={0.0};  	
	unsigned short  DAC[6]={0}; 
	double          Offset[2]={0.0};
	double 			Slope, offset;
	INT16U 			wSlope, wOffset;
	
	double			Slope_arr[3];
	double			Offset_arr[3];
	int 			index;
	double			dTemper_arr[4];
	
	if(my_struConfig.Temper_Room)
	{
		if (temper<15||temper>95) 
		{
			MessagePopup ("��ʾ", "�¶�<15����>95�����LOS���ұ��쳣"); 
			return -1;
		} 

		dTemper_arr[0] = -28 + 50;
		dTemper_arr[1] =  42 + 50;
		dTemper_arr[2] =  98 + 50;	
			
		temper = temper+50;
			
		Slope_arr[0] = my_struDBConfig.LOS_Kl;
		Slope_arr[1] = my_struDBConfig.LOS_Kh;    
														  
		if(temper<dTemper_arr[1])
		{
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - temper) + DAC_LOS;
			DAC[1] =  Slope_arr[0]*(dTemper_arr[1] - dTemper_arr[0]) + DAC[0];
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - dTemper_arr[1]) + DAC[1];  
			if(DAC[0]<0) DAC[0]=0;
			if(DAC[1]<0) DAC[1]=0;   
			if(DAC[2]<0) DAC[2]=0;   
		}
		else if(temper<dTemper_arr[2])
		{
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - temper) + DAC_LOS;
			DAC[1] =  Slope_arr[1]*(dTemper_arr[1] - dTemper_arr[2]) + DAC[2];
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - dTemper_arr[1]) + DAC[1]; 
			if(DAC[0]<0) DAC[0]=0;
			if(DAC[1]<0) DAC[1]=0;   
			if(DAC[2]<0) DAC[2]=0; 
		}
				
		Slope_arr[0]=(DAC[1]-DAC[0])/(dTemper_arr[1]-dTemper_arr[0]);
		Offset_arr[0]=(DAC[0]-Slope_arr[0]*(dTemper_arr[0]));
		
		Slope_arr[1]=(DAC[2]-DAC[1])/(dTemper_arr[2]-dTemper_arr[1]);
		Offset_arr[1]=(DAC[1]-Slope_arr[1]*(dTemper_arr[1]));	

		Slope_arr[2]=0;
		Offset_arr[2]=DAC[2];
				
		dTemper_arr[3] = 255;
				
		for (index=0; index<3; index++)
		{
			wSlope = Slope_arr[index] * 512;
			wOffset = Offset_arr[index];
			Temper_arr[index+1] = dTemper_arr[index+1];
			if (DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], index*6, Temper_arr[index+1], wSlope, wOffset)<0)
				
			{
				return -1; 
			}
		}  
	}
	else if(my_struConfig.Temper_High) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
	
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_LOS_Luk(INST_EVB[channel], 0, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
	
		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;
		
		Temper_arr[2] = temper + 50; 
		DAC_arr[2] = DAC_LOS;
		
		//����slope��offset   д���ұ�
		Slope = (DAC_arr[2] - DAC_arr[1]) * 512 / (Temper_arr[2] - Temper_arr[1]); 
	    offset = (DAC_arr[2] - Slope * (Temper_arr[2]) / 512) * 1;	
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], 6, Temper_arr[2], wSlope, wOffset)<0)
		{
			return -1;
		}
				
		if (DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], 12, 0xFF, 0x00, DAC_LOS)<0)
		{
			return -1; 	
		}
	}
	else if(my_struConfig.Temper_Low) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_LOS_Luk(INST_EVB[channel], 0, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
				
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
	
		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;
	
		Temper_arr[0] = temper + 50; 
		DAC_arr[0] = DAC_LOS;
			
		//����slope��offset   д���ұ�
		Slope = (DAC_arr[1] - DAC_arr[0]) * 512 / (Temper_arr[1] - Temper_arr[0]); 
	    offset = (DAC_arr[1] - Slope * (Temper_arr[1]) / 512) * 1;	
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_LOS_Luk(INST_EVB[channel], 0, Temper_arr[1], wSlope, wOffset)<0)
		{
			return -1;
		}
	}
	
	if(DWDM_ONU_DS4830_Set_LOS_Auto(INST_EVB[channel])<0)
	{
		return -1; 
	}
			
	if(DWDM_ONU_DS4830_Update_LUK10(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
	
	return 0;
}
int DWDM_ONU_Save_Bias_Luk(int channel, int channelindex ,unsigned short DAC_BIAS, double temper)                             //����Bias���ұ� 
{
	float           f32Offset=0.0;
	int	            Fit_Temper_arr[166]={0};
	int    		    Temper_arr[5]={0}; 
	int             Status=0;
	unsigned short  DAC_arr[5]={0}; 
	unsigned short  Fit_LUT_arr[166]={0};
	unsigned short  Fit_LUT_arr_r[192]={0}; 
	double          t_read=0;
	double          t_check=0;
	int             count=0;
	
	double          Core_Temper_All[3]={0.0};  	
	unsigned short  DAC[6]={0}; 
	double          Offset[2]={0.0};
	double 			Slope, offset;
	INT16U 			wSlope, wOffset;
	
	double			Slope_arr[3];
	double			Offset_arr[3];
	int 			index;
	double			dTemper_arr[4];
	
	int 			Luk_Addr[4];
	
	Luk_Addr[0]=1;
	Luk_Addr[1]=7;  
	Luk_Addr[2]=8;  
	Luk_Addr[3]=9;  
	
	if(my_struConfig.Temper_Room)
	{
		if (temper<15||temper>95) 
		{
		//	MessagePopup ("��ʾ", "�¶�<15����>95���������Ȳ��ұ��쳣"); 
			return -1;
		} 

		dTemper_arr[0] = -28 + 50;
		dTemper_arr[1] =  42 + 50;
		dTemper_arr[2] =  98 + 50;	
			
		temper = temper+50;
			
		Slope_arr[0] = 0;
		Slope_arr[1] = 0;    
														  
		if(temper<dTemper_arr[1])
		{
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - temper) + DAC_BIAS;
			DAC[1] =  Slope_arr[0]*(dTemper_arr[1] - dTemper_arr[0]) + DAC[0];
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - dTemper_arr[1]) + DAC[1];  
		}
		else if(temper<dTemper_arr[2])
		{
			DAC[2] =  Slope_arr[1]*(dTemper_arr[2] - temper) + DAC_BIAS;
			DAC[1] =  Slope_arr[1]*(dTemper_arr[1] - dTemper_arr[2]) + DAC[2];
			DAC[0] =  Slope_arr[0]*(dTemper_arr[0] - dTemper_arr[1]) + DAC[1]; 
		}
				
		Slope_arr[0]=(DAC[1]-DAC[0])/(dTemper_arr[1]-dTemper_arr[0]);
		Offset_arr[0]=(DAC[0]-Slope_arr[0]*(dTemper_arr[0]));
		
		Slope_arr[1]=(DAC[2]-DAC[1])/(dTemper_arr[2]-dTemper_arr[1]);
		Offset_arr[1]=(DAC[1]-Slope_arr[1]*(dTemper_arr[1]));	

		Slope_arr[2]=0;
		Offset_arr[2]=DAC[2];
				
		dTemper_arr[3] = 255;
				
		for (index=0; index<3; index++)
		{
			wSlope = Slope_arr[index] * 1024;
			wOffset = Offset_arr[index];
			Temper_arr[index+1] = dTemper_arr[index+1];
			if (DWDM_ONU_DS4830_Write_BIAS_Luk(INST_EVB[channel], Luk_Addr[channelindex],index*6,Temper_arr[index+1], wSlope, wOffset)<0)
			{   							  
				return -1; 
			}
		}  
	}
	else if(my_struConfig.Temper_High) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
	
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_APD_Luk(INST_EVB[channel], 0, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
	
		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;
		
		Temper_arr[2] = temper + 50; 
		DAC_arr[2] = DAC_BIAS;
		
		//����slope��offset   д���ұ�
		Slope = (DAC_arr[2] - DAC_arr[1]) * 512 / (Temper_arr[2] - Temper_arr[1]); 
	    offset = (DAC_arr[2] - Slope * (Temper_arr[2]) / 512) * 1;	
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 6, Temper_arr[2], wSlope, wOffset)<0)
		{
			return -1;
		}
				
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 12, 0xFF, 0x00, DAC_BIAS)<0)
		{
			return -1; 	
		}
	}
	else if(my_struConfig.Temper_Low) 
	{
		for (count=0; count<5; count++)
		{
			DAC_arr[count] = 0;
			Temper_arr[count] = 0;
		}
		//�����һ�ε�DACֵ
		DWDM_ONU_DS4830_Read_APD_Luk(INST_EVB[channel], 0, &Temper_arr[1], &wSlope, &wOffset);
		if (wSlope > 0x7FFF)
		{
			Slope = wSlope - 65536;
		}
		else
		{
			Slope = wSlope;
		}
				
		if (wOffset > 0x7FFF)
		{
			offset = wOffset - 65536;
		}
		else
		{
			offset = wOffset;
		}
	
		DAC_arr[1] = Temper_arr[1] * (Slope / 512) + offset;
	
		Temper_arr[0] = temper + 50; 
		DAC_arr[0] = DAC_BIAS;
			
		//����slope��offset   д���ұ�
		Slope = (DAC_arr[1] - DAC_arr[0]) * 512 / (Temper_arr[1] - Temper_arr[0]); 
	    offset = (DAC_arr[1] - Slope * (Temper_arr[1]) / 512) * 1;	
	
		wSlope = Slope;
		wOffset = offset;
	
		if (DWDM_ONU_DS4830_Write_APD_Luk(INST_EVB[channel], 0, Temper_arr[1], wSlope, wOffset)<0)
		{
			return -1;
		}
	}
	
//	if(DWDM_ONU_DS4830_Set_BIAS_Auto(INST_EVB[channel])<0)
//	{
//		return -1; 
//	}
			
	if(DWDM_ONU_DS4830_Update_LUK1(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK7(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK8(INST_EVB[channel])<0)
	{
		return -1; 
	}
	if(DWDM_ONU_DS4830_Update_LUK9(INST_EVB[channel])<0)
	{
		return -1; 
	}
	
	if(DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel])<0)
	{
		return -1;  
	}
	
	return 0;
}




BOOL DB_GET_Firmware_Ver(void)
{
	int  num;
	char buf[256];
	char myFirmwareVer[50];
	char mySFRM_Number[50];
	char myFEP_Number[50];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	

	num=0;
	memset (my_struDBConfig.FirmwareVer, 0, 50);
	memset (myFirmwareVer, 0, 50); 
	
	memset (my_struDBConfig.SFRM_Number, 0, 50);
	memset (mySFRM_Number, 0, 50); 
	
	memset (my_struDBConfig.FEP_Number, 0, 50);
	memset (myFEP_Number, 0, 50); 
	
	Fmt (buf, "SELECT Firmware_Ver, SFRM_Number, FEP_Number FROM AutoDT_Spec_Image where PartNumber='%s' and Version='%s'", my_struConfig.PN, my_struConfig.BOM);
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return FALSE;}  
    
    resCode = DBBindColChar (hstmt, 1, 50, myFirmwareVer, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	resCode = DBBindColChar (hstmt, 2, 50, mySFRM_Number, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 3, 50, myFEP_Number, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
    
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return FALSE;}
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return FALSE;}
	
	if (num!=1) {MessagePopup("Error","��ȡ���ݿ�AutoDT_Spec_Image���Firmware Versionʧ�ܣ�"); return FALSE;} 
	
	strcpy (my_struDBConfig.FirmwareVer, myFirmwareVer);
	strcpy (my_struDBConfig.SFRM_Number, mySFRM_Number);  
	strcpy (my_struDBConfig.FEP_Number,  myFEP_Number);
	
	return TRUE;
}

int tuningTxOff_Depth(int channel, struct struTestData *data)
{
	int count, DAC,DAC0;
	int DACmin, DACmax;
	int error; 
	int index,Table;
	
	double Slope,Offset;
	
	int DACArray[30];
	float AOPArray[30];
	
	double	temp_AOP,AOP0, temp_AOPmin, temp_AOPmax, temp_AOPavg; 
	BYTE LUT_APC[81], biasdac_m, biasdac_l;

	double temper;  
	unsigned short DAC_Arr[3];
	double Temper_Arr[3];
	int 	ROOM_DAC[4];
	double 	ROOM_Temper[4];
	
	BOOL	Tun2=FALSE;

	double TxOffPower_max,TxOffPower_min;

	//Set TxDisable
	errChk(EVB5_SET_BEN_Level (INST_EVB[channel], 1));
	SET_ATT_ONU (channel, -60);  
	//Set BIAS  ģʽ
	error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
	if(error)
	{
		MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
	  	return -1;
	}
//	Delay(1);    
	
	data->PointCount=0;  	
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{	  
		if(0==index)
		{
			DACmax=0x790;
			DACmin=0x5C0;
			DAC=(DACmax+DACmin)/2;
			DAC0=DACmax; 
		}
		else
		{
			DACmax=data->TxOffDepthDAC[index-1]+0x20;
			DACmin=data->TxOffDepthDAC[index-1];
			DAC0=DACmax;  
			DAC=data->TxOffDepthDAC[index-1];  
		}
		//Set PID 
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],data->TecDAC[index])<0)
		{
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],data->TecDAC[index])<0)
			{
				return -1;
			}
		}
		//Set APC &Mod
		if (!my_struConfig.Temper_Room)
		{
			errChk(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index]));	
			errChk(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index]));
			
			errChk(SET_DAC_APC (channel, data->ucRb[index] ));	
			errChk(SET_DAC_MOD(channel, data->ucRm[index])); 
		}
		else
		{
			errChk(SET_DAC_APC (channel, data->ucRb[index] ));	
			errChk(SET_DAC_MOD(channel, data->ucRm[index]));  
			//Set Mod 0x2A
		//	errChk(SET_DAC_MOD(channel, 0x20));  
		}

		//���Է�Χ���õ��Թ��������ֵ��
		if(DB_Get_TunTxOffPower_Spec(&temp_AOPmin,&temp_AOPmax)<0)
		{
			return -1;
		}
		temp_AOPavg=(temp_AOPmin+temp_AOPmax)/2.0;
		temp_AOPmin=temp_AOPavg-0.3;	  
		temp_AOPmax=temp_AOPavg+0.3;

		//Set DAC0
		errChk(DWDM_ONU_DS4830_Set_BIAS (INST_EVB[channel], DAC0));  
		
		Delay(1.5);
		data->PointCount++;
		errChk (Read_AOP_Ex(channel, &AOP0));

		memset(DACArray,0,sizeof(DACArray));
		memset(AOPArray,0,sizeof(AOPArray));   
		count=0;
		do
		{
			errChk(DWDM_ONU_DS4830_Set_BIAS (INST_EVB[channel], DAC));  
			data->PointCount++;
			Delay(1.5); 
			
			errChk (Read_AOP_Ex(channel, &temp_AOP)); 
			  
		
			if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
			{
				break;
			} 
			else
			{	
				Slope  = (temp_AOP-AOP0)/(DAC-DAC0);
				Offset = temp_AOP-Slope* DAC;
				
				DAC0=DAC; 
				AOP0=temp_AOP;
				DAC= (temp_AOPavg - Offset)/Slope;
				if(DAC<=0) 		DAC=0;
				if(DAC>0x0FFF)  DAC=0x0FFF;
			}
			count++;
		}while (count<10);
	
		if (count >= 10) 
		{
		//	MessagePopup ("Error", "�ض���ȵ��Գ�ʱ!");
			Tun2=TRUE;  
		} 
		
		if(Tun2)
		{
			DACmax=0xA00;
			DACmin=0x5C0;
			DAC=(DACmax+DACmin)/2;
			DAC0=DACmax; 

			count=0;
			do
			{
				errChk(DWDM_ONU_DS4830_Set_BIAS (INST_EVB[channel], DAC));
		
				if(index<1) 
				{
					Delay(1.5);
				}
				else
				{
					Delay(1); 
				}
		
				errChk (Read_AOP_Ex(channel, &temp_AOP)); 
		
				if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
				{
					break;
				}
				else if((DACmax-DACmin)<2) 
				{
				//	MessagePopup("Error", "���Թض����ʧ�ܣ�");
					data->ErrorCode=ERR_TUN_AOP;
					error = -1;
					goto Error;
				} 
				else
				{
					if(temp_AOP>temp_AOPavg) 
					{
						DACmax=DAC;
						DAC=(DACmax+DACmin)/2;
					}
					else					 
					{
						DACmin=DAC;
						DAC=(DACmax+DACmin)/2;
					}
				}
				count++;
			}while (count<20);
	
			if (count >= 20) 
			{
			//	MessagePopup ("Error", "�ض���ȵ��Գ�ʱ!");
				error = -1; 
				goto Error;
			} 
		}
		
		//��¼DAC&Temperatrue
		data->TxOffDepthDAC[index]=DAC;  
		data->TxOffPower[index]=temp_AOP;
		error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &data->TxOffDepthTemp[index]);
		if (error)
		{
			goto Error;        
		}
		
		//�����¶�ѡ�����ж��Ƿ�Щ���ұ�
		if (my_struConfig.Temper_Room)
		{
			;//���µ��Բ������ұ�����¼TEC DAC�͵�ǰ�ں��¶�
			DAC_Arr[0]=data->TxOffDepthDAC[index];
			DAC_Arr[1]=data->TxOffDepthDAC[index]; 
			DAC_Arr[2]=data->TxOffDepthDAC[index]; 	
			
			Temper_Arr[0] = -40.;   
			Temper_Arr[1] = data->TxOffDepthTemp[index];   
			Temper_Arr[2] =0xFF;
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk_EX (INST_EVB[channel], index, DAC_Arr, Temper_Arr)); 					//����дK=0;
			        
		}
		else if(my_struConfig.Temper_High)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
			errChk (DB_Get_Room_TxOffDepth(data->OpticsSN, ROOM_DAC, ROOM_Temper));
			
			DAC_Arr[0]=ROOM_DAC[index];
			DAC_Arr[1]=data->TxOffDepthDAC[index]; 
			
			Temper_Arr[0] = ROOM_Temper[index]; 
			Temper_Arr[1] = data->TxOffDepthTemp[index];
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk (INST_EVB[channel], index, TRUE, DAC_Arr, Temper_Arr)); 
		}
		else if(my_struConfig.Temper_Low)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
			errChk (DB_Get_Room_TxOffDepth(data->OpticsSN, ROOM_DAC, ROOM_Temper));   
			
			DAC_Arr[0]=data->TxOffDepthDAC[index];
			DAC_Arr[1]=ROOM_DAC[index];     
			Temper_Arr[0] = data->TxOffDepthTemp[index];  
			Temper_Arr[1] = ROOM_Temper[index];
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk (INST_EVB[channel], index, FALSE, DAC_Arr, Temper_Arr)); 
		}
		else
		{
			error = -1;
			goto Error;
		}
	}
Error:
//	if(!my_struConfig.Temper_Room)
	{
		//Set Bias Auto Mode
		DWDM_ONU_DS4830_Set_BIAS_Auto(INST_EVB[channel]);

		//����TEC Auto��ģʽ 
		DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]);
		
		DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel]);

	}
	//Set TxEnable
	EVB5_SET_BEN_Level (INST_EVB[channel], 0);
	return error;
}

int tuningTxOff_Depth_Ex(int channel, struct struTestData *data)	  // �ض���ȹ�������ָ��-3dBm;
{
	int count, DAC,DAC0, DAC1,DAC2,DAC3, DACmin, DACmax, count_error, error; 
	double	temp_AOP, temp_AOPmin, temp_AOPmax, temp_AOPavg; 
	BYTE LUT_APC[81], biasdac_m, biasdac_l;
	int index,Table;
	double temper;  
	unsigned short DAC_Arr[3];
	double Temper_Arr[3];
	int 	ROOM_DAC[4];
	double 	ROOM_Temper[4];
	
	double TxOffPower_max,TxOffPower_min;

	//Set TxDisable
	errChk(EVB5_SET_BEN_Level (INST_EVB[channel], 1));
	Delay(1);
		
	SET_ATT_ONU (channel, -60);  
	
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		if(0==index)
		{
			DACmax=0x850;
			DACmin=0x400;
			DAC0=0x650;
		}
		else
		{
			DACmax=data->TxOffDepthDAC[index-1]+0x20;
			DACmin=data->TxOffDepthDAC[index-1];
			DAC0=data->TxOffDepthDAC[index-1];
		}
		
		//Set PID 
		if(data->TecDAC[index]<0x600)  
		{
			data->TecDAC[channel]=0x600;
		}

		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],data->TecDAC[index])<0)
		{
			error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
			if (error) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(INST_EVB[channel],data->TecDAC[index])<0)
			{
				return -1;
			}
		}
		
		//Set APC &Mod
		if (!my_struConfig.Temper_Room)
		{
			errChk(DWDM_ONU_DS4830_Read_APC_LUK6 (INST_EVB[channel],index, &data->ucRb[index]));	
			errChk(DWDM_ONU_DS4830_Read_Mod_LUK6 (INST_EVB[channel],index, &data->ucRm[index]));
			
			errChk(SET_DAC_APC (channel, data->ucRb[index] ));	
			errChk(SET_DAC_MOD(channel, data->ucRm[index])); 
		}
		else
		{
			//Set Mod 0x2A
			errChk(SET_DAC_MOD(channel, 0x20));  
		}

		//Set BIAS
		error=DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
		error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		if(error)
		{
		//	MessagePopup ("Error", "����BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
		
		//���Է�Χ���õ��Թ��������ֵ��
		if(DB_Get_TunTxOffPower_Spec(&temp_AOPmin,&temp_AOPmax)<0)
		{
			return -1;
		}
		temp_AOPavg=((temp_AOPmin+temp_AOPmax)/2.0 -3.0);				 
		temp_AOPmin=temp_AOPavg-0.3;	  
		temp_AOPmax=temp_AOPavg+0.3;
		
		count=0;
		do
		{
			errChk(DWDM_ONU_DS4830_Set_BIAS (INST_EVB[channel], DAC0));
		
			if(index<1) 
			{
				Delay(1.5);
			}
			else
			{
				Delay(1); 
			}
		
			errChk (Read_AOP_Ex(channel, &temp_AOP)); 
		
			if (temp_AOP>=temp_AOPmin && temp_AOP<=temp_AOPmax) 
			{
				break;
			}
			else if((DACmax-DACmin)<2) 
			{
			//	MessagePopup("Error", "���Թض����ʧ�ܣ�");
				data->ErrorCode=ERR_TUN_AOP;
				error = -1;
				goto Error;
			} 
			else
			{
				if(temp_AOP>temp_AOPavg) 
				{
					DACmax=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
				else					 
				{
					DACmin=DAC0;
					DAC0=(DACmax+DACmin)/2;
				}
			}
			count++;
		}while (count<20);
	
		if (count >= 20) 
		{
		//	MessagePopup ("Error", "�ض���ȵ��Գ�ʱ!");
			error = -1; 
			goto Error;
		} 
		
		//��¼DAC&Temperatrue
		data->TxOffDepthDAC[index]=DAC0;  
		data->TxOffPower[index]=temp_AOP;
		error = DWDM_ONU_DS4830_Get_CoreTemper_Ex (INST_EVB[channel], &data->TxOffDepthTemp[index]);
		if (error)
		{
				goto Error;        
		}
		
		//�����¶�ѡ�����ж��Ƿ�Щ���ұ�
		if (my_struConfig.Temper_Room)
		{
			;//���µ��Բ������ұ�����¼TEC DAC�͵�ǰ�ں��¶�
			DAC_Arr[0]=data->TxOffDepthDAC[index];
			Temper_Arr[0] = -40.;
			
			DAC_Arr[1]=data->TxOffDepthDAC[index];
			Temper_Arr[1] = data->TxOffDepthTemp[index];
			
			DAC_Arr[2]=data->TxOffDepthDAC[index];
			Temper_Arr[2] =0xFF;
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk_EX (INST_EVB[channel], index, DAC_Arr, Temper_Arr)); 					//����дK=0;
			        
		}
		else if(my_struConfig.Temper_High)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�
			errChk (DB_Get_Room_TxOffDepth(data->OpticsSN, ROOM_DAC, ROOM_Temper));
			
			DAC_Arr[0]=ROOM_DAC[index];
			Temper_Arr[0] = ROOM_Temper[index];
			
			DAC_Arr[1]=data->TxOffDepthDAC[index];
			Temper_Arr[1] = data->TxOffDepthTemp[index];
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk (INST_EVB[channel], index, TRUE, DAC_Arr, Temper_Arr)); 
		}
		else if(my_struConfig.Temper_Low)
		{
			//���µ��ԣ���ȡ����TEC DAC�͵�ʱ�ں��¶ȣ�д���µ����µĲ��ұ�  
			errChk (DB_Get_Room_TxOffDepth(data->OpticsSN, ROOM_DAC, ROOM_Temper));   
			
			DAC_Arr[0]=data->TxOffDepthDAC[index];
			Temper_Arr[0] = data->TxOffDepthTemp[index];
			
			DAC_Arr[1]=ROOM_DAC[index];
			Temper_Arr[1] = ROOM_Temper[index];
			
			errChk (DWDM_ONU_DS4830_Write_TxOffDepth_Luk (INST_EVB[channel], index, FALSE, DAC_Arr, Temper_Arr)); 
		}
		else
		{
			error = -1;
			goto Error;
		}
/*		
		error = DWDM_ONU_Save_Bias_Luk(channel,index, DAC0, data->TxOffDepthTemp[index]);
		if (error)
		{
			goto Error;        
		}
*/		
		
		error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		if(error)
		{
		//	MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
		
	}
Error:
	if(!my_struConfig.Temper_Room)
	{
		//Set Bias Auto Mode
		DWDM_ONU_DS4830_Set_BIAS_Auto(INST_EVB[channel]);

		//����TEC Auto��ģʽ 
		DWDM_ONU_DS4830_Set_TEC_Auto(INST_EVB[channel]);
		
		DWDM_ONU_DS4830_Update_Base0(INST_EVB[channel]);

	}
	//Set TxEnable
	EVB5_SET_BEN_Level (INST_EVB[channel], 0);
	return error;
}

int testTxOff_Depth(int channel, struct struTestData *data)
{
	int count, error;
	float TxPow_mon;
	int Table,index;
	
//	double TxOffPower_max,TxOffPower_min;
//	double TxOffPower_max0=-30.6;
//	double TxOffPower_min0=-32.5; 
//	double TxOffPower_max1=-30.6;
//	double TxOffPower_min1=-32.5;
	
//	double TxOffPower_max0=-35;
//	double TxOffPower_min0=-40; 
//	double TxOffPower_max1=-35;
//	double TxOffPower_min1=-40;
	
	SET_ATT_ONU(channel, -60);
	//TxDis
	if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_OFF)<0) 
	{
		return -1; 
	}
	
/*	error = EVB5_SET_VTR (INST_EVB[channel], 3.5);
	if (error) 
	{
		goto Error;
	}
*/	
	Delay(1.0);
	for (index = 0; index<WAVELENGTHCHANNEL; index++)
	{
		//���ò���ͨ��
		errChk(DWDM_ONU_Select_Channelindex(channel,index));
		//�жϹ⹦�� 
		count=0;
		do
		{
			if (Read_AOP_Ex(channel, &data->TxOffPower[index])<0) 
			{
				return FALSE; 
			}
		
			if (data->TxOffPower[index]>my_struDBConfig.TxOffPowerMax || data->TxOffPower[index]<my_struDBConfig.TxOffPowerMin)
			{
				Delay (0.5);
				count++;
			}
			else break;
		
		} while (count<5);

		if (data->TxOffPower[index]>my_struDBConfig.TxOffPowerMax || data->TxOffPower[index]<my_struDBConfig.TxOffPowerMin) 
		{
			return -1;
		}
	}

	//TxEn  for AOP test
	if (EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0) 
	{
		return -1; 
	}

	
	SET_ATT_ONU(channel, -10);
	
	//Set TxDisable
	if(EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON)<0)
	{
		return -1;
	}
	
	return 0;

Error:
/*	error = EVB5_SET_VTR (INST_EVB[channel], 3.3);
	if (error) 
	{
		MessagePopup ("Error", "����EVB5 VTR��3.3Vʧ��");
		return -1;
	}
*/	
	//TxEn  for AOP test
	EVB5_SET_BEN_Level(INST_EVB[channel], my_struEVB5.BURST_ON);

	SET_ATT_ONU(channel, -10); 
		
	return -1;
}

int DWDM_ONU_Read_LaserTemperatrue(int channel, struct struTestData *data)
{
	int error,index;

	for(index=0;index<WAVELENGTHCHANNEL;index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0)
		{
			return -1; 
		}  
		Delay(3);
		
		//Read LD Tempe
		error=ux3328s_get_laser_tempe (INST_EVB[channel], &data->LDTemper[index]);
		if (error) 
		{
		//	MessagePopup ("Error", "��ȡLaser�¶�ʧ��");
			return -1;
		}
	}
	return 0;
	
}
int DWDM_ONU_Read_Tec_Current(int channel, struct struTestData *data)
{
	int error,index;

	for(index=0;index<WAVELENGTHCHANNEL;index++)
	{
		//���ò���ͨ��
		if(DWDM_ONU_Select_Channelindex(channel,index)<0)
		{
			return -1; 
		}  
		Delay(3);
		error=ux3328s_get_Tec_Current (INST_EVB[channel], &data->TecCurrent[index]);
		if (error) 
		{
		//	MessagePopup ("Error", "��ȡLaser�¶�ʧ��");
			return -1;
		}
	}
	return 0;
	
}
int DB_Save_Results_Monitor (struct struTestData data)
{
	char buf[5000];
	int  count;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 

	if (SERVER_ORACLE == my_struConfig.servertype)
	{
		sprintf (buf, "begin sp_add_results_monitor ('%s','%s',to_date('%s','yyyy-mm-dd hh24:mi:ss'),'%s','%s',%f,%d,'%s','%s',%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f ); end ;", 
		       	data.PN, data.Ver, data.TestDate, data.ModuleSN, data.OpticsSN,data.AmbientTemperatrue,data.ErrorCode,data.Status,"RxCalibration",data.RxPow_num, 
			   	data.RxPow_mon[0],data.RxPow_mon[1],data.RxPow_mon[2],data.RxPow_mon[3], data.RxPow_mon[4],data.RxPow_mon[5],data.RxPow_mon[6],data.RxPow_mon[7],data.RxPow_mon[8],data.RxPow_mon[9],
				data.RxPow_mon[10],data.RxPow_mon[11],data.RxPow_mon[12],data.RxPow_mon[13],data.RxPow_mon[14], data.RxPow_mon[15],data.RxPow_mon[16],data.RxPow_mon[17],data.RxPow_mon[18],data.RxPow_mon[19],
				data.RxPow_mon[20],data.RxPow_mon[21],data.RxPow_mon[22],data.RxPow_mon[23],data.RxPow_mon[24], data.RxPow_mon[25],data.RxPow_mon[26],data.RxPow_mon[27],data.RxPow_mon[28],data.RxPow_mon[29]); 
	}
	else
	{
		sprintf (buf, "EXEC sp_add_autodt_results_monitor '%s','%s','%s','%s','%s',%f,%d,'%s','%s',%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", 
		       	data.PN, data.Ver, data.TestDate, data.ModuleSN, data.OpticsSN,data.AmbientTemperatrue,data.ErrorCode,data.Status,"RxCalibration",data.RxPow_num, 
			   	data.RxPow_mon[0],data.RxPow_mon[1],data.RxPow_mon[2],data.RxPow_mon[3], data.RxPow_mon[4],data.RxPow_mon[5],data.RxPow_mon[6],data.RxPow_mon[7],data.RxPow_mon[8],data.RxPow_mon[9],
				data.RxPow_mon[10],data.RxPow_mon[11],data.RxPow_mon[12],data.RxPow_mon[13],data.RxPow_mon[14], data.RxPow_mon[15],data.RxPow_mon[16],data.RxPow_mon[17],data.RxPow_mon[18],data.RxPow_mon[19],
				data.RxPow_mon[20],data.RxPow_mon[21],data.RxPow_mon[22],data.RxPow_mon[23],data.RxPow_mon[24], data.RxPow_mon[25],data.RxPow_mon[26],data.RxPow_mon[27],data.RxPow_mon[28],data.RxPow_mon[29]); 
	}
	
	count=0;
	do
	{
		resCode = DBImmediateSQL (hdbc, buf); 
		count++;
	} while (resCode != DB_SUCCESS && count<3);
	
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}  
	
	return 0;
}

int Get_8472_Monitor (int inst, float *Vcc,float *Temperatrue,float *Ibias,float *TxPWR,double *TECtemp, double *TECCurrent) 
{
	int   error, i;
	INT8U A2arr[256];
	float slope, offset; 
	union uA2 localA2;
	int val; 
	
	error = I2C_BYTEs_READ_DLL (inst, 0xA2, 0, 256, A2arr);
	if (error<0) {MessagePopup ("ERROR", "NO Acknowledge from target!");return -1;}

	for (i=0; i<256; i++)
	{localA2.pStrA2[i] = A2arr[255-i];}	
	
	//Vcc
	slope = localA2.sStrA2.voltageSlope/256.;
	offset= localA2.sStrA2.voltageOffset;
	*Vcc = (slope * localA2.sStrA2.vcc + offset) * (100.E-6); 

	//Temperatrue
	slope = localA2.sStrA2.tempSlope/256.;
	offset= localA2.sStrA2.tempOffset;
	*Temperatrue = (slope * localA2.sStrA2.temperature + offset) * (1/256.); 
	
	//Ibias
	slope = localA2.sStrA2.tx_ISlope/256.;
	offset= localA2.sStrA2.tx_IOffset;
	*Ibias = (slope * localA2.sStrA2.tx_Bias + offset) * (2.E-3);

	//Tx Power
	slope = localA2.sStrA2.tx_PwrSlope/256.;
	offset= localA2.sStrA2.tx_PwrOffset;

	DisableBreakOnLibraryErrors ();										 
	*TxPWR = 10.*log10((slope * localA2.sStrA2.tx_Power + offset) * (0.1E-3));
	
	//Read LD Tempe 
	*TECtemp=(localA2.sStrA2.reserved3[3]*256+localA2.sStrA2.reserved3[2])/256.;  
	
	//Tec Current        
	val=localA2.sStrA2.reserved3[1]*256+localA2.sStrA2.reserved3[0]; 
	if(val>32767)
	{
		val=val-65536;	
	}
	*TECCurrent=val *0.1;	
	
	EnableBreakOnLibraryErrors ();
	

	
	return 0;
}

int DWDM_ONU_Set_Ibias_Max(int inst )
{
	char strInpt[256];
	char strOupt[256];   
	int		error,i;
	INT8U 	ChNumSet[2];																						   
	int		ChWLRange[4];
	int		index,channel;
	INT8U	Ibias_DAC;
	int		Ibias_sum;
	INT8U	Ibias_Maxdac[4]; 
	INT8U	temp;  

	//��ȡ������Χ
	error = SetCommand (inst,"MCU_GET_TABLE(base,0,0x8A,4)", strOupt); 
	if(error!=0)
	{
		return -1;
	} 
	sscanf(strOupt, "0x%x,0x%x,0x%x,0x%x", &ChWLRange[0],&ChWLRange[1],&ChWLRange[2],&ChWLRange[3]);   
	
	error = DWDM_ONU_Ibias_Max_Disable(inst);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	channel=0;
	for(index=ChWLRange[2];index>=ChWLRange[0];index--)
	{
		//�л�ͨ��
		error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 2, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}   
	 
		error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 145, index,1);
		if (error<0)			  
		{
			return -1;	
		}
				
		Delay(0.5); 
		
		error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 3, 0.1); 
		if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}   
	
		Ibias_sum=0;
		for(i=0;i<5;i++)
		{
			error = I2C_BYTE_READ_DLL (inst, 0xA2, 0xD6, &Ibias_DAC); 
			if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
			Ibias_sum = Ibias_sum + Ibias_DAC;
			Delay(0.2);
		}
		Ibias_sum=Ibias_sum/i;
		Ibias_Maxdac[channel] = Ibias_sum +6;
		channel++;
	}
			
	//����Ibais Max
	sprintf(strInpt, "MCU_SET_TABLE(base,0,154,0x%02X,0x%02X,0x%02X,0x%02X)",Ibias_Maxdac[0],Ibias_Maxdac[1],Ibias_Maxdac[2],Ibias_Maxdac[3]); 
	error=SetCommand (inst,strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	} 
		
	error = SetCommand (inst,"mcu_update_flash(base,0)", strOupt); 
	if (error<0)		
	{
		return -1;	
	}

//	error = DWDM_ONU_Ibias_Max_Enable(inst);
//	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int DWDM_ONU_Set_Ibias_Max_Ex(int inst,int *Imax_dac )
{
	char strInpt[256];
	char strOupt[256];   
	int		error,i;
	INT8U 	ChNumSet[2];
	int		ChWLRange[4];
	int		index,channel;
	INT8U	Ibias_DAC;
	int		Ibias_sum;
	INT8U	Ibias_Maxdac; 
	INT8U	temp;  

//	Ibias_Maxdac = my_struDBConfig_ERCompens.IBias_MaxDAC;
	
	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 3, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}  
	
	Delay(0.2); 	
	
	error = I2C_BYTE_READ_DLL (inst, 0xA2, 0xD6, &Ibias_Maxdac); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}

	*Imax_dac=(int)Ibias_Maxdac;
	//����Ibais Max
	sprintf(strInpt, "MCU_SET_TABLE(base,0,154,0x%02X,0x%02X,0x%02X,0x%02X)",Ibias_Maxdac,Ibias_Maxdac,Ibias_Maxdac,Ibias_Maxdac); 
	error=SetCommand (inst,strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	} 
		
	error = SetCommand (inst,"mcu_update_flash(base,0)", strOupt); 
	if (error<0)		
	{
		return -1;	
	}

	error = DWDM_ONU_Ibias_Max_Enable(inst);
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int DWDM_ONU_Ibias_Max_Enable(int inst )
{
	int error;
	INT8U	temp; 
	
   	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 3, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	error = I2C_BYTE_READ_DLL (inst, 0xA2, 0x85, &temp); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	temp= temp | 0x80;
	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x85, temp,0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}
 
int DWDM_ONU_Ibias_Max_Disable(int inst)
{
	int error;
	INT8U	temp; 
	
   	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 3, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 
	error = I2C_BYTE_READ_DLL (inst, 0xA2, 0x85, &temp); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	temp= temp & 0x7F;
	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x85, temp,0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int DWDM_ONU_Set_LOS_Squelch (int inst)
{
	int error;
	INT8U	temp; 
	
   	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x7F, 3, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;} 

	error = I2C_BYTE_WRITE_DLL (inst, 0xA2, 0x9B, 0x80,0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	return 0;
}

int DB_READ_AUTODT_Spec_OSA_TEST(void)
{
   	char buf[1024], DriverChip[50], ERCompensChip[50];
	int  count;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */
	
	sprintf (buf, "SELECT dac_apc,dac_mod,wavelenth_range,wavelenth_max,wavelenth_min FROM autodt_spec_osa_test WHERE PartNumber='%s' AND Version='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
    
	resCode = DBBindColChar (hstmt, 1, 50, struDBConfig_OSATEST.DAC_APC, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 2, 50, struDBConfig_OSATEST.DAC_MOD, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 3, 50, struDBConfig_OSATEST.WAVELENTH_RANGE, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 4, &struDBConfig_OSATEST.WaveLenth_Max, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 5, &struDBConfig_OSATEST.WaveLenth_Min, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) {MessagePopup("��ʾ","���ݿ���autodt_spec_osa_test�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����"); return -1;}
	
	
	return 0;

}

int DB_READ_AUTODT_Spec_OSA_TEST_EX(void)
{
   	char buf[1024], DriverChip[50], ERCompensChip[50];
	int  count;
	
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */
	
	sprintf (buf, "SELECT tec_temper_max,tec_temper_min,wavelenth_max,wavelenth_min,temper_min,temper_max,dac_range FROM autodt_spec_osa_test_ex WHERE PartNumber='%s' AND Version='%s'", my_struConfig.PN, my_struConfig.BOM);

	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
    
	resCode = DBBindColFloat (hstmt, 1,&struDBConfig_OSATEST.Tec_Temper_Max, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 2,&struDBConfig_OSATEST.Tec_Temper_Min, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 3,&struDBConfig_OSATEST.WaveLenth_Max, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 4, &struDBConfig_OSATEST.WaveLenth_Min, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 5, &struDBConfig_OSATEST.Temper_Min, &DBConfigStat);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColFloat (hstmt, 6, &struDBConfig_OSATEST.Temper_Max, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	resCode = DBBindColChar (hstmt, 7,50,struDBConfig_OSATEST.DAC_MOD, &DBConfigStat, "");
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	count=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {count++;}      
    
    if ((resCode != DB_SUCCESS) && (resCode != DB_EOF)) {ShowDataBaseError();  return -1;} 
    
    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (count!=1) {MessagePopup("��ʾ","���ݿ���autodt_spec_osa_test�����ҵ���Ӧ�����ݻ������ݼ�¼����һ����"); return -1;}
	
	
	return 0;

}

int GetBosaSN_OSA_WL_TEST(int panle,int channel, struct struTestData *data)
{
	int	 error;
	char substr, str[25]="", myBareBOM[50]="", batch[50]="";
	int	 prenum, lastnum,num;  
	char prestr[30], preserial[20], laststr[30], lastserial[20], strstr[30], serialstr[20]; 

	GetCtrlVal(panle,PAN_MAIN_STR_SN, data->OpticsSN); 
	if(strlen(data->OpticsSN)<14)
	{
		sprintf(str,"������ͨ��%dģ��������", channel);
		error = PromptPopup ("��ʾ", str, data->OpticsSN, 30);
		if (error<0) 
		{
			return -1;
		}
		

		if (strlen(data->OpticsSN) != my_struDBConfig.SNLength  && strlen(data->OpticsSN) != 13) 
		{
			return -1;
		}

		//��ѯAutoDT_Spec_BarCode���Ƿ�����Ӧ������
		error = CheckBarcode (data->OpticsSN);
		if (error) 
		{
			return -1;
		}
	
		//����Ƿ��Ѿ�����APD���ԣ����Ҵ������Ϊ-111
		error = CheckErrorCode (data->PN, data->OpticsSN);
		if (error) 
		{
			return -1;
		}

		// ������κţ���OA�Ƚϣ���OSA��ԱȽ�
		if (1)
		{
				Scan(my_struConfig.firstsn, "%s>%s[w7]%s", prestr, preserial);     
				Scan(preserial, "%s>%i", &prenum);
	
				Scan(my_struConfig.lastsn, "%s>%s[w7]%s", laststr, lastserial);     
				Scan(lastserial, "%s>%i", &lastnum);
	
				Scan(data->OpticsSN, "%s>%s[w7]%s", strstr, serialstr);     
				Scan(serialstr, "%s>%i", &num);
				
				if (0!= strcmp(prestr, strstr) || 0!= strcmp(laststr, strstr))
				{
					MessagePopup ("��ʾ", "���к�ǰ7λ��sgd_scdd_osa����Ĳ�һ��"); 
					return -1; 
				}  	
	
				if (num < prenum || num > lastnum)
				{
					MessagePopup ("��ʾ", "���кŲ���sgd_scdd_osa��������кŷ�Χ��"); 
					return -1; 
				}   
		}	

	}
	return 0;
}

 
int CaculOSALine(double waveLen_Y[], double tempe_X[],float linepara[])
{
	if ((tempe_X[0] == tempe_X[1]) || waveLen_Y == NULL || tempe_X == NULL)
	{
		return -1;
	}
	linepara[0]= ((waveLen_Y[1] - waveLen_Y[0])/(tempe_X[1] - tempe_X[0]));
	linepara[1]= ((tempe_X[1]*waveLen_Y[0] - waveLen_Y[0])/(tempe_X[1] - tempe_X[0]));
	return 0;
}

int TEST_OSA_EX_EX(int channel,struct struTestData *data)
{
	int error=0;
	int LookFlag=0;
	int count1 = 0;
	double TargetTemper = 0;
	double WaveMinTemper = 0;
	double WaveLength = 0;
	double TestTemp = 0;
	int setDACTmp = 0;
	int setDACTmp2 = 0;
	char tempstr[256];
	char str[256]={0};
	int i;
	double TargetDac = 0;
	double ReadI;
	BOOL temperFlag = FALSE;
	
	//����TEC�ֶ�ģʽ
	errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));
	Delay(0.1);
	//���ùض����ֵΪ�ֶ�0x100;
	errChk(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]));
	Delay(0.1);        
	error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
	Delay(0.1);        
	if(error)
	{
		MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
		return -1;
	}
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
	//set ben to Level mode
	errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));  
	 Delay(0.1);   

	//�ر� Ibias_Max����
	error = DWDM_ONU_Ibias_Max_Disable(INST_EVB[channel]);
	if (error<0) 
	{
		MessagePopup ("ERROR", "�ر�Ibias_Max���ƹ���ʧ��"); 
		return -1;
	}
	if(SET_DAC_APC (channel,0x50) < 0) return -1; //Set Pid
	Delay(0.1);        
	if(SET_DAC_MOD(channel,0x20) < 0) return -1;  //Set Mod
	Delay(0.1);        
	strcpy(struDBConfig_OSATEST.str_WaveRecorder,"");
	strcat(struDBConfig_OSATEST.str_WaveRecorder,"��ʼOSA����:");
	
	error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
	if (error) 
	{
		MessagePopup("����", "3328���ù���ģʽ����   "); 
		return -1;
	}
	do
	{
		ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
		Delay(0.2);
		count1++;
	}while(0==LookFlag && count1 <20);
	// ���㲨��
	for (i = 0;i<2;i++)
	{
		if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], m_MyDACData[i])<0) 
		{
			return -1;
		}
		Delay(3);
		errChk (Read_Wavelength(channel, &m_MyWaveData[i]));
	}
	CaculOSALine(m_MyWaveData,m_MyTemperature,m_MyWaveTempePara);
	CaculOSALine(m_MyWaveData,m_MyDACData,m_MyWaveDacPara);
	// ������̲��������Ӧdac
	TargetDac = (struDBConfig_OSATEST.WaveLenth_Min - m_MyWaveDacPara[1])/m_MyWaveDacPara[0];
	if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], m_MyDACData[i])<0) 
	{
		return -1;
	}
	Delay(3);
	// ��ȡ��Ӧ��̲������¶�
	if(ux3328s_get_LDtempe_TecCurr (INST_EVB[channel],  &WaveMinTemper,&ReadI)<0)
	{
		return -1;
	}
	
	//����Ŀ�겨����Ӧ��TEC�¶�,�ж��Ƿ��ںϷ���Χ��
	TargetTemper = (struDBConfig_OSATEST.WaveLenth_Min - m_MyWaveTempePara[1])/m_MyWaveTempePara[0];
	temperFlag = ((TargetTemper>struDBConfig_OSATEST.Tec_Temper_Min)&&(TargetTemper<struDBConfig_OSATEST.Tec_Temper_Max)&&
					(WaveMinTemper>struDBConfig_OSATEST.Tec_Temper_Min)&&(WaveMinTemper<struDBConfig_OSATEST.Tec_Temper_Max));
	if(!temperFlag)
	{
		sprintf(tempstr,"����Ŀ���¶ȣ�%fʧ�ܻ���%fʧ�ܣ�",TargetTemper,WaveMinTemper);
		MessagePopup ("��ʾ", tempstr);   
		return -1;  
	}
	// ʹ��������¶�
	data->LDTemper[0]= TargetTemper;
	sprintf(str,"Temp=%f;TargetWL=%f;TargetTemp=%f",TargetTemper,struDBConfig_OSATEST.WaveLenth_Min,TargetTemper);	

	strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
	  
	if(TargetTemper>struDBConfig_OSATEST.Temper_Max||TargetTemper<struDBConfig_OSATEST.Temper_Min)
	{
		sprintf(tempstr,"��̲�����Ӧ�¶ȣ�%f,������Χ��%f~%f",TargetTemper,struDBConfig_OSATEST.Temper_Min,struDBConfig_OSATEST.Temper_Max);
		MessagePopup("error",tempstr); 			  
	    strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
		strcat(struDBConfig_OSATEST.str_WaveRecorder,"�¶ȳ�����Χ��");      
		 return -1;   
	}

	strcat(struDBConfig_OSATEST.str_WaveRecorder,"OSA������ɣ�"); 

Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	Delay(0.1);        
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
		//ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;
	
	return 0;
}


int TEST_OSA_EX(int channel,struct struTestData *data)
{
	int index;
	int ROOM_TEC[4];
	double ROOM_Temper[4]; 
	double temp_AOP;
	unsigned short DAC_Arr[3];
	double Temper_Arr[3];
	int MaxCount = 15;  
	int LookFlag=0;
	double TargetTemper=0.;
	double ReadI;
	
	int DAC_Min,DAC_Max; 
	int count1=0;
	int DAC0,DACmin,DACmax,SetDAC;
	int error=0;
	char str[256]={0};
	double	Wavelegnth_Arry[2];    
	char str_WaveRecorder[1024]={0};
	double TestTemp;
	char tempstr[256];
	int count=0;

	 // ����Ӧ�����ϵ� 
	   sscanf (struDBConfig_OSATEST.DAC_MOD, "(0x%03x)(0x%03x)",&DAC_Min,&DAC_Max); 
	//����TEC�ֶ�ģʽ
		errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));
		Delay(0.1);
	//���ùض����ֵΪ�ֶ�0x100;
	 	errChk(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]));
		Delay(0.1);        
	    error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		Delay(0.1);        
		if(error)
		{
			MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
	     errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
		//set ben to Level mode
		 errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));  
 		 Delay(0.1);   
		
		 //�ر� Ibias_Max����
		error = DWDM_ONU_Ibias_Max_Disable(INST_EVB[channel]);
		if (error<0) {MessagePopup ("ERROR", "�ر�Ibias_Max���ƹ���ʧ��"); return -1;}
	
		errChk(SET_DAC_APC (channel,0x55)); //Set Pid
		Delay(0.1);        
		errChk(SET_DAC_MOD(channel,0x20));  //Set Mod
		Delay(0.1);        
		strcpy(struDBConfig_OSATEST.str_WaveRecorder,"");
		strcat(struDBConfig_OSATEST.str_WaveRecorder,"��ʼOSA����:");  
		SetDAC=(DAC_Max+DAC_Min)/2;
		
		count=0;
		do
		{   
		    if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel],SetDAC )<0)
			{
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					MessagePopup("����", "3328���ù���ģʽ����   "); 
					return -1;
				}
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], SetDAC)<0) 
				{
					return -1;
				}
			}  
			do
			{
				ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
				Delay(0.2);
				count1++;
			}while(0==LookFlag && count1 <20);
			if(ux3328s_get_LDtempe_TecCurr (INST_EVB[channel],  &TargetTemper,&ReadI)<0)
			{
				return -1;
			}
			
			if((TargetTemper>struDBConfig_OSATEST.Tec_Temper_Min)&&(TargetTemper<struDBConfig_OSATEST.Tec_Temper_Max))
			{
				   break;   
			}
			else if(TargetTemper<struDBConfig_OSATEST.Tec_Temper_Min)
			{
				
				DAC_Min=SetDAC;
				SetDAC=(DAC_Max+DAC_Min)/2;  
			
			}
			else if(TargetTemper>struDBConfig_OSATEST.Tec_Temper_Max)
			{
			  	DAC_Max=SetDAC;
				SetDAC=(DAC_Max+DAC_Min)/2;   
			}
			if((DAC_Max-DAC_Min)<=2)
			{
			   data->LDTemper[0]= TargetTemper;   	  
		       strcat(struDBConfig_OSATEST.str_WaveRecorder,"���ַ��Ҳ�������¶ȵ�");
			    return -1;
			}
	//		sprintf(str,"DAC=%x,TEC�¶�=%f\n",SetDAC,TargetTemper);			  
	//	    strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
			count++;
		 
		}while(count<30);
		if(count>=30)
		{
			sprintf(tempstr,"����Ŀ���¶ȣ�%fʧ�ܣ�",TargetTemper);
			MessagePopup ("��ʾ", tempstr);   
			return -1;
		}
		
		Delay(3);
		errChk (Read_Wavelength(channel, &Wavelegnth_Arry[0]));
		
		if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_MS9740)   
		{
			errChk(MS9740_Config (Inst_MS9740, LASER_TYPE_DFB, Wavelegnth_Arry[0], 20, 0.2)); 
				
		}
		errChk (Read_Wavelength(channel, &Wavelegnth_Arry[0]));
		if (INSTR[channel].SPECTRUM_TYPE == SPECTRUM_TYPE_MS9740)   
		{
			error = MS9740_Stop (Inst_MS9740);
			if (error)
			{
				MessagePopup ("��ʾ", "��������ɨ��ʧ��");   
				return -1;
			}
		}
		//����Ŀ�겨����Ӧ��TEC�¶�
	    TestTemp=	TargetTemper -(Wavelegnth_Arry[0]- struDBConfig_OSATEST.WaveLenth_Min)/0.1;	  
		data->LDTemper[0]= TestTemp;
		sprintf(str,"TEC_DAC=0x%X;WL=%f;Temp=%f;TargetWL=%f;TargetTemp=%f",SetDAC,Wavelegnth_Arry[0],TargetTemper,struDBConfig_OSATEST.WaveLenth_Min,TestTemp);	
		
		strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
		  
		if(TestTemp>struDBConfig_OSATEST.Temper_Max||TestTemp<struDBConfig_OSATEST.Temper_Min)
		{
			sprintf(tempstr,"��̲�����Ӧ�¶ȣ�%f,������Χ��%f~%f",TestTemp,struDBConfig_OSATEST.Temper_Min,struDBConfig_OSATEST.Temper_Max);
			MessagePopup("error",tempstr); 
			sprintf(str,"DAC=%x,����=%f\n",SetDAC,Wavelegnth_Arry[0]);			  
		    strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
			strcat(struDBConfig_OSATEST.str_WaveRecorder,"�¶ȳ�����Χ��");      
			 return -1;   
		}
	
		strcat(struDBConfig_OSATEST.str_WaveRecorder,"OSA������ɣ�"); 
	
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	Delay(0.1);        
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
		//ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;

  return  0;
}

int TEST_OSA(int channel)
{
	int index;
	int ROOM_TEC[4];
	double ROOM_Temper[4]; 
	double temp_AOP;
	unsigned short DAC_Arr[3];
	double Temper_Arr[3];
	int MaxCount = 15;  
	int LookFlag=0;
	int DAC_APC[4]={0};
	int DAC_MOD[4]={0};  
	int count1=0;
	int DAC0,DACmin,DACmax,SetDAC;
	int error=0;
	char str[50]={0};
	double	Wavelegnth_Arry[2];    
	char str_WaveRecorder[200]={0};

	   sscanf (struDBConfig_OSATEST.DAC_APC, "(0x%02x)(0x%02x)(0x%02x)(0x%02x)",&DAC_APC[0],&DAC_APC[1],&DAC_APC[2],&DAC_APC[3]);  
	   sscanf (struDBConfig_OSATEST.DAC_MOD, "(0x%02x)(0x%02x)(0x%02x)(0x%02x)",&DAC_MOD[0],&DAC_MOD[1],&DAC_MOD[2],&DAC_MOD[3]);    
	   sscanf (struDBConfig_OSATEST.WAVELENTH_RANGE, "(0x%03x)(0x%03x)(0x%03x)",&DAC0, &DACmin, &DACmax);      

	//����TEC�ֶ�ģʽ
		errChk(DWDM_ONU_DS4830_Set_TEC_Manual(INST_EVB[channel]));
		Delay(0.1);
	//���ùض����ֵΪ�ֶ�0x100;
	 	errChk(DWDM_ONU_DS4830_Set_BIAS_Manual(INST_EVB[channel]));
		Delay(0.1);        
	    error=DWDM_ONU_DS4830_Set_BIAS(INST_EVB[channel],0x100);
		Delay(0.1);        
		if(error)
		{
			MessagePopup ("Error", "�л�BIAS�ֶ�ʧ�ܣ�");
		  	return -1;
		}
	     errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL)); 
		//set ben to Level mode
		 errChk(EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL));  
 		 Delay(0.1);        
	
		errChk(SET_DAC_APC (channel,DAC_APC[channel])); //Set Mod 
		Delay(0.1);        
		errChk(SET_DAC_MOD(channel,DAC_MOD[channel]));
		Delay(0.1);        
		strcpy(struDBConfig_OSATEST.str_WaveRecorder,"");
		strcat(struDBConfig_OSATEST.str_WaveRecorder,"��ʼ��������\n");  
		SetDAC=DACmin;
		do
		{   
		    if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel],SetDAC )<0)
			{
				error = DWDM_ONU_DS4830_Enter_Password (INST_EVB[channel]);
				if (error) 
				{
					MessagePopup("����", "3328���ù���ģʽ����   "); 
					return -1;
				}
				if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[channel], SetDAC)<0) 
				{
					return -1;
				}
			}  
			do
			{
				ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
				Delay(0.2);
				count1++;
			}while(0==LookFlag && count1 <20);
			Delay(2);
			errChk (Read_Wavelength(channel, &Wavelegnth_Arry[0]));  
			if(Wavelegnth_Arry[0]>struDBConfig_OSATEST.WaveLenth_Max||Wavelegnth_Arry[0]<struDBConfig_OSATEST.WaveLenth_Min)
			{
			  MessagePopup("error","����������Χ"); 
			  sprintf(str,"DAC=%x,����=%f\n",SetDAC,Wavelegnth_Arry[0]);			  
		      strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
			  strcat(struDBConfig_OSATEST.str_WaveRecorder,"����������Χ��");      
			  return -1;   
			}
			sprintf(str,"DAC=%x,����=%f\n",SetDAC,Wavelegnth_Arry[0]);			  
		    strcat(struDBConfig_OSATEST.str_WaveRecorder,str);
			SetDAC=SetDAC+DAC0;
		}while(SetDAC<=DACmax);
		strcat(struDBConfig_OSATEST.str_WaveRecorder,"OSA����������ɣ�"); 
	
Error:
	//set ben to clock mode
	EVB5_SET_BEN_Mode (INST_EVB[channel], EVB5_BEN_MODE_LEVEL);  
	Delay(0.1);        
	if (DRIVERCHIP_UX3328 == my_struDBConfig_ERCompens.DriverChip)
	{
		un3328_set_UserMode(INST_EVB[channel]);
	}
	else if (DRIVERCHIP_UX3328S == my_struDBConfig_ERCompens.DriverChip)
	{
		//ux3328s_set_UserMode(INST_EVB[channel]);
	}
	return error;

  return  0;
}

int SaveDataBase_1( struct struProcessLOG prolog)
{
  	char buf[5000],temp_buf[100];
	int resCode = 0;   /* Result code                      */ 
	int hstmt = 0;	   /* Handle to SQL statement          */ 
	int runcounts=0;
	int num=0;
    sprintf(buf,"select runcount from autodt_process_log where pn='%s' and sn='%s'and log_action='%s' and p_value='%s'",prolog.PN,prolog.SN,prolog.Log_Action,prolog.P_Value);
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;} 
	resCode = DBBindColInt (hstmt, 1, &runcounts, &DBConfigStat);
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS) {num++;}     
	if(num==1)
	{
	   runcounts++;   
	   sprintf(buf,"update autodt_process_log set  operator='%s',log_action='%s',action_time='%s',parameter='%s',comments='%s',softversion='%s',stationid='%s',runtime=%d,runcount=%d where pn='%s' and sn='%s'", prolog.Operator,prolog.Log_Action,prolog.Action_Time,prolog.Parameter,prolog.Comments,
       prolog.SoftVersion,prolog.StationID,prolog.RunTime,runcounts,prolog.PN,prolog.SN); 
	
	}
	else if(num==0)
	{
	  runcounts=1;
	  sprintf(buf,"INSERT INTO autodt_process_log (id,pn,sn,operator,log_action,action_time,parameter,p_value,comments,softversion,stationid,runtime,runcount)VALUES(s_id.nextval, '%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,%d)", 
	  prolog.PN,prolog.SN,prolog.Operator,prolog.Log_Action,prolog.Action_Time,prolog.Parameter,prolog.P_Value,prolog.Comments,prolog.SoftVersion,prolog.StationID,prolog.RunTime,runcounts);   
	
	
	}
	
	hstmt = DBActivateSQL (hdbc, buf);    
	if (hstmt <= 0) {ShowDataBaseError(); return -1;}    

    resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	return 0;

}

int DWDM_ONU_Select_Channelindex(int channel,int channelindex)
{
	int error,Table;

	//���ò���ͨ��
	if(ux3328s_get_table (INST_EVB[channel],&Table))
	{
		return -1; 
	}
//	Delay(0.2);
	if(ux3328s_select_table (INST_EVB[channel],2)) 
	{
		return -1; 
	}
	if(DWDM_ONU_Sel_Wavelength_Channel (INST_EVB[channel], my_struDBConfig.WavelengthChannel[channelindex])) 
	{
		return -1; 
	}
//	Delay(0.2);
	if(ux3328s_select_table (INST_EVB[channel],Table)) 
	{
		return -1; 
	}
	
	return 0;
	
} 

int DWDM_ONU_Set_Burnin_Enable(int channel )
{
	int error;
	INT8U	temp; 
	
   	error = DWDM_ONU_Set_Burnin_ON(channel);
	if(error)
	{
		return -1;
	}
	
	return 0;
}

int DWDM_ONU_Set_Burnin_ON (int channel)
{
	int   error=0;   
	char   temp;
	int table;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	
	//�л�Ibias ����ģʽ
	if(ux3328s_get_table(INST_EVB[channel],&table))
	{
		MessagePopup("����", "��ȡTable ʧ��   "); 
		return -1;
	}
	
	if(ux3328s_select_table(INST_EVB[channel],3))
	{
		MessagePopup("����", "����Table 3ʧ��   "); 
		return -1;
	}
	error = I2C_BYTE_READ_DLL (INST_EVB[channel], 0xA2, 0x81, &temp); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	temp=temp & 0xEF;
	
	error = I2C_BYTE_WRITE_DLL (INST_EVB[channel], 0xA2, 0x81, temp, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	
	if(ux3328s_select_table(INST_EVB[channel],table))
	{
		MessagePopup("����", "����Table 3ʧ��   "); 
		return -1;
	}
	
	//����Burn in LD�¶����
	sprintf(strInpt, "MCU_SET_ADJUST(6,M,0x00,0x0FFF,0x0FFF,0x0000)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}   
	
	//����Burn in Ibias�¶����,Mod=0;
	sprintf(strInpt, "MCU_SET_TABLE(LUK,6,16,0xFF,0x00,0x00,0x00)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	//����Burn in��־��
	sprintf(strInpt, "MCU_SET_TABLE(base,0,0x2E,0x55,0xAA)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	sprintf(strInpt, "MCU_UPDATE_FLASH(LUK,6)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	sprintf(strInpt, "mcu_update_flash(base,0)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	return 0;
}

int DWDM_ONU_Set_Burnin_OFF (int channel)
{
	int error=0; 
	int table;
	char temp;
	
	char strInpt[256];
	char strOupt[256];
	
	memset(strInpt, 0, 256);
	memset(strOupt, 0, 256); 
	
	//�л�Ibias �ջ�ģʽ 
	if(ux3328s_get_table(INST_EVB[channel],&table))
	{
		MessagePopup("����", "��ȡTable ʧ��   "); 
		return -1;
	}
	
	if(ux3328s_select_table(INST_EVB[channel],3))
	{
		MessagePopup("����", "����Table 3ʧ��   "); 
		return -1;
	}
	error = I2C_BYTE_READ_DLL (INST_EVB[channel], 0xA2, 0x81, &temp); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	
	temp=temp|0x10;
	
	error = I2C_BYTE_WRITE_DLL (INST_EVB[channel], 0xA2, 0x81, temp, 0.1); 
	if (error<0) {MessagePopup ("ERROR", "No Acknowledge from target !"); return -1;}
	Delay(0.5);
	
	if(ux3328s_select_table(INST_EVB[channel],table))
	{
		MessagePopup("����", "����Table 3ʧ��   "); 
		return -1;
	}
	
	//���Burn in��־��
	sprintf(strInpt, "MCU_SET_TABLE(base,0,0x2E,0x00,0x00)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	sprintf(strInpt, "mcu_update_flash(base,0)");
	error=SetCommand (INST_EVB[channel], strInpt, strOupt);
	if(error!=0)
	{
		return -1;
	}
	
	return 0;
}

int Get_TecPID_LookState(int channel)
{
	int count=0;
	int LookFlag;
	
	do
	{
		ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
		Delay(0.2);
		count++;
	}while(0==LookFlag && count <20);
	if(count>=20)
	{
		return -1;
	}
	
	Delay(3);
	return 0;
}

int Get_TecPID_LookState_Ex(int channel)
{
	int count=0;
	int LookFlag;
	
	do
	{
		ux3328s_get_PID_LOOKFLAG(INST_EVB[channel],&LookFlag);	
		Delay(0.2);
		count++;
	}while(0==LookFlag && count <20);
	if(count>=20)
	{
		return -1;
	}
	
	Delay(2);
	return 0;
}

float f(float x,float a,float b,float c,float d) //X���η�����
{
     float y;
    	
	 y=a*x*x*x + b*x*x + c*x + d;
     return y;
 }

float xpoint(float x1,float x2,float a,float b,float c,float d) //������x�ύ������
{
     float y;
     y=(x1*f(x2,a,b,c,d)-x2*f(x1,a,b,c,d))/(f(x2,a,b,c,d)-f(x1,a,b,c,d));
     return y;
 }

float root(float x1,float x2,float a,float b,float c,float d) //�������
{
     float x,y,y1;
     y1=f(x1,a,b,c,d);//y1Ϊx1������
	 
    do
    {
       x=xpoint(x1,x2,a,b,c,d); //��x1��x2֮������x�ύ�㸳ֵ��x
       y=f(x,a,b,c,d); //���뷽�������y
       if(y*y1>0) //�ж�y��y1�Ƿ�ͬ��
       {
           x1=x;
           y1=y;
       }
       else
	   {
           x2=x;
	   }
     }while(fabs(y)>=1e-5); //�趨����
	
     return(x);  
}

int Solving_Root(double *ratio,int order,double *y,double *x)			
{
	float f1,f2,x1,x2;
	float a,b,c,d;
	int i;
	
	a=ratio[3];
	b=ratio[2];
	c=ratio[1];

	for(i=0;i<=order;i++)
	{
		d=ratio[0]-y[i];
	 	x1=0x600;
		x2=0xF00;	
		do
		{
		   	f1=f(x1,a,b,c,d);
		   	f2=f(x2,a,b,c,d);
			if(f1*f2>=0) 
			{
				return -1;
			}	 
		}while(f1*f2>=0);

	    x[i]=(int)root(x1,x2,a,b,c,d);
	}
	return 0;
}

int CheckBarcode(char *SN)
{
	int error=0;
	
	if (0 != strncmp(SN, my_struDBConfig.BarCode, 3))
	{
		MessagePopup("Error", "�������кŵ�Barcode���ϺŲ�����    ");
		return -1;
	}    
	
	return 0;
}

int CheckErrorCode(char *PN,char *SN)
{
	int error=0; 
	
	error = DB_GET_Errorcode(PN,SN);
	if (error)   
	{
		return -1;
	}
	
	return 0; 
}

int DB_GET_Errorcode(char *PN,char *SN)
{
	int num=0, errorcode=0;
	char buf[256];	
	int  resCode;
	int  hstmt;  
	
	if (SERVER_ORACLE == my_struConfig.servertype)
	{ 
		sprintf (buf, "SELECT ErrorCode from (SELECT ErrorCode from AutoDT_Results_OpticsData where PN='%s' and SN='%s' ORDER BY ID DESC) where rownum=1", PN, SN);  
	}
	else
	{
		sprintf (buf, "SELECT TOP 1 ErrorCode from AutoDT_Results_OpticsData where PN='%s' and SN='%s' ORDER BY ID DESC", PN, SN); 
	}
	
	hstmt = DBActivateSQL (hdbc, buf);
    if (hstmt <= 0) {ShowDataBaseError(); return -1;}  
	
	resCode = DBBindColInt (hstmt, 1, &errorcode, &DBConfigStat);   
    if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}
	
	num=0;
    while ((resCode = DBFetchNext (hstmt)) == DB_SUCCESS)  {num++;}   
	
	resCode = DBDeactivateSQL (hstmt);
	if (resCode != DB_SUCCESS) {ShowDataBaseError(); return -1;}

	if (1 == num)
	{
		if (ERR_BOSA_APD_VBR == errorcode)
		{
			MessagePopup("��ʾ","�˲�Ʒ������ͨ����ԣ����Ҳ���ʧ�ܣ�      ");    
			return -1;
		}
	}	
	
	return 0;
}



