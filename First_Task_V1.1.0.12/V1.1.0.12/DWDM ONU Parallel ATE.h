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

#define  PAN_ADVA                         1
#define  PAN_ADVA_BUT_QUIT                2       /* callback function: On_Advance_Quit */
#define  PAN_ADVA_BUT_APP                 3       /* callback function: On_Advance_App */
#define  PAN_ADVA_IS_SCAN_MODULESN        4
#define  PAN_ADVA_IS_QANOCHECKBOM         5
#define  PAN_ADVA_IS_NPI                  6
#define  PAN_ADVA_IS_QAUPDATESN           7
#define  PAN_ADVA_IS_ONUTOOLT             8
#define  PAN_ADVA_NUM_WLMeterOffset       9
#define  PAN_ADVA_NUMERIC_Agin_WL         10
#define  PAN_ADVA_NUMERIC_Agin_AOP        11

#define  PAN_CALOLT                       2
#define  PAN_CALOLT_NUM_SENS_MIN          2
#define  PAN_CALOLT_NUM_SENS              3
#define  PAN_CALOLT_NUM_P_RESERVES        4
#define  PAN_CALOLT_NUM_SENS_MAX          5
#define  PAN_CALOLT_RAD_CHAN              6       /* callback function: On_Cali_OLTSENS_Chan */
#define  PAN_CALOLT_NUM_P_IN              7
#define  PAN_CALOLT_BUT_QUIT              8       /* callback function: On_Cali_OLTSENS_Quit */
#define  PAN_CALOLT_BUT_OK                9       /* callback function: On_Cali_OLTSENS_OK */
#define  PAN_CALOLT_BUT_TEST              10      /* callback function: On_Cali_OLTSENS_TEST */
#define  PAN_CALOLT_SLIDE_TIME            11
#define  PAN_CALOLT_BUT_BURST             12      /* callback function: On_Burst_CALOLT */

#define  PAN_CALR                         3
#define  PAN_CALR_BUT_SET                 2       /* callback function: OnCaliLimitSet */
#define  PAN_CALR_TABLE_R                 3
#define  PAN_CALR_BUT_OK                  4       /* callback function: On_CaliRx_OK */
#define  PAN_CALR_BUT_QUIT                5       /* callback function: On_CaliRx_Quit */
#define  PAN_CALR_RAD_CHAN                6       /* callback function: On_CaliRx_Chan */
#define  PAN_CALR_BUT_BURST_CALR          7       /* callback function: On_Burst_CALR */

#define  PAN_CALT                         4
#define  PAN_CALT_BUT_SET                 2       /* callback function: OnCaliLimitSet */
#define  PAN_CALT_BUT_OK                  3       /* callback function: On_CaliTx_OK */
#define  PAN_CALT_BUT_QUIT                4       /* callback function: On_CaliTx_Quit */
#define  PAN_CALT_RAD_CHAN                5       /* callback function: On_CaliTx_Chan */
#define  PAN_CALT_TABLE_T                 6       /* callback function: On_CaliT_Table */
#define  PAN_CALT_BUT_BURST               7       /* callback function: On_Burst_CALT */

#define  PAN_CALT_M                       5
#define  PAN_CALT_M_BUT_SET_Min           2       /* callback function: OnCaliLimitSet_Min */
#define  PAN_CALT_M_BUT_OK_Min            3       /* callback function: On_CaliTx_OK_Min */
#define  PAN_CALT_M_BUT_QUIT_Min          4       /* callback function: On_CaliTx_Quit_Min */
#define  PAN_CALT_M_RAD_CHAN_Min          5       /* callback function: On_CaliTx_Chan_Min */
#define  PAN_CALT_M_TABLE_T_Min           6       /* callback function: On_CaliT_Table_Min */
#define  PAN_CALT_M_BUT_BURST_Min         7       /* callback function: On_Burst_CALT_Min */

