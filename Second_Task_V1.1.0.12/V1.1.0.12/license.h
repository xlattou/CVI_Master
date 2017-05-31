/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2011. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PAN_PW                           1
#define  PAN_PW_STR_PW_NEW2               2       /* callback function: On_PW_Entry2 */
#define  PAN_PW_STR_PW_NEW                3       /* callback function: On_PW_Entry */
#define  PAN_PW_STR_ID                    4       /* callback function: On_PW_Entry */
#define  PAN_PW_STR_PW_OLD                5       /* callback function: On_PW_Entry */
#define  PAN_PW_BUT_CLOSE                 6       /* callback function: On_PW_Close */
#define  PAN_PW_BUT_OK                    7       /* callback function: On_PW_OK */

#define  Panel_log                        2
#define  Panel_log_username               2       /* callback function: On_Login_Username */
#define  Panel_log_password               3       /* callback function: On_Login_Password */
#define  Panel_log_BUT_PW_EDIT            4       /* callback function: On_Login_PW */
#define  Panel_log_BUT_OK                 5       /* callback function: On_Login_OK */
#define  Panel_log_BUT_QUIT               6       /* callback function: On_Login_Quit */
#define  Panel_log_DECORATION_2           7


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK On_Login_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Login_Password(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Login_PW(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Login_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Login_Username(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_PW_Close(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_PW_Entry(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_PW_Entry2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_PW_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
