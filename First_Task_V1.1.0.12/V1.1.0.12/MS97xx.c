/*****************************************************************************
 *  MS97xx Optical Spectrum Analyzer Instrument Driver                               
 *  LabWindows/CVI 5.0 Instrument Driver                                     
 *  Original Release: 11/03/1999                                  
 *  By: Ian Dees, Anritsu                              
 *      PH. 800-ANRITSU   Fax 972-644-3416                              
 *                                                                           
 *  Modification History:                                                    
 *                                                                           
 *       06/15/2000 - Instrument Driver Created.                  
 *                                                                           
 *****************************************************************************/

#include <visa.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include <string.h>
#include <stdio.h>  
#include "MS97xx.h"
  

/*****************************************************************************
 *--------------------- Hidden Attribute Declarations -----------------------*
 *****************************************************************************/

#define MS97XX_ATTR_TRACE_1_ATTR_LOCKED     (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 3L)
#define MS97XX_ATTR_TRACE_1_ATTR_DIRTY      (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 4L)
#define MS97XX_ATTR_TRACE_2_ATTR_LOCKED     (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 5L)
#define MS97XX_ATTR_TRACE_2_ATTR_DIRTY      (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 6L)
#define MS97XX_ATTR_TRACE_3_ATTR_LOCKED     (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 7L)
#define MS97XX_ATTR_TRACE_3_ATTR_DIRTY      (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 8L)
#define MS97XX_ATTR_RMS_DIRTY               (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 9L)
#define MS97XX_ATTR_RMS_LOCKED              (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 10L)
#define MS97XX_ATTR_DISKETTE_ATTR_DIRTY     (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 11L)
#define MS97XX_ATTR_DISKETTE_ATTR_LOCKED    (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 12L)
#define MS97XX_ATTR_DFB_ATTR_LOCKED         (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 13L)
#define MS97XX_ATTR_DFB_ATTR_DIRTY          (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 14L)
#define MS97XX_ATTR_LED_ATTR_LOCKED         (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 15L)
#define MS97XX_ATTR_LED_ATTR_DIRTY          (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 16L)
#define MS97XX_ATTR_AMP_ATTR_LOCKED         (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 17L)
#define MS97XX_ATTR_AMP_ATTR_DIRTY          (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 18L)
#define MS97XX_ATTR_SNR_ATTR_LOCKED         (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 19L)
#define MS97XX_ATTR_SNR_ATTR_DIRTY          (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 20L)
#define MS97XX_ATTR_TABLE_ATTR_LOCKED       (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 21L)
#define MS97XX_ATTR_TABLE_ATTR_DIRTY        (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 22L)
#define MS97XX_ATTR_NF_ATTR_LOCKED          (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 23L)
#define MS97XX_ATTR_NF_ATTR_DIRTY           (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 24L)
#define MS97XX_ATTR_ZONE_MARKER_WL_LOCKED   (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 25L)
#define MS97XX_ATTR_ZONE_MARKER_WL_DIRTY    (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 26L)
#define MS97XX_ATTR_READ_BUF_SIZE           (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 27L)
#define MS97XX_ATTR_READ_BUF                (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 28L)
#define MS97XX_ATTR_TRACE_MARKER_VISIBLE    (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 29L)
#define MS97XX_ATTR_ZONE_MARKER_VISIBLE     (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 30L)
#define MS97XX_ATTR_INST_OUT_OF_RANGE       (IVI_SPECIFIC_PRIVATE_ATTR_BASE + 31L)

/*****************************************************************************
 *--------------------- Hidden Attribute Values -----------------------------*
 *****************************************************************************/


/*****************************************************************************
 *---------------------------- Useful Macros --------------------------------*
 *****************************************************************************/

    /*- I/O buffer size -----------------------------------------------------*/
#define BUFFER_SIZE                             512L        

    /*- 488.2 Event Status Register (ESR) Bits ------------------------------*/
#define IEEE_488_2_QUERY_ERROR_BIT              0x04
#define IEEE_488_2_DEVICE_DEPENDENT_ERROR_BIT   0x08
#define IEEE_488_2_EXECUTION_ERROR_BIT          0x10
#define IEEE_488_2_COMMAND_ERROR_BIT            0x20

    /*- List of channels passed to the Ivi_BuildChannelTable function -------*/ 
#define CHANNEL_LIST                            "1"

	/*- Bit mask for valid measurement modes --------------------------------*/
#define MS97XX_MODE_MASK_SPC					0x0001
#define MS97XX_MODE_MASK_ILO					0x0002
#define MS97XX_MODE_MASK_ISO					0x0004
#define MS97XX_MODE_MASK_DIR					0x0008
#define MS97XX_MODE_MASK_RLO					0x0010
#define MS97XX_MODE_MASK_PMD					0x0020
#define MS97XX_MODE_MASK_NF						0x0040
#define MS97XX_MODE_MASK_WDM					0x0080
#define MS97XX_MODE_MASK_LTS					0x0100
#define MS97XX_MODE_MASK_DFB					0x0200
#define MS97XX_MODE_MASK_FP						0x0400
#define MS97XX_MODE_MASK_LED					0x0800
#define MS97XX_MODE_MASK_AMP					0x1000

	/*- Bit mask for valid WDM modes ----------------------------------------*/
#define MS97XX_WDM_MASK_MPK						0x0001
#define MS97XX_WDM_MASK_SNR						0x0002
#define MS97XX_WDM_MASK_REL						0x0004
#define MS97XX_WDM_MASK_TBL						0x0008

	/*- Bit mask for valid spectrum display modes ---------------------------*/
#define MS97XX_DISPLAY_MASK_NRM						0x0001
#define MS97XX_DISPLAY_MASK_OVL						0x0002
#define MS97XX_DISPLAY_MASK_DBM						0x0004
#define MS97XX_DISPLAY_MASK_DB						0x0008

