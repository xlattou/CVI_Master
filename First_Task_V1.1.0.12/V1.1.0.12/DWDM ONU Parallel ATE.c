#include "radioGroup.h"
#include "combobox.h"
//==============================================================================
//
// Title:       ONU Parallel ATE
// Purpose:     A short description of the application.
//
// Created on:  2013-01-17 at 14:14:49 by Roger.
// Copyright:   . All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>
#include <cvirte.h>     
#include <userint.h>
#include "DWDM ONU Parallel ATE.h"
#include "toolbox.h"
#include "function.h"
#include "TxCalibration.h"  
#include "EVB5.h" 
#include "cvi_db.h"
#include <analysis.h>
#include "About.h"

#define	Debug	0 

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables
static int panMain;
static int panInst;
static int panInit; 
static int panOrder;	//�湤��¼����� 
static int panPopu;		//�˳�����ȷ�϶Ի��� 
static int panConf;     //��������
static int panCaliR;	//�ն�У׼����
static int panAbout;
static int panMenu; 
static int panAdva;     //�߼����ܽ��� 
static int panFiber;	//�������кŽ���
static int phCalibrateOLT;
static int phSMC; 
static int phCalOLT;	//OLT���ܹ�·У׼����
static int phCalT;
static int phCalT_Min; 
static int panOsaMan;   //�ֶ�У׼OSA�������

int ThreadHandle;

int hdbc1 = 0; 
//���̱߳���
static int ghThreadLocalVar;	//���̱߳��ر���
static int gFlag;				//���߳�������־
static unsigned int gIndexVal[CHANNEL_MAX]     = {0, 1, 2, 3, 4, 5, 6, 7}; //���̺߳�������ֵ,ʵ�ʴ�����ǲ���ͨ��ֵ
static int 			gThreadFuncId[CHANNEL_MAX] = {0, 0, 0, 0, 0, 0, 0, 0}; //���߳�ID
static char 		szErrorMsg[CMT_MAX_MESSAGE_BUF_SIZE];


//==============================================================================
// Static functions
int CVICALLBACK OSA_WL_TEST(void *pvFuncData);
int CVICALLBACK tuning (void *pvFuncData);
int CVICALLBACK test (void *pvFuncData);
int CVICALLBACK tuning_Reliability (void *pvFuncData);
int CVICALLBACK test_Reliability (void *pvFuncData);
int CVICALLBACK QAtest (void *pvFuncData);
int CVICALLBACK testAginFront (void *pvFuncData);  
int CVICALLBACK testAginBack (void *pvFuncData);  

int CVICALLBACK Test_Upstream (void *pvFuncData);   

static void CleanUpThreads (void);
static void GetLock (int threadLock, int *obtainedLock);
extern int DWDM_ONU_DS4830_Enter_Password(int handle);
extern int DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7(int handle, int DAC);

//==============================================================================
// Global variables

//==============================================================================
// Global functions
//��Ҫ�����豸�Ĳ������ԣ���������ȡ���ͼ�����׵� 
int testParameterLock(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//�л�ʱ��
		if (CLOCK_TYPE_NONE != INSTR[channel].CLOCK_TYPE)    
		{
			if(SEVB0027_4001_SetChannel(inst_SEVB0027_4001, INSTR[channel].CLOCK_CHAN)<0)
			{
				return -1;
			}
		}
		
		if (INSTR[channel].DCA_TYPE==DCA_TYPE_CSA8000 ) 
		{
			if (!CSA8000_Set_O(inst_CSA8000, my_struCSA8000.Channel_O, my_struCSA8000.X_Scale, my_struCSA8000.X_Position, my_struCSA8000.Y_Scale, my_struCSA8000.Y_Position, my_struCSA8000.Y_Offset, my_struCSA8000.Wavelength, my_struCSA8000.Filter, my_struCSA8000.MaskName)) 
			{
				MessagePopup("Error", "CSA8000 Config for Optics Channel error!");
				error=-1;      
				goto Error;
			} 
		}
		else if (INSTR[channel].DCA_TYPE==DCA_TYPE_Ag86100 )
		{
			if (!Ag86100_SET_O(inst_Ag86100, my_struAg86100.ERFactor, my_struAg86100.X_Scale, my_struAg86100.X_Position, my_struAg86100.Y_Scale, my_struAg86100.Y_Offset, my_struAg86100.Filter, my_struAg86100.Wavelength, my_struAg86100.MaskName, my_struAg86100.MaskMargin, my_struAg86100.Channel_O)) 
			{
				MessagePopup("Error", "Initial Ag86100 error!");
				error=-1;      
				goto Error;
			}
		}
		else
		{
			MessagePopup("Error", "��֧�ָ��豸");
			error=-1;      
			goto Error;	
		}
		
		if (my_struConfig.DT_Tuning || my_struConfig.DT_Tun_Reliability)
		{
			 //ER
		 	if (bOK && my_struConfig.Sel_T_ER)
			{	
			 	strcpy (Info, "���������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
				error = tuningER(channel, data);
				if (error) 
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB ʧ��", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);     
					if (0==data->ErrorCode)
					{
						data->ErrorCode=ERR_TUN_ER;
					}
					bOK=FALSE;  
				}
				else	  	
				{   
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB �ɹ�", data->ER[0], data->ER[1], data->ER[2], data->ER[3]); 
				}  
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
			}

		}
		else if (my_struConfig.DT_Test || my_struConfig.DT_Test_Reliability)
		{    

			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "���������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB ʧ��", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB �ɹ�", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//��ͼ
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "���Է�����ͼ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "���Է�����ͼ:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "���Է�����ͼ:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//ģ��
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "���Է���ģ��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//�����ֵ
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //������
				}	
				if (error) {sprintf (Info, "���Է���ģ�壺ʧ��");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "���Է���ģ�壺�ɹ�");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//����
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "���Թ���...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "���Թ���:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "���Թ���:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}
		else if (my_struConfig.DT_QATest || my_struConfig.DT_OQA)
		{
			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "���������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB ʧ��", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB �ɹ�", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//��ͼ
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "���Է�����ͼ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "���Է�����ͼ:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "���Է�����ͼ:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//ģ��
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "���Է���ģ��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//�����ֵ
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //������
				}	
				if (error) {sprintf (Info, "���Է���ģ�壺ʧ��");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "���Է���ģ�壺�ɹ�");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//����
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "���Թ���...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "���Թ���:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "���Թ���:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}

		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}

int testParameterLock_TestEr(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//�л�ʱ��
		if (CLOCK_TYPE_NONE != INSTR[channel].CLOCK_TYPE)    
		{
			if(SEVB0027_4001_SetChannel(inst_SEVB0027_4001, INSTR[channel].CLOCK_CHAN)<0)
			{
				return -1;
			}
		}
		
		if (INSTR[channel].DCA_TYPE==DCA_TYPE_CSA8000 ) 
		{
			if (!CSA8000_Set_O(inst_CSA8000, my_struCSA8000.Channel_O, my_struCSA8000.X_Scale, my_struCSA8000.X_Position, my_struCSA8000.Y_Scale, my_struCSA8000.Y_Position, my_struCSA8000.Y_Offset, my_struCSA8000.Wavelength, my_struCSA8000.Filter, my_struCSA8000.MaskName)) 
			{
				MessagePopup("Error", "CSA8000 Config for Optics Channel error!");
				error=-1;      
				goto Error;
			} 
		}
		else if (INSTR[channel].DCA_TYPE==DCA_TYPE_Ag86100 )
		{
			if (!Ag86100_SET_O(inst_Ag86100, my_struAg86100.ERFactor, my_struAg86100.X_Scale, my_struAg86100.X_Position, my_struAg86100.Y_Scale, my_struAg86100.Y_Offset, my_struAg86100.Filter, my_struAg86100.Wavelength, my_struAg86100.MaskName, my_struAg86100.MaskMargin, my_struAg86100.Channel_O)) 
			{
				MessagePopup("Error", "Initial Ag86100 error!");
				error=-1;      
				goto Error;
			}
		}
		else
		{
			MessagePopup("Error", "��֧�ָ��豸");
			error=-1;      
			goto Error;	
		}
		
		
	//	else if (my_struConfig.DT_Test || my_struConfig.DT_Test_Reliability)
		{    

			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "���������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB ʧ��", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "��������� Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB �ɹ�", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//��ͼ
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "���Է�����ͼ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "���Է�����ͼ:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "���Է�����ͼ:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//ģ��
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "���Է���ģ��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//�����ֵ
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //������
				}	
				if (error) {sprintf (Info, "���Է���ģ�壺ʧ��");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "���Է���ģ�壺�ɹ�");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//����
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "���Թ���...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "���Թ���:ʧ��");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "���Թ���:�ɹ�");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}

		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}

int testParameterLock_TunEr(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//�л�ʱ��
		if (CLOCK_TYPE_NONE != INSTR[channel].CLOCK_TYPE)    
		{
			if(SEVB0027_4001_SetChannel(inst_SEVB0027_4001, INSTR[channel].CLOCK_CHAN)<0)
			{
				return -1;
			}
		}
		
		if (INSTR[channel].DCA_TYPE==DCA_TYPE_CSA8000 ) 
		{
			if (!CSA8000_Set_O(inst_CSA8000, my_struCSA8000.Channel_O, my_struCSA8000.X_Scale, my_struCSA8000.X_Position, my_struCSA8000.Y_Scale, my_struCSA8000.Y_Position, my_struCSA8000.Y_Offset, my_struCSA8000.Wavelength, my_struCSA8000.Filter, my_struCSA8000.MaskName)) 
			{
				MessagePopup("Error", "CSA8000 Config for Optics Channel error!");
				error=-1;      
				goto Error;
			} 
		}
		else if (INSTR[channel].DCA_TYPE==DCA_TYPE_Ag86100 )
		{
			if (!Ag86100_SET_O(inst_Ag86100, my_struAg86100.ERFactor, my_struAg86100.X_Scale, my_struAg86100.X_Position, my_struAg86100.Y_Scale, my_struAg86100.Y_Offset, my_struAg86100.Filter, my_struAg86100.Wavelength, my_struAg86100.MaskName, my_struAg86100.MaskMargin, my_struAg86100.Channel_O)) 
			{
				MessagePopup("Error", "Initial Ag86100 error!");
				error=-1;      
				goto Error;
			}
		}
		else
		{
			MessagePopup("Error", "��֧�ָ��豸");
			error=-1;      
			goto Error;	
		}
		
		if (my_struConfig.DT_Tuning || my_struConfig.DT_Tun_Reliability)
		{
			 //ER
		 	if (bOK && my_struConfig.Sel_T_ER)
			{	Delay(2);
			 	strcpy (Info, "Ԥ�������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
				error = tuningER_Beforehand(channel, data);
				if (error) 
				{
					sprintf (Info, "Ԥ������� Er4=%.2fdB ʧ��", data->ER[3]);     
					if (0==data->ErrorCode)
					{
						data->ErrorCode=ERR_TUN_ER;
					}
					bOK=FALSE;  
				}
				else	  	
				{   
					sprintf (Info, "��������� Er4=%.2fdB �ɹ�", data->ER[3]); 
				}  
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
			}

		}

Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}


int testParameterLock_TunWavelength(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		if (bOK && my_struConfig.Sel_T_Wavelength) 
		{
			if(my_struConfig.Temper_High )
			{
				strcpy (Info, "����΢������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningWavelength(channel, data);  
//				error = tuningWavelength_HightLow(channel, data);
//				if (error) 
//				{
//					error = tuningWavelength(channel, data);  
//				}
			}
			else if(my_struConfig.Temper_Low)
			{
				strcpy (Info, "����΢������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningWavelength(channel, data);  
//				error = tuningWavelength_HightLow(channel, data);
//				if (error) 
//				{
//					error = tuningWavelength(channel, data);  
//				}
			}
			else
			{
				strcpy (Info, "���µ��Բ���...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningWavelength(channel, data);

			}
		
			if (error) {sprintf (Info, "���Բ��� ʧ��");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "���Բ��� �ɹ� WL1=%f WL2=%f WL3=%f WL4=%f",data->Peakwavelength[0], data->Peakwavelength[1], data->Peakwavelength[2], data->Peakwavelength[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}

int testParameterLock_TunWavelength01(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)		   //Room�ֲ����Բ�������һ�� 
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		if (bOK && my_struConfig.Sel_T_Wavelength) 
		{
			strcpy (Info, "����Ԥ������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningWavelength01(channel, data);
			if (error) {sprintf (Info, "Ԥ������ ʧ��");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "Ԥ������ �ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}

int testParameterLock_TunWavelength02(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)	   //Room�ֲ����Բ������ڶ���
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		if (bOK && my_struConfig.Sel_T_Wavelength) 
		{
			strcpy (Info, "���µ��Բ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningWavelength02(channel, data);
			if (error) {sprintf (Info, "���Բ��� ʧ��");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "���Բ��� �ɹ� WL1=%f WL2=%f WL3=%f WL4=%f",data->Peakwavelength[0], data->Peakwavelength[1], data->Peakwavelength[2], data->Peakwavelength[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}

int testParameterLock_TestWavelength(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)
{
	int error=0,count;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
	char Info_temp[256];
//	char Info[500];
																											  
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//�л���·
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		//����
		if (bOK && my_struConfig.Sel_T_Wavelength)
		{
			strcpy (Info, "���Բ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.DT_Tuning) 
			{
			//	error = testWavelength_Tun(channel, data);
			//	sprintf (Info_temp, "����10%%����:�ɹ� WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio10[0], data->PeakWL_DutyRatio10[1], data->PeakWL_DutyRatio10[2], data->PeakWL_DutyRatio10[3]);
				error = testWavelength(channel, data);  
				sprintf (Info_temp, "����100%%����:�ɹ� WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio99[0], data->PeakWL_DutyRatio99[1], data->PeakWL_DutyRatio99[2], data->PeakWL_DutyRatio99[3]);
			}
			else
			{
				error = testWavelength(channel, data);  
				sprintf (Info_temp, "����100%%����:�ɹ� WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio99[0], data->PeakWL_DutyRatio99[1], data->PeakWL_DutyRatio99[2], data->PeakWL_DutyRatio99[3]);
			}
			if (error)
			{
				strcpy (Info, "���Բ���:ʧ��");												   
				bOK=FALSE;
				data->ErrorCode=ERR_TES_Wavelenth;
			} 
			else	 
			{
				strcpy (Info, Info_temp);
			}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 

Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "��ȡ�豸����Ȩʧ��    ");
		return -1;
	}

	return error;
}



//дEEPROMǰɨ��ģ��SN
int scanModuleSNLock(int channel, struct struTestData *data, int panel, int control, char *Info)
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_ModuleSN, &obtainedLock); 
	if (obtainedLock)
    {
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "�����Ʒ������...");
			Insert_Info(panel, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, data, panSN);
			if (error) 
			{
				sprintf (Info,"�����Ʒ������=%s ʧ��", data->ModuleSN);
				data->ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"�����Ʒ������=%s �ɹ�", data->ModuleSN);
			}         
			Insert_Info(panel, gCtrl_BOX[channel], Info); 
		}
		
Error:	// ע�� ֻ���ڴ˺������ڲ�ʹ��errChk��	
		CmtReleaseLock(ThreadLock_ModuleSN);
    }
	else
	{
		Insert_Info(panel, control, "��ȡģ��SNɨ��Ȩʧ��    ");
		return -1;     
	}

	return error;
}

/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
    int error = 0;
	char version[256], buf[50];
	int i;
    
	memset(Channel_Sel_Flag,0,sizeof(Channel_Sel_Flag)); 
	memset(WaitFlag,0,sizeof(WaitFlag));
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panMain = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_MAIN));
	errChk (panOsaMan = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_OSA));
    errChk (panInst = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_INST));
	errChk (panInit = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_INIT));
	errChk (panPopu = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_POPU));
	errChk (panOrder = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_ORDER)); 
	errChk (panConf = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_CONF)); 
	errChk (phCalT = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_CALT)); 
	errChk (phCalT_Min = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_CALT_M));  
	errChk (phCaliLimit = LoadPanel (0, "TxCalibration.uir", PAN_CALI_L)); 
	errChk (phCheckT = LoadPanel (0, "TxCalibration.uir", PAN_CHE_T)); 
	errChk (panCaliR = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_CALR)); 
	errChk (panAbout = LoadPanel (0, "About.uir", PAN_ABOUT));
	errChk (panAdva = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_ADVA)); 
	errChk (panSN = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_SN)); 
	errChk (panFiber = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_FIBER));
	errChk (phCalibrateOLT = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_CALOLT));
	errChk (phSMC = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PAN_SMC));
	errChk (phCalOLT = LoadPanel (0, "DWDM ONU Parallel ATE.uir", PN_CAL_OLT)); 
	
	Combo_NewComboBox (panOrder, PAN_ORDER_COMBO_ORDER); 
	
	panMenu = GetPanelMenuBar (panMain);

	MyDLLGetServerType(&my_struConfig.servertype); 
		
	sprintf (version, "DWDM ONU���в������\nEXP0000xxx\n%s/%s",SOFTVER,SOFTDATE);
    SetCtrlVal (panAbout, PAN_ABOUT_TEXTMSG, version); 

	//������ؿؼ�ֵ
	for (i=0; i<CHANNEL_MAX; i++)
	{
		sprintf (buf, "%d", i); 
		SetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (1, i+1), buf);
		SetTableCellVal (panFiber, PAN_FIBER_TABLE, MakePoint (1, i+1), buf);
	}
	
	//У׼��������ֵ���ý���
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 1), "���˹�·");
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 2), "�ն˹�·");  
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 3), "OLT�ն˹�·");
	
	showphID = License_Flag_INIT; 
	
	if (!MyDLL_License_Load()) return -1;

	/* Create the thread local variable */
	CmtNewThreadLocalVar (sizeof(unsigned int), NULL, NULL, NULL, &ghThreadLocalVar);
	
	CmtNewLock("123", 0, &ThreadLock);
	CmtNewLock("ModuleSN", 0, &ThreadLock_ModuleSN);
	CmtNewLock("JW8504",0,&ThreadLock_JW8504);
	CmtNewLock("pss",0,&Thread_PSS_PowerPeter);	   //����PSS��ͨ���⹦�ʼ��л� 
	CmtNewLock("ThreadLock_Wavelenth", 0, &ThreadLock_AG86120); 
    /* display the panel and run the user interface */
 //   errChk (DisplayPanel (panMain));
    errChk (RunUserInterface ());
	
