#include <userint.h>
#include <utility.h>
#include <visa.h>
#include <ansi_c.h>

int PSS_init_port(ViSession *instHandle, int COMIndex)
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
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_BAUD, 115200); 
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
	return 0;


}
int PSS_Get_Power(ViSession instHandle,int channel, double *Power)
{
	int Status, index,rcount,count;
	unsigned char  buf[50],WriteBuf[30],ReadBuf[10]; 
	float value;
	Status = viClear (instHandle);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
	sprintf(WriteBuf,"Read:power Channel%d\n",channel);
	Status = viWrite (instHandle, WriteBuf, 21, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "PMSII功率计状态读取命令发送失败！");return -1;} 
	Delay(0.3);
	
	Status=viGetAttribute(instHandle,VI_ATTR_ASRL_AVAIL_NUM,&count);
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, count, &rcount); 
	
	memset(ReadBuf,0,sizeof(ReadBuf));

	for (index=9; index<15; index++)
	{ReadBuf[index-9]= buf[index];}
	  value=atof(ReadBuf);
	*Power=value;	
		
	return 0;
}


int PSS_Set_WaveLength(ViSession instHandle,int channel,int WaveLength)
{
	int Status, index,rcount;
	unsigned char  buf[50],WriteBuf[38],ReadBuf[10]; 
	float value;
	Status = viClear (instHandle);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
	sprintf(WriteBuf,"configure:WaveLength Channel%d %d\n",channel,WaveLength);
	index=(channel>9)? 36:35;
	Status = viWrite (instHandle, WriteBuf, index, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "波长设置失败！");return -1;} 
	Delay(0.3);
	Status=viGetAttribute(instHandle,VI_ATTR_ASRL_AVAIL_NUM,&rcount);
	if(rcount) {MessagePopup("error","波长设置失败！");return -1;}
 return 0;
}
