/****************************************************************************
 *                       MS97xx Optical Spectrum Analyzer                           
 *---------------------------------------------------------------------------
 *   Copyright (c) Anritsu Company 2000.  All Rights Reserved.         
 *---------------------------------------------------------------------------
 *                                                                          
 * Title:    MS97xx.h                                        
 * Purpose:  MS97xx Optical Spectrum Analyzer                                       
 *           instrument driver declarations.                                
 *                                                                          
 ****************************************************************************/

#ifndef __MS97XX_HEADER
#define __MS97XX_HEADER

#include <ivi.h>
#include <vpptype.h>

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

/****************************************************************************
 *----------------- Instrument Driver Revision Information -----------------*
 ****************************************************************************/
#define MS97XX_MAJOR_VERSION         1     /* Instrument driver major version */
#define MS97XX_MINOR_VERSION         0     /* Instrument driver minor version */

/**************************************************************************** 
 *------------------------------ Useful Macros -----------------------------* 
 ****************************************************************************/

/**************************************************************************** 
 *---------------------------- Attribute Defines ---------------------------* 
 ****************************************************************************/

    /*- IVI Inherent Instrument Attributes ---------------------------------*/    

        /* User Options */
#define MS97XX_ATTR_RANGE_CHECK               IVI_ATTR_RANGE_CHECK              /* ViBoolean */
#define MS97XX_ATTR_QUERY_INSTR_STATUS        IVI_ATTR_QUERY_INSTR_STATUS       /* ViBoolean */
#define MS97XX_ATTR_CACHE                     IVI_ATTR_CACHE                    /* ViBoolean */
#define MS97XX_ATTR_SIMULATE                  IVI_ATTR_SIMULATE                 /* ViBoolean */
#define MS97XX_ATTR_RECORD_COERCIONS          IVI_ATTR_RECORD_COERCIONS         /* ViBoolean */

        /* Instrument Capabilities */
#define MS97XX_ATTR_NUM_CHANNELS              IVI_ATTR_NUM_CHANNELS             /* ViInt32, Read-only */
#define MS97XX_ATTR_SPECIFIC_PREFIX           IVI_ATTR_SPECIFIC_PREFIX          /* ViString, Read-only */

        /* Version Info */
#define MS97XX_ATTR_DRIVER_MAJOR_VERSION      IVI_ATTR_DRIVER_MAJOR_VERSION     /* ViInt32, Read-only */
#define MS97XX_ATTR_DRIVER_MINOR_VERSION      IVI_ATTR_DRIVER_MINOR_VERSION     /* ViInt32, Read-only */
#define MS97XX_ATTR_DRIVER_REVISION           IVI_ATTR_DRIVER_REVISION          /* ViString, Read-only */
#define MS97XX_ATTR_ENGINE_MAJOR_VERSION      IVI_ATTR_ENGINE_MAJOR_VERSION     /* ViInt32, Read-only */
#define MS97XX_ATTR_ENGINE_MINOR_VERSION      IVI_ATTR_ENGINE_MINOR_VERSION     /* ViInt32, Read-only */
#define MS97XX_ATTR_ENGINE_REVISION           IVI_ATTR_ENGINE_REVISION          /* ViString, Read-only */

        /* Error Info */
#define MS97XX_ATTR_PRIMARY_ERROR             IVI_ATTR_PRIMARY_ERROR            /* ViInt32   */
#define MS97XX_ATTR_SECONDARY_ERROR           IVI_ATTR_SECONDARY_ERROR          /* ViInt32   */
#define MS97XX_ATTR_ERROR_ELABORATION         IVI_ATTR_ERROR_ELABORATION        /* ViString  */

        /* Advanced Session I/O */
#define MS97XX_ATTR_VISA_RM_SESSION           IVI_ATTR_VISA_RM_SESSION          /* ViSession, Read-only */
#define MS97XX_ATTR_IO_SESSION                IVI_ATTR_IO_SESSION               /* ViSession, Read-only */
#define MS97XX_ATTR_DEFER_UPDATE              IVI_ATTR_DEFER_UPDATE             /* ViBoolean */
#define MS97XX_ATTR_RETURN_DEFERRED_VALUES    IVI_ATTR_RETURN_DEFERRED_VALUES   /* ViBoolean */
    
    /*- Instrument-Specific Attributes -------------------------------------*/