Error:
    /* clean up */
    DiscardPanel (panMain);
	
	/* Discard the thread local variable */
	CmtDiscardThreadLocalVar (ghThreadLocalVar);
	
	CmtDiscardLock(ThreadLock);
	CmtDiscardLock(ThreadLock_ModuleSN);       

    return 0;
}

//==============================================================================
// UI callback function prototypes

/// HIFN Exit when the user dismisses the panel.
int CVICALLBACK panelCB (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
	{
		if (gFlag)
		{
			MessagePopup ("��ʾ", "������ֹ���ԣ�Ȼ�����˳�����");
			return -1;
		}
		
		InstallPopup (panPopu);
	}
    return 0;
}

void CVICALLBACK On_Inst (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	MyDLL_License_Load ();
	showphID = License_Flag_panInst;
}

int CVICALLBACK On_Inst_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			RemovePopup (0); 
			
			//�����豸��������������ؼ�
			SetMainControl(panMain);
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Search_ATT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			GVPM_GET_SN (panel, PAN_INST_RING_GVPM); 
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Search_OLT_ATT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			GVPM_GET_SN (panel, PAN_INST_RING_OLT_GVPM); 
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Search_SEVB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_INST_RING_SEVB_TYPE, &index); 
			MyDLL_GET_SEVB_SN (index, panel, PAN_INST_RING_SEVB);

			break;
	}
	return 0;
}

int CVICALLBACK On_Search_BERT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index; 
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_INST_RING_BERT_TYPE, &index);
			if (index == BERT_TYPE_SBERT)
			{
				MyDLL_GET_SEVB_SN (SEVB_TYPE_SEVB5, panel, PAN_INST_RING_BERT);
			}  
			else
			{
				MessagePopup("Error", "Can not find this BERT type");
			}

			break;
	}
	return 0;
}

int CVICALLBACK On_Search_OLT_BERT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index; 
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_INST_RING_OLT_BERT_TYPE, &index);
			if (index == BERT_TYPE_SBERT)
			{
				MyDLL_GET_SEVB_SN (SEVB_TYPE_SEVB5, panel, PAN_INST_RING_OLT_BERT);
			} 
			else if (index==BERT_TYPE_GBERT)  
			{
				GBERT_GET_SN (panel, PAN_INST_RING_OLT_BERT);                  
			}
			else
			{
				MessagePopup("Error", "Can not find this BERT type");
			}

			break;
	}
	return 0;
}

int CVICALLBACK On_Search_DCA (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index; 
	char dac_add[50];
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_INST_RING_DCA_TYPE, &index);
			GetCtrlVal (panel, PAN_INST_STR_DCA, dac_add); 
			if (index == DCA_TYPE_CSA8000) 
			{
				if (!CSA8000_Init(&inst_CSA8000, dac_add)) 
				{MessagePopup("��ʾ", "Initial CSA8000 error!"); return -1;}  
			
				if (!CSA8000_GET_MainSN(inst_CSA8000, my_struCSA8000.MainSN)) 
				{MessagePopup("��ʾ", "��ȡCSA8000�������кŴ���"); return -1;} 
				
				SetCtrlVal (panel, PAN_INST_STR_SN_DCA, my_struCSA8000.MainSN);	 
			}
			else if (index == DCA_TYPE_Ag86100)
			{
				if (!Ag86100_Init(&inst_Ag86100, dac_add)) 
				{MessagePopup("��ʾ", "Initial Ag86100 error!"); return -1;} 
			
				if (!Ag86100_GET_MainSN(inst_Ag86100, my_struAg86100.MainSN)) 
				{MessagePopup("��ʾ", "��ȡAg86100�������кŴ���"); return -1;} 
				
				SetCtrlVal (panel, PAN_INST_STR_SN_DCA, my_struAg86100.MainSN);
			}
			else 
			{
				MessagePopup("��ʾ", "Can not find this DCA!"); 
				return -1;
			}   

			break;
	}
	return 0;
}

int CVICALLBACK On_Inst_Check (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK On_Inst_AddTree (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	struct struInstrument localInst;
	int index;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_INST_NUM_CHANNEL, &localInst.Channel);
			GetCtrlVal (panel, PAN_INST_BIN_CHAN_EN, &localInst.ChannelEn);
			
			GetCtrlVal (panel, PAN_INST_RING_PM_TYPE, &localInst.PM_TYPE);
			GetCtrlVal (panel, PAN_INST_NUM_PM, &localInst.PM);
			GetCtrlVal (panel, PAN_INST_STR_SN_PM, localInst.SN_PM); 

			GetCtrlVal (panel, PAN_INST_RING_SEVB_TYPE, &localInst.SEVB_TYPE); 
			GetCtrlIndex (panel, PAN_INST_RING_SEVB, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_SEVB, index, localInst.SEVB);}  
			else         {strcpy (localInst.SEVB, "");}
			
			GetCtrlVal (panel, PAN_INST_RING_ATT_TYPE, &localInst.ATT_TYPE_ONU); 
			GetCtrlIndex (panel, PAN_INST_RING_GVPM, &index); 
			GetCtrlVal (panel, PAN_INST_NUM_ATT,&localInst.ATT_COM);  
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_GVPM, index, localInst.GVPM_ONU);}  
			else         {strcpy (localInst.GVPM_ONU, "");}   

			GetCtrlVal (panel, PAN_INST_RING_BERT_TYPE, &localInst.BERT_TYPE_ONU); 
			GetCtrlIndex (panel, PAN_INST_RING_BERT, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_BERT, index, localInst.BERT_ONU);}  
			else         {strcpy (localInst.BERT_ONU, "");}
			
			GetCtrlVal (panel, PAN_INST_RING_OLT_ATT_TYPE, &localInst.ATT_TYPE_OLT); 
			GetCtrlIndex (panel, PAN_INST_RING_OLT_GVPM, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_OLT_GVPM, index, localInst.GVPM_OLT);}  
			else         {strcpy (localInst.GVPM_OLT, "");}   

			GetCtrlVal (panel, PAN_INST_RING_OLT_BERT_TYPE, &localInst.BERT_TYPE_OLT); 
			GetCtrlIndex (panel, PAN_INST_RING_OLT_BERT, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_OLT_BERT, index, localInst.BERT_OLT);}  
			else         {strcpy (localInst.BERT_OLT, "");}

			GetCtrlVal (panel, PAN_INST_RING_SPECTRUM_TYPE, &localInst.SPECTRUM_TYPE);
			GetCtrlVal (panel, PAN_INST_STR_SPECTRUM, localInst.SPECTRUM); 
			GetCtrlVal (panel, PAN_INST_STR_SN_SPECTRUM, localInst.SN_SPECTRUM); 

			GetCtrlVal (panel, PAN_INST_RING_DCA_TYPE, &localInst.DCA_TYPE);
			GetCtrlVal (panel, PAN_INST_STR_DCA, localInst.DCA); 
			GetCtrlVal (panel, PAN_INST_STR_SN_DCA, localInst.SN_DCA); 
			
			GetCtrlVal (panel, PAN_INST_STR_FIBER, localInst.Fiber); 
			MyDLLCheckSN(localInst.Fiber);
			
			//�⿪��
			GetCtrlVal (panel, PAN_INST_RING_SW_TYPE, &localInst.SW_TYPE); 
			GetCtrlVal (panel, PAN_INST_NUM_SW, &localInst.SW); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_CHAN, &localInst.SW_CHAN); 
			GetCtrlIndex (panel, PAN_INST_RING_FSW, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_FSW, index, localInst.SW_SN);}  
			else         {strcpy (localInst.SW_SN, "");}
			
			//�⿪�أ������ƣ�
			GetCtrlVal (panel, PAN_INST_RING_SW_TYPE_2, &localInst.SW_TYPE_2); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_2, &localInst.SW_2); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_CHAN_2, &localInst.SW_CHAN_2); 
			GetCtrlIndex (panel, PAN_INST_RING_FSW_2, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_FSW_2, index, localInst.SW_SN_2);}  
			else         {strcpy (localInst.SW_SN_2, "");}
			
			//ʱ���л���
			GetCtrlVal (panel, PAN_INST_RING_CLOCK_TYPE, &localInst.CLOCK_TYPE); 
			GetCtrlIndex (panel, PAN_INST_RING_CLOCK, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_CLOCK, index, localInst.CLOCK);}  
			else         {strcpy (localInst.CLOCK, "");}
			GetCtrlVal (panel, PAN_INST_NUM_CLOCK_CHAN, &localInst.CLOCK_CHAN);
			
			//��ӵ��豸�б�
			InsertTree (panel, PAN_INST_TREE, &localInst, 0);
			
			break;
	}
	return 0;
}

void CVICALLBACK On_Quit (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	panelCB (panMain,EVENT_CLOSE, 0,0,0);
}

int CVICALLBACK On_Inst_Tree (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char label1[256];
	int  ParentIndex, channel, ImageIndex;
	
	switch (event)
	{
		case EVENT_COMMIT:
			;
			break;
        case EVENT_SELECTION_CHANGE:            

			if (eventData1) 
			{
				GetTreeItemParent (panel, PAN_INST_TREE, eventData2, &ParentIndex);
				if (-1 == ParentIndex)
				{   //ѡ���˸���
					GetLabelFromIndex (panel, PAN_INST_TREE, eventData2, label1); 
					GetTreeItemAttribute (panel, PAN_INST_TREE, eventData2, ATTR_IMAGE_INDEX, &ImageIndex);
					ParentIndex = eventData2;
				}
				else
				{   //ѡ��������
					GetLabelFromIndex (panel, PAN_INST_TREE, ParentIndex, label1);  
					GetTreeItemAttribute (panel, PAN_INST_TREE, ParentIndex, ATTR_IMAGE_INDEX, &ImageIndex);
				}

				sscanf (label1,"ͨ��%d",&channel);
	            SetCtrlVal (panel, PAN_INST_NUM_CHANNEL, channel);
				
				//��ʾ�豸�����Ϣ��ע���ʱ�ṹ��INSTR����Ҫ��ֵ������Ҫ��֤֮ǰ�������й�����GetConfig_Inst
				
				if (TREE_IMAGE_OK==ImageIndex || TREE_IMAGE_WARING==ImageIndex)
				{
					SetCtrlVal (panel, PAN_INST_BIN_CHAN_EN, 1);	
				}
				else
				{
					SetCtrlVal (panel, PAN_INST_BIN_CHAN_EN, 0);
				}
				
				//��ʾ�⹦�ʼ���Ϣ
				SetCtrlVal (panel, PAN_INST_RING_PM_TYPE, INSTR[channel].PM_TYPE); 
				SetCtrlVal (panel, PAN_INST_NUM_PM, INSTR[channel].PM);  
				SetCtrlVal (panel, PAN_INST_STR_SN_PM, INSTR[channel].SN_PM); 

				SetCtrlVal (panel, PAN_INST_RING_ATT_TYPE, INSTR[channel].ATT_TYPE_ONU); 
				SetCtrlVal (panel, PAN_INST_NUM_ATT, INSTR[channel].ATT_COM); 
				ClearListCtrl (panel, PAN_INST_RING_GVPM); 
				InsertListItem (panel, PAN_INST_RING_GVPM, 0, INSTR[channel].GVPM_ONU, 0); 
				
				SetCtrlVal (panel, PAN_INST_RING_SEVB_TYPE, INSTR[channel].SEVB_TYPE); 
				ClearListCtrl (panel, PAN_INST_RING_SEVB); 
				InsertListItem (panel, PAN_INST_RING_SEVB, 0, INSTR[channel].SEVB, 0); 

				SetCtrlVal (panel, PAN_INST_RING_BERT_TYPE, INSTR[channel].BERT_TYPE_ONU); 
				ClearListCtrl (panel, PAN_INST_RING_BERT); 
				InsertListItem (panel, PAN_INST_RING_BERT, 0, INSTR[channel].BERT_ONU, 0); 
				 
				//OLT_GVPM
				SetCtrlVal (panel, PAN_INST_RING_OLT_ATT_TYPE, INSTR[channel].ATT_TYPE_OLT); 
				ClearListCtrl (panel, PAN_INST_RING_OLT_GVPM); 
				InsertListItem (panel, PAN_INST_RING_OLT_GVPM, 0, INSTR[channel].GVPM_OLT, 0); 
				//OLT_Bert  
				SetCtrlVal (panel, PAN_INST_RING_OLT_BERT_TYPE, INSTR[channel].BERT_TYPE_OLT); 
				ClearListCtrl (panel, PAN_INST_RING_OLT_BERT); 
				InsertListItem (panel, PAN_INST_RING_OLT_BERT, 0, INSTR[channel].BERT_OLT, 0); 
				
				SetCtrlVal (panel, PAN_INST_RING_SPECTRUM_TYPE, INSTR[channel].SPECTRUM_TYPE); 
				SetCtrlVal (panel, PAN_INST_STR_SPECTRUM, INSTR[channel].SPECTRUM); 
				SetCtrlVal (panel, PAN_INST_STR_SN_SPECTRUM, INSTR[channel].SN_SPECTRUM); 
				
				SetCtrlVal (panel, PAN_INST_RING_DCA_TYPE, INSTR[channel].DCA_TYPE); 
				SetCtrlVal (panel, PAN_INST_STR_DCA, INSTR[channel].DCA); 
				SetCtrlVal (panel, PAN_INST_STR_SN_DCA, INSTR[channel].SN_DCA); 

				SetCtrlVal (panel, PAN_INST_STR_FIBER, INSTR[channel].Fiber); 
				
				// for sw
				SetCtrlVal (panel, PAN_INST_RING_SW_TYPE, INSTR[channel].SW_TYPE); 
				SetCtrlVal (panel, PAN_INST_NUM_SW, INSTR[channel].SW); 
				SetCtrlVal (panel, PAN_INST_NUM_SW_CHAN, INSTR[channel].SW_CHAN); 
				
				// for sw 2
				SetCtrlVal (panel, PAN_INST_RING_SW_TYPE_2, INSTR[channel].SW_TYPE_2); 
				SetCtrlVal (panel, PAN_INST_NUM_SW_2, INSTR[channel].SW_2); 
				SetCtrlVal (panel, PAN_INST_NUM_SW_CHAN_2, INSTR[channel].SW_CHAN_2); 
				
				// for clock
				SetCtrlVal (panel, PAN_INST_RING_CLOCK_TYPE, INSTR[channel].CLOCK_TYPE); 
				ClearListCtrl (panel, PAN_INST_RING_CLOCK); 
				InsertListItem (panel, PAN_INST_RING_CLOCK, 0, INSTR[channel].CLOCK, 0); 
				SetCtrlVal (panel, PAN_INST_NUM_CLOCK_CHAN, INSTR[channel].CLOCK_CHAN); 
			}
			
            break;
	}
	return 0;
}

int CVICALLBACK On_Inst_Save (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel, ParentIndex, ImageIndex, index;
	char label[256],sn[256];
	union uInst
	{
	struct struInstrument sStr;
	unsigned char pStr[sizeof(struct struInstrument)];
	} uInst;

	switch (event)
	{
		case EVENT_COMMIT:
			
			//�����豸���õ��洢�ļ�
			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				//ͨ����������union��������ֵ0���ٴ������Խ���洢�ṹ��
				memset (uInst.pStr, 0, sizeof (uInst.pStr));
				INSTR[channel] = uInst.sStr; 

				sprintf (label, "ͨ��%d", channel);
				GetTreeItemFromLabel (panel, PAN_INST_TREE, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, label, &ParentIndex);
				if (ParentIndex>=0)
				{
					GetTreeItemAttribute (panel, PAN_INST_TREE, ParentIndex, ATTR_IMAGE_INDEX, &ImageIndex);
					if (TREE_IMAGE_OK==ImageIndex || TREE_IMAGE_WARING==ImageIndex) 
					{
						INSTR[channel].ChannelEn=1;
					}
					else
					{
						INSTR[channel].ChannelEn=0; 	
					}
					INSTR[channel].Channel=channel;
					
					index = ParentIndex;
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_PM, &INSTR[channel].PM_TYPE, &INSTR[channel].PM, INSTR[channel].SN_PM);
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SEVB, &INSTR[channel].SEVB_TYPE, INSTR[channel].SEVB);
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_ATT_MAI, &INSTR[channel].ATT_TYPE_ONU, INSTR[channel].GVPM_ONU,&INSTR[channel].ATT_COM);	
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_BERT, &INSTR[channel].BERT_TYPE_ONU, INSTR[channel].BERT_ONU);
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_ATT_OLT, &INSTR[channel].ATT_TYPE_OLT, INSTR[channel].GVPM_OLT);	   
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_BERT_OLT, &INSTR[channel].BERT_TYPE_OLT, INSTR[channel].BERT_OLT);
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SPECTRUM, &INSTR[channel].SPECTRUM_TYPE, INSTR[channel].SPECTRUM, INSTR[channel].SN_SPECTRUM);
					
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_DCA, &INSTR[channel].DCA_TYPE, INSTR[channel].DCA, INSTR[channel].SN_DCA);
			
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_FIBER, sn);
					//��������б������ҪУ׼
					if (0!= stricmp(sn,INSTR[channel].Fiber)) 
					{
						//�����ˣ�ǿ��У׼�ն˺ͷ��˹�·
						sRxCal.flag[channel] = FALSE;
						my_struTxCal.flag[channel]=FALSE; 
					}
					strcpy (INSTR[channel].Fiber, sn);
					
					//�⿪��
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SW, &INSTR[channel].SW_TYPE, &INSTR[channel].SW, &INSTR[channel].SW_CHAN, INSTR[channel].SW_SN);
				
					//�⿪��2
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SW_2, &INSTR[channel].SW_TYPE_2, &INSTR[channel].SW_2, &INSTR[channel].SW_CHAN_2, INSTR[channel].SW_SN_2);
					
					//ʱ���л���
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_CLOCK, &INSTR[channel].CLOCK_TYPE, INSTR[channel].CLOCK, &INSTR[channel].CLOCK_CHAN);
				}
			}
			
			//д�������ļ�
			SetConfig_Inst ();
			
			break;
	}
	return 0;
}

