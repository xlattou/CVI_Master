#include <formatio.h>
#include <userint.h>
#include <utility.h>
#include "PMSII.h"

unsigned char pmsii_comm_buf[8]={0xEF, 0xEF, 0x05, 0x05, 0x60, 0x03, 0x00, 0x00}; 	//设置命令字节
unsigned char pmsii_status_buf[7]={0xEF, 0xEF, 0x04, 0x04, 0x60, 0x06, 0x4C};   	//状态读取命令字节

int PMSII_Init_Port(ViSession *instHandle, int COMIndex, int WaveLength)
{
ViSession 	defaultRM;
int 		rcount;
char		Temp_str[1024]; 
int 		Status;  
  
	viOpenDefaultRM (&defaultRM);
						
    sprintf (Temp_str, "ASRL%d::INSTR", COMIndex);
    Status = viOpen (defaultRM, Temp_str, VI_NULL, VI_NULL, instHandle);  
    if (Status != VI_SUCCESS) return -1; 
    
    //设置串口通讯的参数		   
	Status = viSetAttribute(*instHandle, VI_ATTR_TMO_VALUE, 2000);  
    if(Status !=VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_BAUD, 9600); 
    if (Status != VI_SUCCESS) return -1;
    Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_DATA_BITS, 8); 
    if (Status != VI_SUCCESS) return -1;
    Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE); 
    if (Status != VI_SUCCESS) return -1;
    Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE); 
    if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_FLOW_CNTRL, VI_ASRL_FLOW_NONE);
    if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_END_IN, VI_ASRL_END_TERMCHAR);
    if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_END_OUT, VI_ASRL_END_NONE);
    if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_TERMCHAR, 0x0A);   //十进制的10 来做 结束符Set the termination character to 0xA
	if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_TERMCHAR_EN, VI_FALSE);  //Specify that the read operation should terminate when a termination character is received.
 	if (Status != VI_SUCCESS) return -1;
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_END_IN, VI_ASRL_END_NONE);  //Specifies that there is no END indicator for read operations, so reads will not terminate until all of the requested data is received (or an error occurs)
 	if (Status != VI_SUCCESS) return -1;	

	Status = PMSII_Set_Unit (*instHandle, PMSII_UNIT_DBM); 
	if (Status<0) return -1;

	Status = PMSII_Set_Mode (*instHandle, PMSII_MODE_WATT_DBM); 
	if (Status<0) return -1;
	
	Status = PMSII_Set_Wavlength (*instHandle, WaveLength); 
	if (Status<0) return -1;	
	return 0;
}

void PMSII_COMM_Transfer(unsigned char comm_byte, unsigned char *comm_buf)
{
	int index;
	unsigned char sum;
	
	comm_buf[6] = comm_byte;
	
	sum=0;
	for (index=0; index<7; index++)
	{sum += comm_buf[index];}
	comm_buf[7] = sum;
	
	return ;
}

int PMSII_Set_Mode(ViSession instHandle, int mode)
{
	int Status, index, rcount;
	unsigned char sum, buf[500], temp;

	Status = viClear (instHandle);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  	
//-----------先读取现有状态---------------------//
	Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 14, &rcount); 

	//检查sum是否正确
	sum=0;
	for (index=0; index<13; index++)
	{sum += buf[index];}
	if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
	
	//判断mode设置值是否正确
	temp = buf[6] & 0x0F;
	switch (temp)
	{
		case 0:
			if (mode==PMSII_MODE_WATT_DBM) return 0;
			break; 
		case 1:
			if (mode==PMSII_MODE_RESERVE) return 0;
			break; 
		case 2:
			if (mode==PMSII_MODE_MINMAX) return 0;
			break; 
		case 3:
			if (mode==PMSII_MODE_DBREL) return 0;
			break; 
		default:
			{MessagePopup ("error", "PMSII功率计模式设置失败！");return -1;} 
			break;
	}		
