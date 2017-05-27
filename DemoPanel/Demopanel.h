/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2017. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1
#define  PANEL_COMMANDBUTTON_3            2       /* callback function: Callback_Quit */
#define  PANEL_COMMANDBUTTON_2            3       /* callback function: Callback_Clear */
#define  PANEL_COMMANDBUTTON              4       /* callback function: Callback_Acquire */
#define  PANEL_GRAPH                      5


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK Callback_Acquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Callback_Clear(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Callback_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
