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
static int panOrder;	//随工单录入界面 
static int panPopu;		//退出程序确认对话框 
static int panConf;     //功能配置
static int panCaliR;	//收端校准界面
static int panAbout;
static int panMenu; 
static int panAdva;     //高级功能界面 
static int panFiber;	//光纤序列号界面
static int phCalibrateOLT;
static int phSMC; 
static int phCalOLT;	//OLT接受光路校准界面
static int phCalT;
static int phCalT_Min; 
static int panOsaMan;   //手动校准OSA的面板句柄

int ThreadHandle;

int hdbc1 = 0; 
//多线程变量
static int ghThreadLocalVar;	//多线程本地变量
static int gFlag;				//多线程启动标志
static unsigned int gIndexVal[CHANNEL_MAX]     = {0, 1, 2, 3, 4, 5, 6, 7}; //多线程函数参数值,实际传入的是测试通道值
static int 			gThreadFuncId[CHANNEL_MAX] = {0, 0, 0, 0, 0, 0, 0, 0}; //多线程ID
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
//需要共享设备的参数测试，比如消光比、眼图、光谱等 
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
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//切换时钟
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
			MessagePopup("Error", "不支持该设备");
			error=-1;      
			goto Error;	
		}
		
		if (my_struConfig.DT_Tuning || my_struConfig.DT_Tun_Reliability)
		{
			 //ER
		 	if (bOK && my_struConfig.Sel_T_ER)
			{	
			 	strcpy (Info, "调试消光比...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
				error = tuningER(channel, data);
				if (error) 
				{
					sprintf (Info, "调试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 失败", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);     
					if (0==data->ErrorCode)
					{
						data->ErrorCode=ERR_TUN_ER;
					}
					bOK=FALSE;  
				}
				else	  	
				{   
					sprintf (Info, "调试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 成功", data->ER[0], data->ER[1], data->ER[2], data->ER[3]); 
				}  
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
			}

		}
		else if (my_struConfig.DT_Test || my_struConfig.DT_Test_Reliability)
		{    

			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "测试消光比...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 失败", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 成功", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//眼图
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "测试发端眼图...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "测试发端眼图:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "测试发端眼图:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//模板
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "测试发端模板...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//测具体值
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //测门限
				}	
				if (error) {sprintf (Info, "测试发端模板：失败");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "测试发端模板：成功");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//光谱
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "测试光谱...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "测试光谱:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "测试光谱:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}
		else if (my_struConfig.DT_QATest || my_struConfig.DT_OQA)
		{
			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "测试消光比...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 失败", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 成功", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//眼图
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "测试发端眼图...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "测试发端眼图:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "测试发端眼图:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//模板
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "测试发端模板...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//测具体值
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //测门限
				}	
				if (error) {sprintf (Info, "测试发端模板：失败");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "测试发端模板：成功");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//光谱
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "测试光谱...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "测试光谱:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "测试光谱:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}

		
Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
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
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//切换时钟
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
			MessagePopup("Error", "不支持该设备");
			error=-1;      
			goto Error;	
		}
		
		
	//	else if (my_struConfig.DT_Test || my_struConfig.DT_Test_Reliability)
		{    

			if (bOK && my_struConfig.Sel_T_ER)
			{
				strcpy (Info, "测试消光比...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testER(channel, data,prolog);
				if (error)
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 失败", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
					bOK=FALSE;
					if (0==data->ErrorCode)
					{
					    data->ErrorCode=ERR_TES_ER;
					}
				} 
				else	  
				{
					sprintf (Info, "测试消光比 Er1=%.2fdB Er2=%.2fdB Er3=%.2fdB Er4=%.2fdB 成功", data->ER[0], data->ER[1], data->ER[2], data->ER[3]);
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//眼图
			if (bOK && my_struConfig.Sel_T_Eye)
			{
				strcpy (Info, "测试发端眼图...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testOEye(channel, data);
				if (error)
				{
					strcpy (Info, "测试发端眼图:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_O_EYE;
				} 
				else	 
				{
					strcpy (Info, "测试发端眼图:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//模板
			if (bOK && my_struConfig.Sel_T_Mask)
			{
				strcpy (Info, "测试发端模板...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				if(my_struConfig.Sel_T_Mask_Real)		//测具体值
				{
					error = testOEye_Mask(channel, data);
				}
				else 
				{
					error = testOEye_Mask_Threshold(channel, data);		  //测门限
				}	
				if (error) {sprintf (Info, "测试发端模板：失败");bOK=FALSE;data->ErrorCode=ERR_TES_O_MASK;} 
				else	   {sprintf (Info, "测试发端模板：成功");}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
			//光谱
			if (bOK && my_struConfig.Sel_T_Spectrum)
			{
				strcpy (Info, "测试光谱...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testSpectrum(channel, data);
				if (error)
				{
					strcpy (Info, "测试光谱:失败");
					bOK=FALSE;
					data->ErrorCode=ERR_TES_SPECTRUM;
				} 
				else	 
				{
					strcpy (Info, "测试光谱:成功");
				}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}  
		}

		
Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
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
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE, INSTR[channel].SW, INSTR[channel].SW_Handle, INSTR[channel].SW_CHAN)<0)
			{
				return -1;
			}
		}
		
		//切换时钟
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
			MessagePopup("Error", "不支持该设备");
			error=-1;      
			goto Error;	
		}
		
		if (my_struConfig.DT_Tuning || my_struConfig.DT_Tun_Reliability)
		{
			 //ER
		 	if (bOK && my_struConfig.Sel_T_ER)
			{	Delay(2);
			 	strcpy (Info, "预调消光比...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
				error = tuningER_Beforehand(channel, data);
				if (error) 
				{
					sprintf (Info, "预调消光比 Er4=%.2fdB 失败", data->ER[3]);     
					if (0==data->ErrorCode)
					{
						data->ErrorCode=ERR_TUN_ER;
					}
					bOK=FALSE;  
				}
				else	  	
				{   
					sprintf (Info, "调试消光比 Er4=%.2fdB 成功", data->ER[3]); 
				}  
				Insert_Info(panMain, gCtrl_BOX[channel], Info);         
			}

		}

Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
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
		//切换光路
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
				strcpy (Info, "高温微调波长...");
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
				strcpy (Info, "低温微调波长...");
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
				strcpy (Info, "常温调试波长...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningWavelength(channel, data);

			}
		
			if (error) {sprintf (Info, "调试波长 失败");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "调试波长 成功 WL1=%f WL2=%f WL3=%f WL4=%f",data->Peakwavelength[0], data->Peakwavelength[1], data->Peakwavelength[2], data->Peakwavelength[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
		return -1;
	}

	return error;
}

int testParameterLock_TunWavelength01(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)		   //Room分步调试波长，第一步 
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		if (bOK && my_struConfig.Sel_T_Wavelength) 
		{
			strcpy (Info, "常温预调波长...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningWavelength01(channel, data);
			if (error) {sprintf (Info, "预调波长 失败");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "预调波长 成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
		return -1;
	}

	return error;
}

int testParameterLock_TunWavelength02(int channel, struct struTestData *data, struct struProcessLOG *prolog, int panel, int control, char *Info)	   //Room分步调试波长，第二步
{
	int error=0;
	int obtainedLock = 0;
	double delay;
	int bOK=TRUE;
//	char Info[500];
	
	GetLock(ThreadLock_AG86120, &obtainedLock); 
	if (obtainedLock)
    {
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		if (bOK && my_struConfig.Sel_T_Wavelength) 
		{
			strcpy (Info, "常温调试波长...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningWavelength02(channel, data);
			if (error) {sprintf (Info, "调试波长 失败");bOK=FALSE;data->ErrorCode=ERR_TUN_Wavelenth;} 
			else	   {sprintf (Info, "调试波长 成功 WL1=%f WL2=%f WL3=%f WL4=%f",data->Peakwavelength[0], data->Peakwavelength[1], data->Peakwavelength[2], data->Peakwavelength[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
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
		//切换光路
		if (SW_TYPE_NONE != INSTR[channel].SW_TYPE)     
		{
			if(Fiber_SW_SetChannel(INSTR[channel].SW_TYPE_2, INSTR[channel].SW_2, INSTR[channel].SW_Handle_2, INSTR[channel].SW_CHAN_2)<0)
			{
				return -1;
			}
		}

		//波长
		if (bOK && my_struConfig.Sel_T_Wavelength)
		{
			strcpy (Info, "测试波长...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.DT_Tuning) 
			{
			//	error = testWavelength_Tun(channel, data);
			//	sprintf (Info_temp, "测试10%%波长:成功 WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio10[0], data->PeakWL_DutyRatio10[1], data->PeakWL_DutyRatio10[2], data->PeakWL_DutyRatio10[3]);
				error = testWavelength(channel, data);  
				sprintf (Info_temp, "测试100%%波长:成功 WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio99[0], data->PeakWL_DutyRatio99[1], data->PeakWL_DutyRatio99[2], data->PeakWL_DutyRatio99[3]);
			}
			else
			{
				error = testWavelength(channel, data);  
				sprintf (Info_temp, "测试100%%波长:成功 WL1=%f WL2=%f WL3=%f WL4=%f",data->PeakWL_DutyRatio99[0], data->PeakWL_DutyRatio99[1], data->PeakWL_DutyRatio99[2], data->PeakWL_DutyRatio99[3]);
			}
			if (error)
			{
				strcpy (Info, "测试波长:失败");												   
				bOK=FALSE;
				data->ErrorCode=ERR_TES_Wavelenth;
			} 
			else	 
			{
				strcpy (Info, Info_temp);
			}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 

Error:	// 注意 只能在此函数块内部使用errChk宏	
        CmtReleaseLock(ThreadLock_AG86120);
    }
	else
	{
		Insert_Info(panel, control, "获取设备控制权失败    ");
		return -1;
	}

	return error;
}



//写EEPROM前扫描模块SN
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
			strcpy (Info, "输入产品序列码...");
			Insert_Info(panel, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, data, panSN);
			if (error) 
			{
				sprintf (Info,"输入产品序列码=%s 失败", data->ModuleSN);
				data->ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"输入产品序列码=%s 成功", data->ModuleSN);
			}         
			Insert_Info(panel, gCtrl_BOX[channel], Info); 
		}
		
Error:	// 注意 只能在此函数块内部使用errChk宏	
		CmtReleaseLock(ThreadLock_ModuleSN);
    }
	else
	{
		Insert_Info(panel, control, "获取模块SN扫描权失败    ");
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
		
	sprintf (version, "DWDM ONU并行测试软件\nEXP0000xxx\n%s/%s",SOFTVER,SOFTDATE);
    SetCtrlVal (panAbout, PAN_ABOUT_TEXTMSG, version); 

	//界面加载控件值
	for (i=0; i<CHANNEL_MAX; i++)
	{
		sprintf (buf, "%d", i); 
		SetTableCellVal (phCheckT, PAN_CHE_T_TABLE, MakePoint (1, i+1), buf);
		SetTableCellVal (panFiber, PAN_FIBER_TABLE, MakePoint (1, i+1), buf);
	}
	
	//校准参数门限值配置界面
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 1), "发端光路");
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 2), "收端光路");  
	SetTableCellVal (phCaliLimit, PAN_CALI_L_TABLE, MakePoint (1, 3), "OLT收端光路");
	
	showphID = License_Flag_INIT; 
	
	if (!MyDLL_License_Load()) return -1;

	/* Create the thread local variable */
	CmtNewThreadLocalVar (sizeof(unsigned int), NULL, NULL, NULL, &ghThreadLocalVar);
	
	CmtNewLock("123", 0, &ThreadLock);
	CmtNewLock("ModuleSN", 0, &ThreadLock_ModuleSN);
	CmtNewLock("JW8504",0,&ThreadLock_JW8504);
	CmtNewLock("pss",0,&Thread_PSS_PowerPeter);	   //用于PSS多通道光功率计切换 
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
			MessagePopup ("提示", "请先终止测试，然后再退出程序");
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
			
			//按照设备配置设置主界面控件
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
				{MessagePopup("提示", "Initial CSA8000 error!"); return -1;}  
			
				if (!CSA8000_GET_MainSN(inst_CSA8000, my_struCSA8000.MainSN)) 
				{MessagePopup("提示", "读取CSA8000主机序列号错误！"); return -1;} 
				
				SetCtrlVal (panel, PAN_INST_STR_SN_DCA, my_struCSA8000.MainSN);	 
			}
			else if (index == DCA_TYPE_Ag86100)
			{
				if (!Ag86100_Init(&inst_Ag86100, dac_add)) 
				{MessagePopup("提示", "Initial Ag86100 error!"); return -1;} 
			
				if (!Ag86100_GET_MainSN(inst_Ag86100, my_struAg86100.MainSN)) 
				{MessagePopup("提示", "读取Ag86100主机序列号错误！"); return -1;} 
				
				SetCtrlVal (panel, PAN_INST_STR_SN_DCA, my_struAg86100.MainSN);
			}
			else 
			{
				MessagePopup("提示", "Can not find this DCA!"); 
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
			
			//光开关
			GetCtrlVal (panel, PAN_INST_RING_SW_TYPE, &localInst.SW_TYPE); 
			GetCtrlVal (panel, PAN_INST_NUM_SW, &localInst.SW); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_CHAN, &localInst.SW_CHAN); 
			GetCtrlIndex (panel, PAN_INST_RING_FSW, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_FSW, index, localInst.SW_SN);}  
			else         {strcpy (localInst.SW_SN, "");}
			
			//光开关（波长计）
			GetCtrlVal (panel, PAN_INST_RING_SW_TYPE_2, &localInst.SW_TYPE_2); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_2, &localInst.SW_2); 
			GetCtrlVal (panel, PAN_INST_NUM_SW_CHAN_2, &localInst.SW_CHAN_2); 
			GetCtrlIndex (panel, PAN_INST_RING_FSW_2, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_FSW_2, index, localInst.SW_SN_2);}  
			else         {strcpy (localInst.SW_SN_2, "");}
			
			//时钟切换板
			GetCtrlVal (panel, PAN_INST_RING_CLOCK_TYPE, &localInst.CLOCK_TYPE); 
			GetCtrlIndex (panel, PAN_INST_RING_CLOCK, &index); 
			if(index>=0) {GetLabelFromIndex (panel, PAN_INST_RING_CLOCK, index, localInst.CLOCK);}  
			else         {strcpy (localInst.CLOCK, "");}
			GetCtrlVal (panel, PAN_INST_NUM_CLOCK_CHAN, &localInst.CLOCK_CHAN);
			
			//添加到设备列表
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
				{   //选中了父项
					GetLabelFromIndex (panel, PAN_INST_TREE, eventData2, label1); 
					GetTreeItemAttribute (panel, PAN_INST_TREE, eventData2, ATTR_IMAGE_INDEX, &ImageIndex);
					ParentIndex = eventData2;
				}
				else
				{   //选中了子项
					GetLabelFromIndex (panel, PAN_INST_TREE, ParentIndex, label1);  
					GetTreeItemAttribute (panel, PAN_INST_TREE, ParentIndex, ATTR_IMAGE_INDEX, &ImageIndex);
				}

				sscanf (label1,"通道%d",&channel);
	            SetCtrlVal (panel, PAN_INST_NUM_CHANNEL, channel);
				
				//显示设备相关信息，注意此时结构体INSTR必须要有值，所以要保证之前必须运行过函数GetConfig_Inst
				
				if (TREE_IMAGE_OK==ImageIndex || TREE_IMAGE_WARING==ImageIndex)
				{
					SetCtrlVal (panel, PAN_INST_BIN_CHAN_EN, 1);	
				}
				else
				{
					SetCtrlVal (panel, PAN_INST_BIN_CHAN_EN, 0);
				}
				
				//显示光功率计信息
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
			
			//保存设备配置到存储文件
			for (channel=0; channel<CHANNEL_MAX; channel++)
			{
				//通过创建本地union变量，赋值0后，再传给测试结果存储结构体
				memset (uInst.pStr, 0, sizeof (uInst.pStr));
				INSTR[channel] = uInst.sStr; 

				sprintf (label, "通道%d", channel);
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
					//如果光纤有变更，需要校准
					if (0!= stricmp(sn,INSTR[channel].Fiber)) 
					{
						//换光纤，强制校准收端和发端光路
						sRxCal.flag[channel] = FALSE;
						my_struTxCal.flag[channel]=FALSE; 
					}
					strcpy (INSTR[channel].Fiber, sn);
					
					//光开关
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SW, &INSTR[channel].SW_TYPE, &INSTR[channel].SW, &INSTR[channel].SW_CHAN, INSTR[channel].SW_SN);
				
					//光开关2
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_SW_2, &INSTR[channel].SW_TYPE_2, &INSTR[channel].SW_2, &INSTR[channel].SW_CHAN_2, INSTR[channel].SW_SN_2);
					
					//时钟切换板
					GetLabelFromIndex (panel, PAN_INST_TREE, ++index, label);
					sscanf (label, LAB_CLOCK, &INSTR[channel].CLOCK_TYPE, INSTR[channel].CLOCK, &INSTR[channel].CLOCK_CHAN);
				}
			}
			
			//写入配置文件
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
				//显示高级功能配置界面
				if (stricmp (my_struLicense.power, "admin")!=0) 		
				{MessagePopup("提示","输入的账号没有使用此功能的权限！"); return -1;} 
				
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
				//显示校准配置界面
				if (stricmp (my_struLicense.power, "admin")!=0) 		
				{MessagePopup("提示","输入的账号没有使用此功能的权限！"); return -1;} 
				
				DisplayPanel (phCaliLimit); 
				
				//显示校准文件相关信息
			 	errChk(GetCaliConfigFile());
				
				//创建进度对话框
//				progresshandle = CreateProgressDialog ("加载校准门限数据", "数据加载进度", 1, VAL_NO_INNER_MARKERS, "");
				
				//默认选择第一个使用的通道
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
					my_struTxCal.flag[channel]=TRUE;    //开机可以不用校准 
					sRxCal.flag[channel] = FALSE;
				}
				
				//获取系统配置
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
					MessagePopup("提示","输入的账号没有使用此功能的权限！"); 
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
				MessagePopup("提示","程序界面显示逻辑异常    "); 
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
			{	 //当前界面是主程序界面
				panelCB (panMain,EVENT_CLOSE, 0,0,0); 
			}
			else if (License_Flag_INIT == showphID)
			{
				//当前界面还没有打开主界面，直接退出
				QuitUserInterface (0); 
			}
			
			break;
		}
	return 0;
}

