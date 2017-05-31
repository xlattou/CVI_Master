/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2013. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: panelCB */
#define  PANEL_BUT_I2C_CHECK              2       /* callback function: on_I2C_CHECK */
#define  PANEL_DECORATION_33              3
#define  PANEL_DECORATION_28              4
#define  PANEL_TEXTMSG_72                 5
#define  PANEL_LISTBOX_I2C_HOST           6       /* callback function: on_LISTBOX_I2C_HOST */
#define  PANEL_LISTBOX_I2C_ADD            7
#define  PANEL_TEXTMSG_73                 8
#define  PANEL_BUT_SEARCH_I2C_ADD         9       /* callback function: on_SEARCH_I2C_ADD */
#define  PANEL_NUM_HOSTHandle             10
#define  PANEL_NUM_DLLVersion             11
#define  PANEL_STR_USBSN                  12
#define  PANEL_STR_EVBSN                  13
#define  PANEL_BUT_SET_USBSN              14      /* callback function: On_Set_USBSN */
#define  PANEL_BUT_SET_EVBSN              15      /* callback function: ON_Set_EVBSN */
#define  PANEL_BUT_Get_USBSN              16      /* callback function: on_Get_USBSN */
#define  PANEL_BUT_Get_EVBSN              17      /* callback function: on_Get_EVBSN */
#define  PANEL_STR_DLLVER                 18
#define  PANEL_USB                        19

     /* tab page panel controls */
#define  I2C_NUM_RATE                     2       /* callback function: OnI2CConfig */
#define  I2C_CHE_STOP                     3       /* callback function: OnI2CConfig */

     /* tab page panel controls */
#define  SEVB27_02_NUM_CHAN               2
#define  SEVB27_02_BUT_SEL                3       /* callback function: On_SEVB27_02_Channel */
#define  SEVB27_02_BUT_GET                4       /* callback function: On_SEVB27_02_Channel_Get */

     /* tab page panel controls */
#define  USB_NUM_F320VER                  2
#define  USB_BUT_READ                     3       /* callback function: OnReadUSB */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK on_Get_EVBSN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_Get_USBSN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_I2C_CHECK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_LISTBOX_I2C_HOST(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_SEARCH_I2C_ADD(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ON_Set_EVBSN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Set_USBSN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_SEVB27_02_Channel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_SEVB27_02_Channel_Get(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnI2CConfig(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnReadUSB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
