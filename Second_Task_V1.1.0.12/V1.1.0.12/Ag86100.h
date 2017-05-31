#ifndef _AG86100_H_
#define _AG86100_H_ 

#include <utility.h>
#include <ansi_c.h>
#include <formatio.h>
#include <visa.h>

struct struAg86100
{
char    MainSN[50];
char    ModuleSN[50];
float	X_Scale;
float  	X_Position;   
float	Y_Scale;
float	Y_Offset;
float 	ERFactor;  
int		Filter;
int		Wavelength;
int		MaskMargin;
char    MaskName[50];
int		Channel_O;   
};

extern BOOL Ag86100_Init(ViSession *instHandle, ViRsrc Viname);
extern BOOL Ag86100_SET_O(ViSession instHandle, const double ERFactor, double X_Scale, double X_Position, double Y_Scale, double Y_Offset, int Filter, int wavelength, char *MaskName, double MaskMargin, int Channel_O);
extern BOOL Ag86100_GET_Er(ViSession instHandle, double *Er);
extern BOOL Ag86100_Set_Clear(ViSession instHandle);
extern BOOL Ag86100_Set_AutoScale(ViSession instHandle);
extern BOOL Ag86100_GET_O_PPj(ViSession instHandle, double *PPj); 
extern BOOL Ag86100_GET_O_RSj(ViSession instHandle, double *RSj);
extern BOOL Ag86100_SET_MaskMargin(ViSession instHandle, double MaskMargin, char *MaskName);
extern BOOL Ag86100_GET_MaskHits(ViSession instHandle, int WaveForms);
extern BOOL Ag86100_GET_O_Rise(ViSession instHandle, double *Rise); 
extern BOOL Ag86100_GET_O_Fall(ViSession instHandle, double *Fall);
extern BOOL Ag86100_SET_O_EYE(ViSession instHandle); 
extern BOOL Ag86100_SET_O_ER(ViSession instHandle, double X_Scale, double X_Position, double Y_Scale, double Y_Offset, int Channel_O); 

//获取主机序列号
extern BOOL Ag86100_GET_MainSN(ViSession instHandle, char *MainSN); 
//获取模块序列号
extern BOOL Ag86100_GET_ModuleSN(ViSession instHandle, char *ModuleSN); 
extern BOOL Ag86100_GET_O_CROSsing(ViSession instHandle, double *CROSsing); 
BOOL Ag86100_Set_Run(ViSession instHandle);
BOOL Ag86100_Set_Stop(ViSession instHandle);
extern BOOL Ag86100_GET_MaskFSample(ViSession instHandle, int *totalcount);
extern BOOL Ag86100_GET_WaveForms(ViSession instHandle, int *wavecount);

#endif 