#define  PAN_CONF                         6
#define  PAN_CONF_RING_PN                 2       /* callback function: On_Config_PN */
#define  PAN_CONF_BUT_OK                  3       /* callback function: On_Config_OK */
#define  PAN_CONF_RING_EED                4
#define  PAN_CONF_RING_BOM                5       /* callback function: On_Config_BOM */
#define  PAN_CONF_RAD_HIGH                6       /* callback function: On_Config_Temper */
#define  PAN_CONF_RAD_ROOM                7       /* callback function: On_Config_Temper */
#define  PAN_CONF_RAD_LOW                 8       /* callback function: On_Config_Temper */
#define  PAN_CONF_RAD_Upstream_Test       9       /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_OQA                 10      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_QA                  11      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TEST_RX             12      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TEST_Reliability    13      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TUNING_Reliabilit   14      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TEST                15      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TUNING              16      /* callback function: On_Config_Item */
#define  PAN_CONF_DECORATION              17
#define  PAN_CONF_TEXTMSG                 18
#define  PAN_CONF_DECORATION_2            19
#define  PAN_CONF_TEXTMSG_2               20
#define  PAN_CONF_CHE_BOX_R_SDHYS         21
#define  PAN_CONF_CHE_BOX_T_MASK          22
#define  PAN_CONF_CHE_BOX_T_EYE           23
#define  PAN_CONF_CHE_BOX_T_DIS           24
#define  PAN_CONF_CHE_BOX_T_ER            25
#define  PAN_CONF_CHE_BOX_T_AOP           26
#define  PAN_CONF_CHE_Wavelength          27
#define  PAN_CONF_CHE_TXSD                28
#define  PAN_CONF_CHE_BOX_EE_PROTECT      29
#define  PAN_CONF_CHE_BOX_CALI_TEST       30
#define  PAN_CONF_CHE_BOX_CALI            31
#define  PAN_CONF_CHE_BOX_R_OVER          32
#define  PAN_CONF_CHE_SD_DETail           33
#define  PAN_CONF_CHE_SENS_DETail         34
#define  PAN_CONF_CHE_BOX_R_SENS          35
#define  PAN_CONF_CHE_BOX_T_SPECTRUM      36
#define  PAN_CONF_RADIOBUTTON_OSATEST     37      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_Agin_Back           38      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_Agin_Front          39      /* callback function: On_Config_Item */
#define  PAN_CONF_RAD_TUN_RX              40      /* callback function: On_Config_Item */
#define  PAN_CONF_DECORATION_7            41
#define  PAN_CONF_DECORATION_6            42
#define  PAN_CONF_DECORATION_5            43
#define  PAN_CONF_DECORATION_4            44

#define  PAN_FIBER                        7
#define  PAN_FIBER_BUT_QUIT               2       /* callback function: OnFiberQuit */
#define  PAN_FIBER_BUT_OK                 3       /* callback function: OnFiberOK */
#define  PAN_FIBER_TABLE                  4       /* callback function: On_Check_Table */

#define  PAN_INIT                         8
#define  PAN_INIT_TEXTMSG                 2

#define  PAN_INST                         9
#define  PAN_INST_TREE                    2       /* callback function: On_Inst_Tree */
#define  PAN_INST_DECORATION              3
#define  PAN_INST_STR_FIBER               4
#define  PAN_INST_STR_SPECTRUM            5
#define  PAN_INST_STR_DCA                 6
#define  PAN_INST_RING_OLT_ATT_TYPE       7
#define  PAN_INST_RING_ATT_TYPE           8
#define  PAN_INST_RING_CLOCK_TYPE         9
#define  PAN_INST_RING_SW_TYPE_2          10
#define  PAN_INST_RING_SW_TYPE            11
#define  PAN_INST_RING_DCA_TYPE           12
#define  PAN_INST_RING_SEVB_TYPE          13
#define  PAN_INST_RING_SPECTRUM_TYPE      14
#define  PAN_INST_RING_OLT_BERT_TYPE      15
#define  PAN_INST_RING_BERT_TYPE          16
#define  PAN_INST_RING_PM_TYPE            17
#define  PAN_INST_RING_OLT_GVPM           18
#define  PAN_INST_RING_CLOCK              19
#define  PAN_INST_RING_OLT_BERT           20
#define  PAN_INST_RING_GVPM               21
#define  PAN_INST_RING_BERT               22
#define  PAN_INST_RING_SEVB               23
#define  PAN_INST_STR_SN_SPECTRUM         24
#define  PAN_INST_STR_SN_DCA              25
#define  PAN_INST_STR_SN_PM               26
#define  PAN_INST_BUT_OLT_BERT            27      /* callback function: On_Search_OLT_BERT */
#define  PAN_INST_BUT_CLOCK               28      /* callback function: On_Search_Clock */
#define  PAN_INST_BUT_OLT_ATT             29      /* callback function: On_Search_OLT_ATT */
#define  PAN_INST_BUT_BERT                30      /* callback function: On_Search_BERT */
#define  PAN_INST_BUT_SEVB                31      /* callback function: On_Search_SEVB */
#define  PAN_INST_BUT_FSW_2               32      /* callback function: On_Search_FSW */
#define  PAN_INST_BUT_ATT                 33      /* callback function: On_Search_ATT */
#define  PAN_INST_BUT_FSW                 34      /* callback function: On_Search_FSW */
#define  PAN_INST_BUT_DCA                 35      /* callback function: On_Search_DCA */
#define  PAN_INST_BUT_SPECTRUM            36
#define  PAN_INST_NUM_SW_CHAN_2           37
#define  PAN_INST_NUM_SW_2                38
#define  PAN_INST_NUM_CHANNEL             39
#define  PAN_INST_NUM_CLOCK_CHAN          40
#define  PAN_INST_NUM_SW_CHAN             41
#define  PAN_INST_NUM_SW                  42
#define  PAN_INST_NUM_ATT                 43
#define  PAN_INST_NUM_PM                  44
#define  PAN_INST_BUT_ADD                 45      /* callback function: On_Inst_AddTree */
#define  PAN_INST_RING_FSW_2              46
#define  PAN_INST_BUT_CHECK               47      /* callback function: On_Inst_Check */
#define  PAN_INST_BUT_OK                  48      /* callback function: On_Inst_OK */
#define  PAN_INST_BIN_CHAN_EN             49
#define  PAN_INST_BUT_SAVE                50      /* callback function: On_Inst_Save */
#define  PAN_INST_RING_FSW                51
#define  PAN_INST_TEXTMSG                 52