#define MS97XX_ATTR_ID_QUERY_RESPONSE       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 1L)     /* ViString (Read Only) */
#define MS97XX_ATTR_CENTER_WAVELENGTH       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 3L)
#define MS97XX_ATTR_WAVELENGTH_SPAN         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 4L)
#define MS97XX_ATTR_START_WAVELENGTH        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 5L)
#define MS97XX_ATTR_STOP_WAVELENGTH         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 6L)
#define MS97XX_ATTR_WAVELENGTH_DISPLAY      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 7L)
#define MS97XX_ATTR_WAVELENGTH_OFFSET       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 8L)
#define MS97XX_ATTR_AUTO_ALIGN_STATUS       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 9L)
#define MS97XX_ATTR_LOG_SCALE               (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 10L)
#define MS97XX_ATTR_REF_LEVEL               (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 11L)
#define MS97XX_ATTR_ATTENUATOR_ON           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 12L)
#define MS97XX_ATTR_AUTO_REF_ON             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 13L)
#define MS97XX_ATTR_LEVEL_OFFSET            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 14L)
#define MS97XX_ATTR_LINEAR_SCALE            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 15L)
#define MS97XX_ATTR_LEVEL_SCALE             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 16L)
#define MS97XX_ATTR_CURRENT_TRACE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 17L)
#define MS97XX_ATTR_MEMORY_A_STORAGE        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 18L)
#define MS97XX_ATTR_MEMORY_B_STORAGE        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 19L)
#define MS97XX_ATTR_MEMORY_C_STORAGE        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 20L)
#define MS97XX_ATTR_TRACE_1_VISIBLE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 21L)
#define MS97XX_ATTR_TRACE_1_SCREEN          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 22L)
#define MS97XX_ATTR_TRACE_1_NUMERATOR       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 23L)
#define MS97XX_ATTR_TRACE_1_DENOMINATOR     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 24L)
#define MS97XX_ATTR_TRACE_2_VISIBLE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 25L)
#define MS97XX_ATTR_TRACE_2_SCREEN          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 26L)
#define MS97XX_ATTR_TRACE_2_NUMERATOR       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 27L)
#define MS97XX_ATTR_TRACE_2_DENOMINATOR     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 28L)
#define MS97XX_ATTR_TRACE_3_VISIBLE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 29L)
#define MS97XX_ATTR_TRACE_3_SCREEN          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 30L)
#define MS97XX_ATTR_TRACE_3_NUMERATOR       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 31L)
#define MS97XX_ATTR_TRACE_3_DENOMINATOR     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 32L)
#define MS97XX_ATTR_ACTIVE_TRACE            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 33L)
#define MS97XX_ATTR_ACTIVE_SCREEN           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 34L)
#define MS97XX_ATTR_UPPER_REF_LEVEL         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 35L)
#define MS97XX_ATTR_LOWER_REF_LEVEL         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 36L)
#define MS97XX_ATTR_UPPER_LOG_SCALE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 37L)
#define MS97XX_ATTR_LOWER_LOG_SCALE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 38L)
#define MS97XX_ATTR_FIRST_WDM_SIGNAL        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 39L)
#define MS97XX_ATTR_WDM_SIGNAL_SPACING      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 40L)
#define MS97XX_ATTR_WDM_SIGNAL_COUNT        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 41L)
#define MS97XX_ATTR_AUTO_MEASURING          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 42L)
#define MS97XX_ATTR_EXTERNAL_TRIGGER_DELAY  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 43L)
#define MS97XX_ATTR_SWEEP_INTERVAL_TIME     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 44L)
#define MS97XX_ATTR_ACTUAL_RESOLUTION       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 45L)
#define MS97XX_ATTR_SHOW_ACTUAL_RESOLUTION  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 46L)
#define MS97XX_ATTR_MODULATION_MODE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 47L)
#define MS97XX_ATTR_PEAK_HOLD               (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 48L)
#define MS97XX_ATTR_WIDE_DYNAMIC_RANGE      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 49L)
#define MS97XX_ATTR_AUTO_RESOLUTION         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 50L)
#define MS97XX_ATTR_SAMPLING_POINTS         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 51L)
#define MS97XX_ATTR_SMOOTHING               (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 52L)
#define MS97XX_ATTR_POINT_AVERAGE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 53L)
#define MS97XX_ATTR_SWEEP_AVERAGE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 54L)
#define MS97XX_ATTR_VIDEO_BAND_WIDTH        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 55L)
#define MS97XX_ATTR_RESOLUTION              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 56L)
#define MS97XX_ATTR_ANALYSIS_MODE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 57L)
#define MS97XX_ATTR_THRESHOLD_CUT           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 58L)
#define MS97XX_ATTR_SMSR_SIDE_MODE          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 59L)
#define MS97XX_ATTR_ENVELOPE_CUT            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 60L)
#define MS97XX_ATTR_RMS_CUT                 (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 61L)
#define MS97XX_ATTR_RMS_COEFFICIENT         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 62L)
#define MS97XX_ATTR_NDB_ATTENUATION         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 63L)
#define MS97XX_ATTR_FILE_OPTION             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 64L)
#define MS97XX_ATTR_FILE_ID_FORMAT          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 65L)
#define MS97XX_ATTR_DISKETTE_SIZE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 66L)
#define MS97XX_ATTR_FILE_NUMERIC_FORMAT     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 67L)
#define MS97XX_ATTR_FILE_INSTRUMENT_FORMAT  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 68L)
#define MS97XX_ATTR_DFB_SIDE_MODE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 69L)
#define MS97XX_ATTR_DFB_NDB_WIDTH           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 70L)
#define MS97XX_ATTR_FP_AXIS_MODE_CUT_LEVEL  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 71L)
#define MS97XX_ATTR_LED_NDB_DOWN_WAVE       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 72L)
#define MS97XX_ATTR_AMP_MEMORY              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 74L)
#define MS97XX_ATTR_AMP_CAL_STATUS          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 75L)
#define MS97XX_ATTR_AMP_CALC_MODE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 76L)
#define MS97XX_ATTR_AMP_MEASURE_METHOD      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 77L)
#define MS97XX_ATTR_AMP_FITTING_METHOD      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 78L)
#define MS97XX_ATTR_AMP_FITTING_SPAN        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 79L)
#define MS97XX_ATTR_AMP_MASKED_SPAN         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 80L)
#define MS97XX_ATTR_AMP_PIN_LOSS            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 81L)
#define MS97XX_ATTR_AMP_POUT_LOSS           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 82L)
#define MS97XX_ATTR_AMP_NF_CAL              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 83L)
#define MS97XX_ATTR_AMP_BAND_PASS_CUT       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 84L)
#define MS97XX_ATTR_AMP_FILTER_BANDWIDTH    (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 85L)
#define MS97XX_ATTR_AMP_POLAR_LOSS          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 86L)
#define MS97XX_ATTR_WDM_CUT_LEVEL           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 87L)
#define MS97XX_ATTR_WDM_MODE                (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 88L)
#define MS97XX_ATTR_WDM_SNR_DIR             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 89L)
#define MS97XX_ATTR_WDM_SNR_DW              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 90L)
#define MS97XX_ATTR_WDM_REF_PEAK            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 91L)
#define MS97XX_ATTR_WDM_TABLE_DIR           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 92L)
#define MS97XX_ATTR_WDM_TABLE_DW            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 93L)
#define MS97XX_ATTR_WDM_TABLE_NORM          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 94L)
#define MS97XX_ATTR_WDM_PEAK_TYPE           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 95L)
#define MS97XX_ATTR_WDM_THRESHOLD           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 96L)
#define MS97XX_ATTR_NF_FITTING_SPAN         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 97L)
#define MS97XX_ATTR_NF_PIN_LOSS             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 98L)
#define MS97XX_ATTR_NF_CAL                  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 99L)
#define MS97XX_ATTR_NF_ASE_CAL              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 100L)
#define MS97XX_ATTR_NF_POUT_LOSS            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 101L)
#define MS97XX_ATTR_NF_CUT_LEVEL            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 102L)
#define MS97XX_ATTR_SCREEN_MODE             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 103L)
#define MS97XX_ATTR_SELECTED_MEM            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 105L)
#define MS97XX_ATTR_LED_POWER_COMP          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 106L)
#define MS97XX_ATTR_MEASURE_MODE            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 107L)
#define MS97XX_ATTR_SEND_HEADER             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 108L)
#define MS97XX_ATTR_TERMINATOR              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 109L)
#define MS97XX_ATTR_EVENT_STATUS_ENABLE     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 110L)
#define MS97XX_ATTR_SERVICE_REQUEST_ENABLE  (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 111L)
#define MS97XX_ATTR_STATUS_BYTE             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 112L)
#define MS97XX_ATTR_EVENT_STATUS            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 113L)
#define MS97XX_ATTR_EXTENDED_STATUS_1_ENABLE (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 114L)
#define MS97XX_ATTR_EXTENDED_STATUS_2_ENABLE (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 115L)
#define MS97XX_ATTR_EXTENDED_STATUS_3_ENABLE (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 116L)
#define MS97XX_ATTR_EXTENDED_STATUS_1       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 117L)
#define MS97XX_ATTR_EXTENDED_STATUS_2       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 118L)
#define MS97XX_ATTR_EXTENDED_STATUS_3       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 119L)
#define MS97XX_ATTR_ERROR                   (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 120L)
#define MS97XX_ATTR_MARKER_A_WAVELENGTH     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 121L)
#define MS97XX_ATTR_MARKER_B_WAVELENGTH     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 122L)
#define MS97XX_ATTR_MARKER_C_LEVEL          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 123L)
#define MS97XX_ATTR_MARKER_D_LEVEL          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 124L)
#define MS97XX_ATTR_TRACE_MARKER_WAVELENGTH (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 125L)
#define MS97XX_ATTR_TRACE_MARKER_LEVEL      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 126L)
#define MS97XX_ATTR_ZONE_MARKER_CENTER      (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 127L)
#define MS97XX_ATTR_ZONE_MARKER_SPAN        (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 128L)
#define MS97XX_ATTR_MARKER_UNITS            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 129L)
#define MS97XX_ATTR_LIGHT_OUTPUT            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 130L)
#define MS97XX_ATTR_TLS_TRACK_ENABLED       (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 131L)
#define MS97XX_ATTR_TLS_CAL_STATUS          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 132L)
#define MS97XX_ATTR_POWER_MONITOR_WAVELENGTH (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 133L)
#define MS97XX_ATTR_POWER_MONITOR_RESULT    (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 134L)
#define MS97XX_ATTR_PMD_COUPLING    (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 135L)
#define MS97XX_ATTR_LAST_PMD_PEAK           (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 136L)
#define MS97XX_ATTR_FIRST_PMD_PEAK          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 137L)
#define MS97XX_ATTR_PMD_PEAK_COUNT          (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 138L)
#define MS97XX_ATTR_LONG_TERM_SAMPLE_TIME   (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 139L)
#define MS97XX_ATTR_LONG_TERM_STATE         (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 140L)
#define MS97XX_ATTR_LONG_TERM_DATA_SIZE     (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 141L)
#define MS97XX_ATTR_PEAK_SEARCH             (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 142L)
#define MS97XX_ATTR_OSA_TYPE                (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 143L)
#define MS97XX_ATTR_LONG_TERM_SPACING_DISPLAY (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 144L)
#define MS97XX_ATTR_WDM_SNR_NORM            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 145L)
#define MS97XX_ATTR_WAVELENGTH_CAL_STATUS   (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 146L)
#define MS97XX_ATTR_DISPLAY_MODE            (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 147L)
#define MS97XX_ATTR_DIP_SEARCH              (IVI_SPECIFIC_PUBLIC_ATTR_BASE + 148L)

