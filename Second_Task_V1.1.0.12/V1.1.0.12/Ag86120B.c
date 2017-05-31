#include <formatio.h>
#include <utility.h>
#include "Ag86120B.h"

int vi_write(ViSession vi, ViBuf buf)
{
int		status, rcount;
char    buffer[128];

	strcpy (buffer, buf);
	strcat (buffer, "\n");
	status = viWrite (vi, buf,StringLength(buf), &rcount); 
	if(status!=VI_SUCCESS) return -1; 
	
	return 0;
}

int Ag86120B_Init(ViSession *instHandle, ViRsrc Viname)
{
	ViSession 	defaultRM;
	ViStatus  	status;
	int			rcount;
	char 		buffer[128] = ""; 
	
    viOpenDefaultRM (&defaultRM);
    
    status = viOpen (defaultRM, Viname, VI_NULL, VI_NULL, instHandle);
    if (status < VI_SUCCESS)                                                 
    {                                                                       
    	MessagePopup ("提示", "打开仪器会话出错，可能是没有GPIB卡，程序将终止");
        return -1;
    }

	viClear (*instHandle);
	
	status = viSetAttribute(*instHandle, VI_ATTR_TMO_VALUE, 10000);  
    status = viWrite (*instHandle, "*IDN?\n",StringLength("*IDN?\n"), &rcount);
    Delay(0.1); 
    status = viRead(*instHandle, buffer, 128, &rcount); 
    if (status < VI_SUCCESS)
    {  
    	MessagePopup ("提示", "error");
    	return -1;
    } 
	
	return 0;
}

int Ag86120B_Config(ViSession instHandle)
{
	ViStatus  	status;
	char  buf[256];
	
	//Preset
	status = vi_write(instHandle, ":SYSTem:PRESet");
	if (status) return -1;
	Delay(2.);
	
	//
	status = vi_write(instHandle, ":INIT:CONT ON");
	if (status) return -1; 
	Delay(0.5);
	//单位
	status = vi_write(instHandle, ":UNIT:POWER DBM");
	if (status) return -1; 
	//
	status = vi_write(instHandle, ":CONF:SCAL:POW MAX");
	if (status) return -1; 
	Delay(0.5); 
	
	return 0;
}

int Ag86120B_Read (ViSession instHandle, double *PeakWavelength, double *Power)
{
	float 		pow, wave;
	ViStatus  	status; 
	char        buf[500];
	int			rcount, count;  
	
	count= 20;
/*  
status = vi_write(instHandle, ":FETC:SCAL:POW?");
	if (status ) return -1;
//	Delay (0.1); 
    status = viRead(instHandle, buf, count, &rcount); 
    if (status < VI_SUCCESS) return -1;
							   
	sscanf (buf, "%f", &pow);   
	*Power = pow;  
*/						 
	//for wave
	status = vi_write(instHandle, ":FETC:SCAL:POW:WAV?");
	if (status) return -1;
//	Delay (0.1); 
    status = viRead(instHandle, buf, count, &rcount); 
    if (status < VI_SUCCESS) return -1;

	sscanf (buf, "%f", &wave);    	 
	
	*PeakWavelength = wave * 1000000000 ;
	 
	
	return 0;
}
