#include "ag8614x.h"
#include "global.h"
#include <visa.h>   
#include <toolbox.h> 

ViStatus  status;  

BOOL Ag86142_Init(ViSession *instHandle, ViRsrc Viname)
{
	status = ag8614x_init (Viname, VI_TRUE, VI_TRUE, instHandle);
	if (status != VI_SUCCESS) {return FALSE;}
	
	return TRUE;
}

BOOL Ag86142_Config(int LaserType, ViSession instHandle, ViReal64 startWavelength, ViReal64 stopWavelength, ViReal64 resolutionBandwidth)
{
	if (LaserType==LASER_TYPE_FP) // for FP
	{
		status = ag8614x_SRC_configureTestFP (instHandle, startWavelength, stopWavelength, resolutionBandwidth, 2000, -70.0);
		if (status != VI_SUCCESS) {return FALSE;} 
	}
	else	//for DFB 
	{
		status = ag8614x_SRC_configureTestDFB (instHandle, startWavelength, stopWavelength, resolutionBandwidth, 2000, -70.0, -20); 
		if (status != VI_SUCCESS) {return FALSE;}
	}
	
	status = ag8614x_referenceLevel (instHandle, 0.0, ag8614x_YAXIS_LEFT);
	if (status != VI_SUCCESS) {return FALSE;}
	
	status = ag8614x_SRC_sweepExecute (instHandle, ag8614x_REPEAT);
	if (status != VI_SUCCESS) {return FALSE;}
	
	return TRUE;
}

BOOL Ag86142_Read(int LaserType, ViSession instHandle, double *PeakWavelength, double *Sigma, double *BandWidth, double *dSMSR)
{
ViReal64 meanWavelength;
ViReal64 peakWavelength;
ViReal64 peakAmplitude;
ViReal64 modeSpacing;
ViReal64 FWHM; 
ViReal64 sigma;
ViReal64 totalPower;

ViReal64 modeOffset;
ViReal64 stopBand;
ViReal64 SMSR;
ViReal64 centerOffset;
ViReal64 bandwidth;
ViReal64 bandwidthAmp;

	if (LaserType==LASER_TYPE_FP) // for FP
	{
		status = ag8614x_startSweep (instHandle, ag8614x_SWEEP_SINGLE);
		if (status != VI_SUCCESS) {return FALSE;} 
		
		status = ag8614x_markerPosition (instHandle, ag8614x_MARKER_1, ag8614x_CALC_MARK_MAX);
		if (status != VI_SUCCESS) {return FALSE;}
		
		status = ag8614x_markerToCenterWavelength (instHandle, ag8614x_MARKER_1);
		if (status != VI_SUCCESS) {return FALSE;}
		
		status = ag8614x_wavelengthSpan (instHandle, 25.0);
		if (status != VI_SUCCESS) {return FALSE;}
		
		if (!Visa_Write(instHandle, "calc:mark:aoff")) return FALSE;
		
		status = ag8614x_SRC_sweepExecute (instHandle, ag8614x_REPEAT);
		if (status != VI_SUCCESS) {return FALSE;}
		
		status = ag8614x_SRC_resultRecordFP_Q (instHandle, &meanWavelength, &peakWavelength, &peakAmplitude, &modeSpacing, &FWHM, &sigma, &totalPower);
		if (status != VI_SUCCESS) {return FALSE;}
	}
	else //for DFB
	{
		status = ag8614x_SRC_resultRecordDFB_Q (instHandle, &peakWavelength, &peakAmplitude, &modeOffset, &stopBand, &SMSR, &centerOffset, &bandwidth, &bandwidthAmp);
	}

	*PeakWavelength=peakWavelength;
	if (LaserType==LASER_TYPE_FP) *Sigma=sigma;
	else *Sigma=0;
	
	if (LaserType==LASER_TYPE_FP)
	{
		*BandWidth=0;
		*dSMSR=0;
	}
	else
	{
		*BandWidth=bandwidth;
		
	//	*dSMSR=SMSR;				SMSR测试结果如果>1000, 强制结果=0，理论上SMSR不会>1000
		*dSMSR=SMSR>1000 ? 0:SMSR;  
	}
	return TRUE;
}
