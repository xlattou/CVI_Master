#ifndef _AQ833X_H_
#define _AQ633X_H_

#include <visa.h>   
#include <toolbox.h> 

extern int AQ637X_Init(ViSession *instHandle, ViRsrc Viname);
extern int AQ637X_Config(ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, ViReal64 resolution);
extern int AQ637X_Read(ViSession instHandle, int LaserType, ViReal64 span, double *PeakWavelength, double *Sigma, double *BandWidth, double *SMSR) ;
#endif  