/**************************************************************************** 
 *------------------------ Attribute Value Defines -------------------------* 
 ****************************************************************************/

        /* Instrument specific attribute value definitions */

/*
   OSA version info:
   High-order 8 bits=flags; e.g., 0x01=card OSA
   Next 8 bits=series; e.g., 0x02=20 series
   Next 8 bits=OSA; e.g., 0x06=26
   Low-order 8 bits=letter; e.g., 0x01=A
*/
#define MS97XX_VAL_OSA_TYPE_FLAGS		0xFF000000
#define MS97XX_VAL_OSA_TYPE_SERIES		0x00FF0000
#define MS97XX_VAL_OSA_TYPE_OSA			0x0000FF00
#define MS97XX_VAL_OSA_TYPE_LETTER		0x000000FF

#define MS97XX_VAL_OSA_TYPE_10_SERIES	0x00010000
#define MS97XX_VAL_OSA_TYPE_20_SERIES	0x00020000
#define MS97XX_VAL_OSA_TYPE_CARD		0x01000000

#define MS97XX_VAL_OSA_TYPE_MS9710		(MS97XX_VAL_OSA_TYPE_10_SERIES)
#define MS97XX_VAL_OSA_TYPE_MS9715		(MS97XX_VAL_OSA_TYPE_10_SERIES |\
										 0x00000500)
#define MS97XX_VAL_OSA_TYPE_MS9720		(MS97XX_VAL_OSA_TYPE_20_SERIES)
#define MS97XX_VAL_OSA_TYPE_MS9726		(MS97XX_VAL_OSA_TYPE_CARD |\
										 MS97XX_VAL_OSA_TYPE_20_SERIES |\
										 0x00000600)
#define MS97XX_VAL_OSA_TYPE_MS9728		(MS97XX_VAL_OSA_TYPE_CARD |\
										 MS97XX_VAL_OSA_TYPE_20_SERIES |\
										 0x00000800)
#define MS97XX_VAL_OSA_TYPE_MS9710B		(MS97XX_VAL_OSA_TYPE_MS9710 |\
										 0x00000002)
#define MS97XX_VAL_OSA_TYPE_MS9710C		(MS97XX_VAL_OSA_TYPE_MS9710 |\
										 0x00000003)
#define MS97XX_VAL_OSA_TYPE_MS9720A		(MS97XX_VAL_OSA_TYPE_MS9720 |\
										 0x00000001)


