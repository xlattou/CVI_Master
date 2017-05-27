//==============================================================================
//
// Title:       dllDemo
// Purpose:     A short description of the application.
//
// Created on:  2013-07-03 at 13:38:13 by Roger.
// Copyright:   . All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>
#include <cvirte.h>     
#include <userint.h>
#include "dllDemo.h"
#include "toolbox.h"
#include "ch341a_dll.h"
#include "evb5.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

static int panelHandle;

//==============================================================================
// Static functions

//==============================================================================
// Global variables

//==============================================================================
// Global functions

/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
    int error = 0;
    
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "dllDemo.uir", PANEL));
    
    /* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
    errChk (RunUserInterface ());

Error:
    /* clean up */
    DiscardPanel (panelHandle);
    return 0;
}

//==============================================================================
// UI callback function prototypes

/// HIFN Exit when the user dismisses the panel.
int CVICALLBACK panelCB (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface (0);
    return 0;
}

int CVICALLBACK on_I2C_CHECK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{ 
	unsigned char HOST_SDA_IN, HOST_SDA_IN_error;
 int error, CONNECTION_OK, i, No=0, iIndex;
 char str[65536], def_filename[256]="m.lnk", txtFile[300]="";
 int stat;
 int status;
 int lpdwNumDevices;
 int I2C_RATE;
char buf[256];

	switch (event)
	{
		case EVENT_COMMIT:

			SetCtrlVal (panel, PANEL_NUM_DLLVersion, GetDLLVersion_DLL());
			
			SetWaitCursor (1);
			ClearListCtrl (panel, PANEL_LISTBOX_I2C_HOST);
			ProcessDrawEvents (); 
			
			memset (buf, '0',sizeof(buf));
			GetDLLVersion2_DLL(buf);
			SetCtrlVal (panel, PANEL_STR_DLLVER, buf); 
			
			//SetUSBiMode_DLL(1);//set the USB-I2C rate, 100KHz, default
			
			//Search I2C host controller and I2C slave MCU at MCU_addr 
			No = 0;
			USBHandle=-1;
			for (iIndex=0; iIndex<255; iIndex++)
			{	error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
				if (error!=-1) //USB host found
			  	{	No++;
			    	sprintf (str, "Host %03d: 0x%03X", No,iIndex);
					InsertListItem (panel, PANEL_LISTBOX_I2C_HOST, No-1, str, No-1);
					//GetUSBHostDescr(iIndex,  OutBuf);
					//GetUSBHostConfigDescr(iIndex,  OutBuf); 
					ProcessDrawEvents ();
 			  	}
			}
			
			iIndex=0x278; //LPT at 0x278
			{	error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
				if (error!=-1) //USB host found
			  	{	No++;
			    	sprintf (str, "Host %03d: 0x%03X", No,iIndex);
					InsertListItem (panel, PANEL_LISTBOX_I2C_HOST, No-1, str, No-1);
					//GetUSBHostDescr(iIndex,  OutBuf);
					//GetUSBHostConfigDescr(iIndex,  OutBuf); 
					ProcessDrawEvents ();
 			  	}
			}
			
			iIndex=0x378; //LPT at 0x378
			{	error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
				if (error!=-1) //USB host found
			  	{	No++;
			    	sprintf (str, "Host %03d: 0x%03X", No,iIndex);
					InsertListItem (panel, PANEL_LISTBOX_I2C_HOST, No-1, str, No-1);
					//GetUSBHostDescr(iIndex,  OutBuf);
					//GetUSBHostConfigDescr(iIndex,  OutBuf); 
					ProcessDrawEvents ();
 			  	}
			}

			status = GetF320Numbers(&lpdwNumDevices);
			if( (status == 0) && (lpdwNumDevices>0) )
			{
				for (iIndex=0x800; iIndex<lpdwNumDevices+0x800; iIndex++)
				{	error = I2C_HOST_INITIALIZATION_DLL (iIndex); 
					if (error!=-1) //USB host found
				  	{	No++;
				    	sprintf (str, "Host %03d: 0x%03X", No,iIndex);
						InsertListItem (panel, PANEL_LISTBOX_I2C_HOST, No-1, str, No-1);
						ProcessDrawEvents ();
	 			  	}
				}
			}

 			if (No>0) 
 			{	GetLabelFromIndex (panel, PANEL_LISTBOX_I2C_HOST, 0, str);
				sscanf (str, "Host %03d : 0x%03X", &No, &iIndex);
				SetCtrlVal (panel, PANEL_NUM_HOSTHandle, iIndex); 
				USBHandle = iIndex;
			}
			else
			{
				SetCtrlVal (panel, PANEL_NUM_HOSTHandle, 0); 
			}
			
 			SetWaitCursor (0);

			break;
	}
	return 0;
}

