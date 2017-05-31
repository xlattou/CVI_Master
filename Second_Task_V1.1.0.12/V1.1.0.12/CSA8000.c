#include "CSA8000.h"
#include <formatio.h>
#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include <toolbox.h> 

static unsigned char	Temp_str[256]; 
static BOOL   			Status; 

BOOL CSA8000_Init(ViSession *instHandle, ViRsrc Viname)
{
ViSession defaultRM;

    viOpenDefaultRM (&defaultRM);
    							 
    Status = viOpen (defaultRM, Viname, VI_NULL, VI_NULL, instHandle);   //Viname "GPIB0::8::INSTR"
    if(Status !=VI_SUCCESS) return FALSE;
 
	Status = viSetAttribute(*instHandle, VI_ATTR_TMO_VALUE, 5000);  
    if(Status !=VI_SUCCESS) return FALSE; 
    
    Status = Visa_Write(*instHandle, "unLOCK ALL");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "TRIGger:SOURce EXTDirect");
	if (!Status ) return FALSE; 
	Status = Visa_Write(*instHandle, "DISplay:STYLE INFPersist");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "WFMDB:WFMDB1:ENABle ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "WFMDB:WFMDB1:DISPlay ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:STANdard ENET1250");		
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:COUNt:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:MARgin:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:MARgin:PERCent 15");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "MASK:DISplay ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(*instHandle, "HEADER OFF");
	if (!Status ) return FALSE;
	
	Status = Visa_Write (*instHandle, "HARDCOPY:FORMAT PNG");   
	if (!Status ) return FALSE; 
	
	Status = Visa_OPC(*instHandle, 3);
	if (!Status ) return FALSE; 
	return TRUE;   
}

BOOL CSA8000_Set_O(ViSession instHandle, int Channel_O, double X_Scale, double X_Position, double Y_Scale, double Y_Position, double Offset, int Wavelength, char *Filter, char *MaskName)  
{						
	sprintf (Temp_str, "SELect:CH%d ON", Channel_O);
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	
	sprintf (Temp_str, "CH%d:WLENgth:VALue %d", Channel_O, Wavelength);
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;

	//时钟恢复
	sprintf (Temp_str, "TRIGger:CH%d:CLKRec:VALue %s", Channel_O, Filter);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
						
	sprintf (Temp_str, "CH%d:FILTer:VALue %s", Channel_O, Filter);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	
	sprintf (Temp_str, "MASK:SOUrce CH%d", Channel_O);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	//MASK:STANdard 的设置必须放在X_Scale等的设置前，否则改变MASK:STANdard 设置会对其有影响
	sprintf (Temp_str, "MASK:STANdard %s", MaskName);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	
	Status = Visa_Write(instHandle, "ACQUIRE:STATE STOP");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "SELect:CH7 OFF");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "SELect:CH6 OFF");
	if (!Status ) return FALSE;
	sprintf (Temp_str, "WFMDB:WFMDB1:SOURce CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "WFMDB:WFMDB1:DISPlay ON");
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS1:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS2:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS3:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS4:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS5:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS6:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
	sprintf (Temp_str, "MEASUrement:MEAS7:SOURCE1:WFM CH%d", Channel_O); 
	Status = Visa_Write(instHandle, Temp_str);	
	if (!Status ) return FALSE;
				
	Status = Visa_Write(instHandle, "ACQUIRE:STATE RUN");
	if (!Status ) return FALSE;
	sprintf (Temp_str, "HORizontal:main:scale %fE-12", X_Scale);
	Status = Visa_Write(instHandle, Temp_str);
	sprintf (Temp_str, "HORizontal:MAIN:POSition %fE-9", X_Position);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	sprintf (Temp_str, "CH%d:SCALE %fE-6", Channel_O, Y_Scale);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	sprintf (Temp_str, "CH%d:OFFSet %fE-6", Channel_O, Offset);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	sprintf (Temp_str, "CH%d:POSition %f", Channel_O, Y_Position);  //±5 divisions for channels
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	
	//for meas type
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:TYPE AOPTPWRDBM");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:TYPE EXTINCTDB");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:TYPE PKPKJitter");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:TYPE RMSJitter");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:TYPE RISE");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:TYPE FALL");				
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:GATing:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:METHod RELative");
	if (!Status ) return FALSE;	
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:Relative:HIGH 80");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:Relative:LOW 20");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:GATing:STATE on");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:METHod RELative");
	if (!Status ) return FALSE;	
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:Relative:HIGH 80");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:Relative:LOW 20");
	if (!Status ) return FALSE;  
	Status = Visa_Write(instHandle, "MEASUrement:MEAS7:TYPE PCTCROss");				
	if (!Status ) return FALSE;
	
	
	//for wfmdb signal type
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:SOURCE1:WFMDB:SIGType eye");		
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS7:SOURCE1:WFMDB:SIGType eye");		
	if (!Status ) return FALSE;
	
	//for wfmdb state
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS7:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;	
	
	Status = Visa_Write(instHandle, "MEASUrement:ANNOtations:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "DISplay:WFMReadout OFF");
	if (!Status ) return FALSE;
	
	Status = Visa_OPC(instHandle, 5);
	if (!Status ) return FALSE;
	return TRUE;
}

