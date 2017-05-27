#include <visa.h>
#include <ansi_c.h>
#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif
   int PSS_init_port(ViSession *instHandle, int COMIndex)  ;
   int PSS_Get_Power(ViSession instHandle,int channel, double *Power) ;
   int PSS_Set_WaveLength(ViSession instHandle,int channel,int WaveLength); 
                            
#ifdef __cplusplus
    }
#endif
