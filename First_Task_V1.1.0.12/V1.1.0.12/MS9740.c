#include "global.h" 
#include <formatio.h>
#include <utility.h>
#include "MS97xx.h" 
#include "MS9740.h"

static const ViChar terminator[]="";
ViSession 	io=0; 
ViAttr attributeId=0;
ViConstString channelName="";
int error;

int MS9740_Init(ViSession *instHandle, ViRsrc Viname)
{
	
	ViStatus  	status;
	int			rcount;
	char 		buffer[128] = ""; 
	
	error = MS97xx_init (Viname, VI_TRUE, VI_TRUE, instHandle);		//初始化   
	if (error)
	{
		MessagePopup ("提示", "初始化失败");   
		return -1;
	}
	
	io=Ivi_IOSession(*instHandle); 
	
	error = MS97xxAttrAttenuatorOn_WriteCallback (*instHandle, io, channelName, attributeId, VI_FALSE);   //设置ATT OFF
	if (error)
	{
		MessagePopup ("提示", "设置ATT OFF失败");   
		return -1;
	}
	
	error = MS97xxAttrSamplingPoints_WriteCallback (*instHandle, io, channelName, attributeId, 5001); 	  //设置点数
	if (error)
	{
		MessagePopup ("提示", "设置MPT失败");   
		return -1;
	}
	
	error = MS97xxAttrVideoBandWidth_WriteCallback (*instHandle, io, channelName, attributeId, 100000);   
	if (error)
	{
		MessagePopup ("提示", "设置VBW失败");   
		return -1;
	}
	
	error = MS97xxAttrRefLevel_WriteCallback (*instHandle, io, channelName, attributeId, 5.); 			  //设置ref
	if (error)
	{
		MessagePopup ("提示", "设置REF失败");   
		return -1;
	}
	
	error = MS97xxAttrLogScale_WriteCallback (*instHandle, io, channelName, attributeId, 6);  			  //设置log
	if (error)
	{
		MessagePopup ("提示", "设置LOG失败");   
		return -1;
	}
	
	error = MS97xxSetWDP_WriteCallback (*instHandle, io, channelName, attributeId, 0); 					  //设置
	if (error)
	{
		MessagePopup ("提示", "设置WDP失败");   
		return -1;
	}

	error = MS97xx_ContinuousSweep (*instHandle);
	if (error)
	{
		MessagePopup ("提示", "设置连续扫描失败");   
		return -1;
	}
	
	return 0;
}

int MS9740_Config(ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, ViReal64 resolution)
{
	ViStatus  	status;
	char  buf[256];
	int error;
	
	error = MS97xx_SetCenterAndSpanWavelength (instHandle, ctrwl, span);	
	if (error)
	{
		MessagePopup ("提示", "设置中心波长失败   ");   
		return -1;
	}
	error = MS97xxAttrResolution_WriteCallback (instHandle, io, channelName, attributeId, resolution);    
	if (error)
	{
		MessagePopup ("提示", "设置resolution失败   ");   
		return -1;
	}

	if (LaserType==0) // for FP
	{
		error = MS97xxAttrFpAxisModeCutLevel_WriteCallback (instHandle,  io,  channelName, attributeId, 3);
		if (error)
		{
			MessagePopup ("提示", "设置FP模式失败    ");   
			return -1;
		}
	}
	else	//for DFB 
	{
		error = MS97xxSetDfb_WriteCallback (instHandle, io, channelName, attributeId, 20); //MS97xxAttrDfb_WriteCallback (instHandle, io, channelName, attributeId, 1);//MS97xxSetDfb_WriteCallback (instHandle, io, channelName, attributeId, 20);
		if (error)
		{
			MessagePopup ("提示", "设置DFB模式失败    ");   
			return -1;
		}
	/*	error = MS97xx_SetDFB_Parameters (instHandle, MS97XX_VAL_DFB_SIDE_MODE_2NDPEAK, 20);   
		if (error)
		{
			MessagePopup ("提示", "设置DFB参数失败    ");   
			return -1;
		}		 */
	}
	
	//为了保证仪器能够相应输入的设置，需要加延时
	Delay (5);
	
	return 0;
}