#define MS97XX_VAL_LEVEL_SCALE_LOG                                  0
#define MS97XX_VAL_LEVEL_SCALE_LINEAR                               1
#define MS97XX_VAL_WAVELENGTH_DISPLAY_VACUUM                        0
#define MS97XX_VAL_WAVELENGTH_DISPLAY_AIR                           1
#define MS97XX_VAL_CURRENT_TRACE_A                                  0
#define MS97XX_VAL_CURRENT_TRACE_B                                  1
#define MS97XX_VAL_CURRENT_TRACE_AB                                 2
#define MS97XX_VAL_CURRENT_TRACE_A_B                                3
#define MS97XX_VAL_CURRENT_TRACE_B_A                                4
#define MS97XX_VAL_MEMORY_STORAGE_REWRITE                           0
#define MS97XX_VAL_MEMORY_STORAGE_FIX                               1
#define MS97XX_VAL_MEMORY_STORAGE_MAX_HOLD                          2
#define MS97XX_VAL_MEMORY_STORAGE_MIN_HOLD                          3
#define MS97XX_VAL_TRACE_SCREEN_UPPER                               0
#define MS97XX_VAL_TRACE_SCREEN_LOWER                               1
#define MS97XX_VAL_TRACE_NONE                                       0
#define MS97XX_VAL_TRACE_A                                          1
#define MS97XX_VAL_TRACE_B                                          2
#define MS97XX_VAL_TRACE_C                                          3
#define MS97XX_VAL_TRACE_A_PLUS_B                                   4
#define MS97XX_VAL_TRACE_A_B                                        5
#define MS97XX_VAL_CAL_SUCCESS                                      0
#define MS97XX_VAL_CAL_IN_PROGRESS                                  1
#define MS97XX_VAL_CAL_INSUFF_LIGHT                                 2
#define MS97XX_VAL_CAL_ABORT                                        3
#define MS97XX_VAL_ACTIVE_SCREEN_LOWER                              0
#define MS97XX_VAL_ACTIVE_SCREEN_UPPER                              1
#define MS97XX_VAL_VBW_1MHZ                                         1e+006
#define MS97XX_VAL_VBW_100KHZ                                       100000.000
#define MS97XX_VAL_VBW_10KHZ                                        10000.000
#define MS97XX_VAL_VBW_1KHZ                                         1000.000
#define MS97XX_VAL_VBW_100HZ                                        100.000000
#define MS97XX_VAL_VBW_10HZ                                         10.000000
#define MS97XX_VAL_MODULATION_MODE_NORMAL                           0
#define MS97XX_VAL_MODULATION_MODE_HOLD                             1
#define MS97XX_VAL_MODULATION_MODE_TRIGGER                          2
#define MS97XX_VAL_ANALYSIS_MODE_OFF                                0
#define MS97XX_VAL_ANALYSIS_MODE_THR                                1
#define MS97XX_VAL_ANALYSIS_MODE_SMSR                               2
#define MS97XX_VAL_ANALYSIS_MODE_PWR                                3
#define MS97XX_VAL_ANALYSIS_MODE_ENV                                4
#define MS97XX_VAL_ANALYSIS_MODE_RMS                                5
#define MS97XX_VAL_FILE_OPTION_NONE                                 0
#define MS97XX_VAL_FILE_OPTION_BMP                                  1
#define MS97XX_VAL_FILE_OPTION_TXT                                  2
#define MS97XX_VAL_FILE_OPTION_BMP_TXT                              3
#define MS97XX_VAL_FILE_ID_FORMAT_NAME                              0
#define MS97XX_VAL_FILE_ID_FORMAT_NUMBER                            1
#define MS97XX_VAL_DISKETTE_SIZE_1_44MB                             0
#define MS97XX_VAL_DISKETTE_SIZE_1_2MB                              1
#define MS97XX_VAL_FILE_NUMERIC_FORMAT_LOG                          0
#define MS97XX_VAL_FILE_NUMERIC_FORMAT_LINEAR                       1
#define MS97XX_VAL_INSTRUMENT_FORMAT_MS9720                         0
#define MS97XX_VAL_INSTRUMENT_FORMAT_MS9715                         1
#define MS97XX_VAL_INSTRUMENT_FORMAT_MS9710                         2
#define MS97XX_VAL_ANALYSIS_MODE_NDB                                6
#define MS97XX_VAL_SMSR_SIDE_MODE_2ND                               0
#define MS97XX_VAL_SMSR_SIDE_MODE_LEFT                              1
#define MS97XX_VAL_SMSR_SIDE_MODE_RIGHT                             2
#define MS97XX_VAL_DFB_SIDE_MODE_2NDPEAK                            0
#define MS97XX_VAL_DFB_SIDE_MODE_LEFT                               1
#define MS97XX_VAL_DFB_SIDE_MODE_RIGHT                              2
#define MS97XX_VAL_AMP_MEMORY_PIN                                   0
#define MS97XX_VAL_AMP_MEMORY_POUT                                  1
#define MS97XX_VAL_AMP_CAL_FINISHED                                 0
#define MS97XX_VAL_AMP_CAL_INSUFF_LIGHT                             1
#define MS97XX_VAL_AMP_CAL_ABORT                                    2
#define MS97XX_VAL_AMP_CALC_NF_S_ASE                                0
#define MS97XX_VAL_AMP_CALC_NF_TOTAL                                1
#define MS97XX_VAL_AMP_NF_NO_SPEC_DIV                               0
#define MS97XX_VAL_AMP_NF_SPEC_DIV                                  1
#define MS97XX_VAL_AMP_NF_POLAR_NULL                                2
#define MS97XX_VAL_AMP_NF_PULSE                                     3
#define MS97XX_VAL_AMP_WDM                                          4
#define MS97XX_VAL_AMP_GAUSS                                        0
#define MS97XX_VAL_AMP_MEAN                                         1
#define MS97XX_VAL_WDM_NONE                                         0
#define MS97XX_VAL_WDM_MULTIPEAK                                    1
#define MS97XX_VAL_WDM_SNR                                          2
#define MS97XX_VAL_WDM_RELATIVE                                     3
#define MS97XX_VAL_WDM_TABLE                                        4
#define MS97XX_VAL_SNR_DIRECTION_LEFT                               0
#define MS97XX_VAL_SNR_DIRECTION_RIGHT                              1
#define MS97XX_VAL_SNR_DIRECTION_HIGHER                             2
#define MS97XX_VAL_WDM_TABLE_DIR_LEFT                               0
#define MS97XX_VAL_WDM_TABLE_DIR_RIGHT                              1
#define MS97XX_VAL_WDM_TABLE_DIR_HIGHER                             2
#define MS97XX_VAL_WDM_TABLE_DIR_AVERAGE                            3
#define MS97XX_VAL_PEAK_TYPE_MAX                                    0
#define MS97XX_VAL_PEAK_TYPE_THRESHOLD                              1
#define MS97XX_VAL_SCREEN_MODE_GRAPH                                0
#define MS97XX_VAL_SCREEN_MODE_TABLE                                1
#define MS97XX_VAL_TERMINATOR_LF_EOI                                0
#define MS97XX_VAL_TERMINATOR_CR_LF_EOI                             1
#define MS97XX_VAL_MARKER_UNITS_WAVELENGTH                          0
#define MS97XX_VAL_MARKER_UNITS_FREQUENCY                           1
#define MS97XX_VAL_ATTR_LONG_TERM_SUCCESS                           0
#define MS97XX_VAL_ATTR_LONG_TERM_IN_PROGRESS                       1
#define MS97XX_VAL_ATTR_LONG_TERM_FAIL                              2
#define MS97XX_VAL_TLS_CAL_STATUS_END                               0
#define MS97XX_VAL_TLS_CAL_STATUS_CALIBRATING                       1
#define MS97XX_VAL_TLS_CAL_STATUS_SUSPEND_ABNORMAL                  2
#define MS97XX_VAL_TLS_CAL_STATUS_NOT_CALIBRATED                    3
#define MS97XX_VAL_LONG_TERM_SPACING_DISPLAY_WAVELENGTH             0
#define MS97XX_VAL_LONG_TERM_SPACING_DISPLAY_FREQUENCY              1
#define MS97XX_VAL_MEASURE_MODE_SPECTRUM                            0
#define MS97XX_VAL_MEASURE_MODE_INSERTION_LOSS                      1
#define MS97XX_VAL_MEASURE_MODE_ISOLATION                           2
#define MS97XX_VAL_MEASURE_MODE_DIRECTIVITY                         3
#define MS97XX_VAL_MEASURE_MODE_RETURN_LOSS                         4
#define MS97XX_VAL_MEASURE_MODE_PMD                                 5
#define MS97XX_VAL_MEASURE_MODE_NF                                  6
#define MS97XX_VAL_MEASURE_MODE_WDM                                 7
#define MS97XX_VAL_MEASURE_MODE_LONG_TERM                           8
#define MS97XX_VAL_MEASURE_MODE_DFB                                 9
#define MS97XX_VAL_MEASURE_MODE_FP                                  10
#define MS97XX_VAL_MEASURE_MODE_LED                                 11
#define MS97XX_VAL_MEASURE_MODE_AMPLIFIER                           12
#define MS97XX_VAL_DISPLAY_MODE_NORMAL                              0
#define MS97XX_VAL_DISPLAY_MODE_OVERLAP                             1
#define MS97XX_VAL_DISPLAY_MODE_DBM_DBM                             2
#define MS97XX_VAL_DISPLAY_MODE_DBM_DB                              3
#define MS97XX_VAL_DIP_SEARCH_DIP                                   0
#define MS97XX_VAL_DIP_SEARCH_NEXT                                  1
#define MS97XX_VAL_DIP_SEARCH_LAST                                  2
#define MS97XX_VAL_DIP_SEARCH_LEFT                                  3
#define MS97XX_VAL_DIP_SEARCH_RIGHT                                 4