static IviRangeTableEntry attrDipSearchRangeTableEntries[] =
	{
		{MS97XX_VAL_DIP_SEARCH_DIP, 0, 0, "DIP", 0},
		{MS97XX_VAL_DIP_SEARCH_NEXT, 0, 0, "NEXT", 0},
		{MS97XX_VAL_DIP_SEARCH_LAST, 0, 0, "LAST", 0},
		{MS97XX_VAL_DIP_SEARCH_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_DIP_SEARCH_RIGHT, 0, 0, "RIGHT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDipSearchRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDipSearchRangeTableEntries,
	};

	/*- String to be sent at the end of every transmission ------------------*/
static const ViChar terminator[]="";

static IviRangeTableEntry attrDisplayModeRangeTable_10Entries[] =
	{
		{MS97XX_VAL_DISPLAY_MODE_NORMAL, 0, 0, "NRM", 0},
		{MS97XX_VAL_DISPLAY_MODE_OVERLAP, 0, 0, "OVL", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDisplayModeRangeTable_10 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDisplayModeRangeTable_10Entries,
	};

static IviRangeTableEntry attrDisplayModeRangeTable_20Entries[] =
	{
		{MS97XX_VAL_DISPLAY_MODE_NORMAL, 0, 0, "NRM", 0},
		{MS97XX_VAL_DISPLAY_MODE_OVERLAP, 0, 0, "OVL", 0},
		{MS97XX_VAL_DISPLAY_MODE_DBM_DBM, 0, 0, "DBM&DBM", 0},
		{MS97XX_VAL_DISPLAY_MODE_DBM_DB, 0, 0, "DBM&DB", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDisplayModeRangeTable_20 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDisplayModeRangeTable_20Entries,
	};

static IviRangeTableEntry attrLongTermSpacingDisplayRangeTableEntries[] =
	{
		{MS97XX_VAL_LONG_TERM_SPACING_DISPLAY_WAVELENGTH, 0, 0, "WAVE", 0},
		{MS97XX_VAL_LONG_TERM_SPACING_DISPLAY_FREQUENCY, 0, 0, "FRQ", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLongTermSpacingDisplayRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLongTermSpacingDisplayRangeTableEntries,
	};

static IviRangeTableEntry attrPeakSearchRangeTableEntries[] =
	{
		{MS97XX_VAL_PEAK_SEARCH_PEAK, 0, 0, "PEAK", 0},
		{MS97XX_VAL_PEAK_SEARCH_NEXT, 0, 0, "NEXT", 0},
		{MS97XX_VAL_PEAK_SEARCH_LAST, 0, 0, "LAST", 0},
		{MS97XX_VAL_PEAK_SEARCH_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_PEAK_SEARCH_RIGHT, 0, 0, "RIGHT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrPeakSearchRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrPeakSearchRangeTableEntries,
	};

static IviRangeTableEntry attrLongTermSampleTimeRangeTableEntries[] =
	{
		{0, 99, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLongTermSampleTimeRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLongTermSampleTimeRangeTableEntries,
	};

static IviRangeTableEntry attrLongTermTestStateRangeTableEntries[] =
	{
		{MS97XX_VAL_ATTR_LONG_TERM_SUCCESS, 0, 0, "", 0},
		{MS97XX_VAL_ATTR_LONG_TERM_IN_PROGRESS, 0, 0, "", 0},
		{MS97XX_VAL_ATTR_LONG_TERM_FAIL, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLongTermTestStateRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLongTermTestStateRangeTableEntries,
	};

static IviRangeTableEntry attrModeConnectCoeffRangeTableEntries[] =
	{
		{0.1, 1.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrModeConnectCoeffRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrModeConnectCoeffRangeTableEntries,
	};

static IviRangeTableEntry attrPmdPeakCountRangeTableEntries[] =
	{
		{1, 99, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrPmdPeakCountRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrPmdPeakCountRangeTableEntries,
	};

static IviRangeTableEntry attrPowerMonitorWavelengthRangeTableEntries[] =
	{
		{0.0, 632.8, 632.8, "", 0},
		{632.8, 850.0, 850.0, "", 0},
		{850.0, 1300.0, 1300.0, "", 0},
		{1300.0, 1550.0, 1550.0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrPowerMonitorWavelengthRangeTable =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrPowerMonitorWavelengthRangeTableEntries,
	};

static IviRangeTableEntry attrTlsCalStatusRangeTableEntries[] =
	{
		{MS97XX_VAL_TLS_CAL_STATUS_END, 0, 0, "", 0},
		{MS97XX_VAL_TLS_CAL_STATUS_CALIBRATING, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrTlsCalStatusRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrTlsCalStatusRangeTableEntries,
	};

static IviRangeTableEntry attrMarkerUnitsRangeTableEntries[] =
	{
		{MS97XX_VAL_MARKER_UNITS_WAVELENGTH, 0, 0, "WL", 0},
		{MS97XX_VAL_MARKER_UNITS_FREQUENCY, 0, 0, "FREQ", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMarkerUnitsRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMarkerUnitsRangeTableEntries,
	};

static IviRangeTableEntry attrMarkerLevelRangeTable_dBmEntries[] =
	{
		{-190.0, 50.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMarkerLevelRangeTable_dBm =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMarkerLevelRangeTable_dBmEntries,
	};

static IviRangeTableEntry attrMarkerLevelRangeTable_dBEntries[] =
	{
		{-160.0, 160.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMarkerLevelRangeTable_dB =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMarkerLevelRangeTable_dBEntries,
	};

static IviRangeTableEntry attrMarkerLevelRangeTable_WEntries[] =
	{
		{0.0, 1.2, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMarkerLevelRangeTable_W =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMarkerLevelRangeTable_WEntries,
	};

static IviRangeTableEntry attrMarkerLevelRangeTable_PercentEntries[] =
	{
		{0.0, 240.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMarkerLevelRangeTable_Percent =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMarkerLevelRangeTable_PercentEntries,
	};

static IviRangeTableEntry attrEventStatusEnableRangeTableEntries[] =
	{
		{0, 255, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrEventStatusEnableRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrEventStatusEnableRangeTableEntries,
	};

static IviRangeTableEntry attrTerminatorRangeTableEntries[] =
	{
		{MS97XX_VAL_TERMINATOR_LF_EOI, 0, 0, "", 0},
		{MS97XX_VAL_TERMINATOR_CR_LF_EOI, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrTerminatorRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrTerminatorRangeTableEntries,
	};


static IviRangeTableEntry attrMeasureModeRangeTable_10Entries[] =
	{
		{MS97XX_VAL_MEASURE_MODE_SPECTRUM, 0, 0, "SPC", 0},
		{MS97XX_VAL_MEASURE_MODE_PMD, 0, 0, "PMD", 0},
		{MS97XX_VAL_MEASURE_MODE_WDM, 0, 0, "WDM", 0},
		{MS97XX_VAL_MEASURE_MODE_DFB, 0, 0, "DFB", 0},
		{MS97XX_VAL_MEASURE_MODE_FP, 0, 0, "FP", 0},
		{MS97XX_VAL_MEASURE_MODE_LED, 0, 0, "LED", 0},
		{MS97XX_VAL_MEASURE_MODE_AMPLIFIER, 0, 0, "AMP", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMeasureModeRangeTable_10 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMeasureModeRangeTable_10Entries,
	};

static IviRangeTableEntry attrMeasureModeRangeTable_20Entries[] =
	{
		{MS97XX_VAL_MEASURE_MODE_SPECTRUM, 0, 0, "SPC", 0},
		{MS97XX_VAL_MEASURE_MODE_INSERTION_LOSS, 0, 0, "ILO", 0},
		{MS97XX_VAL_MEASURE_MODE_ISOLATION, 0, 0, "ISO", 0},
		{MS97XX_VAL_MEASURE_MODE_DIRECTIVITY, 0, 0, "DIR", 0},
		{MS97XX_VAL_MEASURE_MODE_RETURN_LOSS, 0, 0, "RLO", 0},
		{MS97XX_VAL_MEASURE_MODE_PMD, 0, 0, "PMD", 0},
		{MS97XX_VAL_MEASURE_MODE_NF, 0, 0, "NF", 0},
		{MS97XX_VAL_MEASURE_MODE_WDM, 0, 0, "WDM", 0},
		{MS97XX_VAL_MEASURE_MODE_LONG_TERM, 0, 0, "LTS", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMeasureModeRangeTable_20 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMeasureModeRangeTable_20Entries,
	};

static IviRangeTableEntry attrWdmModeRangeTable_10Entries[] =
	{
		{MS97XX_VAL_WDM_NONE, 0, 0, "", 0},
		{MS97XX_VAL_WDM_MULTIPEAK, 0, 0, "MPK", 0},
		{MS97XX_VAL_WDM_SNR, 0, 0, "SNR", 0},
		{MS97XX_VAL_WDM_RELATIVE, 0, 0, "REL", 0},
		{MS97XX_VAL_WDM_TABLE, 0, 0, "TBL", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmModeRangeTable_10 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmModeRangeTable_10Entries,
	};

static IviRangeTableEntry attrWdmModeRangeTable_20Entries[] =
	{
		{MS97XX_VAL_WDM_NONE, 0, 0, "", 0},
		{MS97XX_VAL_WDM_MULTIPEAK, 0, 0, "MPK", 0},
		{MS97XX_VAL_WDM_SNR, 0, 0, "SNR", 0},
		{MS97XX_VAL_WDM_RELATIVE, 0, 0, "REL", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmModeRangeTable_20 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmModeRangeTable_20Entries,
	};

static IviRangeTableEntry attrWdmCutLevelRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmCutLevelRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmCutLevelRangeTableEntries,
	};

static IviRangeTableEntry attrWdmSnrDirRangeTableEntries[] =
	{
		{MS97XX_VAL_SNR_DIRECTION_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_SNR_DIRECTION_RIGHT, 0, 0, "RIGHT", 0},
		{MS97XX_VAL_SNR_DIRECTION_HIGHER, 0, 0, "HIGHER", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmSnrDirRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmSnrDirRangeTableEntries,
	};

static IviRangeTableEntry attrWdmSnrDwRangeTable_10Entries[] =
	{
		{0, 20, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmSnrDwRangeTable_10 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmSnrDwRangeTable_10Entries,
	};

static IviRangeTableEntry attrWdmSnrDwRangeTable_20Entries[] =
	{
		{0, 120, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmSnrDwRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmSnrDwRangeTable_20Entries,
	};

static IviRangeTableEntry attrWdmRefPeakRangeTable_20Entries[] =
	{
		{1, 128, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmRefPeakRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmRefPeakRangeTable_20Entries,
	};

static IviRangeTableEntry attrWdmRefPeakRangeTable_10Entries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmRefPeakRangeTable_10 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmRefPeakRangeTable_10Entries,
	};

static IviRangeTableEntry attrWdmTableDirRangeTableEntries[] =
	{
		{MS97XX_VAL_WDM_TABLE_DIR_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_WDM_TABLE_DIR_RIGHT, 0, 0, "RIGHT", 0},
		{MS97XX_VAL_WDM_TABLE_DIR_HIGHER, 0, 0, "HIGHER", 0},
		{MS97XX_VAL_WDM_TABLE_DIR_AVERAGE, 0, 0, "AVERAGE", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmTableDirRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmTableDirRangeTableEntries,
	};

static IviRangeTableEntry attrWdmTableDwRangeTableEntries[] =
	{
		{0, 20, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmTableDwRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmTableDwRangeTableEntries,
	};

static IviRangeTableEntry attrWdmPeakTypeRangeTableEntries[] =
	{
		{MS97XX_VAL_PEAK_TYPE_MAX, 0, 0, "MAX", 0},
		{MS97XX_VAL_PEAK_TYPE_THRESHOLD, 0, 0, "THRESHOLD", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmPeakTypeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmPeakTypeRangeTableEntries,
	};

static IviRangeTableEntry attrWdmThresholdRangeTableEntries[] =
	{
		{0.1, 50.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmThresholdRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmThresholdRangeTableEntries,
	};

static IviRangeTableEntry attrNfFittingSpanRangeTableEntries[] =
	{
		{0.1, 100.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfFittingSpanRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfFittingSpanRangeTableEntries,
	};

static IviRangeTableEntry attrNfPinLossRangeTableEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfPinLossRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfPinLossRangeTableEntries,
	};

static IviRangeTableEntry attrNfCalRangeTableEntries[] =
	{
		{0.1, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfCalRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfCalRangeTableEntries,
	};

static IviRangeTableEntry attrNfAseCalRangeTableEntries[] =
	{
		{-30.0, 30.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfAseCalRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfAseCalRangeTableEntries,
	};

static IviRangeTableEntry attrNfPoutLossRangeTableEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfPoutLossRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfPoutLossRangeTableEntries,
	};

static IviRangeTableEntry attrNfCutLevelRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNfCutLevelRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNfCutLevelRangeTableEntries,
	};

static IviRangeTableEntry attrScreenModeRangeTableEntries[] =
	{
		{MS97XX_VAL_SCREEN_MODE_GRAPH, 0, 0, "GRAPH", 0},
		{MS97XX_VAL_SCREEN_MODE_TABLE, 0, 0, "TABLE", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrScreenModeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrScreenModeRangeTableEntries,
	};

static IviRangeTableEntry attrSelectedMemRangeTableEntries[] =
	{
		{MS97XX_VAL_MEMORY_A, 0, 0, "A", 0},
		{MS97XX_VAL_MEMORY_B, 0, 0, "B", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSelectedMemRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSelectedMemRangeTableEntries,
	};

static IviRangeTableEntry attrAmpNfCalRangeTableEntries[] =
	{
		{0.1, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpNfCalRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpNfCalRangeTableEntries,
	};

static IviRangeTableEntry attrAmpBandPassCutRangeTableEntries[] =
	{
		{0, 30, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpBandPassCutRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpBandPassCutRangeTableEntries,
	};

static IviRangeTableEntry attrAmpFilterBandwidthRangeTableEntries[] =
	{
		{0.01, 999.99, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpFilterBandwidthRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpFilterBandwidthRangeTableEntries,
	};

static IviRangeTableEntry attrAmpPolarLossRangeTableEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpPolarLossRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpPolarLossRangeTableEntries,
	};

static IviRangeTableEntry attrAmpMeasureMethodRangeTableEntries[] =
	{
		{MS97XX_VAL_AMP_NF_NO_SPEC_DIV, 0, 0, "", 0},
		{MS97XX_VAL_AMP_NF_SPEC_DIV, 0, 0, "", 0},
		{MS97XX_VAL_AMP_NF_POLAR_NULL, 0, 0, "", 0},
		{MS97XX_VAL_AMP_NF_PULSE, 0, 0, "", 0},
		{MS97XX_VAL_AMP_WDM, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpMeasureMethodRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpMeasureMethodRangeTableEntries,
	};

static IviRangeTableEntry attrAmpFittingMethodRangeTableEntries[] =
	{
		{MS97XX_VAL_AMP_GAUSS, 0, 0, "", 0},
		{MS97XX_VAL_AMP_MEAN, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpFittingMethodRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpFittingMethodRangeTableEntries,
	};

static IviRangeTableEntry attrAmpFittingSpanRangeTableEntries[] =
	{
		{0.1, 100.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpFittingSpanRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpFittingSpanRangeTableEntries,
	};

static IviRangeTableEntry attrAmpMaskedSpanRangeTableEntries[] =
	{
		{0.1, 100.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpMaskedSpanRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpMaskedSpanRangeTableEntries,
	};

static IviRangeTableEntry attrAmpPinLossRangeTableEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpPinLossRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpPinLossRangeTableEntries,
	};

static IviRangeTableEntry attrAmpPoutLossRangeTableEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpPoutLossRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpPoutLossRangeTableEntries,
	};

static IviRangeTableEntry attrDfbNdbWidthRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDfbNdbWidthRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDfbNdbWidthRangeTableEntries,
	};

static IviRangeTableEntry attrFpAxisModeCutLevelRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrFpAxisModeCutLevelRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrFpAxisModeCutLevelRangeTableEntries,
	};

static IviRangeTableEntry attrLedNdbDownWaveRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLedNdbDownWaveRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLedNdbDownWaveRangeTableEntries,
	};

static IviRangeTableEntry attrLedPowerCompRangeTableEntries[] =
	{
		{-10, 10, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLedPowerCompRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLedPowerCompRangeTableEntries,
	};

static IviRangeTableEntry attrPmdCouplingRangeTableEntries[] =
	{
		{0.01, 1.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrPmdCouplingRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrPmdCouplingRangeTableEntries,
	};

static IviRangeTableEntry attrAmpMemoryRangeTableEntries[] =
	{
		{MS97XX_VAL_AMP_MEMORY_PIN, 0, 0, "PIN", 0},
		{MS97XX_VAL_AMP_MEMORY_POUT, 0, 0, "POUT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpMemoryRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpMemoryRangeTableEntries,
	};

static IviRangeTableEntry attrAmpCalStatusRangeTableEntries[] =
	{
		{MS97XX_VAL_AMP_CAL_FINISHED, 0, 0, "", 0},
		{MS97XX_VAL_AMP_CAL_INSUFF_LIGHT, 0, 0, "", 0},
		{MS97XX_VAL_AMP_CAL_ABORT, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpCalStatusRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpCalStatusRangeTableEntries,
	};

static IviRangeTableEntry attrAmpCalcModeRangeTableEntries[] =
	{
		{MS97XX_VAL_AMP_CALC_NF_S_ASE, 0, 0, "NF S-ASE", 0},
		{MS97XX_VAL_AMP_CALC_NF_TOTAL, 0, 0, "NF TOTAL", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAmpCalcModeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAmpCalcModeRangeTableEntries,
	};

static IviRangeTableEntry attrDfbSideModeRangeTableEntries[] =
	{
		{MS97XX_VAL_DFB_SIDE_MODE_2NDPEAK, 0, 0, "2NDPEAK", 0},
		{MS97XX_VAL_DFB_SIDE_MODE_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_DFB_SIDE_MODE_RIGHT, 0, 0, "RIGHT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDfbSideModeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDfbSideModeRangeTableEntries,
	};

static IviRangeTableEntry attrFileOptionRangeTableEntries[] =
	{
		{MS97XX_VAL_FILE_OPTION_NONE, 0, 0, "NONE", 0},
		{MS97XX_VAL_FILE_OPTION_BMP, 0, 0, "BMP", 0},
		{MS97XX_VAL_FILE_OPTION_TXT, 0, 0, "TXT", 0},
		{MS97XX_VAL_FILE_OPTION_BMP_TXT, 0, 0, "BMP&TXT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrFileOptionRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrFileOptionRangeTableEntries,
	};

static IviRangeTableEntry attrFileIdFormatRangeTableEntries[] =
	{
		{MS97XX_VAL_FILE_ID_FORMAT_NAME, 0, 0, "NAME", 0},
		{MS97XX_VAL_FILE_ID_FORMAT_NUMBER, 0, 0, "NUMBER", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrFileIdFormatRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrFileIdFormatRangeTableEntries,
	};

static IviRangeTableEntry attrDisketteSizeRangeTableEntries[] =
	{
		{MS97XX_VAL_DISKETTE_SIZE_1_44MB, 0, 0, "1.44M", 0},
		{MS97XX_VAL_DISKETTE_SIZE_1_2MB, 0, 0, "1.2M", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrDisketteSizeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrDisketteSizeRangeTableEntries,
	};

static IviRangeTableEntry attrFileNumericFormatRangeTableEntries[] =
	{
		{MS97XX_VAL_FILE_NUMERIC_FORMAT_LOG, 0, 0, "LOG", 0},
		{MS97XX_VAL_FILE_NUMERIC_FORMAT_LINEAR, 0, 0, "LINEAR", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrFileNumericFormatRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrFileNumericFormatRangeTableEntries,
	};

static IviRangeTableEntry attrInstrumentFormatRangeTableEntries[] =
	{
		{MS97XX_VAL_INSTRUMENT_FORMAT_MS9720, 0, 0, "MS9720", 0},
		{MS97XX_VAL_INSTRUMENT_FORMAT_MS9715, 0, 0, "MS9715", 0},
		{MS97XX_VAL_INSTRUMENT_FORMAT_MS9710, 0, 0, "MS9710", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrInstrumentFormatRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrInstrumentFormatRangeTableEntries,
	};

static IviRangeTableEntry attrNdbAttenuationRangeTableEntries[] =
	{
		{0.1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrNdbAttenuationRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrNdbAttenuationRangeTableEntries,
	};

static IviRangeTableEntry attrAnalysisModeRangeTable_10Entries[] =
	{
		{MS97XX_VAL_ANALYSIS_MODE_OFF, 0, 0, "OFF", 0},
		{MS97XX_VAL_ANALYSIS_MODE_THR, 0, 0, "THR", 0},
		{MS97XX_VAL_ANALYSIS_MODE_SMSR, 0, 0, "SMSR", 0},
		{MS97XX_VAL_ANALYSIS_MODE_PWR, 0, 0, "PWR", 0},
		{MS97XX_VAL_ANALYSIS_MODE_ENV, 0, 0, "ENV", 0},
		{MS97XX_VAL_ANALYSIS_MODE_RMS, 0, 0, "RMS", 0},
		{MS97XX_VAL_ANALYSIS_MODE_NDB, 0, 0, "NDB", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAnalysisModeRangeTable_10 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAnalysisModeRangeTable_10Entries,
	};

static IviRangeTableEntry attrAnalysisModeRangeTable_20Entries[] =
	{
		{MS97XX_VAL_ANALYSIS_MODE_OFF, 0, 0, "OFF", 0},
		{MS97XX_VAL_ANALYSIS_MODE_THR, 0, 0, "THR", 0},
		{MS97XX_VAL_ANALYSIS_MODE_SMSR, 0, 0, "SMSR", 0},
		{MS97XX_VAL_ANALYSIS_MODE_PWR, 0, 0, "PWR", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAnalysisModeRangeTable_20 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAnalysisModeRangeTable_20Entries,
	};

static IviRangeTableEntry attrThresholdCutRangeTableEntries[] =
	{
		{1, 50, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrThresholdCutRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrThresholdCutRangeTableEntries,
	};

static IviRangeTableEntry attrSmsrSideModeRangeTableEntries[] =
	{
		{MS97XX_VAL_SMSR_SIDE_MODE_2ND, 0, 0, "2NDPEAK", 0},
		{MS97XX_VAL_SMSR_SIDE_MODE_LEFT, 0, 0, "LEFT", 0},
		{MS97XX_VAL_SMSR_SIDE_MODE_RIGHT, 0, 0, "RIGHT", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSmsrSideModeRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSmsrSideModeRangeTableEntries,
	};

static IviRangeTableEntry attrEnvelopeCutRangeTableEntries[] =
	{
		{0.1, 20, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrEnvelopeCutRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrEnvelopeCutRangeTableEntries,
	};

static IviRangeTableEntry attrRmsCutRangeTableEntries[] =
	{
		{0.1, 20, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrRmsCutRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrRmsCutRangeTableEntries,
	};

static IviRangeTableEntry attrRmsCoefficientRangeTableEntries[] =
	{
		{0.0, 1.0, 1.0, "", 0},
		{1.0, 2.0, 2.0, "", 0},
		{2.0, 2.35, 2.35, "", 0},
		{2.35, 3.0, 3.0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrRmsCoefficientRangeTable =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrRmsCoefficientRangeTableEntries,
	};

static IviRangeTableEntry attrSamplingPointsRangeTableEntries[] =
	{
		{1, 51, 51, "", 0},
		{52, 101, 101, "", 0},
		{102, 251, 251, "", 0},
		{252, 501, 501, "", 0},
		{502, 1001, 1001, "", 0},
		{1002, 2001, 2001, "", 0},
		{2002, 5001, 5001, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSamplingPointsRangeTable =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSamplingPointsRangeTableEntries,
	};

static IviRangeTableEntry attrSamplingPointsRangeTable_2628Entries[] =
	{
		{51, 0, 0, "", 0},
		{101, 0, 0, "", 0},
		{251, 0, 0, "", 0},
		{501, 0, 0, "", 0},
		{1001, 0, 0, "", 0},
		{2001, 0, 0, "", 0},
		{5001, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSamplingPointsRangeTable_2628 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSamplingPointsRangeTable_2628Entries,
	};

static IviRangeTableEntry attrPeakHoldRangeTableEntries[] =
	{
		{0, 50000, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrPeakHoldRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrPeakHoldRangeTableEntries,
	};

static IviRangeTableEntry attrModulationModeRangeTable_10Entries[] =
	{
		{MS97XX_VAL_MODULATION_MODE_NORMAL, 0, 0, "NORMAL", 0},
		{MS97XX_VAL_MODULATION_MODE_TRIGGER, 0, 0, "TRIGGER", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrModulationModeRangeTable_10 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrModulationModeRangeTable_10Entries,
	};

static IviRangeTableEntry attrModulationModeRangeTable_20Entries[] =
	{
		{MS97XX_VAL_MODULATION_MODE_NORMAL, 0, 0, "NORMAL", 0},
		{MS97XX_VAL_MODULATION_MODE_HOLD, 0, 0, "HOLD", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrModulationModeRangeTable_20 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrModulationModeRangeTable_20Entries,
	};

static IviRangeTableEntry attrSweepIntervalTimeRangeTableEntries[] =
	{
		{0, 5940, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSweepIntervalTimeRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSweepIntervalTimeRangeTableEntries,
	};

static IviRangeTableEntry attrExternalTriggerDelayRangeTableEntries[] =
	{
		{0, 1E6, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrExternalTriggerDelayRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrExternalTriggerDelayRangeTableEntries,
	};

static IviRangeTableEntry attrAverageRangeTableEntries[] =
	{
		{0, 0, 0, "", 0},
		{2, 1000, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrAverageRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrAverageRangeTableEntries,
	};

static IviRangeTableEntry attrSmoothingRangeTableEntries[] =
	{
		{0, 0, 0, "", 0},
		{3, 0, 0, "", 0},
		{5, 0, 0, "", 0},
		{7, 0, 0, "", 0},
		{9, 0, 0, "", 0},
		{11, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrSmoothingRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrSmoothingRangeTableEntries,
	};

static IviRangeTableEntry attrVideoBandWidthRangeTableEntries[] =
	{
		{0, MS97XX_VAL_VBW_10HZ, MS97XX_VAL_VBW_10HZ, "10HZ", 0},
		{MS97XX_VAL_VBW_10HZ, MS97XX_VAL_VBW_100HZ, MS97XX_VAL_VBW_100HZ, "100HZ", 0},
		{MS97XX_VAL_VBW_100HZ, MS97XX_VAL_VBW_1KHZ, MS97XX_VAL_VBW_1KHZ, "1KHZ", 0},
		{MS97XX_VAL_VBW_1KHZ, MS97XX_VAL_VBW_10KHZ, MS97XX_VAL_VBW_10KHZ, "10KHZ", 0},
		{MS97XX_VAL_VBW_10KHZ, MS97XX_VAL_VBW_100KHZ, MS97XX_VAL_VBW_100KHZ, "100KHZ", 0},
		{MS97XX_VAL_VBW_100KHZ, MS97XX_VAL_VBW_1MHZ, MS97XX_VAL_VBW_1MHZ, "1MHZ", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrVideoBandWidthRangeTable =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrVideoBandWidthRangeTableEntries,
	};

static IviRangeTableEntry attrVideoBandWidthRangeTable_2628Entries[] =
	{
		{MS97XX_VAL_VBW_1KHZ, 0, 0, "1 KHZ", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrVideoBandWidthRangeTable_2628 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrVideoBandWidthRangeTable_2628Entries,
	};

static IviRangeTableEntry attrResolutionRangeTable_10BEntries[] =
	{
		{0.5, 1.0, 1.0, "", 0},
		{0.2, 0.5, 0.5, "", 0},
		{0.1, 0.2, 0.2, "", 0},
		{0.07, 0.1, 0.1, "", 0},
		{0, 0.07, 0.07, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrResolutionRangeTable_10B =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrResolutionRangeTable_10BEntries,
	};

static IviRangeTableEntry attrResolutionRangeTable_10CEntries[] =
	{
		{0.5, 1.0, 1.0, "", 0},
		{0.2, 0.5, 0.5, "", 0},
		{0.1, 0.2, 0.2, "", 0},
		{0.07, 0.1, 0.1, "", 0},
		{0.05, 0.07, 0.07, "", 0},
		{0, 0.05, 0.05, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrResolutionRangeTable_10C =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrResolutionRangeTable_10CEntries,
	};

static IviRangeTableEntry attrResolutionRangeTable_20Entries[] =
	{
		{0.5, 1.0, 1.0, "", 0},
		{0.2, 0.5, 0.5, "", 0},
		{0.1, 0.2, 0.2, "", 0},
		{0, 0.1, 0.1, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrResolutionRangeTable_20 =
	{
		IVI_VAL_COERCED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrResolutionRangeTable_20Entries,
	};
static IviRangeTableEntry attrResolutionRangeTable_26Entries[] =
	{
		{0.1, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrResolutionRangeTable_26 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrResolutionRangeTable_26Entries,
	};
static IviRangeTableEntry attrResolutionRangeTable_28Entries[] =
	{
		{0.05, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrResolutionRangeTable_28 =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrResolutionRangeTable_28Entries,
	};

static IviRangeTableEntry attrWdmSignalSpacingRangeTableEntries[] =
	{
		{0, 0, 0, "", 0},
		{0.01, 200, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmSignalSpacingRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmSignalSpacingRangeTableEntries,
	};

static IviRangeTableEntry attrWdmSignalCountRangeTableEntries[] =
	{
		{1, 128, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWdmSignalCountRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWdmSignalCountRangeTableEntries,
	};

static IviRangeTableEntry attrActiveScreenRangeTableEntries[] =
	{
		{MS97XX_VAL_ACTIVE_SCREEN_LOWER, 0, 0, "LOW", 0},
		{MS97XX_VAL_ACTIVE_SCREEN_UPPER, 0, 0, "UP", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrActiveScreenRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrActiveScreenRangeTableEntries,
	};

static IviRangeTableEntry attrCalStatusRangeTableEntries[] =
	{
		{MS97XX_VAL_CAL_SUCCESS, 0, 0, "", 0},
		{MS97XX_VAL_CAL_IN_PROGRESS, 0, 0, "", 0},
		{MS97XX_VAL_CAL_INSUFF_LIGHT, 0, 0, "", 0},
		{MS97XX_VAL_CAL_ABORT, 0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrCalStatusRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrCalStatusRangeTableEntries,
	};

static IviRangeTableEntry attrActiveTraceRangeTableEntries[] =
	{
		{1, 3, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrActiveTraceRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrActiveTraceRangeTableEntries,
	};


static IviRangeTableEntry attrTraceScreenRangeTableEntries[] =
	{
		{MS97XX_VAL_TRACE_SCREEN_UPPER, 0, 0, "UP", 0},
		{MS97XX_VAL_TRACE_SCREEN_LOWER, 0, 0, "LOW", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrTraceScreenRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrTraceScreenRangeTableEntries,
	};

static IviRangeTableEntry attrTraceNumeratorRangeTableEntries[] =
	{
		{MS97XX_VAL_TRACE_A, 0, 0, "A", 0},
		{MS97XX_VAL_TRACE_B, 0, 0, "B", 0},
		{MS97XX_VAL_TRACE_C, 0, 0, "C", 0},
		{MS97XX_VAL_TRACE_A_PLUS_B, 0, 0, "A+B", 0},
		{MS97XX_VAL_TRACE_A_B, 0, 0, "A-B", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrTraceNumeratorRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrTraceNumeratorRangeTableEntries,
	};

static IviRangeTableEntry attrTraceDenominatorRangeTableEntries[] =
	{
		{MS97XX_VAL_TRACE_NONE, 0, 0, "NONE", 0},
		{MS97XX_VAL_TRACE_A, 0, 0, "A", 0},
		{MS97XX_VAL_TRACE_B, 0, 0, "B", 0},
		{MS97XX_VAL_TRACE_C, 0, 0, "C", 0},
		{MS97XX_VAL_TRACE_A_PLUS_B, 0, 0, "A+B", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrTraceDenominatorRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrTraceDenominatorRangeTableEntries,
	};

static IviRangeTableEntry attrMemoryStorageRangeTableEntries[] =
	{
		{MS97XX_VAL_MEMORY_STORAGE_REWRITE, 0, 0, "REWRITE", 0},
		{MS97XX_VAL_MEMORY_STORAGE_FIX, 0, 0, "FIX", 0},
		{MS97XX_VAL_MEMORY_STORAGE_MAX_HOLD, 0, 0, "MAX_HOLD", 0},
		{MS97XX_VAL_MEMORY_STORAGE_MIN_HOLD, 0, 0, "MIN_HOLD", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrMemoryStorageRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrMemoryStorageRangeTableEntries,
	};

static IviRangeTableEntry attrCurrentTraceRangeTableEntries[] =
	{
		{MS97XX_VAL_CURRENT_TRACE_A, 0, 0, "A", 0},
		{MS97XX_VAL_CURRENT_TRACE_B, 0, 0, "B", 0},
		{MS97XX_VAL_CURRENT_TRACE_AB, 0, 0, "AB", 0},
		{MS97XX_VAL_CURRENT_TRACE_A_B, 0, 0, "A-B", 0},
		{MS97XX_VAL_CURRENT_TRACE_B_A, 0, 0, "B-A", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrCurrentTraceRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrCurrentTraceRangeTableEntries,
	};

static IviRangeTableEntry attrLevelOffsetRangeTable_20Entries[] =
	{
		{-15.0, 15.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLevelOffsetRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLevelOffsetRangeTable_20Entries,
	};

static IviRangeTableEntry attrLevelOffsetRangeTable_10CEntries[] =
	{
		{-30.0, 30.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLevelOffsetRangeTable_10C =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLevelOffsetRangeTable_10CEntries,
	};

static IviRangeTableEntry attrLevelOffsetRangeTable_10BEntries[] =
	{
		{-10.0, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLevelOffsetRangeTable_10B =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLevelOffsetRangeTable_10BEntries,
	};

static IviRangeTableEntry attrLinearScaleRangeTable_PercentEntries[] =
	{
		{1, 200, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLinearScaleRangeTable_Percent =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLinearScaleRangeTable_PercentEntries,
	};

static IviRangeTableEntry attrLinearScaleRangeTableEntries[] =
	{
		{1E-12, 1, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLinearScaleRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLinearScaleRangeTableEntries,
	};

static IviRangeTableEntry attrLevelScaleRangeTableEntries[] =
	{
		{MS97XX_VAL_LEVEL_SCALE_LOG, 0, 0, "LOG", 0},
		{MS97XX_VAL_LEVEL_SCALE_LINEAR, 0, 0, "LIN", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLevelScaleRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLevelScaleRangeTableEntries,
	};

static IviRangeTableEntry attrLogScaleRangeTableEntries[] =
	{
		{0.1, 10.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrLogScaleRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrLogScaleRangeTableEntries,
	};

static IviRangeTableEntry attrRefLevelRangeTableEntries[] =
	{
		{-90.0, 30.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrRefLevelRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrRefLevelRangeTableEntries,
	};

static IviRangeTableEntry attrRefdBLevelRangeTableEntries[] =
	{
		{-100.0, 100.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrRefdBLevelRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrRefdBLevelRangeTableEntries,
	};


static IviRangeTableEntry attrWavelengthDisplayRangeTableEntries[] =
	{
		{MS97XX_VAL_WAVELENGTH_DISPLAY_VACUUM, 0, 0, "VACUUM", 0},
		{MS97XX_VAL_WAVELENGTH_DISPLAY_AIR, 0, 0, "AIR", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWavelengthDisplayRangeTable =
	{
		IVI_VAL_DISCRETE,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWavelengthDisplayRangeTableEntries,
	};

static IviRangeTableEntry attrWavelengthOffsetRangeTableEntries[] =
	{
		{-1.0, 1.0, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWavelengthOffsetRangeTable =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWavelengthOffsetRangeTableEntries,
	};

static IviRangeTableEntry attrCenterWavelengthRangeTable_10Entries[] =
	{
		{600, 1650, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrCenterWavelengthRangeTable_10 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrCenterWavelengthRangeTable_10Entries,
	};

static IviRangeTableEntry attrCenterWavelengthRangeTable_20Entries[] =
	{
		{600, 1650, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrCenterWavelengthRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrCenterWavelengthRangeTable_20Entries,
	};

static IviRangeTableEntry attrWavelengthSpanRangeTable_10Entries[] =
	{
		{0, 1200, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWavelengthSpanRangeTable_10 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWavelengthSpanRangeTable_10Entries,
	};

static IviRangeTableEntry attrWavelengthSpanRangeTable_20Entries[] =
	{
		{0, 1200, 0, "", 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrWavelengthSpanRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrWavelengthSpanRangeTable_20Entries,
	};

static IviRangeTableEntry attrStartWavelengthRangeTable_10Entries[] =
	{
		{600, 1800, 0, VI_NULL, 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrStartWavelengthRangeTable_10 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrStartWavelengthRangeTable_10Entries,
	};

static IviRangeTableEntry attrStartWavelengthRangeTable_20Entries[] =
	{
		{1450, 1650, 0, VI_NULL, 0},
		{IVI_RANGE_TABLE_LAST_ENTRY}
	};

static IviRangeTable attrStartWavelengthRangeTable_20 =
	{
		IVI_VAL_RANGED,
        VI_TRUE,
        VI_TRUE,
        VI_NULL,
        attrStartWavelengthRangeTable_20Entries,
	};

/*****************************************************************************
 *-------------- Utility Function Declarations (Non-Exported) ---------------*
 *****************************************************************************/
static ViStatus MS97xx_InitAttributes (ViSession vi);
static ViStatus MS97xx_DefaultInstrSetup (ViSession openInstrSession);
static ViStatus MS97xx_CheckStatus (ViSession vi);
static ViStatus MS97xx_DisableAttribute (ViSession vi, ViInt32 attr);

static ViStatus MS97xx_ReadViReal64Attribute (ViSession vi,
                                              ViSession io,
                                              ViConstString channelName,
                                              ViAttr attributeId,
                                              ViReal64 *value,
                                              ViConstString query,
                                              ViInt32 headers);

static ViStatus MS97xx_ReadViInt32Attribute (ViSession vi,
                                             ViSession io,
                                             ViConstString channelName,
                                             ViAttr attributeId,
                                             ViInt32 *value,
                                             ViConstString query,
                                             ViInt32 headers);

static ViStatus MS97xx_ReadDiscreteViInt32Attribute (ViSession vi,
                                                     ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViInt32 *value,
                                                     ViConstString query,
                                                     ViInt32 headers);

static ViStatus MS97xx_WriteDiscreteViInt32Attribute (ViSession vi,
                                                      ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViInt32 value,
                                                      ViConstString query);

static ViStatus MS97xx_ReadDiscreteViReal64Attribute (ViSession vi,
                                                      ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViReal64 *value,
                                                      ViConstString query,
                                                      ViInt32 headers);

static ViStatus MS97xx_WriteDiscreteViReal64Attribute (ViSession vi,
                                                       ViSession io,
                                                       ViConstString channelName,
                                                       ViAttr attributeId,
                                                       ViReal64 value,
                                                       ViConstString query);
                                                       
static ViStatus MS97xx_ReadBinaryInt16Array (ViSession vi,ViSession io,ViInt32 maxPoints,
											 ViInt16 data[],ViInt32 *numPoints);

static ViStatus MS97xx_ReadBinaryReal64Array (ViSession vi,ViSession io,ViInt32 maxPoints,
											  ViReal64 data[],ViInt32 *numPoints);

static ViStatus MS97xx_GetReadBuffer (ViSession vi,ViInt32 size,ViAddr *readBuf);

static ViStatus MS97xx_CheckFileName (ViSession vi,ViChar fileName[]);

static ViStatus MS97xx_CheckMeasureMode (ViSession vi,ViUInt16 modeMask,ViBoolean allowModes);

static ViStatus MS97xx_CheckWDM_Mode (ViSession vi,ViUInt16 modeMask);

static ViStatus MS97xx_CheckDisplay_Mode (ViSession vi,ViUInt16 modeMask);

static ViBoolean MS97xx_CachingIsOn (ViSession vi);

    /*- File I/O Utility Functions -*/
static ViStatus MS97xx_ReadToFile (ViSession vi, ViConstString filename, 
                                     ViInt32 maxBytesToRead, ViInt32 fileAction, ViInt32 *totalBytesWritten);
static ViStatus MS97xx_WriteFromFile (ViSession vi, ViConstString filename, 
                                        ViInt32 maxBytesToWrite, ViInt32 byteOffset, 
                                        ViInt32 *totalBytesWritten);

/*****************************************************************************
 *----------------- Callback Declarations (Non-Exported) --------------------*
 *****************************************************************************/

    /*- Global Session Callbacks --------------------------------------------*/
    
static ViStatus _VI_FUNC MS97xx_CheckStatusCallback (ViSession vi, ViSession io);
static ViStatus _VI_FUNC MS97xx_WaitForOPCCallback (ViSession vi, ViSession io);

    /*- Attribute callbacks -------------------------------------------------*/

static ViStatus _VI_FUNC MS97xxAttrDriverRevision_ReadCallback (ViSession vi,
                                                                  ViSession io, 
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId, 
                                                                  const ViConstString cacheValue);
static ViStatus _VI_FUNC MS97xxAttrIdQueryResponse_ReadCallback (ViSession vi,
                                                                   ViSession io, 
                                                                   ViConstString channelName, 
                                                                   ViAttr attributeId, 
                                                                   const ViConstString cacheValue);

static ViStatus _VI_FUNC MS97xxAttrStartWavelength_RangeTableCallback (ViSession vi,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrOsaType_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrStartWavelength_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrStartWavelength_WriteCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_RangeTableCallback (ViSession vi,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrAutoAlignStatus_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthOffset_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthOffset_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthDisplay_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthDisplay_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrAutoRefOn_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrAutoRefOn_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLevelScale_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrLinearScale_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrLinearScale_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLinearScale_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLogScale_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrLogScale_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value);   */

static ViStatus _VI_FUNC MS97xxAttrRefLevel_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrRefLevel_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value);		*/

static ViStatus _VI_FUNC MS97xxAttrRefLevel_RangeTableCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrTraceVisible_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrTrace_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrTraceVisible_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrTrace_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrTraceAttr_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrActiveTrace_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrActiveTrace_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrActiveScreen_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrActiveScreen_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrAttenuatorOn_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean *value);

/*static ViStatus _VI_FUNC MS97xxAttrAttenuatorOn_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value);	  */

static ViStatus _VI_FUNC MS97xxAttrCurrentTrace_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrCurrentTrace_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrFirstWdmSignal_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrFirstWdmSignal_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLowerLogScale_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLowerLogScale_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLowerRefLevel_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLowerRefLevel_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrMemoryAStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMemoryAStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrMemoryBStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMemoryBStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrMemoryCStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMemoryCStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrUpperLogScale_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrUpperLogScale_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrUpperRefLevel_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrUpperRefLevel_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrWdmSignalCount_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmSignalCount_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmSignalSpacing_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmSignalSpacing_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrResolution_RangeTableCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrResolution_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrResolution_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value);   */

static ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value);	  */

static ViStatus _VI_FUNC MS97xxAttrActualResolution_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value);


static ViStatus _VI_FUNC MS97xxAttrAutoMeasuring_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean *value);


static ViStatus _VI_FUNC MS97xxAttrAutoResolution_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrAutoResolution_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrExternalTriggerDelay_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrExternalTriggerDelay_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrModulationMode_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrModulationMode_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrModulationMode_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrPeakHold_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrPeakHold_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrPointAverage_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrPointAverage_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSamplingPoints_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrSamplingPoints_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrSamplingPoints_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);	 */

static ViStatus _VI_FUNC MS97xxAttrShowActualResolution_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrShowActualResolution_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrSmoothing_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrSmoothing_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSweepAverage_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrSweepAverage_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSweepIntervalTime_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrSweepIntervalTime_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWideDynamicRange_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrWideDynamicRange_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrEnvelopeCut_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrEnvelopeCut_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrNdbAttenuation_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrNdbAttenuation_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrRmsCoefficient_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrRmsCoefficient_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrRmsCut_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrRmsCut_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrRmsAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrSmsrSideMode_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrSmsrSideMode_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrThresholdCut_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);
 /*
static ViStatus _VI_FUNC MS97xxAttrThresholdCut_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);   */


static ViStatus _VI_FUNC MS97xxAttrRms_ReadCallback (ViSession vi, ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrRms_WriteCallback (ViSession vi, ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_RangeTableCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrDisketteAttr_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrDiskette_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrDiskette_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrAmpAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrAmpCalStatus_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrAmpInt_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrAmpInt_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrAmpMemory_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrAmpMemory_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrAmpReal_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrAmpReal_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrDfbAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value);   

static ViStatus _VI_FUNC MS97xxAttrDfb_ReadCallback (ViSession vi, ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViInt32 *value);
/*
static ViStatus _VI_FUNC MS97xxAttrDfb_WriteCallback (ViSession vi, ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViInt32 value);	  */

static ViStatus _VI_FUNC MS97xxAttrFpAxisModeCutLevel_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 *value);
 /*
static ViStatus _VI_FUNC MS97xxAttrFpAxisModeCutLevel_WriteCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 value);   */

static ViStatus _VI_FUNC MS97xxAttrLedAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrLedNdbDownWave_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrLedNdbDownWave_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrLedPowerComp_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLedPowerComp_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrNfAttr_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrNfCutLevel_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrNfCutLevel_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrNf_ReadCallback (ViSession vi, ViSession io,
                                                    ViConstString channelName,
                                                    ViAttr attributeId,
                                                    ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrNf_WriteCallback (ViSession vi, ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrPmdCoupling_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrPmdCoupling_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrScreenMode_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrScreenMode_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSnrAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrTableAttr_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrWdmCutLevel_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmCutLevel_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmMode_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmPeakType_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmPeakType_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDir_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDir_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableDir_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableDir_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableDw_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableDw_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableNorm_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrWdmTableNorm_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrWdmThreshold_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrWdmThreshold_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);


static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_RangeTableCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_RangeTableCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSendHeader_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrSendHeader_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrTerminator_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrTerminator_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrEventStatusEnable_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrEventStatusEnable_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrEventStatus_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrServiceRequestEnable_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrServiceRequestEnable_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrStatusByte_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrError_ReadCallback (ViSession vi,
                                                       ViSession io,
                                                       ViConstString channelName,
                                                       ViAttr attributeId,
                                                       ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_RangeTableCallback (ViSession vi,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrMarkerUnits_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrMarkerUnits_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);




static ViStatus _VI_FUNC MS97xxAttrTraceMarkerWavelength_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViReal64 value);




static ViStatus _VI_FUNC MS97xxAttrZoneMarkerSpan_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr);



static ViStatus _VI_FUNC MS97xxAttrTraceMarker_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrZoneMarker_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrZoneMarker_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value);


static ViStatus _VI_FUNC MS97xxAttrZoneMarkerWl_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrFirstPmdPeak_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrFirstPmdPeak_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLastPmdPeak_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrLastPmdPeak_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrLightOutput_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrLightOutput_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrLongTermDataSize_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViInt32 *value);


static ViStatus _VI_FUNC MS97xxAttrLongTermSampleTime_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrLongTermSampleTime_WriteCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrLongTermState_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 *value);



static ViStatus _VI_FUNC MS97xxAttrPmdPeakCount_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrPmdPeakCount_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorResult_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorWavelength_ReadCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViReal64 *value);

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorWavelength_WriteCallback (ViSession vi,
                                                                         ViSession io,
                                                                         ViConstString channelName,
                                                                         ViAttr attributeId,
                                                                         ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrTlsCalStatus_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrTlsCalStatus_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrTlsTrackEnabled_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrTlsTrackEnabled_WriteCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrPeakSearch_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrPeakSearch_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrLongTermSpacingDisplay_ReadCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrLongTermSpacingDisplay_WriteCallback (ViSession vi,
                                                                         ViSession io,
                                                                         ViConstString channelName,
                                                                         ViAttr attributeId,
                                                                         ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmMode_RangeTableCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrWdmMode_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 value);

static ViStatus _VI_FUNC MS97xxNoLongTermBoolean_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrLogScale_CheckCallback (ViSession vi,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value);


static ViStatus _VI_FUNC MS97xxExcludeLongTermInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value);

static ViStatus _VI_FUNC MS97xxExcludeLongTermReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireSpectrumBoolean_CheckCallback (ViSession vi,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViBoolean value);

static ViStatus _VI_FUNC MS97xxRequireSpectrumInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireSpectrumReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireLossTestInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireLossTestReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireAmpInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireAmpReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireDfbInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireFpInt32_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireLedInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireLedReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireNfInt32_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireNfReal64_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequirePmdInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequirePmdReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxRequireWdmBoolean_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value);

static ViStatus _VI_FUNC MS97xxRequireWdmInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireWdmReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrScreenMode_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireLongTermInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrNorm_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean *value);

static ViStatus _VI_FUNC MS97xxAttrWdmSnrNorm_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value);

static ViStatus _VI_FUNC MS97xxAttrWavelengthCalStatus_ReadCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr);

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireSplitInt32_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 value);

static ViStatus _VI_FUNC MS97xxExcludeOverlapInt32_CheckCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireNormalInt32_CheckCallback (ViSession vi,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value);

static ViStatus _VI_FUNC MS97xxRequireNormalReal64_CheckCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 value);

static ViStatus _VI_FUNC MS97xxAttrDipSearch_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value);

static ViStatus _VI_FUNC MS97xxAttrDipSearch_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value);


/*****************************************************************************
 *------------ User-Callable Functions (Exportable Functions) ---------------*
 *****************************************************************************/

/* Sweep */

/*****************************************************************************
 * Function: MS97xx_SingleSweep                                       
 * Purpose:  This function causes the instrument to measure from the
 *           starting wavelength to the stop wavelength.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SingleSweep(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SSI%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ContinuousSweep                                       
 * Purpose:  This function causes the instrument to continue sweeping until
 *           the stop command is issued.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_ContinuousSweep(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SRT%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_StopSweep                                       
 * Purpose:  This function causes the instrument to stop performing
 *           a continuous sweep.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_StopSweep(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SST%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_PeakToCenter                                       
 * Purpose:  This function causes the instrument to center the start and stop
 *           wavelengths on the peak measurement.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_PeakToCenter(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"PKC%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_PeakToReference                                       
 * Purpose:  This function causes the instrument to set the peak value as
 *           the reference amplitude.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_PeakToReference(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC |
										  MS97XX_MODE_MASK_WDM,VI_TRUE));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_REF_LEVEL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));
	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_AUTO_REF_ON));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"PKL%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Wavelength */

/*****************************************************************************
 * Function: MS97xx_GetStartAndStopWavelength
 * Purpose:  This function retrieves the range of the sweep in terms of the
 *           beginning and end of the sweep.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetStartAndStopWavelength(ViSession vi,ViReal64 *startWl,ViReal64 *stopWl)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (startWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Start Wavelength");
	if (stopWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Stop Wavelength");

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_START_WAVELENGTH,0,startWl));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_STOP_WAVELENGTH,0,stopWl));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetStartAndStopWavelength
 * Purpose:  This function sets the range of the sweep in terms of the
 *           beginning and end of the sweep.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetStartAndStopWavelength(ViSession vi,ViReal64 startWl,ViReal64 stopWl)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;
	ViReal64 oldStart,oldStop;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (stopWl<=startWl)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Stop Wavelength");

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_START_WAVELENGTH,0,&oldStart));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_STOP_WAVELENGTH,0,&oldStop));
	
	if (startWl<oldStop)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_START_WAVELENGTH,0,startWl));
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_STOP_WAVELENGTH,0,stopWl));
	}
	else /* (stopWl>oldStart) */
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_STOP_WAVELENGTH,0,stopWl));
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_START_WAVELENGTH,0,startWl));
	}

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetCenterAndSpanWavelength
 * Purpose:  This function retrieves the range of the sweep in terms of the
 *           middle and width of the sweep.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetCenterAndSpanWavelength(ViSession vi,ViReal64 *centerWl,ViReal64 *wlSpan)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Center Wavelength");
	if (wlSpan==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Span Wavelength");

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH,0,centerWl));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN,0,wlSpan));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetCenterAndSpanWavelength
 * Purpose:  This function sets the range of the sweep in terms of the
 *           middle and width of the sweep.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetCenterAndSpanWavelength(ViSession vi,ViReal64 centerWl,ViReal64 wlSpan)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH,0,centerWl));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN,0,wlSpan));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CalibrateWavelength
 * Purpose:  This function carries out wavelenght calibration using an
 *           external or reference light source.  The "EXTERNAL"
 *           option can only be used on the MS9710 series.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_CalibrateWavelength(ViSession vi,ViInt32 calType)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if (calType < MS97XX_VAL_CAL_WAVELENGTH_DEFAULT || calType > MS97XX_VAL_CAL_WAVELENGTH_STOP)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Calibration Type");
	
	if (calType==MS97XX_VAL_CAL_WAVELENGTH_EXTERNAL &&
		(osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Calibration Type");

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_MARKER_A_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_MARKER_B_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_TRACE_MARKER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_OFFSET));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"WCAL %ld%s",calType,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_AutoAlign
 * Purpose:  This function causes the instrument to create alignment
 *           position data.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_AutoAlign(ViSession vi,ViInt32 alignCommand)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (alignCommand < MS97XX_VAL_AUTO_ALIGN_DEFAULT || alignCommand > MS97XX_VAL_AUTO_ALIGN_TERMINATE)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Alignment Command");

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ALIN %ld%s",alignCommand,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_AutoMeasure
 * Purpose:  This function sets the instrument to the best wavelength and
 *           resolution for the incident light spectrum.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_AutoMeasure(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_RESOLUTION));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"AUT%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Marker */

/*****************************************************************************
 * Function: MS97xx_GetDelta
 * Purpose:  This function moves the delta marker to the specified position
             and then returns the difference in wavelength and level from
             the trace marker.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDelta(ViSession vi,ViReal64 wavelength,ViReal64 *dWav,ViReal64 *dLevel)
{
	ViStatus error=VI_SUCCESS;
	ViReal64 startWl,stopWl;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_START_WAVELENGTH,0,&startWl));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_STOP_WAVELENGTH,0,&stopWl));

	if (wavelength<startWl || wavelength>stopWl)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Wavelength");

	if (dWav==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Delta Wavelength");

	if (dLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Delta Level");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"DMK %.4lf%s",wavelength,terminator));
		checkErr(viPrintf(io,"DMK?%s",terminator));
		checkErr(viScanf(io,"%lf%*[^,],%lf%*s",dWav,dLevel));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*dWav=((ViReal64)rand())/((ViReal64)RAND_MAX)*(stopWl-startWl);
		*dLevel=((ViReal64)rand())/((ViReal64)RAND_MAX)*-10.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_EraseMarkers
 * Purpose:  This function removes all trace, wavelength, and level
 *           markers from the display.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_EraseMarkers(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"EMK%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_TraceMarkerToCenter
 * Purpose:  This function centers the wavelength span on the trace marker.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_TraceMarkerToCenter(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViBoolean markerVisible;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(viPrintf(io,"TMC%s",terminator));
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_TraceMarkerToReference
 * Purpose:  This function sets the reference level equal to the level
 *           of the trace marker.  MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_TraceMarkerToReference(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;
	ViBoolean markerVisible;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));
    
    if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_REF_LEVEL));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_AUTO_REF_ON));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"TML%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetZoneMarkerCenterAndSpan
 * Purpose:  This function retrieves the properties of the zone marker.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetZoneMarkerCenterAndSpan(ViSession vi,ViReal64 *centerWl,ViReal64 *wlSpan)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Center Wavelength");
	if (wlSpan==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Span Wavelength");

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER,0,centerWl));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN,0,wlSpan));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetZoneMarkerCenterAndSpan
 * Purpose:  This function sets the properties of the zone marker.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetZoneMarkerCenterAndSpan(ViSession vi,ViReal64 centerWl,ViReal64 wlSpan)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER,0,centerWl));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN,0,wlSpan));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ZoneMarkerToSpan
 * Purpose:  This function sets the start and stop wavelengths to the span
 *           enclosed by the zone markers.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_ZoneMarkerToSpan(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViBoolean markerVisible;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_VISIBLE,0,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_MARKER_A_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_MARKER_B_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_TRACE_MARKER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ZMK SPN%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ZoneMarkerZoom
 * Purpose:  This function zooms in or out to the zone markers.
 *           MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_ZoneMarkerZoom(ViSession vi,ViInt32 zoomDirection)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;
	ViBoolean markerVisible;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));
    
    if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (zoomDirection != MS97XX_VAL_ZONE_MARKER_ZOOM_IN &&
	    zoomDirection != MS97XX_VAL_ZONE_MARKER_ZOOM_OUT)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Zoom Direction");
	    
	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_VISIBLE,0,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ZMK ZOOM,%s%s",
		                  (zoomDirection==MS97XX_VAL_ZONE_MARKER_ZOOM_OUT ? "OUT" : "IN"),
		                  terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_EraseZoneMarkers
 * Purpose:  This function removes the zone markers from the display.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_EraseZoneMarkers(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_VISIBLE,0,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ZMK ERS%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Analysis */

/*****************************************************************************
 * Function: MS97xx_GetRMS_Parameters
 * Purpose:  This function retrieves the factors used to generate the
 *           RMS spectrum analysis.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetRMS_Parameters(ViSession vi,ViInt32 *cutLevel,ViReal64 *coefficient)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (cutLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Cut Level");
	if (coefficient==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Coefficient");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_RMS_CUT,0,cutLevel));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_RMS_COEFFICIENT,0,coefficient));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetRMS_Parameters
 * Purpose:  This function sets the factors used to generate the
 *           RMS spectrum analysis.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetRMS_Parameters(ViSession vi,ViInt32 cutLevel,ViReal64 coefficient)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_RMS_CUT,0,cutLevel));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_RMS_COEFFICIENT,0,coefficient));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLevelAnalysis
 * Purpose:  This function retrieves the results of envelope, threshold,
 *           or RMS analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLevelAnalysis(ViSession vi,ViReal64 *centerWl,ViReal64 *width)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 mode;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Center Wavelength");

	if (width==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Width");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,&mode));
	
	if (mode != MS97XX_VAL_ANALYSIS_MODE_ENV &&
	    mode != MS97XX_VAL_ANALYSIS_MODE_THR &&
	    mode != MS97XX_VAL_ANALYSIS_MODE_RMS)
		checkErr(Ivi_SetAttributeViInt32(vi,"",
			MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_THR));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ANAR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf",centerWl,width));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*centerWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*width=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Get_ndB_Analysis
 * Purpose:  This function retrieves the results of n-dB analysis.
 *           MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Get_ndB_Analysis(ViSession vi,ViReal64 *centerWl,ViReal64 *width,ViInt32 *numModes)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

//	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
//		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Center Wavelength");

	if (width==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Width");

	if (numModes==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Number of Modes");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

//	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_NDB));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ANAR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%ld",centerWl,width,numModes));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*centerWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*width=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*numModes=rand() % 2 + 1;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetSMSR_Analysis
 * Purpose:  This function retrieves the results of side-mode suppression
 *           ratio analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetSMSR_Analysis(ViSession vi,ViReal64 *dWavelength,ViReal64 *dLevel)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (dWavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Delta Wavelength");

	if (dLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Delta Level");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

//	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_SMSR));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ANAR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf",dWavelength,dLevel));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*dWavelength=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX)-25.0;
		*dLevel=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX)-5.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetSpectrumPowerAnalysis
 * Purpose:  This function retrieves the results of spectrum power analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetSpectrumPowerAnalysis(ViSession vi,ViReal64 *power,ViReal64 *centerWl)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (power==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Power");

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Center Wavelength");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,
									 MS97XX_VAL_ANALYSIS_MODE_PWR));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"ANAR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf",power,centerWl));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*power=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX)-20.0;
		*centerWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Application */

/*****************************************************************************
 * Function: MS97xx_GetAmpParameters
 * Purpose:  This function retrieves the properties of the
 *           amplifier measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetAmpParameters(ViSession vi,ViInt32 *calcMode,ViInt32 *measMethod,ViInt32 *fitMethod,ViReal64 *fitSpan,ViReal64 *maskSpan,ViReal64 *pinLoss,ViReal64 *poutLoss,
										  ViReal64 *nfCal,ViReal64 *bandCut,ViReal64 *filterBW,ViReal64 *polarLoss)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (calcMode==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Calculation Mode");
	if (measMethod==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Measurement Method");
	if (fitMethod==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Fitting Method");
	if (fitSpan==VI_NULL)					
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Fitting Span");
	if (maskSpan==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Masked Span");
	if (pinLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Pin Loss");
	if (poutLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Pout Loss");
	if (nfCal==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"NF Calibration");
	if (bandCut==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,10,"Band Pass Cut Level");
	if (filterBW==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,11,"Filter Bandwidth");
	if (polarLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,12,"Polarization Loss");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_CALC_MODE,0,calcMode));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_MEASURE_METHOD,0,measMethod));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_FITTING_METHOD,0,fitMethod));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FITTING_SPAN,0,fitSpan));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_MASKED_SPAN,0,maskSpan));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_PIN_LOSS,0,pinLoss));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POUT_LOSS,0,poutLoss));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_NF_CAL,0,nfCal));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_BAND_PASS_CUT,0,bandCut));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FILTER_BANDWIDTH,0,filterBW));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POLAR_LOSS,0,polarLoss));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetAmpParameters
 * Purpose:  This function sets the properties of the
 *           amplifier measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetAmpParameters(ViSession vi,ViInt32 calcMode,ViInt32 measMethod,ViInt32 fitMethod,ViReal64 fitSpan,ViReal64 maskSpan,ViReal64 pinLoss,ViReal64 poutLoss,
										  ViReal64 nfCal,ViReal64 bandCut,ViReal64 filterBW,ViReal64 polarLoss)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_CALC_MODE,0,calcMode));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_MEASURE_METHOD,0,measMethod));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_FITTING_METHOD,0,fitMethod));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FITTING_SPAN,0,fitSpan));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_MASKED_SPAN,0,maskSpan));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_PIN_LOSS,0,pinLoss));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POUT_LOSS,0,poutLoss));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_NF_CAL,0,nfCal));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_BAND_PASS_CUT,0,bandCut));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FILTER_BANDWIDTH,0,filterBW));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POLAR_LOSS,0,polarLoss));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_AmpCalibrateResolution
 * Purpose:  This function calibrates the resolution of the OSA for
 *           amplifier measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_AmpCalibrateResolution(ViSession vi,ViInt32 setting)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (setting<MS97XX_VAL_AMP_RESOLUTION_CURRENT ||
	    setting>MS97XX_VAL_AMP_RESOLUTION_CALIBRATE)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Setting");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"AP AMP,CAL,%ld%s",setting,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_AmpPoutToPase
 * Purpose:  This function copies the Pout spectrum into the OSA's
 *           internal memory as an ASE.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_AmpPoutToPase(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"AP AMP,ASE%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetAmpResults
 * Purpose:  This function retrieves the results of amplifier analysis.
 *           MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetAmpResults(ViSession vi,ViReal64 *G,ViReal64 *NF,ViReal64 *signalWl,ViReal64 *aseLevel,ViReal64 *resolution)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (G==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Gain");
	if (NF==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Noise Figure");
	if (signalWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Signal Wavelength");
	if (aseLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"ASE Level");
	if (resolution==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Resolution");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf,%lf",G,NF,signalWl,aseLevel,resolution));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*G=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*NF=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*signalWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*aseLevel=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*resolution=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetDFB_Parameters
 * Purpose:  This function retrieves the properties of distributed-feedback
 *           diode analysis.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDFB_Parameters(ViSession vi,ViInt32 *sideMode,ViInt32 *ndB_Width)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (sideMode==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Side Mode");
	if (ndB_Width==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"n-dB Width");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,0,sideMode));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_NDB_WIDTH,0,ndB_Width));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetDFB_Parameters
 * Purpose:  This function sets the properties of distributed-feedback
 *           diode analysis.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetDFB_Parameters(ViSession vi,ViInt32 sideMode,ViInt32 ndB_Width)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

//	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
//		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,0,sideMode));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_NDB_WIDTH,0,ndB_Width));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetDFB_Results
 * Purpose:  This function retrieves the results of distributed-feedback
 *           diode analysis.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDFB_Results(ViSession vi,ViReal64 *SMSR,ViReal64 *ndbBW,ViReal64 *peakWl,ViReal64 *peakLevel,ViReal64 *sideWl,ViReal64 *sideLevel,ViReal64 *modeOffset,ViReal64 *stopBand,ViReal64 *centerOffset)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

//	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
//		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (SMSR==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Side Mode Suppression Ratio");
	if (ndbBW==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"ndB-down Wave Width");
	if (peakWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Peak Wavelength");
	if (peakLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Peak Level");
	if (sideWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Side Mode Wavelength");
	if (sideLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Side Mode Level");
	if (modeOffset==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Mode Offset");
	if (stopBand==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Stop Band");
	if (centerOffset==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,10,"Center Offset");

//	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_DFB,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
		                 SMSR,ndbBW,peakWl,peakLevel,sideWl,sideLevel,
		                 modeOffset,stopBand,centerOffset));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*SMSR=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*ndbBW=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*peakWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*peakLevel=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*sideWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*sideLevel=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*modeOffset=fabs(*sideWl-*peakWl);
		*stopBand=*modeOffset/2.0;
		*centerOffset=*modeOffset*2.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetFP_Results
 * Purpose:  This function retrieves the results of Fabry-Perot measurement.
 *           MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetFP_Results(ViSession vi,ViReal64 *fwhmWl,ViReal64 *centerWl,ViReal64 *peakWl,ViReal64 *peakLevel,ViInt32 *modes,ViReal64 *modeSpacing,ViReal64 *power, ViReal64 *sigma)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

//	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
//		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (fwhmWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Half-Magnitude Full Width");
	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Center Wavelength");
	if (peakWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Peak Wavelength");
	if (peakLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Peak Level");
	if (modes==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Modes");
	if (modeSpacing==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Mode Spacing");
	if (power==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Power");

//	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_FP,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf,%ld,%lf,%lf,%lf",
		                 fwhmWl,centerWl,peakWl,peakLevel,
		                 modes,modeSpacing,power,sigma));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*fwhmWl=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*centerWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*peakWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*peakLevel=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*modes=rand() % 2 + 1;
		*modeSpacing=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*power=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLED_Parameters
 * Purpose:  This function retrieves the properties of light-emitting diode
 *           measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLED_Parameters(ViSession vi,ViInt32 *ndB_Down,ViReal64 *powerComp)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (ndB_Down==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"n-dB Down Wave Width");
	if (powerComp==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Power Compensation");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_LED_NDB_DOWN_WAVE,0,ndB_Down));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_LED_POWER_COMP,0,powerComp));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetLED_Parameters
 * Purpose:  This function sets the properties of light-emitting diode
 *           measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetLED_Parameters(ViSession vi,ViInt32 ndB_Down,ViReal64 powerComp)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_LED_NDB_DOWN_WAVE,0,ndB_Down));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_LED_POWER_COMP,0,powerComp));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLED_Results
 * Purpose:  This function retrieves the results of light-emitting diode
 *           measurement.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLED_Results(ViSession vi,ViReal64 *centerWl,ViReal64 *ndbWl,ViReal64 *fwhmWl,ViReal64 *ndbBW,ViReal64 *peakWl,ViReal64 *peakLevel,ViReal64 *pkPowDens,ViReal64 *power)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (centerWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Center Wavelength from RMS");
	if (ndbWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Center Wavelength from Threshold");
	if (fwhmWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Half-Magnitude Full Width");
	if (ndbBW==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"ndB Bandwidth");
	if (peakWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Peak Wavelength");
	if (peakLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Peak Level");
	if (pkPowDens==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Peak Power Density");
	if (power==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Power");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LED,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
		                 centerWl,ndbWl,fwhmWl,ndbBW,peakWl,peakLevel,
		                 pkPowDens,power));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*centerWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*ndbWl=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+*centerWl-5.0;
		*fwhmWl=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*ndbBW=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*peakWl=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*peakLevel=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*pkPowDens=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*power=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetPMD_Results
 * Purpose:  This function retrieves the results of the polarization-mode
 *           dispersion measurement.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetPMD_Results(ViSession vi,ViReal64 *deltaT,ViReal64 *firstPeak,ViReal64 *lastPeak,ViInt32 *peakCount)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (deltaT==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Group Delay");
	if (firstPeak==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"First Peak");
	if (lastPeak==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Last Peak");
	if (peakCount==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Peak Count");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%ld",
		                 deltaT,firstPeak,lastPeak,peakCount));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*deltaT=1E-6*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*firstPeak=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*lastPeak=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1600.0;
		*peakCount=rand() % 4 + 1;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_MpkResults
 * Purpose:  This function retrieves one peak of multipeak WDM analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_MpkResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_MPK));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? WDM,MPK,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%*[^,],%*[^,],%lf,%lf",wavelength,level));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_SNR_Parameters
 * Purpose:  This function retrieves the properties of signal-to-noise WDM
 *           measurements.  "Normalize" parameter ignored on MS9720 series.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_Parameters(ViSession vi,ViInt32 *direction,ViReal64 *dWavelength,ViBoolean *normalize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if (direction==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Direction");
	if (dWavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Delta Wavelength");
	if (normalize==VI_NULL &&
		(osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Normalization");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_SNR_DIR,0,direction));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_SNR_DW,0,dWavelength));
	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_SNR_NORM,0,normalize));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetWDM_SNR_Parameters
 * Purpose:  This function sets the properties of signal-to-noise WDM
 *           measurements.  "Normalize" parameter ignored on MS9720 series.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetWDM_SNR_Parameters(ViSession vi,ViInt32 direction,ViReal64 dWavelength,ViBoolean normalize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_SNR_DIR,0,direction));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_SNR_DW,0,dWavelength));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_SNR_NORM,0,normalize));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_SNR_Results
 * Purpose:  This function retrieves one peak of signal-to-noise WDM
 *           measurements.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_Results(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *SNR,ViInt32 *direction)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViChar dirBuf[10];
	ViInt32 dirBufSize=sizeof(dirBuf)-1;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");
	if (SNR==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"SNR");
	if (direction==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Direction");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_SNR));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? WDM,SNR,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%*[^,],%*[^,],%lf,%lf,%lf,%#s",wavelength,level,SNR,&dirBufSize,dirBuf));
		if (strcmp(dirBuf,"LEFT")==0)
			*direction=MS97XX_VAL_WDM_PEAK_LEFT;
		else if (strcmp(dirBuf,"RIGHT")==0)
			*direction=MS97XX_VAL_WDM_PEAK_RIGHT;
		else
			*direction=MS97XX_VAL_WDM_PEAK_ERROR;
		
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*SNR=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*direction=rand() % 2;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_GainResults
 * Purpose:  This function retrieves one peak of gain tilt WDM
 *           measurements.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_GainResults(ViSession vi,ViReal64 *gainTilt)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (gainTilt==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Gain Tilt");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_SNR));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? WDM,SNR,GAT%s",terminator));
		checkErr(viScanf(io,"%*[^,],%*[^,],%*[^,],%lf",gainTilt));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*gainTilt=((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_RelResults
 * Purpose:  This function retrieves one peak of relative WDM analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_RelResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *spacing,ViReal64 *relWavelength,ViReal64 *level,ViReal64 *relLevel)
{
	ViStatus	error=VI_SUCCESS;
	ViChar		readBuf[BUFFER_SIZE],
				wlBuf[15],lBuf[15];
	ViInt32		readBufSize=sizeof(readBuf)-1,
				wlBufSize=sizeof(wlBuf)-1,lBufSize=sizeof(lBuf)-1;
	ViSession	io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (spacing==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Spacing");
	if (relWavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Relative Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Level");
	if (relLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Relative Level");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_REL));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? WDM,REL,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,readBuf));

		/*
		  Parse the response.  We don't care about the first two
		  entries (they should always be "WDM" and "REL"), so we
		  let them be read into buffers that will be overwritten.
		*/
		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%f,%f,%s[w*t44q],%f,%s[w*q]",
					  wlBufSize,wlBuf,lBufSize,lBuf,
					  wavelength,spacing,wlBufSize,wlBuf,
					  level,lBufSize,lBuf));

/*
  The following line should work instead of the bulky Scan function,
  but a bug in VISA causes viScanf to fail to parse the %#[^,] tag
  in the middle.  Also note that relative wavelength and relative
  level must be scanned in as strings, because sometimes the
  instrument reports their values as "EFR" instead of as numbers.
  		checkErr(viScanf(io,"%*[^,],%*[^,],%lf,%lf,%#[^,],%lf,%#s",
  		                 wavelength,spacing,&wlBufSize,wlBuf,
  		                 level,&lBufSize,lBuf));
*/

		*relWavelength=atof(wlBuf);
		*relLevel=atof(lBuf);
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*spacing=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relWavelength=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*level=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relLevel=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_TableParameters
 * Purpose:  This function retrieves the properties of tabular WDM
 *           measurements.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_TableParameters(ViSession vi,ViInt32 *direction,ViReal64 *dWavelength,ViBoolean *normalize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (direction==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Direction");
	if (dWavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Delta Wavelength");
	if (normalize==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Normalization");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,0,direction));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_TABLE_DW,0,dWavelength));
	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_TABLE_NORM,0,normalize));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetWDM_TableParameters
 * Purpose:  This function sets the properties of tabular WDM
 *           measurements.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetWDM_TableParameters(ViSession vi,ViInt32 direction,ViReal64 dWavelength,ViBoolean normalize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,0,direction));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_TABLE_DW,0,dWavelength));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_TABLE_NORM,0,normalize));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_TableResults
 * Purpose:  This function retrieves one peak of data from the WDM results
 *           table.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_TableResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *freq,ViReal64 *level,ViReal64 *SNR,ViInt32 *direction,ViReal64 *spacing,ViReal64 *freqSpacing)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViChar readBuf[BUFFER_SIZE],dirBuf[10];
	ViInt32 readBufSize=sizeof(readBuf)-1,dirBufSize=sizeof(dirBuf)-1;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (freq==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Frequency");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Level");
	if (SNR==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"SNR");
	if (direction==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Direction");
	if (spacing==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Spacing");
	if (freqSpacing==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Frequency Spacing");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_TBL));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? WDM,TBL,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%f,%f,%f,%f,%s[w*t44q],%f,%f",
					  dirBufSize,dirBuf,dirBufSize,dirBuf,
					  wavelength,freq,level,SNR,
					  dirBufSize,dirBuf,spacing,freqSpacing));

/*
  Cleaner viPrintf format removed due to VISA bug.
  		checkErr(viScanf(io,"%*[^,],%*[^,],%lf,%lf,%lf,%lf,%#[^,],%lf,%lf",
  		                 wavelength,freq,level,SNR,
  		                 &dirBufSize,dirBuf,spacing,freqSpacing));
*/

		if (strcmp(dirBuf,"LEFT")==0)
			*direction=MS97XX_VAL_WDM_PEAK_LEFT;
		else if (strcmp(dirBuf,"RIGHT")==0)
			*direction=MS97XX_VAL_WDM_PEAK_RIGHT;
		else
			*direction=MS97XX_VAL_WDM_PEAK_ERROR;
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
		*freq=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*level=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*SNR=((ViReal64)rand())/((ViReal64)RAND_MAX);
		*direction=rand() % 2;
		*spacing=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*freqSpacing=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_PeakCount
 * Purpose:  This function returns the number of peaks for WDM measurement.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_PeakCount(ViSession vi,ViInt32 *numPeaks)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Number of Peaks");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? MPKC%s",terminator));
		checkErr(viScanf(io,"%*[^,],%ld",numPeaks));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % 5  + 1;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_MpkBulkResults
 * Purpose:  This function retrieves all peaks of multipeak WDM analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_MpkBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[])
{
	ViStatus error=VI_SUCCESS;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*2-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_MPK));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_SNR_BulkResults
 * Purpose:  This function retrieves all peaks of signal-to-noise WDM
 *           measurements.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_BulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 SNRs[],ViInt32 directions[])
{
	ViStatus error=VI_SUCCESS;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");
	if (SNRs==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"SNRs");
	if (directions==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Directions");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_SNR));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			SNRs[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			directions[loop]=
				(strcmp(pToken,"LEFT")==0 ? MS97XX_VAL_WDM_PEAK_LEFT :
					    (strcmp(pToken,"RIGHT")==0 ? MS97XX_VAL_WDM_PEAK_RIGHT :
													 MS97XX_VAL_WDM_PEAK_ERROR));
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			SNRs[loop]=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			directions[loop]=rand() % 2;
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_RelBulkResults
 * Purpose:  This function retrieves all peaks of relative WDM analysis.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_RelBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViInt32 *refPeak,ViReal64 wavelengths[],ViReal64 spacings[],ViReal64 relWavelengths[],ViReal64 levels[],ViReal64 relLevels[])
{
	ViStatus error=VI_SUCCESS;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (spacings==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Spacings");
	if (relWavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Relative Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Levels");
	if (relLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Relative Levels");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_REL));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");

		if (pToken!=NULL)
		{
			pToken=strtok(NULL,",");
			if (pToken!=NULL)
				*refPeak=atol(pToken);
		}

		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			spacings[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relWavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relLevels[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=50.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			spacings[loop]=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relWavelengths[loop]=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			levels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relLevels[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_TableBulkResults
 * Purpose:  This function retrieves all peaks of data from the WDM results
 *           table.  MS9710 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_TableBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 freqs[],ViReal64 levels[],ViReal64 SNRs[],ViInt32 directions[],ViReal64 spacings[],ViReal64 freqSpacings[])
{
	ViStatus error=VI_SUCCESS;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*6-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9710)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (freqs==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Frequencies");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Levels");
	if (SNRs==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"SNRs");
	if (directions==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Directions");
	if (spacings==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Spacings");
	if (freqSpacings==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,10,"Frequency Spacings");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	viCheckErr(MS97xx_CheckWDM_Mode(vi,MS97XX_WDM_MASK_TBL));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			freqs[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			SNRs[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			directions[loop]=
				(strcmp(pToken,"LEFT")==0 ? MS97XX_VAL_WDM_PEAK_LEFT :
					    (strcmp(pToken,"RIGHT")==0 ? MS97XX_VAL_WDM_PEAK_RIGHT :
													 MS97XX_VAL_WDM_PEAK_ERROR));

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			spacings[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			freqSpacings[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=100.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1500.0;
			freqs[loop]=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			levels[loop]=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			SNRs[loop]=((ViReal64)rand())/((ViReal64)RAND_MAX);
			directions[loop]=rand() % 2;
			spacings[loop]=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			freqSpacings[loop]=10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetI_LossResults
 * Purpose:  This function retrieves one peak of insertion loss results.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetI_LossResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *insertionLoss)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");
	if (relLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Relative Level");
	if (insertionLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Insertion Loss");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf",
		                 wavelength,level,relLevel,insertionLoss));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relLevel=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*insertionLoss=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetI_LossBulkResults
 * Purpose:  This function retrieves all peaks of insertion loss results.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetI_LossBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 insertionLosses[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");
	if (relLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Relative Levels");
	if (insertionLosses==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Insertion Losses");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			insertionLosses[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relLevels[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			insertionLosses[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetIsoResults
 * Purpose:  This function retrieves one peak of isolation results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetIsoResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *isolation)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");
	if (relLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Relative Level");
	if (isolation==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Isolation");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ISO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf",
		                 wavelength,level,relLevel,isolation));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relLevel=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*isolation=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetIsoBulkResults
 * Purpose:  This function retrieves all peaks of isolation results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetIsoBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 isolations[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");
	if (relLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Relative Levels");
	if (isolations==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Isolations");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ISO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			isolations[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relLevels[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			isolations[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetDirResults
 * Purpose:  This function retrieves one peak of directivity results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDirResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *directivity)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");
	if (relLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Relative Level");
	if (directivity==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Directivity");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_DIR,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf",
		                 wavelength,level,relLevel,directivity));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relLevel=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*directivity=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetDirBulkResults
 * Purpose:  This function retrieves all peaks of directivity results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDirBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 directivities[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");
	if (relLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Relative Levels");
	if (directivities==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Directivities");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_DIR,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			directivities[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relLevels[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			directivities[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetR_LossResults
 * Purpose:  This function retrieves one peak of return loss results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetR_LossResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *returnLoss)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (level==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");
	if (relLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Relative Level");
	if (returnLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Return Loss");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_RLO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf",
		                 wavelength,level,relLevel,returnLoss));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*level=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*relLevel=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*returnLoss=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetRLossBulkResults
 * Purpose:  This function retrieves all peaks of return loss results.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetR_LossBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 returnLosses[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*4-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (levels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Levels");
	if (relLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Relative Levels");
	if (returnLosses==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Return Losses");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_RLO,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			levels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			relLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			returnLosses[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			levels[loop]=-20.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			relLevels[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			returnLosses[loop]=-5.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetNF_Parameters
 * Purpose:  This function retrieves the properties of noise figure results.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetNF_Parameters(ViSession vi,ViReal64 *fitSpan,ViReal64 *pinLoss,ViReal64 *cal,ViReal64 *aseCal,ViReal64 *poutLoss,ViInt32 *cutLevel)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (fitSpan==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Fitting Span");
	if (pinLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Pin Loss");
	if (cal==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"NF Calibration");
	if (aseCal==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"ASE Calibration");
	if (poutLoss==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Pout Loss");
	if (cutLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Cut Level");

	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_FITTING_SPAN,0,fitSpan));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_PIN_LOSS,0,pinLoss));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_CAL,0,cal));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_ASE_CAL,0,aseCal));
	checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_POUT_LOSS,0,poutLoss));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_NF_CUT_LEVEL,0,cutLevel));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetNF_Parameters
 * Purpose:  This function sets the properties of noise figure analysis.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetNF_Parameters(ViSession vi,ViReal64 fitSpan,ViReal64 pinLoss,ViReal64 cal,ViReal64 aseCal,ViReal64 poutLoss,ViInt32 cutLevel)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_FITTING_SPAN,0,fitSpan));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_PIN_LOSS,0,pinLoss));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_CAL,0,cal));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_ASE_CAL,0,aseCal));
	checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_POUT_LOSS,0,poutLoss));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_NF_CUT_LEVEL,0,cutLevel));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetNF_Results
 * Purpose:  This function retrieves one peak of noise figure results.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetNF_Results(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *NF,ViReal64 *gain,ViReal64 *aseLevel,ViReal64 *pinPeakLevel,ViReal64 *poutPeakLevel)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");
	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
	if (NF==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Noise Figure");
	if (gain==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Gain");
	if (aseLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"ASE Level");
	if (pinPeakLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Pin Peak Level");
	if (poutPeakLevel==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Pout Peak Level");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%lf,%lf,%lf,%lf,%lf,%lf",
		                 wavelength,NF,gain,aseLevel,pinPeakLevel,poutPeakLevel));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
		*NF=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*gain=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*aseLevel=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*pinPeakLevel=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*poutPeakLevel=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
	}
	
	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetNF_BulkResults
 * Purpose:  This function retrieves all peaks of noise figure results.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetNF_BulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 NFs[],ViReal64 gains[],ViReal64 aseLevels[],ViReal64 pinPeakLevels[],ViReal64 poutPeakLevels[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViAddr readBuf=VI_NULL;
	ViInt32 readBufSize=128*15*6-1,loop;
	ViChar *pResponse=VI_NULL,*pToken;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPeaks<0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Peaks");
	if (numPeaks==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Number of Peaks");
	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelengths");
	if (NFs==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Noise Figures");
	if (gains==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Gains");
	if (aseLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"ASE Levels");
	if (pinPeakLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Pin Peak Levels");
	if (poutPeakLevels==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Pout Peak Levels");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		checkErr(MS97xx_GetReadBuffer(vi,readBufSize+1,&readBuf));
		pResponse=(ViChar*)readBuf;
		
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"APR?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,pResponse));

		*numPeaks=atol(pResponse);
		if (maxPeaks<*numPeaks)
			*numPeaks=maxPeaks;

		pToken=strtok(pResponse,",");
		for (loop=0; loop<*numPeaks && pToken!=NULL;++loop)
		{
			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			wavelengths[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			NFs[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			gains[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			aseLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			pinPeakLevels[loop]=atof(pToken);

			pToken=strtok(NULL,",");
			if (pToken==NULL) break;
			poutPeakLevels[loop]=atof(pToken);
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPeaks=rand() % (maxPeaks < 5 ? maxPeaks : 5) + 1;
		for (loop=0; loop<*numPeaks; ++loop)
		{
			wavelengths[loop]=200.0*((ViReal64)rand())/((ViReal64)RAND_MAX)+1450.0;
			NFs[loop]=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			gains[loop]=1.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			aseLevels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			pinPeakLevels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
			poutPeakLevels[loop]=-10.0*((ViReal64)rand())/((ViReal64)RAND_MAX);
		}
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Memory/Trace */

/*****************************************************************************
 * Function: MS97xx_GetTraceParameters
 * Purpose:  This function retrieves information about how the given trace
 *           is displayed.  MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetTraceParameters(ViSession vi,ViInt32 trace,ViBoolean *visible,ViInt32 *screen,ViInt32 *numerator,ViInt32 *denominator)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType,visibleAttr,screenAttr,numerAttr,denomAttr;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (trace<1 || trace>3)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Trace");
	if (visible==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Visible");
	if (screen==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Screen");
	if (numerator==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Numerator");
	if (denominator==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Denominator");

	switch (trace)
	{
		case 2:
			visibleAttr=MS97XX_ATTR_TRACE_2_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_2_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_2_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_2_DENOMINATOR;
		break;

		case 3:
			visibleAttr=MS97XX_ATTR_TRACE_3_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_3_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_3_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_3_DENOMINATOR;
		break;

		default:
			visibleAttr=MS97XX_ATTR_TRACE_1_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_1_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_1_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_1_DENOMINATOR;
		break;
	}

	checkErr(Ivi_GetAttributeViBoolean(vi,"",visibleAttr,0,visible));
	checkErr(Ivi_GetAttributeViInt32(vi,"",screenAttr,0,screen));
	checkErr(Ivi_GetAttributeViInt32(vi,"",numerAttr,0,numerator));
	checkErr(Ivi_GetAttributeViInt32(vi,"",denomAttr,0,denominator));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetTraceParameters
 * Purpose:  This function writes information about how the given trace
 *           is displayed.  MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetTraceParameters(ViSession vi,ViInt32 trace,ViBoolean visible,ViInt32 screen,ViInt32 numerator,ViInt32 denominator)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType,visibleAttr,screenAttr,numerAttr,denomAttr,lockedAttr;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (trace<1 || trace>3)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Trace");

	switch (trace)
	{
		case 2:
			visibleAttr=MS97XX_ATTR_TRACE_2_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_2_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_2_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_2_DENOMINATOR;
			lockedAttr=MS97XX_ATTR_TRACE_2_ATTR_LOCKED;
		break;

		case 3:
			visibleAttr=MS97XX_ATTR_TRACE_3_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_3_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_3_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_3_DENOMINATOR;
			lockedAttr=MS97XX_ATTR_TRACE_3_ATTR_LOCKED;
		break;

		default:
			visibleAttr=MS97XX_ATTR_TRACE_1_VISIBLE;
			screenAttr=MS97XX_ATTR_TRACE_1_SCREEN;
			numerAttr=MS97XX_ATTR_TRACE_1_NUMERATOR;
			denomAttr=MS97XX_ATTR_TRACE_1_DENOMINATOR;
			lockedAttr=MS97XX_ATTR_TRACE_1_ATTR_LOCKED;
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",lockedAttr,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",visibleAttr,0,visible));
	checkErr(Ivi_SetAttributeViInt32(vi,"",screenAttr,0,screen));
	checkErr(Ivi_SetAttributeViInt32(vi,"",numerAttr,0,numerator));
	checkErr(Ivi_SetAttributeViInt32(vi,"",denomAttr,0,denominator));

Error:
	Ivi_SetAttributeViBoolean(vi,"",lockedAttr,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetMemory
 * Purpose:  This function retrieves measured data from one of the
 *           instrument's memory sets.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetMemory(ViSession vi,ViInt32 memory,ViInt32 maxPoints,
								   ViReal64 data[],ViInt32 *numPoints)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType,loop;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

    if (memory < 1 ||
        memory > ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720 ? 3 : 2))
    	viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Memory");

	if (maxPoints < 0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Maximum Number of Points");

	if (data==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Data");

	if (numPoints==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Number of Points");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));

		if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720)
			checkErr(viPrintf(io,"MBD? %c%s",'A'+memory-1,terminator));
		else
			checkErr(viPrintf(io,(memory==1 ? "DBA?%s" : "DBB?%s"),terminator));

		checkErr(MS97xx_ReadBinaryReal64Array(vi,io,maxPoints,data,numPoints));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPoints=(maxPoints<501 ? maxPoints : 501);
		for (loop=0; loop<*numPoints; ++loop)
			data[loop]=((ViReal64)rand())/((ViReal64)RAND_MAX)*-30.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetTrace
 * Purpose:  This function retrieves the results of the specified sweep.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetTrace(ViSession vi,ViInt32 trace,ViInt32 maxPoints,
								  ViReal64 data[],ViInt32 *numPoints)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	ViInt32 loop,bytesRead;
	ViInt16 readVal;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

    if (trace < 1 ||
        trace > ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720 ? 3 : 2))
    	viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Trace");

	if (maxPoints < 0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Maximum Number of Points");

	if (data==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Data");

	if (numPoints==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Number of Points");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));

		if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720)
			checkErr(viPrintf(io,"TBD? %ld%s",trace,terminator));
		else
			checkErr(viPrintf(io,(trace==1 ? "DBA?%s" : "DBB?%s"),terminator));

		checkErr(MS97xx_ReadBinaryReal64Array(vi,io,maxPoints,data,numPoints));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPoints=(maxPoints<501 ? maxPoints : 501);
		for (loop=0; loop<*numPoints; ++loop)
			data[loop]=((ViReal64)rand())/((ViReal64)RAND_MAX)*-30.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetDataConditions
 * Purpose:  This function retrieves the measurement conditions associated
 *           with the given memory set.  Data type "TRACE" not recognized
 *           on MS9710 series.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetDataConditions(ViSession vi,ViInt32 dataType,ViInt32 memory,
										   ViReal64 *startWl,ViReal64 *stopWl,ViInt32 *samplePoints)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

    if (dataType < 0 ||
        dataType > ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720 ? 1 : 0))
    	viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Data Type");

    if (memory < 1 ||
        memory > ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)==MS97XX_VAL_OSA_TYPE_MS9720 ? 3 : 2))
    	viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Memory");

	if (startWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Start Wavelength");

	if (stopWl==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Stop Wavelength");

	if (samplePoints==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Sampling Points");


	if (dataType==MS97XX_VAL_DATA_CONDITIONS_MEMORY)
	{
		if (memory==MS97XX_VAL_MEMORY_A)
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
		else if (memory==MS97XX_VAL_MEMORY_B)
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS |
												  MS97XX_MODE_MASK_WDM,VI_FALSE));
		else
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	}
	else
	{
		if (memory==MS97XX_VAL_MEMORY_A)
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
		else if (memory==MS97XX_VAL_MEMORY_B)
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS |
												  MS97XX_MODE_MASK_WDM |
												  MS97XX_MODE_MASK_PMD,VI_FALSE));
		else
			viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS |
												  MS97XX_MODE_MASK_WDM |
												  MS97XX_MODE_MASK_NF,VI_FALSE));
	}
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		if (dataType==MS97XX_VAL_DATA_CONDITIONS_MEMORY)
			checkErr(viPrintf(io,"DC%c?%s",'A'+memory-1,terminator));
		else
			checkErr(viPrintf(io,"DCT%ld?%s",memory,terminator));
		checkErr(viScanf(io,"%lf,%lf,%ld",startWl,stopWl,samplePoints));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*startWl=1450.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*100.0;
		*stopWl=*startWl+((ViReal64)rand())/((ViReal64)RAND_MAX)*100.0;
		*samplePoints=251;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_Signal
 * Purpose:  This function returns the wavelength of the given channel.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_Signal(ViSession vi,ViInt32 channel,ViReal64 *wavelength)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (channel < 1 || channel > 128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Channel");

	if (wavelength==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Wavelength");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SGW? %ld%s",channel,terminator));
		checkErr(viScanf(io,"%*[^,],%lf",wavelength));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wavelength=1450.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*200.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetWDM_Signal
 * Purpose:  This function writes the wavelength of the given channel.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetWDM_Signal(ViSession vi,ViInt32 channel,ViReal64 wavelength)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (channel < 1 || channel > 128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Channel");

	if (wavelength < 1450.0 || wavelength > 1650.0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SGW %ld,%.2lf%s",channel,wavelength,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetWDM_Signals
 * Purpose:  This function returns all the analysis channels.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetWDM_Signals(ViSession vi,ViInt32 maxPoints,
									   ViReal64 wavelengths[],ViInt32 *numPoints)
{
	ViStatus	error=VI_SUCCESS;
	ViSession	io;
	ViInt32		osaType;
	ViInt32		points,loop;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (maxPoints < 0)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Maximum Number of Points");

	if (wavelengths==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelengths");

	if (numPoints==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Number of Points");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SGWS?%s",terminator));
		points=maxPoints;
		checkErr(viScanf(io,"%ld,%,#lf",numPoints,&points,wavelengths));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*numPoints=(maxPoints<64 ? maxPoints : 64);
		for (*numPoints=0; *numPoints<maxPoints; ++(*numPoints))
			wavelengths[*numPoints]=1450.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*200.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* File/ Print */

/*****************************************************************************
 * Function: MS97xx_GetFileParameters
 * Purpose:  This function retrieves information about how files are stored
 *           on the floppy disk of the OSA.  The "numericFormat" and
 *           "instrumentFormat" parameters are ignored on the MS9710 series.
 *           Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetFileParameters(ViSession vi,ViInt32 *outputFormat,ViInt32 *inputFormat,ViInt32 *numericFormat,ViInt32 *instrumentFormat,ViInt32 *diskSize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (outputFormat==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Output Format");
	if (inputFormat==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Input Format");
	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9720)
	{
		if (numericFormat==VI_NULL)
			viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Numeric Format");
		if (instrumentFormat==VI_NULL)
			viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Instrument Format");
	}
	if (diskSize==VI_NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Disk Size");

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_OPTION,0,outputFormat));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,0,inputFormat));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9720)
	{
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,0,numericFormat));
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,0,instrumentFormat));
	}
	
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DISKETTE_SIZE,0,diskSize));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetFileParameters
 * Purpose:  This function retrieves information about how files are stored
 *           on the floppy disk of the OSA.  The "numericFormat" and
 *           "instrumentFormat" parameters are ignored on the MS9710 series.
 *           Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetFileParameters(ViSession vi,ViInt32 outputFormat,ViInt32 inputFormat,ViInt32 numericFormat,ViInt32 instrumentFormat,ViInt32 diskSize)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
		viCheckErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_LOCKED,0,VI_TRUE));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_OPTION,0,outputFormat));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,0,inputFormat));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9720)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,0,numericFormat));
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,0,instrumentFormat));
	}

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DISKETTE_SIZE,0,diskSize));

Error:
	Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_LOCKED,0,VI_FALSE);
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Save
 * Purpose:  This function saves the measurement data in the specified file.
 *           Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Save(ViSession vi,ViChar fileName[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
    	
    viCheckParm(MS97xx_CheckFileName(vi,fileName),2,"File Name");

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"SAV %s%s",fileName,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Recall
 * Purpose:  This function loads measurement data from the specified file.
 *           Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Recall(ViSession vi,ViChar fileName[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

    viCheckParm(MS97xx_CheckFileName(vi,fileName),2,"File Name");

	checkErr(Ivi_InvalidateAllAttributes(vi));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD) == 0)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ATTENUATOR_ON));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"RCL %s%s",fileName,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SaveConditions
 * Purpose:  This function causes the instrument to save the measurement
 *           conditions into the specified page of internal backup RAM.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SaveConditions(ViSession vi,ViInt32 backupMemory)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (backupMemory < 1 || backupMemory > 5)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Backup Memory");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"CSAV %ld%s",backupMemory,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_RecallConditions
 * Purpose:  This function causes the instrument to load the measurement
 *           conditions stored in the specified page of internal backup RAM.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_RecallConditions(ViSession vi,ViInt32 backupMemory)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (backupMemory < 1 || backupMemory > 5)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Backup Memory");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_MEASURE_MODE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ANALYSIS_MODE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_MODE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_DISPLAY_MODE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_RESOLUTION));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_POINT_AVERAGE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_VIDEO_BAND_WIDTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_SAMPLING_POINTS));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_REF_LEVEL));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_LOG_SCALE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD) == 0)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ATTENUATOR_ON));
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"CRCL %ld%s",backupMemory,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_RecallMemoryConditions
 * Purpose:  This function sets the active screen's measurement parameters
 *           equal to those used for the given memory block.
 *           MS9720 series only.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_RecallMemoryConditions(ViSession vi,ViInt32 memory)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0 ||
		(osaType & MS97XX_VAL_OSA_TYPE_SERIES)!=MS97XX_VAL_OSA_TYPE_MS9720)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (memory < 1 || memory > 3)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Memory");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_START_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STOP_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_CENTER_WAVELENGTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WAVELENGTH_SPAN));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_RESOLUTION));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_POINT_AVERAGE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_VIDEO_BAND_WIDTH));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_SAMPLING_POINTS));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_REF_LEVEL));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_LOG_SCALE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
		checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_ATTENUATOR_ON));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"MCRCL %c%s",'A'+memory-1,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_DeleteFile
 * Purpose:  This function causes the instrument to remove the specified
 *           file from the floppy disk.  Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_DeleteFile(ViSession vi,ViChar fileName[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

    viCheckParm(MS97xx_CheckFileName(vi,fileName),2,"File Name");
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"DEL %s%s",fileName,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Format
 * Purpose:  This function causes the instrument to format the floppy disk.
 *           Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Format(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;
	ViUInt32 timeoutMs;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));

		viGetAttribute(io,VI_ATTR_TMO_VALUE,&timeoutMs);
		viSetAttribute(io,VI_ATTR_TMO_VALUE,300000);

		checkErr(viPrintf(io,"FMT%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

	viSetAttribute(io,VI_ATTR_TMO_VALUE,timeoutMs);

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Print
 * Purpose:  This function causes the instrument's built-in printer
 *           to print the screen.  Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Print(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"CPY%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_Feed
 * Purpose:  This function feeds the printer paper by the given number of
 *           line.  Not valid for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_Feed(ViSession vi,ViInt32 lines)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_CARD)!=0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (lines<0 || lines>25)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Number of lines");
	
	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"FED %ld%s",lines,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Long-Term */

/*****************************************************************************
 * Function: MS97xx_StartLongTermTest
 * Purpose:  This function begins logging data to the floppy disk.
 *           Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_StartLongTermTest(ViSession vi,ViChar fileName[])
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

    viCheckParm(MS97xx_CheckFileName(vi,fileName),2,"File Name");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"TSA %s%s",fileName,terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_StopLongTermTest
 * Purpose:  This function stops logging data to the floppy disk.
 *           Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_StopLongTermTest(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
		(osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"TSO%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermPeak
 * Purpose:  This function retrieves long-term measurement data for the
 *           given peak.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermPeak(ViSession vi,ViInt32 peak,ViReal64 *meanWave,ViReal64 *maxWave,ViReal64 *minWave,ViReal64 *meanLevel,ViReal64 *maxLevel,ViReal64 *minLevel,ViReal64 *meanSNR,ViReal64 *maxSNR,ViReal64 *minSNR)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
		(osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");

	if (meanWave==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Mean Wavelength");
		
	if (maxWave==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Maximum Wavelength");

	if (minWave==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Minimum Wavelength");

	if (meanLevel==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,6,"Mean Level");

	if (maxLevel==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,7,"Maximum Level");

	if (minLevel==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,8,"Minimum Level");

	if (meanSNR==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,9,"Mean SNR");

	if (maxSNR==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,10,"Maximum SNR");

	if (minSNR==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,11,"Minimum SNR");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? PKD,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%*[^,],%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
						 meanWave,maxWave,minWave,
						 meanLevel,maxLevel,minLevel,
						 meanSNR,maxSNR,minSNR));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*maxWave=1550.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*100.0;
		*minWave=1450.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*100.0;
		*meanWave=(*maxWave+*minWave)/2.0;
		*maxLevel=((ViReal64)rand())/((ViReal64)RAND_MAX)*-40.0;
		*minLevel=-40.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*-40.0;
		*meanLevel=(*maxLevel+*minLevel)/2.0;
		*maxSNR=0.5+((ViReal64)rand())/((ViReal64)RAND_MAX)*0.5;
		*minSNR=((ViReal64)rand())/((ViReal64)RAND_MAX)*0.5;
		*meanSNR=(*maxSNR+*minSNR)/2.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermStartPeak
 * Purpose:  This function retrieves measurement data for the given starting
 *           peak.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermStartPeak(ViSession vi,ViInt32 peak,ViReal64 *wave,ViReal64 *level,ViReal64 *SNR)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");

	if (wave==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Wavelength");
		
	if (level==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Level");

	if (SNR==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"SNR");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? SPKD,%ld%s",peak,terminator));
		checkErr(viScanf(io,"%*[^,]",wave,level,SNR));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*wave=1450.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*200.0;
		*level=((ViReal64)rand())/((ViReal64)RAND_MAX)*-40.0;
		*SNR=0.5+((ViReal64)rand())/((ViReal64)RAND_MAX)*0.5;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermPower
 * Purpose:  This function retrieves total power data.
 *           Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermPower(ViSession vi,ViReal64 *meanPower,ViReal64 *maxPower,ViReal64 *minPower)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);

	if (meanPower==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Mean Power");

	if (maxPower==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Maximum Power");

	if (minPower==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Minimum Power");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? TPWD%s",terminator));
		checkErr(viScanf(io,"%*[^,],%lf,%lf,%lf",meanPower,maxPower,minPower));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*maxPower=((ViReal64)rand())/((ViReal64)RAND_MAX)*-10.0;
		*minPower=-10.0+((ViReal64)rand())/((ViReal64)RAND_MAX)*-10.0;
		*meanPower=(*maxPower+*minPower)/2.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermGainTilt
 * Purpose:  This function retrieves statistical information about
 *           gain tilt.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermGainTilt(ViSession vi,ViReal64 *meanTilt,ViReal64 *maxTilt,ViReal64 *minTilt)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (meanTilt==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Mean Tilt");

	if (maxTilt==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Maximum Tilt");

	if (minTilt==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Minimum Tilt");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? GAT%s",terminator));
		checkErr(viScanf(io,"%*[^,],%lf,%lf,%lf",meanTilt,maxTilt,minTilt));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*maxTilt=0.2*((ViReal64)rand())/((ViReal64)RAND_MAX)+0.4;
		*minTilt=0.2*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*meanTilt=(*maxTilt+*minTilt)/2.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermSlope
 * Purpose:  This function retrieves statistical information about slope.
 *           Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermSlope(ViSession vi,ViReal64 *meanSlope,ViReal64 *maxSlope,ViReal64 *minSlope)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (meanSlope==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Mean Slope");

	if (maxSlope==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Maximum Slope");

	if (minSlope==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Minimum Slope");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? SLP%s",terminator));
		checkErr(viScanf(io,"%*[^,],%lf,%lf,%lf",meanSlope,maxSlope,minSlope));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*maxSlope=0.2*((ViReal64)rand())/((ViReal64)RAND_MAX)+0.4;
		*minSlope=0.2*((ViReal64)rand())/((ViReal64)RAND_MAX);
		*meanSlope=(*maxSlope+*minSlope)/2.0;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermTime
 * Purpose:  This function retrieves the start and stop times of the
 *           long-term test.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermTime(ViSession vi,ViInt32 *startHour,ViInt32 *startMinute,ViInt32 *stopHour,ViInt32 *stopMinute)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (startHour==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Start Hour");

	if (startMinute==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Start Minute");

	if (stopHour==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,4,"Stop Hour");

	if (stopMinute==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,5,"Stop Minute");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"LTSD? TIM%s",terminator));
		checkErr(viScanf(io,"%*[^,],%ld,%ld,%ld,%ld",startHour,startMinute,stopHour,stopMinute));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*startHour=rand() % 24;
		*startMinute=rand() % 60;
		*stopHour=rand() % 24;
		*stopMinute=rand() % 60;
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetLongTermPeakMark
 * Purpose:  This function retrieves whether or not a peak is marked for
 *           further analysis.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetLongTermPeakMark(ViSession vi,ViInt32 peak,ViBoolean *isMarked)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViChar markBuf[10];
	ViInt32 markBufSize=sizeof(markBuf)-1;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");

	if (isMarked==NULL)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,3,"Peak Marking State");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"PMK? %ld%s",peak,terminator));
		checkErr(viScanf(io,"%#s",&markBufSize,markBuf));
		*isMarked=(strcmp(markBuf,"ON")==0 ? VI_TRUE : VI_FALSE);
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
		*isMarked=(rand() % 2 == 1 ? VI_TRUE : VI_FALSE);
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_SetLongTermPeakMark
 * Purpose:  This function sets whether or not a peak is marked for
 *           further analysis.  Valid only for MS9715 and MS9720, not for card OSAs.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetLongTermPeakMark(ViSession vi,ViInt32 peak,ViBoolean mark)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViChar markBuf[10];
	ViInt32 markBufSize=sizeof(markBuf)-1;
	ViInt32 osaType;

	checkErr(Ivi_LockSession(vi,VI_NULL));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) != MS97XX_VAL_OSA_TYPE_MS9720 &&
	    (osaType & MS97XX_VAL_OSA_TYPE_OSA) != MS97XX_VAL_OSA_TYPE_MS9715 ||
	    (osaType & MS97XX_VAL_OSA_TYPE_CARD) != 0)
    	checkErr(IVI_ERROR_FUNCTION_NOT_SUPPORTED);
	
	if (peak<1 || peak>128)
		viCheckParm(IVI_ERROR_INVALID_PARAMETER,2,"Peak");

	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"PMK %ld,%s%s",
						  peak,(mark==VI_TRUE ? "ON" : "OFF"),terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* GPIB */

/*****************************************************************************
 * Function: MS97xx_ClearStatus
 * Purpose:  This function resets the status registers.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_ClearStatus(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_STATUS_BYTE));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_EVENT_STATUS));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_EXTENDED_STATUS_1));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_EXTENDED_STATUS_2));
	checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_EXTENDED_STATUS_3));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"*CLS%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_WaitUntilOperationComplete
 * Purpose:  This function pauses until the instrument is done with the
 *           current operation, or until the specified time has elapsed.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_WaitUntilOperationComplete(ViSession vi,ViInt32 timeoutSeconds)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;
	ViInt32 status,seconds,enable2,enable3,srqEnable;
	time_t start,now;

	checkErr(Ivi_LockSession(vi,VI_NULL));

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE,0,&enable2));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE,0,&enable3));
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_SERVICE_REQUEST_ENABLE,0,&srqEnable));

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE,0,31));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE,0,3));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_SERVICE_REQUEST_ENABLE,0,6));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		time(&start);
		do
		{
			checkErr(viPrintf(io,"*STB?%s",terminator));
			checkErr(viScanf(io,"%ld",&status));
			time(&now);
			seconds=difftime(now,start);
			Delay(0.2);
		} while (status==0 && seconds<timeoutSeconds);

		checkErr(viPrintf(io,"*CLS%s",terminator));

		if (status==0)
		{
			Ivi_UnlockSession(vi,VI_NULL);
			return MS97XX_WARN_OPERATION_TIMEOUT;
		}
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE,0,enable2));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE,0,enable3));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_SERVICE_REQUEST_ENABLE,0,srqEnable));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/* Display */

/*****************************************************************************
 * Function: MS97xx_ClearGraph
 * Purpose:  This function clears the displayed measurement data.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_ClearGraph(ViSession vi)
{
	ViStatus error=VI_SUCCESS;
	ViSession io;

	checkErr(Ivi_LockSession(vi,VI_NULL));
	
	viCheckErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	if (Ivi_Simulating(vi)==VI_FALSE)
	{
		io=Ivi_IOSession(vi);
		checkErr(Ivi_SetNeedToCheckStatus(vi,VI_TRUE));
		checkErr(viPrintf(io,"GCL%s",terminator));
	}
	else if (Ivi_UseSpecificSimulation(vi)==VI_TRUE)
	{
	}

	checkErr(MS97xx_CheckStatus(vi));

Error:
	Ivi_UnlockSession(vi,VI_NULL);
	return error;
}

/*****************************************************************************
 * Function: MS97xx_init   
 * Purpose:  VXIplug&play required function.  Calls the   
 *           MS97xx_InitWithOptions function.   
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_init (ViRsrc resourceName, ViBoolean IDQuery,
                                 ViBoolean resetDevice, ViSession *newVi)
{   
    ViStatus    error = VI_SUCCESS;

    if (newVi == VI_NULL)
        {
        Ivi_SetErrorInfo (VI_NULL, VI_FALSE, IVI_ERROR_INVALID_PARAMETER, 
                          VI_ERROR_PARAMETER4, "Null address for Instrument Handle");
        checkErr( IVI_ERROR_INVALID_PARAMETER);
        }

    checkErr( MS97xx_InitWithOptions (resourceName, IDQuery, resetDevice, 
                                        "", newVi));

Error:  
    return error;
}

/*****************************************************************************
 * Function: MS97xx_InitWithOptions                                       
 * Purpose:  This function creates a new IVI session and calls the 
 *           IviInit function.                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_InitWithOptions (ViRsrc resourceName, ViBoolean IDQuery,
                                            ViBoolean resetDevice, ViString optionString, 
                                            ViSession *newVi)
{   
    ViStatus    error = VI_SUCCESS;
    ViSession   vi = VI_NULL;
    
    if (newVi == VI_NULL)
        {
        Ivi_SetErrorInfo (VI_NULL, VI_FALSE, IVI_ERROR_INVALID_PARAMETER, 
                          VI_ERROR_PARAMETER5, "Null address for Instrument Handle");
        checkErr( IVI_ERROR_INVALID_PARAMETER);
        }

    *newVi = VI_NULL;
    
        /* create a new interchangeable driver */
    checkErr( Ivi_SpecificDriverNew ("MS97xx", optionString, &vi));  
    
        /* init the driver */
    checkErr( MS97xx_IviInit (resourceName, IDQuery, resetDevice, vi)); 
    *newVi = vi;
    
Error:
    if (error < VI_SUCCESS) 
        Ivi_Dispose (vi);
        
    return error;
}

/*****************************************************************************
 * Function: MS97xx_IviInit                                                       
 * Purpose:  This function is called by MS97xx_InitWithOptions  
 *           or by an IVI class driver.  This function initializes the I/O 
 *           interface, optionally resets the device, optionally performs an
 *           ID query, and sends a default setup to the instrument.                
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_IviInit (ViRsrc resourceName, ViBoolean IDQuery,
                                    ViBoolean reset, ViSession vi)
{
    ViStatus    error = VI_SUCCESS;
    ViSession   io = VI_NULL;
    ViInt32		osaType;
    
    checkErr( Ivi_BuildChannelTable (vi, CHANNEL_LIST, VI_FALSE, VI_NULL));
    
     /* Add attributes */
    checkErr( MS97xx_InitAttributes (vi));

    if (!Ivi_Simulating(vi))
        {
        ViSession   rmSession = VI_NULL;

        /* Open instrument session */
        checkErr( Ivi_GetAttributeViSession (vi, VI_NULL, IVI_ATTR_VISA_RM_SESSION, 0,
                                             &rmSession)); 
        viCheckErr( viOpen (rmSession, resourceName, VI_NULL, VI_NULL, &io));
        /* io session owned by driver now */
        checkErr( Ivi_SetAttributeViSession (vi, VI_NULL, IVI_ATTR_IO_SESSION, 0, io));  

		/* Configure VISA Formatted I/O */
		viCheckErr( viSetAttribute (io, VI_ATTR_TMO_VALUE, 5000 ));
		viCheckErr( viSetBuf (io, VI_READ_BUF | VI_WRITE_BUF, 4000));
		viCheckErr( viSetAttribute (io, VI_ATTR_WR_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS));
		viCheckErr( viSetAttribute (io, VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS));
        }
        
    /*- Reset instrument ----------------------------------------------------*/
    if (reset) 
        checkErr( MS97xx_reset (vi));
    else  /*- Send Default Instrument Setup ---------------------------------*/
        checkErr( MS97xx_DefaultInstrSetup (vi));
	
	/*- Identification Query ------------------------------------------------*/
	if (IDQuery)                               
	    {
	    ViChar rdBuffer[BUFFER_SIZE];
	
	    #define VALID_RESPONSE_STRING_START        "ANRITSU,MS97"
	
	    checkErr( Ivi_GetAttributeViString (vi, VI_NULL, MS97XX_ATTR_ID_QUERY_RESPONSE, 
	                                        0, BUFFER_SIZE, rdBuffer));
	    
	    if (strncmp (rdBuffer, VALID_RESPONSE_STRING_START, 
	                 strlen(VALID_RESPONSE_STRING_START)) != 0)
	        {             
	        viCheckErr( VI_ERROR_FAIL_ID_QUERY);
	        }
	    }


//	checkErr( Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SEND_HEADER,0,VI_FALSE) );

	/* Selectively enable or disable model-specific attributes. */
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

    checkErr( MS97xx_CheckStatus (vi));

Error:
    if (error < VI_SUCCESS)
        {
        if (!Ivi_Simulating (vi) && io)
            viClose (io);
        }
    return error;
}

/*****************************************************************************
 * Function: MS97xx_close                                                           
 * Purpose:  This function closes the instrument.                            
 *
 *           Note:  This function must unlock the session before calling
 *           Ivi_Dispose.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_close (ViSession vi)
{
    ViStatus    error = VI_SUCCESS;
    
    checkErr( Ivi_LockSession (vi, VI_NULL));
    
    checkErr( MS97xx_IviClose (vi));

Error:    
    Ivi_UnlockSession (vi, VI_NULL);
    Ivi_Dispose (vi);  

    return error;
}

/*****************************************************************************
 * Function: MS97xx_IviClose                                                        
 * Purpose:  This function performs all of the drivers clean-up operations   
 *           except for closing the IVI session.  This function is called by 
 *           MS97xx_close or by an IVI class driver. 
 *
 *           Note:  This function must close the I/O session and set 
 *           IVI_ATTR_IO_SESSION to 0.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_IviClose (ViSession vi)
{
    ViStatus error = VI_SUCCESS;
    ViSession io = VI_NULL;

        /* Do not lock here.  The caller manages the lock. */

    checkErr( Ivi_GetAttributeViSession (vi, VI_NULL, IVI_ATTR_IO_SESSION, 0, &io));

Error:
    Ivi_SetAttributeViSession (vi, VI_NULL, IVI_ATTR_IO_SESSION, 0, VI_NULL);
    if(io)                                                      
        {
        viClose (io);
        }
    return error;   
}

/*****************************************************************************
 * Function: MS97xx_reset                                                         
 * Purpose:  This function resets the instrument.                          
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_reset (ViSession vi)
{
    ViStatus    error = VI_SUCCESS;

    checkErr( Ivi_LockSession (vi, VI_NULL));

	if (!Ivi_Simulating(vi))                /* call only when locked */
	    {
	    ViSession   io = Ivi_IOSession(vi); /* call only when locked */
	    
	    checkErr( Ivi_SetNeedToCheckStatus (vi, VI_TRUE));
	    viCheckErr( viPrintf (io, "*RST"));
	    }
	
	checkErr( MS97xx_DefaultInstrSetup (vi));                                
	
	checkErr( MS97xx_CheckStatus (vi));                                      

Error:
    Ivi_UnlockSession (vi, VI_NULL);
    return error;
}

/*****************************************************************************
 * Function: MS97xx_self_test                                                       
 * Purpose:  This function executes the instrument self-test and returns the 
 *           result.                                                         
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_self_test (ViSession vi, ViInt16 *testResult, 
                                      ViChar testMessage[])
{
    ViStatus    error = VI_SUCCESS;

    checkErr( Ivi_LockSession (vi, VI_NULL));

    if (testResult == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 2, "Null address for Test Result");
    if (testMessage == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 3, "Null address for Test Message");

	if (!Ivi_Simulating(vi))                /* call only when locked */
	    {
	    ViSession   io = Ivi_IOSession(vi); /* call only when locked */
	
	    checkErr( Ivi_SetNeedToCheckStatus (vi, VI_TRUE));
	    viCheckErr( viPrintf (io, "*TST?"));
	        
		viCheckErr( viScanf (io, "%hd", testResult));
		
		testMessage[0] = 0;
	
	    }
	else if (Ivi_UseSpecificSimulation(vi)) /* call only when locked */
	    {
	        /* Simulate Self Test */
	    *testResult = 0;
	    strcpy (testMessage, "No error.");
	    }
	
	checkErr( MS97xx_CheckStatus (vi));

Error:
    Ivi_UnlockSession(vi, VI_NULL);
    return error;
}

/*****************************************************************************
 * Function: MS97xx_error_query                                                     
 * Purpose:  This function queries the instrument error queue and returns   
 *           the result.                                                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_error_query (ViSession vi, ViInt32 *errCode, 
                                        ViChar errMessage[])
{
    ViStatus    error = VI_SUCCESS;
    
    checkErr( Ivi_LockSession (vi, VI_NULL));
    
    if (errCode == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 2, "Null address for Error Code");
    if (errMessage == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 3, "Null address for Error Message");

	viCheckWarn( VI_WARN_NSUP_ERROR_QUERY);

Error:
    Ivi_UnlockSession(vi, VI_NULL);
    return error;
}

/*****************************************************************************
 * Function: MS97xx_error_message                                                  
 * Purpose:  This function translates the error codes returned by this       
 *           instrument driver into user-readable strings.  
 *
 *           Note:  The caller can pass VI_NULL for the vi parameter.  This 
 *           is useful if one of the init functions fail.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_error_message (ViSession vi, ViStatus errorCode,
                                          ViChar errorMessage[256])
{
    ViStatus    error = VI_SUCCESS;
    
    static      IviStringValueTable errorTable = 
        {
        	{MS97XX_ERROR_INVALID_FILE_NAME,		"The specified string cannot be used as a filename for this instrument."},
			{MS97XX_ERROR_INCORRECT_MODE,			"The instrument is not in the right mode for this command."},
			{MS97XX_ERROR_MARKER_NOT_ON,			"The marker must be enabled before this command can be used."},
			{MS97XX_WARN_OPERATION_TIMEOUT,			"The operation did not complete within the allotted time."},
			{VI_NULL,                               VI_NULL}
        };
        
    if (vi)
        Ivi_LockSession(vi, VI_NULL);

        /* all VISA and IVI error codes are handled as well as codes in the table */
    if (errorMessage == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 3, "Null address for Error Message");

    checkErr( Ivi_GetSpecificDriverStatusDesc(vi, errorCode, errorMessage, errorTable));

Error:
    if (vi)
        Ivi_UnlockSession(vi, VI_NULL);
    return error;
}

/*****************************************************************************
 * Function: MS97xx_revision_query                                                  
 * Purpose:  This function returns the driver and instrument revisions.      
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_revision_query (ViSession vi, ViChar driverRev[], 
                                           ViChar instrRev[])
{
    ViChar      rdBuffer[BUFFER_SIZE];
    ViStatus    error = VI_SUCCESS;
    
    checkErr( Ivi_LockSession (vi, VI_NULL));

    if (driverRev == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 2, "Null address for Driver Revision");
    if (instrRev == VI_NULL)
        viCheckParm( IVI_ERROR_INVALID_PARAMETER, 3, "Null address for Instrument Revision");

    checkErr( Ivi_GetAttributeViString (vi, VI_NULL, IVI_ATTR_DRIVER_REVISION, 
                                        0, -1, driverRev));

	if (!Ivi_Simulating(vi))                /* call only when locked */
	    {
	    ViSession   io = Ivi_IOSession(vi); /* call only when locked */
	
	    checkErr( Ivi_SetNeedToCheckStatus (vi, VI_TRUE));
	    viCheckErr( viPrintf (io, "*IDN?"));
	
		viCheckErr( viScanf (io, "%*[^,],%*[^,],%*[^,],%256[^\n]", instrRev));
	    }
	else if (Ivi_UseSpecificSimulation(vi)) /* call only when locked */
	    {
	        /* Simulate Instrument Revision Query */
	    strcpy (instrRev, "No revision information available while simulating.");
	    }

    checkErr( MS97xx_CheckStatus (vi));

Error:    
    Ivi_UnlockSession(vi, VI_NULL);
    return error;
}

/*****************************************************************************
 * Function: MS97xx_GetAttribute<type> Functions                                    
 * Purpose:  These functions enable the instrument driver user to get 
 *           attribute values directly.  There are typesafe versions for 
 *           ViInt32, ViReal64, ViString, ViBoolean, and ViSession attributes.                                         
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetAttributeViInt32 (ViSession vi, ViConstString channelName, 
                                                ViAttr attributeId, ViInt32 *value)
{                                                                                                           
    return Ivi_GetAttributeViInt32 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                    value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_GetAttributeViReal64 (ViSession vi, ViConstString channelName, 
                                                 ViAttr attributeId, ViReal64 *value)
{                                                                                                           
    return Ivi_GetAttributeViReal64 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                     value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_GetAttributeViString (ViSession vi, ViConstString channelName, 
                                                 ViAttr attributeId, ViInt32 bufSize, 
                                                 ViChar value[]) 
{   
    return Ivi_GetAttributeViString (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                     bufSize, value);
}   
ViStatus _VI_FUNC MS97xx_GetAttributeViBoolean (ViSession vi, ViConstString channelName, 
                                                  ViAttr attributeId, ViBoolean *value)
{                                                                                                           
    return Ivi_GetAttributeViBoolean (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                      value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_GetAttributeViSession (ViSession vi, ViConstString channelName, 
                                                  ViAttr attributeId, ViSession *value)
{                                                                                                           
    return Ivi_GetAttributeViSession (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                      value);
}                                                                                                           

/*****************************************************************************
 * Function: MS97xx_SetAttribute<type> Functions                                    
 * Purpose:  These functions enable the instrument driver user to set 
 *           attribute values directly.  There are typesafe versions for 
 *           ViInt32, ViReal64, ViString, ViBoolean, and ViSession datatypes.                                         
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_SetAttributeViInt32 (ViSession vi, ViConstString channelName, 
                                                ViAttr attributeId, ViInt32 value)
{                                                                                                           
    return Ivi_SetAttributeViInt32 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                    value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_SetAttributeViReal64 (ViSession vi, ViConstString channelName, 
                                                 ViAttr attributeId, ViReal64 value)
{                                                                                                           
    return Ivi_SetAttributeViReal64 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                     value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_SetAttributeViString (ViSession vi, ViConstString channelName, 
                                                 ViAttr attributeId, ViConstString value) 
{   
    return Ivi_SetAttributeViString (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                     value);
}   
ViStatus _VI_FUNC MS97xx_SetAttributeViBoolean (ViSession vi, ViConstString channelName, 
                                                  ViAttr attributeId, ViBoolean value)
{                                                                                                           
    return Ivi_SetAttributeViBoolean (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                      value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_SetAttributeViSession (ViSession vi, ViConstString channelName, 
                                                  ViAttr attributeId, ViSession value)
{                                                                                                           
    return Ivi_SetAttributeViSession (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                      value);
}                                                                                                           

/*****************************************************************************
 * Function: MS97xx_CheckAttribute<type> Functions                                  
 * Purpose:  These functions enable the instrument driver user to check  
 *           attribute values directly.  These functions check the value you
 *           specify even if you set the MS97XX_ATTR_RANGE_CHECK 
 *           attribute to VI_FALSE.  There are typesafe versions for ViInt32, 
 *           ViReal64, ViString, ViBoolean, and ViSession datatypes.                         
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_CheckAttributeViInt32 (ViSession vi, ViConstString channelName, 
                                                  ViAttr attributeId, ViInt32 value)
{                                                                                                           
    return Ivi_CheckAttributeViInt32 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                      value);
}
ViStatus _VI_FUNC MS97xx_CheckAttributeViReal64 (ViSession vi, ViConstString channelName, 
                                                   ViAttr attributeId, ViReal64 value)
{                                                                                                           
    return Ivi_CheckAttributeViReal64 (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                       value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_CheckAttributeViString (ViSession vi, ViConstString channelName, 
                                                   ViAttr attributeId, ViConstString value)  
{   
    return Ivi_CheckAttributeViString (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                       value);
}   
ViStatus _VI_FUNC MS97xx_CheckAttributeViBoolean (ViSession vi, ViConstString channelName, 
                                                    ViAttr attributeId, ViBoolean value)
{                                                                                                           
    return Ivi_CheckAttributeViBoolean (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                        value);
}                                                                                                           
ViStatus _VI_FUNC MS97xx_CheckAttributeViSession (ViSession vi, ViConstString channelName, 
                                                    ViAttr attributeId, ViSession value)
{                                                                                                           
    return Ivi_CheckAttributeViSession (vi, channelName, attributeId, IVI_VAL_DIRECT_USER_CALL, 
                                        value);
}                                                                                                           

/*****************************************************************************
 * Function: MS97xx_LockSession and MS97xx_UnlockSession Functions                        
 * Purpose:  These functions enable the instrument driver user to lock the 
 *           session around a sequence of driver calls during which other
 *           execution threads must not disturb the instrument state.
 *                                                                          
 *           NOTE:  The callerHasLock parameter must be a local variable 
 *           initialized to VI_FALSE and passed by reference, or you can pass 
 *           VI_NULL.                     
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_LockSession (ViSession vi, ViBoolean *callerHasLock)  
{                                              
    return Ivi_LockSession(vi,callerHasLock);      
}                                              
ViStatus _VI_FUNC MS97xx_UnlockSession (ViSession vi, ViBoolean *callerHasLock) 
{                                              
    return Ivi_UnlockSession(vi,callerHasLock);    
}   

/*****************************************************************************
 * Function: MS97xx_GetErrorInfo and MS97xx_ClearErrorInfo Functions                       
 * Purpose:  These functions enable the instrument driver user to  
 *           get or clear the error information the driver associates with the
 *           IVI session.                                                        
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_GetErrorInfo (ViSession vi, ViStatus *primaryError, 
                                         ViStatus *secondaryError, ViChar errorElaboration[256])  
{                                                                                                           
    return Ivi_GetErrorInfo(vi, primaryError, secondaryError, errorElaboration);                                
}                                                                                                           
ViStatus _VI_FUNC MS97xx_ClearErrorInfo (ViSession vi)                                                        
{                                                                                                           
    return Ivi_ClearErrorInfo (vi);                                                                             
}

/*****************************************************************************
 * Function: WriteInstrData and ReadInstrData Functions                      
 * Purpose:  These functions enable the instrument driver user to  
 *           write and read commands directly to and from the instrument.            
 *                                                                           
 *           Note:  These functions bypass the IVI attribute state caching.  
 *                  WriteInstrData invalidates the cached values for all 
 *                  attributes.
 *****************************************************************************/
ViStatus _VI_FUNC MS97xx_WriteInstrData (ViSession vi, ViConstString writeBuffer)   
{   
    return Ivi_WriteInstrData (vi, writeBuffer);    
}   
ViStatus _VI_FUNC MS97xx_ReadInstrData (ViSession vi, ViInt32 numBytes, 
                                          ViChar rdBuf[], ViInt32 *bytesRead)  
{   
    return Ivi_ReadInstrData (vi, numBytes, rdBuf, bytesRead);   
} 

/*****************************************************************************
 *-------------------- Utility Functions (Not Exported) ---------------------*
 *****************************************************************************/

/*****************************************************************************
 * Function: MS97xx_CheckStatus                                                 
 * Purpose:  This function checks the status of the instrument to detect 
 *           whether the instrument has encountered an error.  This function  
 *           is called at the end of most exported driver functions.  If the    
 *           instrument reports an error, this function returns      
 *           IVI_ERROR_INSTR_SPECIFIC.  The user can set the 
 *           IVI_ATTR_QUERY_INSTR_STATUS attribute to VI_FALSE to disable this 
 *           check and increase execution speed.                                   
 *
 *           Note:  Call this function only when the session is locked.
 *****************************************************************************/
static ViStatus MS97xx_CheckStatus (ViSession vi)
{
    ViStatus    error = VI_SUCCESS;

    if (Ivi_QueryInstrStatus (vi) && Ivi_NeedToCheckStatus (vi) && !Ivi_Simulating (vi))
        {
        checkErr( MS97xx_CheckStatusCallback (vi, Ivi_IOSession(vi)));
        checkErr( Ivi_SetNeedToCheckStatus (vi, VI_FALSE));
        }
        
Error:
    return error;
}

/*****************************************************************************
 * Function: MS97xx_DefaultInstrSetup                                               
 * Purpose:  This function sends a default setup to the instrument.  The    
 *           MS97xx_reset function calls this function.  The 
 *           MS97xx_IviInit function calls this function when the
 *           user passes VI_FALSE for the reset parameter.  This function is 
 *           useful for configuring settings that other instrument driver 
 *           functions require.    
 *
 *           Note:  Call this function only when the session is locked.
 *****************************************************************************/
static ViStatus MS97xx_DefaultInstrSetup (ViSession vi)
{
    ViStatus    error = VI_SUCCESS;
        
    /* Invalidate all attributes */
    checkErr( Ivi_InvalidateAllAttributes (vi));
    

    if (!Ivi_Simulating(vi))
        {
        ViSession   io = Ivi_IOSession(vi); /* call only when locked */

        checkErr( Ivi_SetNeedToCheckStatus (vi, VI_TRUE));
	//	checkErr( Ivi_SetAttributeViBoolean (vi, "", MS97XX_ATTR_SEND_HEADER, 0, VI_FALSE));
		
		viCheckErr( viPrintf (io, "*CLS%s",terminator));
        }
    
Error:
    return error;
}

/*****************************************************************************
 * Function: MS97xx_DisableAttribute
 * Purpose:  This function marks an attribute as unsupported.
 *****************************************************************************/
static ViStatus MS97xx_DisableAttribute (ViSession vi, ViInt32 attr)
{
	ViStatus error=VI_SUCCESS;
	ViInt32 flags;
	
	viCheckErr(Ivi_GetAttributeFlags(vi,attr,&flags));
	flags |= IVI_VAL_NOT_SUPPORTED;
	viCheckErr(Ivi_SetAttributeFlags(vi,attr,flags));
  
Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ReadViReal64Attribute
 * Purpose:  This function sends the specified query and formats the response
 *           as a floating-point number.
 *****************************************************************************/
static ViStatus MS97xx_ReadViReal64Attribute (ViSession vi,
                                              ViSession io,
                                              ViConstString channelName,
                                              ViAttr attributeId,
                                              ViReal64 *value,
                                              ViConstString query,
                                              ViInt32 headers)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				formatBuf[BUFFER_SIZE];
	ViInt32				formatBufSize=sizeof(formatBuf)-1,loop;

	checkErr(viPrintf(io,"%s%s",query,terminator));

	formatBuf[0]=0x00;
	for (loop=0; loop<headers && loop<formatBufSize/7 - 1; ++loop)
		strcat(formatBuf,"%*[^,],");
	strcat(formatBuf,"%lf");
	
	checkErr(viScanf(io,formatBuf,value));

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ReadViInt32Attribute
 * Purpose:  This function sends the specified query and formats the response
 *           as an integer.
 *****************************************************************************/
static ViStatus MS97xx_ReadViInt32Attribute (ViSession vi,
                                             ViSession io,
                                             ViConstString channelName,
                                             ViAttr attributeId,
                                             ViInt32 *value,
                                             ViConstString query,
                                             ViInt32 headers)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				formatBuf[BUFFER_SIZE];
	ViInt32				formatBufSize=sizeof(formatBuf)-1,loop;

	checkErr(viPrintf(io,"%s%s",query,terminator));

	formatBuf[0]=0x00;
	for (loop=0; loop<headers && loop<formatBufSize/7 - 1; ++loop)
		strcat(formatBuf,"%*[^,],");
	strcat(formatBuf,"%ld");
	
	error=viScanf(io,formatBuf,value);
	if (error==VI_ERROR_INV_FMT)
		error=VI_SUCCESS;

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ReadDiscreteViInt32Attribute
 * Purpose:  This function sends the specified query and matches the response
 *           to a list of predefined possible answers.
 *****************************************************************************/
static ViStatus MS97xx_ReadDiscreteViInt32Attribute (ViSession vi,
                                                     ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViInt32 *value,
                                                     ViConstString query,
                                                     ViInt32 headers)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				formatBuf[BUFFER_SIZE];
	ViInt32				formatBufSize=sizeof(formatBuf)-1,loop;
	ViChar				readBuf[BUFFER_SIZE];
	ViInt32				readBufSize=sizeof(readBuf)-1;
	IviRangeTablePtr	pRanges;

	checkErr(viPrintf(io,"%s%s",query,terminator));

	formatBuf[0]=0x00;
	for (loop=0; loop<headers && loop<formatBufSize/7 - 1; ++loop)
		strcat(formatBuf,"%*[^,],");
	strcat(formatBuf,"%#s");
	
	checkErr(viScanf(io,formatBuf,&readBufSize,readBuf));
	checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(readBuf,pRanges,value,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_WriteDiscreteViInt32Attribute
 * Purpose:  This function sends one of the predefined strings for the given
 *           command, based on the integer input.
 *****************************************************************************/
static ViStatus MS97xx_WriteDiscreteViInt32Attribute (ViSession vi,
                                                      ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViInt32 value,
                                                      ViConstString query)
{
	ViStatus			error = VI_SUCCESS;
	IviRangeTablePtr	pRanges;
	ViString 			cmd;
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromValue(value,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&cmd,VI_NULL));
	checkErr(viPrintf(io,"%s %s%s",query,cmd,terminator));

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ReadDiscreteViReal64Attribute
 * Purpose:  This function sends the specified query and coerces the response
 *           to one of a list of predefined values.
 *****************************************************************************/
static ViStatus MS97xx_ReadDiscreteViReal64Attribute (ViSession vi,
                                                      ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViReal64 *value,
                                                      ViConstString query,
                                                      ViInt32 headers)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				formatBuf[BUFFER_SIZE];
	ViInt32				formatBufSize=sizeof(formatBuf)-1,loop;
	ViChar				readBuf[BUFFER_SIZE];
	ViInt32				readBufSize=sizeof(readBuf)-1;
	IviRangeTablePtr	pRanges;

	checkErr(viPrintf(io,"%s%s",query,terminator));

	formatBuf[0]=0x00;
	for (loop=0; loop<headers && loop<formatBufSize/7 - 1; ++loop)
		strcat(formatBuf,"%*[^,],");
	strcat(formatBuf,"%#s");
	
	checkErr(viScanf(io,formatBuf,&readBufSize,readBuf));
	checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
	checkErr(Ivi_GetViReal64EntryFromString(readBuf,pRanges,value,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_WriteDiscreteViReal64Attribute
 * Purpose:  This function coerces the floating-point value to one of the
 *           predefined values applicable for the given command.
 *****************************************************************************/
static ViStatus MS97xx_WriteDiscreteViReal64Attribute (ViSession vi,
                                                       ViSession io,
                                                       ViConstString channelName,
                                                       ViAttr attributeId,
                                                       ViReal64 value,
                                                       ViConstString query)
{
	ViStatus			error = VI_SUCCESS;
	IviRangeTablePtr	pRanges;
	ViString 			cmd;
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
	checkErr(Ivi_GetViReal64EntryFromValue(value,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&cmd,VI_NULL));
	checkErr(viPrintf(io,"%s %s%s",query,cmd,terminator));

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_ReadBinaryInt16Array
 * Purpose:  This function reads raw data from the instrument and converts
 *           them to short integer values.
 *****************************************************************************/
static ViStatus MS97xx_ReadBinaryInt16Array (ViSession vi,ViSession io,ViInt32 maxPoints,
											 ViInt16 data[],ViInt32 *numPoints)
{
	*numPoints=maxPoints;
	return (viScanf(io,"%#!obhy",numPoints,data));
}

/*****************************************************************************
 * Function: MS97xx_ReadBinaryReal64Array
 * Purpose:  This function reads raw data from the instrument and converts
 *           them to floating-point values scaled by 100.
 *****************************************************************************/
static ViStatus MS97xx_ReadBinaryReal64Array (ViSession vi,ViSession io,ViInt32 maxPoints,
											  ViReal64 data[],ViInt32 *numPoints)
{
	ViStatus			error=VI_SUCCESS;
	ViAddr				readBuf=VI_NULL;
	ViInt16				*intBuf;
	ViInt32				expected=5002,received=0,loop;
	
	checkErr(MS97xx_GetReadBuffer(vi,expected*sizeof(ViInt16),&readBuf));
	intBuf=(ViInt16*)readBuf;
	
	*numPoints=0;

	checkErr(MS97xx_ReadBinaryInt16Array(vi,io,expected,intBuf,&received));
	--received;

	for (loop=0; loop<received && *numPoints<maxPoints; ++loop, ++*numPoints)
		data[*numPoints]=((ViReal64)(intBuf[loop]))/100.0;

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_GetReadBuffer
 * Purpose:  This function makes sure that the read buffer is at least
 *           as long as the indicated size, then returns a pointer to the
 *           buffer.
 *****************************************************************************/
static ViStatus MS97xx_GetReadBuffer (ViSession vi,ViInt32 size,ViAddr *readBuf)
{
	ViStatus	error=VI_SUCCESS;
	ViInt32		oldSize;
	
	
	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_READ_BUF_SIZE,0,&oldSize));
	checkErr(Ivi_GetAttributeViAddr(vi,"",MS97XX_ATTR_READ_BUF,0,readBuf));

	if (size>oldSize)
	{
		if (*readBuf!=VI_NULL)
			checkErr(Ivi_Free(vi,*readBuf));
		checkErr(Ivi_Alloc(vi,size,readBuf));
		checkErr(Ivi_SetAttributeViAddr(vi,"",MS97XX_ATTR_READ_BUF,0,*readBuf));
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_READ_BUF_SIZE,0,size));
	}

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CheckFileName
 * Purpose:  This function verifies that the given file name is valid for
 *           the instrument.
 *****************************************************************************/
static ViStatus MS97xx_CheckFileName (ViSession vi,ViChar fileName[])
{
	ViStatus error=VI_SUCCESS;
	ViInt32 idFormat,len,loop,fileNum;
	ViChar ext[4];
	
	checkErr(MS97xx_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,&idFormat));

	len=strlen(fileName);
	if (len==0)
		error=MS97XX_ERROR_INVALID_FILE_NAME;

	if (idFormat==MS97XX_VAL_FILE_ID_FORMAT_NAME)
	{
		ViChar *pExt=strchr(fileName,'.');
		if (pExt==NULL)
			error=(len<=8 ? VI_SUCCESS : MS97XX_ERROR_INVALID_FILE_NAME);
		else if (strlen(pExt)==0)
			error=MS97XX_ERROR_INVALID_FILE_NAME;
		else
		{
			loop=0;
			do
			{
				ext[loop]=tolower(pExt[loop+1]);
			} while (++loop<3 && pExt[loop]!='\0');
			ext[3]='\0';
			if (strcmp(ext,"dat")!=0)
				error=MS97XX_ERROR_INVALID_FILE_NAME;
			else if (len>12)
				error=MS97XX_ERROR_INVALID_FILE_NAME;
		}
	}
	else
	{
		 if (isdigit(fileName[0]))
		 {
		 	fileNum=atoi(fileName);
		 	if (fileNum<0 || fileNum>999)
		 		error=MS97XX_ERROR_INVALID_FILE_NAME;
		 }
		 else
		 	error=MS97XX_ERROR_INVALID_FILE_NAME;
	}

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CheckMeasureMode
 * Purpose:  This function verifies that the instrument is in the given
 *           measurement mode.
 *****************************************************************************/
static ViStatus MS97xx_CheckMeasureMode (ViSession vi,ViUInt16 modeMask,ViBoolean allowModes)
{
	ViStatus	error = VI_FALSE;
	ViInt32		measureMode;
	ViUInt16	instrModeMask=0;

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_MEASURE_MODE,0,&measureMode));
	
	switch (measureMode)
	{
		case MS97XX_VAL_MEASURE_MODE_SPECTRUM:
			instrModeMask=MS97XX_MODE_MASK_SPC;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_INSERTION_LOSS:
			instrModeMask=MS97XX_MODE_MASK_ILO;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_ISOLATION:
			instrModeMask=MS97XX_MODE_MASK_ISO;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_DIRECTIVITY:
			instrModeMask=MS97XX_MODE_MASK_DIR;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_RETURN_LOSS:
			instrModeMask=MS97XX_MODE_MASK_RLO;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_PMD:
			instrModeMask=MS97XX_MODE_MASK_PMD;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_NF:
			instrModeMask=MS97XX_MODE_MASK_NF;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_WDM:
			instrModeMask=MS97XX_MODE_MASK_WDM;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_LONG_TERM:
			instrModeMask=MS97XX_MODE_MASK_LTS;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_DFB:
			instrModeMask=MS97XX_MODE_MASK_DFB;
		break;

		case MS97XX_VAL_MEASURE_MODE_FP:
			instrModeMask=MS97XX_MODE_MASK_FP;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_LED:
			instrModeMask=MS97XX_MODE_MASK_LED;
		break;
		
		case MS97XX_VAL_MEASURE_MODE_AMPLIFIER:
			instrModeMask=MS97XX_MODE_MASK_AMP;
		break;
		
		default:
		break;
	}

	if (allowModes==VI_TRUE && (instrModeMask & modeMask) == 0 ||
		allowModes==VI_FALSE && (instrModeMask & modeMask) !=0)
		error=MS97XX_ERROR_INCORRECT_MODE;

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CheckWDM_Mode
 * Purpose:  This function verifies that the instrument is in the given
 *           WDM measurement mode.
 *****************************************************************************/
static ViStatus MS97xx_CheckWDM_Mode (ViSession vi,ViUInt16 modeMask)
{
	ViStatus	error = VI_FALSE;
	ViInt32		wdmMode;
	ViUInt16	instrWdmMode=0;

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_MODE,0,&wdmMode));
	
	switch (wdmMode)
	{
		case MS97XX_VAL_WDM_MULTIPEAK:
			instrWdmMode=MS97XX_WDM_MASK_MPK;
		break;
		
		case MS97XX_VAL_WDM_SNR:
			instrWdmMode=MS97XX_WDM_MASK_SNR;
		break;
		
		case MS97XX_VAL_WDM_RELATIVE:
			instrWdmMode=MS97XX_WDM_MASK_REL;
		break;
		
		case MS97XX_VAL_WDM_TABLE:
			instrWdmMode=MS97XX_WDM_MASK_TBL;
		break;
		
		default:
		break;
	}

	if ((instrWdmMode & modeMask) == 0)
		error=MS97XX_ERROR_INCORRECT_MODE;

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CheckDisplayMode
 * Purpose:  This function verifies that the instrument is in the given
 *           spectrum display mode.
 *****************************************************************************/
static ViStatus MS97xx_CheckDisplayMode (ViSession vi,ViUInt16 modeMask)
{
	ViStatus	error = VI_FALSE;
	ViInt32		displayMode;
	ViUInt16	instrDisplayMode=0;

	checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DISPLAY_MODE,0,&displayMode));
	
	switch (displayMode)
	{
		case MS97XX_VAL_DISPLAY_MODE_NORMAL:
			instrDisplayMode=MS97XX_DISPLAY_MASK_NRM;
		break;
		
		case MS97XX_VAL_DISPLAY_MODE_OVERLAP:
			instrDisplayMode=MS97XX_DISPLAY_MASK_OVL;
		break;
		
		case MS97XX_VAL_DISPLAY_MODE_DBM_DBM:
			instrDisplayMode=MS97XX_DISPLAY_MASK_DBM;
		break;
		
		case MS97XX_VAL_DISPLAY_MODE_DBM_DB:
			instrDisplayMode=MS97XX_DISPLAY_MASK_DB;
		break;
		
		default:
		break;
	}

	if ((instrDisplayMode & modeMask) == 0)
		error=MS97XX_ERROR_INCORRECT_MODE;

Error:
	return error;
}

/*****************************************************************************
 * Function: MS97xx_CachingIsOn
 * Purpose:  This function returns VI_TRUE only if state caching is turned on.
 *****************************************************************************/
static ViBoolean MS97xx_CachingIsOn (ViSession vi)
{
	ViBoolean caching;
	if (Ivi_GetAttributeViBoolean(vi,"",IVI_ATTR_CACHE,0,&caching)==VI_SUCCESS)
		return caching;
	else
		return VI_FALSE;
}

/*****************************************************************************
 * Function: ReadToFile and WriteFromFile Functions                          
 * Purpose:  Functions for instrument driver developers to read/write        
 *           instrument data to/from a file.                                 
 *****************************************************************************/
static ViStatus MS97xx_ReadToFile (ViSession vi, ViConstString filename, 
                                     ViInt32 maxBytesToRead, ViInt32 fileAction, 
                                     ViInt32 *totalBytesWritten)  
{   
    return Ivi_ReadToFile (vi, filename, maxBytesToRead, fileAction, totalBytesWritten);  
}   
static ViStatus MS97xx_WriteFromFile (ViSession vi, ViConstString filename, 
                                        ViInt32 maxBytesToWrite, ViInt32 byteOffset, 
                                        ViInt32 *totalBytesWritten) 
{   
    return Ivi_WriteFromFile (vi, filename, maxBytesToWrite, byteOffset, totalBytesWritten); 
}

/*****************************************************************************
 *------------------------ Global Session Callbacks -------------------------*
 *****************************************************************************/

/*****************************************************************************
 * Function: MS97xx_CheckStatusCallback                                               
 * Purpose:  This function queries the instrument to determine if it has 
 *           encountered an error.  If the instrument has encountered an 
 *           error, this function returns the IVI_ERROR_INSTRUMENT_SPECIFIC 
 *           error code.  This function is called by the 
 *           MS97xx_CheckStatus utility function.  The 
 *           Ivi_SetAttribute and Ivi_GetAttribute functions invoke this 
 *           function when the optionFlags parameter includes the
 *           IVI_VAL_DIRECT_USER_CALL flag.
 *           
 *           The user can disable calls to this function by setting the 
 *           IVI_ATTR_QUERY_INSTR_STATUS attribute to VI_FALSE.  The driver can 
 *           disable the check status callback for a particular attribute by 
 *           setting the IVI_VAL_DONT_CHECK_STATUS flag.
 *****************************************************************************/
static ViStatus _VI_FUNC MS97xx_CheckStatusCallback (ViSession vi, ViSession io)
{
    ViStatus    error = VI_SUCCESS;

    ViInt16     esr = 0; 
    viCheckErr( viWrite (io, "*ESR?", 5, VI_NULL));
    viCheckErr( viScanf (io, "%hd", &esr));
	
    if ((esr & IEEE_488_2_QUERY_ERROR_BIT) ||
        (esr & IEEE_488_2_DEVICE_DEPENDENT_ERROR_BIT) ||
        (esr & IEEE_488_2_EXECUTION_ERROR_BIT) ||
        (esr & IEEE_488_2_COMMAND_ERROR_BIT))
    {     
        viCheckErr( IVI_ERROR_INSTR_SPECIFIC);
    }
            
Error:
    return error;
}

/*****************************************************************************
 * Function: MS97xx_WaitForOPCCallback                                               
 * Purpose:  The IVI engine invokes this function in the following two cases:
 *           - Before invoking the read callback for attributes for which the 
 *             IVI_VAL_WAIT_FOR_OPC_BEFORE_READS flag is set.
 *           - After invoking the write callback for attributes for which the 
 *             IVI_VAL_WAIT_FOR_OPC_AFTER_WRITES flag is set.
 *
 *           The MS97xx instruments do not always wait for an operation to
 *           complete, so this function is kept only for IVI compatibility.
 *
 *           To wait until an operation is complete, use
 *           WaitUntilOperationComplete instead.
 *****************************************************************************/
static ViStatus _VI_FUNC MS97xx_WaitForOPCCallback (ViSession vi, ViSession io)
{
    ViStatus    error = VI_SUCCESS;

/*
	viCheckErr( viEnableEvent (io, VI_EVENT_SERVICE_REQ, VI_QUEUE, VI_NULL));
    viCheckErr( viPrintf (io, "*OPC\r\n"));

    viCheckErr( viWaitOnEvent (io, VI_EVENT_SERVICE_REQ, opcTimeout, VI_NULL, VI_NULL));
    viCheckErr( viDisableEvent (io, VI_EVENT_SERVICE_REQ, VI_QUEUE));

    viCheckErr( viBufWrite (io, "*CLS", 4, VI_NULL));
    viCheckErr( viReadSTB (io, &statusByte));
    
Error:
    viDiscardEvents (io, VI_EVENT_SERVICE_REQ, VI_QUEUE);
*/
    return error;
}

/*****************************************************************************
 *----------------- Attribute Range Tables and Callbacks --------------------*
 *****************************************************************************/

/*- MS97XX_ATTR_ID_QUERY_RESPONSE -*/

static ViStatus _VI_FUNC MS97xxAttrIdQueryResponse_ReadCallback (ViSession vi, 
                                                                   ViSession io,
                                                                   ViConstString channelName, 
                                                                   ViAttr attributeId,
                                                                   const ViConstString cacheValue)
{
    ViStatus    error = VI_SUCCESS;
    ViChar      rdBuffer[BUFFER_SIZE];
    ViUInt32    retCnt;
    
    viCheckErr( viPrintf (io, "*IDN?"));
    viCheckErr( viRead (io, rdBuffer, BUFFER_SIZE-1, &retCnt));
    rdBuffer[retCnt] = 0;

    checkErr( Ivi_SetValInStringCallback (vi, attributeId, rdBuffer));
    
Error:
    return error;
}
    
/*- MS97XX_ATTR_DRIVER_REVISION -*/

static ViStatus _VI_FUNC MS97xxAttrDriverRevision_ReadCallback (ViSession vi, 
                                                                  ViSession io,
                                                                  ViConstString channelName, 
                                                                  ViAttr attributeId,
                                                                  const ViConstString cacheValue)
{
    ViStatus    error = VI_SUCCESS;
    ViChar      driverRevision[256];
    
    
    sprintf (driverRevision, 
             "Driver: MS97xx %.2lf, Compiler: %s %3.2f, "
             "Components: IVIEngine %.2lf, VISA-Spec %.2lf",
             MS97XX_MAJOR_VERSION + MS97XX_MINOR_VERSION/1000.0, 
             IVI_COMPILER_NAME, IVI_COMPILER_VER_NUM, 
             IVI_ENGINE_MAJOR_VERSION + IVI_ENGINE_MINOR_VERSION/1000.0, 
             Ivi_ConvertVISAVer(VI_SPEC_VERSION));

    checkErr( Ivi_SetValInStringCallback (vi, attributeId, driverRevision));    
Error:
    return error;
}


static ViStatus _VI_FUNC MS97xxAttrStartWavelength_RangeTableCallback (ViSession vi,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrStartWavelengthRangeTable_20;
	else
		tblPtr=&attrStartWavelengthRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrOsaType_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViInt32		ch=0;
	ViChar		readBuf[BUFFER_SIZE];
	ViInt32		readBufSize=sizeof(readBuf)-1;
	IviAttrFlags flags;
	
	*value=0;

	Ivi_GetAttributeViString (vi, "", MS97XX_ATTR_ID_QUERY_RESPONSE, 0,
								512, readBuf);
	
	ch=readBuf[12];
	if (ch >= '0' && ch <= '9')
		*value |= ((ch-'0') << 16);
	ch=readBuf[13];
	if (ch >= '0' && ch <= '9')
		*value |= ((ch-'0') << 8);
	ch=readBuf[14];
	if (ch >= 'A' && ch <= 'Z')
		*value |= (ch-'A'+1);

	if ((*value & MS97XX_VAL_OSA_TYPE_MS9710) != 0)
	{
		/* Disable attributes which work on the MS9710C but not on the MS9710C. */
		if ((*value & MS97XX_VAL_OSA_TYPE_LETTER) < 0x00000003)
		{
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_MARKER_UNITS);
		}
		
		/* Disable attributes which don't work on the MS9710 series. */
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AUTO_REF_ON);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AUTO_RESOLUTION);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_NUMERIC_FORMAT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_INSTRUMENT_FORMAT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_FITTING_SPAN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_PIN_LOSS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_CAL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_ASE_CAL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_POUT_LOSS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NF_CUT_LEVEL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_SCREEN_MODE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_MEMORY_A_STORAGE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_MEMORY_B_STORAGE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_MEMORY_C_STORAGE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_1_VISIBLE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_1_SCREEN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_1_NUMERATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_1_DENOMINATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_2_VISIBLE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_2_SCREEN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_2_NUMERATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_2_DENOMINATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_3_VISIBLE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_3_SCREEN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_3_NUMERATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TRACE_3_DENOMINATOR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_ACTIVE_TRACE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_ACTIVE_SCREEN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_UPPER_REF_LEVEL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LOWER_REF_LEVEL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_UPPER_LOG_SCALE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LOWER_LOG_SCALE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_FIRST_WDM_SIGNAL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_SIGNAL_SPACING);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_SIGNAL_COUNT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_PMD_COUPLING);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LAST_PMD_PEAK);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_FIRST_PMD_PEAK);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_PMD_PEAK_COUNT);

		/* Disable attributes which don't work on the MS9715. */
		if ((*value & MS97XX_VAL_OSA_TYPE_OSA) != 5)
		{
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_SAMPLE_TIME);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_STATE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_DATA_SIZE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY);
		}
	}
	else if ((*value & MS97XX_VAL_OSA_TYPE_MS9720) != 0)
	{
		if ((*value & MS97XX_VAL_OSA_TYPE_OSA) >= 5)
		{
			/* Disable attributes which don't work on the card OSAs. */
	  		*value |= MS97XX_VAL_OSA_TYPE_CARD;
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_ATTENUATOR_ON);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_AUTO_RESOLUTION);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_MODULATION_MODE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_PEAK_HOLD);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_WIDE_DYNAMIC_RANGE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_OPTION);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_ID_FORMAT);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_DISKETTE_SIZE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_NUMERIC_FORMAT);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_FILE_INSTRUMENT_FORMAT);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LIGHT_OUTPUT);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_SAMPLE_TIME);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_STATE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_DATA_SIZE);
			MS97xx_DisableAttribute(vi,MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY);
	  	}
	 
		/* Disable attributes which don't work on the MS9720 series. */
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WAVELENGTH_OFFSET);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LINEAR_SCALE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LEVEL_SCALE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AUTO_MEASURING);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_EXTERNAL_TRIGGER_DELAY);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_SWEEP_INTERVAL_TIME);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_ACTUAL_RESOLUTION);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_SHOW_ACTUAL_RESOLUTION);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_ENVELOPE_CUT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_RMS_CUT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_RMS_COEFFICIENT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_NDB_ATTENUATION);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_DFB_SIDE_MODE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_DFB_NDB_WIDTH);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_FP_AXIS_MODE_CUT_LEVEL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_LED_NDB_DOWN_WAVE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_MEMORY);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_CAL_STATUS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_CALC_MODE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_MEASURE_METHOD);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_FITTING_METHOD);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_FITTING_SPAN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_MASKED_SPAN);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_PIN_LOSS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_POUT_LOSS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_NF_CAL);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_BAND_PASS_CUT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_FILTER_BANDWIDTH);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_AMP_POLAR_LOSS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_SNR_NORM);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_TABLE_DIR);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_TABLE_DW);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_TABLE_NORM);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_PEAK_TYPE);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_WDM_THRESHOLD);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_MARKER_UNITS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TLS_TRACK_ENABLED);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_TLS_CAL_STATUS);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_POWER_MONITOR_WAVELENGTH);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_POWER_MONITOR_RESULT);
		MS97xx_DisableAttribute(vi,MS97XX_ATTR_CURRENT_TRACE);
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStartWavelength_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"STA?",0));
	
Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStartWavelength_WriteCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"STA %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrStartWavelengthRangeTable_20;
	else
		tblPtr=&attrStartWavelengthRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"STO?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStopWavelength_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"STO %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_RangeTableCallback (ViSession vi,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

	viCheckErr(Ivi_GetAttributeViInt32 (vi, "", MS97XX_ATTR_OSA_TYPE,0,
										   &osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrCenterWavelengthRangeTable_20;
	else
		tblPtr=&attrCenterWavelengthRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"CNT?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrCenterWavelength_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"CNT %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

	viCheckErr(Ivi_GetAttributeViInt32 (vi, "", MS97XX_ATTR_OSA_TYPE,0,
										   &osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrWavelengthSpanRangeTable_20;
	else
		tblPtr=&attrWavelengthSpanRangeTable_10;
		
Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"SPN?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthSpan_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"SPN %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAutoAlignStatus_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ALIN?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthOffset_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"WOFS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthOffset_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"WOFS %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthDisplay_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"WDP?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthDisplay_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"WDP"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAutoRefOn_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(viPrintf(io,"ATRF?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAutoRefOn_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ATRF %s%s",
	                    (value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrLevelOffsetRangeTable_20;
	else if (osaType==MS97XX_VAL_OSA_TYPE_MS9710C)
		tblPtr=&attrLevelOffsetRangeTable_10C;
	else
		tblPtr=&attrLevelOffsetRangeTable_10B;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"LOFS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLevelOffset_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"LOFS %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLevelScale_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"LVS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLinearScale_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType,trace,denominator;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9710)
	{
	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_CURRENT_TRACE,0,&trace));
	    if (trace==MS97XX_VAL_CURRENT_TRACE_A_B ||
	    	trace==MS97XX_VAL_CURRENT_TRACE_B_A)
	    	tblPtr=&attrLinearScaleRangeTable_Percent;
	    else
	    	tblPtr=&attrLinearScaleRangeTable;
	}

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLinearScale_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		unitBuf[10];
	ViInt32		unitBufSize=sizeof(unitBuf)-1;

	checkErr(viPrintf(io,"LLV?%s",terminator));
	checkErr(viScanf(io,"%lf%#s",&unitBufSize,unitBuf,value));

	if (strcmp(unitBuf,"PW")==0)
		*value/=1E12;
	else if (strcmp(unitBuf,"NW")==0)
		*value/=1E9;
	else if (strcmp(unitBuf,"UW")==0)
		*value/=1E6;
	else if (strcmp(unitBuf,"MW")==0)
		*value/=1E3;

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLinearScale_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	pRanges;
	ViInt32 magnitude;

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_LINEAR_SCALE,&pRanges));
	if (pRanges==&attrLinearScaleRangeTable_Percent)
	{
		checkErr(viPrintf(io,"LLV %.1lf PCT%s",value,terminator));
	}
	else
	{
		magnitude=log10(value);
		if (value<-9)
			checkErr(viPrintf(io,"LLV %.3lf PW%s",value*1E12,terminator));
		else if (value<-6)
			checkErr(viPrintf(io,"LLV %.3lf NW%s",value*1E9,terminator));
		else if (value<-3)
			checkErr(viPrintf(io,"LLV %.3lf UW%s",value*1E6,terminator));
		else if (value<0)
			checkErr(viPrintf(io,"LLV %.3lf MW%s",value*1E3,terminator));
		else
			checkErr(viPrintf(io,"LLV %.3lf W%s",value,terminator));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLogScale_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC |
										MS97XX_MODE_MASK_WDM |
										MS97XX_MODE_MASK_PMD |
										MS97XX_MODE_MASK_NF,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"LOG?",0));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrLogScale_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"LOG %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRefLevel_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC |
										MS97XX_MODE_MASK_WDM |
										MS97XX_MODE_MASK_PMD |
										MS97XX_MODE_MASK_NF,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"RLV?",0));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrRefLevel_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"RLV %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRefLevel_RangeTableCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType,trace,denominator;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
	{
	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_ACTIVE_TRACE,0,&trace));

	    switch (trace)
	    {
	    	case 2:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_2_DENOMINATOR,0,&denominator));
	    	break;
	    	
	    	case 3:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_3_DENOMINATOR,0,&denominator));
	    	break;
	    	
	    	default:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_1_DENOMINATOR,0,&denominator));
	    	break;
	    }
	    
	    if (denominator==MS97XX_VAL_TRACE_NONE)
	    	tblPtr=&attrRefLevelRangeTable;
	    else
	    	tblPtr=&attrRefdBLevelRangeTable;
	}
	else
	{
	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_CURRENT_TRACE,0,&trace));
	    if (trace==MS97XX_VAL_CURRENT_TRACE_A_B ||
	    	trace==MS97XX_VAL_CURRENT_TRACE_B_A)
	    	tblPtr=&attrRefdBLevelRangeTable;
	    else
	    	tblPtr=&attrRefLevelRangeTable;
	}

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTraceVisible_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;

	MS97xxAttrTrace_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTrace_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],
						visibleBuf[10],
						screenBuf[10],
						numeratorBuf[10],
						denominatorBuf[10];
	ViInt32				readBufSize=sizeof(readBuf)-1,
						visibleBufSize=sizeof(visibleBuf)-1,
						screenBufSize=sizeof(screenBuf)-1,
						numeratorBufSize=sizeof(numeratorBuf)-1,
						denominatorBufSize=sizeof(denominatorBuf)-1;
	IviRangeTablePtr	pRanges;
	ViInt32				visibleAttr,screenAttr,numerAttr,denomAttr,dirtyAttr,
						screen=0,numer=0,denom=0;

	ViBoolean			visible,cached;
	
	if (attributeId==MS97XX_ATTR_TRACE_2_VISIBLE ||
		attributeId==MS97XX_ATTR_TRACE_2_SCREEN ||
		attributeId==MS97XX_ATTR_TRACE_2_NUMERATOR ||
		attributeId==MS97XX_ATTR_TRACE_2_DENOMINATOR)
	{
		visibleAttr=MS97XX_ATTR_TRACE_2_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_2_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_2_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_2_DENOMINATOR;
		dirtyAttr=MS97XX_ATTR_TRACE_2_ATTR_DIRTY;
		checkErr(viPrintf(io,"TPR2?%s",terminator));
	}
	else if (attributeId==MS97XX_ATTR_TRACE_3_VISIBLE ||
		attributeId==MS97XX_ATTR_TRACE_3_SCREEN ||
		attributeId==MS97XX_ATTR_TRACE_3_NUMERATOR ||
		attributeId==MS97XX_ATTR_TRACE_3_DENOMINATOR)
	{
		visibleAttr=MS97XX_ATTR_TRACE_3_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_3_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_3_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_3_DENOMINATOR;
		dirtyAttr=MS97XX_ATTR_TRACE_3_ATTR_DIRTY;
		checkErr(viPrintf(io,"TPR3?%s",terminator));
	}
	else
	{
		visibleAttr=MS97XX_ATTR_TRACE_1_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_1_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_1_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_1_DENOMINATOR;
		dirtyAttr=MS97XX_ATTR_TRACE_1_ATTR_DIRTY;
		checkErr(viPrintf(io,"TPR1?%s",terminator));
	}

/*
  The following call should be used instead of Scan,
  but a bug in VISA causes a failure to break
  the string up into the required pieces.
  	checkErr(viScanf(io,"%#[^,],%#[^,],%#[^,],%#s",
  					 &visibleBufSize,visibleBuf,
  					 &screenBufSize,screenBuf,
  					 &numeratorBufSize,numeratorBuf,
  					 &denominatorBufSize,denominatorBuf));
*/

	checkErr(viScanf(io,"%#s",
			 		 &readBufSize,readBuf));

	checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*q]",
				  visibleBufSize,visibleBuf,
				  screenBufSize,screenBuf,
				  numeratorBufSize,numeratorBuf,
				  denominatorBufSize,denominatorBuf));
	
	visible=(strcmp(visibleBuf,"VIEW")==0 ? VI_TRUE : VI_FALSE);

	if (strcmp(screenBuf,"***")!=0)
	{
		checkErr(Ivi_GetAttrRangeTable(vi,"",screenAttr,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromString(screenBuf,pRanges,&screen,VI_NULL,VI_NULL,VI_NULL,VI_NULL));
	}
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",numerAttr,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(numeratorBuf,pRanges,&numer,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

	if (strcmp(denominatorBuf,"***")!=0)
	{
		checkErr(Ivi_GetAttrRangeTable(vi,"",denomAttr,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromString(denominatorBuf,pRanges,&denom,VI_NULL,VI_NULL,VI_NULL,VI_NULL));
	}

	if (Ivi_AttributeIsCached(vi,channelName,visibleAttr,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViBoolean (vi, "", visibleAttr,
										    IVI_VAL_SET_CACHE_ONLY, visible));
	}

	if (Ivi_AttributeIsCached(vi,channelName,screenAttr,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32 (vi, "", screenAttr,
										  IVI_VAL_SET_CACHE_ONLY,
										  screen));
	}
	
	if (Ivi_AttributeIsCached(vi,channelName,numerAttr,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32 (vi, "", numerAttr,
										  IVI_VAL_SET_CACHE_ONLY,
									   	  numer));
	}

	if (Ivi_AttributeIsCached(vi,channelName,denomAttr,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32 (vi, "", denomAttr,
											IVI_VAL_SET_CACHE_ONLY,
									   		denom));
	}
	
	if (attributeId==visibleAttr)
		*((ViBoolean*)value)=visible;
	else if (attributeId==screenAttr)
		*value=screen;
	else if (attributeId==numerAttr)
		*value=numer;
	else if (attributeId==denomAttr)
		*value=denom;

	checkErr(Ivi_SetAttributeViBoolean(vi,"",dirtyAttr,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTraceVisible_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViBoolean(vi,channelName,attributeId,
									   IVI_VAL_SET_CACHE_ONLY,value));

	switch (attributeId)
	{
		case MS97XX_ATTR_TRACE_1_VISIBLE:
		case MS97XX_ATTR_TRACE_1_SCREEN:
		case MS97XX_ATTR_TRACE_1_NUMERATOR:
		case MS97XX_ATTR_TRACE_1_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_1_ATTR_DIRTY,0,VI_TRUE));
		break;

		case MS97XX_ATTR_TRACE_2_VISIBLE:
		case MS97XX_ATTR_TRACE_2_SCREEN:
		case MS97XX_ATTR_TRACE_2_NUMERATOR:
		case MS97XX_ATTR_TRACE_2_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_2_ATTR_DIRTY,0,VI_TRUE));
		break;

		case MS97XX_ATTR_TRACE_3_VISIBLE:
		case MS97XX_ATTR_TRACE_3_SCREEN:
		case MS97XX_ATTR_TRACE_3_NUMERATOR:
		case MS97XX_ATTR_TRACE_3_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_3_ATTR_DIRTY,0,VI_TRUE));
		break;
		
		default:
		break;
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTrace_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,channelName,attributeId,
									 IVI_VAL_SET_CACHE_ONLY,value));

	switch (attributeId)
	{
		case MS97XX_ATTR_TRACE_1_VISIBLE:
		case MS97XX_ATTR_TRACE_1_SCREEN:
		case MS97XX_ATTR_TRACE_1_NUMERATOR:
		case MS97XX_ATTR_TRACE_1_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_1_ATTR_DIRTY,0,VI_TRUE));
		break;

		case MS97XX_ATTR_TRACE_2_VISIBLE:
		case MS97XX_ATTR_TRACE_2_SCREEN:
		case MS97XX_ATTR_TRACE_2_NUMERATOR:
		case MS97XX_ATTR_TRACE_2_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_2_ATTR_DIRTY,0,VI_TRUE));
		break;

		case MS97XX_ATTR_TRACE_3_VISIBLE:
		case MS97XX_ATTR_TRACE_3_SCREEN:
		case MS97XX_ATTR_TRACE_3_NUMERATOR:
		case MS97XX_ATTR_TRACE_3_DENOMINATOR:
			checkErr(Ivi_SetAttributeViBoolean(vi,"",
					 MS97XX_ATTR_TRACE_3_ATTR_DIRTY,0,VI_TRUE));
		break;
		
		default:
		break;
	}


Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrTraceAttr_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;
	ViBoolean	dirty,locked,visible;
	ViInt32		screen,numerator,denominator;
	ViString	screenCmd,numeratorCmd,denominatorCmd;
	ViInt32		lockedAttr,dirtyAttr,visibleAttr,screenAttr,numerAttr,denomAttr;
	ViInt32		displayMode;
	IviRangeTablePtr pRanges;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	if (attributeId==MS97XX_ATTR_TRACE_2_ATTR_LOCKED ||
		attributeId==MS97XX_ATTR_TRACE_2_ATTR_DIRTY)
	{
		lockedAttr=MS97XX_ATTR_TRACE_2_ATTR_LOCKED;
		dirtyAttr=MS97XX_ATTR_TRACE_2_ATTR_DIRTY;
		visibleAttr=MS97XX_ATTR_TRACE_2_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_2_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_2_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_2_DENOMINATOR;
	}
	else if (attributeId==MS97XX_ATTR_TRACE_3_ATTR_LOCKED ||
		attributeId==MS97XX_ATTR_TRACE_3_ATTR_DIRTY)
	{
		lockedAttr=MS97XX_ATTR_TRACE_3_ATTR_LOCKED;
		dirtyAttr=MS97XX_ATTR_TRACE_3_ATTR_DIRTY;
		visibleAttr=MS97XX_ATTR_TRACE_3_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_3_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_3_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_3_DENOMINATOR;
	}
	else
	{
		lockedAttr=MS97XX_ATTR_TRACE_1_ATTR_LOCKED;
		dirtyAttr=MS97XX_ATTR_TRACE_1_ATTR_DIRTY;
		visibleAttr=MS97XX_ATTR_TRACE_1_VISIBLE;
		screenAttr=MS97XX_ATTR_TRACE_1_SCREEN;
		numerAttr=MS97XX_ATTR_TRACE_1_NUMERATOR;
		denomAttr=MS97XX_ATTR_TRACE_1_DENOMINATOR;
	}

	if (attributeId==lockedAttr)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",dirtyAttr,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",lockedAttr,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DISPLAY_MODE,0,&displayMode));
		
		checkErr(Ivi_GetAttributeViBoolean(vi,"",visibleAttr,0,&visible));
		checkErr(Ivi_GetAttributeViInt32(vi,"",screenAttr,0,&screen));
		checkErr(Ivi_GetAttributeViInt32(vi,"",numerAttr,0,&numerator));
		checkErr(Ivi_GetAttributeViInt32(vi,"",denomAttr,0,&denominator));
			
		checkErr(Ivi_GetAttrRangeTable(vi,"",screenAttr,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(screen,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&screenCmd,VI_NULL));
		checkErr(Ivi_GetAttrRangeTable(vi,"",numerAttr,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(numerator,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&numeratorCmd,VI_NULL));

		switch (displayMode)
		{
			case MS97XX_VAL_DISPLAY_MODE_DBM_DBM:
				denominator=MS97XX_VAL_TRACE_NONE;
			break;
			
			case MS97XX_VAL_DISPLAY_MODE_DBM_DB:
				if (screen==MS97XX_VAL_TRACE_SCREEN_LOWER)
				{
					if (denominator==MS97XX_VAL_TRACE_NONE)
						denominator=MS97XX_VAL_TRACE_A;
				}
				else
					denominator=MS97XX_VAL_TRACE_NONE;
			break;
		}

		checkErr(Ivi_GetAttrRangeTable(vi,"",denomAttr,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(denominator,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&denominatorCmd,VI_NULL));

		if (denominator != MS97XX_VAL_TRACE_NONE)
			checkErr(viPrintf(io,"%s %s,%s,%s,%s%s",
								(screenAttr==MS97XX_ATTR_TRACE_2_SCREEN ? "TPR2" :
									(screenAttr==MS97XX_ATTR_TRACE_3_SCREEN ? "TPR3" : "TPR1")),
								(visible==VI_TRUE ? "VIEW" : "HIDE"),
								screenCmd,
								numeratorCmd,
								denominatorCmd,
								terminator));
		else
			checkErr(viPrintf(io,"%s %s,%s,%s%s",
								(screenAttr==MS97XX_ATTR_TRACE_2_SCREEN ? "TPR2" :
									(screenAttr==MS97XX_ATTR_TRACE_3_SCREEN ? "TPR3" : "TPR1")),
								(visible==VI_TRUE ? "VIEW" : "HIDE"),
								screenCmd,
								numeratorCmd,
								terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",dirtyAttr,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrActiveTrace_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ATC?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrActiveTrace_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ATC %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrActiveScreen_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_DBM |
										MS97XX_DISPLAY_MASK_DB));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"ATS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrActiveScreen_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	pRanges;
	ViString cmd;
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_ACTIVE_SCREEN,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromValue(value,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&cmd,VI_NULL));
	checkErr(viPrintf(io,"ATS %s%s",cmd,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAttenuatorOn_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	checkErr(viPrintf(io,"ATT?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrAttenuatorOn_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ATT %s%s",
	                    (value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrCurrentTrace_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"TSL?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrCurrentTrace_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"TSL"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrFirstWdmSignal_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	
	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"FSG?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrFirstWdmSignal_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"FSG %.2lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLowerLogScale_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"LLOG?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLowerLogScale_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"LLOG %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLowerRefLevel_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"LRLV?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLowerRefLevel_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"LRLV %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryAStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTA?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryAStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTA"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryBStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTB?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryBStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTB"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryCStorage_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTC?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMemoryCStorage_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSTC"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrUpperLogScale_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"ULOG?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrUpperLogScale_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ULOG %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrUpperRefLevel_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"URLV?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrUpperRefLevel_WriteCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"URLV %.1lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSignalCount_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"SGC?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSignalCount_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"SGC %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSignalSpacing_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"SPA?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSignalSpacing_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"SPA %.2lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrResolution_RangeTableCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

    switch (osaType)
    {
    	case MS97XX_VAL_OSA_TYPE_MS9710C:
    		tblPtr=&attrResolutionRangeTable_10C;
    	break;

    	case MS97XX_VAL_OSA_TYPE_MS9720:
    		tblPtr=&attrResolutionRangeTable_20;
    	break;

    	case MS97XX_VAL_OSA_TYPE_MS9726:
    		tblPtr=&attrResolutionRangeTable_26;
    	break;

    	case MS97XX_VAL_OSA_TYPE_MS9728:
    		tblPtr=&attrResolutionRangeTable_28;
    	break;

    	default:
			if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9720)
	    		tblPtr=&attrResolutionRangeTable_20;
	    	else
	    		tblPtr=&attrResolutionRangeTable_10B;
    	break;
    }
	
Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrResolution_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"RES?",0));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrResolution_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"RES %0.2f%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;

	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

    if (osaType==MS97XX_VAL_OSA_TYPE_CARD)
  		tblPtr=&attrVideoBandWidthRangeTable_2628;
  	else
  		tblPtr=&attrVideoBandWidthRangeTable;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadDiscreteViReal64Attribute(vi,io,channelName,attributeId,value,"VBW?",0));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"VBW %.0f%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrActualResolution_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"ARED?",0));
    
Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrAutoMeasuring_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[15];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(viPrintf(io,"AUT?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"MEASURING")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrAutoResolution_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(viPrintf(io,"ATRS?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAutoResolution_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ATRS %s%s",
	                    (value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExternalTriggerDelay_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"TDL?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExternalTriggerDelay_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"TDL %.0f%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrModulationMode_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;

	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
  		tblPtr=&attrModulationModeRangeTable_20;
  	else
  		tblPtr=&attrModulationModeRangeTable_10;


Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrModulationMode_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MDM?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrModulationMode_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MDM"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPeakHold_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"PHD?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPeakHold_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"PHD %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPointAverage_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"AVT?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPointAverage_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	if (value==0)
		checkErr(viPrintf(io,"AVT OFF%s",value,terminator));
	else
		checkErr(viPrintf(io,"AVT %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSamplingPoints_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

    if (osaType==MS97XX_VAL_OSA_TYPE_CARD)
  		tblPtr=&attrSamplingPointsRangeTable_2628;
  	else
  		tblPtr=&attrSamplingPointsRangeTable;


Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSamplingPoints_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"MPT?",0));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrSamplingPoints_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"MPT %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrShowActualResolution_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(viPrintf(io,"ARES?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrShowActualResolution_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ARES %s%s",
	                    (value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSmoothing_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"SMT?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSmoothing_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	if (value==0)
		checkErr(viPrintf(io,"SMT OFF%s",value,terminator));
	else
		checkErr(viPrintf(io,"SMT %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSweepAverage_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM |
										MS97XX_DISPLAY_MASK_DBM |
										MS97XX_DISPLAY_MASK_DB));

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"AVS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSweepAverage_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	if (value==0)
		checkErr(viPrintf(io,"AVS OFF%s",value,terminator));
	else
		checkErr(viPrintf(io,"AVS %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSweepIntervalTime_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ITM?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSweepIntervalTime_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ITM %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWideDynamicRange_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(viPrintf(io,"DRG?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"HIGH")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWideDynamicRange_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"DRG %s%s",
	                    (value==VI_TRUE ? "HIGH" : "NORMAL"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	IviRangeTablePtr	pRanges;
	ViChar				readBuf[50],*pComma=NULL;
	ViInt32				readBufSize=sizeof(readBuf)-1;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));

	checkErr(viPrintf(io,"ANA?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));

	pComma=strchr(readBuf,',');
	if (pComma!=NULL)
		*pComma=0x00;
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(readBuf,pRanges,value,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"ANA"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrEnvelopeCut_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_ENV));
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ANA?",1));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrEnvelopeCut_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ANA ENV, %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNdbAttenuation_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_NDB));
    
    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ANA?",1));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNdbAttenuation_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	
	checkErr(viPrintf(io,"ANA NDB, %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRms_ReadCallback (ViSession vi,
                                                     ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViReal64 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],commandBuf[10];
	ViInt32				readBufSize=sizeof(readBuf)-1,
						commandBufSize=sizeof(commandBuf)-1;
	IviRangeTablePtr	pRanges;
	ViInt32				cut;
	ViReal64			realCut,coeff;
	ViBoolean			cached;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_RMS));
	
	checkErr(viPrintf(io,"ANA?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	checkErr(Scan(readBuf,"%s[w*t44q],%f,%f",commandBufSize,commandBuf,&realCut,&coeff));

/*  checkErr(viScanf(io,"%*[^,],%lf,%lf",&realCut,&coeff)); */

	cut=(ViInt32)realCut;

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_RMS_COEFFICIENT,&pRanges));
	checkErr(Ivi_GetViReal64EntryFromValue(coeff,pRanges,VI_NULL,VI_NULL,&coeff,VI_NULL,VI_NULL,VI_NULL));

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_RMS_CUT,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32 (vi, "", MS97XX_ATTR_RMS_CUT, IVI_VAL_SET_CACHE_ONLY, cut));
	}
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_RMS_COEFFICIENT,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_RMS_COEFFICIENT,IVI_VAL_SET_CACHE_ONLY,coeff));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_RMS_CUT:
			*((ViInt32*)value)=cut;
		break;

		case MS97XX_ATTR_RMS_COEFFICIENT:
			*value=coeff;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRms_WriteCallback (ViSession vi,
                                                      ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRmsAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	ViBoolean	dirty,locked;
	ViInt32		cut;
	ViReal64	coeff;
	IviRangeTablePtr pRanges;

	if (attributeId==MS97XX_ATTR_RMS_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_RMS_CUT,0,&cut));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_RMS_COEFFICIENT,0,&coeff));
			
		checkErr(viPrintf(io,"ANA RMS,%ld,%.2lf%s",cut,coeff,terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_DIRTY, IVI_VAL_SET_CACHE_ONLY, VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSmsrSideMode_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_SMSR));

    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"ANA?",1));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSmsrSideMode_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"ANA SMSR,"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrThresholdCut_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;
	
	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_ANALYSIS_MODE,0,MS97XX_VAL_ANALYSIS_MODE_THR));

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ANA?",1));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrThresholdCut_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ANA THR, %ld%s",value,terminator));

Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrAnalysisMode_RangeTableCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;

	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
  		tblPtr=&attrAnalysisModeRangeTable_20;
  	else
  		tblPtr=&attrAnalysisModeRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDiskette_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;

	ViChar				readBuf[BUFFER_SIZE],
						optionBuf[10],
						idFormatBuf[10],
						sizeBuf[10],
						numFormatBuf[10],
						instFormatBuf[10];

	ViInt32				readBufSize=sizeof(readBuf)-1,
						optionBufSize=sizeof(optionBuf)-1,
						idFormatBufSize=sizeof(idFormatBuf)-1,
						sizeBufSize=sizeof(sizeBuf)-1,
						numFormatBufSize=sizeof(numFormatBuf)-1,
						instFormatBufSize=sizeof(instFormatBuf)-1;

	ViInt32				osaType,option,idFormat,size,numFormat,instFormat;
	
	ViBoolean			cached;
	
	IviRangeTablePtr	pRanges;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	checkErr(viPrintf(io,"FOPT?%s",terminator));
	
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
	{
		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*q]",
					  optionBufSize,optionBuf,
					  idFormatBufSize,idFormatBuf,
					  numFormatBufSize,numFormatBuf,
					  instFormatBufSize,instFormatBuf,
					  sizeBufSize,sizeBuf));

