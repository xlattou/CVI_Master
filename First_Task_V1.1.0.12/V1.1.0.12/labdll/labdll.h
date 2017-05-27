#include "extcode.h"
#pragma pack(push)
#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

double __cdecl Labdll_Imod(double Imod, double K4, double K3, double K2, 
	double K1, double K0, double start, double end);
void __cdecl Labdll_PolyFit(double X[], double Y[], int32_t polynomialOrder, 
	double PolynomialCoefficients[], double *mse, int32_t len_x, int32_t len_y, 
	int32_t len_coef);
int32_t __cdecl labdll_ExcelGET_ED(char FilePath[], LVBoolean *haveA2, 
	uint8_t A0[], int32_t len_A0, uint8_t A2[], int32_t len_A2);
int32_t __cdecl Labdll_FQC_report(char report_path[], char report_no[], 
	char PN[], char SNstr[], double AOP[], int32_t len_AOP, LVBoolean fillFlag[], 
	int32_t len_FillFlag, double ER[], int32_t len_ER, uint8_t fillERFlag);
int32_t __cdecl Labdll_Version(void);
int32_t __cdecl Labdll_ExcelGET_XFPED(char FilePath[], LVBoolean *Customized, 
	uint8_t lower[], int32_t len_Lower, uint8_t Table1[], int32_t len_Table1, 
	uint8_t Table2[], int32_t len_Table2);
void __cdecl Labdll_GetUpdateFile(char path[], int32_t *errorCode, 
	char filestr[], int32_t *filenum, int32_t len);
int32_t __cdecl GetExcelEEDFileData(char FilePath[], uint8_t A0[], 
	int32_t len, uint8_t A2[], int32_t len2, uint8_t Table1[], int32_t len3, 
	uint8_t Table2[], int32_t len4, char Version[], int32_t len5, char PN[], 
	int32_t len6);

long __cdecl LVDLLStatus(char *errStr, int errStrLen, void *module);

#ifdef __cplusplus
} // extern "C"
#endif

#pragma pack(pop)

