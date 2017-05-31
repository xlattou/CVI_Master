#include "global.h" 
#include <formatio.h>
#include <utility.h>
#include "aq637x.h"


int AQ637X_Init(ViSession *instHandle, ViRsrc Viname)
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

int AQ637X_Config(ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, ViReal64 resolution)
{
	ViStatus  	status;
	char  buf[256];
	
	 //先停止扫描
	status = Visa_Write(instHandle, ":ABORt");
	if (!status) return -1;
	
	if (LaserType==0) // for FP
	{
		//设置中心波长
		sprintf (buf, ":SENSe:WAVelength:CENTer %.3fnm", ctrwl);
		status = Visa_Write(instHandle, buf);
		if (!status) return -1;
		
		//设置SPAN
		//status = Visa_Write(instHandle, ":SENSE:WEVELENGTH:SPAN 20.0NM");
		sprintf (buf, ":SENSE:WAVELENGTH:SPAN %.1fnm", span); 
		status = Visa_Write(instHandle, buf);
		if (!status) return -1;
		
		//设置RMS模式			 //是不是把上面两条都替代了？
		status = Visa_Write(instHandle, ":CALCULATE:PARAMETER:SWRMS:TH 10.00DB");
		if (!status) return -1;
	}
	else	//for DFB 
	{
		//设置中心波长
		sprintf (buf, ":sens:wav:cent %.3fnm", ctrwl); 
		status = Visa_Write(instHandle, buf);
		if (!status) return -1;
		
		//设置SPAN
		sprintf (buf, ":sens:wav:span %.1fnm", span);      
		status = Visa_Write(instHandle, buf);
		//status = Visa_Write(instHandle, "SPAN30.00");
		if (!status) return -1;
		
		//设置bandwidth模式
		status = Visa_Write(instHandle, ":CALCULATE:PARAMETER:SWTHRESH:TH 10.00DB");
		if (!status) return -1;
		
		//设置带宽测试模式   
		status = Visa_Write(instHandle, ":CALCULATE:PARAMETER:SWTHRESH:MFIT OFF");
		if (!status) return -1;
	}
	
	//设置Y坐标SCALE模式
	status = Visa_Write(instHandle, ":DISPLAY:TRACE:Y1:PDIV 10.0DB");
	if (!status) return -1; 
	
	//设置分辨率
	sprintf (buf, ":SENSE:BANDWIDTH:RESOLUTION %fPM", resolution);   
	status = Visa_Write(instHandle, buf);
	if (!status) return -1; 
	
	//设置灵敏度自动模式
	status = Visa_Write(instHandle, ":SENSE:SENSE MID");
	if (!status) return -1; 
	
	//设置重复扫描模式
	status = Visa_Write(instHandle, ":INITIATE:SMODE SINGLE");
//	status = Visa_Write(instHandle, ":INITIATE:SMODE AUTO");  
	if (!status) return -1; 
	
	status = Visa_Write(instHandle, ":INITIATE");
	if (!status) return -1; 
	
	//设置自动更新测试结果模式
	status = Visa_Write(instHandle, ":CALCULATE:AUTO ON");
	if (!status) return -1; 

	status = Visa_OPC(instHandle, 5);
	if (!status) return -1; 
	
	//为了保证仪器能够相应输入的设置，需要加延时
	Delay (5);
	
	return 0;
}

int AQ637X_Read(ViSession instHandle, int LaserType, ViReal64 span, double *PeakWavelength, double *Sigma, double *BandWidth, double *SMSR)
{
	float 		peakWavelength, sigma, bandwidth, smsr;
	float 		temp1, temp2, temp3, temp4, temp5; 
	ViStatus  	status; 
	char        buf[500];
	int			rcount, count,i;
	
	if (LaserType==0) // for FP 
	{
		//设置中心波长到峰值
		status = Visa_Write(instHandle, ":CALCulate:MARKer:MAXimum:SCENter");
		if (!status) return -1;
		
		//设置SPAN
		sprintf (buf, ":sens:wav:span %.1fnm", span); 
		status = Visa_Write(instHandle, buf);
		if (!status) return -1;
		
		//完成一次完整的扫描时间10s
		Delay (10); 
			
		//for smsr
		status = Visa_Write(instHandle, ":CALCULATE:PARAMETER:SWRMS:TH 10.00DB");
		if (!status) return -1;
		
		Delay (1);
		
	    status = Visa_Write(instHandle, ":CALCULATE:DATA?");
		if (!status ) return FALSE;
    
	    status = viRead(instHandle, buf, 200, &rcount); 
	    if (!status < VI_SUCCESS) return -1;
	
		if (strcmp (strstr (buf, "SWRM"), "") == 0)
		{
			MessagePopup ("Error", "AQ637X read SMSR results error");
			return -1;
		}
		//由于返回的字符串可能有错误，先进行刷选
		strcpy (buf, strstr (buf, "SWRM"));
		sscanf (buf, "SWRM %f,%f", &peakWavelength, &sigma);
	}
	else //for DFB 
	{
		//???
		status = Visa_Write(instHandle, ":INITIATE");
		if (!status) return -1; 
	
		//设置自动更新测试结果模式  ？
		status = Visa_Write(instHandle, ":CALCULATE:AUTO ON");
		if (!status) return -1; 

		//完成一次完整的扫描时间5s
		Delay (1); 
		
		//for bandwith
		status = Visa_Write(instHandle, ":CALCULATE:PARAMETER:SWTHRESH:TH 20.00DB");
		if (!status) return -1;
		
		Delay (1);
		
	    status = Visa_Write(instHandle, ":CALCULATE:DATA?");
		if (!status ) return -1;
    
	    status = viRead(instHandle, buf, 200, &rcount); 
	    if (status < VI_SUCCESS) 
	    {	
	    	MessagePopup ("ERROR", "读波长超时!     "); 
	    	return -1;
	    }
	
		//由于返回的字符串可能有错误，先进行刷选
		sscanf (buf, "%f,%f", &peakWavelength, &bandwidth); 
							 
		sprintf (buf, "SPAN%.2f", span); 
		status = Visa_Write(instHandle, buf);
		if (!status) return -1;
	}
	
	*PeakWavelength=peakWavelength * 1000000000;
	
	if (LaserType==0)
	{
		*BandWidth=0;
		*SMSR=0;
		*Sigma=0; 
	}
	else
	{
		*BandWidth=bandwidth;
		*SMSR=0;
		*Sigma=0;
	}
	
	return 0;
}
