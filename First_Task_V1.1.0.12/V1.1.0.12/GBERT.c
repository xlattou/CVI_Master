#include <formatio.h>
#include <utility.h>
#include <visa.h>
#include <userint.h>
#include <ansi_c.h>
#include <toolbox.h> 
#include "GBERT_DLL.h"  
#include "GBERT.h"  

BOOL GBERT_Init(int *GBERT_USBHandle, unsigned char *GBERT_SerialNumber, int Pattern, int Rate, int myBERT_SFPTxDisable)
{
	int GBERT_ErrorCode;
	enum emum_BERT_Pattern BERT_Pattern;
	enum emum_BERT_Application BERT_Application;
	enum emum_BERT_CheckerMode BERT_CheckerMode; 
	double BERT_GatingTime;

	int BERT_SFPout_en;
	int BERT_SFPin_en;
	int BERT_SMAout_en;

	int BERT_InputPolarity; 
	int BERT_OutputPolarity; 
	int BERT_SFPTxDisable;
	int BERT_InputEqualization; 
	int BERT_OutputSwing;  
	int BERT_OutputSlewRate;	
	
	//step1, GBERT selftest
	GBERT_ErrorCode = GBERT_Selftest_withSN_DLL (GBERT_SerialNumber, GBERT_USBHandle);
	if (GBERT_ErrorCode<0) return FALSE;
	
	//step2, GBERT initialize
//	BERT_Pattern           = PRBS7;
//	BERT_Application       = R1250Mbps;
	BERT_Pattern           = Pattern;
	BERT_Application       = Rate;
	BERT_CheckerMode       = SingleRun; 
	BERT_SFPin_en          = 0; //SFP input disable, so SMA input enable
	BERT_SFPout_en         = 1; //output enable
	BERT_SMAout_en         = 1; //output enable 
	BERT_SFPTxDisable      = myBERT_SFPTxDisable; //output enable 
	BERT_InputPolarity     = 0; //not invert
	BERT_OutputPolarity    = 0; //not invert
	BERT_InputEqualization = 3; //0 dB
	BERT_OutputSwing       = 0; //small 
	BERT_OutputSlewRate    = 0; //fast
	

	GBERT_ErrorCode = GBERT_Initialize_DLL(*GBERT_USBHandle,
				 BERT_Pattern,  BERT_Application, BERT_CheckerMode,
				 BERT_SFPin_en,  BERT_SFPout_en, BERT_SMAout_en,  BERT_SFPTxDisable,
				 BERT_OutputPolarity,  BERT_InputPolarity,
				 BERT_InputEqualization,  BERT_OutputSwing, BERT_OutputSlewRate);
	if (GBERT_ErrorCode<0) return FALSE; 
	
	//step3, GBERT single run
	//BERT start check
	BERT_GatingTime = 1.0;//1s
	GBERT_ErrorCode = GBERT_StartCheck_DLL(*GBERT_USBHandle, BERT_GatingTime);
	if (GBERT_ErrorCode<0) return FALSE; 
	
	return TRUE;
}