/* Values for use with instrument driver functions */
#define MS97XX_VAL_CAL_WAVELENGTH_DEFAULT                           0
#define MS97XX_VAL_CAL_WAVELENGTH_EXTERNAL                          1
#define MS97XX_VAL_CAL_WAVELENGTH_REFERENCE                         2
#define MS97XX_VAL_CAL_WAVELENGTH_STOP                              3
#define MS97XX_VAL_AUTO_ALIGN_DEFAULT                               0
#define MS97XX_VAL_AUTO_ALIGN_AUTO                                  1
#define MS97XX_VAL_AUTO_ALIGN_TERMINATE                             2
#define MS97XX_VAL_PEAK_SEARCH_PEAK                                 0
#define MS97XX_VAL_PEAK_SEARCH_NEXT                                 1
#define MS97XX_VAL_PEAK_SEARCH_LAST                                 2
#define MS97XX_VAL_PEAK_SEARCH_LEFT                                 3
#define MS97XX_VAL_PEAK_SEARCH_RIGHT                                4
#define MS97XX_VAL_ZONE_MARKER_ZOOM_IN                              0
#define MS97XX_VAL_ZONE_MARKER_ZOOM_OUT                             1
#define MS97XX_VAL_DATA_CONDITIONS_MEMORY                           0
#define MS97XX_VAL_DATA_CONDITIONS_TRACE                            1
#define MS97XX_VAL_MEMORY_A                                         1
#define MS97XX_VAL_MEMORY_B                                         2
#define MS97XX_VAL_MEMORY_C                                         3
#define MS97XX_VAL_AMP_RESOLUTION_CURRENT                           0
#define MS97XX_VAL_AMP_RESOLUTION_CALIBRATE                         1
#define MS97XX_VAL_WDM_PEAK_LEFT                                    0
#define MS97XX_VAL_WDM_PEAK_RIGHT                                   1
#define MS97XX_VAL_WDM_PEAK_ERROR                                   2

/**************************************************************************** 
 *---------------- Instrument Driver Function Declarations -----------------* 
 ****************************************************************************/

    /*- Init and Close Functions -------------------------------------------*/
ViStatus _VI_FUNC  MS97xx_init (ViRsrc resourceName, ViBoolean IDQuery,
                                  ViBoolean resetDevice, ViSession *vi);
ViStatus _VI_FUNC  MS97xx_InitWithOptions (ViRsrc resourceName, ViBoolean IDQuery,
                                             ViBoolean resetDevice, ViString optionString, 
                                             ViSession *newVi);
ViStatus _VI_FUNC  MS97xx_close (ViSession vi);   

    /*- Locking Functions --------------------------------------------------*/
ViStatus _VI_FUNC  MS97xx_LockSession (ViSession vi, ViBoolean *callerHasLock);   
ViStatus _VI_FUNC  MS97xx_UnlockSession (ViSession vi, ViBoolean *callerHasLock);

	/*- Instrument Functions -*/
/* Sweep */
ViStatus _VI_FUNC MS97xx_SingleSweep(ViSession vi);
ViStatus _VI_FUNC MS97xx_ContinuousSweep(ViSession vi);
ViStatus _VI_FUNC MS97xx_StopSweep(ViSession vi);
ViStatus _VI_FUNC MS97xx_PeakToCenter(ViSession vi);
ViStatus _VI_FUNC MS97xx_PeakToReference(ViSession vi);

/* Wavelength */
ViStatus _VI_FUNC MS97xx_GetStartAndStopWavelength(ViSession vi,ViReal64 *startWl,ViReal64 *stopWl);
ViStatus _VI_FUNC MS97xx_SetStartAndStopWavelength(ViSession vi,ViReal64 startWl,ViReal64 stopWl);
ViStatus _VI_FUNC MS97xx_GetCenterAndSpanWavelength(ViSession vi,ViReal64 *centerWl,ViReal64 *wlSpan);
ViStatus _VI_FUNC MS97xx_SetCenterAndSpanWavelength(ViSession vi,ViReal64 centerWl,ViReal64 wlSpan);
ViStatus _VI_FUNC MS97xx_CalibrateWavelength(ViSession vi,ViInt32 calType);
ViStatus _VI_FUNC MS97xx_AutoAlign(ViSession vi,ViInt32 alignCommand);
ViStatus _VI_FUNC MS97xx_AutoMeasure(ViSession vi);

/* Marker */
ViStatus _VI_FUNC MS97xx_GetDelta(ViSession vi,ViReal64 wavelength,ViReal64 *dWav,ViReal64 *dLevel);
ViStatus _VI_FUNC MS97xx_EraseMarkers(ViSession vi);
ViStatus _VI_FUNC MS97xx_TraceMarkerToCenter(ViSession vi);
ViStatus _VI_FUNC MS97xx_TraceMarkerToReference(ViSession vi);
ViStatus _VI_FUNC MS97xx_GetZoneMarkerCenterAndSpan(ViSession vi,ViReal64 *centerWl,ViReal64 *wlSpan);
ViStatus _VI_FUNC MS97xx_SetZoneMarkerCenterAndSpan(ViSession vi,ViReal64 centerWl,ViReal64 wlSpan);
ViStatus _VI_FUNC MS97xx_ZoneMarkerToSpan(ViSession vi);
ViStatus _VI_FUNC MS97xx_ZoneMarkerZoom(ViSession vi,ViInt32 zoomDirection);
ViStatus _VI_FUNC MS97xx_EraseZoneMarkers(ViSession vi);

