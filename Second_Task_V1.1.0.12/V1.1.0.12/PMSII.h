/****************************************************************************
 *
 * File:                PMSII.h
 *
 * Author:              Superxon(Roger Li)
 *
 * Description:         PMSII功率计基本功能代码
 *
 * Time:                2009-01-09
 *
 * version:				V1.0.0.0
 * version note			new document
 
 *现有问题：
 1.Mode:1-low power  0-normal power  的支持有问题
 2.dBrel模式下的功率值读取
****************************************************************************/
#include <toolbox.h>
#include <visa.h>  

#ifndef _PMSII_H_
#define _PMSII_H_ 

#define PMSII_UNIT_WATT		0
#define PMSII_UNIT_DBM 		1 

#define PMSII_MODE_NORMAL_POWER		0
#define PMSII_MODE_LOW_POWER 		1 

#define PMSII_MODE_WATT_DBM		0
#define PMSII_MODE_RESERVE		1 
#define PMSII_MODE_MINMAX		2
#define PMSII_MODE_DBREL		3 

#define PMSII_MODE_WAVE_1550	0
#define PMSII_MODE_WAVE_1310	1 
#define PMSII_MODE_WAVE_850		2 
#define PMSII_MODE_WAVE_980		3 
#define PMSII_MODE_WAVE_1480	4 
#define PMSII_MODE_WAVE_1610	5 

//内部函数
void PMSII_COMM_Transfer(unsigned char comm_byte, unsigned char *comm_buff); 

extern int PMSII_Init_Port(ViSession *instHandle, int COMIndex, int WaveLength);
extern int PMSII_Set_Unit(ViSession instHandle, int unit);
extern int PMSII_Set_Wavlength(ViSession instHandle, int wavelength); 
extern int PMSII_Set_Mode(ViSession instHandle, int mode); 
extern int PMSII_Get_Power(ViSession instHandle, double *Power); 
#endif
