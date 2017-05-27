

/****************************************************************************
 *
 * File:                FSW.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         绵阳超光光开关控制代码
 *
 * Time:                2013-07-17
 *
 * version:				v1.0.0.0
 * 
 * Update Description:  
此版本只支持设备地址为01的光开关 
****************************************************************************/

#ifndef _JHFSW_H_
#define _JHFSW_H_

int JHFSW_Init(int COMIndex); //初始化函数，可能存在端口被占用的情况，可以通过再次调用此函数解决  
int JHFSW_Close(int COMIndex); 
int JHFSW_SetChannel(int COMIndex, int channel);

#endif  