#define  PAN_MAIN                         10      /* callback function: panelCB */
#define  PAN_MAIN_CANVAS                  2
#define  PAN_MAIN_TREE                    3
#define  PAN_MAIN_BUT_RUN                 4       /* callback function: On_Run */
#define  PAN_MAIN_BUT_STOP                5       /* callback function: On_Stop */
#define  PAN_MAIN_LED                     6
#define  PAN_MAIN_TEXTMSG_INFO            7
#define  PAN_MAIN_TEXTBOX_CHAN7           8
#define  PAN_MAIN_TEXTBOX_CHAN6           9
#define  PAN_MAIN_TEXTBOX_CHAN5           10
#define  PAN_MAIN_TEXTBOX_CHAN4           11
#define  PAN_MAIN_TEXTBOX_CHAN3           12
#define  PAN_MAIN_TEXTBOX_CHAN2           13
#define  PAN_MAIN_TEXTBOX_CHAN1           14
#define  PAN_MAIN_TEXTBOX_CHAN0           15
#define  PAN_MAIN_STR_PN                  16
#define  PAN_MAIN_STR_OPERATOR            17
#define  PAN_MAIN_STR_BOM                 18
#define  PAN_MAIN_STR_BATCH               19
#define  PAN_MAIN_STR_SN                  20
#define  PAN_MAIN_LED_CHAN1               21
#define  PAN_MAIN_BIN_CHAN7               22      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN5               23      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN4               24      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN1               25      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN2               26      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN0               27      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN6               28      /* callback function: On_P */
#define  PAN_MAIN_BIN_CHAN3               29      /* callback function: On_P */
#define  PAN_MAIN_LED_CHAN4               30
#define  PAN_MAIN_LED_CHAN7               31
#define  PAN_MAIN_LED_CHAN6               32
#define  PAN_MAIN_LED_CHAN5               33
#define  PAN_MAIN_LED_CHAN3               34
#define  PAN_MAIN_LED_CHAN2               35
#define  PAN_MAIN_LED_CHAN0               36
#define  PAN_MAIN_But_Star_Ch0            37      /* callback function: Start_Channel */
#define  PAN_MAIN_But_Star_Ch1            38      /* callback function: Start_Channel */
#define  PAN_MAIN_But_Star_Ch2            39      /* callback function: Start_Channel */
#define  PAN_MAIN_But_Star_Ch3            40      /* callback function: Start_Channel */

#define  PAN_ORDER                        11
#define  PAN_ORDER_COMBO_ORDER            2
#define  PAN_ORDER_BUT_ORDER              3       /* callback function: On_Order */
#define  PAN_ORDER_TEXTMSG                4

#define  PAN_OSA                          12
#define  PAN_OSA_COMMANDBUTTON            2       /* callback function: CallBackHideOSAManual */
#define  PAN_OSA_STRING_WAVE1             3
#define  PAN_OSA_STRING_TMPE1             4
#define  PAN_OSA_STRING_DAC1              5
#define  PAN_OSA_STRING_WAVE0             6
#define  PAN_OSA_COMMANDBUTTON_SET1       7       /* callback function: Callback_DAC1 */
#define  PAN_OSA_STRING_TMPE0             8
#define  PAN_OSA_STRING_DAC0              9
#define  PAN_OSA_COMMANDBUTTON_SET0       10      /* callback function: Callback_DAC0 */
#define  PAN_OSA_COMMANDBUTTON_2          11      /* callback function: Callback_DAC_Save */
#define  PAN_OSA_DECORATION               12

#define  PAN_POPU                         13
#define  PAN_POPU_BUT_CANCEL              2       /* callback function: On_Pup_Cancel */
#define  PAN_POPU_BUT_USER                3       /* callback function: On_Pup_User */
#define  PAN_POPU_BUT_EXIT                4       /* callback function: On_Pup_Exit */
#define  PAN_POPU_TEXTMSG                 5