/* Analysis */
ViStatus _VI_FUNC MS97xx_GetRMS_Parameters(ViSession vi,ViInt32 *cutLevel,ViReal64 *coefficient);
ViStatus _VI_FUNC MS97xx_SetRMS_Parameters(ViSession vi,ViInt32 cutLevel,ViReal64 coefficient);
ViStatus _VI_FUNC MS97xx_GetLevelAnalysis(ViSession vi,ViReal64 *centerWl,ViReal64 *width);
ViStatus _VI_FUNC MS97xx_Get_ndB_Analysis(ViSession vi,ViReal64 *centerWl,ViReal64 *width,ViInt32 *numModes);
ViStatus _VI_FUNC MS97xx_GetSMSR_Analysis(ViSession vi,ViReal64 *dWavelength,ViReal64 *dLevel);
ViStatus _VI_FUNC MS97xx_GetSpectrumPowerAnalysis(ViSession vi,ViReal64 *power,ViReal64 *centerWl);

/* Application */
ViStatus _VI_FUNC MS97xx_GetAmpParameters(ViSession vi,ViInt32 *calcMode,ViInt32 *measMethod,ViInt32 *fitMethod,ViReal64 *fitSpan,ViReal64 *maskSpan,ViReal64 *pinLoss,ViReal64 *poutLoss,
										  ViReal64 *nfCal,ViReal64 *bandCut,ViReal64 *filterBW,ViReal64 *polarLoss);
ViStatus _VI_FUNC MS97xx_SetAmpParameters(ViSession vi,ViInt32 calcMode,ViInt32 measMethod,ViInt32 fitMethod,ViReal64 fitSpan,ViReal64 maskSpan,ViReal64 pinLoss,ViReal64 poutLoss,
										  ViReal64 nfCal,ViReal64 bandCut,ViReal64 filterBW,ViReal64 polarLoss);
ViStatus _VI_FUNC MS97xx_AmpCalibrateResolution(ViSession vi,ViInt32 setting);
ViStatus _VI_FUNC MS97xx_AmpPoutToPase(ViSession vi);
ViStatus _VI_FUNC MS97xx_GetAmpResults(ViSession vi,ViReal64 *G,ViReal64 *NF,ViReal64 *signalWl,ViReal64 *ASElevel,ViReal64 *resolution);

ViStatus _VI_FUNC MS97xx_GetDFB_Parameters(ViSession vi,ViInt32 *sideMode,ViInt32 *ndB_Width);
ViStatus _VI_FUNC MS97xx_SetDFB_Parameters(ViSession vi,ViInt32 sideMode,ViInt32 ndB_Width);
ViStatus _VI_FUNC MS97xx_GetDFB_Results(ViSession vi,ViReal64 *SMSR,ViReal64 *ndbBW,ViReal64 *peakWl,ViReal64 *peakLevel,ViReal64 *sideWl,ViReal64 *sideLevel,ViReal64 *modeOffset,ViReal64 *stopBand,ViReal64 *centerOffset);
ViStatus _VI_FUNC MS97xx_GetFP_Results(ViSession vi,ViReal64 *fwhmWl,ViReal64 *centerWl,ViReal64 *peakWl,ViReal64 *peakLevel,ViInt32 *modes,ViReal64 *modeSpacing,ViReal64 *power, ViReal64 *sigma);
ViStatus _VI_FUNC MS97xx_GetLED_Parameters(ViSession vi,ViInt32 *ndB_Down,ViReal64 *powerComp);
ViStatus _VI_FUNC MS97xx_SetLED_Parameters(ViSession vi,ViInt32 ndB_Down,ViReal64 powerComp);
ViStatus _VI_FUNC MS97xx_GetLED_Results(ViSession vi,ViReal64 *centerWl,ViReal64 *ndbWl,ViReal64 *fwhmWl,ViReal64 *ndbBW,ViReal64 *peakWl,ViReal64 *peakLevel,ViReal64 *pkPowDens,ViReal64 *power);
ViStatus _VI_FUNC MS97xx_GetPMD_Results(ViSession vi,ViReal64 *deltaT,ViReal64 *firstPeak,ViReal64 *lastPeak,ViInt32 *peakCount);

ViStatus _VI_FUNC MS97xx_GetWDM_MpkResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level);
ViStatus _VI_FUNC MS97xx_GetWDM_MpkBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[]);
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_Parameters(ViSession vi,ViInt32 *direction,ViReal64 *dWavelength,ViBoolean *normalize);
ViStatus _VI_FUNC MS97xx_SetWDM_SNR_Parameters(ViSession vi,ViInt32 direction,ViReal64 dWavelength,ViBoolean normalize);
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_Results(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *SNR,ViInt32 *direction);
ViStatus _VI_FUNC MS97xx_GetWDM_SNR_BulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 SNRs[],ViInt32 directions[]);
ViStatus _VI_FUNC MS97xx_GetWDM_RelResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *spacing,ViReal64 *relWavelength,ViReal64 *level,ViReal64 *relLevel);
ViStatus _VI_FUNC MS97xx_GetWDM_RelBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViInt32 *refPeak,ViReal64 wavelengths[],ViReal64 spacings[],ViReal64 relWavelengths[],ViReal64 levels[],ViReal64 relLevels[]);
ViStatus _VI_FUNC MS97xx_GetWDM_TableParameters(ViSession vi,ViInt32 *direction,ViReal64 *dWavelength,ViBoolean *normalize);
ViStatus _VI_FUNC MS97xx_SetWDM_TableParameters(ViSession vi,ViInt32 direction,ViReal64 dWavelength,ViBoolean normalize);
ViStatus _VI_FUNC MS97xx_GetWDM_TableResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *freq,ViReal64 *level,ViReal64 *SNR,ViInt32 *direction,ViReal64 *spacing,ViReal64 *freqSpacing);
ViStatus _VI_FUNC MS97xx_GetWDM_TableBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 freqs[],ViReal64 levels[],ViReal64 SNRs[],ViInt32 directions[],ViReal64 spacings[],ViReal64 freqSpacings[]);
ViStatus _VI_FUNC MS97xx_GetWDM_GainResults(ViSession vi,ViReal64 *gainTilt);
ViStatus _VI_FUNC MS97xx_GetWDM_PeakCount(ViSession vi,ViInt32 *numPeaks);

