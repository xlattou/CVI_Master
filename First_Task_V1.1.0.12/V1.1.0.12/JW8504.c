#include <formatio.h>
#include <userint.h>
#include <utility.h>
#include "JW8504.h"
unsigned char JW8504_connect_buf[5]={0X7B,0X03,0X40,0X42,0X7D};
unsigned char JW8504_set_buf[10]={0x7B, 0x08, 0x4E, 0x01, 0x03, 0x00, 0x00, 0x12,0x19,0x7D}; 	//设置命令字节 
unsigned char JW8504_read_buf[5]={0X7B,0X03,0X4C,0X36,0X7D};     
unsigned char JW8504_enable_buf[7]={0X7B,0X05,0X4A,0x01,0x01,0X34,0X7D};  
unsigned char JW8504_lock_buf[8]={0X7B,0X06,0X48,0x00,0X00,0x05,0X31,0X7D};   
int JW8504_Init_Port(ViSession *instHandle, int COMIndex)
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
	Status = viSetAttribute (*instHandle, VI_ATTR_ASRL_BAUD, 38400); 
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
	Status=JW8504_Connect(*instHandle) ;
	if (Status<0) return -1;
	return 0;
}
int JW8504_Connect(ViSession instHandle)    
{   
	int Status,index,rcount;
  	unsigned char sum, buf[500], temp;     

  Status=viClear(instHandle);
  if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
 //-----------先读取现有状态---------------------//
	Status = viWrite (instHandle, JW8504_connect_buf, 5, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器连接失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 5, &rcount);
	return 0;
}
int JW8504_Set(ViSession instHandle,int channel,int wavelength,float att)
{
	int Status,index,rcount,att_H,att_L,sum,w;
  	unsigned char  buf[500], temp;     
	 JW8504_set_buf[3]=channel;
	 /****************************************************************************
		wl 对应的波长(nm) 0x00:1310 0x01:1490 0x02:1550 0x03:1625
	****************************************************************************/
	 switch(wavelength)
	  {
	  case 1310: w=0; break;
	  case 1490: w=1;break;
	  case 1550: w=2;break;
	  case 1625: w=3;break;
	  }
	 JW8504_set_buf[4]=w;
	 att_H=((int)(att*100)& 0xff00)>>8;
	 att_L=((int)(att*100))&0xff;
	 JW8504_set_buf[6]=att_L;
	 JW8504_set_buf[7]=att_H;
	 sum=0;
	 for(index=0;index<8;index++)
	 {
		 sum+=(int)JW8504_set_buf[index];
	 }
	 sum=sum>256? 512-sum:256-sum;
	 JW8504_set_buf[8]=sum;
    Status=viClear(instHandle);
    if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
 //-----------先读取现有状态---------------------//
	Status = viWrite (instHandle, JW8504_set_buf, 10, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 5, &rcount);
	//if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	
	return 0;

}
int JW8504_read(ViSession instHandle,float *ref1,float *att1,float *ref2,float *att2,float *ref3,float *att3,float *ref4,float *att4)
{	 int a,b,c,d,e,f,g,h;
 	int Status,index,rcount;
  	unsigned char sum, buf[500], temp;     

  Status=viClear(instHandle);
  if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  

	Status = viWrite (instHandle, JW8504_read_buf, 5, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器连接失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 37, &rcount);
	a=((buf[5]<<8)+buf[4]);
	b=((buf[7]<<8)+buf[6]);
	c=((buf[13]<<8)+buf[12]);
	d=((buf[15]<<8)+buf[14]);
	e=((buf[21]<<8)+buf[20]);
	f=((buf[23]<<8)+buf[22]);
	g=((buf[29]<<8)+buf[28]);
	h=((buf[31]<<8)+buf[30]);
	*ref1=(float)a/100;
	*att1=(float)b/100;
	*ref2=(float)c/100;
	*att2=(float)d/100;
	*ref3=(float)e/100;
	*att3=(float)f/100;
	*ref4=(float)g/100;
	*att4=(float)h/100;
	
	
	return 0;

}
int JW8504_enable(ViSession instHandle,int channel,int en_flag)
{
	 int Status,index,rcount,sum;
  	 unsigned char  buf[500], temp;     
	 JW8504_enable_buf[3]=channel;
	 JW8504_enable_buf[4]=en_flag;
	 sum=0;
	 for(index=0;index<5;index++)
	 {
		 sum+=(int)JW8504_enable_buf[index];
	 }
	 sum=sum>256? 512-sum:256-sum;
	 JW8504_enable_buf[5]=sum;
    Status=viClear(instHandle);
    if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
 
	Status = viWrite (instHandle, JW8504_enable_buf, 7, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	Delay(0.3);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 5, &rcount);
	//if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	
	return 0;

}
int JW8504_set_lock(ViSession instHandle,int channel,float lock) 
{
	int Status,index,rcount,att_H,att_L,sum;
  	unsigned char  buf[500], temp;     
	 JW8504_lock_buf[3]=channel;
	 att_H=((int)(lock*100)& 0xff00)>>8;
	 att_L=((int)(lock*100))&0xff;
	 JW8504_lock_buf[4]=att_L;
	 JW8504_lock_buf[5]=att_H;
	 sum=0;
	 for(index=0;index<6;index++)
	 {
		 sum+=(int)JW8504_lock_buf[index];
	 }
	 sum=sum>256? 512-sum:256-sum;
	 JW8504_lock_buf[6]=sum;
    Status=viClear(instHandle);
    if (Status != VI_SUCCESS) {MessagePopup ("error", "visa clear device error");return -1;}  
 
	Status = viWrite (instHandle, JW8504_lock_buf, 8, &rcount);
	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	Delay(0.5);
	//读取buf
	memset (buf, 0, sizeof(buf));
	Status = viRead(instHandle, buf, 5, &rcount);
//	if (Status != VI_SUCCESS) {MessagePopup ("error", "JW8504衰减器设置失败！");return -1;} 
	
	return 0;
}