/*
  The following call should be used in place of the Scan function, but
  a bug in VISA is causing viScanf to fail to parse the first %#[^,] tag.
  		checkErr(viScanf(io,"%#[^,],%#[^,],%#[^,],%#[^,],%#s",
  				 &optionBufSize,optionBuf,
  				 &idFormatBufSize,idFormatBuf,
  		 		 &numFormatBufSize,numFormatBuf,
  		 	 	 &instFormatBufSize,instFormatBuf,
  		 	 	 &sizeBufSize,sizeBuf));
*/
	}
  	else
  	{
		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*q]",
					  optionBufSize,optionBuf,
					  idFormatBufSize,idFormatBuf,
					  sizeBufSize,sizeBuf));

/*	
  		checkErr(viScanf(io,"%#[^,],%#[^,],%#s",
  				 &optionBufSize,optionBuf,
  				 &idFormatBufSize,idFormatBuf,
  			 	 &sizeBufSize,sizeBuf));
*/
  	}
  	
	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_OPTION,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(optionBuf,pRanges,&option,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(idFormatBuf,pRanges,&idFormat,VI_NULL,VI_NULL,VI_NULL,VI_NULL));
	
	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_DISKETTE_SIZE,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(sizeBuf,pRanges,&size,VI_NULL,VI_NULL,VI_NULL,VI_NULL));
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_FILE_OPTION,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_OPTION,IVI_VAL_SET_CACHE_ONLY,option));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_FILE_ID_FORMAT,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,IVI_VAL_SET_CACHE_ONLY,idFormat));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_DISKETTE_SIZE,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DISKETTE_SIZE,IVI_VAL_SET_CACHE_ONLY,size));
	}

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
	{
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromString(numFormatBuf,pRanges,&numFormat,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromString(instFormatBuf,pRanges,&instFormat,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

		if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_FILE_NUMERIC_FORMAT,&cached), cached==VI_FALSE)
		{
			checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,IVI_VAL_SET_CACHE_ONLY,numFormat));
		}

		if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,&cached), cached==VI_FALSE)
		{
			checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,IVI_VAL_SET_CACHE_ONLY,instFormat));
		}
	}
	
	switch (attributeId)
	{
		case MS97XX_ATTR_FILE_OPTION:
			*value=option;
		break;
		
		case MS97XX_ATTR_FILE_ID_FORMAT:
			*value=idFormat;
		break;

		case MS97XX_ATTR_DISKETTE_SIZE:
			*value=size;
		break;
		
		case MS97XX_ATTR_FILE_NUMERIC_FORMAT:
			*value=numFormat;
		break;

		case MS97XX_ATTR_FILE_INSTRUMENT_FORMAT:
			*value=instFormat;
		break;
		
		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDisketteAttr_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	ViBoolean	dirty,locked;
	ViInt32		osaType,option,idFormat,size,numFormat,instFormat;
	ViString	optionStr,idFormatStr,sizeStr,numFormatStr,instFormatStr;

	IviRangeTablePtr pRanges;

	if (attributeId==MS97XX_ATTR_DISKETTE_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_OPTION,0,&option));
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,0,&idFormat));
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DISKETTE_SIZE,0,&size));

		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_OPTION,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(option,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&optionStr,VI_NULL));
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_ID_FORMAT,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(idFormat,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&idFormatStr,VI_NULL));
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_DISKETTE_SIZE,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(size,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&sizeStr,VI_NULL));

	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

		if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    	MS97XX_VAL_OSA_TYPE_MS9720)
	    {
			checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,0,&numFormat));
			checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,0,&instFormat));

			checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_NUMERIC_FORMAT,&pRanges));
			checkErr(Ivi_GetViInt32EntryFromValue(numFormat,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&numFormatStr,VI_NULL));
			checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,&pRanges));
			checkErr(Ivi_GetViInt32EntryFromValue(instFormat,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&instFormatStr,VI_NULL));
			
			checkErr(viPrintf(io,"FOPT %s,%s,%s,%s,%s%s",optionStr,idFormatStr,numFormatStr,instFormatStr,sizeStr,terminator));
		}
		else
		{
			checkErr(viPrintf(io,"FOPT %s,%s,%s%s",optionStr,idFormatStr,sizeStr,terminator));
		}
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}


Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDiskette_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DISKETTE_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	ViBoolean	dirty,locked;
	ViInt32		mode,method,fitMethod,sent;
	ViReal64	fitSpan,maskSpan,pin,pout,nf,band,filter,polar;
	ViAddr		sendBuf=VI_NULL;
	ViChar		*pQuery=VI_NULL;

	if (attributeId==MS97XX_ATTR_AMP_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_CALC_MODE,0,&mode));
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_MEASURE_METHOD,0,&method));
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_AMP_FITTING_METHOD,0,&fitMethod));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FITTING_SPAN,0,&fitSpan));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_MASKED_SPAN,0,&maskSpan));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_PIN_LOSS,0,&pin));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POUT_LOSS,0,&pout));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_NF_CAL,0,&nf));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_BAND_PASS_CUT,0,&band));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_FILTER_BANDWIDTH,0,&filter));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_AMP_POLAR_LOSS,0,&polar));

		checkErr(MS97xx_GetReadBuffer(vi,2048,&sendBuf));
		pQuery=(ViChar*)sendBuf;
			
		sprintf(pQuery,"AP AMP,PRM,%ld,%ld,%ld,%.2lf,%.2lf,%.2lf,%.2lf,%.3lf,%.2lf,%.2lf,%.2lf%s",
						mode,method,fitMethod,fitSpan,maskSpan,pin,pout,nf,band,filter,polar,terminator);
		checkErr(viWrite(io,pQuery,strlen(pQuery),&sent));

		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpCalStatus_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP? AMP,CAL",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpInt_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xxAttrAmpReal_ReadCallback(vi,io,channelName,attributeId,(ViReal64*)value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpInt_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpMemory_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP? AMP,MSL",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpMemory_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP AMP,MSL,"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpReal_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	ViAddr				readBuf=VI_NULL;
	ViChar				*pResponse=VI_NULL,
						commandBuf[10];
	ViInt32				readBufSize=2048,
						commandBufSize=sizeof(commandBuf)-1;

	ViInt32				mode,method,fitMethod,received;
	ViReal64			fitSpan,maskSpan,pin,pout,nf,
						band,filter,polar;

	IviRangeTablePtr	pRanges;
	ViReal64   			doubleVal;
	ViBoolean			cached;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));

	checkErr(viPrintf(io,"AP? AMP,PRM%s",terminator));

	checkErr(MS97xx_GetReadBuffer(vi,readBufSize,&readBuf));
	pResponse=(ViChar*)readBuf;

	checkErr(viRead(io,pResponse,readBufSize,&received));
	checkErr(Scan(pResponse,"%s[w*t44q],%s[w*t44q],%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f",
				  commandBufSize,commandBuf,commandBufSize,commandBuf,
				  &mode,&method,&fitMethod,
				  &fitSpan,&maskSpan,&pin,&pout,&nf,&band,&filter,&polar));