ViStatus _VI_FUNC MS97xx_GetI_LossResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *insertionLoss);
ViStatus _VI_FUNC MS97xx_GetI_LossBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 insertionLosses[]);
ViStatus _VI_FUNC MS97xx_GetIsoResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *isolation);
ViStatus _VI_FUNC MS97xx_GetIsoBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 isolations[]);
ViStatus _VI_FUNC MS97xx_GetDirResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *directivity);
ViStatus _VI_FUNC MS97xx_GetDirBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 directivities[]);
ViStatus _VI_FUNC MS97xx_GetR_LossResults(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *level,ViReal64 *relLevel,ViReal64 *returnLoss);
ViStatus _VI_FUNC MS97xx_GetR_LossBulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 levels[],ViReal64 relLevels[],ViReal64 returnLosses[]);
ViStatus _VI_FUNC MS97xx_GetNF_Parameters(ViSession vi,ViReal64 *fitSpan,ViReal64 *pinLoss,ViReal64 *cal,ViReal64 *aseCal,ViReal64 *poutLoss,ViInt32 *cutLevel);
ViStatus _VI_FUNC MS97xx_SetNF_Parameters(ViSession vi,ViReal64 fitSpan,ViReal64 pinLoss,ViReal64 cal,ViReal64 aseCal,ViReal64 poutLoss,ViInt32 cutLevel);
ViStatus _VI_FUNC MS97xx_GetNF_Results(ViSession vi,ViInt32 peak,ViReal64 *wavelength,ViReal64 *NF,ViReal64 *gain,ViReal64 *aseLevel,ViReal64 *pinPeakLevel,ViReal64 *poutPeakLevel);
ViStatus _VI_FUNC MS97xx_GetNF_BulkResults(ViSession vi,ViInt32 maxPeaks,ViInt32 *numPeaks,ViReal64 wavelengths[],ViReal64 NFs[],ViReal64 gains[],ViReal64 aseLevels[],ViReal64 pinPeakLevels[],ViReal64 poutPeakLevels[]);

/* Memory/Trace */
ViStatus _VI_FUNC MS97xx_GetTraceParameters(ViSession vi,ViInt32 trace,ViBoolean *visible,ViInt32 *screen,ViInt32 *numerator,ViInt32 *denominator);
ViStatus _VI_FUNC MS97xx_SetTraceParameters(ViSession vi,ViInt32 trace,ViBoolean visible,ViInt32 screen,ViInt32 numerator,ViInt32 denominator);
ViStatus _VI_FUNC MS97xx_GetMemory(ViSession vi,ViInt32 memory,ViInt32 maxPoints,ViReal64 data[],ViInt32 *numPoints);
ViStatus _VI_FUNC MS97xx_GetTrace(ViSession vi,ViInt32 trace,ViInt32 maxPoints,ViReal64 data[],ViInt32 *numPoints);
ViStatus _VI_FUNC MS97xx_GetDataConditions(ViSession vi,ViInt32 dataType,ViInt32 memory,ViReal64 *startWl,ViReal64 *stopWl,ViInt32 *samplePoints);
ViStatus _VI_FUNC MS97xx_GetWDM_Signal(ViSession vi,ViInt32 channel,ViReal64 *wavelength);
ViStatus _VI_FUNC MS97xx_SetWDM_Signal(ViSession vi,ViInt32 channel,ViReal64 wavelength);
ViStatus _VI_FUNC MS97xx_GetWDM_Signals(ViSession vi,ViInt32 maxPoints,ViReal64 wavelengths[],ViInt32 *numPoints);

/* File/Print */
ViStatus _VI_FUNC MS97xx_GetFileParameters(ViSession vi,ViInt32 *outputFormat,ViInt32 *inputFormat,ViInt32 *numericFormat,ViInt32 *instrumentFormat,ViInt32 *diskSize);
ViStatus _VI_FUNC MS97xx_SetFileParameters(ViSession vi,ViInt32 outputFormat,ViInt32 inputFormat,ViInt32 numericFormat,ViInt32 instrumentFormat,ViInt32 diskSize);
ViStatus _VI_FUNC MS97xx_Save(ViSession vi,ViChar fileName[]);
ViStatus _VI_FUNC MS97xx_Recall(ViSession vi,ViChar fileName[]);
ViStatus _VI_FUNC MS97xx_SaveConditions(ViSession vi,ViInt32 backupMemory);
ViStatus _VI_FUNC MS97xx_RecallConditions(ViSession vi,ViInt32 backupMemory);
ViStatus _VI_FUNC MS97xx_RecallMemoryConditions(ViSession vi,ViInt32 memory);
ViStatus _VI_FUNC MS97xx_DeleteFile(ViSession vi,ViChar fileName[]);
ViStatus _VI_FUNC MS97xx_Format(ViSession vi);
ViStatus _VI_FUNC MS97xx_Print(ViSession vi);
ViStatus _VI_FUNC MS97xx_Feed(ViSession vi,ViInt32 lines);

/* Long-Term */
ViStatus _VI_FUNC MS97xx_StartLongTermTest(ViSession vi,ViChar fileName[]);
ViStatus _VI_FUNC MS97xx_StopLongTermTest(ViSession vi);
ViStatus _VI_FUNC MS97xx_GetLongTermPeak(ViSession vi,ViInt32 peak,ViReal64 *meanWave,ViReal64 *maxWave,ViReal64 *minWave,ViReal64 *meanLevel,ViReal64 *maxLevel,ViReal64 *minLevel,ViReal64 *meanSNR,ViReal64 *maxSNR,ViReal64 *minSNR);
ViStatus _VI_FUNC MS97xx_GetLongTermStartPeak(ViSession vi,ViInt32 peak,ViReal64 *wave,ViReal64 *level,ViReal64 *SNR);
ViStatus _VI_FUNC MS97xx_GetLongTermPower(ViSession vi,ViReal64 *meanPower,ViReal64 *maxPower,ViReal64 *minPower);
ViStatus _VI_FUNC MS97xx_GetLongTermGainTilt(ViSession vi,ViReal64 *meanTilt,ViReal64 *maxTilt,ViReal64 *minTilt);
ViStatus _VI_FUNC MS97xx_GetLongTermSlope(ViSession vi,ViReal64 *meanSlope,ViReal64 *maxSlope,ViReal64 *minSlope);
ViStatus _VI_FUNC MS97xx_GetLongTermTime(ViSession vi,ViInt32 *startHour,ViInt32 *startMinute,ViInt32 *stopHour,ViInt32 *stopMinute);
ViStatus _VI_FUNC MS97xx_GetLongTermPeakMark(ViSession vi,ViInt32 peak,ViBoolean *isMarked);
ViStatus _VI_FUNC MS97xx_SetLongTermPeakMark(ViSession vi,ViInt32 peak,ViBoolean mark);

