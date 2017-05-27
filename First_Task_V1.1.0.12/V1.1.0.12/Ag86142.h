#ifndef _AG86142_H_
#define _AG86142_H_

#include <visa.h>   
#include <toolbox.h> 

extern BOOL Ag86142_Init(ViSession *instHandle, ViRsrc Viname);
extern BOOL Ag86142_Config(int LaserType, ViSession instHandle, ViReal64 startWavelength, ViReal64 stopWavelength, ViReal64 resolutionBandwidth);
extern BOOL Ag86142_Read(int LaserType, ViSession instHandle, double *PeakWavelength, double *Sigma, double *BandWidth, double *SMSR) ;  

#endif  