/*
  Replaced due to VISA bug.
  	checkErr(viScanf(io,"%*[^,],%#[^,],%ld,%ld,%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
  					 &commandBufSize,commandBuf,
  					 &mode,&method,&fitMethod,
  					 &fitSpan,&maskSpan,&pin,&pout,&nf,&band,&filter,&polar));
*/

	if (strcmp(commandBuf,"PRM")!=0)
		return MS97XX_ERROR_INCORRECT_MODE;
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_CALC_MODE,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViInt32 (vi, "", MS97XX_ATTR_AMP_CALC_MODE, IVI_VAL_SET_CACHE_ONLY, mode);
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_MEASURE_METHOD,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViInt32 (vi, "", MS97XX_ATTR_AMP_MEASURE_METHOD, IVI_VAL_SET_CACHE_ONLY, method);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_FITTING_METHOD,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViInt32 (vi, "", MS97XX_ATTR_AMP_FITTING_METHOD, IVI_VAL_SET_CACHE_ONLY, fitMethod);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_FITTING_SPAN,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_FITTING_SPAN, IVI_VAL_SET_CACHE_ONLY, fitSpan);
		
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_MASKED_SPAN,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_MASKED_SPAN, IVI_VAL_SET_CACHE_ONLY, maskSpan);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_PIN_LOSS,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_PIN_LOSS, IVI_VAL_SET_CACHE_ONLY, pin);
		
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_POUT_LOSS,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_POUT_LOSS, IVI_VAL_SET_CACHE_ONLY, pout);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_NF_CAL,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_NF_CAL, IVI_VAL_SET_CACHE_ONLY, nf);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_BAND_PASS_CUT,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_BAND_PASS_CUT, IVI_VAL_SET_CACHE_ONLY, band);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_FILTER_BANDWIDTH,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_FILTER_BANDWIDTH, IVI_VAL_SET_CACHE_ONLY, filter);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_AMP_POLAR_LOSS,&cached), cached==VI_FALSE)
		Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_AMP_POLAR_LOSS, IVI_VAL_SET_CACHE_ONLY, polar);

	switch (attributeId)
	{
		case MS97XX_ATTR_AMP_CALC_MODE:
			*((ViInt32*)value)=mode;
		break;
		
		case MS97XX_ATTR_AMP_MEASURE_METHOD:
			*((ViInt32*)value)=method;
		break;
		
		case MS97XX_ATTR_AMP_FITTING_METHOD:
			*((ViInt32*)value)=fitMethod;
		break;
		
		case MS97XX_ATTR_AMP_FITTING_SPAN:
			*value=fitSpan;
		break;
		
		case MS97XX_ATTR_AMP_MASKED_SPAN:
			*value=maskSpan;
		break;
		
		case MS97XX_ATTR_AMP_PIN_LOSS:
			*value=pin;
		break;
		
		case MS97XX_ATTR_AMP_POUT_LOSS:
			*value=pout;
		break;
		
		case MS97XX_ATTR_AMP_NF_CAL:
			*value=nf;
		break;
		
		case MS97XX_ATTR_AMP_BAND_PASS_CUT:
			*value=band;
		break;
		
		case MS97XX_ATTR_AMP_FILTER_BANDWIDTH:
			*value=filter;
		break;
		
		case MS97XX_ATTR_AMP_POLAR_LOSS:
			*value=polar;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrAmpReal_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_AMP_ATTR_DIRTY,0L,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDfbAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value)
{
	ViStatus			error = VI_SUCCESS;

	ViBoolean			dirty,locked;
	ViInt32				mode,width;
	ViString			modeStr;
	
	IviRangeTablePtr	pRanges;

	if (attributeId==MS97XX_ATTR_DFB_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,0,&mode));
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(mode,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&modeStr,VI_NULL));

		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_NDB_WIDTH,0,&width));

		checkErr(viPrintf(io,"AP DFB,%s,%ld%s",modeStr,width,terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDfb_ReadCallback (ViSession vi, ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],modeBuf[10];
	ViInt32				readBufSize=sizeof(readBuf)-1,
						modeBufSize=sizeof(modeBuf)-1;
	ViInt32				mode,width;
	ViBoolean			cached;
	IviRangeTablePtr	pRanges;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_DFB,VI_TRUE));
	
	checkErr(viPrintf(io,"AP?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%d",
				  modeBufSize,modeBuf,
				  modeBufSize,modeBuf,&width));