void CVICALLBACK On_Init (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error;
	int channel;
	char Info[256];
	
	memset(Info,0,sizeof(Info));
	error = Initial (panInit, panMain, &channel, TRUE);
	if(error<0) 
	{
		SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 1);
	}
	else		
	{
		SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 0);
	} 
	Insert_Info(panMain, PAN_MAIN_TEXTBOX_CHAN0, Info);
}

int CVICALLBACK On_Pup_Exit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			quittest();
			
			RemovePopup (0);
			
			QuitUserInterface (0);

			break;
	}
	return 0;
}

int CVICALLBACK On_Pup_User (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			RemovePopup (0);  
			
			MyDLL_License_Load ();
			showphID = License_Flag_USER;  
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Pup_Cancel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			RemovePopup (0);
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Login_Username (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_KEYPRESS:

   			if (1280==GetKeyPressEventVirtualKey (eventData2))//"Enter"  
			{
				MyDLL_License_Entry_Username();
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK On_Login_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status;
	int error=0;
	int channel;
	int progresshandle;
	char Info[256]; 
	int i;
	struct struInstrument localInst;
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			status = MyDLL_License_Check(my_struLicense.username, my_struLicense.password, my_struLicense.power);
			if (!status) return -1;
				
			if (License_Flag_panADV==showphID) 
			{
				//��ʾ�߼��������ý���
				if (stricmp (my_struLicense.power, "admin")!=0) 		
				{MessagePopup("��ʾ","������˺�û��ʹ�ô˹��ܵ�Ȩ�ޣ�"); return -1;} 
				
				DisplayPanel (panAdva);
				GetConfig(); 
			
				SetCtrlVal (panAdva, PAN_ADVA_IS_NPI, my_struConfig.isNPI); 
				SetCtrlVal (panAdva, PAN_ADVA_IS_ONUTOOLT, my_struConfig.isONUtoOLT); 
				SetCtrlVal (panAdva, PAN_ADVA_IS_QAUPDATESN, my_struConfig.isQAUpdateSN); 
				SetCtrlVal (panAdva, PAN_ADVA_IS_QANOCHECKBOM, my_struConfig.isQANoCheckBOM);  
				SetCtrlVal (panAdva, PAN_ADVA_IS_SCAN_MODULESN, my_struConfig.isScanModuleSN);
				SetCtrlVal (panAdva, PAN_ADVA_NUMERIC_Agin_AOP, my_struConfig.AOP_AginDelta); 
				SetCtrlVal (panAdva, PAN_ADVA_NUMERIC_Agin_WL, my_struConfig.WL_AginDelta); 
				SetCtrlVal (panAdva, PAN_ADVA_NUM_WLMeterOffset, my_struConfig.WLMeterOffset);  
			}
			else if (License_Flag_panCALI==showphID) 
			{
				//��ʾУ׼���ý���
				if (stricmp (my_struLicense.power, "admin")!=0) 		
				{MessagePopup("��ʾ","������˺�û��ʹ�ô˹��ܵ�Ȩ�ޣ�"); return -1;} 
				
				DisplayPanel (phCaliLimit); 
				
				//��ʾУ׼�ļ������Ϣ
			 	errChk(GetCaliConfigFile());
				
				//�������ȶԻ���
//				progresshandle = CreateProgressDialog ("����У׼��������", "���ݼ��ؽ���", 1, VAL_NO_INNER_MARKERS, "");
				
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
	
				Radio_SetMarkedOption (phCaliLimit, PAN_CALI_L_RAD_CHAN, channel);

			//	SET_EVB_SHDN(channel, 1);
				
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (2, 1), 0.); 
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (3, 1), my_struTxCal.power_min[channel]); 
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (4, 1), my_struTxCal.power_max[channel]); 
			
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (2, 2), sRxCal.power_in[channel]);  
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (3, 2), sRxCal.power_min[channel]); 
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (4, 2), sRxCal.power_max[channel]); 
				
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (2, 3), my_struCal_OLT.Power_In[channel]);  
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (3, 3), my_struCal_OLT.Power_Min[channel]); 
				SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (4, 3), my_struCal_OLT.Power_Max[channel]); 
			}
			else if (License_Flag_USER == showphID)
			{
				SetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, my_struLicense.username);
			}
			else if (License_Flag_INIT == showphID)
			{
				showphID = License_Flag_panMain; 
				
				DisplayPanel (panMain);
				
				SetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, my_struLicense.username); 
				
				for (channel=0; channel<CHANNEL_MAX; channel++)
				{
					my_struTxCal.flag[channel]=TRUE;    //�������Բ���У׼ 
					sRxCal.flag[channel] = FALSE;
				}
				
				//��ȡϵͳ����
				error = Initial (panInit, panMain, &channel, FALSE);  
				if(error<0) 
				{
					SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 1);
				}
				else		
				{
					SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 0);
				} 
			}
			else if (License_Flag_panInst==showphID)
			{
				if (stricmp (my_struLicense.power, "admin")!=0) 		
				{
					MessagePopup("��ʾ","������˺�û��ʹ�ô˹��ܵ�Ȩ�ޣ�"); 
					return -1;
				} 
				
				InstallPopup (panInst);
	
				GetConfig_Inst();
	
				for (i=0; i<CHANNEL_MAX; i++)
				{
					localInst = INSTR[i]; 
					InsertTree (panInst, PAN_INST_TREE, &localInst, 1); 
				}
			}
			else
			{
				MessagePopup("��ʾ","���������ʾ�߼��쳣    "); 
				return -1;	
			}
			
			break;
		}
	
Error:
	
	return error;
}

int CVICALLBACK On_Login_Password (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_KEYPRESS:

   			if (1280==GetKeyPressEventVirtualKey (eventData2))//"Enter"
			{
				On_Login_OK (panel, control, EVENT_COMMIT, 0, 0, 0);
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK On_Login_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			
			if (!MyDLL_License_Close()) return -1;
			
			if (License_Flag_panMain==showphID)  
			{	 //��ǰ���������������
				panelCB (panMain,EVENT_CLOSE, 0,0,0); 
			}
			else if (License_Flag_INIT == showphID)
			{
				//��ǰ���滹û�д������棬ֱ���˳�
				QuitUserInterface (0); 
			}
			
			break;
		}
	return 0;
}

void CVICALLBACK On_SGD (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	//�������
	SetCtrlVal (panMain, PAN_MAIN_STR_BATCH, ""); 
	
	if (0==stricmp (my_struConfig.PN, ""))
	{
		MessagePopup("��ʾ", "����ѡ���Ʒ�ͺ�    "); 
		return;
	}
	
	InstallPopup (panOrder); 
	
	SetCtrlVal (panOrder, PAN_ORDER_TEXTMSG, "���ڲ�ѯOAϵͳ�����Ե�...    "); 
	SetWaitCursor (1); 
//	DBOA_Read_pch (my_struConfig.PN, my_struConfig.BOM, panOrder, PAN_ORDER_COMBO_ORDER);
	DB_Read_pch (my_struConfig.PN, my_struConfig.BOM, panOrder, PAN_ORDER_COMBO_ORDER); 		//�л�ΪK3ϵͳ
	SetWaitCursor (0); 
	SetCtrlVal (panOrder, PAN_ORDER_TEXTMSG, "��ѯOAϵͳ����    ");  
}

void CVICALLBACK On_User (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	MyDLL_License_Load ();
	showphID = License_Flag_USER;  
}

int CVICALLBACK On_Order (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char buf[50];
	char tempstr[50], BatchArray[8][3]={"PT","CT","GT","RT","WT","DT","ST","VT"}, WorkOrder[30]; 
	int  BatchNum=8, i, error;
	char str0[256],str1[256],str2[256]; 
	switch (event)
	{
		case EVENT_COMMIT:  
			memset(str0,0,sizeof(str0));
			memset(str1,0,sizeof(str1)); 
			memset(str2,0,sizeof(str2)); 
			GetCtrlVal (panel, PAN_ORDER_COMBO_ORDER, buf);
			Scan(buf, "%s>%s[xt59]%s[xt59]%s", str0,str1,str2);   
			strcpy (WorkOrder, str0);  
			error = DB_Read_Order (WorkOrder);			//MySQL�л���Oracle	wenyao.xi 2015-11-27   
			if (error) 	{Insert_Info(panMain, PAN_MAIN_TEXTBOX_CHAN0, "��ѯOA���ݿ��쳣");}  
			else    	{Insert_Info(panMain, PAN_MAIN_TEXTBOX_CHAN0, "��ѯOA���ݿ�ɹ�");} 
	
			//��������
			SetCtrlVal (panMain, PAN_MAIN_STR_BATCH, WorkOrder); 
			strcpy(my_struConfig.batch, WorkOrder);
			RemovePopup (0); 
	}
	return 0;
}

int CVICALLBACK On_Config_PN (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			Update_Config_Ver(panConf, TRUE);
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Config_BOM (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			Update_Config_Ver(panConf, FALSE);
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Config_Item (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			SetCtrlVal (panel, PAN_CONF_RAD_TUNING, 0);
			SetCtrlVal (panel, PAN_CONF_RAD_TUN_RX, 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_TEST_RX, 0);  
			SetCtrlVal (panel, PAN_CONF_RAD_TEST, 0);
			SetCtrlVal (panel, PAN_CONF_RAD_QA, 0);
			SetCtrlVal (panel, PAN_CONF_RAD_OQA, 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_Agin_Front, 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_Agin_Back, 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_TUNING_Reliabilit, 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_TEST_Reliability , 0); 
			SetCtrlVal (panel, PAN_CONF_RAD_Upstream_Test , 0);  
			SetCtrlVal (panel, PAN_CONF_RADIOBUTTON_OSATEST , 0); 
			SetCtrlVal (panel, control, 1);

			Update_Config_Ver(panConf, FALSE);

			break;
	}
	return 0;
}

int CVICALLBACK On_Config_Temper (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			SetCtrlVal (panel, PAN_CONF_RAD_HIGH, 0);
			SetCtrlVal (panel, PAN_CONF_RAD_ROOM, 0);
			SetCtrlVal (panel, PAN_CONF_RAD_LOW, 0);
			SetCtrlVal (panel, control, 1);
			
			Update_Config_Ver(panConf, FALSE);

			break;
	}
	return 0;
}

void CVICALLBACK On_Config (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int count;
	
	InstallPopup (panConf);  
	
	MyDLLGETPartNumber (panConf, PAN_CONF_RING_PN);
	
	//��ȡ�����ļ�
	GetConfig(); 
	//�����ļ���ŵ���Ϣ���ý����ϵ���ʾ��Ϣ
	//����PNIndex>PAN_CONF_RING_PN��Ŀ��bug
	GetNumListItems (panConf, PAN_CONF_RING_PN, &count);
	if (my_struConfig.PNIndex>(count-1)) my_struConfig.PNIndex=0;
	
	SetCtrlIndex (panConf, PAN_CONF_RING_PN, my_struConfig.PNIndex);
	SetCtrlVal (panConf, PAN_CONF_RAD_TUNING, my_struConfig.DT_Tuning);
	SetCtrlVal (panConf, PAN_CONF_RAD_TUN_RX, my_struConfig.DT_Tun_RX); 
	SetCtrlVal (panConf, PAN_CONF_RAD_TEST_RX, my_struConfig.DT_Test_RX); 
	SetCtrlVal (panConf, PAN_CONF_RAD_TEST, my_struConfig.DT_Test); 
	SetCtrlVal (panConf, PAN_CONF_RAD_QA, my_struConfig.DT_QATest); 
	SetCtrlVal (panConf, PAN_CONF_RAD_OQA, my_struConfig.DT_OQA);       
	SetCtrlVal (panConf, PAN_CONF_RAD_LOW, my_struConfig.Temper_Low); 
	SetCtrlVal (panConf, PAN_CONF_RAD_ROOM, my_struConfig.Temper_Room); 
	SetCtrlVal (panConf, PAN_CONF_RAD_HIGH, my_struConfig.Temper_High); 
	
	SetCtrlVal (panConf, PAN_CONF_RAD_Agin_Front, my_struConfig.DT_Agin_Front); 
	SetCtrlVal (panConf, PAN_CONF_RAD_Agin_Back, my_struConfig.DT_Agin_Back);
	
	SetCtrlVal (panConf, PAN_CONF_RAD_Upstream_Test, my_struConfig.DT_Test_Upstream);  
	SetCtrlVal (panConf, PAN_CONF_RADIOBUTTON_OSATEST, my_struConfig.DT_OSA_WL_Test);    
	
	SetCtrlVal (panConf, PAN_CONF_RAD_TUNING_Reliabilit, my_struConfig.DT_Tun_Reliability); 
	SetCtrlVal (panConf, PAN_CONF_RAD_TEST_Reliability, my_struConfig.DT_Test_Reliability);

	Update_Config_Ver(panConf, TRUE);
}

int CVICALLBACK On_Config_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index, channel;
	int error;
	char testitem[50], Info[500];
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlIndex (panConf, PAN_CONF_RING_PN, &my_struConfig.PNIndex);
			if(my_struConfig.PNIndex>=0)
			{
				GetLabelFromIndex (panConf, PAN_CONF_RING_PN, my_struConfig.PNIndex, my_struConfig.PN);  
			}
			else
			{	
				strcpy (my_struConfig.PN, "");
			}
			 
			//���²�Ʒ�汾
			GetCtrlIndex (panConf, PAN_CONF_RING_BOM, &index);  
			if(index>=0)
			{
				GetLabelFromIndex (panConf, PAN_CONF_RING_BOM, index, my_struConfig.BOM);
			}
			else
			{
				strcpy (my_struConfig.BOM, "");
			}

			//���²�ƷEEPROM�汾
			GetCtrlIndex (panConf, PAN_CONF_RING_EED, &index);
			if(index>=0)
			{
				GetLabelFromIndex (panConf, PAN_CONF_RING_EED, index, my_struConfig.EED);
			}
			else
			{
				strcpy (my_struConfig.EED, "");
			}
			
			//ʶ���Ƿ���AL�ͻ���Ʒ
			if (strstr (my_struConfig.PN, "-AL") != NULL) my_struConfig.CUSTOMER = CUSTOMER_AL;
			else if (strstr (my_struConfig.PN, "-AP") != NULL) my_struConfig.CUSTOMER = CUSTOMER_AP; 
			else if (strstr (my_struConfig.PN, "-01") != NULL) my_struConfig.CUSTOMER = CUSTOMER_01;		/***����HW�ͻ�**Eric.Yao***/       
			else   										  my_struConfig.CUSTOMER = CUSTOMER_STANDARD;

			//function
			GetCtrlVal (panConf, PAN_CONF_RAD_TUNING, &my_struConfig.DT_Tuning);
			GetCtrlVal (panConf, PAN_CONF_RAD_TUN_RX, &my_struConfig.DT_Tun_RX); 
			GetCtrlVal (panConf, PAN_CONF_RAD_TEST_RX, &my_struConfig.DT_Test_RX); 
			GetCtrlVal (panConf, PAN_CONF_RAD_TEST, &my_struConfig.DT_Test); 
			GetCtrlVal (panConf, PAN_CONF_RAD_QA, &my_struConfig.DT_QATest); 
			GetCtrlVal (panConf, PAN_CONF_RAD_OQA, &my_struConfig.DT_OQA);     /***���OQA����ѡ��**Eric.Yao***/
			GetCtrlVal (panConf, PAN_CONF_RAD_Agin_Front, &my_struConfig.DT_Agin_Front); 
			GetCtrlVal (panConf, PAN_CONF_RAD_Agin_Back, &my_struConfig.DT_Agin_Back);   
			GetCtrlVal (panConf, PAN_CONF_RAD_TUNING_Reliabilit, &my_struConfig.DT_Tun_Reliability); 
			GetCtrlVal (panConf, PAN_CONF_RAD_TEST_Reliability, &my_struConfig.DT_Test_Reliability); 
			
			GetCtrlVal (panConf, PAN_CONF_RAD_LOW, &my_struConfig.Temper_Low); 
			GetCtrlVal (panConf, PAN_CONF_RAD_ROOM, &my_struConfig.Temper_Room); 
			GetCtrlVal (panConf, PAN_CONF_RAD_HIGH, &my_struConfig.Temper_High); 
			//item
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_AOP, &my_struConfig.Sel_T_AOP); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_ER, &my_struConfig.Sel_T_ER);
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_DIS, &my_struConfig.Sel_T_TxDis); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_EYE, &my_struConfig.Sel_T_Eye);
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_MASK, &my_struConfig.Sel_T_Mask); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_T_SPECTRUM, &my_struConfig.Sel_T_Spectrum); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_R_SENS, &my_struConfig.Sel_R_Sens); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_R_OVER, &my_struConfig.Sel_R_Over); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_R_SDHYS, &my_struConfig.Sel_R_SDHys); 
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_CALI, &my_struConfig.Sel_Calibrate);
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_CALI_TEST, &my_struConfig.Sel_Calibrate_Test);
			GetCtrlVal (panConf, PAN_CONF_CHE_BOX_EE_PROTECT, &my_struConfig.Sel_EE_P); 
			GetCtrlVal (panConf, PAN_CONF_CHE_SENS_DETail, &my_struConfig.Sel_R_Sens_Detail); 
			GetCtrlVal (panConf, PAN_CONF_CHE_TXSD, &my_struConfig.Sel_TxSD); 
			GetCtrlVal (panConf, PAN_CONF_CHE_SD_DETail, &my_struConfig.Sel_RxSD_Detail); 
		
			//��ȡ����	
			error = DB_Get_ConfigInfo ();
			if(error) return -1;
			
			//OSA��������	 
			if (my_struConfig.DT_OSA_WL_Test)  
			{
				DB_READ_AUTODT_Spec_OSA_TEST();   //lxf 2016-11-30
				DB_READ_AUTODT_Spec_OSA_TEST_EX();   //lxf 2016-11-30   

				RemovePopup (0); 
				SetCtrlVal (panMain, PAN_MAIN_STR_PN, my_struConfig.PN); 
				SetCtrlVal (panMain, PAN_MAIN_STR_BOM, my_struConfig.BOM);  
				SetConfig();  
				error = Initial(panInit, panMain, &channel, TRUE);
				if(error<0) 
				{
					SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 1);
				}
				else		
				{
					SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 0);
				}
			
				SetActiveCtrl (panMain, PAN_MAIN_STR_SN);
				my_struDBConfig_ERCompens.ChiefChip=CHIEFCHIP_DS4830;
				my_struDBConfig_ERCompens.DriverChip=	DRIVERCHIP_UX3328S;
				 
				strcpy (testitem, "OSA��������");  
				SetCtrlVal (panMain, PAN_MAIN_TEXTMSG_INFO, testitem);  
				SetCtrlAttribute (panMain, PAN_MAIN_TEXTMSG_INFO, ATTR_VISIBLE, 1); 
			   
				return 0;
			}
			

			
			error = DB_Read_Spec_Tracking_Ex1();
			if(error) return -1;
			
			if (DB_READ_AutoDT_Spec_ERCompens()<0)
			{
				return -1;
			}
			
			if (!DB_GET_Spec_ERCompens_Ex())
			{
				return -1;
			}
			
			if(!DB_GET_Firmware_Ver())
			{
			   return -1;  
			}
			
			
			//������оƬ��ȡ����
			if (CHIEFCHIP_UX3328 ==my_struDBConfig_ERCompens.ChiefChip || CHIEFCHIP_UX3328S ==my_struDBConfig_ERCompens.ChiefChip || CHIEFCHIP_DS4830 ==my_struDBConfig_ERCompens.ChiefChip) 
			{
				error = DB_Get_AutoDT_Spec_UX3328();
				if (error) return -1;
			}
