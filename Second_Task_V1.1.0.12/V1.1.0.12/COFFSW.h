/****************************************************************************
 *
 * File:                FSW.h
 *
 * Author:              SUPERXON INC.(Roger Li)
 *
 * Description:         ��������⿪�ؿ��ƴ���
 *
 * Time:                2013-07-17
 *
 * version:				v1.0.0.0
 * 
 * Update Description:  
�˰汾ֻ֧���豸��ַΪ01�Ĺ⿪�� 
****************************************************************************/

#ifndef _COFFSW_H_
#define _COFFSW_H_

int COFFSW_Init(int COMIndex); //��ʼ�����������ܴ��ڶ˿ڱ�ռ�õ����������ͨ���ٴε��ô˺������  
int COFFSW_Close(int COMIndex); 
int COFFSW_SetChannel(int COMIndex, int channel);
int COFFSW02_SetChannel(int COMIndex, int channel);

#endif  