int MS9740_Read (ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, double *PeakWavelength, double *Sigma, double *BandWidth, double *SMSR)
{
	ViReal64 		peakWavelength, sigma, bandwidth, smsr;
	ViReal64 		temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8,temp9; 
	ViStatus  	status; 
	char        buf[500];
	int			rcount, count;
	ViInt32     numModes;
	
/*	
	MS97xx_PeakToCenter(instHandle); 
	if (error)
	{
		MessagePopup ("提示", "设置peak->center失败");   
		return -1;
	}		
*/
	
	if (LaserType==0) // for FP 
	{
		Delay (1);
		error = MS97xx_GetFP_Results (instHandle,&temp1, &temp2, &peakWavelength, &temp3, &numModes, &temp5, &temp6, &sigma);
		if (error)
		{
			MessagePopup ("提示", "读取数据失败    ");   
			return -1;
		}
	}
	else //for DFB 
	{
		//完成一次完整的扫描时间5s
	//	Delay (1); 
		
		error = MS97xx_StopSweep (instHandle);  
		if (error)
		{
			MessagePopup ("提示", "停止扫描失败    ");   
			return -1;
		}	 
		
	    error = MS97xx_GetDFB_Results (instHandle, &smsr, &temp7, &peakWavelength, &temp1, &temp2, &temp3, &temp4, &temp5, &temp6);   
		if (error)
		{
			MessagePopup ("提示", "读取数据失败    ");   
			return -1;
		}
	
		error = MS97xxSetDfbOFF_WriteCallback (instHandle, io, channelName, attributeId, 20); 
		if (error)
		{
			MessagePopup ("提示", "设置AP OFF失败   ");   
			return -1;
		}
		Delay (0.5); 
		error = MS97xxAttrThresholdCut_WriteCallback (instHandle, io, channelName, attributeId, 20); 
		if (error)
		{
			MessagePopup ("提示", "设置ANA THR模式失败   ");   
			return -1;
		}
		Delay (1); 
		count = 0;
		do
		{
			error = MS97xx_GetSMSR_Analysis (instHandle,&temp1, &bandwidth);
			if (error)
			{
				MessagePopup ("提示", "读取分析数据失败   ");   
				return -1;
			}
			if (0 <bandwidth)	
			{
				break;
			}

			count++;
		}while(count<3);
		
		if(count>=3) 
		{
			MessagePopup ("提示", "读取bandwidth数据失败   ");   
			
		}
		
		error = MS97xxSetDfb_WriteCallback (instHandle, io, channelName, attributeId, 20); 
		if (error)
		{
			MessagePopup ("提示", "设置DFB模式失败   ");   
			return -1;
		}
	}
	
	*PeakWavelength=peakWavelength;
	
	if (LaserType==0)
	{
		*BandWidth=0;
		*SMSR=0;
		*Sigma=sigma; 
	}
	else
	{
		*BandWidth=bandwidth;
		*SMSR=smsr;
		*Sigma=0;
	}
	
/*	error = MS97xx_SetCenterAndSpanWavelength (instHandle, ctrwl, span); 
	if (error)
	{
		MessagePopup ("提示", "设置中心波长失败   ");   
		return -1;
	}		  */
	error = MS97xx_ContinuousSweep (instHandle);
	if (error)
	{
		MessagePopup ("提示", "设置连续扫描失败");   
		return -1;
	}	  
	
	return 0;
}

int MS9740_Stop(ViSession instHandle)
{
	int error;
	error = MS97xx_StopSweep (instHandle);  
	if (error)
	{
		MessagePopup ("提示", "停止扫描失败    ");   
		return -1;
	}
	return 0;
}

int MS9740_Close(ViSession instHandle)
{
	MS97xx_close (instHandle);
	return 0;
}