/*			else if (CHIEFCHIP_MEGA88 == my_struDBConfig_ERCompens.ChiefChip  || CHIEFCHIP_TINY13 == my_struDBConfig_ERCompens.ChiefChip)
			{
				error = DB_Get_Spec_Image_Mega88();
				if (error) return -1;
			}
			else if (CHIEFCHIP_TINY13 == my_struDBConfig_ERCompens.ChiefChip)
			{
				error = DB_GeT_Spec_Image_Tiny13();
				if (error) return -1;		
			}
			else
			{
				MessagePopup ("��ʾ", "���ݿ�AutoDT_Spec_ERCompens���ж����ERCompensChip����ʶ��"); 
				return -1;
			}
			
			//��������оƬ��ȡ����
			if (DRIVERCHIP_VSC7967==my_struDBConfig_ERCompens.DriverChip || DRIVERCHIP_VSC7965==my_struDBConfig_ERCompens.DriverChip)
			{
				error = DB_Get_AutoDT_Spec_VSC796x();
				if (error) return -1;
			}
			else if (DRIVERCHIP_NT25L90==my_struDBConfig_ERCompens.DriverChip)
			{
				error = DB_Get_AutoDT_Spec_NT25L90();
				if (error) return -1;

				//��ȡfOVERDFlag����ֵ,ֻ��Vsc7967��Ч��Ĭ��ֵ0
				my_struDBConfig_VSC7965.UpdatefOVERDFlag=0;
			}
			else if (DRIVERCHIP_UX3328==my_struDBConfig_ERCompens.DriverChip) 
			{
				//��ȡfOVERDFlag����ֵ,ֻ��Vsc7967��Ч��Ĭ��ֵ0
				my_struDBConfig_VSC7965.UpdatefOVERDFlag=0;
			}
			else
			{
				MessagePopup("��ʾ","���ݿ�AutoDT_Spec_ERCompens��DriverChip����ֵ����ȷ��"); 
				return -1;  
			}
*/			
			//���˼������
			error = DB_Read_Spec_TxCal();
			if(error) return -1;
			
			//��ȡ��for Calibrate  
			if(my_struConfig.Sel_Calibrate_Test || my_struConfig.Sel_Calibrate)
			{
				error = DB_Get_Config_Monitor_Info ();
				if(error) return -1;
				
				//ֻ��ȡ��־λ==2��У׼���Ե�
				if (my_struConfig.Sel_Calibrate)
				{
					error = DB_Read_Spec_Monitor_Ex (1);
					if (error) return -1; 
				}
				else
				{
					error = DB_Read_Spec_Monitor_Ex (2);
					if (error) return -1; 					
				}
			}
			
			error = DB_Get_EEPROM ();
			if (error) return -1;
			
			error = DB_GET_Wavelength();
			if (error) return -1; 
			
			error = checkfunction(); 
			if (error) return -1;
			
			RemovePopup (0); 
			
			SetCtrlVal (panMain, PAN_MAIN_STR_PN, my_struConfig.PN); 
			SetCtrlVal (panMain, PAN_MAIN_STR_BOM, my_struConfig.BOM);
			
			SetConfig();  
			
			if (my_struConfig.DT_Tuning) 	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "���µ���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "���µ���");
				}
				else
				{
					strcpy (testitem, "���µ���"); 
				}   
			}
			else if (my_struConfig.DT_Tun_Reliability) 	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "�ɿ���_���µ���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "�ɿ���_���µ���");
				}
				else
				{
					strcpy (testitem, "�ɿ���_���µ���"); 
				}   
			}
			else if (my_struConfig.DT_Test_Reliability) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "�ɿ���_���²���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "�ɿ���_���²���");
				}
				else
				{
					strcpy (testitem, "�ɿ���_���²���"); 
				}
			}
			else if (my_struConfig.DT_Test_Upstream) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "��������_���²���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "��������_���²���");
				}
				else
				{
					strcpy (testitem, "��������_���²���"); 
				}
			}
			else if (my_struConfig.DT_Tun_RX) 	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "�����ն˵���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "�����ն˵���");
				}
				else
				{
					strcpy (testitem, "�����ն˵���"); 
				}
			}
			else if (my_struConfig.DT_Test_RX) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "�����ն˲���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "�����ն˲���");
				}
				else
				{
					strcpy (testitem, "�����ն˲���"); 
				}
			}
			else if (my_struConfig.DT_Test) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "���²���");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "���²���");
				}
				else
				{
					strcpy (testitem, "���²���"); 
				}
			}
			else if (my_struConfig.DT_QATest)	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "����QA����");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "����QA����");
				}
				else
				{
					strcpy (testitem, "����QA����"); 
				}
			}
			else if (my_struConfig.DT_OQA)	
			{
				 /***����OQA**Eric.Yao***/       
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "����OQA����");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "����OQA����");
				}
				else
				{
					strcpy (testitem, "����OQA����"); 
				}
			}
			else if (my_struConfig.DT_Agin_Front)	
			{
				strcpy (testitem, "�����ϻ�ǰ");  
			}
			else if (my_struConfig.DT_Agin_Back)	
			{
				strcpy (testitem, "�����ϻ���");  
			}
			else if (my_struConfig.DT_OSA_WL_Test)	
			{
				strcpy (testitem, "����OSA��������");  
			}
			else						
			{
				strcpy (testitem, ""); 
			}
			
			SetCtrlVal (panMain, PAN_MAIN_TEXTMSG_INFO, testitem);  
			
			SetCtrlAttribute (panMain, PAN_MAIN_TEXTMSG_INFO, ATTR_VISIBLE, 1); 
			if (my_struConfig.DT_QATest || my_struConfig.DT_OQA)
			{			
				SetCtrlAttribute (panMain, PAN_MAIN_TEXTMSG_INFO, ATTR_TEXT_COLOR, VAL_RED); 
			}
			else
			{
				SetCtrlAttribute (panMain, PAN_MAIN_TEXTMSG_INFO, ATTR_TEXT_COLOR, VAL_BLACK); 
			}
			
			error = Initial(panInit, panMain, &channel, TRUE);
			if(error<0) 
			{
				SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 1);
			}
			else		
			{
				SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 0);
			}
			
			SetActiveCtrl (panMain, PAN_MAIN_STR_SN);
			
			break;
	}
	return 0;				  
}

int CVICALLBACK On_Run (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	int status;
	
	switch (event)
	{
		case EVENT_COMMIT:

			gFlag = TRUE;
			
			//�޸Ľ���ؼ�����
			SetCtrlVal (panMain, PAN_MAIN_BUT_STOP, 0); 
			SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 1); 
			SetMenuBarAttribute (panMenu, 0, ATTR_DIMMED, 1);
			
			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				SetCtrlAttribute (panMain, gCtrl_BIN[channel], ATTR_DIMMED, 1);  
			}
			
			status = CmtNewThreadPool(CHANNEL_MAX, &ThreadHandle);
			if (status<0)	
			{
				CmtGetErrorMessage (status, szErrorMsg);
				MessagePopup ("Error Starting Thread Function", szErrorMsg);
			}
			else
			{
				for (channel = 0; channel < CHANNEL_MAX; channel++)
				{
					if (INSTR[channel].ChannelEn)
					{
						if (my_struConfig.DT_Tuning) 
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, tuning, &gIndexVal[channel], &gThreadFuncId[channel]);
						}
						else if (my_struConfig.DT_Test)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, test, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else if (my_struConfig.DT_QATest)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, QAtest, &gIndexVal[channel], &gThreadFuncId[channel]);     
						}
						else if (my_struConfig.DT_Agin_Front)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, testAginFront, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else if (my_struConfig.DT_Agin_Back)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, testAginBack, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else if (my_struConfig.DT_Tun_Reliability) 
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, tuning_Reliability, &gIndexVal[channel], &gThreadFuncId[channel]);
						}
						else if (my_struConfig.DT_Test_Reliability)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, test_Reliability, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else if (my_struConfig.DT_Test_Upstream)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, Test_Upstream, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else if (my_struConfig.DT_OSA_WL_Test)
						{
							status = CmtScheduleThreadPoolFunction (ThreadHandle, OSA_WL_TEST, &gIndexVal[channel], &gThreadFuncId[channel]);   
						}
						else
						{
							MessagePopup ("��ʾ", "��ǰ����֧�ִ˲��Թ���");
							On_Stop (panel, PAN_MAIN_BUT_STOP, EVENT_COMMIT, 0,0,0);
							status=-1;
						}
					
						if (status<0)	
						{
							CmtGetErrorMessage (status, szErrorMsg);
							MessagePopup ("Error Starting Thread Function", szErrorMsg);
						}
					}
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK On_Stop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	switch (event)
	{
		case EVENT_COMMIT:

			gFlag = FALSE;
			
			CleanUpThreads ();

			//�޸Ľ���ؼ�����
			SetCtrlVal (panMain, PAN_MAIN_BUT_RUN, 0);
			SetCtrlAttribute (panMain, PAN_MAIN_BUT_RUN, ATTR_DIMMED, 0); 
			SetMenuBarAttribute (panMenu, 0, ATTR_DIMMED, 0); 
			
			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				SetCtrlAttribute (panMain, gCtrl_BIN[channel], ATTR_DIMMED, !INSTR[channel].ChannelEn);  
			}
			
			break;
	}
	return 0;
}

void CVICALLBACK On_His (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char filename[MAX_PATHNAME_LEN], tempstr[1024];	
	
	GetProjectDir (filename);
	strcat (filename, "\\�汾��ʷ.txt");
	
    sprintf (tempstr, "Notepad.exe %s", filename);
    LaunchExecutable (tempstr);
}