/*
    Replaced due to VISA bug.
  	checkErr(viScanf(io,"%*[^,],%#[^,],%ld",&modeBufSize,modeBuf,&width));
*/

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(modeBuf,pRanges,&mode,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_DFB_SIDE_MODE,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_SIDE_MODE,IVI_VAL_SET_CACHE_ONLY,mode));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_DFB_NDB_WIDTH,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_DFB_NDB_WIDTH,IVI_VAL_SET_CACHE_ONLY,width));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_DFB_SIDE_MODE:
			*value=mode;
		break;
		
		case MS97XX_ATTR_DFB_NDB_WIDTH:
			*value=width;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrDfb_WriteCallback (ViSession vi, ViSession io,
                                                      ViConstString channelName,
                                                      ViAttr attributeId,
                                                      ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_DFB_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrFpAxisModeCutLevel_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_FP,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP? AMP,FP",2));

Error:
	return error;
}

ViStatus _VI_FUNC MS97xxSetDfbOFF_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 value)
{
	ViStatus			error = VI_SUCCESS;
  
	checkErr(viPrintf(io,"AP OFF%s",terminator));
	
Error:
	return error;
}

ViStatus _VI_FUNC MS97xxSetWDP_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 value)
{
	ViStatus			error = VI_SUCCESS;
  
	checkErr(viPrintf(io,"WDP VACUUM%s",terminator));
	
Error:
	return error;
}

