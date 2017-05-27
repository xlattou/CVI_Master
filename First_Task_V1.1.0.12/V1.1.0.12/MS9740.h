#ifndef _MS9740_H_
#define _MS9740_H_

#include <visa.h>   
#include <toolbox.h> 

extern int MS9740_Init(ViSession *instHandle, ViRsrc Viname);
extern int MS9740_Config(ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, ViReal64 resolution);
extern int MS9740_Read(ViSession instHandle, int LaserType, ViReal64 ctrwl, ViReal64 span, double *PeakWavelength, double *Sigma, double *BandWidth, double *SMSR);
extern int MS9740_Close(ViSession instHandle);
extern int MS9740_Stop(ViSession instHandle);
#endif  