//-----------先读取现有状态---------------------// 
//-----------当前状态需要更新-------------------// 		
	switch (mode)
	{
		case PMSII_MODE_WATT_DBM:
			PMSII_COMM_Transfer (0x00, pmsii_comm_buf);  
			break; 
		case PMSII_MODE_RESERVE:
			PMSII_COMM_Transfer (0x01, pmsii_comm_buf);  
			break; 
		case PMSII_MODE_MINMAX:
			PMSII_COMM_Transfer (0x02, pmsii_comm_buf);  
			break; 
		case PMSII_MODE_DBREL:
			PMSII_COMM_Transfer (0x03, pmsii_comm_buf);  
			break; 
		default:
			{MessagePopup ("error", "PMSII功率计模式设置失败！");return -1;} 
			break;
	}
	
	Status = viWrite (instHandle, pmsii_comm_buf, 8, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计设置命令发送失败！");return -1;}  
	Delay(0.3);
	Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 14, &rcount); 

	//检查sum是否正确
	sum=0;
	for (index=0; index<13; index++)
	{sum += buf[index];}
	if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
	
	//判断设置值是否正确
	temp = buf[6] & 0x0F;
	switch (temp)
	{
		case 0:
			if (mode==PMSII_MODE_WATT_DBM) return 0;
			break; 
		case 1:
			if (mode==PMSII_MODE_RESERVE) return 0;
			break; 
		case 2:
			if (mode==PMSII_MODE_MINMAX) return 0;
			break; 
		case 3:
			if (mode==PMSII_MODE_DBREL) return 0;
			break; 
		default:
			{MessagePopup ("error", "PMSII功率计模式设置失败！");return -1;} 
			break;
	}		
	
	//到这一步说明模式设置失败
	{MessagePopup ("error", "PMSII功率计模式设置失败！");return -1;}   
//-----------当前状态需要更新-------------------// 	
	
	return 0;
}

int PMSII_Set_Unit(ViSession instHandle, int unit)
{
	int Status, index, count, rcount;
	unsigned char sum, buf[500], temp;

	Status = viClear (instHandle);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;} 
	
//-----------先读取现有状态---------------------//
	Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 14, &rcount); 

	//检查sum是否正确
	sum=0;
	for (index=0; index<13; index++)
	{sum += buf[index];}
	if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
	
	//判断unit设置值是否正确
	temp = buf[9] & 0x0F;
	if (temp<4) 		//unit = watt mw\uw\nw
	{
		if (unit==PMSII_UNIT_WATT) return 0;  	
	}
	else				//unit = dBm or dB
	{
		if (unit==PMSII_UNIT_DBM) return 0;
	}
//-----------先读取现有状态---------------------// 
	
//-----------当前状态需要更新-------------------// 	
	count=0;  
	do
	{
		PMSII_COMM_Transfer (0x00, pmsii_comm_buf);
		
		Status = viWrite (instHandle, pmsii_comm_buf, 8, &rcount);
		if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计设置命令发送失败！");return -1;}  
		Delay(0.3);
		Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
		if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
		Delay(0.3);
		//读取buf
		memset (buf, 0, sizeof(buf));
		Status = viRead(instHandle, buf, 14, &rcount); 
	
		//检查sum是否正确
		sum=0;
		for (index=0; index<13; index++)
		{sum += buf[index];}
		if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
	
		//判断设置值是否正确
		temp = buf[9] & 0x0F;
		if (temp<4) 		//unit = watt mw\uw\nw
		{
			if (unit==PMSII_UNIT_WATT) break;	
		}
		else				//unit = dBm or dB
		{
			if (unit==PMSII_UNIT_DBM) break;
		}
		count++;
	} while (count<2);
	
	if (count>=2) {MessagePopup ("error", "PMSII功率计单位设置失败！");return -1;}   
//-----------当前状态需要更新-------------------// 	
	
	return 0;
}

