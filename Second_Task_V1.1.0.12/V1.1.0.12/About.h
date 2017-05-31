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

#define  PAN_ABOUT                        1       /* callback function: On_About_EXIT */
#define  PAN_ABOUT_CANVAS                 2
#define  PAN_ABOUT_SPLITTER_2             3
#define  PAN_ABOUT_SPLITTER               4
#define  PAN_ABOUT_TEXTMSG_2              5
#define  PAN_ABOUT_TEXTMSG                6


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK On_About_EXIT(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif

