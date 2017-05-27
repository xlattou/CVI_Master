/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2016. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PAN_CALI_L                       1
#define  PAN_CALI_L_BUT_APP               2       /* callback function: On_Cali_Limit_APPLICATION */
#define  PAN_CALI_L_BUT_OK                3       /* callback function: On_Cali_Limit_OK */
#define  PAN_CALI_L_BUT_QUIT              4       /* callback function: On_Cali_Limit_Quit */
#define  PAN_CALI_L_TABLE                 5
#define  PAN_CALI_L_RAD_CHAN              6       /* callback function: On_CaliLimit_Chan */

#define  PAN_CHE_T                        2
#define  PAN_CHE_T_BUT_QUIT               2       /* callback function: On_CheckT_Quit */
#define  PAN_CHE_T_BUT_OK                 3       /* callback function: On_CheckT_OK */
#define  PAN_CHE_T_TABLE                  4       /* callback function: On_Check_Table */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK On_Cali_Limit_APPLICATION(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_Limit_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_Limit_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliLimit_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Check_Table(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CheckT_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CheckT_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