/* GPIB */
ViStatus _VI_FUNC MS97xx_ClearStatus(ViSession vi);
ViStatus _VI_FUNC MS97xx_WaitUntilOperationComplete(ViSession vi,ViInt32 timeoutSeconds);

/* Display */
ViStatus _VI_FUNC MS97xx_ClearGraph(ViSession vi);

    /*- Error Functions ----------------------------------------------------*/
ViStatus _VI_FUNC  MS97xx_error_query (ViSession vi, ViInt32 *errorCode,
                                         ViChar errorMessage[]);
ViStatus _VI_FUNC  MS97xx_GetErrorInfo (ViSession vi, ViStatus *primaryError, 
                                          ViStatus *secondaryError, 
                                          ViChar errorElaboration[256]);
ViStatus _VI_FUNC  MS97xx_ClearErrorInfo (ViSession vi);
ViStatus _VI_FUNC  MS97xx_error_message (ViSession vi, ViStatus errorCode,
                                           ViChar errorMessage[256]);
    
    /*- Utility Functions --------------------------------------------------*/
ViStatus _VI_FUNC  MS97xx_reset (ViSession vi);
ViStatus _VI_FUNC  MS97xx_self_test (ViSession vi, ViInt16 *selfTestResult,
                                       ViChar selfTestMessage[]);
ViStatus _VI_FUNC  MS97xx_revision_query (ViSession vi, 
                                            ViChar instrumentDriverRevision[],
                                            ViChar firmwareRevision[]);
ViStatus _VI_FUNC  MS97xx_WriteInstrData (ViSession vi, ViConstString writeBuffer); 
ViStatus _VI_FUNC  MS97xx_ReadInstrData  (ViSession vi, ViInt32 numBytes, 
                                            ViChar rdBuf[], ViInt32 *bytesRead);

    /*- Set, Get, and Check Attribute Functions ----------------------------*/
ViStatus _VI_FUNC  MS97xx_GetAttributeViInt32 (ViSession vi, ViConstString channelName, ViAttr attribute, ViInt32 *value);
ViStatus _VI_FUNC  MS97xx_GetAttributeViReal64 (ViSession vi, ViConstString channelName, ViAttr attribute, ViReal64 *value);
ViStatus _VI_FUNC  MS97xx_GetAttributeViString (ViSession vi, ViConstString channelName, ViAttr attribute, ViInt32 bufSize, ViChar value[]); 
ViStatus _VI_FUNC  MS97xx_GetAttributeViSession (ViSession vi, ViConstString channelName, ViAttr attribute, ViSession *value);
ViStatus _VI_FUNC  MS97xx_GetAttributeViBoolean (ViSession vi, ViConstString channelName, ViAttr attribute, ViBoolean *value);

ViStatus _VI_FUNC  MS97xx_SetAttributeViInt32 (ViSession vi, ViConstString channelName, ViAttr attribute, ViInt32 value);
ViStatus _VI_FUNC  MS97xx_SetAttributeViReal64 (ViSession vi, ViConstString channelName, ViAttr attribute, ViReal64 value);
ViStatus _VI_FUNC  MS97xx_SetAttributeViString (ViSession vi, ViConstString channelName, ViAttr attribute, ViConstString value); 
ViStatus _VI_FUNC  MS97xx_SetAttributeViSession (ViSession vi, ViConstString channelName, ViAttr attribute, ViSession value);
ViStatus _VI_FUNC  MS97xx_SetAttributeViBoolean (ViSession vi, ViConstString channelName, ViAttr attribute, ViBoolean value);

ViStatus _VI_FUNC  MS97xx_CheckAttributeViInt32 (ViSession vi, ViConstString channelName, ViAttr attribute, ViInt32 value);
ViStatus _VI_FUNC  MS97xx_CheckAttributeViReal64 (ViSession vi, ViConstString channelName, ViAttr attribute, ViReal64 value);
ViStatus _VI_FUNC  MS97xx_CheckAttributeViString (ViSession vi, ViConstString channelName, ViAttr attribute, ViConstString value); 
ViStatus _VI_FUNC  MS97xx_CheckAttributeViSession (ViSession vi, ViConstString channelName, ViAttr attribute, ViSession value);
ViStatus _VI_FUNC  MS97xx_CheckAttributeViBoolean (ViSession vi, ViConstString channelName, ViAttr attribute, ViBoolean value);

/*------------------------*/
ViStatus _VI_FUNC MS97xxAttrAttenuatorOn_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViBoolean value);
ViStatus _VI_FUNC MS97xxAttrSamplingPoints_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
ViStatus _VI_FUNC MS97xxAttrResolution_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViReal64 value);
ViStatus _VI_FUNC MS97xxAttrVideoBandWidth_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViReal64 value);
ViStatus _VI_FUNC MS97xxAttrFpAxisModeCutLevel_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
ViStatus _VI_FUNC MS97xxSetDfb_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
ViStatus _VI_FUNC MS97xxAttrRefLevel_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViReal64 value);
ViStatus _VI_FUNC MS97xxAttrLogScale_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViReal64 value);
ViStatus _VI_FUNC MS97xxSetWDP_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
ViStatus _VI_FUNC MS97xxSetDfbOFF_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
ViStatus _VI_FUNC MS97xxAttrThresholdCut_WriteCallback (ViSession vi, ViSession io, ViConstString channelName, ViAttr attributeId, ViInt32 value);
/*-------------------------*/

    /*********************************************************
        Functions reserved for class driver use only.
        End-users should not call these functions.  
     *********************************************************/
ViStatus _VI_FUNC  MS97xx_IviInit (ViRsrc resourceName, ViBoolean IDQuery, 
                                     ViBoolean reset, ViSession vi);
ViStatus _VI_FUNC  MS97xx_IviClose (ViSession vi);   

/****************************************************************************
 *------------------------ Error And Completion Codes ----------------------*
 ****************************************************************************/
#define MS97XX_ERROR_INVALID_FILE_NAME  (IVI_SPECIFIC_ERROR_BASE + 1)
#define MS97XX_ERROR_INCORRECT_MODE     (IVI_SPECIFIC_ERROR_BASE + 2)
#define MS97XX_ERROR_MARKER_NOT_ON		(IVI_SPECIFIC_ERROR_BASE + 3)

#define MS97XX_WARN_OPERATION_TIMEOUT   (IVI_SPECIFIC_WARN_BASE + 1)

/**************************************************************************** 
 *---------------------------- End Include File ----------------------------* 
 ****************************************************************************/
#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif
#endif /* __MS97XX_HEADER */
