#ifndef _AG86120B_H_
#define _AG86120B_H_

#include <visa.h>   
#include <toolbox.h> 

extern int Ag86120B_Init(ViSession *instHandle, ViRsrc Viname);
extern int Ag86120B_Config(ViSession instHandle);
extern int Ag86120B_Read (ViSession instHandle, double *PeakWavelength, double *Power);  

#endif  