BOOL CSA8000_Set_E(ViSession instHandle) 
{
	Status = Visa_Write(instHandle, "SELect:CH1 OFF");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "SELect:CH7 ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:SOURCE1:WFM CH7");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:SOURCE1:WFM CH7");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:SOURCE1:WFM CH7");				
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:SOURCE1:WFM CH7");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:SOURCE1:WFM CH7");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:SOURCE1:WFM CH7");   
	if (!Status ) return FALSE; 
	Status = Visa_Write(instHandle, "WFMDB:WFMDB2:SOURce CH7");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:TYPE AMPLitude");
	if (!Status ) return FALSE;
//	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:TYPE EXTINCTDB");
//	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:TYPE PKPKJitter");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:TYPE RMSJitter");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:TYPE RISE");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:TYPE FALL");				
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:GATing:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:METHod RELative");
	if (!Status ) return FALSE;	
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:Relative:HIGH 80");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:REFLevel1:Relative:LOW 20");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:GATing:STATE on");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:METHod RELative");
	if (!Status ) return FALSE;	
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:Relative:HIGH 80");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:REFLevel1:Relative:LOW 20");
	if (!Status ) return FALSE; 
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
//	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:SOURCE1:WFMDB:SIGType eye");
//	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:SOURCE1:WFMDB:SIGType eye");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:SOURCE1:WFMDB:SIGType eye");		
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
//	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:SOURCE1:WFMDB:STATE ON");
//	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:SOURCE1:WFMDB:STATE ON");
	if (!Status ) return FALSE;
	Status = Visa_Write(instHandle, "HORizontal:main:scale 0.228E-9");
	if (!Status ) return FALSE;
	Status = Visa_OPC(instHandle, 5);
	if (!Status ) return FALSE;
	return TRUE;
}