int PMSII_Set_Wavlength(ViSession instHandle, int wavelength)
{
	int Status, index, count, rcount;
	unsigned char sum, buf[500], temp; 

	Status = viClear (instHandle);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  	

//-----------先读取现有状态---------------------// 
	Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 14, &rcount); 

	//检查sum是否正确
	sum=0;
	for (index=0; index<13; index++)
	{sum += buf[index];}
	if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}

	//判断设置值是否正确
	temp = buf[6] & 0x70;
	switch (temp)
	{
		case 0x00:
			if (wavelength==1550) return 0;
			break; 
		case 0x10:
			if (wavelength==1310) return 0;
			break; 
		case 0x20:
			if (wavelength==850) return 0;
			break; 
		case 0x30:
			if (wavelength==980) return 0;
			break; 
		case 0x40:
			if (wavelength==1480) return 0;
			break; 
		case 0x50:
			if (wavelength==1610) return 0;
			break; 
		default:
			{MessagePopup ("error", "PMSII功率计波长设置失败！");return -1;}         
			break;
	}	
//-----------先读取现有状态---------------------//  

//-----------当前状态需要更新-------------------// 		
	count=0;
	do
	{
		PMSII_COMM_Transfer (0x04, pmsii_comm_buf);
		
		Status = viWrite (instHandle, pmsii_comm_buf, 8, &rcount);
		if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计设置命令发送失败！");return -1;}  
		Delay(0.3);
		Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
		if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
		Delay(0.3);
		//读取buf
		memset (buf, 0, sizeof(buf));
		Status = viRead(instHandle, buf, 14, &rcount); 
	
		//检查sum是否正确
		sum=0;
		for (index=0; index<13; index++)
		{sum += buf[index];}
		if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
		
		//判断设置值是否正确
		temp = buf[6] & 0x70;
		switch (temp)
		{
			case 0x00:
				if (wavelength==1550) return 0;
				break; 
			case 0x10:
				if (wavelength==1310) return 0;
				break; 
			case 0x20:
				if (wavelength==850) return 0;
				break; 
			case 0x30:
				if (wavelength==980) return 0;
				break; 
			case 0x40:
				if (wavelength==1480) return 0;
				break; 
			case 0x50:
				if (wavelength==1610) return 0;
				break; 
			default:
				{MessagePopup ("error", "PMSII功率计波长设置失败！");return -1;}         
				break;
		}
		count++;
	} while (count<7); /***波长设置循环次数加1**Eric.Yao***/
	
	if (count>=7) {MessagePopup ("error", "PMSII功率计波长设置失败！");return -1;}   
//-----------当前状态需要更新-------------------// 	
	
	return 0;
}

int PMSII_Get_Power(ViSession instHandle, double *Power)
{
	int Status, index, count, rcount;
	unsigned char sum, buf[500], temp; 
	float value;
	
	Status = viClear (instHandle);
	if ( Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
	
	Status = viWrite (instHandle, pmsii_status_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 14, &rcount); 

	//检查sum是否正确
	sum=0;
	for (index=0; index<13; index++)
	{sum += buf[index];}
	if (buf[13] != sum) {MessagePopup ("error", "PMSII功率计状态读取sum失败！");return -1;}
	
	temp = buf[9] & 0x0F; 
	if (temp==4) //for dBm or dB
	{
		value=(256*buf[7]+buf[8]-9000)/100.;
	}
	else if (temp<4)		//for mw\uw\nw 
	{
		temp = buf[9] & 0xF0;
		switch (temp)
		{
			case 0x10:
				value= (256*buf[7]+buf[8])/10.; 
				break; 
			case 0x20:
				value= (256*buf[7]+buf[8])/100.; 
				break; 
			case 0x30:
				value= (256*buf[7]+buf[8])/1000.; 
				break; 
			default:
				{MessagePopup ("error", "PMSII功率计测量值读取失败！");return -1;}      
				break;
		}
	}
	else
	{MessagePopup ("error", "PMSII功率计测量值读取失败！");return -1;}    
		
	*Power=value;
		
	return 0;
}