ViStatus _VI_FUNC MS97xxSetDfb_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 value)
{
	ViStatus			error = VI_SUCCESS;
  
	checkErr(viPrintf(io,"AP DFB%s",terminator));
	
Error:
	return error;
}

ViStatus _VI_FUNC MS97xxAttrFpAxisModeCutLevel_WriteCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"AP FP,%ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLedAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	ViBoolean			dirty,locked;
	ViInt32				width;
	ViReal64			power;

	if (attributeId==MS97XX_ATTR_LED_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_LED_NDB_DOWN_WAVE,0,&width));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_LED_POWER_COMP,0,&power));

		checkErr(viPrintf(io,"AP LED,%ld,%.2lf%s",width,power,terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLedNdbDownWave_ReadCallback (ViSession vi,
                                                                ViSession io,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViInt32				width;
	ViReal64			power;
	ViBoolean			cached;
	
	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LED,VI_TRUE));

	checkErr(viPrintf(io,"AP?%s",terminator));
	checkErr(viScanf(io,"%*[^,],%ld,%lf",&width,&power));

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_LED_NDB_DOWN_WAVE,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_LED_NDB_DOWN_WAVE,IVI_VAL_SET_CACHE_ONLY,width));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_LED_POWER_COMP,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_LED_POWER_COMP,IVI_VAL_SET_CACHE_ONLY,power));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_LED_NDB_DOWN_WAVE:
			*value=width;
		break;
		
		case MS97XX_ATTR_LED_POWER_COMP:
			*((ViReal64*)value)=power;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLedNdbDownWave_WriteCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLedPowerComp_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	MS97xxAttrLedNdbDownWave_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLedPowerComp_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_LED_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNfAttr_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViBoolean value)
{
	ViStatus			error = VI_SUCCESS;

	ViBoolean	dirty,locked;
	ViReal64	fitting,pin,cal,ase,pout;

	if (attributeId==MS97XX_ATTR_NF_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_FITTING_SPAN,0,&fitting));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_PIN_LOSS,0,&pin));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_CAL,0,&cal));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_ASE_CAL,0,&ase));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_NF_POUT_LOSS,0,&pout));
			
		checkErr(viPrintf(io,"AP NF,PRM,%.2lf,%.2lf,%.3lf,%.2lf,%.2lf%s",fitting,pin,cal,ase,pout,
		                  terminator));
		                  
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_DIRTY,
										   IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNfCutLevel_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP? NF,SLV",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNfCutLevel_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"AP NF,SLV,%ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNf_ReadCallback (ViSession vi, ViSession io,
                                                    ViConstString channelName,
                                                    ViAttr attributeId,
                                                    ViReal64 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViReal64   			fitting,pin,cal,ase,pout;
	ViBoolean			cached;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));

	checkErr(viPrintf(io,"AP? NF,PRM%s",terminator));
	checkErr(viScanf(io,"%*[^,],%*[^,],%lf,%lf,%lf,%lf,%lf",
	                    &fitting,&pin,&cal,&ase,&pout));
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_NF_FITTING_SPAN,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_NF_FITTING_SPAN, IVI_VAL_SET_CACHE_ONLY, fitting));
	}
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_NF_PIN_LOSS,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_PIN_LOSS,IVI_VAL_SET_CACHE_ONLY,pin));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_NF_CAL,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_CAL,IVI_VAL_SET_CACHE_ONLY,cal));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_NF_ASE_CAL,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_ASE_CAL,IVI_VAL_SET_CACHE_ONLY,ase));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_NF_POUT_LOSS,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_NF_POUT_LOSS,IVI_VAL_SET_CACHE_ONLY,pout));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_NF_FITTING_SPAN:
			*value=fitting;
		break;

		case MS97XX_ATTR_NF_PIN_LOSS:
			*value=pin;
		break;

		case MS97XX_ATTR_NF_CAL:
			*value=cal;
		break;

		case MS97XX_ATTR_NF_ASE_CAL:
			*value=ase;
		break;

		case MS97XX_ATTR_NF_POUT_LOSS:
			*value=pout;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrNf_WriteCallback (ViSession vi, ViSession io,
                                                     ViConstString channelName,
                                                     ViAttr attributeId,
                                                     ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_NF_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPmdCoupling_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViInt32 	osaType;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"AP?",1));
	else
		checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"MCF?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPmdCoupling_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;
	ViInt32 	osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		checkErr(viPrintf(io,"AP PMD,%.2lf%s",value,terminator));
	else
		checkErr(viPrintf(io,"MCF %.2lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrScreenMode_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO |
										MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"SMD?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrScreenMode_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"SMD"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO |
										MS97XX_MODE_MASK_NF |
										MS97XX_MODE_MASK_PMD,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSL?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSL"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSnrAttr_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViBoolean value)
{
	ViStatus			error = VI_SUCCESS;

	ViBoolean			dirty,locked;

	ViString			dirStr;
	ViInt32				dir;
	ViReal64			dw;
	ViBoolean			norm;
	IviRangeTablePtr	pRanges;

	ViInt32 osaType;

	if (attributeId==MS97XX_ATTR_SNR_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_SNR_DIR,0,&dir));
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_WDM_SNR_DIR,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(dir,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&dirStr,VI_NULL));

		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_SNR_DW,0,&dw));

	    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));
		if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
		{
			checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_SNR_NORM,0,&norm));
			checkErr(viPrintf(io,"AP WDM,SNR,%s,%.2lf,%s%s",
				dirStr,dw,(norm==VI_TRUE ? "ON" : "OFF"),terminator));
		}
		else
			checkErr(viPrintf(io,"AP WDM,SNR,%s,%.2lf%s",dirStr,dw,terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTableAttr_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	ViBoolean			dirty,locked;

	ViString			dirStr;
	ViInt32				dir;
	ViReal64			dw;
	ViBoolean			norm;
	IviRangeTablePtr	pRanges;

	if (attributeId==MS97XX_ATTR_TABLE_ATTR_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,0,&dir));
		checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,&pRanges));
		checkErr(Ivi_GetViInt32EntryFromValue(dir,pRanges,VI_NULL,VI_NULL,VI_NULL,VI_NULL,&dirStr,VI_NULL));

		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_TABLE_DW,0,&dw));
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_TABLE_NORM,0,&norm));

		checkErr(viPrintf(io,"AP WDM,TBL,%s,%.2lf,%s%s",dirStr,dw,(norm==VI_TRUE ? "ON" : "OFF"),terminator));
							
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,IVI_VAL_SET_CACHE_ONLY,VI_FALSE));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmCutLevel_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"AP? WDM,SLV",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmCutLevel_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"AP WDM,SLV,%ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmMode_ReadCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[BUFFER_SIZE];
	ViInt32		readBufSize=sizeof(readBuf)-1;
	
	if (MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE) != VI_SUCCESS)
	{
		*value=MS97XX_VAL_WDM_NONE;
		return error;
	}

	checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"AP?",1));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmPeakType_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(MS97xx_ReadDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP? WDM,PKT",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmPeakType_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(
		vi,io,channelName,attributeId,value,"AP WDM,PKT,"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"AP? WDM,REL",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"AP WDM,REL,%ld",value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDir_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],
						dirBuf[10],
						dwBuf[15],
						normBuf[10];
	ViInt32				readBufSize=sizeof(readBuf)-1,
						dirBufSize=sizeof(dirBuf)-1,
						dwBufSize=sizeof(dwBuf)-1,
						normBufSize=sizeof(normBuf)-1;
	ViInt32				dir,osaType;
	ViReal64			dw;
	ViBoolean			cached,norm;
	IviRangeTablePtr	pRanges;
	
	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

    checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));

	checkErr(viPrintf(io,"AP? WDM,SNR%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
	{
/*
  		checkErr(viScanf(io,"%*[^,],%*[^,],%#[^,],%lf,%#s",
  			&dirBufSize,dirBuf,&dw,&normBufSize,normBuf));
*/

		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*q]",
					  dirBufSize,dirBuf,dirBufSize,dirBuf,dirBufSize,dirBuf,
					  dwBufSize,dwBuf,normBufSize,normBuf));
		
		norm=(strcmp(normBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

		if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_SNR_NORM,&cached), cached==VI_FALSE)
		{
			checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_SNR_NORM,IVI_VAL_SET_CACHE_ONLY,norm));
		}
	}
	else
	{
		checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*t44q],%s[w*q]",
					  dirBufSize,dirBuf,dirBufSize,dirBuf,dirBufSize,dirBuf,
					  dwBufSize,dwBuf));

/*  	checkErr(viScanf(io,"%*[^,],%*[^,],%#[^,],%lf",&dirBufSize,dirBuf,&dw)); */
	}

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_WDM_SNR_DIR,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(dirBuf,pRanges,&dir,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

	dw=atof(dwBuf);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_SNR_DIR,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_SNR_DIR,IVI_VAL_SET_CACHE_ONLY,dir));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_SNR_DW,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_SNR_DW,IVI_VAL_SET_CACHE_ONLY,dw));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_WDM_SNR_DIR:
			*value=dir;
		break;
		
		case MS97XX_ATTR_WDM_SNR_DW:
			*((ViReal64*)value)=dw;
		break;

		case MS97XX_ATTR_WDM_SNR_NORM:
			*((ViBoolean*)value)=norm;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDir_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_ReadCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	
	MS97xxAttrWdmSnrDir_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_WriteCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableDir_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],
						dirBuf[10],
						normBuf[10];
	ViInt32				readBufSize=sizeof(readBuf)-1,
						dirBufSize=sizeof(dirBuf)-1,
						normBufSize=sizeof(normBuf)-1;
	ViInt32				dir;
	ViReal64			dw;
	ViBoolean			norm;
	ViBoolean			cached;
	IviRangeTablePtr	pRanges;
	
	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

	checkErr(viPrintf(io,"AP? WDM,TBL%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	checkErr(Scan(readBuf,"%s[w*t44q],%s[w*t44q],%s[w*t44q],%f,%s[w*q]",
				  dirBufSize,dirBuf,dirBufSize,dirBuf,dirBufSize,dirBuf,
				  &dw,normBufSize,normBuf));

/*
  Removed due to VISA bug.
  	checkErr(viScanf(io,"%*[^,],%*[^,],%#[^,],%lf,%#s",
  	                    &dirBufSize,dirBuf,&dw,&normBufSize,normBuf));
*/

	checkErr(Ivi_GetAttrRangeTable(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,&pRanges));
	checkErr(Ivi_GetViInt32EntryFromString(dirBuf,pRanges,&dir,VI_NULL,VI_NULL,VI_NULL,VI_NULL));

	norm=((strcmp(normBuf,"ON")==0) ? VI_TRUE : VI_FALSE);

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_TABLE_DIR,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViInt32(vi,"",MS97XX_ATTR_WDM_TABLE_DIR,IVI_VAL_SET_CACHE_ONLY,dir));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_TABLE_DW,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_WDM_TABLE_DW,IVI_VAL_SET_CACHE_ONLY,dw));
	}

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_WDM_TABLE_NORM,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_WDM_TABLE_NORM,IVI_VAL_SET_CACHE_ONLY,norm));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_WDM_TABLE_DIR:
			*value=dir;
		break;
		
		case MS97XX_ATTR_WDM_TABLE_DW:
			*((ViReal64*)value)=dw;
		break;

		case MS97XX_ATTR_WDM_TABLE_NORM:
			*((ViBoolean*)value)=norm;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableDir_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableDw_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	MS97xxAttrWdmTableDir_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);
	
Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableDw_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableNorm_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;

	MS97xxAttrWdmTableDir_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmTableNorm_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViBoolean(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmThreshold_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"AP? WDM,TCL",2));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmThreshold_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"AP WDM,TCL,%.1lf%s",value,terminator));

Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrWdmRefPeak_RangeTableCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrWdmRefPeakRangeTable_20;
	else
		tblPtr=&attrWdmRefPeakRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrDw_RangeTableCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrWdmSnrDwRangeTable_20;
	else
		tblPtr=&attrWdmSnrDwRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrMeasureModeRangeTable_20;
	else
		tblPtr=&attrMeasureModeRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;
	ViChar				readBuf[BUFFER_SIZE],*pComma;
	ViInt32				readBufSize=sizeof(readBuf)-1,loop;
	IviRangeTablePtr	pRanges;
	ViInt32				osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
	{
	    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSR?",0));
	}
	else
	{
		checkErr(viPrintf(io,"AP?%s",terminator));
		checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
		if (strcmp(readBuf,"OFF")==0)
			value=MS97XX_VAL_MEASURE_MODE_SPECTRUM;
		else
		{
			pComma=strchr(readBuf,',');
			if (pComma!=NULL)
				*pComma=0x00;
			checkErr(Ivi_GetAttrRangeTable(vi,"",attributeId,&pRanges));
			checkErr(Ivi_GetViInt32EntryFromString(readBuf,pRanges,value,VI_NULL,VI_NULL,VI_NULL,VI_NULL));
		}
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMeasureMode_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
	{
	    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MSR"));
	}
	else
	{
		checkErr(viPrintf(io,"AP OFF%s",terminator));
		if (value!=MS97XX_VAL_MEASURE_MODE_SPECTRUM)
			checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"AP "));
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSendHeader_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(viPrintf(io,"HEAD?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=((strcmp(readBuf,"ON")==0) ? VI_TRUE : VI_FALSE);
	
Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSendHeader_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"HEAD %s%s",(value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTerminator_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"TRM?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTerminator_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"TRM %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrEventStatusEnable_ReadCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"*ESE?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrEventStatusEnable_WriteCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"*ESE %u%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrEventStatus_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"*ESR?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESE1?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ESE1 %u%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus1_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESR1?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESE2?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ESE2 %u%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus2_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESR2?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3Enable_ReadCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESE3?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3Enable_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"ESE3 %u%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrExtendedStatus3_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ESR3?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrServiceRequestEnable_ReadCallback (ViSession vi,
                                                                      ViSession io,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"*SRE?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrServiceRequestEnable_WriteCallback (ViSession vi,
                                                                       ViSession io,
                                                                       ViConstString channelName,
                                                                       ViAttr attributeId,
                                                                       ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"*SRE %u%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrStatusByte_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"*STB?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrError_ReadCallback (ViSession vi,
                                                       ViSession io,
                                                       ViConstString channelName,
                                                       ViAttr attributeId,
                                                       ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"ERR?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	        error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType,trace,denominator,scale;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9710)
	{
	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_CURRENT_TRACE,0,&trace));
	    if (trace==MS97XX_VAL_CURRENT_TRACE_A_B ||
	    	trace==MS97XX_VAL_CURRENT_TRACE_B_A)
	    {
            checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_LEVEL_SCALE,0,&scale));
            if (scale==MS97XX_VAL_LEVEL_SCALE_LOG)
                tblPtr=&attrMarkerLevelRangeTable_dB;
            else
                tblPtr=&attrMarkerLevelRangeTable_Percent;
	    }
	    else
	    {
            checkErr(Ivi_GetAttributeViInt32 (vi,channelName,MS97XX_ATTR_LEVEL_SCALE,0,&scale));
            if (scale==MS97XX_VAL_LEVEL_SCALE_LOG)
                tblPtr=&attrMarkerLevelRangeTable_dBm;
            else
                tblPtr=&attrMarkerLevelRangeTable_W;
        }

	}
	else
	{
	    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_ACTIVE_TRACE,0,&trace));

	    switch (trace)
	    {
	    	case 2:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_2_DENOMINATOR,0,&denominator));
	    	break;
	    	
	    	case 3:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_3_DENOMINATOR,0,&denominator));
	    	break;
	    	
	    	default:
			    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_TRACE_1_DENOMINATOR,0,&denominator));
	    	break;
	    }
	    
	    if (denominator==MS97XX_VAL_TRACE_NONE)
            tblPtr=&attrMarkerLevelRangeTable_dBm;
	    else
            tblPtr=&attrMarkerLevelRangeTable_dB;
	}


Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,
             attributeId==MS97XX_ATTR_MARKER_C_LEVEL ? "MKC?" : "MKD?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerLevel_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	pRanges = VI_NULL;
	ViInt32 magnitude;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(Ivi_GetAttrRangeTable (vi, "", attributeId, &pRanges));
	if (pRanges==&attrMarkerLevelRangeTable_W)
	{
		checkErr(viPrintf(io,attributeId==MS97XX_ATTR_MARKER_C_LEVEL ? "MKC " : "MKD "));
		magnitude=log10(value);
		if (value<-9)
			checkErr(viPrintf(io,"%.3lf PW%s",value*1E12,terminator));
		else if (value<-6)
			checkErr(viPrintf(io,"%.3lf NW%s",value*1E9,terminator));
		else if (value<-3)
			checkErr(viPrintf(io,"%.3lf UW%s",value*1E6,terminator));
		else if (value<0)
			checkErr(viPrintf(io,"%.3lf MW%s",value*1E3,terminator));
		else
			checkErr(viPrintf(io,"%.3lf W%s",value,terminator));
	}
	else if (pRanges==&attrMarkerLevelRangeTable_dBm)
	{
	    checkErr(viPrintf(io,attributeId==MS97XX_ATTR_MARKER_C_LEVEL ? "MKC %.1lfDBM%s" : "MKD %.1lfDBM%s",
    	                  value,terminator));
	}
	else if (pRanges==&attrMarkerLevelRangeTable_dB)
	{
	    checkErr(viPrintf(io,attributeId==MS97XX_ATTR_MARKER_C_LEVEL ? "MKC %.1lfDB%s" : "MKD %.1lfDB%s",
    	                  value,terminator));
	}
	else
	{
	    checkErr(viPrintf(io,attributeId==MS97XX_ATTR_MARKER_C_LEVEL ? "MKC %.1lf%s" : "MKD %.1lf%s",
    	                  value,terminator));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_RangeTableCallback (ViSession vi,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViReal64            start=0.0,stop=0.0,temp=0.0;
	ViBoolean 			ignore;

    checkErr(Ivi_GetStoredRangeTablePtr(vi,attributeId,&tblPtr));
    
    checkErr(Ivi_GetAttributeViReal64(vi,channelName,MS97XX_ATTR_START_WAVELENGTH,0,&start));
    checkErr(Ivi_GetAttributeViReal64(vi,channelName,MS97XX_ATTR_STOP_WAVELENGTH,0,&stop));

    if (start>stop)
    {
      temp=start;
      start=stop;
      stop=temp;
    }

	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,&ignore));
	if (ignore==VI_TRUE)
	    checkErr(Ivi_SetRangeTableEntry(tblPtr,0,600.0,1800.0,0.0,VI_NULL,0));
	else
	    checkErr(Ivi_SetRangeTableEntry(tblPtr,0,start,stop,0.0,VI_NULL,0));

    checkErr(Ivi_SetRangeTableEnd(tblPtr,1));

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,
             attributeId==MS97XX_ATTR_MARKER_A_WAVELENGTH ? "MKA?" : "MKB?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerWavelength_WriteCallback (ViSession vi,
                                                                   ViSession io,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(viPrintf(io,attributeId==MS97XX_ATTR_MARKER_A_WAVELENGTH ? "MKA %.4lf%s" : "MKB %.4lf%s",
                      value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerUnits_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MKV?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrMarkerUnits_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"MKV"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTraceMarker_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViReal64	wavelength,level;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));

	checkErr(viPrintf(io,"TMK?%s",terminator));
	checkErr(viScanf(io,"%lf,%lf",&wavelength,&level));

	if (attributeId==MS97XX_ATTR_TRACE_MARKER_WAVELENGTH)
	{
	  *value=wavelength;
	}
	else
	{
	  *value=level;
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTraceMarkerWavelength_WriteCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"TMK %.4lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrZoneMarkerSpan_RangeTableCallback (ViSession vi,
                                                                      ViConstString channelName,
                                                                      ViAttr attributeId,
                                                                      IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViBoolean			ignore=VI_FALSE;
	ViReal64            start=0.0,stop=0.0,temp=0.0,zoneCenter=0.0,zoneSpan=0.0;

    checkErr(Ivi_GetStoredRangeTablePtr(vi,attributeId,&tblPtr));
    
    checkErr(Ivi_GetAttributeViReal64(vi,channelName,MS97XX_ATTR_START_WAVELENGTH,0,&start));
    checkErr(Ivi_GetAttributeViReal64(vi,channelName,MS97XX_ATTR_STOP_WAVELENGTH,0,&stop));

    if (start>stop)
    {
      temp=start;
      start=stop;
      stop=temp;
    }

    checkErr(Ivi_GetAttributeViReal64(vi,channelName,MS97XX_ATTR_ZONE_MARKER_CENTER,0,&zoneCenter));
    if (start!=stop)
    {
      if (zoneCenter >= (stop-start)/2.0)
        zoneSpan=stop-zoneCenter;
      else
        zoneSpan=zoneCenter-start;
    }
    
	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,&ignore));
	if (ignore==VI_TRUE)
	    checkErr(Ivi_SetRangeTableEntry(tblPtr,0,0.0,1200.0,0.0,VI_NULL,0));
	else
	    checkErr(Ivi_SetRangeTableEntry(tblPtr,0,0.0,zoneSpan,0.0,VI_NULL,0));

    checkErr(Ivi_SetRangeTableEnd(tblPtr,1));

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrZoneMarker_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViReal64   			center,span;
	ViBoolean			cached;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(viPrintf(io,"ZMK? WL%s",terminator));
	checkErr(viScanf(io,"%*[^,],%lf,%lf",&center,&span));

	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_ZONE_MARKER_CENTER,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,VI_TRUE));
		checkErr(Ivi_SetAttributeViReal64 (vi, "", MS97XX_ATTR_ZONE_MARKER_CENTER, IVI_VAL_SET_CACHE_ONLY, center));
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,VI_FALSE));
	}
	
	if (Ivi_AttributeIsCached(vi,channelName,MS97XX_ATTR_ZONE_MARKER_SPAN,&cached), cached==VI_FALSE)
	{
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,VI_TRUE));
		checkErr(Ivi_SetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN,IVI_VAL_SET_CACHE_ONLY,span));
		checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_INST_OUT_OF_RANGE,0,VI_FALSE));
	}

	switch (attributeId)
	{
		case MS97XX_ATTR_ZONE_MARKER_CENTER:
			*value=center;
		break;

		case MS97XX_ATTR_ZONE_MARKER_SPAN:
			*value=span;
		break;

		default:
		break;
	}

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_DIRTY,
				IVI_VAL_SET_CACHE_ONLY,VI_FALSE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrZoneMarker_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViReal64(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_DIRTY,0,VI_TRUE));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_VISIBLE,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrZoneMarkerWl_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;
	ViBoolean	dirty,locked;
	ViReal64	center,span;

	if (attributeId==MS97XX_ATTR_ZONE_MARKER_WL_LOCKED)
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_DIRTY,0,&dirty));
		locked=value;
	}
	else
	{
		checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_ZONE_MARKER_WL_LOCKED,0,&locked));
		dirty=value;
	}
	
	if (locked==VI_FALSE && (dirty==VI_TRUE || MS97xx_CachingIsOn(vi)==VI_FALSE))
	{
		/* If dirty and (unlocking or already unlocked),
		   perform a write and reset to clean. */
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_CENTER,0,&center));
		checkErr(Ivi_GetAttributeViReal64(vi,"",MS97XX_ATTR_ZONE_MARKER_SPAN,0,&span));
			
		checkErr(viPrintf(io,"ZMK WL,%.3lf,%.3lf%s",center,span,terminator));
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrFirstPmdPeak_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));
	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"FPK?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrFirstPmdPeak_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"FPK %.4lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLastPmdPeak_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));
	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"LPK?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLastPmdPeak_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"LPK %.4lf%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLightOutput_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(viPrintf(io,"OPT?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLightOutput_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"OPT %s%s",(value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLongTermDataSize_ReadCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"LTSD? DTA",1));

Error:
	return error;
}


static ViStatus _VI_FUNC MS97xxAttrLongTermSampleTime_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"SPT?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLongTermSampleTime_WriteCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));
	if (value>0)
		checkErr(viPrintf(io,"SPT %ld%s",value,terminator));
	else
		checkErr(viPrintf(io,"SPT OFF",terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLongTermState_ReadCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"LTSS?",0));

Error:
	return error;
}



