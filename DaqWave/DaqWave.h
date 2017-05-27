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
#define  PANEL_GRAPH                      2
#define  PANEL_CHANNEL_STRING             3
#define  PANEL_SCANRATE                   4
#define  PANEL_ACQUIRE                    5       /* callback function: AcquireCallback */
#define  PANEL_CLEAR                      6       /* callback function: ClearCallback */
#define  PANEL_QUIT                       7       /* callback function: QuitCallback */
#define  PANEL_COMMANDBUTTON              8       /* callback function: CallBackTest */
#define  PANEL_COMMANDBUTTON_2            9       /* callback function: CallBackTest1 */
#define  PANEL_TIMER                      10      /* callback function: AITimerCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcquireCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AITimerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallBackTest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallBackTest1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClearCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
