

/****************************************************************************
 *
 * File:                FSW.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         电子34所单模光开关控制代码
 *
 * Time:                2013-07-17
 *
 * version:				v1.0.0.0
 * 
 * Update Description:  
****************************************************************************/

#ifndef _FSW_H_
#define _FSW_H_

int FSW_Init(int COMIndex); //初始化函数，可能存在端口被占用的情况，可以通过再次调用此函数解决  
int FSW_Close(int COMIndex); 
int FSW_SetChannel(int COMIndex, int channel);

#endif  
