#include "global.h"
#include <formatio.h>
#include <utility.h>
#include <visa.h>
#include <userint.h>
#include <ansi_c.h>
#include <toolbox.h> 

static unsigned char	Temp_str[256]; 
static BOOL   			Status; 

BOOL Ag86100_Init(ViSession *instHandle, ViRsrc Viname)
{
ViSession defaultRM;

	//Open instruments resource
    viOpenDefaultRM (&defaultRM);
    							 
    Status = viOpen (defaultRM, Viname, VI_NULL, VI_NULL, instHandle);   //Viname "GPIB0::8::INSTR"
    if(Status !=VI_SUCCESS) return FALSE;
 
	Status = viSetAttribute(*instHandle, VI_ATTR_TMO_VALUE, 5000);  
    if(Status !=VI_SUCCESS) return FALSE; 
    
    if (!Visa_Write(*instHandle, "*RST")) return FALSE;
	if (!Visa_Write(*instHandle, "*WAI")) return FALSE;
	if (!Visa_Write(*instHandle, ":SYSTem:HEADer OFF")) return FALSE;
	if (!Visa_Write(*instHandle, ":SYST:MODE EYE")) return FALSE; 
	if (!Visa_OPC(*instHandle, 3)) return FALSE;
	
	return TRUE;
}

BOOL Ag86100_SET_O(ViSession instHandle, const double ERFactor, double X_Scale, double X_Position, double Y_Scale, double Y_Offset, int Filter, int wavelength, char *MaskName, double MaskMargin, int Channel_O)
{
	//设置通道
	sprintf (Temp_str, "CHANNEL%d:DISPLAY ON", Channel_O); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE;     
	
	//设置Filter
	sprintf (Temp_str, ":CHANnel%d:FSELect FILTER%d", Channel_O,Filter); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	//打开Filter
	sprintf (Temp_str, ":CHANnel%d:FILTER ON", Channel_O); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 

	//设置波长
	sprintf (Temp_str, ":CHANnel%d:WAVelength WAV%d", Channel_O, wavelength); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 	

	if (!Visa_OPC(instHandle, 3)) return FALSE; 

	if (!Visa_Write(instHandle, "*WAI")) return FALSE;
	
	//此处命令异常
	if (!Visa_Write(instHandle, ":MTESt:ALLGn")) return FALSE;
	
	if (!Visa_Write(instHandle, ":MTESt:AMEThod NRZ")) return FALSE; 
	if (!Visa_Write(instHandle, ":MTESt:TEST ON")) return FALSE;
	if (!Visa_Write(instHandle, ":DISPlay:PERSistence INFinite")) return FALSE; 
	
	if (!Visa_Write(instHandle, ":TIMEBASE:UNITs TIME")) return FALSE;  
		
	if (!Visa_OPC(instHandle, 3)) return FALSE; 
	
	sprintf (Temp_str, ":TIMEBASE:SCALE %fE-12", X_Scale); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":TIMEBASE:POSITION %fE-3", X_Position); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":CHANnel%d:SCALE %fE-6", Channel_O,Y_Scale); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":CHANNEL%d:OFFSET %fE-6", Channel_O,Y_Offset); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 	

	sprintf (Temp_str, ":MTESt:LOAD \"%s\"", MaskName); 
    if (!Visa_Write(instHandle, Temp_str)) return FALSE;
   
	sprintf (Temp_str, ":MTEST:MMAR:PERC %f", MaskMargin); 
    if (!Visa_Write(instHandle, Temp_str)) return FALSE;
	
	if (!Visa_Write(instHandle, ":MTESt:MMARgin:STAT ON")) return FALSE; 
	
	if (!Visa_Write(instHandle, ":MEASure:APOWer DECibel")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer RMS")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:RISetime")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:FALLtime")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:DEFine THResholds,T2080")) return FALSE;  
	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer PP")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:CGRade:CROSsing")) return FALSE;  	
	if (!Visa_Write(instHandle, ":Measure:CGRade:ERATio DECibel")) return FALSE;
	//2008-11-19，消光比校准功能屏蔽,
	//2011-5-31 启用
	sprintf (Temp_str, ":MEASure:CGRade:ERFactor CHANnel%d,ON,%f", Channel_O, ERFactor); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE;  
	
	return TRUE;   
}

