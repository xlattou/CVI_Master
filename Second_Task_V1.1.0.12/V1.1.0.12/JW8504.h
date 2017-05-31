#ifndef _JW8504_H_
#define _JW8504_H_
#include <toolbox.h>
#include <visa.h>  

extern int JW8504_Init_Port(ViSession *instHandle, int COMIndex);
extern int JW8504_Connect(ViSession instHandle); 
extern int JW8504_Set(ViSession instHandle,int channel,int wavelength,float att);
extern int JW8504_read(ViSession instHandle,float *ref1,float *att1,float *ref2,float *att2,float *ref3,float *att3,float *ref4,float *att4);
extern int JW8504_enable(ViSession instHandle,int channel,int en_flag)  ;
extern int JW8504_set_lock(ViSession instHandle,int channel,float lock)  ;

#endif