BOOL CSA8000_GET_Aop(ViSession instHandle, double *Aop)
{
char 	buf[128]="";
int		rcount;
double	temp;

	Status = Visa_Write(instHandle, "MEASUrement:MEAS1:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*Aop=temp>1000 ? 1000:temp;
	
	return TRUE;
}

BOOL CSA8000_GET_Er(ViSession instHandle, double *Er)
{
char 	buf[128]="";
int		rcount;
double	temp;

	Status = Visa_Write(instHandle, "MEASUrement:MEAS2:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*Er=temp>1000 ? 1000:temp;
	
	return TRUE;
}

BOOL CSA8000_GET_O_PPj(ViSession instHandle, double *PPj)
{
char 	buf[128]="";
int		rcount;
double	temp; 

	Status = Visa_Write(instHandle, "MEASUrement:MEAS3:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*PPj=temp>1000 ? 1000:temp;	
	
	return TRUE;
}

BOOL CSA8000_GET_O_RSj(ViSession instHandle, double *RSj)
{
char 	buf[128]="";
int		rcount;
double  temp;

	Status = Visa_Write(instHandle, "MEASUrement:MEAS4:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*RSj=temp>1000 ? 1000:temp;
	
	return TRUE;
}

BOOL CSA8000_GET_O_PCTCROss(ViSession instHandle, double *PCTCROss)
{
char 	buf[128]="";
int		rcount;
double	temp; 

	Status = Visa_Write(instHandle, "MEASUrement:MEAS7:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*PCTCROss=temp>1000 ? 1000:temp;	
	
	return TRUE;
}

BOOL CSA8000_Set_Clear(ViSession instHandle)
{
	Status = Visa_Write(instHandle, "ACQUire:data:clear");
	if (!Status ) return FALSE;
	
	return TRUE;
}

BOOL CSA8000_SET_MaskMargin(ViSession instHandle, double MaskMargin, char *MaskName)
{
	sprintf (Temp_str, "MASK:MARgin:PERCent %f", MaskMargin);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	
	sprintf (Temp_str, "MASK:STANdard %s", MaskName);
	Status = Visa_Write(instHandle, Temp_str);
	if (!Status ) return FALSE;
	
	return TRUE;
}

BOOL CSA8000_GET_MaskCount(ViSession instHandle, int WaveForms)
{
double  temp; 
char 	buf[128];
int		rcount;  
	//set AUTOSet EXECute
	if (!CSA8000_SET_AUTOSet_EXECute(instHandle)) return FALSE;
	
	Status = Visa_OPC(instHandle, 30);
	if (!Status ) return FALSE;
	
	Delay(5);
	
	Status = Visa_Write(instHandle, "MASK:COUNt:STATE ON");
	if (!Status ) return FALSE;
	
	do
	{
		if (!Visa_Write(instHandle, "MASK:COUNt:WAVeforms?")) return FALSE; 
		
	    Status = viRead(instHandle, buf, 128, &rcount);
		if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

		Scan (buf, "%s>%f", &temp);
		
		if (temp>WaveForms) break;
		Delay(1);
	} while (temp<WaveForms);
	
	if (!Visa_Write(instHandle, "MASK:COUNt:TOTal?")) return FALSE; 
	
    Status = viRead(instHandle, buf, 128, &rcount);
	if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

	Scan (buf, "%s>%f", &temp);
	
	if (temp>0) return FALSE;
	
	return TRUE;
}

BOOL CSA8000_GET_MaskCount_Ex(ViSession instHandle, int *totalcount)
{
	char 	buf[128];
	int		rcount;  
	double  temp; 
	
	if (!Visa_Write(instHandle, "MASK:COUNt:TOTal?")) return FALSE; 
	
    Status = viRead(instHandle, buf, 128, &rcount);
	if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

	Scan (buf, "%s>%i", totalcount);
	
	return TRUE;
}

BOOL CSA8000_SET_AUTOSet_EXECute(ViSession instHandle)
{
	Status = Visa_Write(instHandle, "AUTOSet EXECute");
	if (!Status ) return FALSE;
	
	return TRUE;
}

BOOL CSA8000_SET_AUTOSet_UNDo(ViSession instHandle)
{
	Status = Visa_Write(instHandle, "AUTOSet:UNDO");
	if (!Status ) return FALSE;
	return TRUE;
}

BOOL CSA8000_GET_O_Rise(ViSession instHandle, double *Rise)
{
char 	buf[128]="";
int		rcount;
double	temp; 
	Status = Visa_Write(instHandle, "MEASUrement:MEAS5:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*Rise=temp>1000 ? 1000:temp;
	
	return TRUE;
}

BOOL CSA8000_GET_O_Fall(ViSession instHandle, double *Fall)
{
char 	buf[128]="";
int		rcount;
double	temp; 
	Status = Visa_Write(instHandle, "MEASUrement:MEAS6:VALue?");
	if (!Status ) return FALSE;
	Delay(0.1); 
    Status = viRead(instHandle, buf, 10, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	Scan (buf, "%s>%f", &temp);
	*Fall=temp>1000 ? 1000:temp;	
	
	return TRUE;
}

BOOL CSA8000_GET_MainSN(ViSession instHandle, char *MainSN)
{
	char 	buf[128]="", tempstr[50]="";  
	int		rcount;  
	
	Status = Visa_Write(instHandle, "SYSTem:PROPerties:MAInframe:SERialnum?");
	if (!Status ) return FALSE;
	
	Status = viRead(instHandle, buf, 50, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	
	strcpy (tempstr, strstr (buf, "B"));
	
	strncpy(buf, tempstr, 7);
	buf[7]='\0';
	
	strcpy (MainSN, buf); 
	
	return TRUE;
}

BOOL CSA8000_GET_ModuleSN(ViSession instHandle, char *ModuleSN)
{
	char 	buf[128]="", tempstr[50]="";  
	int		rcount;  
	
	Status = Visa_Write(instHandle, "SYSTem:PROPerties:CH1:SERialnum?");
	if (!Status ) return FALSE;
	
	Status = viRead(instHandle, buf, 50, &rcount); 
    if (Status < VI_SUCCESS) return FALSE;
	
	strcpy (tempstr, strstr (buf, "B"));
	
	strncpy(buf, tempstr, 7);
	buf[7]='\0';
	
	strcpy (ModuleSN, buf); 
	
	return TRUE;
}

BOOL CSA8000_Transfer_GPIB (ViSession instHandle, char *sourceFileName, char *destinationFileName)
{
	char str[200]="";      
	//将HARDCOPY:FORMAT设置移到CSA8000_Init函数
//	Status = viWrite (instHandle, "HARDCOPY:FORMAT PNG\n",StringLength("HARDCOPY:FORMAT PNG\n"), &retCnt);
//    if(Status<0)	return FALSE;
	
	sprintf(str,"HARDCOPY:FILEName \"%s\"",sourceFileName); 
	Status = Visa_Write (instHandle, str);  
	if (!Status ) return FALSE; 
	
	tktds8k_TransferFile (instHandle, sourceFileName, destinationFileName);
	
	return TRUE; 
}	

BOOL CSA8000_SET_State_ON(ViSession instHandle)
{
	Status = Visa_Write(instHandle, "ACQuire:STATE RUN"); 
	if (!Status ) return FALSE;
	return TRUE;
}

BOOL CSA8000_SET_State_OFF(ViSession instHandle)
{
	Status = Visa_Write(instHandle, "ACQuire:STATE STOP"); 
	if (!Status ) return FALSE;
	return TRUE;
}

BOOL CSA8000_GET_WaveForms(ViSession instHandle, int *wavecount)
{
	char 	buf[128];
	int		rcount;  
	double  temp; 
	
	if (!Visa_Write(instHandle, "MASK:COUNt:WAVeforms?"))    return FALSE; 
	
    Status = viRead(instHandle, buf, 128, &rcount);
	if ((Status != VI_SUCCESS) && (Status != VI_SUCCESS_TERM_CHAR) && (Status != VI_SUCCESS_MAX_CNT)) return FALSE;

	Scan (buf, "%s>%i", wavecount);
	
	return TRUE;
}

BOOL CSA8000_Transfer_LAN(ViSession instHandle, char *destinationFileName)		 //use net
{
	char str[500]="";
	
//	"HARDCOPY:FILEName \"\\\\192.168.15.172\\10G eye\\"
	sprintf(str,"HARDCOPY:FILEName \"%s\"",destinationFileName); 
	Status = Visa_Write (instHandle, str);  
	if (!Status ) return FALSE; 
	
	return TRUE; 
}