#define  PAN_SMC                          14
#define  PAN_SMC_NUM_SENSTIME             2
#define  PAN_SMC_RAD_CHAN                 3       /* callback function: On_Cali_SMC_Chan */
#define  PAN_SMC_NUM_SENSVALUE            4
#define  PAN_SMC_BUT_QUIT                 5       /* callback function: On_Cali_SMC_Quit */
#define  PAN_SMC_BUT_TEST                 6       /* callback function: On_Cali_SMC_Test */
#define  PAN_SMC_SLIDE_TIME               7
#define  PAN_SMC_LED                      8
#define  PAN_SMC_BUT_BURST                9       /* callback function: On_Burst_SMC */

#define  PAN_SN                           15
#define  PAN_SN_STRING_MODULESN           2
#define  PAN_SN_CMD_OK                    3
#define  PAN_SN_CHECKBOX                  4

#define  PN_CAL_OLT                       16
#define  PN_CAL_OLT_BUT_SET               2       /* callback function: OnCaliLimitSet */
#define  PN_CAL_OLT_TABLE_R               3
#define  PN_CAL_OLT_BUT_OK                4       /* callback function: On_CaliOLT_OK */
#define  PN_CAL_OLT_BUT_QUIT              5       /* callback function: On_CaliOLT_Quit */
#define  PN_CAL_OLT_RAD_CHAN              6       /* callback function: On_CaliOLT_Chan */
#define  PN_CAL_OLT_BUT_BURST             7       /* callback function: On_Burst_CAL_OLT */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_SGD                 3       /* callback function: On_SGD */
#define  MENUBAR_FILE_USER                4       /* callback function: On_User */
#define  MENUBAR_FILE_EXIT                5       /* callback function: On_Quit */
#define  MENUBAR_SET                      6
#define  MENUBAR_SET_INIT                 7       /* callback function: On_Init */
#define  MENUBAR_SET_CONFIG               8       /* callback function: On_Config */
#define  MENUBAR_SET_INSTRUMENT           9       /* callback function: On_Inst */
#define  MENUBAR_SET_FUN_ADVANCE          10      /* callback function: On_Advance */
#define  MENUBAR_SET_OSA_MANUAL           11      /* callback function: On_OSA_Manual */
#define  MENUBAR_CALI                     12
#define  MENUBAR_CALI_TX_CHECK            13      /* callback function: On_CheckT */
#define  MENUBAR_CALI_TXRX_CALT           14      /* callback function: On_CalibrateT */
#define  MENUBAR_CALI_TXRX_CALT_Min       15      /* callback function: On_CalibrateT_Min */
#define  MENUBAR_CALI_CALT_RX             16      /* callback function: On_CalibrateR */
#define  MENUBAR_CALI_OLT_RXPOWERC        17      /* callback function: On_Calibrate_R_OLT */
#define  MENUBAR_CALI_SM_CHECK            18      /* callback function: On_SMCheck */
#define  MENUBAR_CALI_CALT_OLT            19      /* callback function: On_CalibrateOLT */
#define  MENUBAR_FIBER                    20
#define  MENUBAR_FIBER_FIBERSN            21      /* callback function: OnFiber */
#define  MENUBAR_HELP                     22
#define  MENUBAR_HELP_HIS                 23      /* callback function: On_His */
#define  MENUBAR_HELP_ABOUT               24      /* callback function: On_About */


     /* Callback Prototypes: */ 

int  CVICALLBACK Callback_DAC0(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Callback_DAC1(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Callback_DAC_Save(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallBackHideOSAManual(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_About(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_Advance(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_Advance_App(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Advance_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_CAL_OLT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_CALOLT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_CALR(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_CALT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_CALT_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Burst_SMC(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_OLTSENS_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_OLTSENS_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_OLTSENS_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_OLTSENS_TEST(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_SMC_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_SMC_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Cali_SMC_Test(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_Calibrate_R_OLT(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_CalibrateOLT(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_CalibrateR(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_CalibrateT(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_CalibrateT_Min(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_CaliOLT_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliOLT_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliOLT_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliRx_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliRx_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliRx_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliT_Table(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliT_Table_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_Chan(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_Chan_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_OK_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_CaliTx_Quit_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Check_Table(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_CheckT(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_Config(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_Config_BOM(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Config_Item(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Config_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Config_PN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Config_Temper(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_His(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_Init(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_Inst(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_Inst_AddTree(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Inst_Check(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Inst_OK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Inst_Save(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Inst_Tree(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Order(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_OSA_Manual(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_P(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Pup_Cancel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Pup_Exit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Pup_User(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_Quit(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_Run(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_ATT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_BERT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_Clock(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_DCA(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_FSW(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_OLT_ATT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_OLT_BERT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK On_Search_SEVB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_SGD(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK On_SMCheck(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK On_Stop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK On_User(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnCaliLimitSet(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCaliLimitSet_Min(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK OnFiber(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnFiberOK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFiberQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Start_Channel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