void CVICALLBACK On_CalibrateT (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error, i;
	int channel=0;
	float val=999.0;
	char buf[256];
	
	errChk(GetCaliConfigFile()); 
	
	DisplayPanel (phCalT); 

	//���У׼�����ʾ
	FillTableCellRange (phCalT, PAN_CALT_TABLE_T, MakeRect (1, 1, 3, 2), val);
	
	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	SET_ATT_ONU (channel, -60); 
	
	Radio_SetMarkedOption (phCalT, PAN_CALT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	EVB5_SET_BEN_Level(INST_EVB[channel], 0);  
	//===================���ط���У׼��¼===================//
	for (i=0; i<3; i++)
	{
		SetTableCellVal (phCalT, PAN_CALT_TABLE_T, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
		SetTableCellVal (phCalT, PAN_CALT_TABLE_T, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
	}
	//===================���ط���У׼��¼===================//
	
Error:
	
}

void CVICALLBACK On_CalibrateR (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error, i;
	int channel=0;
	float val=-999.0;
	char buf[256];
	
	errChk(GetCaliConfigFile()); 
	
	DisplayPanel (panCaliR); 

	//���У׼�����ʾ
	FillTableCellRange (panCaliR, PAN_CALR_TABLE_R, MakeRect (1, 1, 1, 1), val);

	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (panCaliR, PAN_CALR_RAD_CHAN, channel);

//	errChk(SET_EVB_SHDN(channel, 1));
	
	//===================�����ն�У׼��¼===================//
	if (INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_NONE)
	{
		sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
		MessagePopup("��ʾ", buf);
		error=-1;
		goto Error;
	} 
	
	SetTableCellVal (panCaliR, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 

	errChk(Init_BERT (channel));

	errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
	//===================�����ն�У׼��¼===================// 
	
Error:
	
}


void CVICALLBACK On_CalibrateOLT (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error, i;
	int channel=0;
	float val=-999.0;
	char buf[256];
	
	errChk(GetCaliConfigFile());  

	//���У׼�����ʾ
	FillTableCellRange (phCalOLT, PN_CAL_OLT_TABLE_R, MakeRect (1, 1, 1, 1), val);
	
	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalOLT, PAN_CALOLT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	
	DisplayPanel (phCalOLT);
	
	//===================�����ն�У׼��¼===================//
	if (INSTR[channel].ATT_TYPE_OLT == ATT_TYPE_NONE)
	{
		sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
		MessagePopup("��ʾ", buf);
		error=-1;
		goto Error;
	} 
	
//	SetTableCellVal (phCalOLT, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 

//	errChk(Init_BERT (channel));
	
	//For OLT bert
	if (INSTR[channel].BERT_TYPE_OLT == BERT_TYPE_GBERT) 
	{
		if (!GBERT_Init(&INST_BERT_OLT[channel], INSTR[channel].BERT_OLT, INSTR[channel].GBert_PRBS, INSTR[channel].GBert_Rate, 0)) 
		{
			MessagePopup("Error", "Initial GBERT error!");
			error=-1;
			goto Error;
		} 
	}

	errChk(SET_ATT_OLT(channel, my_struCal_OLT.Power_In[channel]));
	//===================�����ն�У׼��¼===================// 
	
Error:
	
}

/*--------------------------------------------------------------------------*/
/* Wait for threads to finish and reset the function ids				    */
/*--------------------------------------------------------------------------*/
static void CleanUpThreads (void)
{
	int i;
	
	for (i = 0; i < CHANNEL_MAX; i++)
	{
		if (gThreadFuncId[i] != 0) 
		{
			CmtWaitForThreadPoolFunctionCompletion (ThreadHandle, gThreadFuncId[i], 0);
			
			CmtReleaseThreadPoolFunctionID (ThreadHandle, gThreadFuncId[i]);
			
			gThreadFuncId[i] = 0;
		}
	}
	
//	CmtDiscardThreadPool (ThreadHandle);
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

int CVICALLBACK OSA_WL_TEST(void *pvFuncData)
{
  	int channel; 
  	struct struTestData data;    
  	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	struct struProcessLOG prolog;
	int	timestart;

  	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	ResetTextBox (panMain, gCtrl_BOX[channel], "");  
	strcpy (prolog.Log_Action,"OSA_WL_TEST");
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־ 
	while (gFlag)
	{
		
		bOK=TRUE; 
		SaveData=TRUE;		
		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			Delay(0.5);
			//if (!Channel_Sel_Flag[channel]) 
			//{   
			//	Delay(0.1);
			//	continue;
			//}
			
			//�����������
			memset(&data,0,sizeof(data));
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			//.//errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(2);
			WaitFlag[channel] = FALSE;
		} 
		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    "); 	
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼");  
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);

	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ����OSA��������...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0));   
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}  
	    //get bosasn
		if (bOK)
		{
			strcpy (Info, "��������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN_OSA_WL_TEST(panMain,channel, &data); 
			if (error){sprintf (Info, "��������=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;SaveData=FALSE;}  
			else	  {sprintf (Info, "��������=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	//.//  	if(my_struConfig.DT_OSA_WL_Test&&bOK)   
		{
	     	strcpy (Info, "OSA����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TEST_OSA_EX(channel,&data);
			if (error)
			{   sprintf (Info, "OSA����ʧ��;%f������Ӧ�¶�=%f��",struDBConfig_OSATEST.WaveLenth_Min,data.LDTemper[0]);     
			    Insert_Info(panMain, gCtrl_BOX[channel], Info);
				bOK=FALSE;
			}
			else
			{
			    sprintf (Info, "OSA���Գɹ�;%f������Ӧ�¶�=%f��",struDBConfig_OSATEST.WaveLenth_Min,data.LDTemper[0]);
			    Insert_Info(panMain, gCtrl_BOX[channel], Info);
			    SaveData=TRUE;
				bOK=TRUE;
			}
	     }
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN		
//		strcpy (prolog.Operator, my_struLicense.username);
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (prolog.P_Value, "PASS");
			strcpy (data.Status, "PASS"); 
			strcpy (prolog.Comments,struDBConfig_OSATEST.str_WaveRecorder );
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			strcpy (prolog.P_Value, "FAIL");
			strcpy (data.Status, "FAIL");
			strcpy (prolog.Comments, struDBConfig_OSATEST.str_WaveRecorder);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d",my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{ 
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase_1(prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "OSA��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "OSA��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);   
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��OSA������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);
	return 0;   
}

int CVICALLBACK tuning (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  OverTime; 
	int  SensTime; 
	
	char buf[256] = "";  
	int senstime_olt;  

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.DT_Tun_Reliability) 
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TunReliability]);  
	}
	else
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Tuning]);
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־     	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);   
				Delay(0.1);
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}

		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    "); 	
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���е���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}		    
		
		data.Time_Start=Timer();	
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "��ʼ��ʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��鹤��
		if (bOK && !my_struConfig.DT_Tun_Reliability && !my_struConfig.DT_Test_Reliability)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "ģ���·��ʧ��");
				}
				else
				{
					sprintf (Info, "���Թ���ʧ��");       
					data.ErrorCode=ERR_TUN_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "���Թ��� �ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD_Detect�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "���Է���SD_Detect�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "���Է���SD_Detect�źţ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		// ��·����
		if (bOK && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "��ʼ��·����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "��·���ԣ�ʧ��");bOK=FALSE;} 
			else	  {strcpy (Info, "��·���ԣ��ɹ�");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//test SD		
		if (bOK && CHIEFCHIP_DS4830!=my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "����SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testSD (channel);
			if (error) {strcpy (Info, "����SD�źţ�ʧ��");bOK=FALSE;}  
			else	   {strcpy (Info, "����SD�źţ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK)
		{
			strcpy (Info, "����Vapd��ѹ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd (channel, &data);
			if (error)	{strcpy (Info, "����Vapd��ѹ��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "����Vapd��ѹ���ɹ�;APD_DAC=0x%X",(int)data.Vapd_DAC);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;												 
		sprintf(Info, "TunVAPDʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//test SDA SDD	
		if (bOK && my_struConfig.Sel_R_SDHys) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSD(channel,&data);
			if (error){sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD); bOK=FALSE; data.ErrorCode=ERR_TES_SD;} 
			else	  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode)) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestSDʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�������ز���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "�������ز��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/						 
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestOverStartʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//��ѡ��ʾ����ʱѡ�ô˷���
		if (INSTR[channel].DCA_TYPE != DCA_TYPE_NONE)
		{
			//AOP
			if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "���Թ⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP(channel, &data);
				if (error) 
				{
					sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);
					bOK=FALSE;
					if (data.ErrorCode==0) 
					{
						data.ErrorCode=ERR_TUN_AOP; 
					}
				}
				else	  	
				{
					sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm�ɹ�", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);  
				}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
		}
		else
		{
			//AOP & ER
			if (bOK && my_struConfig.Sel_T_AOP && my_struConfig.Sel_T_ER && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "��������Ⱥ͹⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP_ER(channel, &data);
				if (error) {sprintf (Info, "��������Ⱥ͹⹦��=%.2fdBm FAIL", data.AOP);bOK=FALSE;}
				else	   {sprintf (Info, "��������Ⱥ͹⹦��=%.2fdBm PASS", data.AOP);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunAOPʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//�����ز��Խ��
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�����ز��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "���ز���=%.2fdBm ʧ��, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 #if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "OverCheckʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//test sens  �����Ȳ���ʱ������Բ�����ER�����Թ��׸���	  	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "���������Ȳ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "���������Ȳ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenTestStartʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Test Sen Detail
		if (bOK && my_struConfig.Sel_R_Sens && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail)) ) 
		{
			strcpy (Info, "����������ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //�ƽ�����SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //���Ʒ���������1.0e-12������SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Test Sen Detailʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Tuning Wavelength 01
		if (bOK)
		{
			strcpy (Info, "������01,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength01(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 01ʱ��=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/	  
#endif
		// Tun Er, ��ͼ mask ���׵�  
		if (bOK)
		{
			strcpy (Info, "��Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunErʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//Tuning Wavelength  02
		if (bOK)
		{
			strcpy (Info, "������02,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength02(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 02ʱ��=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/	
#endif
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
#if Debug 
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunTxOffʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//Sen Check	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenCheckʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))	/***����У׼����Ϊ��ѡ��**Eric.Yao**20131203***/
		{	
		 	strcpy (Info, "����У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "����У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "����У׼���ɹ�");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
#if Debug	
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TxCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//calRxPower
	 	if (bOK && my_struConfig.Sel_Calibrate)
		{	
		 	strcpy (Info, "�ն�У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = calRxPower(channel, &data, panMain);
			if (error) {sprintf (Info, "�ն�У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_RXPOWER;} 
			else	   {sprintf (Info, "�ն�У׼���ɹ�");} 	      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		  
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "RxCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/   
#endif	
#if Debug
  		//��֤����ǰ��AOP,Er,WL��TxOffPW�仯�Ա��ã�����ʹ����رգ�
		// Test aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestAOPʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
		
		//Test Er
		if (bOK)
		{
			strcpy (Info, "����Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestErʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/

		//Test TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestTxOffʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif 		
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "�Ⲩ��,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestWLʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif	
		if(data.WL_ReTun_FLAG)
		{
			// Test aop
/*			if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "����΢���󣬲��Թ⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testAOP(channel, &data);
				if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
				else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
*/
			
#if Debug		
			/****************************��ʱͳ��********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestAOPʱ��=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************��ʱͳ��********************************/
#endif		
/*			//Test Er
			if (bOK)
			{
				strcpy (Info, "����΢���󣬲���Er,�л���·...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
				if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
			}
*/			
#if Debug
			/****************************��ʱͳ��********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestErʱ��=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************��ʱͳ��********************************/
#endif
			//Test TxOff Depth   
			if (bOK && my_struConfig.Sel_T_TxDis) 
			{
				strcpy (Info, "����΢���󣬲��Թض����...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testTxOff_Depth(channel,&data);
				if (error) 
				{
					//TxOff Depth   
					if (my_struConfig.Sel_T_TxDis) 
					{
						strcpy (Info, "�ص��Թض����...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = tuningTxOff_Depth(channel, &data);
						if (error)
						{
							error = tuningTxOff_Depth_Ex(channel, &data);
							if (error) {sprintf (Info, "�ص��Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
							else	   {sprintf (Info, "�ص��Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
						}
						else
						{
						 	sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
						}
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
					}
					
					//Test TxOff Depth   
					if (bOK && my_struConfig.Sel_T_TxDis) 
					{
						strcpy (Info, "�ز��Թض����...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = testTxOff_Depth(channel,&data);
						if (error) 
						{  	
							sprintf (Info, "�ز��Թض���� ʧ��");
							bOK=FALSE;
							data.ErrorCode=ERR_TES_TxOffPower;
						}
						else	   
						{
							error = testTxOff_Depth(channel,&data);  
							sprintf (Info, "�ز��Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);
						}      
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
					}
					//Test Wavelength 
					if (bOK)
					{
						strcpy (Info, "�Ⲩ��,�л���·...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
						if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
					} 
				
				} 
				else	   
				{
					error = testTxOff_Depth(channel,&data);  
					sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);
				}      
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
		//Cal temperature
		if (bOK)
		{
			strcpy (Info, "ģ���¶�У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = CalTemperature(channel);
			if (error<0) {sprintf (Info, "ģ���¶�У׼: ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TEMPERATURE;} 
			else	   {sprintf (Info, "ģ���¶�У׼���ɹ�");}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TempCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		
#if 0	//Ibias Max�����ɿ�������·���ñ������� ��Ϊ ����AOP���ô���6dBm���쳣���ƣ������ϻ��������ã� 2017-02-23  wenyao.xi
		// �������Ibias�޶�ֵ
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "�������Ibias�޶�ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Ibias_Max_Ex(INST_EVB[channel]);
			if (error) 
			{
				strcpy (Info,"�������Ibias�޶�ֵ��ʧ��");
				data.ErrorCode=ERR_TES_SetIbiasMax; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"�������Ibias�޶�ֵ���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SetImaxʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (prolog.P_Value, "PASS");
			strcpy (data.Status, "PASS"); 
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			strcpy (prolog.P_Value, "FAIL");
			strcpy (data.Status, "FAIL");
			strcpy (prolog.Comments, Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//���ò���ͨ��1
//	DWDM_ONU_Select_Channelindex(channel,0);
	
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK test (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  SensTime,OverTime;
	int senstime_olt;   

	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.Temper_Room)
	{
		if (my_struConfig.DT_Test_Reliability) 
		{
			strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TestReliability]); 
		}
		else
		{
			strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing]);
		}
	}
	else if (my_struConfig.Temper_Low)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing_L]);   
	}
	else if (my_struConfig.Temper_High)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing_H]);   
	}
	else
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣");	
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    ");
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��鹤��
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "ģ���·��ʧ��");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "�⹦�ʳ����ޣ�ʧ��"); 
				}
				else
				{
					sprintf (Info, "���Թ���ʧ��");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "���Թ��� �ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD_Detect�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "���Է���SD_Detect�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "���Է���SD_Detect�źţ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		} 
		
		//test TxSD
/*		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "���Է���SD�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "���Է���SD�źţ��ɹ�");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
*///������Ŀ�ظ���ȡ�� 2017-02-25			   
		//TxDisable
/*		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Է��˹ضϹ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "���Է��˹ضϹ⹦��AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm �ɹ�", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Է��˹ضϹ⹦��AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm �ɹ�", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} */
		//�ص�Vapd
		if(bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "�ߵ�������Vapd��ѹ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd_HighLow (channel, &data);
			if (error)	{strcpy (Info, "����Vapd��ѹ��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "����Vapd��ѹ���ɹ�;APC_DAC=0x%X",(int)data.Vapd_DAC);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
			  
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode) && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�������ز���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "�������ز��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "����ONU�������ܲ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "����ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "����ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Check Over
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�����ز��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "���ز���=%.2fdBm ʧ��, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***���������Ȳ��Է����ڷ��˲�����Ŀǰ**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)
		{
			strcpy (Info, "���������Ȳ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "���������Ȳ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
//------------------------------------------------		
		//Tuning Wavelength 
		if (bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "������,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
		// er ��ͼ mask ���׵�  
		if (bOK)
		{
			strcpy (Info, "��Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
//-------------------------------------------------	
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room && !my_struConfig.DT_Test_Reliability) 
		{
			strcpy (Info, "����TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f ʧ��", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f �ɹ�", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && data.RetuningAopFlag )	/***�ص��⹦�ʺ󷢶�У׼***/
		{	
		 	strcpy (Info, "Re����У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "Re����У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "Re����У׼���ɹ�");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "����У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"����У׼���ԣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"����У׼���ԣ��ɹ�");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		if (bOK)
		{
			strcpy (Info, "����Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "����Ibias=%.2fmA ʧ��", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "����Ibias=%.2fmA �ɹ�", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// ��·����
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "��ʼ��·����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "��·���ԣ�ʧ��");bOK=FALSE;} 
			else	  {strcpy (Info, "��·���ԣ��ɹ�");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		// check sens
/*		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)        
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
			
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "����������ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //�ƽ�����SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //���Ʒ���������1.0e-12������SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}

		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "�ն�У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"�ն�У׼���ԣ�ʧ��");
				if (data.ErrorCode==0) data.ErrorCode=error;  //�����Ժ����Ĵ�����븳ֵ��error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"�ն�У׼���ԣ��ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "���ONU�������ܲ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "�Ⲩ��,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		if(data.WL_ReTun_FLAG)
		{
			// Test aop
/*			if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "����΢���󣬲��Թ⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testAOP(channel, &data);
				if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
				else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
*/
#if Debug
			/****************************��ʱͳ��********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestAOPʱ��=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************��ʱͳ��********************************/
#endif		
			//Test Er
/*			if (bOK)
			{
				strcpy (Info, "����΢���󣬲���Er,�л���·...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
				if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
			}	 
*/
#if Debug 
			/****************************��ʱͳ��********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestErʱ��=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************��ʱͳ��********************************/
#endif
			//Test TxOff Depth   
			if (bOK && my_struConfig.Sel_T_TxDis) 
			{
				strcpy (Info, "����΢���󣬲��Թض����...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testTxOff_Depth(channel,&data);
				if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
				else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
		// ɨ��ģ��SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "�����Ʒ������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"�����Ʒ������=%s ʧ��", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"�����Ʒ������=%s �ɹ�", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		// save eeprom
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "����EEPROM...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveEEPROM(channel, &data);
			if (error) 
			{
				strcpy (Info,"����EEPROM��ʧ��");
				data.ErrorCode=ERR_TES_SAVE_EEPROM; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"����EEPROM���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}  
/*		
		// ����
		if (bOK)
		{
		//	if(!bOK)
		//	{
		//		strcpy(temp_Info,Info);
		//	}
			strcpy (Info, "���Ե���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f ʧ��",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f �ɹ�",data.RxI[0],data.TxI[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(error<0)
			{
				strcat(temp_Info,Info);
			}
			if(!bOK)
			{
				strcpy(Info,temp_Info); 
			}
		}
		
		//����Burnin״̬��
		if (bOK && my_struConfig.Temper_Room && my_struConfig.DT_Test_Reliability)    		//�ɿ��Բ��Ժ���Burn in���ܣ�	
		{
			strcpy (Info, "����Burnin״̬...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Burnin_Enable(channel);
			if (error) 
			{
				strcpy (Info,"����Burnin״̬��ʧ��");
				data.ErrorCode=ERR_TUN_SetBurnin; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"����Burnin״̬���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
*/		
		if ((CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		
		//	save_EEPROM (channel, &data);
		}
		else if ((CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328S_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		} 
	
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) 
			{
				data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
			strcat (prolog.Comments, data.APD_Info);  
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "����������ݣ�ʧ��");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("��ʾ", buf);
			}  
			else	   
			{
				strcpy (Info, "����������ݣ��ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("��ʾ", "δ����������ݣ�����"); 
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//���ģ���Ƿ��ڲ��԰���
/*		if (bMOD0)
		{
			strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "�����԰��Ƿ���ģ������쳣");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//���ò���ͨ��1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//���ò���ͨ�� 1
//	DWDM_ONU_Select_Channelindex(channel,0); 
	
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK tuning_Reliability (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  OverTime; 
	int  SensTime; 
	
	char buf[256] = "";  
	int senstime_olt;  

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.DT_Tun_Reliability) 
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TunReliability]);  
	}
	else
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Tuning]);
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־     	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);   
				Delay(0.1);
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}

		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    "); 	
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���е���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}		    
		
		data.Time_Start=Timer();	
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "��ʼ��ʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��鹤��
		if (bOK && !my_struConfig.DT_Tun_Reliability && !my_struConfig.DT_Test_Reliability)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "ģ���·��ʧ��");
				}
				else
				{
					sprintf (Info, "���Թ���ʧ��");       
					data.ErrorCode=ERR_TUN_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "���Թ��� �ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD_Detect�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "���Է���SD_Detect�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "���Է���SD_Detect�źţ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		// ��·����
		if (bOK && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "��ʼ��·����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "��·���ԣ�ʧ��");bOK=FALSE;} 
			else	  {strcpy (Info, "��·���ԣ��ɹ�");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//test SD		
		if (bOK && CHIEFCHIP_DS4830!=my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "����SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testSD (channel);
			if (error) {strcpy (Info, "����SD�źţ�ʧ��");bOK=FALSE;}  
			else	   {strcpy (Info, "����SD�źţ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK)
		{
			strcpy (Info, "����Vapd��ѹ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd (channel, &data);
			if (error)	{strcpy (Info, "����Vapd��ѹ��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "����Vapd��ѹ���ɹ�;APD_DAC=0x%X",(int)data.Vapd_DAC);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;												 
		sprintf(Info, "TunVAPDʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//test SDA SDD	
		if (bOK && my_struConfig.Sel_R_SDHys) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSD(channel,&data);
			if (error){sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD); bOK=FALSE; data.ErrorCode=ERR_TES_SD;} 
			else	  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode)) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestSDʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�������ز���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "�������ز��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/						 
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestOverStartʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//��ѡ��ʾ����ʱѡ�ô˷���
		if (INSTR[channel].DCA_TYPE != DCA_TYPE_NONE)
		{
			//AOP
			if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "���Թ⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP(channel, &data);
				if (error) 
				{
					sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);
					bOK=FALSE;
					if (data.ErrorCode==0) 
					{
						data.ErrorCode=ERR_TUN_AOP; 
					}
				}
				else	  	
				{
					sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm�ɹ�", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);  
				}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
		}
		else
		{
			//AOP & ER
			if (bOK && my_struConfig.Sel_T_AOP && my_struConfig.Sel_T_ER && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "��������Ⱥ͹⹦��...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP_ER(channel, &data);
				if (error) {sprintf (Info, "��������Ⱥ͹⹦��=%.2fdBm FAIL", data.AOP);bOK=FALSE;}
				else	   {sprintf (Info, "��������Ⱥ͹⹦��=%.2fdBm PASS", data.AOP);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunAOPʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//�����ز��Խ��
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�����ز��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "���ز���=%.2fdBm ʧ��, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 #if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "OverCheckʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//test sens  �����Ȳ���ʱ������Բ�����ER�����Թ��׸���	  	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "���������Ȳ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "���������Ȳ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenTestStartʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Test Sen Detail
		if (bOK && my_struConfig.Sel_R_Sens && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail)) ) 
		{
			strcpy (Info, "����������ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //�ƽ�����SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //���Ʒ���������1.0e-12������SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Test Sen Detailʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Tuning Wavelength 01
		if (bOK)
		{
			strcpy (Info, "������01,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength01(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 01ʱ��=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/	  
#endif
		// Tun Er, ��ͼ mask ���׵�  
		if (bOK)
		{
			strcpy (Info, "��Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunErʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//Tuning Wavelength  02
		if (bOK)
		{
			strcpy (Info, "������02,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength02(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 02ʱ��=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/	
#endif
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
#if Debug 
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunTxOffʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//Sen Check	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenCheckʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))	/***����У׼����Ϊ��ѡ��**Eric.Yao**20131203***/
		{	
		 	strcpy (Info, "����У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "����У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "����У׼���ɹ�");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
#if Debug	
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TxCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//calRxPower
	 	if (bOK && my_struConfig.Sel_Calibrate)
		{	
		 	strcpy (Info, "�ն�У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = calRxPower(channel, &data, panMain);
			if (error) {sprintf (Info, "�ն�У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_RXPOWER;} 
			else	   {sprintf (Info, "�ն�У׼���ɹ�");} 	      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "RxCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/   
#endif	
#if Debug
  		//��֤����ǰ��AOP,Er,WL��TxOffPW�仯�Ա��ã�����ʹ����رգ�
		// Test aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestAOPʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
		
		//Test Er
		if (bOK)
		{
			strcpy (Info, "����Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestErʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/

		//Test TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestTxOffʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif 		
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "�Ⲩ��,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestWLʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif		
		//Cal temperature
		if (bOK)
		{
			strcpy (Info, "ģ���¶�У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = CalTemperature(channel);
			if (error<0) {sprintf (Info, "ģ���¶�У׼: ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TEMPERATURE;} 
			else	   {sprintf (Info, "ģ���¶�У׼���ɹ�");}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TempCalʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		
#if 0	//Ibias Max�����ɿ�������·���ñ������� ��Ϊ ����AOP���ô���6dBm���쳣���ƣ������ϻ��������ã� 2017-02-23  wenyao.xi
		// �������Ibias�޶�ֵ
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "�������Ibias�޶�ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Ibias_Max_Ex(INST_EVB[channel]);
			if (error) 
			{
				strcpy (Info,"�������Ibias�޶�ֵ��ʧ��");
				data.ErrorCode=ERR_TES_SetIbiasMax; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"�������Ibias�޶�ֵ���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		/****************************��ʱͳ��********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SetImaxʱ��=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************��ʱͳ��********************************/
#endif
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (prolog.P_Value, "PASS");
			strcpy (data.Status, "PASS"); 
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			strcpy (prolog.P_Value, "FAIL");
			strcpy (data.Status, "FAIL");
			strcpy (prolog.Comments, Info);
			strcat (prolog.Comments,data.APD_Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//���ò���ͨ��1
//	DWDM_ONU_Select_Channelindex(channel,0);
	
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK test_Reliability (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  SensTime,OverTime;
	int senstime_olt;   

	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.Temper_Room)
	{
		if (my_struConfig.DT_Test_Reliability) 
		{
			strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TestReliability]); 
		}
		else
		{
			strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing]);
		}
	}
	else if (my_struConfig.Temper_Low)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing_L]);   
	}
	else if (my_struConfig.Temper_High)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_Testing_H]);   
	}
	else
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣");	
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    ");
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��鹤��
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "ģ���·��ʧ��");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "�⹦�ʳ����ޣ�ʧ��"); 
				}
				else
				{
					sprintf (Info, "���Թ���ʧ��");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "���Թ��� �ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD_Detect�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "���Է���SD_Detect�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "���Է���SD_Detect�źţ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		} 
		
		//test TxSD
/*		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "���Է���SD�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "���Է���SD�źţ��ɹ�");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
*///������Ŀ�ظ���ȡ�� 2017-02-25			   
		//TxDisable
/*		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Է��˹ضϹ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "���Է��˹ضϹ⹦��AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm �ɹ�", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Է��˹ضϹ⹦��AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm �ɹ�", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} */
		//�ص�Vapd
		if(bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "�ߵ�������Vapd��ѹ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd_HighLow (channel, &data);
			if (error)	{strcpy (Info, "����Vapd��ѹ��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "����Vapd��ѹ���ɹ�;APC_DAC=0x%X",(int)data.Vapd_DAC);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
			  
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode) && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�������ز���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "�������ز��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "����ONU�������ܲ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "����ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "����ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Check Over
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "�����ز��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "�������ز��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "���ز���=%.2fdBm ʧ��, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "���ز���=%.2fdBm �ɹ�", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***���������Ȳ��Է����ڷ��˲�����Ŀǰ**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)
		{
			strcpy (Info, "���������Ȳ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "���������Ȳ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
//------------------------------------------------		
		//Tuning Wavelength 
		if (bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "������,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
		// er ��ͼ mask ���׵�  
		if (bOK)
		{
			strcpy (Info, "��Er,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
//-------------------------------------------------	
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room && !my_struConfig.DT_Test_Reliability) 
		{
			strcpy (Info, "����TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f ʧ��", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f �ɹ�", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && data.RetuningAopFlag )	/***�ص��⹦�ʺ󷢶�У׼***/
		{	
		 	strcpy (Info, "Re����У׼...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "Re����У׼��ʧ��");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "Re����У׼���ɹ�");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "����У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"����У׼���ԣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"����У׼���ԣ��ɹ�");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		if (bOK)
		{
			strcpy (Info, "����Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "����Ibias=%.2fmA ʧ��", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "����Ibias=%.2fmA �ɹ�", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// ��·����
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "��ʼ��·����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "��·���ԣ�ʧ��");bOK=FALSE;} 
			else	  {strcpy (Info, "��·���ԣ��ɹ�");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		// check sens
/*		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)        
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
			
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "����������ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //�ƽ�����SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //���Ʒ���������1.0e-12������SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}

		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "�ն�У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"�ն�У׼���ԣ�ʧ��");
				if (data.ErrorCode==0) data.ErrorCode=error;  //�����Ժ����Ĵ�����븳ֵ��error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"�ն�У׼���ԣ��ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "���ONU�������ܲ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "�Ⲩ��,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		// ɨ��ģ��SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "�����Ʒ������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"�����Ʒ������=%s ʧ��", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"�����Ʒ������=%s �ɹ�", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		// save eeprom
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "����EEPROM...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveEEPROM(channel, &data);
			if (error) 
			{
				strcpy (Info,"����EEPROM��ʧ��");
				data.ErrorCode=ERR_TES_SAVE_EEPROM; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"����EEPROM���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}  
/*		
		// ����
		if (bOK)
		{
		//	if(!bOK)
		//	{
		//		strcpy(temp_Info,Info);
		//	}
			strcpy (Info, "���Ե���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f ʧ��",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f �ɹ�",data.RxI[0],data.TxI[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(error<0)
			{
				strcat(temp_Info,Info);
			}
			if(!bOK)
			{
				strcpy(Info,temp_Info); 
			}
		}
		
		//����Burnin״̬��
		if (bOK && my_struConfig.Temper_Room && my_struConfig.DT_Test_Reliability)    		//�ɿ��Բ��Ժ���Burn in���ܣ�	
		{
			strcpy (Info, "����Burnin״̬...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Burnin_Enable(channel);
			if (error) 
			{
				strcpy (Info,"����Burnin״̬��ʧ��");
				data.ErrorCode=ERR_TUN_SetBurnin; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"����Burnin״̬���ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
*/		
		if ((CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		
		//	save_EEPROM (channel, &data);
		}
		else if ((CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328S_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		} 
	
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) 
			{
				data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
			strcat (prolog.Comments, data.APD_Info);  
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "����������ݣ�ʧ��");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("��ʾ", buf);
			}  
			else	   
			{
				strcpy (Info, "����������ݣ��ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("��ʾ", "δ����������ݣ�����"); 
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//���ģ���Ƿ��ڲ��԰���
/*		if (bMOD0)
		{
			strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "�����԰��Ƿ���ģ������쳣");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//���ò���ͨ��1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//���ò���ͨ�� 1
//	DWDM_ONU_Select_Channelindex(channel,0); 
	
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK Test_Upstream (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int SensTime;
	int senstime_olt=0;   
	int count;

	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.Temper_Room)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TestUpstream]);  
	}
	else if (my_struConfig.Temper_Low)
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣"); 
	}
	else if (my_struConfig.Temper_High)
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣");
	}
	else
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣");	
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "������ն˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    ");
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��鹤��
		//��鹤��
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "ģ���·��ʧ��");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "�⹦�ʳ����ޣ�ʧ��"); 
				}
				else
				{
					sprintf (Info, "���Թ���ʧ��");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "���Թ��� �ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		if (bOK && my_struConfig.Sel_T_Upstream)  
		{
			strcpy (Info, "����ONU�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			count = 0;
			do
			{
				error = test_olt_errbit_start(channel, &senstime_olt, buf);
				count++;
			}while(error<0 && count<3);
			if (error) {sprintf (Info, "����ONU����������ԣ�ʧ�� %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "����ONU����������ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
		if (bOK && my_struConfig.Sel_T_Upstream) 
		{
			strcpy (Info, "���ONU����������Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU����������ԣ�ʧ�� %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU����������ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// ɨ��ģ��SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "�����Ʒ������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"�����Ʒ������=%s ʧ��", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"�����Ʒ������=%s �ɹ�", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) 
			{
				data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "����������ݣ�ʧ��");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("��ʾ", buf);
			}  
			else	   
			{
				strcpy (Info, "����������ݣ��ɹ�");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("��ʾ", "δ����������ݣ�����"); 
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//���ģ���Ƿ��ڲ��԰���
/*		if (bMOD0)
		{
			strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "�����԰��Ƿ���ģ������쳣");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//���ò���ͨ��1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//���ò���ͨ�� 1
//	DWDM_ONU_Select_Channelindex(channel,0); 
	
	SET_EVB_SHDN(channel, 0);
	return error;
  }  

int CVICALLBACK QAtest (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500],temp_Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  SensTime; 
	int senstime_olt;             
	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	if (my_struConfig.Temper_Room)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_QA_Testing]);
	}
	else if (my_struConfig.Temper_Low)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_QA_Testing_L]);   
	}
	else if (my_struConfig.Temper_High)
	{
		strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_TRX_QA_Testing_H]);   
	}
	else
	{
		MessagePopup("��ʾ", "�����¶�ѡ���쳣");	
	}
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//����·У׼�͵���Ƿ����
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("��ʾ", "����з��˹�·У׼    "); 
				}
			}
			else
			{
				 MessagePopup("��ʾ", "����з��˹�·У׼    ");
				 
			}
		}
	
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "����ģ���¶�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "����ģ���¶�=%.2f�� ʧ��", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "����ģ���¶�=%.2f�� �ɹ�", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD_Detect�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "���Է���SD_Detect�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "���Է���SD_Detect�źţ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		}
		
		//test SD
		if (bOK && my_struConfig.Sel_R_SDHys)     
		{
			strcpy (Info, "�����ն�SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			//error = testSD(channel);
			error = testFixSD(channel,&data);  
			if (error)  {strcpy (Info, "�����ն�SD�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_SD;} 
			else		{strcpy (Info, "�����ն�SD�źţ��ɹ�");}	          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "���Է���SD�ź�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "���Է���SD�źţ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "���Է���SD�źţ��ɹ�");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
		
/*		//TxDisable
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Է��˹ضϹ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "���Է��˹ضϹ⹦��=%.2fdBm ʧ��", data.TxOffPower);bOK=FALSE;} 
			else	  {sprintf (Info, "���Է��˹ضϹ⹦��=%.2fdBm �ɹ�", data.TxOffPower);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		//over
		if (bOK && my_struConfig.Sel_R_Over)
		{
			strcpy (Info, "���Թ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver(channel, &data,&prolog);
			if (error){sprintf (Info, "���Թ���=%.2fdBm ʧ��", data.Over);bOK=FALSE;data.ErrorCode=ERR_TES_OVER;} 
			else	  {sprintf (Info, "���Թ���=%.2fdBm �ɹ�", data.Over);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***���������Ȳ��Է����ڷ��˲�����Ŀǰ**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens)
		{
			strcpy (Info, "���������Ȳ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "���������Ȳ��ԣ�ʧ��, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "���������Ȳ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm ʧ��",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm �ɹ�",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room) 
		{
			strcpy (Info, "����TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f ʧ��", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f �ɹ�", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "����У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"����У׼���ԣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"����У׼���ԣ��ɹ�");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "����ONU�������ܲ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "����ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "����ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "���Թض����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "���Թض���� ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "���Թض���� �ɹ� TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "�Ⲩ��,�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}
		// er ��ͼ mask ���׵�  
		if (bOK)
		{
			strcpy (Info, "�л���·...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //�˴�����Ҫ��ʾ��ؽ����testParameterLock�����Ѿ���ʾ 
		}	 
		
		if (bOK)
		{
			strcpy (Info, "����Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "����Ibias=%.2fmA ʧ��", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "����Ibias=%.2fmA �ɹ�", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// ��·����
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room)
		{
			strcpy (Info, "��ʼ��·����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "��·���ԣ�ʧ��");bOK=FALSE;} 
			else	  {strcpy (Info, "��·���ԣ��ɹ�");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 /*
		// ����
		if (1)
		{
			if(!bOK)
			{
				strcpy(temp_Info,Info);
			}
			strcpy (Info, "���Ե���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f ʧ��",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "�����ն˵���=%.2f ���˵���=%.2f �ɹ�",data.RxI[0],data.TxI[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(error<0)
			{
				strcat(temp_Info,Info);
			}
			if(!bOK)
			{
				strcpy(Info,temp_Info); 
			}
		} 
*/		
		// check sens
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)        
		{
			strcpy (Info, "��������Ȳ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "����������ֵ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //�ƽ�����SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //���Ʒ���������1.0e-12������SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "�����Ȳ���=%.2fdBm ʧ��", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "�����Ȳ���=%.2fdBm �ɹ�", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
	
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "����SDA��SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm ʧ��", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"����SDA=%.2fdBm SDD=%.2fdBm �ɹ�", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "�ն�У׼����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"�ն�У׼���ԣ�ʧ��");
				if (data.ErrorCode==0) data.ErrorCode=error;  //�����Ժ����Ĵ�����븳ֵ��error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"�ն�У׼���ԣ��ɹ�");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "���ONU�������ܲ��Խ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU�������ܲ��ԣ�ʧ�� %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU�������ܲ��ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//eeprom protect check
		if (bOK && my_struConfig.Sel_EE_P && CHIEFCHIP_UX3328 != my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "EEPROMд��������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = ee_protect_check(channel);
			if (error) {strcpy (Info, "EEPROMд�������ԣ�ʧ��");bOK=FALSE;}  
			else	   {strcpy (Info, "EEPROMд�������ԣ��ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
/*	
		//update BurstMode
		if (bOK && my_struDBConfig_ERCompens.DriverChip==DiverType_VSC7965)
		{
			strcpy (Info, "����BurstModeģʽ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = UpdateBurstMode(channel);
			if (error) {strcpy (Info, "����BurstModeģʽ��ʧ��");bOK=FALSE;}  
			else	   {strcpy (Info, "����BurstModeģʽ���ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}	
		//update WorkMode
		if (bOK && my_struDBConfig_VSC7965.UpdatefOVERDFlag)
		{
			strcpy (Info, "����VSC7967����ģʽ...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = UpdateWorkMode(channel);
			if (error)	{strcpy (Info, "����VSC7967����ģʽ��ʧ��");bOK=FALSE;}   
			else	    {strcpy (Info, "����VSC7967����ģʽ���ɹ�");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/	
		if (my_struConfig.isQAUpdateSN)
		{				  
			// ɨ��ģ��SN
			if (bOK && my_struConfig.Temper_Room)
			{
				strcpy (Info, "�����Ʒ������...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = GetModuleSN(channel, &data, panSN);
				if (error) 
				{
					sprintf (Info,"�����Ʒ������=%s ʧ��", data.ModuleSN);
					data.ErrorCode=ERR_TES_INPUT_SN; 
					bOK=FALSE; 
				}  
				else	   
				{
					sprintf (Info,"�����Ʒ������=%s �ɹ�", data.ModuleSN);
				}         
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
		
			// save eeprom
			if (bOK && my_struConfig.Temper_Room)    			
			{
				strcpy (Info, "����EEPROM...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = SaveEEPROM(channel, &data);
				if (error) 
				{
					strcpy (Info,"����EEPROM��ʧ��");
					data.ErrorCode=ERR_TES_SAVE_EEPROM; 
					bOK=FALSE; 
				}  
				else	   
				{
					strcpy (Info,"����EEPROM���ɹ�");
				}         
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}	
		}
/*		
		if ((CHIEFCHIP_UX3328 == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		
		//	save_EEPROM (channel, &data);
		}
		else if ((CHIEFCHIP_UX3328S == my_struDBConfig_ERCompens.ChiefChip) && (ERR_UX3328_E2_CHECK != data.ErrorCode)) 
		{
			error = UX3328S_UpdateA2CheckSum (channel, &data, panMain, Info);
			if (error) 	
			{
				bOK=FALSE;
			}  
		} 
 */
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//���ģ���Ƿ��ڲ��԰���
/*		if (bMOD0)
		{
			strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				if (error) 
				{
					strcpy (Info, "�����԰��Ƿ���ģ������쳣");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		}  */

		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK testAginFront (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  SensTime; 
	int senstime_olt;             
	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_testAginFront]);
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		
		// AOP
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP_AginFront(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��=%.2fdBm ʧ��",data.AOP[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��=%.2fdBm �ɹ�",data.AOP[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// WaveLenhth
		if (bOK && my_struConfig.Sel_T_Wavelength && (SPECTRUM_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Բ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testWaveLength_AginFront(channel, &data);
			if (error){sprintf (Info, "���Բ���=%.2f nm ʧ��",data.PeakWL_DutyRatio99[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Բ���=%.2f nm �ɹ�",data.PeakWL_DutyRatio99[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	SET_EVB_SHDN(channel, 0);
	return error;
}

int CVICALLBACK testAginBack (void *pvFuncData)
{
	int channel;
	struct struTestData data;
	struct struProcessLOG prolog;
	int	timestart, cnt;
	char Info[500];
	BOOL SaveData=FALSE, bOK=TRUE, bMOD0; 
	int error=0;
	int  SensTime; 
	int senstime_olt;             
	
	char buf[256] = "";

	/* 	Get the index corresponding to the thread */
	channel = *(int *)pvFuncData;
	
	/*	Get the address of the thread local variable */
	CmtGetThreadLocalVar (ghThreadLocalVar, &data);
	CmtGetThreadLocalVar (ghThreadLocalVar, &prolog); 
	
	ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
	
	strcpy (prolog.Log_Action, ProcessTypeArray[PROCESS_TYPE_testAginBack]);
	WaitFlag[channel] = FALSE;   // ������ʾ��Ϣˢ�±�־   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//���ģ���Ƿ��ڲ��԰���
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "�ȴ�������ͨ������ϵͳ...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//�����������
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//������԰��Ƿ���ģ���ʾ 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "ģ���ϵ�...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//�ȴ��ϵ���ȶ�
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//��ʼ����
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "��ʼ���в���...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//���ҪҪ���TSSI��TXSD���ȹرյ�Դ
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "����TxSD_Detect���Թ��ܣ�ʧ��");bOK=FALSE;}  
				else	    {strcpy (Info, "����TxSD_Detect���Թ��ܣ��ɹ�");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//���¿���
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//��ʼ������ģ��
		if (bOK)
		{
			strcpy (Info, "��ʼ��ģ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "��ʼ��ģ�飺ʧ��");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "��ʼ��ģ�飺�ɹ�");
			}                      										   
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "��ȡ����...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "��ȡ����=%s ʧ��", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "��ȡ����=%s �ɹ�", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP_AginBlack(channel, &data);
			if (error){sprintf (Info, "���Թ⹦��=%.2fdBm ʧ��",data.AOP[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Թ⹦��=%.2fdBm �ɹ�",data.AOP[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		// WaveLenhth
		if (bOK && my_struConfig.Sel_T_Wavelength && (SPECTRUM_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Բ���...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testWaveLength_AginBlack(channel, &data);
			if (error){sprintf (Info, "���Բ���=%.2f nm ʧ��",data.PeakWL_DutyRatio99[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "���Բ���=%.2f nm �ɹ�",data.PeakWL_DutyRatio99[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tun AOP ȷ��Imax DAC
		if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "���Թ⹦��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningAOP_AginBack(channel, &data);
			if (error) 
			{
				sprintf (Info, "���Թ⹦��Aop4=%.2fdBm ʧ��", data.AOP[3]);
				bOK=FALSE;
				if (data.ErrorCode==0) 
				{
					data.ErrorCode=ERR_TUN_AOP; 
				}
			}
			else	  	
			{
				sprintf (Info, "���Թ⹦��Aop4=%.2fdBm�ɹ�", data.AOP[3]);  
			}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//������־��ֵ
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //���ǲ��Թ����н���'��¼'������my_struLicense.username ������ʸ�Ϊ�����ȡ�� wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  �ڲ��Ժ�����ʼλ�ø�ֵ   
		strcpy (prolog.Action_Time, data.TestDate);	//Action_time
		strcpy (prolog.Parameter, "Status");		//Parameter
		if (bOK)
		{
			strcpy (data.Status, "PASS");
			strcpy (prolog.P_Value, "PASS");
			strcpy (prolog.Comments, "");
		}
		else
		{
			if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			strcpy (data.Status, "FAIL");
			strcpy (prolog.P_Value, "FAIL");
			strcpy (prolog.Comments, Info);
		}											 
		strcpy (prolog.SoftVersion, SOFTVER); 
		sprintf (prolog.StationID, "%s-%d", my_struProcessLOG.StationID, channel); 
		prolog.RunTime=Timer()-timestart;
		//�����������
		if (SaveData)
		{
			strcpy (Info, "�����������...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "����������ݣ�ʧ��");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "����������ݣ��ɹ�");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//��ʾ���н��
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//�����Գɹ�����ʾ�������
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d��", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "��������ʱ��=%ds��\n������ʹ�ô���=%d�Σ�\n�������=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//���ģ���Ƿ��ڲ��԰���
/*		if (bMOD0)
		{
			strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				if (error) 
				{
					strcpy (Info, "�����԰��Ƿ���ģ������쳣");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "�뽫����ģ��Ӳ���ϵͳȡ��...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		}  */

		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "��ͨ��������ֹͣ");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	SET_EVB_SHDN(channel, 0);
	return error;
}


int CVICALLBACK On_About_EXIT (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			HidePanel (panAbout);
			break;
	}
	return 0;
}

void CVICALLBACK On_About (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	DisplayPanel (panAbout);
}

int CVICALLBACK On_P (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	int val;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal (panel, control, &val); 
			
			 switch (control)
			 {
				 case PAN_MAIN_BIN_CHAN0:
					 channel=0;
					 break;
				 case PAN_MAIN_BIN_CHAN1:
					 channel=1; 
					 break;
				 case PAN_MAIN_BIN_CHAN2:
					 channel=2; 
					 break;
				 case PAN_MAIN_BIN_CHAN3:
					 channel=3; 
					 break;
				 case PAN_MAIN_BIN_CHAN4:
					 channel=4; 
					 break;
				 case PAN_MAIN_BIN_CHAN5:
					 channel=5; 
					 break;
				 case PAN_MAIN_BIN_CHAN6:
					 channel=6; 
					 break;
				 case PAN_MAIN_BIN_CHAN7:
					 channel=7; 
					 break;
					 
				 default:
					 MessagePopup ("��ʾ", "��ͨ����Ч");
					 break;
			 }
			
			 SET_EVB_SHDN (channel,val);
			 
			break;
	}
	return 0;
}

int CVICALLBACK On_Search_Clock (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			MyDLL_GET_SEVB_SN (SEVB_TYPE_EVB27_4001, panel, PAN_INST_RING_CLOCK); 
			
			break;
	}
	return 0;
}

void CVICALLBACK On_Advance (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	MyDLL_License_Load ();
	showphID = License_Flag_panADV;
}

int CVICALLBACK On_Advance_App (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panAdva, PAN_ADVA_IS_NPI, &my_struConfig.isNPI); 
			GetCtrlVal (panAdva, PAN_ADVA_IS_ONUTOOLT, &my_struConfig.isONUtoOLT); 
			GetCtrlVal (panAdva, PAN_ADVA_IS_QAUPDATESN, &my_struConfig.isQAUpdateSN); 
			GetCtrlVal (panAdva, PAN_ADVA_IS_QANOCHECKBOM, &my_struConfig.isQANoCheckBOM);
			GetCtrlVal (panAdva, PAN_ADVA_IS_QANOCHECKBOM, &my_struConfig.isScanModuleSN); 
		
			GetCtrlVal (panAdva, PAN_ADVA_NUMERIC_Agin_AOP, &my_struConfig.AOP_AginDelta); 
			GetCtrlVal (panAdva, PAN_ADVA_NUMERIC_Agin_WL, &my_struConfig.WL_AginDelta); 
			GetCtrlVal (panAdva, PAN_ADVA_NUM_WLMeterOffset, &my_struConfig.WLMeterOffset); 
			//���浽�����ļ�
			SetConfig(); 

			break;
	}
	return 0;
}

int CVICALLBACK On_Advance_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panAdva);  
			
			break;
	}
	return 0;
}

void CVICALLBACK OnFiber (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error;
	int channel;
	char buf[256];
	
	errChk(GetConfig_Inst());
	
	DisplayPanel (panFiber);
	
	//for BERT
	for (channel=0;channel<CHANNEL_MAX;channel++)
	{
		if (INSTR[channel].ChannelEn)
		{
			SetTableRowAttribute (panFiber, PAN_FIBER_TABLE, channel+1, ATTR_CELL_DIMMED, 0);
			SetTableCellVal (panFiber, PAN_FIBER_TABLE, MakePoint (2, channel+1), INSTR[channel].Fiber); 
		}
		else
		{
			SetTableCellAttribute (panFiber, PAN_FIBER_TABLE, MakePoint (1, channel+1), ATTR_CELL_DIMMED, 1); 
			SetTableCellAttribute (panFiber, PAN_FIBER_TABLE, MakePoint (2, channel+1), ATTR_CELL_DIMMED, 1); 
		}
	}
	
Error:
	
}

int CVICALLBACK OnFiberOK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	char buf[256];
	int error;
	int i;
	struct struInstrument localInst;
	
	switch (event)
	{
		case EVENT_COMMIT:

			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				if (INSTR[channel].ChannelEn)
				{
					//��ȡ�������к�
					GetTableCellVal (panFiber, PAN_FIBER_TABLE, MakePoint (2, channel+1), buf);
					
					//��������б������ҪУ׼
					if (0!= stricmp(buf,INSTR[channel].Fiber)) 
					{
						//�����ˣ�ǿ��У׼�ն˺ͷ��˹�·
						sRxCal.flag[channel] = FALSE;
						my_struTxCal.flag[channel]=FALSE; 
					}
					strcpy (INSTR[channel].Fiber, buf);
				}
			}
			
			//��У׼�������浽�ļ�
			error = SetConfig_Inst ();
			if (error) return -1;
			
			//�˳�����
			HidePanel (panFiber);    
			
			//�����������豸�б�
			for (i=0; i<CHANNEL_MAX; i++)
			{
				localInst = INSTR[i]; 
				InsertTree (panMain, PAN_MAIN_TREE, &localInst, 1); 
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK OnFiberQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel (panFiber);  
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliTx_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliTx_Quit_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliRx_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliOLT_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_OLTSENS_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_SMC_Quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_SMC_Quit_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			HidePanel (panel);
			
			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &channel); 
			
			//Power off 
			if (SET_EVB_SHDN(channel, 0)<0) 
			{
				MessagePopup("Error", "Set Power OFF error!");
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliT_Table (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Point focus; 
	double Power;
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetActiveTableCell(panel, control, &focus);
			
            if (focus.x == 3)
			{
				//Getͨ��
				Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
				if (Get_Power(channel, &Power)<0) 
				{
					MessagePopup ("ERROR", "Read power meter error !"); 
					return -1;
				} 
				
				SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, focus.y), Power);
			}

			break;
	}
	return 0;
}

int CVICALLBACK On_CaliT_Table_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Point focus; 
	double Power;
	int channel;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetActiveTableCell(panel, control, &focus);
			
            if (focus.x == 3)
			{
				//Getͨ��
				Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
				if (Get_Power(channel, &Power)<0) 
				{
					MessagePopup ("ERROR", "Read power meter error !"); 
					return -1;
				} 
				
				SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, focus.y), Power);
			}

			break;
	}
	return 0;
}

int CVICALLBACK On_CaliTx_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, err, channel;
	float temp_cal, temp_st, temp, temp_delta=999.;
	char buf[1024];
	float val=999;
	
	switch (event)
	{
		case EVENT_COMMIT:

			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
			
			//================����У׼����=================//
			temp_delta=999.;
			for (i=0; i<3; i++)
			{
				GetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (1, i+1), &temp_st); 
				GetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, i+1), &temp_cal); 
		
				//�����С��ֵ
				temp = temp_st-temp_cal;
				if (temp<temp_delta)
				{
					temp_delta = temp;	
		
					if(temp_delta>my_struTxCal.power_max[channel] || temp_delta<my_struTxCal.power_min[channel])
					{
						sprintf (buf, "ͨ��%d��У׼�⹦�ʳ������÷�Χ    ", channel);
						MessagePopup("��ʾ", buf);
						return -1;
					}
				}
			}
			
			my_struTxCal.power[channel] = temp_delta;
			my_struTxCal.power_st[channel] = temp_st;

			my_struTxCal.power_array[channel*4+3] = temp_delta;
			my_struTxCal.power_array[channel*4+0] = temp_st;

			//����У׼��������ݿ�
			DB_save_txcali(channel); 
		
			//update Flag
			my_struTxCal.flag[channel]=TRUE;
			//================����У׼����=================// 
			//�ر�����ͨ����Դ    
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{   
				if (INSTR[channel].ChannelEn)
				{	 
					SET_EVB_SHDN(channel, 0);
				}
			}
			//��У׼�������浽�ļ�
			err = SetCaliConfigFile ();
			if (err) 
			{
				return -1;
			}
			//���У׼�����ʾ
			FillTableCellRange (phCalT, PAN_CALT_TABLE_T, MakeRect (1, 1, 3, 2), val);
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliTx_OK_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, err, channel;
	float temp_cal, temp_st, temp, temp_delta=999.;
	char buf[1024];
	float val=999;
	
	switch (event)
	{
		case EVENT_COMMIT:

			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, &channel); 
			
			//================����У׼����=================//
			temp_delta=999.;
			for (i=0; i<3; i++)
			{
				GetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), &temp_st); 
				GetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), &temp_cal); 
		
				//�����С��ֵ
				temp = temp_st-temp_cal;
				if (temp<temp_delta)
				{
					temp_delta = temp;	
		
					if(temp_delta>my_struTxCal.minpower_max[channel] || temp_delta<my_struTxCal.minpower_min[channel])
					{
						sprintf (buf, "ͨ��%d��У׼�⹦�ʳ������÷�Χ    ", channel);
						MessagePopup("��ʾ", buf);
						return -1;
					}
				}
			}
			
			my_struTxCal.minpower[channel] = temp_delta;
			my_struTxCal.minpower_st[channel] = temp_st;

			my_struTxCal.minpower_array[channel*4+3] = temp_delta;
			my_struTxCal.minpower_array[channel*4+0] = temp_st;

			//����У׼��������ݿ�
//			DB_save_txcali(channel); 
		
			//update Flag
			my_struTxCal.flag[channel]=TRUE;
			//================����У׼����=================// 
			//�ر�����ͨ����Դ   
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{
				if (INSTR[channel].ChannelEn) 
				{
				
					SET_EVB_SHDN(channel, 0);
				}
			}
			//��У׼�������浽�ļ�
			err = SetCaliConfigFile ();
			if (err) 
			{
				return -1;
			}
			//���У׼�����ʾ
			FillTableCellRange (phCalT_Min, PAN_CALT_M_TABLE_T_Min, MakeRect (1, 1, 3, 2), val);
			break;
	}
	return 0;
}



int CVICALLBACK On_CaliRx_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, err, channel;
	float temp_cal, temp_st, temp, temp_delta=999.;
	char buf[1024];
	float val=999.;
	
	switch (event)
	{
		case EVENT_COMMIT:

			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &channel); 
			
			//================�ն�У׼����=================//
			//�ж�У׼�����Ƿ�����
			GetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), &temp_cal);
			
			temp_cal= sRxCal.power_in[channel] - temp_cal;
			
			if(temp_cal>sRxCal.power_max[channel] || temp_cal<sRxCal.power_min[channel])
			{
				MessagePopup("��ʾ", "�ն�У׼�⹦�ʳ������÷�Χ��"); 
				return -1;
			}
	
			//�����ն�У׼ֵ
			sRxCal.power[channel] = temp_cal; 
			sRxCal.power_array[channel*4+3] = temp_cal;
	
			//update Flag
			sRxCal.flag[channel]=TRUE;
	
			err = SET_ATT_ONU(channel, sRxCal.power_in[channel]+temp_cal);
			if (err) 
			{
				MessagePopup("Error", "Set Att error!");
			}
			
			err = DB_save_rxcali (CALI_TYPE_RX, channel);
			if (err)
			{
				return -1;
			}
			//================�ն�У׼����=================//
			
			//��У׼�������浽�ļ�
			err = SetCaliConfigFile ();
			if (err)
			{
				return -1;
			}
		
			//���У׼�����ʾ
			FillTableCellRange (panCaliR, PAN_CALR_TABLE_R, MakeRect (1, 1, 1, 1), val);
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliOLT_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, err, channel;
	float temp_cal, temp_st, temp, temp_delta=999.;
	char buf[1024];
	float val=999;
	
	switch (event)
	{
		case EVENT_COMMIT:

			//��ȡͨ��
			Radio_GetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, &channel); 
			
			//================�ն�У׼����=================//
			//�ж�У׼�����Ƿ�����
			GetTableCellVal (panel, PN_CAL_OLT_TABLE_R, MakePoint (1, 1), &temp_cal);
			
			temp_cal= my_struCal_OLT.Power_In[channel] - temp_cal;
			
			if(temp_cal>my_struCal_OLT.Power_Max[channel] || temp_cal<my_struCal_OLT.Power_Min[channel])
			{
				MessagePopup("��ʾ", "�ն�У׼�⹦�ʳ������÷�Χ��"); 
				return -1;
			}
	
			//�����ն�У׼ֵ
		//	sRxCal.power[channel] = temp_cal; 
		//	sRxCal.power_array[channel*4+3] = temp_cal;
			//�����ն�У׼ֵ
			my_struCal_OLT.Power[channel] = temp_cal; 
			my_struCal_OLT.Array[channel*5+3] = temp_cal;
	
			//update Flag
			my_struCal_OLT.flag[channel]=TRUE;
	
			err = SET_ATT_OLT(channel, my_struCal_OLT.Power_In[channel]+temp_cal);
			if (err) 
			{
				MessagePopup("Error", "Set Att error!");
			}
			
			err = DB_save_rxcali (CALI_TYPE_RX_OLT, channel);
			if (err) 
			{
				return -1;
			}
			//================�ն�У׼����=================//
			
			//��У׼�������浽�ļ�
			err = SetCaliConfigFile ();
			if (err)
			{
				return -1;
			}
			//���У׼�����ʾ
			FillTableCellRange (phCalOLT, PN_CAL_OLT_TABLE_R, MakeRect (1, 1, 1, 1), val);
			//�رյ�ǰͨ����Դ
			err=SET_EVB_SHDN(channel, 0);
			if (err)
			{
				MessagePopup("��ʾ", "�ر�ģ���Դʧ��"); 
				return -1;
			}			
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_OLTSENS_OK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, err, channel;
	float temp_cal, temp_st, temp, temp_delta=999.;
	char buf[1024];
	
	switch (event)
	{
		case EVENT_COMMIT:

			//��ȡͨ��
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel); 
			
			GetCtrlVal (panel, PAN_CALOLT_NUM_SENS, &my_struCal_OLT.OLT_sens[channel]); 
			
			//�ж�У׼�����Ƿ�����
			if(my_struCal_OLT.OLT_sens[channel]>my_struCal_OLT.OLT_sens_max[channel] || my_struCal_OLT.OLT_sens[channel]<my_struCal_OLT.OLT_sens_min[channel])
			{MessagePopup("��ʾ", "OLT�����Ȳ���ֵ�������÷�Χ��"); return -1;}
			
			//����У׼ֵ����
	 		my_struCal_OLT.OLT_sens_arr[channel*5] = my_struCal_OLT.OLT_sens[channel];  
			
			//�˳�У׼����
			HidePanel (panel);    
			
			//Power off
			err = SET_EVB_SHDN(channel, 0); 
			if (err) {MessagePopup("Error", "Set Power OFF error!");} 
			
			//����OLT��·�����ļ�
//			my_struCal_OLT.Power_In[channel]=my_struCal_OLT.OLT_sens[channel]+my_struCal_OLT.OLT_Reserves[channel];
//			my_struCal_OLT.Array[channel*5+0] = my_struCal_OLT.Power_In[channel]; 
			
			//��У׼�������浽�ļ�
			err = SetCaliConfigFile ();
			if (err) return -1;
			
			//��ʱ�ر�����У׼����
	//		my_struCal_OLT.flag=FALSE;
	//		MessagePopup("��ʾ", "����������OLT���ܹ�·У׼");
			
			break;
	}
	return 0;
}

int CVICALLBACK On_CaliTx_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//��ǰͨ���л�
				channel=temp;   
				
				//������ǰͨ����Դ
				errChk(SET_EVB_SHDN(channel, 1));
				EVB5_SET_BEN_Level(INST_EVB[channel], 0);
				SET_ATT_ONU (channel, -60); 
				//���ص�ǰͨ����·У׼����
				for (i=0; i<3; i++)
				{
					SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
					SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
				}
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			}   
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PAN_CALT_RAD_CHAN, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}

int CVICALLBACK On_CaliTx_Chan_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//��ǰͨ���л�
				channel=temp;   
				
				//������ǰͨ����Դ
				errChk(SET_EVB_SHDN(channel, 1));
				SET_ATT_ONU (channel, -60); 
				if(EVB5_SET_BEN_Level(INST_EVB[channel], 1))
				{
					return -1;
				}
				//���ص�ǰͨ����·У׼����
				for (i=0; i<3; i++)
				{
					SetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
					SetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
				}
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			}   
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}



int CVICALLBACK On_CaliRx_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//��ǰͨ���л�
				channel=temp;   
				
				//������ǰͨ����Դ
	//			errChk(SET_EVB_SHDN(channel, 1));
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
				SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PAN_CALR_RAD_CHAN, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}

int CVICALLBACK On_CaliOLT_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//��ǰͨ���л�
				channel=temp;   
				
				//������ǰͨ����Դ
				errChk(SET_EVB_SHDN(channel, 1));
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
				SetTableCellVal (panel, PN_CAL_OLT_TABLE_R, MakePoint (1, 1), my_struCal_OLT.Power_In[channel]-my_struCal_OLT.Power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_OLT(channel, my_struCal_OLT.Power_In[channel]));
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}

int CVICALLBACK On_Cali_OLTSENS_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//��ǰͨ���л�
				channel=temp;   
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_IN, my_struCal_OLT.OLT_sens_in[channel]);
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MIN, my_struCal_OLT.OLT_sens_min[channel]); 
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MAX, my_struCal_OLT.OLT_sens_max[channel]); 
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS, my_struCal_OLT.OLT_sens[channel]);
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_RESERVES, my_struCal_OLT.OLT_Reserves[channel]); 
				
				//������ǰͨ����Դ
				errChk(SET_EVB_SHDN(channel, 1));
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
//				SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}

int CVICALLBACK On_Cali_SMC_Chan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int channel=0;
	int temp;
	int i;
	int error=0;
	char buf[256];
	
	switch (event)
	{
		case EVENT_COMMIT:
			//��ȡУ׼����
			errChk(GetCaliConfigFile());
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//�ر�����ͨ����Դ   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					SET_EVB_SHDN(channel, 0);
				}
			
				//��ǰͨ���л�
				channel=temp;   
				SetCtrlVal (phSMC, PAN_SMC_NUM_SENSVALUE, my_struCal_OLT.OLT_sens[channel]);
				SetCtrlVal (phSMC, PAN_SMC_NUM_SENSTIME, my_struCal_OLT.time[channel]);
				//������ǰͨ����Դ
				errChk(SET_EVB_SHDN(channel, 1));
				
				//���Թ���δ����
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "ͨ��%d û��ѡ��˥�����ͺţ����ܽ����ն�У׼    ", channel);
	//				MessagePopup("��ʾ", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
	//			SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, my_struCal_OLT.OLT_sens[channel]));
			}
			else
			{
				//Ĭ��ѡ���һ��ʹ�õ�ͨ��
				errChk(GetEnableChannel(&channel));
				MessagePopup ("��ʾ", "��ѡͨ��δ����    ");
				Radio_SetMarkedOption(panel, PAN_SMC_RAD_CHAN, channel);	
			}
			
			break;
	}

Error:	
	
	return error;
}

int CVICALLBACK On_Cali_OLTSENS_TEST (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	int error;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel);
			
			if (!my_struCal_OLT.flag && INSTR[channel].ATT_TYPE_OLT != ATT_TYPE_NONE) 
			{
				MessagePopup("��ʾ", "�������OLT���ܹ�·У׼");
				return -1;
			}  
			SET_EVB_SHDN(channel, 1); 
			 Delay(1.5);
			
			SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, 0.); 
			
			error = testDetailSens_OLT (channel, panel);
			if (error) {MessagePopup("Error", "����OLT�������쳣"); return -1;} 
			
			SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, my_struCal_OLT.OLT_sens[channel]); 
			SET_EVB_SHDN(channel, 0); 
			break;
	}
	return 0;
}

int CVICALLBACK On_Cali_SMC_Test (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel;
	BOOL status;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			//�ж�ͨ���Ƿ����
			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &channel);
			
			SetCtrlVal (panel, PAN_SMC_LED, 0);
			SetCtrlAttribute (panel, PAN_SMC_LED, ATTR_OFF_COLOR, VAL_BLACK); 
			
			SET_EVB_SHDN(channel, 1);
			
			status = testFixSens (channel, panel);
			
			SET_EVB_SHDN(channel, 0);
			
			SetCtrlVal (panel, PAN_SMC_LED, status); 
			SetCtrlAttribute (panel, PAN_SMC_LED, ATTR_ON_COLOR, VAL_GREEN); 
			SetCtrlAttribute (panel, PAN_SMC_LED, ATTR_OFF_COLOR, VAL_RED); 

			break;
	}
	return 0;
}

void CVICALLBACK On_Calibrate_R_OLT (int menuBar, int menuItem, void *callbackData,
		int panel)
{
		int error;
		int channel; 
		
		
	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalibrateOLT, PAN_CALOLT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));	
	
	if (INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_NONE)
	{
		MessagePopup("��ʾ", "û��ѡ��˥�����ͺţ����ܽ����ն�У׼��");
		return;
	} 
	
	//read rx cal file
	error = GetCaliConfigFile ();
	if (error) return ;
	
	DisplayPanel (phCalibrateOLT);
	
	SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_IN, my_struCal_OLT.OLT_sens_in[channel]);
	SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MIN, my_struCal_OLT.OLT_sens_min[channel]); 
	SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MAX, my_struCal_OLT.OLT_sens_max[channel]); 
	SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS, my_struCal_OLT.OLT_sens[channel]);
	SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_RESERVES, my_struCal_OLT.OLT_Reserves[channel]); 
	
Error:
	
}

void CVICALLBACK On_SMCheck (int menuBar, int menuItem, void *callbackData,
		int panel)
{
		int error;
		int channel; 
		
		
	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phSMC, PAN_SMC_RAD_CHAN, channel);
	DisplayPanel (phSMC); 

	SetCtrlVal (phSMC, PAN_SMC_NUM_SENSVALUE, my_struCal_OLT.OLT_sens[channel]);
	SetCtrlVal (phSMC, PAN_SMC_NUM_SENSTIME, my_struCal_OLT.time[channel]);
Error:
}


int CVICALLBACK On_Burst_CALOLT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			break;
	}
	return 0;
}
int CVICALLBACK On_Burst_CAL_OLT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			break;
	}
	return 0;
}
int CVICALLBACK On_Burst_CALR (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			break;
	}
	return 0;
}
int CVICALLBACK On_Burst_CALT (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			break;
	}
	return 0;
}

int CVICALLBACK On_Burst_CALT_Min (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			SET_ATT_ONU (channel, -60);	  	//����-60����     
			break;
	}
	return 0;
}

int CVICALLBACK On_Burst_SMC (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int channel,val;
	switch (event)
	{
		case EVENT_COMMIT:

			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &channel);
			GetCtrlVal (panel, control, &val); 
			if(EVB5_SET_BEN_Level(INST_EVB[channel], val))
			{
				return -1;
			}
			break;
	}
	return 0;
}
int CVICALLBACK Start_Channel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(gFlag)
			{
				if     (gCtrl_StartBut[0]==control)  Channel_Sel_Flag[0]=TRUE;
				else if(gCtrl_StartBut[1]==control)  Channel_Sel_Flag[1]=TRUE; 
				else if(gCtrl_StartBut[2]==control)  Channel_Sel_Flag[2]=TRUE; 
				else if(gCtrl_StartBut[3]==control)  Channel_Sel_Flag[3]=TRUE; 
				else 
				{
					;
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK On_Search_FSW (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			MyDLL_GET_SEVB_SN (SEVB_TYPE_SEVB5, panel, PAN_INST_RING_FSW_2); 
			
			break;
	}
	return 0;
}

void CVICALLBACK On_CalibrateT_Min (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int error, i;
	int channel=0;
	float val=999.0;
	char buf[256];
	
	errChk(GetCaliConfigFile()); 
	
	DisplayPanel (phCalT_Min); 

	//���У׼�����ʾ
	FillTableCellRange (phCalT_Min, PAN_CALT_M_TABLE_T_Min, MakeRect (1, 1, 3, 2), val);
	
	//Ĭ��ѡ���һ��ʹ�õ�ͨ��
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalT_Min, PAN_CALT_M_RAD_CHAN_Min, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	SET_ATT_ONU (channel, -60);
	EVB5_SET_BEN_Level(INST_EVB[channel], 1);

	//===================���ط���У׼��¼===================//
	for (i=0; i<3; i++)
	{
		SetTableCellVal (phCalT, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
		SetTableCellVal (phCalT, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
	}
	//===================���ط���У׼��¼===================//
	
Error:
}


int CVICALLBACK Demo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
//	double x[10]={1792,1840,1888,1936};
	double y[10];
//	double a[10],b[10],c;
	int order;
	char char_temp[1024];
	float x1,x2,f1,f2,x;  
		
	struct struTestData *data;
	
	switch (event)
	{
		case EVENT_COMMIT:
		   //-41.25,571.75,1841.75
		/*	
			memset(y,0,sizeof(y));
			memset(a,0,sizeof(a));  
			memset(b,0,sizeof(b));  
			
			y[0]=1536.16394982236;
			y[1]=1536.2339809144; 
			y[2]=1536.29503074625; 
			y[3]=1536.35699007281; 
			y[4]=1536.35699007281;  
			order=2;
			PolyFit(x,y,4,order,a,b,&c);
		 */

	break;
	}
	return 0;
}



void CVICALLBACK On_OSA_Manual (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	DisplayPanel(panOsaMan);
}

int CVICALLBACK CallBackHideOSAManual (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			HidePanel(panOsaMan); 
			break;
	}
	return 0;
}

int CVICALLBACK Callback_DAC0 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int power_on_flag = 0;
	double dac0 = 0;
	int error1 = -1;
	switch (event)
	{
		case EVENT_COMMIT:
			// �ж�ģ���Ƿ��ϵ磬û�ϵ����ϵ�
			// ��ʱ����ͨ���жϿؼ�����ȡ�ؼ�״̬���ж��ϵ�״̬����ʱĬ��0ͨ��
			GetCtrlVal(panel, PAN_MAIN_BIN_CHAN0, &power_on_flag);
			if (power_on_flag != 1)
			{
				MessagePopup("����", "��ǰģ��û���ϵ�!");
				return -1;
			}
			// ��������
			error1 = DWDM_ONU_DS4830_Enter_Password (INST_EVB[0]);
			if (error1) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			
			// ��ȡ���õ�DAC
			GetCtrlVal (panel, PAN_OSA_STRING_DAC0, &dac0);
			m_MyDACData[0] = dac0;
			// DAC��Ϣ�·���Ӳ��
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[0], dac0)<0) 
			{
				return -1;
			}
			Delay(0.2);
			// ��Ϊ��������Ϣ���������ĺ���Read_Wavelength
			break;
	}
	return 0;
}

int CVICALLBACK Callback_DAC1 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int power_on_flag = 0;
	double dac1 = 0;
	int error1 = -1;
	switch (event)
	{
		case EVENT_COMMIT:
			// �ж�ģ���Ƿ��ϵ磬û�ϵ����ϵ�
			// ��ʱ����ͨ���жϿؼ�����ȡ�ؼ�״̬���ж��ϵ�״̬����ʱĬ��0ͨ��
			GetCtrlVal(panel, PAN_MAIN_BIN_CHAN0, &power_on_flag);
			if (power_on_flag != 1)
			{
				MessagePopup("����", "��ǰģ��û���ϵ�!");
				return -1;
			}
			// ��������
			error1 = DWDM_ONU_DS4830_Enter_Password (INST_EVB[0]);
			if (error1) 
			{
				MessagePopup("����", "3328���ù���ģʽ����   "); 
				return -1;
			}
			
			// ��ȡ���õ�DAC
			GetCtrlVal (panel, PAN_OSA_STRING_DAC1, &dac1);
			m_MyDACData[1] = dac1;
			// DAC��Ϣ�·���Ӳ��
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[0], dac1)<0) 
			{
				return -1;
			}
			Delay(0.2);
			// ��Ϊ��������Ϣ���������ĺ���Read_Wavelength
			break;
	}
	return 0;
}

int CVICALLBACK Callback_DAC_Save (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double caliTemper[2];
	double caliWave[2];
	switch (event)
	{
		case EVENT_COMMIT:
			// ��m_MyDACData�����ݱ��浽���ݿ�
			GetCtrlVal (panel, PAN_OSA_STRING_TMPE0, &caliTemper[0]);
			GetCtrlVal (panel, PAN_OSA_STRING_TMPE1, &caliTemper[1]);
			GetCtrlVal (panel, PAN_OSA_STRING_WAVE0, &caliWave[0]);
			GetCtrlVal (panel, PAN_OSA_STRING_WAVE1, &caliWave[1]);
			break;
	}
	return 0;
}

void CVICALLBACK CallbackTest1 (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char buf[] = "select * from autodt_process_log where id = 209195038";
	// �������ݿ�
	int err;	
	err = MyDLL_DB_Init (&hdbc1); 
	if (err) 
	{
		MessagePopup("��ʾ","���ݿ��ʼ��ʧ�ܣ�");
		return ;
	}
	// �·���ѯ���
	err = DBActivateSQL(hdbc1,buf);
	if (err <= 0)
	{
		ShowDataBaseError();
		return;
	}
}