int CVICALLBACK on_SEARCH_I2C_ADD (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int No, item_index, I2CSLAADD;
int device_addr, rom_addr, error;
char str[50];
unsigned char rom_value_arr[256];
	
	switch (event)
	{
		case EVENT_COMMIT:

			SetWaitCursor (1);
			ClearListCtrl (panel, PANEL_LISTBOX_I2C_ADD);
			ProcessDrawEvents ();  
			
			No=0;
		  	for (device_addr=0; device_addr<0xFF; device_addr+=2)
			{   //device_addr = 0xa2;
				if (I2C_SLAVE_SEARCH_DLL(USBHandle, device_addr)==0)
			  	{	No++;
			    	sprintf (str, "Device %2d: 0x%02X", No,device_addr);
					InsertListItem (panel, PANEL_LISTBOX_I2C_ADD, No-1, str, No-1);
					ProcessDrawEvents ();
 			  	}
		 	}

			SetWaitCursor (0);
			
			break;
	}
	return 0;
}

int CVICALLBACK on_LISTBOX_I2C_HOST (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
char str[50],temp_char;
int rom_addr, I2CSLAADD;
int  item_index;
int No, iIndex;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlIndex (panel, PANEL_LISTBOX_I2C_HOST, &item_index);
		
			GetLabelFromIndex (panel, PANEL_LISTBOX_I2C_HOST, item_index, str);

			sscanf (str, "Host %03d : 0x%03X", &No, &iIndex);

			SetCtrlVal (panel, PANEL_NUM_HOSTHandle, iIndex); 
			USBHandle = iIndex;

			break;
	}
	return 0;
}

int CVICALLBACK on_Get_USBSN (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int error;
char USBSN[30];

	switch (event)
	{
		case EVENT_COMMIT:

			SetCtrlVal (panel, PANEL_STR_USBSN, "");

			GetCtrlVal (panel, PANEL_NUM_HOSTHandle, &USBHandle);
			memset (USBSN, 0, sizeof(USBSN));
			error = GetF320SerialNumber(USBHandle-0x800, USBSN);
			if (error<0) return -1; //USB host not found!
			SetCtrlVal (panel, PANEL_STR_USBSN, USBSN);

			break;
	}
	return 0;
}

int CVICALLBACK on_Get_EVBSN (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error, EVB_addr;
    int i, reg_add, reg_length; 
    char EVBSN[30];

	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlVal (panel, PANEL_STR_EVBSN, "");

			GetCtrlVal (panel, PANEL_NUM_HOSTHandle, &USBHandle);
			error = I2C_BYTEs_READ_DLL (USBHandle, 0x84, 0, 256, EVB5.pStrF0);
			if (error<0) return -1; //EVB access error!
	
			memset (EVBSN, 0, sizeof(EVBSN));
			for(i=0;i<16; i++)
			{EVBSN[i]=EVB5.sStrF0.vendorSN[i];}
			SetCtrlVal (panel, PANEL_STR_EVBSN, EVBSN);
			
			break;
	}
	return 0;
}