static ViStatus _VI_FUNC MS97xxAttrPmdPeakCount_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));
	checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"PCT?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPmdPeakCount_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"PCT %ld%s",value,terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorResult_ReadCallback (ViSession vi,
                                                                    ViSession io,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"PWRR?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorWavelength_ReadCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViReal64 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadViReal64Attribute(vi,io,channelName,attributeId,value,"PWR?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPowerMonitorWavelength_WriteCallback (ViSession vi,
                                                                         ViSession io,
                                                                         ViConstString channelName,
                                                                         ViAttr attributeId,
                                                                         ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViReal64Attribute(vi,io,channelName,attributeId,value,"PWR?"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTlsCalStatus_ReadCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"TLSA?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTlsCalStatus_WriteCallback (ViSession vi,
                                                               ViSession io,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"TLSA"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTlsTrackEnabled_ReadCallback (ViSession vi,
                                                                 ViSession io,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		readBuf[10];
	ViInt32		readBufSize=sizeof(readBuf)-1;

	checkErr(viPrintf(io,"TLST?%s",terminator));
	checkErr(viScanf(io,"%#s",&readBufSize,readBuf));
	*value=(strcmp(readBuf,"ON")==0 ? VI_TRUE : VI_FALSE);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrTlsTrackEnabled_WriteCallback (ViSession vi,
                                                                  ViSession io,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"TLST %s%s",(value==VI_TRUE ? "ON" : "OFF"),terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPeakSearch_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViBoolean	markerVisible;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,0,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);
	
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"PKS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrPeakSearch_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		searchString[5][6]={"PEAK","NEXT","LAST","LEFT","RIGHT"};

	checkErr(viPrintf(io,"PKS %s%s",searchString[value],terminator));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLongTermSpacingDisplay_ReadCallback (ViSession vi,
                                                                        ViSession io,
                                                                        ViConstString channelName,
                                                                        ViAttr attributeId,
                                                                        ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));
    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"SPG?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrLongTermSpacingDisplay_WriteCallback (ViSession vi,
                                                                         ViSession io,
                                                                         ViConstString channelName,
                                                                         ViAttr attributeId,
                                                                         ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(viPrintf(io,"SPG %s%s",
					  (value==MS97XX_VAL_LONG_TERM_SPACING_DISPLAY_FREQUENCY ? "FRQ" : "WAVE"),
					  terminator));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmMode_RangeTableCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrWdmModeRangeTable_20;
	else
		tblPtr=&attrWdmModeRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmMode_WriteCallback (ViSession vi,
                                                          ViSession io,
                                                          ViConstString channelName,
                                                          ViAttr attributeId,
                                                          ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	ViInt32		refPeak=0,osaType;

	switch (value)
	{
		case MS97XX_VAL_WDM_MULTIPEAK:
			checkErr(viPrintf(io,"AP WDM,MPK%s",terminator));
		break;
		
		case MS97XX_VAL_WDM_SNR:
			checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_SNR_DIR));
			checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_SNR_DW));
		
			checkErr(Ivi_GetAttributeViInt32(vi,"",MS97XX_ATTR_OSA_TYPE,0,&osaType));
			if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) == MS97XX_VAL_OSA_TYPE_MS9710)
				checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_SNR_NORM));

			checkErr(Ivi_SetAttributeViBoolean(
				vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,0,VI_TRUE));
		break;
		
		case MS97XX_VAL_WDM_RELATIVE:
			checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_WDM_REF_PEAK,0,&refPeak));
			checkErr(MS97xxAttrWdmRefPeak_WriteCallback(vi,io,channelName,attributeId,refPeak));
		break;
		
		case MS97XX_VAL_WDM_TABLE:
			checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_TABLE_DIR));
			checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_TABLE_DW));
			checkErr(Ivi_InvalidateAttribute(vi,"",MS97XX_ATTR_WDM_TABLE_NORM));

			checkErr(Ivi_SetAttributeViBoolean(
				vi,"",MS97XX_ATTR_TABLE_ATTR_DIRTY,0,VI_TRUE));
		break;
		
		default:
		break;
	}

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxNoLongTermBoolean_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}



