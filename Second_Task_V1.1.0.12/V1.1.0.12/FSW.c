#include <rs232.h>
#include <userint.h>
#include <utility.h>
#include "FSW.h" 

int FSW_Init(int COMIndex)
{
int  Status; 

	Status = OpenComConfig (COMIndex, "", 9600, 0, 8, 1, 0, -1);  
	if(Status != 0) 
	{
		MessagePopup ("error", "光开关串口连接失败，请检查串口设置是否正确及串口是否被占用！"); 
		return -1;
	} 
	
	return 0;
}

int FSW_Close(int COMIndex)
{
int Status; 

	Status = CloseCom (COMIndex);
	if(Status != 0) {MessagePopup ("error", "光开关串口关闭失败");return -1;} 
	
	return 0;
}

int FSW_SetChannel(int COMIndex, int channel)
{
	int Status;

	if ((8<channel) || (1>channel))
	{
		MessagePopup ("提示", "FSW通道选择错误    ");
		return -1;
	}
	
	Status = ComWrtByte (COMIndex, channel);
	if(1 != Status) {MessagePopup ("error", "FSW Send command fail"); return -1;}  
	
	Delay(0.1); 

	return 0;
}
