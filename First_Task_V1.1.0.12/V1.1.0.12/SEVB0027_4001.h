

/****************************************************************************
 *
 * File:               _SEVB0027_4001_H_.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         SEVB0027_4001¿ØÖÆ´úÂë
 *
 * Time:                2013-07-17
 *
 * version:				v1.0.0.0
 * 
 * Update Description:  
****************************************************************************/

#ifndef _SEVB0027_4001_H_
#define _SEVB0027_4001_H_

#include "CH341A_DLL.h"   

int SEVB0027_4001_Init (int *Inst, char *SN);
int SEVB0027_4001_SetChannel(int Inst, int channel);

#endif  