int CVICALLBACK ON_Set_EVBSN (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{int error;
 int i, reg_add, reg_length; 
 unsigned char temp_arr[256];
 char EVBSN[30];
 int EVB_addr=0x84;

	switch (event)
		{
		case EVENT_COMMIT:
	
			SetWaitCursor (1);
			GetCtrlVal (panel, PANEL_NUM_HOSTHandle, &USBHandle);
			GetCtrlVal (panel, PANEL_STR_EVBSN, EVBSN);

			//set password
			reg_add=0x7B;
			EVB5.sStrF0.Password = 0x6B63614A; //"Jack" 
			reg_add =(int)(&EVB5.sStrF0.Password) - (int)(&EVB5.sStrF0.FirstByte); 
			error = I2C_BYTEs_WRITE_DLL (USBHandle, EVB_addr, reg_add, 4, EVB5.pStrF0, 0.1);			
	
			//table0
 			EVB5.sStrF0.LUTIndex=0;
 			reg_add = (int)(&EVB5.sStrF0.LUTIndex)-(int)(&EVB5.sStrF0.FirstByte); 
			error = I2C_BYTEs_WRITE_DLL (USBHandle, EVB_addr, reg_add, 1, EVB5.pStrF0, 0.1);			
		
			for(i=0;i<16; i++)
			{	
				EVB5.sStrF0.vendorSN[i] = EVBSN[i];
			}
	
 			reg_add = (int)(&EVB5.sStrF0.vendorPN)-(int)(&EVB5.sStrF0.FirstByte); 
			error = I2C_BYTEs_WRITE_DLL (USBHandle, EVB_addr, reg_add, 16, EVB5.pStrF0, 0.1); 
												
			//'W'
			EVB5.sStrF0.FirstByte = 'W';
			reg_add =(int)(&EVB5.sStrF0.FirstByte) - (int)(&EVB5.sStrF0.FirstByte); 
			error = I2C_BYTEs_WRITE_DLL (USBHandle, EVB_addr, reg_add, 1, EVB5.pStrF0, 0.2);
			
			SetWaitCursor (0);

			break;
	}
	return 0;
}

int CVICALLBACK On_Set_USBSN (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)

{
	int error;
	char USBSN[30];

	switch (event)
		{
		case EVENT_COMMIT:
	
			SetWaitCursor (1);

 			//search USB Host
			GetCtrlVal (panel, PANEL_NUM_HOSTHandle, &USBHandle);
			error = I2C_HOST_INITIALIZATION_DLL (USBHandle); 
			if (error<0) return -1; //USB host not found!
			
			memset (USBSN, 0, sizeof(USBSN));
			GetCtrlVal (panel, PANEL_STR_USBSN, USBSN);
			error = SetF320SerialNumber(USBHandle-0x800, USBSN);
			if (error<0) return -1; //USB host not found!
			
			SetWaitCursor (0);

			break;
	}
	return 0;
}

int CVICALLBACK On_SEVB27_02_Channel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error;
	BYTE channel;
				
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, SEVB27_02_NUM_CHAN, &channel);  
			
			error  = SEL_CLK_F320_DLL(USBHandle-0x800, channel);
			if(error) return -1;
				
			break;
	}
	return 0;
}

int CVICALLBACK OnI2CConfig (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int I2C_RATE;
int error;
int I2C_STOP; 

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panelHandle, PANEL_NUM_HOSTHandle, &USBHandle);
			
			GetCtrlVal (panel, I2C_NUM_RATE, &I2C_RATE);
			GetCtrlVal (panel, I2C_CHE_STOP, &I2C_STOP); //在主I2C读时序的ReSTART之前加一个STOP?

			I2C_RATE = 256-24*1E3/3./I2C_RATE;

			//可以直接调用函数I2C_HOST_RESET_F320_DLL
	//		I2C_HOST_RESET_F320_DLL (USBHandle-0x800, I2C_RATE, I2C_STOP);

			//也可以通过间接函数调用
			SetF320I2CRate_DLL(I2C_RATE);
			SetF320I2CSTOPbeforeReSTART_DLL(I2C_STOP);
			error = I2C_HOST_INITIALIZATION_DLL(USBHandle);
			
			break;
	}
	return 0;
}

int CVICALLBACK OnReadUSB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned char f320ver;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panelHandle, PANEL_NUM_HOSTHandle, &USBHandle); 
			
			GET_Version_F320_DLL(USBHandle-0x800, &f320ver);
			
			SetCtrlVal (panel, USB_NUM_F320VER, f320ver); 
			
			break;
	}
	return 0;
}

int CVICALLBACK On_SEVB27_02_Channel_Get (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int error;
	BYTE channel;

	switch (event)
	{
		case EVENT_COMMIT:

			error  = CLK_Get_F320_DLL(USBHandle-0x800, &channel);
			if(error) return -1;
			
			SetCtrlVal (panel, SEVB27_02_NUM_CHAN, channel); 
			
			break;
	}
	return 0;
}
