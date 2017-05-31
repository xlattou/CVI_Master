#include "global.h"
#include <visa.h>
#include <userint.h>
#include <utility.h>
#include <ansi_c.h>

BOOL Visa_Write(ViSession vi, ViBuf buf)
{
int		status, rcount;
char    buffer[128];

	strcpy (buffer, buf);
	strcat (buffer, "\n");
	status = viWrite (vi, buf,StringLength(buf), &rcount); 
	if(status!=VI_SUCCESS) return FALSE; 
	
	return TRUE;
}

BOOL Visa_OPC(ViSession vi, int Timeout)
{
int  	status, rcount; 
char 	buf[128]=""; 
int 	timer_star, timer; 

	timer_star=Timer();
	do
	{
		status = viWrite (vi, "*OPC?\n",StringLength("*OPC?\n"), &rcount);
		if (status < VI_SUCCESS) return FALSE; 
		
		Delay(0.1); 
    	status = viRead(vi, buf, 10, &rcount); 
    	if (status < VI_SUCCESS) return FALSE;
		
		sscanf (buf, "%d", &rcount);
    	
    	timer = Timer()-timer_star;
		
	}while ((timer<Timeout)&&(rcount==0)); //要验证执行结束后buf的返回值判断
		
	return TRUE;
}

BOOL Visa_Query(ViSession vi, ViBuf buf)
{
int		status, rcount;
char	buffer[128];

	strcpy (buffer, buf);
	strcat (buffer, "\n");
	status = viWrite (vi, buf,StringLength(buf), &rcount); 
	if(status!=VI_SUCCESS) return FALSE; 
		
	Delay(0.2); 
	status = viRead(vi, buffer, 127, &rcount); 
	if (status < VI_SUCCESS) return FALSE;
    	
	return TRUE;
}

void Update_RESULTSInfo(int panel, int control, unsigned char *resultstr, BOOL bInsert)
{
int Count;

	GetNumTextBoxLines (panel, control, &Count);
	
	if (bInsert) {InsertTextBoxLine (panel, control, Count, resultstr);} 
	else 		 {ReplaceTextBoxLine (panel, control, Count-2, resultstr);} 
	
	SetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE, Count);
	ProcessDrawEvents();
	Delay(0); 
}

void Insert_Info(int panel, int control, unsigned char *resultstr)
{
int Count;

	GetNumTextBoxLines (panel, control, &Count);
	
	InsertTextBoxLine (panel, control, Count, resultstr); 

	SetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE, Count);
	ProcessDrawEvents();
	Delay(0); 
}

void Update_Info(int panel, int control, unsigned char *resultstr)
{
int Count;

	GetNumTextBoxLines (panel, control, &Count);

	ReplaceTextBoxLine (panel, control, Count-2, resultstr); 
	
	SetCtrlAttribute (panel, control, ATTR_FIRST_VISIBLE_LINE, Count);
	ProcessDrawEvents();
	Delay(0); 
}

void set_led_status(int panel, int control, int status_led, int status_cursor)
{
	SetWaitCursor (status_cursor); 
	SetCtrlVal (panel, control, status_led);  
}
