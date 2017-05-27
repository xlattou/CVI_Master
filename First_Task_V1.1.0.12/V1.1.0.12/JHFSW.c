#include <ansi_c.h>
#include <rs232.h>
#include <userint.h>
#include <utility.h>
#include "JHFSW.h" 

int JHFSW_Init(int COMIndex)
{
int  Status; 
int count;

char buf_w[8],buf_r[8];           

	Status = OpenComConfig (COMIndex, "", 9600, 0, 8, 1, 0, -1);  
	if(Status != 0) 
	{
		MessagePopup ("error", "光开关串口连接失败，请检查串口设置是否正确及串口是否被占用！"); 
		return -1;
	} 
	
	buf_w[00] = 0x01;
	buf_w[01] = 0x06;
	buf_w[02] = 0x00;
	buf_w[03] = 0x01;
	buf_w[04] = 0x00;
	buf_w[05] = 0x01;
	buf_w[06] = 0x19;
	buf_w[07] = 0xCA;
			
	count = ComWrt (COMIndex, buf_w, sizeof(buf_w));
	if(sizeof(buf_w) != count) {MessagePopup ("error", "FSW Send command fail"); return -1;}  
	
	count = ComRd (COMIndex, buf_r, sizeof(buf_w));
	if(sizeof(buf_w) != count) 
	{
		MessagePopup ("error", "JHFSW Get responce fail"); 
		return -1;
	}    
	
	if (JHFSW_SetChannel (COMIndex, 1) < 0)
	{
		MessagePopup ("error", "JHFSW Send command fail"); 
		return -1;
	}   
	
	return 0;
}

int JHFSW_Close(int COMIndex)
{
int Status; 

	Status = CloseCom (COMIndex);
	if(Status != 0) {MessagePopup ("error", "光开关串口关闭失败");return -1;} 
	
	return 0;
}

int JHFSW_SetChannel(int COMIndex, int channel)
{
	int count;
	char buf_w[8],buf_r[8];

	switch (channel)
	{
		case 1:
			buf_w[00] = 0x01;
			buf_w[01] = 0x06;
			buf_w[02] = 0x00;
			buf_w[03] = 0x03;
			buf_w[04] = 0x00;
			buf_w[05] = 0x00;
			buf_w[06] = 0x79;
			buf_w[07] = 0xCA;
		break;
		
		case 2:
			buf_w[00] = 0x01;
			buf_w[01] = 0x06;
			buf_w[02] = 0x00;
			buf_w[03] = 0x03;
			buf_w[04] = 0x00;
			buf_w[05] = 0x01;
			buf_w[06] = 0xB8;
			buf_w[07] = 0x0A;
		break;
		
		case 3:
			buf_w[00] = 0x01;
			buf_w[01] = 0x06;
			buf_w[02] = 0x00;
			buf_w[03] = 0x03;
			buf_w[04] = 0x00;
			buf_w[05] = 0x02;
			buf_w[06] = 0xF8;
			buf_w[07] = 0x0B;
		break;
		
		case 4:
			buf_w[00] = 0x01;
			buf_w[01] = 0x06;
			buf_w[02] = 0x00;
			buf_w[03] = 0x03;
			buf_w[04] = 0x00;
			buf_w[05] = 0x03;
			buf_w[06] = 0x39;
			buf_w[07] = 0xCB;
		break;
		
		default:
			
			MessagePopup ("提示", "光开关通道选择错误    ");
		return -1;
	}
	
	count = ComWrt (COMIndex, buf_w, sizeof(buf_w));
	if(sizeof(buf_w) != count) 
	{
		MessagePopup ("error", "FSW Send command fail"); 
		return -1;
	}  
	
	Delay(1); 
	
	count = ComRd (COMIndex, buf_r, sizeof(buf_w));
	if(sizeof(buf_w) != count) 
	{
		MessagePopup ("error", "JHFSW Get responce fail"); 
		return -1;
	}

	return 0;
}