BOOL Ag86100_GET_Er(ViSession instHandle, double *Er)
{
char buf[128];
int	 rcount;
double temp; 

	if (!Visa_Write(instHandle, ":Measure:CGRade:ERATio? DecIBEL")) return FALSE;

    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
    
	Scan (buf, "%s>%f", &temp);
	*Er=temp>1000? 1000:temp;
	
	return TRUE;
}

BOOL Ag86100_GET_O_PPj(ViSession instHandle, double *PPj)
{
char 	buf[128];
int		rcount;
double  temp;

	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer? PP")) return FALSE;
    
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	Scan (buf, "%s>%f", &temp);
	*PPj=(temp*1E+12)>1000? 1000:(temp*1E+12);
	
	return TRUE;
}

BOOL Ag86100_GET_O_RSj(ViSession instHandle, double *RSj)
{
char 	buf[128];
int		rcount;
double  temp; 

	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer? RMS")) return FALSE;
    
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	Scan (buf, "%s>%f", &temp); 
	*RSj=(temp*1E+12)>1000? 1000:(temp*1E+12); 
	
	return TRUE;
}

BOOL Ag86100_Set_Clear(ViSession instHandle)
{

	if (!Visa_Write(instHandle, ":CDISPLAY")) return FALSE;
	
	return TRUE;
}

BOOL Ag86100_Set_AutoScale(ViSession instHandle)
{
	if (!Visa_Write(instHandle, ":AUToscale")) return FALSE;
	
	if (!Visa_OPC(instHandle, 5)) return FALSE;
	
	return TRUE;
}

BOOL Ag86100_SET_MaskMargin(ViSession instHandle, double MaskMargin, char *MaskName)
{
	sprintf (Temp_str, ":MTESt:LOAD \"%s\"", MaskName); 
    if (!Visa_Write(instHandle, Temp_str)) return FALSE;
   
	sprintf (Temp_str, ":MTEST:MMAR:PERC %f", MaskMargin); 
    if (!Visa_Write(instHandle, Temp_str)) return FALSE;
	
	if (!Visa_Write(instHandle, ":MTESt:MMARgin:STATe ON")) return FALSE;  
	
	return TRUE;
}

BOOL Ag86100_GET_MaskHits(ViSession instHandle, int WaveForms)
{
char 	buf[128];
int		rcount, count;
double  temp; 
	
	if (!Ag86100_Set_AutoScale(instHandle)) return FALSE;
	Delay(1); 
	if (!Visa_Write(instHandle, ":MTESt:TEST ON")) return FALSE;
	if (!Visa_OPC(instHandle, 5)) return FALSE; 

	//重复设置5次，确保眼图模板设置正确
	count = 0;
	do
	{
		if (!Visa_Write(instHandle, ":MTESt:TEST ON")) return FALSE;  

		do
		{
			if (!Visa_Write(instHandle, ":MTESt:COUNt:WAVeforms?")) return FALSE; 
		
		    Status = viRead(instHandle, buf, 128, &rcount);
			if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

			Scan (buf, "%s>%f", &temp);
		
			if (temp>WaveForms) break;
			Delay(1);
		} while (temp<WaveForms);
	
		if (!Visa_Write(instHandle, ":MTESt:COUNt:FSAMples?")) return FALSE; 
		
	    Status = viRead(instHandle, buf, 128, &rcount);
		if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

		Scan (buf, "%s>%f", &temp);
		
		count++;
		
	} while (count<5 && temp>0);
	
	if (temp>0) return FALSE;

	return TRUE;
}

BOOL Ag86100_GET_O_Rise(ViSession instHandle, double *Rise)
{
char 	buf[128];
int		rcount;
double  temp; 
										  
	if (!Visa_Write(instHandle, ":MEASure:RISetime?")) return FALSE;
    									  
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	Scan (buf, "%s>%f", &temp); 
	*Rise=(temp*1E+12)>1000? 1000:(temp*1E+12); 
	
	return TRUE;
}

BOOL Ag86100_GET_O_Fall(ViSession instHandle, double *Fall)
{
char 	buf[128];
int		rcount;
double  temp; 

	if (!Visa_Write(instHandle, ":MEASure:FALLtime?")) return FALSE;
    									  
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	Scan (buf, "%s>%f", &temp); 
	*Fall=(temp*1E+12)>1000? 1000:(temp*1E+12);
	
	return TRUE;
}