static ViStatus _VI_FUNC MS97xxAttrLogScale_CheckCallback (ViSession vi,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC |
										MS97XX_MODE_MASK_WDM |
										MS97XX_MODE_MASK_PMD |
										MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxExcludeLongTermInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxExcludeLongTermReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_FALSE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireSpectrumBoolean_CheckCallback (ViSession vi,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireSpectrumInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireSpectrumReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireLossTestInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireLossTestReal64_CheckCallback (ViSession vi,
                                                                    ViConstString channelName,
                                                                    ViAttr attributeId,
                                                                    ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireAmpInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireAmpReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_AMP,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireDfbInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_DFB,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireFpInt32_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireLedInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LED,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireLedReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LED,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireNfInt32_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireNfReal64_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequirePmdInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequirePmdReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_PMD,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireWdmBoolean_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireWdmInt32_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireWdmReal64_CheckCallback (ViSession vi,
                                                               ViConstString channelName,
                                                               ViAttr attributeId,
                                                               ViReal64 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_WDM,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrScreenMode_CheckCallback (ViSession vi,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO |
										MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrSelectedMem_CheckCallback (ViSession vi,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_ILO |
										MS97XX_MODE_MASK_ISO |
										MS97XX_MODE_MASK_DIR |
										MS97XX_MODE_MASK_RLO |
										MS97XX_MODE_MASK_PMD |
										MS97XX_MODE_MASK_NF,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireLongTermInt32_CheckCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_LTS,VI_TRUE));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrNorm_ReadCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViBoolean *value)
{
	ViStatus	error = VI_SUCCESS;

	MS97xxAttrWdmSnrDir_ReadCallback(vi,io,channelName,attributeId,(ViInt32*)value);

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWdmSnrNorm_WriteCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViBoolean value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViBoolean(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_SNR_ATTR_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrWavelengthCalStatus_ReadCallback (ViSession vi,
                                                                     ViSession io,
                                                                     ViConstString channelName,
                                                                     ViAttr attributeId,
                                                                     ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

    checkErr(MS97xx_ReadViInt32Attribute(vi,io,channelName,attributeId,value,"WCAL?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_RangeTableCallback (ViSession vi,
                                                                   ViConstString channelName,
                                                                   ViAttr attributeId,
                                                                   IviRangeTablePtr *rangeTablePtr)
{
	ViStatus	error = VI_SUCCESS;
	IviRangeTablePtr	tblPtr = VI_NULL;
	ViInt32 osaType;

    checkErr(Ivi_GetAttributeViInt32(vi,channelName,MS97XX_ATTR_OSA_TYPE,0,&osaType));

	if ((osaType & MS97XX_VAL_OSA_TYPE_SERIES) ==
	    MS97XX_VAL_OSA_TYPE_MS9720)
		tblPtr=&attrDisplayModeRangeTable_20;
	else
		tblPtr=&attrDisplayModeRangeTable_10;

Error:
	*rangeTablePtr = tblPtr;
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_ReadCallback (ViSession vi,
                                                             ViSession io,
                                                             ViConstString channelName,
                                                             ViAttr attributeId,
                                                             ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"DMD?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDisplayMode_WriteCallback (ViSession vi,
                                                              ViSession io,
                                                              ViConstString channelName,
                                                              ViAttr attributeId,
                                                              ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_WriteDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"DMD"));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireSplitInt32_CheckCallback (ViSession vi,
                                                                ViConstString channelName,
                                                                ViAttr attributeId,
                                                                ViInt32 value){
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_DBM |
										MS97XX_DISPLAY_MASK_DB));
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxExcludeOverlapInt32_CheckCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViInt32 value){
	ViStatus	error = VI_SUCCESS;
	ViInt32		displayMode;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));

	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM |
										MS97XX_DISPLAY_MASK_DBM |
										MS97XX_DISPLAY_MASK_DB));

	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireNormalInt32_CheckCallback (ViSession vi,
                                                                 ViConstString channelName,
                                                                 ViAttr attributeId,
                                                                 ViInt32 value){
	ViStatus	error = VI_SUCCESS;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));
	
	checkErr(Ivi_DefaultCheckCallbackViInt32(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxRequireNormalReal64_CheckCallback (ViSession vi,
                                                                  ViConstString channelName,
                                                                  ViAttr attributeId,
                                                                  ViReal64 value){
	ViStatus	error = VI_SUCCESS;
	ViInt32		displayMode;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(MS97xx_CheckDisplayMode(vi,MS97XX_DISPLAY_MASK_NRM));

	checkErr(Ivi_DefaultCheckCallbackViReal64(vi,channelName,attributeId,value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRmsCut_ReadCallback (ViSession vi,
                                                        ViSession io,
                                                        ViConstString channelName,
                                                        ViAttr attributeId,
                                                        ViInt32 *value)
{
	ViStatus			error = VI_SUCCESS;

	checkErr(MS97xxAttrRms_ReadCallback(vi,io,channelName,attributeId,(ViReal64*)value));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrRmsCut_WriteCallback (ViSession vi,
                                                         ViSession io,
                                                         ViConstString channelName,
                                                         ViAttr attributeId,
                                                         ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;

	checkErr(Ivi_SetAttributeViInt32(vi,"",attributeId,IVI_VAL_SET_CACHE_ONLY,value));
	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_RMS_DIRTY,0,VI_TRUE));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDipSearch_ReadCallback (ViSession vi,
                                                           ViSession io,
                                                           ViConstString channelName,
                                                           ViAttr attributeId,
                                                           ViInt32 *value)
{
	ViStatus	error = VI_SUCCESS;
	ViBoolean	markerVisible;

	checkErr(MS97xx_CheckMeasureMode(vi,MS97XX_MODE_MASK_SPC,VI_TRUE));
	checkErr(Ivi_GetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,0,&markerVisible));
	if (markerVisible==VI_FALSE)
		checkErr(MS97XX_ERROR_MARKER_NOT_ON);

    checkErr(MS97xx_ReadDiscreteViInt32Attribute(vi,io,channelName,attributeId,value,"DPS?",0));

Error:
	return error;
}

static ViStatus _VI_FUNC MS97xxAttrDipSearch_WriteCallback (ViSession vi,
                                                            ViSession io,
                                                            ViConstString channelName,
                                                            ViAttr attributeId,
                                                            ViInt32 value)
{
	ViStatus	error = VI_SUCCESS;
	ViChar		searchString[5][6]={"DIP","NEXT","LAST","LEFT","RIGHT"};

	checkErr(viPrintf(io,"DPS %s%s",searchString[value],terminator));

	checkErr(Ivi_SetAttributeViBoolean(vi,"",MS97XX_ATTR_TRACE_MARKER_VISIBLE,
									   IVI_VAL_SET_CACHE_ONLY,VI_TRUE));

Error:
	return error;
}


/*****************************************************************************
 * Function: MS97xx_InitAttributes                                                  
 * Purpose:  This function adds attributes to the IVI session, initializes   
 *           instrument attributes, and sets attribute invalidation          
 *           dependencies.                                                   
 *****************************************************************************/
static ViStatus MS97xx_InitAttributes (ViSession vi)
{
    ViStatus            error = VI_SUCCESS;
    IviRangeTablePtr    pWavelengthRange = NULL;
    IviRangeTablePtr    pWavelengthSpanRange = NULL;

    checkErr( Ivi_RangeTableNew (vi, 2, IVI_VAL_RANGED, VI_TRUE, VI_TRUE, &pWavelengthRange));
    checkErr( Ivi_RangeTableNew (vi, 2, IVI_VAL_RANGED, VI_TRUE, VI_TRUE, &pWavelengthSpanRange));
    
        /*- Initialize instrument attributes --------------------------------*/            

    checkErr( Ivi_SetAttributeViInt32 (vi, VI_NULL, MS97XX_ATTR_DRIVER_MAJOR_VERSION, 
                                       0, MS97XX_MAJOR_VERSION));                
    checkErr( Ivi_SetAttributeViInt32 (vi, VI_NULL, MS97XX_ATTR_DRIVER_MINOR_VERSION, 
                                       0, MS97XX_MINOR_VERSION));                
    checkErr( Ivi_SetAttrReadCallbackViString (vi, MS97XX_ATTR_DRIVER_REVISION,          
                                               MS97xxAttrDriverRevision_ReadCallback));

    checkErr( Ivi_SetAttributeViAddr (vi, VI_NULL, IVI_ATTR_OPC_CALLBACK, 0,
                                      MS97xx_WaitForOPCCallback));
    checkErr( Ivi_SetAttributeViAddr (vi, VI_NULL, IVI_ATTR_CHECK_STATUS_CALLBACK, 0,
                                      MS97xx_CheckStatusCallback));
	checkErr( Ivi_SetAttributeViBoolean (vi, VI_NULL, IVI_ATTR_SUPPORTS_WR_BUF_OPER_MODE, 
	                                     0, VI_TRUE));

        /*- Add instrument-specific attributes ------------------------------*/            
	
	checkErr( Ivi_AddAttributeViString (vi, MS97XX_ATTR_ID_QUERY_RESPONSE,
	                                    "MS97XX_ATTR_ID_QUERY_RESPONSE", 
	                                    "ANRITSU,MS97",
	                                    IVI_VAL_NOT_USER_WRITABLE,
	                                    MS97xxAttrIdQueryResponse_ReadCallback,
	                                    VI_NULL));
	                                           
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                    "MS97XX_ATTR_CENTER_WAVELENGTH", 0, 0,
	                                    MS97xxAttrCenterWavelength_ReadCallback,
	                                    MS97xxAttrCenterWavelength_WriteCallback,
	                                    &attrCenterWavelengthRangeTable_10, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                         MS97xxAttrCenterWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                    "MS97XX_ATTR_WAVELENGTH_SPAN", 0, 0,
	                                    MS97xxAttrWavelengthSpan_ReadCallback,
	                                    MS97xxAttrWavelengthSpan_WriteCallback,
	                                    &attrWavelengthSpanRangeTable_10, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                         MS97xxAttrWavelengthSpan_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                    "MS97XX_ATTR_START_WAVELENGTH", 0, 0,
	                                    MS97xxAttrStartWavelength_ReadCallback,
	                                    MS97xxAttrStartWavelength_WriteCallback,
	                                    &attrStartWavelengthRangeTable_10, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                         MS97xxAttrStartWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                    "MS97XX_ATTR_STOP_WAVELENGTH", 0, 0,
	                                    MS97xxAttrStopWavelength_ReadCallback,
	                                    MS97xxAttrStopWavelength_WriteCallback,
	                                    &attrStartWavelengthRangeTable_10, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                         MS97xxAttrStopWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_OSA_TYPE,
	                                   "MS97XX_ATTR_OSA_TYPE", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrOsaType_ReadCallback, VI_NULL,
	                                   VI_NULL));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WAVELENGTH_DISPLAY,
	                                   "MS97XX_ATTR_WAVELENGTH_DISPLAY", 0, 0,
	                                   MS97xxAttrWavelengthDisplay_ReadCallback,
	                                   MS97xxAttrWavelengthDisplay_WriteCallback,
	                                   &attrWavelengthDisplayRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WAVELENGTH_DISPLAY,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WAVELENGTH_OFFSET,
	                                    "MS97XX_ATTR_WAVELENGTH_OFFSET", 0, 0,
	                                    MS97xxAttrWavelengthOffset_ReadCallback,
	                                    MS97xxAttrWavelengthOffset_WriteCallback,
	                                    &attrWavelengthOffsetRangeTable, 0));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AUTO_ALIGN_STATUS,
	                                   "MS97XX_ATTR_AUTO_ALIGN_STATUS", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrAutoAlignStatus_ReadCallback,
	                                   VI_NULL, &attrCalStatusRangeTable));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LOG_SCALE,
	                                    "MS97XX_ATTR_LOG_SCALE", 0, 0,
	                                    MS97xxAttrLogScale_ReadCallback,
	                                    MS97xxAttrLogScale_WriteCallback,
	                                    &attrLogScaleRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_LOG_SCALE,
	                                            MS97xxAttrLogScale_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_REF_LEVEL,
	                                    "MS97XX_ATTR_REF_LEVEL", 0, 0,
	                                    MS97xxAttrRefLevel_ReadCallback,
	                                    MS97xxAttrRefLevel_WriteCallback,
	                                    &attrRefLevelRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_REF_LEVEL,
	                                            MS97xxAttrLogScale_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_REF_LEVEL,
	                                         MS97xxAttrRefLevel_RangeTableCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_ATTENUATOR_ON,
	                                     "MS97XX_ATTR_ATTENUATOR_ON", 0, 0,
	                                     MS97xxAttrAttenuatorOn_ReadCallback,
	                                     MS97xxAttrAttenuatorOn_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_ATTENUATOR_ON,
	                                             MS97xxNoLongTermBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_AUTO_REF_ON,
	                                     "MS97XX_ATTR_AUTO_REF_ON", 0, 0,
	                                     MS97xxAttrAutoRefOn_ReadCallback,
	                                     MS97xxAttrAutoRefOn_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_AUTO_REF_ON,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LEVEL_OFFSET,
	                                    "MS97XX_ATTR_LEVEL_OFFSET", 0, 0,
	                                    MS97xxAttrLevelOffset_ReadCallback,
	                                    MS97xxAttrLevelOffset_WriteCallback,
	                                    &attrLevelOffsetRangeTable_10B, 0));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_LEVEL_OFFSET,
	                                         MS97xxAttrLevelOffset_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LINEAR_SCALE,
	                                    "MS97XX_ATTR_LINEAR_SCALE", 0, 0,
	                                    MS97xxAttrLinearScale_ReadCallback,
	                                    MS97xxAttrLinearScale_WriteCallback,
	                                    &attrLinearScaleRangeTable, 0));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_LINEAR_SCALE,
	                                         MS97xxAttrLinearScale_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LEVEL_SCALE,
	                                   "MS97XX_ATTR_LEVEL_SCALE", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE,
	                                   MS97xxAttrLevelScale_ReadCallback, VI_NULL,
	                                   &attrLevelScaleRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_CURRENT_TRACE,
	                                   "MS97XX_ATTR_CURRENT_TRACE", 0, 0,
	                                   MS97xxAttrCurrentTrace_ReadCallback,
	                                   MS97xxAttrCurrentTrace_WriteCallback,
	                                   &attrCurrentTraceRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_CURRENT_TRACE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MEMORY_A_STORAGE,
	                                   "MS97XX_ATTR_MEMORY_A_STORAGE", 0, 0,
	                                   MS97xxAttrMemoryAStorage_ReadCallback,
	                                   MS97xxAttrMemoryAStorage_WriteCallback,
	                                   &attrMemoryStorageRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_MEMORY_A_STORAGE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MEMORY_B_STORAGE,
	                                   "MS97XX_ATTR_MEMORY_B_STORAGE", 0, 0,
	                                   MS97xxAttrMemoryBStorage_ReadCallback,
	                                   MS97xxAttrMemoryBStorage_WriteCallback,
	                                   &attrMemoryStorageRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_MEMORY_B_STORAGE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MEMORY_C_STORAGE,
	                                   "MS97XX_ATTR_MEMORY_C_STORAGE", 0, 0,
	                                   MS97xxAttrMemoryCStorage_ReadCallback,
	                                   MS97xxAttrMemoryCStorage_WriteCallback,
	                                   &attrMemoryStorageRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_MEMORY_C_STORAGE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_1_VISIBLE,
	                                     "MS97XX_ATTR_TRACE_1_VISIBLE", VI_TRUE,
	                                     IVI_VAL_ALWAYS_CACHE,
	                                     MS97xxAttrTraceVisible_ReadCallback,
	                                     MS97xxAttrTraceVisible_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_TRACE_1_VISIBLE,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_1_SCREEN,
	                                   "MS97XX_ATTR_TRACE_1_SCREEN",
	                                   MS97XX_VAL_TRACE_SCREEN_UPPER,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceScreenRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_1_SCREEN,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_1_NUMERATOR,
	                                   "MS97XX_ATTR_TRACE_1_NUMERATOR",
	                                   MS97XX_VAL_TRACE_A, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceNumeratorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_1_NUMERATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_1_DENOMINATOR,
	                                   "MS97XX_ATTR_TRACE_1_DENOMINATOR",
	                                   MS97XX_VAL_TRACE_NONE, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceDenominatorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_1_DENOMINATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_2_VISIBLE,
	                                     "MS97XX_ATTR_TRACE_2_VISIBLE", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE,
	                                     MS97xxAttrTraceVisible_ReadCallback,
	                                     MS97xxAttrTraceVisible_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_TRACE_2_VISIBLE,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_2_SCREEN,
	                                   "MS97XX_ATTR_TRACE_2_SCREEN",
	                                   MS97XX_VAL_TRACE_SCREEN_LOWER,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceScreenRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_2_SCREEN,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_2_NUMERATOR,
	                                   "MS97XX_ATTR_TRACE_2_NUMERATOR",
	                                   MS97XX_VAL_TRACE_B, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceNumeratorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_2_NUMERATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_2_DENOMINATOR,
	                                   "MS97XX_ATTR_TRACE_2_DENOMINATOR",
	                                   MS97XX_VAL_TRACE_NONE, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceDenominatorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_2_DENOMINATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_3_VISIBLE,
	                                     "MS97XX_ATTR_TRACE_3_VISIBLE", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE,
	                                     MS97xxAttrTraceVisible_ReadCallback,
	                                     MS97xxAttrTraceVisible_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_TRACE_3_VISIBLE,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_3_SCREEN,
	                                   "MS97XX_ATTR_TRACE_3_SCREEN",
	                                   MS97XX_VAL_TRACE_SCREEN_LOWER,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceScreenRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_3_SCREEN,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_3_NUMERATOR,
	                                   "MS97XX_ATTR_TRACE_3_NUMERATOR",
	                                   MS97XX_VAL_TRACE_C, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceNumeratorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_3_NUMERATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TRACE_3_DENOMINATOR,
	                                   "MS97XX_ATTR_TRACE_3_DENOMINATOR",
	                                   MS97XX_VAL_TRACE_NONE, IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrTrace_ReadCallback,
	                                   MS97xxAttrTrace_WriteCallback,
	                                   &attrTraceDenominatorRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_TRACE_3_DENOMINATOR,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_1_ATTR_LOCKED,
	                                     "MS97XX_ATTR_TRACE_1_ATTR_LOCKED", VI_FALSE,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_1_ATTR_DIRTY,
	                                     "MS97XX_ATTR_TRACE_1_ATTR_DIRTY", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_2_ATTR_LOCKED,
	                                     "MS97XX_ATTR_TRACE_2_ATTR_LOCKED", VI_FALSE,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_2_ATTR_DIRTY,
	                                     "MS97XX_ATTR_TRACE_2_ATTR_DIRTY", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_3_ATTR_LOCKED,
	                                     "MS97XX_ATTR_TRACE_3_ATTR_LOCKED", VI_FALSE,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_3_ATTR_DIRTY,
	                                     "MS97XX_ATTR_TRACE_3_ATTR_DIRTY", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTraceAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_ACTIVE_TRACE,
	                                   "MS97XX_ATTR_ACTIVE_TRACE", 1, 0,
	                                   MS97xxAttrActiveTrace_ReadCallback,
	                                   MS97xxAttrActiveTrace_WriteCallback,
	                                   &attrActiveTraceRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_ACTIVE_TRACE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_ACTIVE_SCREEN,
	                                   "MS97XX_ATTR_ACTIVE_SCREEN",
	                                   MS97XX_VAL_ACTIVE_SCREEN_LOWER, 0,
	                                   MS97xxAttrActiveScreen_ReadCallback,
	                                   MS97xxAttrActiveScreen_WriteCallback,
	                                   &attrActiveScreenRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_ACTIVE_SCREEN,
	                                           MS97xxRequireSplitInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_UPPER_REF_LEVEL,
	                                    "MS97XX_ATTR_UPPER_REF_LEVEL", 0, 0,
	                                    MS97xxAttrUpperRefLevel_ReadCallback,
	                                    MS97xxAttrUpperRefLevel_WriteCallback,
	                                    &attrRefLevelRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_UPPER_REF_LEVEL,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LOWER_REF_LEVEL,
	                                    "MS97XX_ATTR_LOWER_REF_LEVEL", 0, 0,
	                                    MS97xxAttrLowerRefLevel_ReadCallback,
	                                    MS97xxAttrLowerRefLevel_WriteCallback,
	                                    &attrRefdBLevelRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_LOWER_REF_LEVEL,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_UPPER_LOG_SCALE,
	                                    "MS97XX_ATTR_UPPER_LOG_SCALE", 0, 0,
	                                    MS97xxAttrUpperLogScale_ReadCallback,
	                                    MS97xxAttrUpperLogScale_WriteCallback,
	                                    &attrLogScaleRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_UPPER_LOG_SCALE,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LOWER_LOG_SCALE,
	                                    "MS97XX_ATTR_LOWER_LOG_SCALE", 0, 0,
	                                    MS97xxAttrLowerLogScale_ReadCallback,
	                                    MS97xxAttrLowerLogScale_WriteCallback,
	                                    &attrLogScaleRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_LOWER_LOG_SCALE,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_FIRST_WDM_SIGNAL,
	                                    "MS97XX_ATTR_FIRST_WDM_SIGNAL", 0, 0,
	                                    MS97xxAttrFirstWdmSignal_ReadCallback,
	                                    MS97xxAttrFirstWdmSignal_WriteCallback,
	                                    &attrStartWavelengthRangeTable_20, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_FIRST_WDM_SIGNAL,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WDM_SIGNAL_SPACING,
	                                    "MS97XX_ATTR_WDM_SIGNAL_SPACING", 0, 0,
	                                    MS97xxAttrWdmSignalSpacing_ReadCallback,
	                                    MS97xxAttrWdmSignalSpacing_WriteCallback,
	                                    &attrWdmSignalSpacingRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_WDM_SIGNAL_SPACING,
	                                            MS97xxRequireLossTestReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_SIGNAL_COUNT,
	                                   "MS97XX_ATTR_WDM_SIGNAL_COUNT", 0, 0,
	                                   MS97xxAttrWdmSignalCount_ReadCallback,
	                                   MS97xxAttrWdmSignalCount_WriteCallback,
	                                   &attrWdmSignalCountRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_SIGNAL_COUNT,
	                                           MS97xxRequireLossTestInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_AUTO_MEASURING,
	                                     "MS97XX_ATTR_AUTO_MEASURING", 0,
	                                     IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                     MS97xxAttrAutoMeasuring_ReadCallback,
	                                     VI_NULL));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_EXTERNAL_TRIGGER_DELAY,
	                                    "MS97XX_ATTR_EXTERNAL_TRIGGER_DELAY", 0, 0,
	                                    MS97xxAttrExternalTriggerDelay_ReadCallback,
	                                    MS97xxAttrExternalTriggerDelay_WriteCallback,
	                                    &attrExternalTriggerDelayRangeTable, 0));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SWEEP_INTERVAL_TIME,
	                                   "MS97XX_ATTR_SWEEP_INTERVAL_TIME", 0, 0,
	                                   MS97xxAttrSweepIntervalTime_ReadCallback,
	                                   MS97xxAttrSweepIntervalTime_WriteCallback,
	                                   &attrSweepIntervalTimeRangeTable));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_ACTUAL_RESOLUTION,
	                                    "MS97XX_ATTR_ACTUAL_RESOLUTION", 0,
	                                    IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                    MS97xxAttrActualResolution_ReadCallback,
	                                    VI_NULL, VI_NULL, 0));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_SHOW_ACTUAL_RESOLUTION,
	                                     "MS97XX_ATTR_SHOW_ACTUAL_RESOLUTION", 0, 0,
	                                     MS97xxAttrShowActualResolution_ReadCallback,
	                                     MS97xxAttrShowActualResolution_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MODULATION_MODE,
	                                   "MS97XX_ATTR_MODULATION_MODE", 0, 0,
	                                   MS97xxAttrModulationMode_ReadCallback,
	                                   MS97xxAttrModulationMode_WriteCallback,
	                                   &attrModulationModeRangeTable_10));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_MODULATION_MODE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MODULATION_MODE,
	                                         MS97xxAttrModulationMode_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_PEAK_HOLD,
	                                   "MS97XX_ATTR_PEAK_HOLD", 0, 0,
	                                   MS97xxAttrPeakHold_ReadCallback,
	                                   MS97xxAttrPeakHold_WriteCallback,
	                                   &attrPeakHoldRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_PEAK_HOLD,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_WIDE_DYNAMIC_RANGE,
	                                     "MS97XX_ATTR_WIDE_DYNAMIC_RANGE", 0, 0,
	                                     MS97xxAttrWideDynamicRange_ReadCallback,
	                                     MS97xxAttrWideDynamicRange_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_WIDE_DYNAMIC_RANGE,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_AUTO_RESOLUTION,
	                                     "MS97XX_ATTR_AUTO_RESOLUTION", 0, 0,
	                                     MS97xxAttrAutoResolution_ReadCallback,
	                                     MS97xxAttrAutoResolution_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_AUTO_RESOLUTION,
	                                             MS97xxRequireSpectrumBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SAMPLING_POINTS,
	                                   "MS97XX_ATTR_SAMPLING_POINTS", 0, 0,
	                                   MS97xxAttrSamplingPoints_ReadCallback,
	                                   MS97xxAttrSamplingPoints_WriteCallback,
	                                   &attrSamplingPointsRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SAMPLING_POINTS,
	                                           MS97xxExcludeLongTermInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_SAMPLING_POINTS,
	                                         MS97xxAttrSamplingPoints_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SMOOTHING,
	                                   "MS97XX_ATTR_SMOOTHING", 0, 0,
	                                   MS97xxAttrSmoothing_ReadCallback,
	                                   MS97xxAttrSmoothing_WriteCallback,
	                                   &attrSmoothingRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SMOOTHING,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_POINT_AVERAGE,
	                                   "MS97XX_ATTR_POINT_AVERAGE", 0, 0,
	                                   MS97xxAttrPointAverage_ReadCallback,
	                                   MS97xxAttrPointAverage_WriteCallback,
	                                   &attrAverageRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_POINT_AVERAGE,
	                                           MS97xxExcludeLongTermInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SWEEP_AVERAGE,
	                                   "MS97XX_ATTR_SWEEP_AVERAGE", 0, 0,
	                                   MS97xxAttrSweepAverage_ReadCallback,
	                                   MS97xxAttrSweepAverage_WriteCallback,
	                                   &attrAverageRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SWEEP_AVERAGE,
	                                           MS97xxExcludeOverlapInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_VIDEO_BAND_WIDTH,
	                                    "MS97XX_ATTR_VIDEO_BAND_WIDTH", 0, 0,
	                                    MS97xxAttrVideoBandWidth_ReadCallback,
	                                    MS97xxAttrVideoBandWidth_WriteCallback,
	                                    &attrVideoBandWidthRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_VIDEO_BAND_WIDTH,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_VIDEO_BAND_WIDTH,
	                                         MS97xxAttrVideoBandWidth_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_RESOLUTION,
	                                    "MS97XX_ATTR_RESOLUTION", 0, 0,
	                                    MS97xxAttrResolution_ReadCallback,
	                                    MS97xxAttrResolution_WriteCallback,
	                                    &attrResolutionRangeTable_10B, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_RESOLUTION,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                   "MS97XX_ATTR_ANALYSIS_MODE", 0, 0,
	                                   MS97xxAttrAnalysisMode_ReadCallback,
	                                   MS97xxAttrAnalysisMode_WriteCallback,
	                                   &attrAnalysisModeRangeTable_10));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                         MS97xxAttrAnalysisMode_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_THRESHOLD_CUT,
	                                   "MS97XX_ATTR_THRESHOLD_CUT", 0, 0,
	                                   MS97xxAttrThresholdCut_ReadCallback,
	                                   MS97xxAttrThresholdCut_WriteCallback,
	                                   &attrThresholdCutRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_THRESHOLD_CUT,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SMSR_SIDE_MODE,
	                                   "MS97XX_ATTR_SMSR_SIDE_MODE", 0, 0,
	                                   MS97xxAttrSmsrSideMode_ReadCallback,
	                                   MS97xxAttrSmsrSideMode_WriteCallback,
	                                   &attrSmsrSideModeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SMSR_SIDE_MODE,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_ENVELOPE_CUT,
	                                   "MS97XX_ATTR_ENVELOPE_CUT", 0, 0,
	                                   MS97xxAttrEnvelopeCut_ReadCallback,
	                                   MS97xxAttrEnvelopeCut_WriteCallback,
	                                   &attrThresholdCutRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_ENVELOPE_CUT,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_RMS_CUT,
	                                   "MS97XX_ATTR_RMS_CUT", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrRmsCut_ReadCallback,
	                                   MS97xxAttrRmsCut_WriteCallback,
	                                   &attrThresholdCutRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_RMS_CUT,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_RMS_COEFFICIENT,
	                                    "MS97XX_ATTR_RMS_COEFFICIENT", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrRms_ReadCallback,
	                                    MS97xxAttrRms_WriteCallback,
	                                    &attrRmsCoefficientRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_RMS_COEFFICIENT,
	                                            MS97xxRequireNormalReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_NDB_ATTENUATION,
	                                   "MS97XX_ATTR_NDB_ATTENUATION", 0, 0,
	                                   MS97xxAttrNdbAttenuation_ReadCallback,
	                                   MS97xxAttrNdbAttenuation_WriteCallback,
	                                   &attrThresholdCutRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_NDB_ATTENUATION,
	                                           MS97xxRequireNormalInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_RMS_DIRTY,
	                                     "MS97XX_ATTR_RMS_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrRmsAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_RMS_LOCKED,
	                                     "MS97XX_ATTR_RMS_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrRmsAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_FILE_OPTION,
	                                   "MS97XX_ATTR_FILE_OPTION", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDiskette_ReadCallback,
	                                   MS97xxAttrDiskette_WriteCallback,
	                                   &attrFileOptionRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_FILE_ID_FORMAT,
	                                   "MS97XX_ATTR_FILE_ID_FORMAT", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDiskette_ReadCallback,
	                                   MS97xxAttrDiskette_WriteCallback,
	                                   &attrFileIdFormatRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_DISKETTE_SIZE,
	                                   "MS97XX_ATTR_DISKETTE_SIZE", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDiskette_ReadCallback,
	                                   MS97xxAttrDiskette_WriteCallback,
	                                   &attrDisketteSizeRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_FILE_NUMERIC_FORMAT,
	                                   "MS97XX_ATTR_FILE_NUMERIC_FORMAT", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDiskette_ReadCallback,
	                                   MS97xxAttrDiskette_WriteCallback,
	                                   &attrFileNumericFormatRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_FILE_INSTRUMENT_FORMAT,
	                                   "MS97XX_ATTR_FILE_INSTRUMENT_FORMAT", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDiskette_ReadCallback,
	                                   MS97xxAttrDiskette_WriteCallback,
	                                   &attrInstrumentFormatRangeTable));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_DISKETTE_ATTR_DIRTY,
	                                     "MS97XX_ATTR_DISKETTE_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL,
	                                     MS97xxAttrDisketteAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_DISKETTE_ATTR_LOCKED,
	                                     "MS97XX_ATTR_DISKETTE_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL,
	                                     MS97xxAttrDisketteAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_DFB_SIDE_MODE,
	                                   "MS97XX_ATTR_DFB_SIDE_MODE", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDfb_ReadCallback,
	                                   MS97xxAttrDfb_WriteCallback,
	                                   &attrDfbSideModeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_DFB_SIDE_MODE,
	                                           MS97xxRequireDfbInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_DFB_NDB_WIDTH,
	                                   "MS97XX_ATTR_DFB_NDB_WIDTH", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrDfb_ReadCallback,
	                                   MS97xxAttrDfb_WriteCallback,
	                                   &attrDfbNdbWidthRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_DFB_NDB_WIDTH,
	                                           MS97xxRequireDfbInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_FP_AXIS_MODE_CUT_LEVEL,
	                                   "MS97XX_ATTR_FP_AXIS_MODE_CUT_LEVEL", 0, 0,
	                                   MS97xxAttrFpAxisModeCutLevel_ReadCallback,
	                                   MS97xxAttrFpAxisModeCutLevel_WriteCallback,
	                                   &attrFpAxisModeCutLevelRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi,
	                                           MS97XX_ATTR_FP_AXIS_MODE_CUT_LEVEL,
	                                           MS97xxRequireFpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LED_NDB_DOWN_WAVE,
	                                   "MS97XX_ATTR_LED_NDB_DOWN_WAVE", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrLedNdbDownWave_ReadCallback,
	                                   MS97xxAttrLedNdbDownWave_WriteCallback,
	                                   &attrLedNdbDownWaveRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_LED_NDB_DOWN_WAVE,
	                                           MS97xxRequireLedInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AMP_MEMORY,
	                                   "MS97XX_ATTR_AMP_MEMORY", 0, 0,
	                                   MS97xxAttrAmpMemory_ReadCallback,
	                                   MS97xxAttrAmpMemory_WriteCallback,
	                                   &attrAmpMemoryRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_AMP_MEMORY,
	                                           MS97xxRequireAmpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AMP_CAL_STATUS,
	                                   "MS97XX_ATTR_AMP_CAL_STATUS", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrAmpCalStatus_ReadCallback, VI_NULL,
	                                   &attrAmpCalStatusRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_AMP_CAL_STATUS,
	                                           MS97xxRequireAmpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AMP_CALC_MODE,
	                                   "MS97XX_ATTR_AMP_CALC_MODE", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrAmpInt_ReadCallback,
	                                   MS97xxAttrAmpInt_WriteCallback,
	                                   &attrAmpCalcModeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_AMP_CALC_MODE,
	                                           MS97xxRequireAmpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AMP_MEASURE_METHOD,
	                                   "MS97XX_ATTR_AMP_MEASURE_METHOD", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrAmpInt_ReadCallback,
	                                   MS97xxAttrAmpInt_WriteCallback,
	                                   &attrAmpMeasureMethodRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_AMP_MEASURE_METHOD,
	                                           MS97xxRequireAmpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_AMP_FITTING_METHOD,
	                                   "MS97XX_ATTR_AMP_FITTING_METHOD", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrAmpInt_ReadCallback,
	                                   MS97xxAttrAmpInt_WriteCallback,
	                                   &attrAmpFittingMethodRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_AMP_FITTING_METHOD,
	                                           MS97xxRequireAmpInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_FITTING_SPAN,
	                                    "MS97XX_ATTR_AMP_FITTING_SPAN", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpFittingSpanRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_FITTING_SPAN,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_MASKED_SPAN,
	                                    "MS97XX_ATTR_AMP_MASKED_SPAN", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpMaskedSpanRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_MASKED_SPAN,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_PIN_LOSS,
	                                    "MS97XX_ATTR_AMP_PIN_LOSS", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpPinLossRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_PIN_LOSS,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_POUT_LOSS,
	                                    "MS97XX_ATTR_AMP_POUT_LOSS", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpPoutLossRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_POUT_LOSS,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_NF_CAL,
	                                    "MS97XX_ATTR_AMP_NF_CAL", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpNfCalRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_NF_CAL,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_BAND_PASS_CUT,
	                                    "MS97XX_ATTR_AMP_BAND_PASS_CUT", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpBandPassCutRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_BAND_PASS_CUT,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_FILTER_BANDWIDTH,
	                                    "MS97XX_ATTR_AMP_FILTER_BANDWIDTH", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpFilterBandwidthRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_FILTER_BANDWIDTH,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_AMP_POLAR_LOSS,
	                                    "MS97XX_ATTR_AMP_POLAR_LOSS", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrAmpReal_ReadCallback,
	                                    MS97xxAttrAmpReal_WriteCallback,
	                                    &attrAmpPolarLossRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_AMP_POLAR_LOSS,
	                                            MS97xxRequireAmpReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_CUT_LEVEL,
	                                   "MS97XX_ATTR_WDM_CUT_LEVEL", 0, 0,
	                                   MS97xxAttrWdmCutLevel_ReadCallback,
	                                   MS97xxAttrWdmCutLevel_WriteCallback,
	                                   &attrWdmCutLevelRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_CUT_LEVEL,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_MODE,
	                                   "MS97XX_ATTR_WDM_MODE", 0, 0,
	                                   MS97xxAttrWdmMode_ReadCallback,
	                                   MS97xxAttrWdmMode_WriteCallback,
	                                   &attrWdmModeRangeTable_10));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_MODE,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_WDM_MODE,
	                                         MS97xxAttrWdmMode_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_SNR_DIR,
	                                   "MS97XX_ATTR_WDM_SNR_DIR", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrWdmSnrDir_ReadCallback,
	                                   MS97xxAttrWdmSnrDir_WriteCallback,
	                                   &attrWdmSnrDirRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_SNR_DIR,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WDM_SNR_DW,
	                                    "MS97XX_ATTR_WDM_SNR_DW", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrWdmSnrDw_ReadCallback,
	                                    MS97xxAttrWdmSnrDw_WriteCallback,
	                                    &attrWdmSnrDwRangeTable_10, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_WDM_SNR_DW,
	                                            MS97xxRequireWdmReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_WDM_SNR_DW,
	                                         MS97xxAttrWdmSnrDw_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_REF_PEAK,
	                                   "MS97XX_ATTR_WDM_REF_PEAK", 0, 0,
	                                   MS97xxAttrWdmRefPeak_ReadCallback,
	                                   MS97xxAttrWdmRefPeak_WriteCallback,
	                                   &attrWdmRefPeakRangeTable_10));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_REF_PEAK,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_WDM_REF_PEAK,
	                                         MS97xxAttrWdmRefPeak_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_TABLE_DIR,
	                                   "MS97XX_ATTR_WDM_TABLE_DIR", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrWdmTableDir_ReadCallback,
	                                   MS97xxAttrWdmTableDir_WriteCallback,
	                                   &attrWdmTableDirRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_TABLE_DIR,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WDM_TABLE_DW,
	                                    "MS97XX_ATTR_WDM_TABLE_DW", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrWdmTableDw_ReadCallback,
	                                    MS97xxAttrWdmTableDw_WriteCallback,
	                                    &attrWdmTableDwRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_WDM_TABLE_DW,
	                                            MS97xxRequireWdmReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_WDM_TABLE_NORM,
	                                     "MS97XX_ATTR_WDM_TABLE_NORM", 0,
	                                     IVI_VAL_ALWAYS_CACHE,
	                                     MS97xxAttrWdmTableNorm_ReadCallback,
	                                     MS97xxAttrWdmTableNorm_WriteCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_WDM_TABLE_NORM,
	                                             MS97xxRequireWdmBoolean_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WDM_PEAK_TYPE,
	                                   "MS97XX_ATTR_WDM_PEAK_TYPE", 0, 0,
	                                   MS97xxAttrWdmPeakType_ReadCallback,
	                                   MS97xxAttrWdmPeakType_WriteCallback,
	                                   &attrWdmPeakTypeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_WDM_PEAK_TYPE,
	                                           MS97xxRequireWdmInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_WDM_THRESHOLD,
	                                    "MS97XX_ATTR_WDM_THRESHOLD", 0, 0,
	                                    MS97xxAttrWdmThreshold_ReadCallback,
	                                    MS97xxAttrWdmThreshold_WriteCallback,
	                                    &attrWdmThresholdRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_WDM_THRESHOLD,
	                                            MS97xxRequireWdmReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_NF_FITTING_SPAN,
	                                    "MS97XX_ATTR_NF_FITTING_SPAN", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrNf_ReadCallback,
	                                    MS97xxAttrNf_WriteCallback,
	                                    &attrNfFittingSpanRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_NF_FITTING_SPAN,
	                                            MS97xxRequireNfReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_NF_PIN_LOSS,
	                                    "MS97XX_ATTR_NF_PIN_LOSS", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrNf_ReadCallback,
	                                    MS97xxAttrNf_WriteCallback,
	                                    &attrNfPinLossRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_NF_PIN_LOSS,
	                                            MS97xxRequireNfReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_NF_CAL, "MS97XX_ATTR_NF_CAL",
	                                    0, IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrNf_ReadCallback,
	                                    MS97xxAttrNf_WriteCallback,
	                                    &attrNfCalRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_NF_CAL,
	                                            MS97xxRequireNfReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_NF_ASE_CAL,
	                                    "MS97XX_ATTR_NF_ASE_CAL", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrNf_ReadCallback,
	                                    MS97xxAttrNf_WriteCallback,
	                                    &attrNfAseCalRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_NF_ASE_CAL,
	                                            MS97xxRequireNfReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_NF_POUT_LOSS,
	                                    "MS97XX_ATTR_NF_POUT_LOSS", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrNf_ReadCallback,
	                                    MS97xxAttrNf_WriteCallback,
	                                    &attrNfPoutLossRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_NF_POUT_LOSS,
	                                            MS97xxRequireNfReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_NF_CUT_LEVEL,
	                                   "MS97XX_ATTR_NF_CUT_LEVEL", 0, 0,
	                                   MS97xxAttrNfCutLevel_ReadCallback,
	                                   MS97xxAttrNfCutLevel_WriteCallback,
	                                   &attrNfCutLevelRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_NF_CUT_LEVEL,
	                                           MS97xxRequireNfInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SCREEN_MODE,
	                                   "MS97XX_ATTR_SCREEN_MODE", 0, 0,
	                                   MS97xxAttrScreenMode_ReadCallback,
	                                   MS97xxAttrScreenMode_WriteCallback,
	                                   &attrScreenModeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SCREEN_MODE,
	                                           MS97xxAttrScreenMode_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SELECTED_MEM,
	                                   "MS97XX_ATTR_SELECTED_MEM", 0, 0,
	                                   MS97xxAttrSelectedMem_ReadCallback,
	                                   MS97xxAttrSelectedMem_WriteCallback,
	                                   &attrSelectedMemRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_SELECTED_MEM,
	                                           MS97xxAttrSelectedMem_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_DFB_ATTR_LOCKED,
	                                     "MS97XX_ATTR_DFB_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrDfbAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_DFB_ATTR_DIRTY,
	                                     "MS97XX_ATTR_DFB_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrDfbAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LED_POWER_COMP,
	                                    "MS97XX_ATTR_LED_POWER_COMP", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrLedPowerComp_ReadCallback,
	                                    MS97xxAttrLedPowerComp_WriteCallback,
	                                    &attrLedPowerCompRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_LED_POWER_COMP,
	                                            MS97xxRequireLedReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_LED_ATTR_LOCKED,
	                                     "MS97XX_ATTR_LED_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrLedAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_LED_ATTR_DIRTY,
	                                     "MS97XX_ATTR_LED_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrLedAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_AMP_ATTR_LOCKED,
	                                     "MS97XX_ATTR_AMP_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrAmpAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_AMP_ATTR_DIRTY,
	                                     "MS97XX_ATTR_AMP_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrAmpAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_SNR_ATTR_LOCKED,
	                                     "MS97XX_ATTR_SNR_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrSnrAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_SNR_ATTR_DIRTY,
	                                     "MS97XX_ATTR_SNR_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrSnrAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TABLE_ATTR_LOCKED,
	                                     "MS97XX_ATTR_TABLE_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTableAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TABLE_ATTR_DIRTY,
	                                     "MS97XX_ATTR_TABLE_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrTableAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_NF_ATTR_LOCKED,
	                                     "MS97XX_ATTR_NF_ATTR_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrNfAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_NF_ATTR_DIRTY,
	                                     "MS97XX_ATTR_NF_ATTR_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, MS97xxAttrNfAttr_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MEASURE_MODE,
	                                   "MS97XX_ATTR_MEASURE_MODE", 0,
	                                   IVI_VAL_ALWAYS_CACHE,
	                                   MS97xxAttrMeasureMode_ReadCallback,
	                                   MS97xxAttrMeasureMode_WriteCallback,
	                                   &attrMeasureModeRangeTable_10));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_SEND_HEADER,
	                                     "MS97XX_ATTR_SEND_HEADER", VI_FALSE,
	                                     IVI_VAL_NOT_USER_WRITABLE,
	                                     MS97xxAttrSendHeader_ReadCallback,
	                                     MS97xxAttrSendHeader_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TERMINATOR,
	                                   "MS97XX_ATTR_TERMINATOR", 0, 0,
	                                   MS97xxAttrTerminator_ReadCallback,
	                                   MS97xxAttrTerminator_WriteCallback,
	                                   &attrTerminatorRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EVENT_STATUS_ENABLE,
	                                   "MS97XX_ATTR_EVENT_STATUS_ENABLE", 0, 0,
	                                   MS97xxAttrEventStatusEnable_ReadCallback,
	                                   MS97xxAttrEventStatusEnable_WriteCallback,
	                                   &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_SERVICE_REQUEST_ENABLE,
	                                   "MS97XX_ATTR_SERVICE_REQUEST_ENABLE", 0, 0,
	                                   MS97xxAttrServiceRequestEnable_ReadCallback,
	                                   MS97xxAttrServiceRequestEnable_WriteCallback,
	                                   &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_STATUS_BYTE,
	                                   "MS97XX_ATTR_STATUS_BYTE", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrStatusByte_ReadCallback, VI_NULL,
	                                   VI_NULL));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EVENT_STATUS,
	                                   "MS97XX_ATTR_EVENT_STATUS", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrEventStatus_ReadCallback, VI_NULL,
	                                   VI_NULL));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_1_ENABLE,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_1_ENABLE", 0, 0,
	                                   MS97xxAttrExtendedStatus1Enable_ReadCallback,
	                                   MS97xxAttrExtendedStatus1Enable_WriteCallback,
	                                   &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE", 0, 0,
	                                   MS97xxAttrExtendedStatus2Enable_ReadCallback,
	                                   MS97xxAttrExtendedStatus2Enable_WriteCallback,
	                                   &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE", 0, 0,
	                                   MS97xxAttrExtendedStatus3Enable_ReadCallback,
	                                   MS97xxAttrExtendedStatus3Enable_WriteCallback,
	                                   &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_1,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_1", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrExtendedStatus1_ReadCallback,
	                                   VI_NULL, &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_2,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_2", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrExtendedStatus2_ReadCallback,
	                                   VI_NULL, &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_EXTENDED_STATUS_3,
	                                   "MS97XX_ATTR_EXTENDED_STATUS_3", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrExtendedStatus3_ReadCallback,
	                                   VI_NULL, &attrEventStatusEnableRangeTable));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_ERROR, "MS97XX_ATTR_ERROR", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrError_ReadCallback, VI_NULL,
	                                   VI_NULL));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_MARKER_A_WAVELENGTH,
	                                    "MS97XX_ATTR_MARKER_A_WAVELENGTH", 0, 0,
	                                    MS97xxAttrMarkerWavelength_ReadCallback,
	                                    MS97xxAttrMarkerWavelength_WriteCallback,
	                                    pWavelengthRange, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_MARKER_A_WAVELENGTH,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MARKER_A_WAVELENGTH,
	                                         MS97xxAttrMarkerWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_MARKER_B_WAVELENGTH,
	                                    "MS97XX_ATTR_MARKER_B_WAVELENGTH", 0, 0,
	                                    MS97xxAttrMarkerWavelength_ReadCallback,
	                                    MS97xxAttrMarkerWavelength_WriteCallback,
	                                    pWavelengthRange, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_MARKER_B_WAVELENGTH,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MARKER_B_WAVELENGTH,
	                                         MS97xxAttrMarkerWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_MARKER_C_LEVEL,
	                                    "MS97XX_ATTR_MARKER_C_LEVEL", 0, 0,
	                                    MS97xxAttrMarkerLevel_ReadCallback,
	                                    MS97xxAttrMarkerLevel_WriteCallback,
	                                    &attrMarkerLevelRangeTable_dBm, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_MARKER_C_LEVEL,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MARKER_C_LEVEL,
	                                         MS97xxAttrMarkerLevel_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_MARKER_D_LEVEL,
	                                    "MS97XX_ATTR_MARKER_D_LEVEL", 0, 0,
	                                    MS97xxAttrMarkerLevel_ReadCallback,
	                                    MS97xxAttrMarkerLevel_WriteCallback,
	                                    &attrMarkerLevelRangeTable_dBm, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_MARKER_D_LEVEL,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                    "MS97XX_ATTR_TRACE_MARKER_WAVELENGTH", 0, 0,
	                                    MS97xxAttrTraceMarker_ReadCallback,
	                                    MS97xxAttrTraceMarkerWavelength_WriteCallback,
	                                    pWavelengthRange, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi,
	                                            MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                         MS97xxAttrMarkerWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_TRACE_MARKER_LEVEL,
	                                    "MS97XX_ATTR_TRACE_MARKER_LEVEL", 0,
	                                    IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                    MS97xxAttrTraceMarker_ReadCallback, VI_NULL,
	                                    VI_NULL, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_TRACE_MARKER_LEVEL,
	                                            MS97xxExcludeLongTermReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_ZONE_MARKER_CENTER,
	                                    "MS97XX_ATTR_ZONE_MARKER_CENTER", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrZoneMarker_ReadCallback,
	                                    MS97xxAttrZoneMarker_WriteCallback,
	                                    pWavelengthRange, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_ZONE_MARKER_CENTER,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_ZONE_MARKER_CENTER,
	                                         MS97xxAttrMarkerWavelength_RangeTableCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_ZONE_MARKER_SPAN,
	                                    "MS97XX_ATTR_ZONE_MARKER_SPAN", 0,
	                                    IVI_VAL_ALWAYS_CACHE,
	                                    MS97xxAttrZoneMarker_ReadCallback,
	                                    MS97xxAttrZoneMarker_WriteCallback,
	                                    pWavelengthSpanRange, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_ZONE_MARKER_SPAN,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_ZONE_MARKER_SPAN,
	                                         MS97xxAttrZoneMarkerSpan_RangeTableCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_MARKER_UNITS,
	                                   "MS97XX_ATTR_MARKER_UNITS", 0, 0,
	                                   MS97xxAttrMarkerUnits_ReadCallback,
	                                   MS97xxAttrMarkerUnits_WriteCallback,
	                                   &attrMarkerUnitsRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_MARKER_UNITS,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_ZONE_MARKER_WL_LOCKED,
	                                     "MS97XX_ATTR_ZONE_MARKER_WL_LOCKED", 0,
	                                     IVI_VAL_NEVER_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL,
	                                     MS97xxAttrZoneMarkerWl_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_ZONE_MARKER_WL_DIRTY,
	                                     "MS97XX_ATTR_ZONE_MARKER_WL_DIRTY", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL,
	                                     MS97xxAttrZoneMarkerWl_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_LIGHT_OUTPUT,
	                                     "MS97XX_ATTR_LIGHT_OUTPUT", 0, 0,
	                                     MS97xxAttrLightOutput_ReadCallback,
	                                     MS97xxAttrLightOutput_WriteCallback));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TLS_TRACK_ENABLED,
	                                     "MS97XX_ATTR_TLS_TRACK_ENABLED", 0, 0,
	                                     MS97xxAttrTlsTrackEnabled_ReadCallback,
	                                     MS97xxAttrTlsTrackEnabled_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_TLS_CAL_STATUS,
	                                   "MS97XX_ATTR_TLS_CAL_STATUS", 0,
	                                   IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrTlsCalStatus_ReadCallback,
	                                   MS97xxAttrTlsCalStatus_WriteCallback,
	                                   &attrTlsCalStatusRangeTable));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_POWER_MONITOR_WAVELENGTH,
	                                    "MS97XX_ATTR_POWER_MONITOR_WAVELENGTH", 0, 0,
	                                    MS97xxAttrPowerMonitorWavelength_ReadCallback,
	                                    MS97xxAttrPowerMonitorWavelength_WriteCallback,
	                                    &attrPowerMonitorWavelengthRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi,
	                                            MS97XX_ATTR_POWER_MONITOR_WAVELENGTH,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_POWER_MONITOR_RESULT,
	                                    "MS97XX_ATTR_POWER_MONITOR_RESULT", 0,
	                                    IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                    MS97xxAttrPowerMonitorResult_ReadCallback,
	                                    VI_NULL, VI_NULL, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_POWER_MONITOR_RESULT,
	                                            MS97xxRequireSpectrumReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_PMD_COUPLING,
	                                    "MS97XX_ATTR_PMD_COUPLING", 0, 0,
	                                    MS97xxAttrPmdCoupling_ReadCallback,
	                                    MS97xxAttrPmdCoupling_WriteCallback,
	                                    &attrModeConnectCoeffRangeTable, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_PMD_COUPLING,
	                                            MS97xxRequirePmdReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_LAST_PMD_PEAK,
	                                    "MS97XX_ATTR_LAST_PMD_PEAK", 0, 0,
	                                    MS97xxAttrLastPmdPeak_ReadCallback,
	                                    MS97xxAttrLastPmdPeak_WriteCallback,
	                                    &attrStartWavelengthRangeTable_20, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_LAST_PMD_PEAK,
	                                            MS97xxRequirePmdReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViReal64 (vi, MS97XX_ATTR_FIRST_PMD_PEAK,
	                                    "MS97XX_ATTR_FIRST_PMD_PEAK", 0, 0,
	                                    MS97xxAttrFirstPmdPeak_ReadCallback,
	                                    MS97xxAttrFirstPmdPeak_WriteCallback,
	                                    &attrStartWavelengthRangeTable_20, 0));
	checkErr (Ivi_SetAttrCheckCallbackViReal64 (vi, MS97XX_ATTR_FIRST_PMD_PEAK,
	                                            MS97xxRequirePmdReal64_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_PMD_PEAK_COUNT,
	                                   "MS97XX_ATTR_PMD_PEAK_COUNT", 0, 0,
	                                   MS97xxAttrPmdPeakCount_ReadCallback,
	                                   MS97xxAttrPmdPeakCount_WriteCallback,
	                                   &attrPmdPeakCountRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_PMD_PEAK_COUNT,
	                                           MS97xxRequirePmdInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LONG_TERM_SAMPLE_TIME,
	                                   "MS97XX_ATTR_LONG_TERM_SAMPLE_TIME", 0, 0,
	                                   MS97xxAttrLongTermSampleTime_ReadCallback,
	                                   MS97xxAttrLongTermSampleTime_WriteCallback,
	                                   &attrLongTermSampleTimeRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_LONG_TERM_SAMPLE_TIME,
	                                           MS97xxRequireLongTermInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LONG_TERM_STATE,
	                                   "MS97XX_ATTR_LONG_TERM_STATE", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrLongTermState_ReadCallback, VI_NULL,
	                                   &attrLongTermTestStateRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_LONG_TERM_STATE,
	                                           MS97xxRequireLongTermInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LONG_TERM_DATA_SIZE,
	                                   "MS97XX_ATTR_LONG_TERM_DATA_SIZE", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrLongTermDataSize_ReadCallback,
	                                   VI_NULL, VI_NULL));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_LONG_TERM_DATA_SIZE,
	                                           MS97xxRequireLongTermInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_PEAK_SEARCH,
	                                   "MS97XX_ATTR_PEAK_SEARCH", 0, 0,
	                                   MS97xxAttrPeakSearch_ReadCallback,
	                                   MS97xxAttrPeakSearch_WriteCallback,
	                                   &attrPeakSearchRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_PEAK_SEARCH,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY,
	                                   "MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY", 0, 0,
	                                   MS97xxAttrLongTermSpacingDisplay_ReadCallback,
	                                   MS97xxAttrLongTermSpacingDisplay_WriteCallback,
	                                   &attrLongTermSpacingDisplayRangeTable));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi,
	                                           MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY,
	                                           MS97xxRequireLongTermInt32_CheckCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_READ_BUF_SIZE,
	                                   "MS97XX_ATTR_READ_BUF_SIZE", 0,
	                                   IVI_VAL_HIDDEN, VI_NULL, VI_NULL, VI_NULL));
	checkErr (Ivi_AddAttributeViAddr (vi, MS97XX_ATTR_READ_BUF,
	                                  "MS97XX_ATTR_READ_BUF", VI_NULL,
	                                  IVI_VAL_HIDDEN, VI_NULL, VI_NULL));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_WDM_SNR_NORM,
	                                     "MS97XX_ATTR_WDM_SNR_NORM", 0,
	                                     IVI_VAL_ALWAYS_CACHE,
	                                     MS97xxAttrWdmSnrNorm_ReadCallback,
	                                     MS97xxAttrWdmSnrNorm_WriteCallback));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_WAVELENGTH_CAL_STATUS,
	                                   "MS97XX_ATTR_WAVELENGTH_CAL_STATUS", 0,
	                                   IVI_VAL_NOT_USER_WRITABLE | IVI_VAL_NEVER_CACHE,
	                                   MS97xxAttrWavelengthCalStatus_ReadCallback,
	                                   VI_NULL, &attrCalStatusRangeTable));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_TRACE_MARKER_VISIBLE,
	                                     "MS97XX_ATTR_TRACE_MARKER_VISIBLE", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, VI_NULL));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_ZONE_MARKER_VISIBLE,
	                                     "MS97XX_ATTR_ZONE_MARKER_VISIBLE", 0,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, VI_NULL));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                   "MS97XX_ATTR_DISPLAY_MODE", 0, 0,
	                                   MS97xxAttrDisplayMode_ReadCallback,
	                                   MS97xxAttrDisplayMode_WriteCallback,
	                                   &attrDisplayModeRangeTable_10));
	checkErr (Ivi_AddAttributeViInt32 (vi, MS97XX_ATTR_DIP_SEARCH,
	                                   "MS97XX_ATTR_DIP_SEARCH", 0, 0,
	                                   MS97xxAttrDipSearch_ReadCallback,
	                                   MS97xxAttrDipSearch_WriteCallback,
	                                   &attrDipSearchRangeTable));
	checkErr (Ivi_AddAttributeViBoolean (vi, MS97XX_ATTR_INST_OUT_OF_RANGE,
	                                     "MS97XX_ATTR_INST_OUT_OF_RANGE", VI_FALSE,
	                                     IVI_VAL_ALWAYS_CACHE | IVI_VAL_HIDDEN,
	                                     VI_NULL, VI_NULL));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_DIP_SEARCH,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                         MS97xxAttrDisplayMode_RangeTableCallback));
	checkErr (Ivi_SetAttrCheckCallbackViInt32 (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                           MS97xxRequireSpectrumInt32_CheckCallback));
	checkErr (Ivi_SetAttrCheckCallbackViBoolean (vi, MS97XX_ATTR_WDM_SNR_NORM,
	                                             MS97xxRequireWdmBoolean_CheckCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MARKER_D_LEVEL,
	                                         MS97xxAttrMarkerLevel_RangeTableCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_MEASURE_MODE,
	                                         MS97xxAttrMeasureMode_RangeTableCallback));
	checkErr (Ivi_SetAttrRangeTableCallback (vi, MS97XX_ATTR_RESOLUTION,
	                                         MS97xxAttrResolution_RangeTableCallback));
                                       
        /*- Setup attribute invalidations -----------------------------------*/            

	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                        MS97XX_ATTR_START_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                        MS97XX_ATTR_STOP_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                        MS97XX_ATTR_START_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                        MS97XX_ATTR_STOP_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                        MS97XX_ATTR_CENTER_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                        MS97XX_ATTR_WAVELENGTH_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                        MS97XX_ATTR_CENTER_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                        MS97XX_ATTR_WAVELENGTH_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ID_QUERY_RESPONSE,
	                                        MS97XX_ATTR_OSA_TYPE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CURRENT_TRACE,
	                                        MS97XX_ATTR_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CURRENT_TRACE,
	                                        MS97XX_ATTR_LINEAR_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_AUTO_RESOLUTION,
	                                        MS97XX_ATTR_RESOLUTION, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_POINT_AVERAGE,
	                                        MS97XX_ATTR_SWEEP_AVERAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_SWEEP_AVERAGE,
	                                        MS97XX_ATTR_POINT_AVERAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_RESOLUTION,
	                                        MS97XX_ATTR_AUTO_RESOLUTION, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_THRESHOLD_CUT,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_SMSR_SIDE_MODE,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ENVELOPE_CUT,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_RMS_CUT,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_RMS_COEFFICIENT,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_NDB_ATTENUATION,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_LOG_SCALE,
	                                        MS97XX_ATTR_LEVEL_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_LINEAR_SCALE,
	                                        MS97XX_ATTR_LEVEL_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_PEAK_SEARCH,
	                                        MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_PEAK_SEARCH,
	                                        MS97XX_ATTR_TRACE_MARKER_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                        MS97XX_ATTR_MARKER_A_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                        MS97XX_ATTR_MARKER_B_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                        MS97XX_ATTR_MARKER_C_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_ANALYSIS_MODE,
	                                        MS97XX_ATTR_MARKER_D_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_CENTER_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_WAVELENGTH_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_START_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_STOP_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ATTENUATOR_ON, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_LEVEL_OFFSET, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_UPPER_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_LOWER_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_UPPER_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_LOWER_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_SAMPLING_POINTS, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_POINT_AVERAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_VIDEO_BAND_WIDTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_RESOLUTION, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MARKER_A_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MARKER_B_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MARKER_C_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MARKER_D_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_REF_LEVEL,
	                                        MS97XX_ATTR_LEVEL_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_AUTO_REF_ON,
	                                        MS97XX_ATTR_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        MS97XX_ATTR_TRACE_MARKER_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MARKER_UNITS,
	                                        MS97XX_ATTR_MARKER_A_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MARKER_UNITS,
	                                        MS97XX_ATTR_MARKER_B_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MARKER_UNITS,
	                                        MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_SNR_DIR,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_SNR_DW,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_REF_PEAK,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_TABLE_DIR,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_TABLE_DW,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_TABLE_NORM,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WDM_SNR_NORM,
	                                        MS97XX_ATTR_WDM_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_CENTER_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_WAVELENGTH_SPAN,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_START_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_STOP_WAVELENGTH,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_CURRENT_TRACE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MEMORY_A_STORAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MEMORY_B_STORAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_MEMORY_C_STORAGE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_1_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_1_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_1_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_1_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_2_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_2_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_2_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_2_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_3_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_3_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_3_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_TRACE_3_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ACTIVE_TRACE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_ACTIVE_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_SCREEN_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_SELECTED_MEM, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_MEASURE_MODE,
	                                        MS97XX_ATTR_DISPLAY_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_AUTO_REF_ON, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_LEVEL_OFFSET, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_LINEAR_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_1_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_1_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_1_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_1_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_2_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_2_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_2_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_2_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_3_VISIBLE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_3_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_3_NUMERATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_3_DENOMINATOR, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_ACTIVE_SCREEN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_UPPER_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_LOWER_REF_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_UPPER_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_LOWER_LOG_SCALE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_MARKER_A_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_MARKER_B_WAVELENGTH, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_MARKER_C_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_MARKER_D_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_TRACE_MARKER_LEVEL, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_ZONE_MARKER_CENTER, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_ZONE_MARKER_SPAN, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DISPLAY_MODE,
	                                        MS97XX_ATTR_ANALYSIS_MODE, VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DIP_SEARCH,
	                                        MS97XX_ATTR_TRACE_MARKER_WAVELENGTH,
	                                        VI_TRUE));
	checkErr (Ivi_AddAttributeInvalidation (vi, MS97XX_ATTR_DIP_SEARCH,
	                                        MS97XX_ATTR_TRACE_MARKER_LEVEL, VI_TRUE));

Error:
    return error;
}

/*****************************************************************************
 *------------------- End Instrument Driver Source Code ---------------------*
 *****************************************************************************/