void CVICALLBACK On_SGD (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	//清除批次
	SetCtrlVal (panMain, PAN_MAIN_STR_BATCH, ""); 
	
	if (0==stricmp (my_struConfig.PN, ""))
	{
		MessagePopup("提示", "请先选择产品型号    "); 
		return;
	}
	
	InstallPopup (panOrder); 
	
	SetCtrlVal (panOrder, PAN_ORDER_TEXTMSG, "正在查询OA系统，请稍等...    "); 
	SetWaitCursor (1); 
//	DBOA_Read_pch (my_struConfig.PN, my_struConfig.BOM, panOrder, PAN_ORDER_COMBO_ORDER);
	DB_Read_pch (my_struConfig.PN, my_struConfig.BOM, panOrder, PAN_ORDER_COMBO_ORDER); 		//切换为K3系统
	SetWaitCursor (0); 
	SetCtrlVal (panOrder, PAN_ORDER_TEXTMSG, "查询OA系统结束    ");  
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
			error = DB_Read_Order (WorkOrder);			//MySQL切换至Oracle	wenyao.xi 2015-11-27   
			if (error) 	{Insert_Info(panMain, PAN_MAIN_TEXTBOX_CHAN0, "查询OA数据库异常");}  
			else    	{Insert_Info(panMain, PAN_MAIN_TEXTBOX_CHAN0, "查询OA数据库成功");} 
	
			//设置批次
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
	
	//读取配置文件
	GetConfig(); 
	//根据文件存放的信息设置界面上的显示信息
	//避免PNIndex>PAN_CONF_RING_PN数目的bug
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
			 
			//更新产品版本
			GetCtrlIndex (panConf, PAN_CONF_RING_BOM, &index);  
			if(index>=0)
			{
				GetLabelFromIndex (panConf, PAN_CONF_RING_BOM, index, my_struConfig.BOM);
			}
			else
			{
				strcpy (my_struConfig.BOM, "");
			}

			//更新产品EEPROM版本
			GetCtrlIndex (panConf, PAN_CONF_RING_EED, &index);
			if(index>=0)
			{
				GetLabelFromIndex (panConf, PAN_CONF_RING_EED, index, my_struConfig.EED);
			}
			else
			{
				strcpy (my_struConfig.EED, "");
			}
			
			//识别是否是AL客户产品
			if (strstr (my_struConfig.PN, "-AL") != NULL) my_struConfig.CUSTOMER = CUSTOMER_AL;
			else if (strstr (my_struConfig.PN, "-AP") != NULL) my_struConfig.CUSTOMER = CUSTOMER_AP; 
			else if (strstr (my_struConfig.PN, "-01") != NULL) my_struConfig.CUSTOMER = CUSTOMER_01;		/***增加HW客户**Eric.Yao***/       
			else   										  my_struConfig.CUSTOMER = CUSTOMER_STANDARD;

			//function
			GetCtrlVal (panConf, PAN_CONF_RAD_TUNING, &my_struConfig.DT_Tuning);
			GetCtrlVal (panConf, PAN_CONF_RAD_TUN_RX, &my_struConfig.DT_Tun_RX); 
			GetCtrlVal (panConf, PAN_CONF_RAD_TEST_RX, &my_struConfig.DT_Test_RX); 
			GetCtrlVal (panConf, PAN_CONF_RAD_TEST, &my_struConfig.DT_Test); 
			GetCtrlVal (panConf, PAN_CONF_RAD_QA, &my_struConfig.DT_QATest); 
			GetCtrlVal (panConf, PAN_CONF_RAD_OQA, &my_struConfig.DT_OQA);     /***添加OQA测试选项**Eric.Yao***/
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
		
			//读取配置	
			error = DB_Get_ConfigInfo ();
			if(error) return -1;
			
			//OSA波长测试	 
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
				 
				strcpy (testitem, "OSA波长测试");  
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
			
			
			//按照主芯片读取配置
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
				MessagePopup ("提示", "数据库AutoDT_Spec_ERCompens表中定义的ERCompensChip不能识别"); 
				return -1;
			}
			
			//按照驱动芯片读取配置
			if (DRIVERCHIP_VSC7967==my_struDBConfig_ERCompens.DriverChip || DRIVERCHIP_VSC7965==my_struDBConfig_ERCompens.DriverChip)
			{
				error = DB_Get_AutoDT_Spec_VSC796x();
				if (error) return -1;
			}
			else if (DRIVERCHIP_NT25L90==my_struDBConfig_ERCompens.DriverChip)
			{
				error = DB_Get_AutoDT_Spec_NT25L90();
				if (error) return -1;

				//获取fOVERDFlag配置值,只对Vsc7967有效，默认值0
				my_struDBConfig_VSC7965.UpdatefOVERDFlag=0;
			}
			else if (DRIVERCHIP_UX3328==my_struDBConfig_ERCompens.DriverChip) 
			{
				//获取fOVERDFlag配置值,只对Vsc7967有效，默认值0
				my_struDBConfig_VSC7965.UpdatefOVERDFlag=0;
			}
			else
			{
				MessagePopup("提示","数据库AutoDT_Spec_ERCompens中DriverChip设置值不正确！"); 
				return -1;  
			}