BOOL Ag86100_SET_O_EYE(ViSession instHandle)
{
	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer PP")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:CGRade:JITTer RMS")) return FALSE;  	
	if (!Visa_Write(instHandle, ":MEASure:RISetime")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:FALLtime")) return FALSE; 

	return TRUE; 	
}

BOOL Ag86100_SET_O_ER(ViSession instHandle, double X_Scale, double X_Position, double Y_Scale, double Y_Offset, int Channel_O)
{
	if (!Visa_Write(instHandle, ":Measure:CGRade:ERATio DECibel")) return FALSE;
	if (!Visa_Write(instHandle, ":MEASure:APOWer DECibel")) return FALSE; 
	if (!Visa_Write(instHandle, ":MEASure:CGRade:CROSsing")) return FALSE; 
	
	//为了防止眼图测试失败后使用AUToscale函数，测试结束后需要将scale设置回初始设置
	sprintf (Temp_str, ":TIMEBASE:SCALE %fE-12", X_Scale); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":TIMEBASE:POSITION %fE-3", X_Position); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":CHANnel%d:SCALE %fE-6", Channel_O,Y_Scale); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 
	
	sprintf (Temp_str, ":CHANNEL%d:OFFSET %fE-6", Channel_O,Y_Offset); 
	if (!Visa_Write(instHandle, Temp_str)) return FALSE; 	

	return TRUE; 	
}

BOOL Ag86100_GET_MainSN(ViSession instHandle, char *MainSN)
{
	char 	buf[128]="", tempstr[50]="";  
	int		rcount;  
	
	if (!Visa_Write(instHandle, "*IDN?")) return FALSE;
    									  
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	//将获取的设备信息字符转换为大写
	StringUpperCase (buf);
	if (strstr (buf, "US") != NULL)
	{
		strcpy (tempstr, strstr (buf, "US"));
	}
	else if (strstr (buf, "MY") != NULL) 
	{
		strcpy (tempstr, strstr (buf, "MY"));
	}
	else
	{
		MessagePopup("提示", "当前设备的主机序列号不是以US或MY开头，软件不能正常识别");
		return FALSE;
	}
	
	strncpy(buf, tempstr, 10);
	buf[10]='\0';
	
	strcpy (MainSN, buf); 
	
	return TRUE;
}

BOOL Ag86100_GET_ModuleSN(ViSession instHandle, char *ModuleSN)
{
	char 	buf[128]="", tempstr[50]="";  
	int		rcount;  
	
	if (!Visa_Write(instHandle, ":SERial? LMODule")) return FALSE;
    									  
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	strcpy (tempstr, strstr (buf, "US"));
	
	strncpy(buf, tempstr, 10);
	buf[10]='\0';
	
	strcpy (ModuleSN, buf);
	
	return TRUE;
}

BOOL Ag86100_GET_O_CROSsing(ViSession instHandle, double *CROSsing)
{
char 	buf[128];
int		rcount;
double  temp; 
								
	if (!Visa_Write(instHandle, ":MEASURE:CGRade:CROSsing?")) return FALSE;
    									  
    Status = viRead(instHandle, buf, 128, &rcount);
    if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
	
	Scan (buf, "%s>%f", &temp); 
	*CROSsing=temp>1000? 1000:temp;   

	return TRUE;
}

BOOL Ag86100_Set_Run(ViSession instHandle)
{

	if (!Visa_Write(instHandle, ":RUN")) return FALSE;
	
	return TRUE;
}

BOOL Ag86100_Set_Stop(ViSession instHandle)
{

	if (!Visa_Write(instHandle, ":STOP")) return FALSE;
	
	return TRUE;
}

BOOL Ag86100_GET_MaskFSample(ViSession instHandle, int *totalcount)   
{
	char 	buf[128];
	int		rcount;  

	if (!Visa_Write(instHandle, ":MTESt:COUNt:FSAMples?")) return FALSE; 
		
    Status = viRead(instHandle, buf, 128, &rcount);
	if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;
		 
	Scan (buf, "%s>%i", totalcount);
	
	return TRUE;
}

BOOL Ag86100_GET_WaveForms(ViSession instHandle, int *wavecount)   
{
	char 	buf[128];
	int		rcount;  

	if (!Visa_Write(instHandle, ":MTESt:COUNt:WAVeforms?")) return FALSE;   
	
    Status = viRead(instHandle, buf, 128, &rcount);
	if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

	Scan (buf, "%s>%i", wavecount);
	
	return TRUE;
}