BOOL GBERT_Init_SFPTxDisable(int *GBERT_USBHandle, unsigned char *GBERT_SerialNumber, int myBERT_SFPTxDisable)
{
	int GBERT_ErrorCode;

	enum emum_BERT_Pattern BERT_Pattern;
	enum emum_BERT_Application BERT_Application;
	enum emum_BERT_CheckerMode BERT_CheckerMode; 
	double BERT_GatingTime;

	int BERT_SFPout_en;
	int BERT_SFPin_en;
	int BERT_SMAout_en;

	int BERT_InputPolarity; 
	int BERT_OutputPolarity; 
	int BERT_SFPTxDisable;
	int BERT_InputEqualization; 
	int BERT_OutputSwing;  
	int BERT_OutputSlewRate;	 
	
	//step1, GBERT selftest
	GBERT_ErrorCode = GBERT_Selftest_withSN_DLL (GBERT_SerialNumber, GBERT_USBHandle);
	if (GBERT_ErrorCode<0) return FALSE;
	
	//step2, GBERT initialize
	BERT_Pattern           = PRBS7;
	BERT_Application       = R1250Mbps;
	BERT_CheckerMode       = SingleRun; 
	BERT_SFPin_en          = 0; //SFP input disable, so SMA input enable
	BERT_SFPout_en         = 1; //output enable
	BERT_SMAout_en         = 1; //output enable 
	BERT_SFPTxDisable      = myBERT_SFPTxDisable; //output enable 
	BERT_InputPolarity     = 0; //not invert
	BERT_OutputPolarity    = 0; //not invert
	BERT_InputEqualization = 3; //0 dB
	BERT_OutputSwing       = 0; //small 
	BERT_OutputSlewRate    = 0; //fast
	
	GBERT_ErrorCode = GBERT_Initialize_DLL(*GBERT_USBHandle,
				 BERT_Pattern,  BERT_Application, BERT_CheckerMode,
				 BERT_SFPin_en,  BERT_SFPout_en, BERT_SMAout_en,  BERT_SFPTxDisable,
				 BERT_OutputPolarity,  BERT_InputPolarity,
				 BERT_InputEqualization,  BERT_OutputSwing, BERT_OutputSlewRate);
	if (GBERT_ErrorCode<0) return FALSE; 
	
	//step3, GBERT single run
	//BERT start check
	BERT_GatingTime = 1.0;//1s
	GBERT_ErrorCode = GBERT_StartCheck_DLL(*GBERT_USBHandle, BERT_GatingTime);
	if (GBERT_ErrorCode<0) return FALSE; 
	
	return TRUE;
}

BOOL GBERT_Start(int GBERT_USBHandle, double testTime)
{		
	double BERT_GatingTime;
	int GBERT_ErrorCode;     
	
	BERT_GatingTime=testTime;
	GBERT_ErrorCode = GBERT_StartCheck_DLL(GBERT_USBHandle, BERT_GatingTime);
	if (GBERT_ErrorCode<0) return FALSE;
	
	return TRUE;
}