*/			
			//发端监控门限
			error = DB_Read_Spec_TxCal();
			if(error) return -1;
			
			//读取表for Calibrate  
			if(my_struConfig.Sel_Calibrate_Test || my_struConfig.Sel_Calibrate)
			{
				error = DB_Get_Config_Monitor_Info ();
				if(error) return -1;
				
				//只读取标志位==2的校准测试点
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
					strcpy (testitem, "高温调试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温调试");
				}
				else
				{
					strcpy (testitem, "常温调试"); 
				}   
			}
			else if (my_struConfig.DT_Tun_Reliability) 	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "可靠性_高温调试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "可靠性_低温调试");
				}
				else
				{
					strcpy (testitem, "可靠性_常温调试"); 
				}   
			}
			else if (my_struConfig.DT_Test_Reliability) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "可靠性_高温测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "可靠性_低温测试");
				}
				else
				{
					strcpy (testitem, "可靠性_常温测试"); 
				}
			}
			else if (my_struConfig.DT_Test_Upstream) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "上行误码_高温测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "上行误码_低温测试");
				}
				else
				{
					strcpy (testitem, "上行误码_常温测试"); 
				}
			}
			else if (my_struConfig.DT_Tun_RX) 	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "高温收端调试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温收端调试");
				}
				else
				{
					strcpy (testitem, "常温收端调试"); 
				}
			}
			else if (my_struConfig.DT_Test_RX) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "高温收端测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温收端测试");
				}
				else
				{
					strcpy (testitem, "常温收端测试"); 
				}
			}
			else if (my_struConfig.DT_Test) 
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "高温测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温测试");
				}
				else
				{
					strcpy (testitem, "常温测试"); 
				}
			}
			else if (my_struConfig.DT_QATest)	
			{
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "高温QA测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温QA测试");
				}
				else
				{
					strcpy (testitem, "常温QA测试"); 
				}
			}
			else if (my_struConfig.DT_OQA)	
			{
				 /***增加OQA**Eric.Yao***/       
				if (my_struConfig.Temper_High)
				{
					strcpy (testitem, "高温OQA测试");
				}
				else if (my_struConfig.Temper_Low)  
				{
					strcpy (testitem, "低温OQA测试");
				}
				else
				{
					strcpy (testitem, "常温OQA测试"); 
				}
			}
			else if (my_struConfig.DT_Agin_Front)	
			{
				strcpy (testitem, "常温老化前");  
			}
			else if (my_struConfig.DT_Agin_Back)	
			{
				strcpy (testitem, "常温老化后");  
			}
			else if (my_struConfig.DT_OSA_WL_Test)	
			{
				strcpy (testitem, "常温OSA波长测试");  
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
			
			//修改界面控件属性
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
							MessagePopup ("提示", "当前程序不支持此测试功能");
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

			//修改界面控件属性
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
	strcat (filename, "\\版本历史.txt");
	
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

	//清空校准结果显示
	FillTableCellRange (phCalT, PAN_CALT_TABLE_T, MakeRect (1, 1, 3, 2), val);
	
	//默认选择第一个使用的通道
	errChk(GetEnableChannel(&channel));
	SET_ATT_ONU (channel, -60); 
	
	Radio_SetMarkedOption (phCalT, PAN_CALT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	EVB5_SET_BEN_Level(INST_EVB[channel], 0);  
	//===================加载发端校准记录===================//
	for (i=0; i<3; i++)
	{
		SetTableCellVal (phCalT, PAN_CALT_TABLE_T, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
		SetTableCellVal (phCalT, PAN_CALT_TABLE_T, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
	}
	//===================加载发端校准记录===================//
	
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

	//清空校准结果显示
	FillTableCellRange (panCaliR, PAN_CALR_TABLE_R, MakeRect (1, 1, 1, 1), val);

	//默认选择第一个使用的通道
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (panCaliR, PAN_CALR_RAD_CHAN, channel);

//	errChk(SET_EVB_SHDN(channel, 1));
	
	//===================加载收端校准记录===================//
	if (INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_NONE)
	{
		sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
		MessagePopup("提示", buf);
		error=-1;
		goto Error;
	} 
	
	SetTableCellVal (panCaliR, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 

	errChk(Init_BERT (channel));

	errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
	//===================加载收端校准记录===================// 
	
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

	//清空校准结果显示
	FillTableCellRange (phCalOLT, PN_CAL_OLT_TABLE_R, MakeRect (1, 1, 1, 1), val);
	
	//默认选择第一个使用的通道
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalOLT, PAN_CALOLT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	
	DisplayPanel (phCalOLT);
	
	//===================加载收端校准记录===================//
	if (INSTR[channel].ATT_TYPE_OLT == ATT_TYPE_NONE)
	{
		sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
		MessagePopup("提示", buf);
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
	//===================加载收端校准记录===================// 
	
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
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志 
	while (gFlag)
	{
		
		bOK=TRUE; 
		SaveData=TRUE;		
		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			Delay(0.5);
			//if (!Channel_Sel_Flag[channel]) 
			//{   
			//	Delay(0.1);
			//	continue;
			//}
			
			//清除测试数据
			memset(&data,0,sizeof(data));
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			//.//errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(2);
			WaitFlag[channel] = FALSE;
		} 
		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    "); 	
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准");  
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);

	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行OSA波长测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0));   
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}  
	    //get bosasn
		if (bOK)
		{
			strcpy (Info, "输入条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN_OSA_WL_TEST(panMain,channel, &data); 
			if (error){sprintf (Info, "输入条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;SaveData=FALSE;}  
			else	  {sprintf (Info, "输入条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	//.//  	if(my_struConfig.DT_OSA_WL_Test&&bOK)   
		{
	     	strcpy (Info, "OSA测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TEST_OSA_EX(channel,&data);
			if (error)
			{   sprintf (Info, "OSA测试失败;%f波长对应温度=%f℃",struDBConfig_OSATEST.WaveLenth_Min,data.LDTemper[0]);     
			    Insert_Info(panMain, gCtrl_BOX[channel], Info);
				bOK=FALSE;
			}
			else
			{
			    sprintf (Info, "OSA测试成功;%f波长对应温度=%f℃",struDBConfig_OSATEST.WaveLenth_Min,data.LDTemper[0]);
			    Insert_Info(panMain, gCtrl_BOX[channel], Info);
			    SaveData=TRUE;
				bOK=TRUE;
			}
	     }
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN		
//		strcpy (prolog.Operator, my_struLicense.username);
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{ 
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase_1(prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "OSA测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "OSA测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);   
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道OSA测试已停止");
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
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志     	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);   
				Delay(0.1);
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}

		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    "); 	
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行调试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}		    
		
		data.Time_Start=Timer();	
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "初始化时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//检查工序
		if (bOK && !my_struConfig.DT_Tun_Reliability && !my_struConfig.DT_Test_Reliability)
		{
			strcpy (Info, "测试工序...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "模块短路：失败");
				}
				else
				{
					sprintf (Info, "测试工序：失败");       
					data.ErrorCode=ERR_TUN_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "测试工序 成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD_Detect信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "测试发端SD_Detect信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "测试发端SD_Detect信号：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		// 短路测试
		if (bOK && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "开始短路测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "短路测试：失败");bOK=FALSE;} 
			else	  {strcpy (Info, "短路测试：成功");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//test SD		
		if (bOK && CHIEFCHIP_DS4830!=my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "测试SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testSD (channel);
			if (error) {strcpy (Info, "测试SD信号：失败");bOK=FALSE;}  
			else	   {strcpy (Info, "测试SD信号：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK)
		{
			strcpy (Info, "调试Vapd电压...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd (channel, &data);
			if (error)	{strcpy (Info, "调试Vapd电压：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "调试Vapd电压：成功;APD_DAC=0x%X",(int)data.Vapd_DAC);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;												 
		sprintf(Info, "TunVAPD时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//test SDA SDD	
		if (bOK && my_struConfig.Sel_R_SDHys) 
		{
			strcpy (Info, "测试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSD(channel,&data);
			if (error){sprintf (Info, "测试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD); bOK=FALSE; data.ErrorCode=ERR_TES_SD;} 
			else	  {sprintf (Info, "测试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode)) 
		{
			strcpy (Info, "调试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestSD时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "启动过载测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "启动过载测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/						 
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestOverStart时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//当选择示波器时选用此方法
		if (INSTR[channel].DCA_TYPE != DCA_TYPE_NONE)
		{
			//AOP
			if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "调试光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP(channel, &data);
				if (error) 
				{
					sprintf (Info, "调试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);
					bOK=FALSE;
					if (data.ErrorCode==0) 
					{
						data.ErrorCode=ERR_TUN_AOP; 
					}
				}
				else	  	
				{
					sprintf (Info, "调试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm成功", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);  
				}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
		}
		else
		{
			//AOP & ER
			if (bOK && my_struConfig.Sel_T_AOP && my_struConfig.Sel_T_ER && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "调试消光比和光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP_ER(channel, &data);
				if (error) {sprintf (Info, "调试消光比和光功率=%.2fdBm FAIL", data.AOP);bOK=FALSE;}
				else	   {sprintf (Info, "调试消光比和光功率=%.2fdBm PASS", data.AOP);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunAOP时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//检查过载测试结果
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "检查过载测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "过载测试=%.2fdBm 失败, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 #if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "OverCheck时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//test sens  灵敏度测试时间与调试波长，ER，测试光谱复用	  	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "启动灵敏度测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "启动灵敏度测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenTestStart时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Test Sen Detail
		if (bOK && my_struConfig.Sel_R_Sens && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail)) ) 
		{
			strcpy (Info, "测试灵敏度值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //逼近法测SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //外推法测误码率1.0e-12数量级SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Test Sen Detail时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Tuning Wavelength 01
		if (bOK)
		{
			strcpy (Info, "调波长01,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength01(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 01时间=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/	  
#endif
		// Tun Er, 眼图 mask 光谱等  
		if (bOK)
		{
			strcpy (Info, "调Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunEr时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//Tuning Wavelength  02
		if (bOK)
		{
			strcpy (Info, "调波长02,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength02(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 02时间=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/	
#endif
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "调试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "调试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
#if Debug 
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunTxOff时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//Sen Check	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenCheck时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))	/***发端校准设置为比选项**Eric.Yao**20131203***/
		{	
		 	strcpy (Info, "发端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "发端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "发端校准：成功");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
#if Debug	
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TxCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//calRxPower
	 	if (bOK && my_struConfig.Sel_Calibrate)
		{	
		 	strcpy (Info, "收端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = calRxPower(channel, &data, panMain);
			if (error) {sprintf (Info, "收端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_RXPOWER;} 
			else	   {sprintf (Info, "收端校准：成功");} 	      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		  
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "RxCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/   
#endif	
#if Debug
  		//验证调试前后AOP,Er,WL，TxOffPW变化对比用；正常使用需关闭；
		// Test aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestAOP时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
		
		//Test Er
		if (bOK)
		{
			strcpy (Info, "测试Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestEr时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/

		//Test TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestTxOff时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif 		
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "测波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestWL时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif	
		if(data.WL_ReTun_FLAG)
		{
			// Test aop
/*			if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "波长微调后，测试光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testAOP(channel, &data);
				if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
				else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
*/
			
#if Debug		
			/****************************耗时统计********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestAOP时间=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************耗时统计********************************/
#endif		
/*			//Test Er
			if (bOK)
			{
				strcpy (Info, "波长微调后，测试Er,切换光路...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
				if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
			}
*/			
#if Debug
			/****************************耗时统计********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestEr时间=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************耗时统计********************************/
#endif
			//Test TxOff Depth   
			if (bOK && my_struConfig.Sel_T_TxDis) 
			{
				strcpy (Info, "波长微调后，测试关断深度...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testTxOff_Depth(channel,&data);
				if (error) 
				{
					//TxOff Depth   
					if (my_struConfig.Sel_T_TxDis) 
					{
						strcpy (Info, "重调试关断深度...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = tuningTxOff_Depth(channel, &data);
						if (error)
						{
							error = tuningTxOff_Depth_Ex(channel, &data);
							if (error) {sprintf (Info, "重调试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
							else	   {sprintf (Info, "重调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
						}
						else
						{
						 	sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
						}
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
					}
					
					//Test TxOff Depth   
					if (bOK && my_struConfig.Sel_T_TxDis) 
					{
						strcpy (Info, "重测试关断深度...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = testTxOff_Depth(channel,&data);
						if (error) 
						{  	
							sprintf (Info, "重测试关断深度 失败");
							bOK=FALSE;
							data.ErrorCode=ERR_TES_TxOffPower;
						}
						else	   
						{
							error = testTxOff_Depth(channel,&data);  
							sprintf (Info, "重测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);
						}      
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
					}
					//Test Wavelength 
					if (bOK)
					{
						strcpy (Info, "测波长,切换光路...");
						Insert_Info(panMain, gCtrl_BOX[channel], Info);
						error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
						if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
					} 
				
				} 
				else	   
				{
					error = testTxOff_Depth(channel,&data);  
					sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);
				}      
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
		//Cal temperature
		if (bOK)
		{
			strcpy (Info, "模块温度校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = CalTemperature(channel);
			if (error<0) {sprintf (Info, "模块温度校准: 失败");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TEMPERATURE;} 
			else	   {sprintf (Info, "模块温度校准：成功");}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TempCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		
#if 0	//Ibias Max功能由考虑柔板断路设置保护功能 改为 限制AOP不得大于6dBm的异常限制；移至老化后工序设置； 2017-02-23  wenyao.xi
		// 设置最大Ibias限定值
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "设置最大Ibias限定值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Ibias_Max_Ex(INST_EVB[channel]);
			if (error) 
			{
				strcpy (Info,"设置最大Ibias限定值：失败");
				data.ErrorCode=ERR_TES_SetIbiasMax; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"设置最大Ibias限定值：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SetImax时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "调试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "调试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道调测已停止");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//设置波长通道1
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
		MessagePopup("提示", "测试温度选择异常");	
	}
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    ");
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//检查工序
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "测试工序...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "模块短路：失败");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "光功率超上限：失败"); 
				}
				else
				{
					sprintf (Info, "测试工序：失败");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "测试工序 成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD_Detect信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "测试发端SD_Detect信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "测试发端SD_Detect信号：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		} 
		
		//test TxSD
/*		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "测试发端SD信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "测试发端SD信号：成功");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
*///测试项目重复，取消 2017-02-25			   
		//TxDisable
/*		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试发端关断光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "测试发端关断光功率AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm 成功", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试发端关断光功率AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm 成功", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} */
		//重调Vapd
		if(bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "高低温重试Vapd电压...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd_HighLow (channel, &data);
			if (error)	{strcpy (Info, "重试Vapd电压：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "重试Vapd电压：成功;APC_DAC=0x%X",(int)data.Vapd_DAC);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
			  
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "测试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode) && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "调试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "启动过载测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "启动过载测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "启动ONU传输性能测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "启动ONU传输性能测试：失败 %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "启动ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Check Over
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "检查过载测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "过载测试=%.2fdBm 失败, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***启动灵敏度测试放置在发端测试项目前**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)
		{
			strcpy (Info, "启动灵敏度测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "启动灵敏度测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
//------------------------------------------------		
		//Tuning Wavelength 
		if (bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "调波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "调试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "调试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
		// er 眼图 mask 光谱等  
		if (bOK)
		{
			strcpy (Info, "测Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
//-------------------------------------------------	
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room && !my_struConfig.DT_Test_Reliability) 
		{
			strcpy (Info, "测试TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f 失败", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f 成功", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && data.RetuningAopFlag )	/***重调光功率后发端校准***/
		{	
		 	strcpy (Info, "Re发端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "Re发端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "Re发端校准：成功");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "发端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"发端校准测试：失败");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"发端校准测试：成功");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		if (bOK)
		{
			strcpy (Info, "测试Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "测试Ibias=%.2fmA 失败", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "测试Ibias=%.2fmA 成功", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// 短路测试
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "开始短路测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "短路测试：失败");bOK=FALSE;} 
			else	  {strcpy (Info, "短路测试：成功");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		// check sens
/*		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)        
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
			
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "测试灵敏度值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //逼近法测SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //外推法测误码率1.0e-12数量级SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}

		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "收端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"收端校准测试：失败");
				if (data.ErrorCode==0) data.ErrorCode=error;  //将测试函数的错误代码赋值给error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"收端校准测试：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "检查ONU传输性能测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU传输性能测试：失败 %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "测波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		if(data.WL_ReTun_FLAG)
		{
			// Test aop
/*			if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "波长微调后，测试光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testAOP(channel, &data);
				if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
				else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
*/
#if Debug
			/****************************耗时统计********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestAOP时间=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************耗时统计********************************/
#endif		
			//Test Er
/*			if (bOK)
			{
				strcpy (Info, "波长微调后，测试Er,切换光路...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
				if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
			}	 
*/
#if Debug 
			/****************************耗时统计********************************/		
			SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
			data.Time_End=Timer()-data.Time_Start;
			sprintf(Info, "TestEr时间=%ds",data.Time_End);
			Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
			data.Time_Start=Timer();	
			/****************************耗时统计********************************/
#endif
			//Test TxOff Depth   
			if (bOK && my_struConfig.Sel_T_TxDis) 
			{
				strcpy (Info, "波长微调后，测试关断深度...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = testTxOff_Depth(channel,&data);
				if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
				else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
		// 扫描模块SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "输入产品序列码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"输入产品序列码=%s 失败", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"输入产品序列码=%s 成功", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		// save eeprom
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "保存EEPROM...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveEEPROM(channel, &data);
			if (error) 
			{
				strcpy (Info,"保存EEPROM：失败");
				data.ErrorCode=ERR_TES_SAVE_EEPROM; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"保存EEPROM：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}  
/*		
		// 电流
		if (bOK)
		{
		//	if(!bOK)
		//	{
		//		strcpy(temp_Info,Info);
		//	}
			strcpy (Info, "测试电流...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 失败",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 成功",data.RxI[0],data.TxI[0]);}	  	       
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
		
		//开启Burnin状态；
		if (bOK && my_struConfig.Temper_Room && my_struConfig.DT_Test_Reliability)    		//可靠性测试后开启Burn in功能；	
		{
			strcpy (Info, "开启Burnin状态...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Burnin_Enable(channel);
			if (error) 
			{
				strcpy (Info,"开启Burnin状态：失败");
				data.ErrorCode=ERR_TUN_SetBurnin; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"开启Burnin状态：成功");
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
	
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "保存调试数据：失败");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("提示", buf);
			}  
			else	   
			{
				strcpy (Info, "保存调试数据：成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("提示", "未保存测试数据！！！"); 
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//检测模块是否在测试板上
/*		if (bMOD0)
		{
			strcpy (Info, "请将被测模块从测试系统取出...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "检测测试板是否有模块出现异常");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "请将被测模块从测试系统取出...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//设置波长通道1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "此通道调测已停止");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//设置波长通道 1
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
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志     	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);   
				Delay(0.1);
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}

		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    "); 	
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		} 
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行调试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//for tssi
		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}		    
		
		data.Time_Start=Timer();	
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TUN_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "初始化时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TUN_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//检查工序
		if (bOK && !my_struConfig.DT_Tun_Reliability && !my_struConfig.DT_Test_Reliability)
		{
			strcpy (Info, "测试工序...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "模块短路：失败");
				}
				else
				{
					sprintf (Info, "测试工序：失败");       
					data.ErrorCode=ERR_TUN_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "测试工序 成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD_Detect信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "测试发端SD_Detect信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "测试发端SD_Detect信号：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		// 短路测试
		if (bOK && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "开始短路测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "短路测试：失败");bOK=FALSE;} 
			else	  {strcpy (Info, "短路测试：成功");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//test SD		
		if (bOK && CHIEFCHIP_DS4830!=my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "测试SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testSD (channel);
			if (error) {strcpy (Info, "测试SD信号：失败");bOK=FALSE;}  
			else	   {strcpy (Info, "测试SD信号：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK)
		{
			strcpy (Info, "调试Vapd电压...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd (channel, &data);
			if (error)	{strcpy (Info, "调试Vapd电压：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "调试Vapd电压：成功;APD_DAC=0x%X",(int)data.Vapd_DAC);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;												 
		sprintf(Info, "TunVAPD时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//test SDA SDD	
		if (bOK && my_struConfig.Sel_R_SDHys) 
		{
			strcpy (Info, "测试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSD(channel,&data);
			if (error){sprintf (Info, "测试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD); bOK=FALSE; data.ErrorCode=ERR_TES_SD;} 
			else	  {sprintf (Info, "测试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode)) 
		{
			strcpy (Info, "调试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestSD时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "启动过载测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "启动过载测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/						 
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestOverStart时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//当选择示波器时选用此方法
		if (INSTR[channel].DCA_TYPE != DCA_TYPE_NONE)
		{
			//AOP
			if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "调试光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP(channel, &data);
				if (error) 
				{
					sprintf (Info, "调试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);
					bOK=FALSE;
					if (data.ErrorCode==0) 
					{
						data.ErrorCode=ERR_TUN_AOP; 
					}
				}
				else	  	
				{
					sprintf (Info, "调试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm成功", data.AOP[0], data.AOP[1], data.AOP[2], data.AOP[3]);  
				}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		
		}
		else
		{
			//AOP & ER
			if (bOK && my_struConfig.Sel_T_AOP && my_struConfig.Sel_T_ER && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
			{
				strcpy (Info, "调试消光比和光功率...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = tuningAOP_ER(channel, &data);
				if (error) {sprintf (Info, "调试消光比和光功率=%.2fdBm FAIL", data.AOP);bOK=FALSE;}
				else	   {sprintf (Info, "调试消光比和光功率=%.2fdBm PASS", data.AOP);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunAOP时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//检查过载测试结果
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "检查过载测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "过载测试=%.2fdBm 失败, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 #if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "OverCheck时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//test sens  灵敏度测试时间与调试波长，ER，测试光谱复用	  	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "启动灵敏度测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "启动灵敏度测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenTestStart时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Test Sen Detail
		if (bOK && my_struConfig.Sel_R_Sens && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail)) ) 
		{
			strcpy (Info, "测试灵敏度值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //逼近法测SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //外推法测误码率1.0e-12数量级SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Test Sen Detail时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Tuning Wavelength 01
		if (bOK)
		{
			strcpy (Info, "调波长01,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength01(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 01时间=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/	  
#endif
		// Tun Er, 眼图 mask 光谱等  
		if (bOK)
		{
			strcpy (Info, "调Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunEr时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//Tuning Wavelength  02
		if (bOK)
		{
			strcpy (Info, "调波长02,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength02(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "Set PID Count=%d,TunWL 02时间=%ds",data.PointCount,data.Time_End); 
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/	
#endif
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "调试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "调试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
#if Debug 
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TunTxOff时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//Sen Check	  
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SenCheck时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))	/***发端校准设置为比选项**Eric.Yao**20131203***/
		{	
		 	strcpy (Info, "发端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "发端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "发端校准：成功");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
#if Debug	
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TxCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//calRxPower
	 	if (bOK && my_struConfig.Sel_Calibrate)
		{	
		 	strcpy (Info, "收端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = calRxPower(channel, &data, panMain);
			if (error) {sprintf (Info, "收端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_RXPOWER;} 
			else	   {sprintf (Info, "收端校准：成功");} 	      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "RxCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/   
#endif	
#if Debug
  		//验证调试前后AOP,Er,WL，TxOffPW变化对比用；正常使用需关闭；
		// Test aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestAOP时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
		
		//Test Er
		if (bOK)
		{
			strcpy (Info, "测试Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestEr(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestEr时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/

		//Test TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestTxOff时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif 		
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "测波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TestWL时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif		
		//Cal temperature
		if (bOK)
		{
			strcpy (Info, "模块温度校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = CalTemperature(channel);
			if (error<0) {sprintf (Info, "模块温度校准: 失败");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TEMPERATURE;} 
			else	   {sprintf (Info, "模块温度校准：成功");}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
#if Debug
		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "TempCal时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		
#if 0	//Ibias Max功能由考虑柔板断路设置保护功能 改为 限制AOP不得大于6dBm的异常限制；移至老化后工序设置； 2017-02-23  wenyao.xi
		// 设置最大Ibias限定值
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "设置最大Ibias限定值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Ibias_Max_Ex(INST_EVB[channel]);
			if (error) 
			{
				strcpy (Info,"设置最大Ibias限定值：失败");
				data.ErrorCode=ERR_TES_SetIbiasMax; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"设置最大Ibias限定值：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		/****************************耗时统计********************************/		
		SetCtrlAttribute (panMain, gCtrl_BOX[channel+4], ATTR_DIMMED, 0);  
		data.Time_End=Timer()-data.Time_Start;
		sprintf(Info, "SetImax时间=%ds",data.Time_End);
		Insert_Info(panMain, gCtrl_BOX[channel+4], Info);
		data.Time_Start=Timer();	
		/****************************耗时统计********************************/
#endif
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "调试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "调试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
	 	Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	

Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道调测已停止");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//设置波长通道1
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
		MessagePopup("提示", "测试温度选择异常");	
	}
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    ");
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//检查工序
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "测试工序...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "模块短路：失败");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "光功率超上限：失败"); 
				}
				else
				{
					sprintf (Info, "测试工序：失败");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "测试工序 成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD_Detect信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "测试发端SD_Detect信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "测试发端SD_Detect信号：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		} 
		
		//test TxSD
/*		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "测试发端SD信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "测试发端SD信号：成功");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
*///测试项目重复，取消 2017-02-25			   
		//TxDisable
/*		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试发端关断光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "测试发端关断光功率AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm 成功", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试发端关断光功率AOP0=%.2fdBm AOP1=%.2fdBm AOP2=%.2fdBm AOP3=%.2fdBm 成功", data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} */
		//重调Vapd
		if(bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "高低温重试Vapd电压...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = TunVapd_HighLow (channel, &data);
			if (error)	{strcpy (Info, "重试Vapd电压：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_RAPD;}  
			else	   	{sprintf (Info, "重试Vapd电压：成功;APC_DAC=0x%X",(int)data.Vapd_DAC);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
			  
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "测试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//Tuning SD
		if (!bOK && my_struConfig.Sel_R_SDHys && (ERR_TES_SD ==data.ErrorCode) && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "调试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningSD(channel,&data);
			if (error)  {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;data.ErrorCode=ERR_TUN_SDA;} 
			else	    {sprintf (Info, "调试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);bOK=TRUE;}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Over TestStart
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "启动过载测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;}
			else	   {sprintf (Info, "启动过载测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "启动ONU传输性能测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "启动ONU传输性能测试：失败 %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "启动ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Check Over
		if (bOK && my_struConfig.Sel_R_Over) 
		{
			strcpy (Info, "检查过载测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver_End(channel, OverTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixOver_Start(channel, &OverTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动过载测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_OVER;
				}
				else	   
				{
					error = testFixSens_End(channel, OverTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "过载测试=%.2fdBm 失败, %s", data.Over, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_OVER; 
					}
					else
					{
						sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "过载测试=%.2fdBm 成功", data.Over);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***启动灵敏度测试放置在发端测试项目前**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)
		{
			strcpy (Info, "启动灵敏度测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "启动灵敏度测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
//------------------------------------------------		
		//Tuning Wavelength 
		if (bOK && !my_struConfig.Temper_Room)
		{
			strcpy (Info, "调波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TunWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis && !my_struConfig.Temper_Room) 
		{
			strcpy (Info, "调试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningTxOff_Depth(channel, &data);
			if (error)
			{
				error = tuningTxOff_Depth_Ex(channel, &data);
				if (error) {sprintf (Info, "调试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TUN_TxOFFPower;} 
				else	   {sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			}
			else
			{
			 	sprintf (Info, "调试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);      
			}
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		} 
		// er 眼图 mask 光谱等  
		if (bOK)
		{
			strcpy (Info, "测Er,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
//-------------------------------------------------	
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room && !my_struConfig.DT_Test_Reliability) 
		{
			strcpy (Info, "测试TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f 失败", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f 成功", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//calTxPower
	 	if (bOK && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && data.RetuningAopFlag )	/***重调光功率后发端校准***/
		{	
		 	strcpy (Info, "Re发端校准...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = calTxPower(channel, &data);
			if (error) {sprintf (Info, "Re发端校准：失败");bOK=FALSE;data.ErrorCode=ERR_TUN_CAL_TXPOWER;}
			else	   {sprintf (Info, "Re发端校准：成功");}          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "发端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"发端校准测试：失败");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"发端校准测试：成功");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		if (bOK)
		{
			strcpy (Info, "测试Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "测试Ibias=%.2fmA 失败", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "测试Ibias=%.2fmA 成功", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// 短路测试
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room && (SEVB_TYPE_SEVB5==INSTR[channel].SEVB_TYPE))
		{
			strcpy (Info, "开始短路测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "短路测试：失败");bOK=FALSE;} 
			else	  {strcpy (Info, "短路测试：成功");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		// check sens
/*		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail)        
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		if (bOK && my_struConfig.Sel_R_Sens && !my_struConfig.Sel_R_Sens_Detail) 
		{
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) 
			{	 
				error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
				if (error) 
				{
					sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;
				}
				else	   
				{
					error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
					if (error) 
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);
						bOK=FALSE;data.ErrorCode=ERR_TUN_SENS; 
					}
					else
					{
						sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);  
					}  
				}
			}
			else	   
			{
				sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
			
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "测试灵敏度值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //逼近法测SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //外推法测误码率1.0e-12数量级SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}

		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "收端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"收端校准测试：失败");
				if (data.ErrorCode==0) data.ErrorCode=error;  //将测试函数的错误代码赋值给error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"收端校准测试：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "检查ONU传输性能测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU传输性能测试：失败 %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "测波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		// 扫描模块SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "输入产品序列码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"输入产品序列码=%s 失败", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"输入产品序列码=%s 成功", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		// save eeprom
		if (bOK && my_struConfig.Temper_Room)    			
		{
			strcpy (Info, "保存EEPROM...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveEEPROM(channel, &data);
			if (error) 
			{
				strcpy (Info,"保存EEPROM：失败");
				data.ErrorCode=ERR_TES_SAVE_EEPROM; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"保存EEPROM：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}  
/*		
		// 电流
		if (bOK)
		{
		//	if(!bOK)
		//	{
		//		strcpy(temp_Info,Info);
		//	}
			strcpy (Info, "测试电流...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 失败",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 成功",data.RxI[0],data.TxI[0]);}	  	       
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
		
		//开启Burnin状态；
		if (bOK && my_struConfig.Temper_Room && my_struConfig.DT_Test_Reliability)    		//可靠性测试后开启Burn in功能；	
		{
			strcpy (Info, "开启Burnin状态...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = DWDM_ONU_Set_Burnin_Enable(channel);
			if (error) 
			{
				strcpy (Info,"开启Burnin状态：失败");
				data.ErrorCode=ERR_TUN_SetBurnin; 
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"开启Burnin状态：成功");
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
	
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "保存调试数据：失败");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("提示", buf);
			}  
			else	   
			{
				strcpy (Info, "保存调试数据：成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("提示", "未保存测试数据！！！"); 
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//检测模块是否在测试板上
/*		if (bMOD0)
		{
			strcpy (Info, "请将被测模块从测试系统取出...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "检测测试板是否有模块出现异常");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "请将被测模块从测试系统取出...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//设置波长通道1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "此通道调测已停止");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//设置波长通道 1
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
		MessagePopup("提示", "测试温度选择异常"); 
	}
	else if (my_struConfig.Temper_High)
	{
		MessagePopup("提示", "测试温度选择异常");
	}
	else
	{
		MessagePopup("提示", "测试温度选择异常");	
	}
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		
		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行收端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    ");
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		}

		data.RetuningAopFlag=FALSE;
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
	
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		

	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//检查工序
		//检查工序
		if (bOK && !my_struConfig.isNPI)
		{
			strcpy (Info, "测试工序...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = teststation(channel, &data, prolog);
			if (error)
			{
				if ((ERR_EEP_SHORT_VCC == data.ErrorCode) || (ERR_EEP_SHORT_VAPD == data.ErrorCode) || (ERR_EEP_SHORT_GND == data.ErrorCode))
				{
					sprintf (Info, "模块短路：失败");
				}
				else if (ERR_TES_AOP_MAX == data.ErrorCode)
				{
					sprintf (Info, "光功率超上限：失败"); 
				}
				else
				{
					sprintf (Info, "测试工序：失败");       
					data.ErrorCode=ERR_TES_STATION;	        
				}
				bOK=FALSE;
			}
			else	  
			{
				sprintf (Info, "测试工序 成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		if (bOK && my_struConfig.Sel_T_Upstream)  
		{
			strcpy (Info, "启动ONU上行误码测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			count = 0;
			do
			{
				error = test_olt_errbit_start(channel, &senstime_olt, buf);
				count++;
			}while(error<0 && count<3);
			if (error) {sprintf (Info, "启动ONU上行误码测试：失败 %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "启动ONU上行误码测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		
		if (bOK && my_struConfig.Sel_T_Upstream) 
		{
			strcpy (Info, "检查ONU上行误码测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU上行误码测试：失败 %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU上行误码测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// 扫描模块SN
		if (bOK && my_struConfig.Temper_Room)
		{
			strcpy (Info, "输入产品序列码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetModuleSN(channel, &data, panSN);
			if (error) 
			{
				sprintf (Info,"输入产品序列码=%s 失败", data.ModuleSN);
				data.ErrorCode=ERR_TES_INPUT_SN; 
				bOK=FALSE; 
			}  
			else	   
			{
				sprintf (Info,"输入产品序列码=%s 成功", data.ModuleSN);
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}

		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) 
			{
				strcpy (Info, "保存调试数据：失败");
				bOK=FALSE;
				data.ErrorCode=ERR_TES_SAVE_DATA;
				sprintf(buf,"TestDate=%s;Action_Time=%s",data.TestDate,prolog.Action_Time);
				MessagePopup ("提示", buf);
			}  
			else	   
			{
				strcpy (Info, "保存调试数据：成功");
			}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		else
		{
			MessagePopup ("提示", "未保存测试数据！！！"); 
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//检测模块是否在测试板上
/*		if (bMOD0)
		{
			strcpy (Info, "请将被测模块从测试系统取出...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				bMOD0 = 0;
				if (error) 
				{
					strcpy (Info, "检测测试板是否有模块出现异常");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "请将被测模块从测试系统取出...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		} */
		
		//设置波长通道1
//		DWDM_ONU_Select_Channelindex(channel,0);
		
		Channel_Sel_Flag[channel]=FALSE;
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:

	Channel_Sel_Flag[channel]=FALSE;	
	strcpy (Info, "此通道调测已停止");
	Insert_Info(panMain, gCtrl_BOX[channel], Info);  
	
	//设置波长通道 1
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
		MessagePopup("提示", "测试温度选择异常");	
	}
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//检查光路校准和点检是否过期
		if (!sRxCal.flag[channel] && INSTR[channel].ATT_TYPE_ONU != ATT_TYPE_NONE && (!my_struConfig.isNPI)) 
		{
			if(DB_get_cali (CALI_TYPE_RX, channel)<0)
			{
				 MessagePopup("提示", "请进行发端光路校准    "); 
			}
		}
		
		if (my_struConfig.Sel_T_AOP && (!my_struConfig.isNPI))
		{
			if (my_struTxCal.flag[channel])
			{
				if(DB_get_cali (CALI_TYPE_TX_CHECKUP, channel)<0)
				{
					MessagePopup("提示", "请进行发端光路校准    "); 
				}
			}
			else
			{
				 MessagePopup("提示", "请进行发端光路校准    ");
				 
			}
		}
	
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}

		//test temperature
		if (bOK)
		{
			strcpy (Info, "测试模块温度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTemperature(channel, panMain, &data);
			if (error) {sprintf (Info, "测试模块温度=%.2f℃ 失败", data.Temperatrue);bOK=FALSE;data.ErrorCode=ERR_TUN_TEMPER;} 
			else	   {sprintf (Info, "测试模块温度=%.2f℃ 成功", data.Temperatrue);}        
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD_Detect信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  
			error = test_TxSD_Detect(channel);
			if (error) 	{strcpy (Info, "测试发端SD_Detect信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_NO_DEFINE;}
			else		{strcpy (Info, "测试发端SD_Detect信号：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);  	
		}
		
		//test SD
		if (bOK && my_struConfig.Sel_R_SDHys)     
		{
			strcpy (Info, "测试收端SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			//error = testSD(channel);
			error = testFixSD(channel,&data);  
			if (error)  {strcpy (Info, "测试收端SD信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_SD;} 
			else		{strcpy (Info, "测试收端SD信号：成功");}	          
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		//test TxSD
		if (bOK && my_struConfig.Sel_TxSD) 
		{
			strcpy (Info, "测试发端SD信号...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			error = testTxSD(channel);
			if (error) {strcpy (Info, "测试发端SD信号：失败");bOK=FALSE;data.ErrorCode=ERR_TES_ONU_TXSD;} 
			else	   {strcpy (Info, "测试发端SD信号：成功");}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		} 
		
/*		//TxDisable
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试发端关断光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxDis(channel, &data);
			if (error){sprintf (Info, "测试发端关断光功率=%.2fdBm 失败", data.TxOffPower);bOK=FALSE;} 
			else	  {sprintf (Info, "测试发端关断光功率=%.2fdBm 成功", data.TxOffPower);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/		
		//over
		if (bOK && my_struConfig.Sel_R_Over)
		{
			strcpy (Info, "测试过载...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixOver(channel, &data,&prolog);
			if (error){sprintf (Info, "测试过载=%.2fdBm 失败", data.Over);bOK=FALSE;data.ErrorCode=ERR_TES_OVER;} 
			else	  {sprintf (Info, "测试过载=%.2fdBm 成功", data.Over);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		/***启动灵敏度测试放置在发端测试项目前**Eric.Yao**20130903***/
		// sens start
		if (bOK && my_struConfig.Sel_R_Sens)
		{
			strcpy (Info, "启动灵敏度测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_Start(channel, &SensTime, buf,prolog.Comments);
			if (error) {sprintf (Info, "启动灵敏度测试：失败, %s", buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS_START;}
			else	   {sprintf (Info, "启动灵敏度测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP(channel, &data);
			if (error){sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 失败",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率Aop1=%.2fdBm Aop2=%.2fdBm Aop3=%.2fdBm Aop4=%.2fdBm 成功",data.AOP[0],data.AOP[1],data.AOP[2],data.AOP[3]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//test TE
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE) && my_struConfig.Temper_Room) 
		{
			strcpy (Info, "测试TE...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTE(channel, &data);
			if (error) {sprintf (Info, "TE=%.2f 失败", data.TE);bOK=FALSE;}    
			else	   {sprintf (Info, "TE=%.2f 成功", data.TE);}    
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	
		// test tx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "发端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestTxPower(channel, &data);
			if (error) {strcpy (Info,"发端校准测试：失败");bOK=FALSE;data.ErrorCode=ERR_TES_CAL_TXPOWER;}  
			else	   {strcpy (Info,"发端校准测试：成功");}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT)  
		{
			strcpy (Info, "启动ONU传输性能测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_start(channel, &senstime_olt, buf);
			if (error) {sprintf (Info, "启动ONU传输性能测试：失败 %s", buf);bOK=FALSE;}
			else	   {sprintf (Info, "启动ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//TxOff Depth   
		if (bOK && my_struConfig.Sel_T_TxDis) 
		{
			strcpy (Info, "测试关断深度...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testTxOff_Depth(channel,&data);
			if (error) {sprintf (Info, "测试关断深度 失败");bOK=FALSE;data.ErrorCode=ERR_TES_TxOffPower;} 
			else	   {sprintf (Info, "测试关断深度 成功 TxOffPW1=%f TxOffPW2=%f TxOffPW3=%f TxOffPW4=%f",data.TxOffPower[0], data.TxOffPower[1], data.TxOffPower[2], data.TxOffPower[3]);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//Test Wavelength 
		if (bOK)
		{
			strcpy (Info, "测波长,切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock_TestWavelength(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}
		// er 眼图 mask 光谱等  
		if (bOK)
		{
			strcpy (Info, "切换光路...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testParameterLock(channel, &data, &prolog, panMain, gCtrl_BOX[channel], Info);
			if (error){bOK=FALSE;} //此处不需要显示相关结果，testParameterLock函数已经显示 
		}	 
		
		if (bOK)
		{
			strcpy (Info, "测试Ibias...");				 
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testIbias(channel, &data);
			if (error) {sprintf (Info, "测试Ibias=%.2fmA 失败", data.TxV);bOK=FALSE;data.ErrorCode=ERR_TUN_BIAS_MAX;}
			else	   {sprintf (Info, "测试Ibias=%.2fmA 成功", data.TxV);}  
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// 短路测试
		if (bOK && my_struConfig.DT_Test && my_struConfig.Temper_Room)
		{
			strcpy (Info, "开始短路测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = short_circuit_check(channel, &data);
			if (error){strcpy (Info, "短路测试：失败");bOK=FALSE;} 
			else	  {strcpy (Info, "短路测试：成功");}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 /*
		// 电流
		if (1)
		{
			if(!bOK)
			{
				strcpy(temp_Info,Info);
			}
			strcpy (Info, "测试电流...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testCurrent(channel, &data);
			if (error){sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 失败",data.RxI[0],data.TxI[0]);bOK=FALSE;data.ErrorCode=ERR_TES_CURRENT;} 
			else	  {sprintf (Info, "测试收端电流=%.2f 发端电流=%.2f 成功",data.RxI[0],data.TxI[0]);}	  	       
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
			strcpy (Info, "检查灵敏度测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testFixSens_End(channel, SensTime, &data, buf,prolog.Comments);
			if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败, %s", data.Sens, buf);bOK=FALSE;data.ErrorCode=ERR_TUN_SENS;} 
			else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		if (bOK && ((1 == my_struConfig.Sel_R_Sens_Detail) || (2 == my_struConfig.Sel_R_Sens_Detail))) 
		{
			strcpy (Info, "测试灵敏度值...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if(1 == my_struConfig.Sel_R_Sens_Detail)   //逼近法测SENS
			{
				error = testDetailSens (channel, &data);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
			else						  //外推法测误码率1.0e-12数量级SENS 
			{
				error = testDetailSens_Extra (channel, &data, &prolog);
				if (error) {sprintf (Info, "灵敏度测试=%.2fdBm 失败", data.Sens);bOK=FALSE;data.ErrorCode=ERR_TES_SENS;} 
				else	   {sprintf (Info, "灵敏度测试=%.2fdBm 成功", data.Sens);}       
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
			}	
		}
	
		// test sda sdd
		if (bOK && my_struConfig.Sel_R_SDHys)    			
		{
			strcpy (Info, "测试SDA、SDD...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			if (my_struConfig.isSDRealFlag)  
			{
				error = testDetailSD(channel, &data);          
			}
			else
			{
				error = testFixSD(channel, &data);
			}
			if (error) {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 失败", data.SDA, data.SDD);bOK=FALSE;}  
			else	   {sprintf (Info,"测试SDA=%.2fdBm SDD=%.2fdBm 成功", data.SDA, data.SDD);}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		// test rx cali
		if (bOK && my_struConfig.Sel_Calibrate_Test)    			
		{
			strcpy (Info, "收端校准测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = caltestRxPower(channel, &data, panMain);
			if (error) 
			{
				strcpy (Info,"收端校准测试：失败");
				if (data.ErrorCode==0) data.ErrorCode=error;  //将测试函数的错误代码赋值给error code
				bOK=FALSE; 
			}  
			else	   
			{
				strcpy (Info,"收端校准测试：成功");
			}         
			Insert_Info(panMain, gCtrl_BOX[channel], Info); 
		}
		
		if (bOK && my_struConfig.isONUtoOLT) 
		{
			strcpy (Info, "检查ONU传输性能测试结果...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = test_olt_errbit_end(channel, senstime_olt, buf);
			if (error) {sprintf (Info, "ONU传输性能测试：失败 %s", buf);bOK=FALSE;data.ErrorCode=ERR_TES_FIBER_LINK;}
			else	   {sprintf (Info, "ONU传输性能测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//eeprom protect check
		if (bOK && my_struConfig.Sel_EE_P && CHIEFCHIP_UX3328 != my_struDBConfig_ERCompens.ChiefChip)
		{
			strcpy (Info, "EEPROM写保护测试...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = ee_protect_check(channel);
			if (error) {strcpy (Info, "EEPROM写保护测试：失败");bOK=FALSE;}  
			else	   {strcpy (Info, "EEPROM写保护测试：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
/*	
		//update BurstMode
		if (bOK && my_struDBConfig_ERCompens.DriverChip==DiverType_VSC7965)
		{
			strcpy (Info, "更新BurstMode模式...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = UpdateBurstMode(channel);
			if (error) {strcpy (Info, "更新BurstMode模式：失败");bOK=FALSE;}  
			else	   {strcpy (Info, "更新BurstMode模式：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}	
		//update WorkMode
		if (bOK && my_struDBConfig_VSC7965.UpdatefOVERDFlag)
		{
			strcpy (Info, "更新VSC7967工作模式...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = UpdateWorkMode(channel);
			if (error)	{strcpy (Info, "更新VSC7967工作模式：失败");bOK=FALSE;}   
			else	    {strcpy (Info, "更新VSC7967工作模式：成功");}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
*/	
		if (my_struConfig.isQAUpdateSN)
		{				  
			// 扫描模块SN
			if (bOK && my_struConfig.Temper_Room)
			{
				strcpy (Info, "输入产品序列码...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = GetModuleSN(channel, &data, panSN);
				if (error) 
				{
					sprintf (Info,"输入产品序列码=%s 失败", data.ModuleSN);
					data.ErrorCode=ERR_TES_INPUT_SN; 
					bOK=FALSE; 
				}  
				else	   
				{
					sprintf (Info,"输入产品序列码=%s 成功", data.ModuleSN);
				}         
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
		
			// save eeprom
			if (bOK && my_struConfig.Temper_Room)    			
			{
				strcpy (Info, "保存EEPROM...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				error = SaveEEPROM(channel, &data);
				if (error) 
				{
					strcpy (Info,"保存EEPROM：失败");
					data.ErrorCode=ERR_TES_SAVE_EEPROM; 
					bOK=FALSE; 
				}  
				else	   
				{
					strcpy (Info,"保存EEPROM：成功");
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
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//检测模块是否在测试板上
/*		if (bMOD0)
		{
			strcpy (Info, "请将被测模块从测试系统取出...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				if (error) 
				{
					strcpy (Info, "检测测试板是否有模块出现异常");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "请将被测模块从测试系统取出...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		}  */

		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道调测已停止");
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
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		
		// AOP
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP_AginFront(channel, &data);
			if (error){sprintf (Info, "测试光功率=%.2fdBm 失败",data.AOP[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率=%.2fdBm 成功",data.AOP[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		// WaveLenhth
		if (bOK && my_struConfig.Sel_T_Wavelength && (SPECTRUM_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试波长...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testWaveLength_AginFront(channel, &data);
			if (error){sprintf (Info, "测试波长=%.2f nm 失败",data.PeakWL_DutyRatio99[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试波长=%.2f nm 成功",data.PeakWL_DutyRatio99[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道调测已停止");
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
	WaitFlag[channel] = FALSE;   // 启动提示信息刷新标志   	
	while (gFlag)
	{
		bOK=TRUE; 
		SaveData=FALSE;

		//检测模块是否在测试板上
		if (bOK)
		{
			if(!WaitFlag[channel])
			{
				strcpy (Info, "等待启动此通道测试系统...");
				Insert_Info(panMain, gCtrl_BOX[channel], Info);
				WaitFlag[channel] = TRUE;	
			}
			if (!Channel_Sel_Flag[channel]) 
			{
			//	strcpy (Info, "等待启动此通道测试系统...");
			//	Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE); 
				Delay(0.1);  
				continue;
			}
			//清除测试数据
			memset(&data,0,sizeof(data));
//			memset(&prolog,0,sizeof(prolog));
			
			//清除测试板是否有模块标示 
			ResetTextBox (panMain, gCtrl_BOX[channel], "");  
			errChk(SET_EVB_SHDN(channel, 1));
			strcpy (Info, "模块上电...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			//等待上电后稳定
			Delay(1);
			WaitFlag[channel] = FALSE;
		}
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		//LED
		SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_YELLOW);
		SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		
		//开始调测
	    ResetTextBox (panMain, gCtrl_BOX[channel], ""); 
		timestart=Timer();
		strcpy (Info, "开始进行测试...");
		Insert_Info(panMain, gCtrl_BOX[channel], Info);

		//如果要要检测TSSI或TXSD，先关闭电源
		if (my_struConfig.Sel_TxSD)
		{
			errChk(SET_EVB_SHDN(channel, 0)); 
			
			if (bOK && my_struConfig.Sel_TxSD) 
			{
				error = Init_TxSD_Detect(channel);	
				if (error) 	{strcpy (Info, "配置TxSD_Detect测试功能：失败");bOK=FALSE;}  
				else	    {strcpy (Info, "配置TxSD_Detect测试功能：成功");}                  
				Insert_Info(panMain, gCtrl_BOX[channel], Info); 
			}
			
			//重新开电
			errChk(SET_EVB_SHDN(channel, 1)); 
		}
		
		//初始化配置模块
		if (bOK)
		{
			strcpy (Info, "初始化模块...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = Init_Test2(channel, panMain, &data);
			if (error) 
			{
				strcpy (Info, "初始化模块：失败");bOK=FALSE;
				if (data.ErrorCode==0) data.ErrorCode=ERR_TES_NO_DEFINE;
			}
			else	  	
			{
				strcpy (Info, "初始化模块：成功");
			}                      										   
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
	 	//get bosasn
		if (bOK)
		{
			strcpy (Info, "读取条码...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = GetBosaSN(channel, &data);
			if (error){sprintf (Info, "读取条码=%s 失败", data.OpticsSN);bOK=FALSE;data.ErrorCode=ERR_TES_READ_INNERSN;}  
			else	  {sprintf (Info, "读取条码=%s 成功", data.OpticsSN);SaveData=TRUE;}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 		
		// aop
		if (bOK && my_struConfig.Sel_T_AOP && (PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testAOP_AginBlack(channel, &data);
			if (error){sprintf (Info, "测试光功率=%.2fdBm 失败",data.AOP[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试光功率=%.2fdBm 成功",data.AOP[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
 
		// WaveLenhth
		if (bOK && my_struConfig.Sel_T_Wavelength && (SPECTRUM_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "测试波长...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = testWaveLength_AginBlack(channel, &data);
			if (error){sprintf (Info, "测试波长=%.2f nm 失败",data.PeakWL_DutyRatio99[0]);bOK=FALSE;} 
			else	  {sprintf (Info, "测试波长=%.2f nm 成功",data.PeakWL_DutyRatio99[0]);}	  	       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//Tun AOP 确定Imax DAC
		if (bOK && my_struConfig.Sel_T_AOP&&(PowMeter_TYPE_NONE!=INSTR[channel].PM_TYPE))
		{
			strcpy (Info, "调试光功率...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = tuningAOP_AginBack(channel, &data);
			if (error) 
			{
				sprintf (Info, "调试光功率Aop4=%.2fdBm 失败", data.AOP[3]);
				bOK=FALSE;
				if (data.ErrorCode==0) 
				{
					data.ErrorCode=ERR_TUN_AOP; 
				}
			}
			else	  	
			{
				sprintf (Info, "调试光功率Aop4=%.2fdBm成功", data.AOP[3]);  
			}       
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		
		//测试日志赋值
		strcpy (prolog.PN, my_struConfig.PN); 	//PN
		strcpy (prolog.SN, data.OpticsSN);		//SN			
//		strcpy (prolog.Operator, my_struLicense.username);//Operator
		GetCtrlVal (panMain, PAN_MAIN_STR_OPERATOR, prolog.Operator);	   //考虑测试过程中进行'登录'，导致my_struLicense.username 变更，故改为界面获取； wenyao.xi 2016-5-16  
		prolog.ResultsID=0;    						//ResultsID
		//Log_Action  在测试函数开始位置赋值   
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
		//保存调试数据
		if (SaveData)
		{
			strcpy (Info, "保存调试数据...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			error = SaveDataBase(data, prolog);
			if (error) {strcpy (Info, "保存调试数据：失败");bOK=FALSE;data.ErrorCode=ERR_TES_SAVE_DATA;}  
			else	   {strcpy (Info, "保存调试数据：成功");}      
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
		}
		//显示运行结果
		prolog.RunTime=Timer()-timestart; 
		if (bOK)	//当测试成功不显示错误代码
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次", prolog.RunTime, data.FiberTime);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_ON_COLOR, VAL_GREEN);
			SetCtrlVal (panMain, gCtrl_LED[channel], 1);
		}
		else
		{
			sprintf (Info, "测试运行时间=%ds，\n光纤已使用次数=%d次，\n错误代码=%d", prolog.RunTime, data.FiberTime, data.ErrorCode);
			SetCtrlAttribute (panMain, gCtrl_LED[channel], ATTR_OFF_COLOR, VAL_RED);
			SetCtrlVal (panMain, gCtrl_LED[channel], 0);
		} 
		Insert_Info(panMain, gCtrl_BOX[channel], Info);
		
		//检测模块是否在测试板上
/*		if (bMOD0)
		{
			strcpy (Info, "请将被测模块从测试系统取出...");
			Insert_Info(panMain, gCtrl_BOX[channel], Info);
			do 
			{
				error = checktestbed(channel, &bMOD0);
				if (error) 
				{
					strcpy (Info, "检测测试板是否有模块出现异常");
					Insert_Info(panMain, gCtrl_BOX[channel], Info);  
				}
				else if (!bMOD0)	  	
				{
					break;
				} 
				else
				{
					strcpy (Info, "请将被测模块从测试系统取出...");
					Update_RESULTSInfo(panMain, gCtrl_BOX[channel], Info, FALSE);
				}
			} while (bMOD0 && gFlag);
		}  */

		Channel_Sel_Flag[channel]=FALSE;  
		errChk(SET_EVB_SHDN(channel, 0));
	}	
	
Error:
	Channel_Sel_Flag[channel]=FALSE;
	strcpy (Info, "此通道调测已停止");
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
					 MessagePopup ("提示", "此通道无效");
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
			//保存到配置文件
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
					//获取光纤序列号
					GetTableCellVal (panFiber, PAN_FIBER_TABLE, MakePoint (2, channel+1), buf);
					
					//如果光纤有变更，需要校准
					if (0!= stricmp(buf,INSTR[channel].Fiber)) 
					{
						//换光纤，强制校准收端和发端光路
						sRxCal.flag[channel] = FALSE;
						my_struTxCal.flag[channel]=FALSE; 
					}
					strcpy (INSTR[channel].Fiber, buf);
				}
			}
			
			//将校准参数保存到文件
			error = SetConfig_Inst ();
			if (error) return -1;
			
			//退出界面
			HidePanel (panFiber);    
			
			//更新主界面设备列表
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
			
			//获取通道
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
			
			//获取通道
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
			
			//获取通道
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
			
			//获取通道
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
			
			//获取通道
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
			
			//获取通道
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
			
			//获取通道
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
				//Get通道
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
				//Get通道
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

			//获取通道
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &channel); 
			
			//================发端校准参数=================//
			temp_delta=999.;
			for (i=0; i<3; i++)
			{
				GetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (1, i+1), &temp_st); 
				GetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, i+1), &temp_cal); 
		
				//获得最小差值
				temp = temp_st-temp_cal;
				if (temp<temp_delta)
				{
					temp_delta = temp;	
		
					if(temp_delta>my_struTxCal.power_max[channel] || temp_delta<my_struTxCal.power_min[channel])
					{
						sprintf (buf, "通道%d的校准光功率超出设置范围    ", channel);
						MessagePopup("提示", buf);
						return -1;
					}
				}
			}
			
			my_struTxCal.power[channel] = temp_delta;
			my_struTxCal.power_st[channel] = temp_st;

			my_struTxCal.power_array[channel*4+3] = temp_delta;
			my_struTxCal.power_array[channel*4+0] = temp_st;

			//保存校准结果到数据库
			DB_save_txcali(channel); 
		
			//update Flag
			my_struTxCal.flag[channel]=TRUE;
			//================发端校准参数=================// 
			//关闭所有通道电源    
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{   
				if (INSTR[channel].ChannelEn)
				{	 
					SET_EVB_SHDN(channel, 0);
				}
			}
			//将校准参数保存到文件
			err = SetCaliConfigFile ();
			if (err) 
			{
				return -1;
			}
			//清空校准结果显示
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

			//获取通道
			Radio_GetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, &channel); 
			
			//================发端校准参数=================//
			temp_delta=999.;
			for (i=0; i<3; i++)
			{
				GetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), &temp_st); 
				GetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), &temp_cal); 
		
				//获得最小差值
				temp = temp_st-temp_cal;
				if (temp<temp_delta)
				{
					temp_delta = temp;	
		
					if(temp_delta>my_struTxCal.minpower_max[channel] || temp_delta<my_struTxCal.minpower_min[channel])
					{
						sprintf (buf, "通道%d的校准光功率超出设置范围    ", channel);
						MessagePopup("提示", buf);
						return -1;
					}
				}
			}
			
			my_struTxCal.minpower[channel] = temp_delta;
			my_struTxCal.minpower_st[channel] = temp_st;

			my_struTxCal.minpower_array[channel*4+3] = temp_delta;
			my_struTxCal.minpower_array[channel*4+0] = temp_st;

			//保存校准结果到数据库
//			DB_save_txcali(channel); 
		
			//update Flag
			my_struTxCal.flag[channel]=TRUE;
			//================发端校准参数=================// 
			//关闭所有通道电源   
			for(channel=0;channel<CHANNEL_MAX;channel++)
			{
				if (INSTR[channel].ChannelEn) 
				{
				
					SET_EVB_SHDN(channel, 0);
				}
			}
			//将校准参数保存到文件
			err = SetCaliConfigFile ();
			if (err) 
			{
				return -1;
			}
			//清空校准结果显示
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

			//获取通道
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &channel); 
			
			//================收端校准参数=================//
			//判断校准数据是否正常
			GetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), &temp_cal);
			
			temp_cal= sRxCal.power_in[channel] - temp_cal;
			
			if(temp_cal>sRxCal.power_max[channel] || temp_cal<sRxCal.power_min[channel])
			{
				MessagePopup("提示", "收端校准光功率超出设置范围！"); 
				return -1;
			}
	
			//更新收端校准值
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
			//================收端校准参数=================//
			
			//将校准参数保存到文件
			err = SetCaliConfigFile ();
			if (err)
			{
				return -1;
			}
		
			//清空校准结果显示
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

			//获取通道
			Radio_GetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, &channel); 
			
			//================收端校准参数=================//
			//判断校准数据是否正常
			GetTableCellVal (panel, PN_CAL_OLT_TABLE_R, MakePoint (1, 1), &temp_cal);
			
			temp_cal= my_struCal_OLT.Power_In[channel] - temp_cal;
			
			if(temp_cal>my_struCal_OLT.Power_Max[channel] || temp_cal<my_struCal_OLT.Power_Min[channel])
			{
				MessagePopup("提示", "收端校准光功率超出设置范围！"); 
				return -1;
			}
	
			//更新收端校准值
		//	sRxCal.power[channel] = temp_cal; 
		//	sRxCal.power_array[channel*4+3] = temp_cal;
			//更新收端校准值
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
			//================收端校准参数=================//
			
			//将校准参数保存到文件
			err = SetCaliConfigFile ();
			if (err)
			{
				return -1;
			}
			//清空校准结果显示
			FillTableCellRange (phCalOLT, PN_CAL_OLT_TABLE_R, MakeRect (1, 1, 1, 1), val);
			//关闭当前通道电源
			err=SET_EVB_SHDN(channel, 0);
			if (err)
			{
				MessagePopup("提示", "关闭模块电源失败"); 
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

			//获取通道
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel); 
			
			GetCtrlVal (panel, PAN_CALOLT_NUM_SENS, &my_struCal_OLT.OLT_sens[channel]); 
			
			//判断校准数据是否正常
			if(my_struCal_OLT.OLT_sens[channel]>my_struCal_OLT.OLT_sens_max[channel] || my_struCal_OLT.OLT_sens[channel]<my_struCal_OLT.OLT_sens_min[channel])
			{MessagePopup("提示", "OLT灵敏度测试值超出设置范围！"); return -1;}
			
			//更新校准值数组
	 		my_struCal_OLT.OLT_sens_arr[channel*5] = my_struCal_OLT.OLT_sens[channel];  
			
			//退出校准界面
			HidePanel (panel);    
			
			//Power off
			err = SET_EVB_SHDN(channel, 0); 
			if (err) {MessagePopup("Error", "Set Power OFF error!");} 
			
			//更新OLT光路配置文件
//			my_struCal_OLT.Power_In[channel]=my_struCal_OLT.OLT_sens[channel]+my_struCal_OLT.OLT_Reserves[channel];
//			my_struCal_OLT.Array[channel*5+0] = my_struCal_OLT.Power_In[channel]; 
			
			//将校准参数保存到文件
			err = SetCaliConfigFile ();
			if (err) return -1;
			
			//暂时关闭重新校准功能
	//		my_struCal_OLT.flag=FALSE;
	//		MessagePopup("提示", "必须重新做OLT接受光路校准");
			
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_CALT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//当前通道切换
				channel=temp;   
				
				//开启当前通道电源
				errChk(SET_EVB_SHDN(channel, 1));
				EVB5_SET_BEN_Level(INST_EVB[channel], 0);
				SET_ATT_ONU (channel, -60); 
				//加载当前通道光路校准参数
				for (i=0; i<3; i++)
				{
					SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
					SetTableCellVal (panel, PAN_CALT_TABLE_T, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
				}
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			}   
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_CALT_M_RAD_CHAN_Min, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//当前通道切换
				channel=temp;   
				
				//开启当前通道电源
				errChk(SET_EVB_SHDN(channel, 1));
				SET_ATT_ONU (channel, -60); 
				if(EVB5_SET_BEN_Level(INST_EVB[channel], 1))
				{
					return -1;
				}
				//加载当前通道光路校准参数
				for (i=0; i<3; i++)
				{
					SetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
					SetTableCellVal (panel, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
				}
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			}   
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_CALR_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//当前通道切换
				channel=temp;   
				
				//开启当前通道电源
	//			errChk(SET_EVB_SHDN(channel, 1));
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
				SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PN_CAL_OLT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//当前通道切换
				channel=temp;   
				
				//开启当前通道电源
				errChk(SET_EVB_SHDN(channel, 1));
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
				SetTableCellVal (panel, PN_CAL_OLT_TABLE_R, MakePoint (1, 1), my_struCal_OLT.Power_In[channel]-my_struCal_OLT.Power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_OLT(channel, my_struCal_OLT.Power_In[channel]));
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					if (INSTR[channel].ChannelEn) 
					{
						SET_EVB_SHDN(channel, 0);
					}
				}
			
				//当前通道切换
				channel=temp;   
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_IN, my_struCal_OLT.OLT_sens_in[channel]);
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MIN, my_struCal_OLT.OLT_sens_min[channel]); 
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS_MAX, my_struCal_OLT.OLT_sens_max[channel]); 
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_SENS, my_struCal_OLT.OLT_sens[channel]);
				SetCtrlVal (phCalibrateOLT, PAN_CALOLT_NUM_P_RESERVES, my_struCal_OLT.OLT_Reserves[channel]); 
				
				//开启当前通道电源
				errChk(SET_EVB_SHDN(channel, 1));
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
//				SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, sRxCal.power_in[channel]));
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			//读取校准参数
			errChk(GetCaliConfigFile());
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_SMC_RAD_CHAN, &temp);
			
			if (INSTR[temp].ChannelEn)
			{
				//关闭所有通道电源   
				for(channel=0;channel<CHANNEL_MAX;channel++)
				{
					SET_EVB_SHDN(channel, 0);
				}
			
				//当前通道切换
				channel=temp;   
				SetCtrlVal (phSMC, PAN_SMC_NUM_SENSVALUE, my_struCal_OLT.OLT_sens[channel]);
				SetCtrlVal (phSMC, PAN_SMC_NUM_SENSTIME, my_struCal_OLT.time[channel]);
				//开启当前通道电源
				errChk(SET_EVB_SHDN(channel, 1));
				
				//调试过程未启用
	//			if (INSTR[channel].ATT_TYPE == ATT_TYPE_NONE)
	//			{
	//				sprintf (buf, "通道%d 没有选择衰减器型号，不能进行收端校准    ", channel);
	//				MessagePopup("提示", buf);
	//				error=-1;
	//				goto Error;
	//			} 
			
	//			SetTableCellVal (panel, PAN_CALR_TABLE_R, MakePoint (1, 1), sRxCal.power_in[channel]-sRxCal.power[channel]); 
		
				errChk(Init_BERT(channel));
	
				errChk(SET_ATT_ONU(channel, my_struCal_OLT.OLT_sens[channel]));
			}
			else
			{
				//默认选择第一个使用的通道
				errChk(GetEnableChannel(&channel));
				MessagePopup ("提示", "被选通道未启用    ");
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
			
			//判定通道是否可用
			Radio_GetMarkedOption(panel, PAN_CALOLT_RAD_CHAN, &channel);
			
			if (!my_struCal_OLT.flag && INSTR[channel].ATT_TYPE_OLT != ATT_TYPE_NONE) 
			{
				MessagePopup("提示", "请先完成OLT接受光路校准");
				return -1;
			}  
			SET_EVB_SHDN(channel, 1); 
			 Delay(1.5);
			
			SetCtrlVal (panel, PAN_CALOLT_NUM_SENS, 0.); 
			
			error = testDetailSens_OLT (channel, panel);
			if (error) {MessagePopup("Error", "测试OLT灵敏度异常"); return -1;} 
			
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
			
			//判定通道是否可用
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
		
		
	//默认选择第一个使用的通道
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalibrateOLT, PAN_CALOLT_RAD_CHAN, channel);

	errChk(SET_EVB_SHDN(channel, 1));	
	
	if (INSTR[channel].ATT_TYPE_ONU == ATT_TYPE_NONE)
	{
		MessagePopup("提示", "没有选择衰减器型号，不能进行收端校准！");
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
		
		
	//默认选择第一个使用的通道
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
			SET_ATT_ONU (channel, -60);	  	//设置-60功率     
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

	//清空校准结果显示
	FillTableCellRange (phCalT_Min, PAN_CALT_M_TABLE_T_Min, MakeRect (1, 1, 3, 2), val);
	
	//默认选择第一个使用的通道
	errChk(GetEnableChannel(&channel));
	
	Radio_SetMarkedOption (phCalT_Min, PAN_CALT_M_RAD_CHAN_Min, channel);

	errChk(SET_EVB_SHDN(channel, 1));
	SET_ATT_ONU (channel, -60);
	EVB5_SET_BEN_Level(INST_EVB[channel], 1);

	//===================加载发端校准记录===================//
	for (i=0; i<3; i++)
	{
		SetTableCellVal (phCalT, PAN_CALT_M_TABLE_T_Min, MakePoint (1, i+1), my_struTxCal.power_st[channel]); 
		SetTableCellVal (phCalT, PAN_CALT_M_TABLE_T_Min, MakePoint (2, i+1), my_struTxCal.power_st[channel]-my_struTxCal.power[channel]); 
	}
	//===================加载发端校准记录===================//
	
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
			// 判断模块是否上电，没上电则上电
			// 此时根据通道判断控件，获取控件状态以判断上电状态，暂时默认0通道
			GetCtrlVal(panel, PAN_MAIN_BIN_CHAN0, &power_on_flag);
			if (power_on_flag != 1)
			{
				MessagePopup("错误", "当前模块没有上电!");
				return -1;
			}
			// 输入密码
			error1 = DWDM_ONU_DS4830_Enter_Password (INST_EVB[0]);
			if (error1) 
			{
				MessagePopup("错误", "3328设置工厂模式错误   "); 
				return -1;
			}
			
			// 获取设置的DAC
			GetCtrlVal (panel, PAN_OSA_STRING_DAC0, &dac0);
			m_MyDACData[0] = dac0;
			// DAC信息下发到硬件
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[0], dac0)<0) 
			{
				return -1;
			}
			Delay(0.2);
			// 人为看波长信息；读波长的函数Read_Wavelength
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
			// 判断模块是否上电，没上电则上电
			// 此时根据通道判断控件，获取控件状态以判断上电状态，暂时默认0通道
			GetCtrlVal(panel, PAN_MAIN_BIN_CHAN0, &power_on_flag);
			if (power_on_flag != 1)
			{
				MessagePopup("错误", "当前模块没有上电!");
				return -1;
			}
			// 输入密码
			error1 = DWDM_ONU_DS4830_Enter_Password (INST_EVB[0]);
			if (error1) 
			{
				MessagePopup("错误", "3328设置工厂模式错误   "); 
				return -1;
			}
			
			// 获取设置的DAC
			GetCtrlVal (panel, PAN_OSA_STRING_DAC1, &dac1);
			m_MyDACData[1] = dac1;
			// DAC信息下发到硬件
			if(DWDM_ONU_DS4830_Set_TEC_DAC7_Adjust7 (INST_EVB[0], dac1)<0) 
			{
				return -1;
			}
			Delay(0.2);
			// 人为看波长信息；读波长的函数Read_Wavelength
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
			// 将m_MyDACData的数据保存到数据库
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
	// 连接数据库
	int err;	
	err = MyDLL_DB_Init (&hdbc1); 
	if (err) 
	{
		MessagePopup("提示","数据库初始化失败！");
		return ;
	}
	// 下发查询语句
	err = DBActivateSQL(hdbc1,buf);
	if (err <= 0)
	{
		ShowDataBaseError();
		return;
	}
}