int GBERT_READ_Status (int inst_GBERT, unsigned int *ErrorBit)
{
	int GBERT_ErrorCode;   
	double BERT_Accumulated_ErrorRatio;
	double BERT_Accumulated_BitCount;
	double BERT_Accumulated_ErrorCount;
	double BERT_Accumulated_ElapsedTime;
	
	GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(inst_GBERT, &BERT_Accumulated_ErrorRatio, 
							&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
	
	if (GBERT_ErrorCode<0) {*ErrorBit=99;}
	else   				   {*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;}

	return 0;
}

BOOL GBERT_Check_Start(int GBERT_USBHandle, unsigned int *ErrorBit)
{
int count, errnum;

	int GBERT_ErrorCode;  
	double BERT_Accumulated_ErrorRatio;
	double BERT_Accumulated_BitCount;
	double BERT_Accumulated_ErrorCount;
	double BERT_Accumulated_ElapsedTime;
	double BERT_GatingTime = 1.0;  

	count=0;*ErrorBit=1;
	do
	{
		GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(GBERT_USBHandle, &BERT_Accumulated_ErrorRatio, 
							&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
		
		if (GBERT_ErrorCode<0)  Delay(0.5);	//GBERT_ErrorCode<0 状态读取有错误 延时1s后等待下一次循环
		
		count++;
		
		if (count>3) break;			//防止死循环,强制5次循环后退出
	} while (GBERT_ErrorCode<0);  	
	
	*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;
	
	//测试状态有问题，重新初始化BERT
	if (GBERT_ErrorCode<0) 
	{
		errnum=0;
		do
		{
			if(!GBERT_RESET(GBERT_USBHandle)) {MessagePopup("Error", "Reset GBERT error!");return FALSE;}
			Delay(1);
			if(!GBERT_Start(GBERT_USBHandle, BERT_GatingTime)) {MessagePopup("Error", "Reset GBERT error!");return FALSE;}
			Delay(1.5); 

			count=0;*ErrorBit=1;
			do
			{
				GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(GBERT_USBHandle, &BERT_Accumulated_ErrorRatio, 
									&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
		
				if (GBERT_ErrorCode==1)  Delay(1);	//GBERT_ErrorCode==1 一个测试周期没有完成 延时1s后等待下一次循环
		
				count++;
		
				if (count>5) break;			//防止死循环,强制5次循环后退出
			} while (GBERT_ErrorCode==1);  	//GBERT_ErrorCode==1 一个测试周期没有完成
			
			errnum++;
			
		}while (GBERT_ErrorCode<0 && errnum<3);

		if (GBERT_ErrorCode<0) {*ErrorBit=99;return FALSE;}
		else   				   {*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;}
	}

	return TRUE;
}

BOOL GBERT_Check(int GBERT_USBHandle, unsigned int *ErrorBit)
{
int count, errnum;

	int GBERT_ErrorCode;  
	double BERT_Accumulated_ErrorRatio;
	double BERT_Accumulated_BitCount;
	double BERT_Accumulated_ErrorCount;
	double BERT_Accumulated_ElapsedTime;
	double BERT_GatingTime = 1.0; 

	count=0;*ErrorBit=1;
	do
	{
		GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(GBERT_USBHandle, &BERT_Accumulated_ErrorRatio, 
							&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
		
		if (GBERT_ErrorCode==1)  Delay(0.5);	//GBERT_ErrorCode==1 一个测试周期没有完成 延时1s后等待下一次循环
		
		count++;
		
		if (count>3) break;			//防止死循环,强制5次循环后退出
	} while (GBERT_ErrorCode==1);  	//GBERT_ErrorCode==1 一个测试周期没有完成
	
	*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;

	//测试状态有问题，重新初始化BERT
	if (GBERT_ErrorCode<0) 
	{
		errnum=0;
		do
		{
		//	if(!GBERT_RESET(GBERT_USBHandle)) {MessagePopup("Error", "Reset GBERT error!");return FALSE;}
		//	Delay(1);
		//	if(!GBERT_Start(GBERT_USBHandle, BERT_GatingTime)) {MessagePopup("Error", "Reset GBERT error!");return FALSE;}
		//	Delay(1.5); 

			count=0;*ErrorBit=1;
			do
			{
				GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(GBERT_USBHandle, &BERT_Accumulated_ErrorRatio, 
									&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
		
				if (GBERT_ErrorCode==1)  Delay(1);	//GBERT_ErrorCode==1 一个测试周期没有完成 延时1s后等待下一次循环
		
				count++;
		
				if (count>5) break;			//防止死循环,强制5次循环后退出
			} while (GBERT_ErrorCode==1);  	//GBERT_ErrorCode==1 一个测试周期没有完成
			
			errnum++;
			
		}while (GBERT_ErrorCode<0 && errnum<3);

		if (GBERT_ErrorCode<0) {*ErrorBit=99;return FALSE;}
		else   				   {*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;}
	}
	
	return TRUE;
}

BOOL GBERT_Stop(int GBERT_USBHandle)
{
	int GBERT_ErrorCode;    
	
	GBERT_ErrorCode = GBERT_StopCheck_DLL(GBERT_USBHandle);
	if (GBERT_ErrorCode<0) return FALSE;
	
	return TRUE;
}

BOOL GBERT_RESET(int GBERT_USBHandle)
{
	int GBERT_ErrorCode;    
	enum emum_BERT_Pattern BERT_Pattern = PRBS7;
	enum emum_BERT_Application BERT_Application=R1250Mbps;
	enum emum_BERT_CheckerMode BERT_CheckerMode = SingleRun;  
	int BERT_SFPin_en=0;
	int BERT_SFPout_en=1;
	int BERT_SMAout_en=1;
	int BERT_SFPTxDisable=0;
	int BERT_OutputPolarity=0;
	int BERT_InputPolarity=0;
	int BERT_InputEqualization=3; 
	int BERT_OutputSwing=0;
	int BERT_OutputSlewRate=0;      
	
	GBERT_ErrorCode = GBERT_Initialize_DLL(GBERT_USBHandle,
				 BERT_Pattern,  BERT_Application, BERT_CheckerMode,
				 BERT_SFPin_en,  BERT_SFPout_en, BERT_SMAout_en,  BERT_SFPTxDisable,
				 BERT_OutputPolarity,  BERT_InputPolarity,
				 BERT_InputEqualization,  BERT_OutputSwing, BERT_OutputSlewRate);
	if (GBERT_ErrorCode<0) return FALSE; 
	
	Delay(0.5);
	
	return TRUE;
}

BOOL GBERT_Check_Ratio(int GBERT_USBHandle, double *ErrorRatio, int *errorcode)
{
	int errnum;
	
	int GBERT_ErrorCode;
	double BERT_Accumulated_ErrorRatio;
	double BERT_Accumulated_BitCount;
	double BERT_Accumulated_ErrorCount;
	double BERT_Accumulated_ElapsedTime;

	GBERT_ErrorCode = GBERT_GetCheckStatus_DLL(GBERT_USBHandle, &BERT_Accumulated_ErrorRatio, 
					&BERT_Accumulated_BitCount, &BERT_Accumulated_ErrorCount, &BERT_Accumulated_ElapsedTime);  
		 
//	*ErrorBit=(unsigned int)BERT_Accumulated_ErrorCount;
	*ErrorRatio = BERT_Accumulated_ErrorRatio;
	*errorcode = GBERT_ErrorCode;
	
	return TRUE;     
}

int GBERT_GET_SN(int panel, int control)
{
	int error=0, num=0;
	
	int GMeter_USBHandle;
	char GMeter_SerialNumber[17];               
	
	ClearListCtrl (panel, control);
	num=0;
	for (GMeter_USBHandle=0; GMeter_USBHandle<256; GMeter_USBHandle++)
	{	error = GMeter_GetSN_DLL(GMeter_USBHandle, GMeter_SerialNumber);
		if ((!error)&&(0 == strncmp(GMeter_SerialNumber, "GB", 2)))
		{   
			num++;
			InsertListItem (panel, control, -1, GMeter_SerialNumber, num);
		}
	}
	return 0;
}

int	GVPM_Init (int *GVPM_USBHandle, unsigned char *GOAM_SerialNumber, int Wavelength, float AttValue)
{
	int error;
	char info[500];
	
	int GOAM_WaveLength1, GOAM_WaveLength2, GOAM_WaveLength3, GOAM_WaveLength4;
	double GOAM_MaxAttenuateValue1, GOAM_MaxAttenuateValue2, GOAM_MaxAttenuateValue3, GOAM_MaxAttenuateValue4;
	int GOAM_OutputEnable;
	int GOAM_WaveLength;
	double GOAM_Attenuate;
	double GOAM_Temperature;
	int GOAM_ErrorCode;  
	
	error = GOAM_Selftest_DLL (GOAM_SerialNumber, GVPM_USBHandle);
	if (error<0) 
	{
		sprintf (info, "GVPM selftest fail, error code:%d", error);
		MessagePopup ("Error", info);
		return -1; 
	}
	
	error = GOAM_Initialize_DLL (*GVPM_USBHandle, &GOAM_Temperature, &GOAM_OutputEnable, &GOAM_WaveLength, &GOAM_Attenuate,
								&GOAM_WaveLength1, &GOAM_MaxAttenuateValue1, &GOAM_WaveLength2, &GOAM_MaxAttenuateValue2, 
								&GOAM_WaveLength3, &GOAM_MaxAttenuateValue3, &GOAM_WaveLength4, &GOAM_MaxAttenuateValue4);
	if (error<0) 
	{
		sprintf (info, "GVPM initial fail, error code:%d", error);
		MessagePopup ("Error", info);
		return -1; 
	}
	
	if (Wavelength==1490) Wavelength=1480; 
	GOAM_WaveLength=Wavelength;
	
	GVPM_SET_OutputEnable (*GVPM_USBHandle, 1);
		
	GOAM_Attenuate =AttValue; 
	error = GOAM_Attenuate_DLL(*GVPM_USBHandle, GOAM_WaveLength, GOAM_Attenuate);
	if(error==-5)	//第一次设置出现-5的错误，表明此衰减器支持lock 模式输出，并且当前正是lock模式，必须先设置到free模式
	{
		GOAM_ErrorCode = GVPM_SET_LockEnable (*GVPM_USBHandle, 0);
		if (GOAM_ErrorCode) return -1;
		
		//设置为free模式后，重新再试一次设置衰减量
		error = GOAM_Attenuate_DLL(*GVPM_USBHandle, GOAM_WaveLength, GOAM_Attenuate);
	}
	if (error<0) 
	{
		sprintf (info, "GVPM set attenuate fail, error code:%d", error);
		MessagePopup ("Error", info);
		return -1; 
	}	
	
	return 0;
}

int	GVPM_SET_Attenuate (int GVPM_USBHandle, float AttValue, int Wavelength)
{
	int error;
	char info[500]; 
	
	int GOAM_WaveLength;
	double GOAM_Attenuate;
	
	if (Wavelength==1490) Wavelength=1480; 
	GOAM_WaveLength=Wavelength;
	
	GOAM_Attenuate =AttValue; 
	error = GOAM_Attenuate_DLL(GVPM_USBHandle, GOAM_WaveLength, GOAM_Attenuate);
	if (error<0) 
	{
		sprintf (info, "GVPM set attenuate fail, error code:%d", error);
		MessagePopup ("Error", info);
		return -1; 
	}	
	
	error = GVPM_SET_OutputEnable (GVPM_USBHandle, 1);
	if (error<0) {MessagePopup ("Error", "GOAM set OutputEnable fail");return -1;}  
	
	return 0;
}

int	GVPM_SET_OutputEnable (int GVPM_USBHandle, int GOAM_OutputEnable)
{
	int error;
	char info[500];   
	
	error = GOAM_OutputEnable_DLL(GVPM_USBHandle, GOAM_OutputEnable);
	if (error<0) 
	{
		sprintf (info, "GVPM set OutputEnable fail, error code:%d", error);
		MessagePopup ("Error", info);
		return -1; 
	}	
	
	return 0;
}

int GVPM_SET_LockEnable (int inst_GVPM, int lockEnable)
{
	char info[500];
	
	int GOAM_ErrorCode;
	
	GOAM_ErrorCode = GOAM_LockEnable_DLL(inst_GVPM, lockEnable);
	if (GOAM_ErrorCode<0) 
	{
		sprintf (info, "GVPM set lock enable fail, error code:%d", GOAM_ErrorCode);
		MessagePopup ("Error", info);
		return -1; 
	}	
	
	return 0;
}

int GVPM_SET_LockAttenuate (int inst_GVPM, double LockAttValue, int Wavelength, int checklock)
{
	char info[500];
	int  count, error=0;
	
	int GOAM_WaveLength; 
	int GOAM_ErrorCode;    
	int GOAM_LockEnable;
	double GOAM_LockOPdBm;
	double GOAM_Attenuate;
	double GOPM_OPdBm; 
	double GOPM_OPmW;
	
	int index = 0;
	
	if (Wavelength==1490) Wavelength=1480; 
	GOAM_WaveLength=Wavelength;
	
	//必须放在GOAM_LockAttenuateInit_DLL前面
/*	error = GVPM_SET_OutputEnable (inst_GVPM, 1);
	if (error<0) {MessagePopup ("Error", "GVPM set OutputEnable fail");return -1;}  
	
	GOAM_ErrorCode = GOAM_LockAttenuateInit_DLL (inst_GVPM, GOAM_WaveLength, LockAttValue);
	if (GOAM_ErrorCode<0) 
	{
		sprintf (info, "GVPM set lock attenuate fail, error code:%d", GOAM_ErrorCode);
		MessagePopup ("Error", info);
		return -1; 
	}
	
	if (checklock)
	{
		count=0;
		do
		{
			GOAM_ErrorCode = GOAM_GOPM_LockOutput_GetStatus_DLL (inst_GVPM, &GOAM_WaveLength, &GOAM_LockOPdBm, &GOAM_LockEnable, &GOAM_Attenuate, &GOPM_OPdBm, &GOPM_OPmW);
			if (GOAM_ErrorCode>0 || GOAM_ErrorCode==-5 || GOAM_ErrorCode==-6 || GOAM_ErrorCode==-7 || (fabs(LockAttValue - GOPM_OPdBm)>0.2))	*/   /***增加锁定衰减光功率值检查**Eric.Yao**20130619***/
/*			{
				Delay (0.2);
				count++;
			}   
		} while (count<15 && (GOAM_ErrorCode>0 || GOAM_ErrorCode==-5 || GOAM_ErrorCode==-6 || GOAM_ErrorCode==-7 || (fabs(LockAttValue - GOPM_OPdBm)>0.2)));
	
		if (GOAM_ErrorCode!=0) 
		{
			sprintf (info, "GVPM get lock attenuate status, error code:%d", GOAM_ErrorCode);
			MessagePopup ("Error", info);
			return -1; 
		}
		
		if (fabs(LockAttValue - GOPM_OPdBm)>0.2)
		{
			sprintf (info, "GVPM Set LockAttValue = %.2f not the target LockAttValue = %.2f", GOPM_OPdBm, LockAttValue);
			MessagePopup ("Error", info);
			return -1; 
		}
	}
*/
	
	/***因设置锁定衰减存在锁定状态OK，但是锁定的光功率值并非设定值的情况，当失败时，程序重新设置，共循环三次，如果仍然失败，则认为锁定不成功**Eric.Yao**20130627***/
	if (checklock)
	{
		for (index = 0; index < 3; index++)	/***设置次数由3次修改为20次**Eric.Yao**20130906***/
		{
			error = GVPM_SET_OutputEnable (inst_GVPM, 1);
			if (error<0) {MessagePopup ("Error", "GVPM set OutputEnable fail");return -1;}  
		
			GOAM_ErrorCode = GOAM_LockAttenuateInit_DLL (inst_GVPM, GOAM_WaveLength, LockAttValue);
			if (GOAM_ErrorCode<0) 
			{
				sprintf (info, "GVPM set lock attenuate fail, error code:%d", GOAM_ErrorCode);
				MessagePopup ("Error", info);
				return -1; 
			}
		
			count=0;
			do
			{
				GOAM_ErrorCode = GOAM_GOPM_LockOutput_GetStatus_DLL (inst_GVPM, &GOAM_WaveLength, &GOAM_LockOPdBm, &GOAM_LockEnable, &GOAM_Attenuate, &GOPM_OPdBm, &GOPM_OPmW);
				if (GOAM_ErrorCode>0 || GOAM_ErrorCode==-5 || GOAM_ErrorCode==-6 || GOAM_ErrorCode==-7 || (fabs(LockAttValue - GOPM_OPdBm)>0.2))	   /***增加锁定衰减光功率值检查**Eric.Yao**20130619***/
				{
					Delay (0.2);
					count++;
				}   
			} while (count<15 && (GOAM_ErrorCode>0 || GOAM_ErrorCode==-5 || GOAM_ErrorCode==-6 || GOAM_ErrorCode==-7 || (fabs(LockAttValue - GOPM_OPdBm)>0.2)));
		
			if ((0 == GOAM_ErrorCode) && (fabs(LockAttValue - GOPM_OPdBm)<0.2))   /***当错误代码为0，并且衰减器读出来的光功率值与设定值相等，则认为设置锁定OK**Eric.Yao**20130627***/
			{
				break;
			} 
		}

		if (GOAM_ErrorCode!=0) 
		{
			sprintf (info, "GVPM get lock attenuate status, error code:%d", GOAM_ErrorCode);
			MessagePopup ("Error", info);
			return -1; 
		}
			
		if (fabs(LockAttValue - GOPM_OPdBm)>0.2)
		{
			sprintf (info, "GVPM Set LockAttValue = %.2f not the target LockAttValue = %.2f", GOPM_OPdBm, LockAttValue);
			MessagePopup ("Error", info);
			return -1; 
		}
	}
	else  /***不检查是否锁定则程序直接调测循环**Eric.Yao**20130627***/
	{
		//必须放在GOAM_LockAttenuateInit_DLL前面
		error = GVPM_SET_OutputEnable (inst_GVPM, 1);
		if (error<0) {MessagePopup ("Error", "GVPM set OutputEnable fail");return -1;}  
	
		GOAM_ErrorCode = GOAM_LockAttenuateInit_DLL (inst_GVPM, GOAM_WaveLength, LockAttValue);
		if (GOAM_ErrorCode<0) 
		{
			sprintf (info, "GVPM set lock attenuate fail, error code:%d", GOAM_ErrorCode);
			MessagePopup ("Error", info);
			return -1; 
		}			
	}

	return 0; 
}

int GVPM_GET_SN(int panel, int control)
{
	int error=0, num=0;
	
	int GMeter_USBHandle; 
	char GMeter_SerialNumber[17];         
	
	ClearListCtrl (panel, control);
	num=0;
	for (GMeter_USBHandle=0; GMeter_USBHandle<256; GMeter_USBHandle++)
	{	error = GMeter_GetSN_DLL(GMeter_USBHandle, GMeter_SerialNumber);
		if ((!error)&&(0 == strncmp(GMeter_SerialNumber, "GVPM", 4)))
		{   
			num++;
			InsertListItem (panel, control, -1, GMeter_SerialNumber, num);
		}
	}
	return 0;
}
