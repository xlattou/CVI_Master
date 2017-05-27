/****************************************************************************
 *
 * File:                cvidll.h
 *
 * Author:              SUPERXON LTD.(Roger Li)
 *
 * Description:         自动测试通用函数动态库文件
 *
 * Time:                2012-3-22 
 *
 * 
版本历史

======================================
更新时间：2015-03-25
更新版本：V1.1.2.42
更新人：  Eric.Yao
1.升级MyDLLReadModuleSN函数，支持8077协议产品的模块条码；

======================================
更新时间：2015-04-22      
更新版本：V1.1.2.41
更新人：  Eric.Yao
1、添加客户DEONET在CustomerNameArray

======================================
更新时间：2015-03-25
更新版本：V1.1.2.40
更新人：  Eric.Yao
1.MyDLL_GET_SEVB_SN函数增加对EVB31deSN读取支持；

======================================
更新时间：2015-02-06
更新版本：V1.1.2.39
更新人：  Eric.Yao
1.更新MyDLLGetInnerSN函数功能，当0xA2的240-255字节没有InnerSN时，程序从0xA0的240-255字节读取InnerSN；

======================================
更新时间：2014-07-08
更新版本：V1.1.2.38
更新人：  Eric.Yao
1、添加客户MI在CustomerNameArray
2、添加产品类型XGPON_ONU在ProductTypeArray

======================================
更新时间：2014-01-20
更新版本：V1.1.2.37
更新人：  Eric.Yao
1、添加客户MC在CustomerNameArray

======================================
更新时间：2013-12-27
更新版本：V1.1.2.36
更新人：  Eric.Yao 
更新内容
1、修改数据库账户的密码；

======================================
更新时间：2013-10-12
更新版本：V1.1.2.35
更新人：  Eric.Yao 
更新内容
1、新增产线标示定义PRODUCTLINE_OFA表示成都瓯福安；

======================================
更新时间：2013-10-11
更新版本：V1.1.2.34
更新人：  Eric.Yao
1、添加客户AD在CustomerNameArray

======================================
更新时间：2013-07-18
更新版本：V1.1.2.33
更新人：  Roger
1.添加SEVB0027-4001的USB序列号读取功能

======================================
更新时间：2013-05-02
更新版本：V1.1.2.32
更新人：  Sally
1.读取innersn函数中，需将A2索引表切换回默认table

======================================
更新时间：2013-03-11
更新版本：V1.1.2.31
更新人：  Eric.Yao
1、添加客户DZ在CustomerNameArray
2、MyDLL_8472_Getxxx系列函数采用局部union定义

======================================
更新时间：2013-02-03
更新版本：V1.1.2.30
更新人：  Eric.Yao
1、添加客户AP在CustomerNameArray

======================================
更新时间：2013-01-11
更新版本：V1.1.2.29
更新人：  Sally
1、ProductTypeArray里增加SFP_PLUS产品类型

======================================
更新时间：2012-12-13
更新版本：V1.1.2.28
更新人：  Roger
1、添加函数MyDLL_DBOA_Init
2、删除定义
#define  DB_ID "superxon"
#define  DB_PW "superxontestdata"
#define  DBOA_IP  "172.16.0.75"
#define  DBOA_IP2 "172.30.0.75"   //192.168.30.75
#define  DBOA_DB  "sup_web"
#define  DBOA_ID  "root"   
#define  DBOA_PW  "123"   

======================================
更新时间：2012-11-29
更新版本：V1.1.2.27
更新人：  Eric.Yao
1、添加客户FLEX(伟创力)在CustomerNameArray

======================================
更新时间：2012-11-27
更新版本：V1.1.2.26
更新人：  Roger
1、添加WNetAddConnection2函数出现1219错误代码的处理

======================================
更新时间：2012-09-29
更新版本：V1.1.2.25
更新人：  Sally
1、增加change_batch_osa和reform_osa工序
2、MyDLLDeleteSpace已被函数MyDLL_TRIM代替

======================================
更新时间：2012-09-18
更新版本：V1.1.2.24
更新人：  Sally
1、将工序名称统一改为大写

======================================
更新时间：2012-09-10
更新版本：V1.1.2.23
更新人：  Eric.Yao
1、添加客户ZY在CustomerNameArray

======================================
更新时间：2012-07-06
更新版本：V1.1.2.22
更新人：  Roger 
1、添加OQA工序
2、添加函数MyDLLGetPCBA
****************************************************************************/

#ifndef _CVIDLL_H_
#define _CVIDLL_H_ 

#include <cvidef.h>
#include <windows.h> 
#include <ansi_c.h>
#include <utility.h>
#include <winnetwk.h>    
#include <cvirte.h>		
#include <userint.h>
#include "global.h"
/*Here are the function prototypes suitable for including in a program which uses the dll. */
 
#define  VER	"1.1.2.35"

/*************************************************************for base64*************************************************************/
#define BASE64_PAD64 '='

static char base64_alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                          'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                          'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a',
                          'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                          'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
                          't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1',
                          '2', '3', '4', '5', '6', '7', '8', '9', '+',
                          '/'};
                          
static unsigned char base64_suffix_map[256] = 
	{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255,  62, 255, 255, 255,  63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
     255, 255, 255, 255, 255,  0,   1,    2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,   16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, 255,  26,  27,  28,
     29,   30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,   50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}; 
static char cmove_bits(unsigned char src, unsigned lnum, unsigned rnum);
/*************************************************************end for base64*************************************************************/ 

/*************************************************************end for getdiskn************************************************************/   
//  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  		4
#define  IDENTIFY_BUFFER_SIZE  	512

//  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition

//  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

//  GETVERSIONOUTPARAMS contains the data returned from the 
//  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
   BYTE bVersion;      // Binary driver version.
   BYTE bRevision;     // Binary driver revision.
   BYTE bReserved;     // Not used.
   BYTE bIDEDeviceMap; // Bit map of IDE devices.
   DWORD fCapabilities; // Bit mask of driver capabilities.
   DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

//  IDE registers
typedef struct _IDEREGS
{
   BYTE bFeaturesReg;       // Used for specifying SMART "commands".
   BYTE bSectorCountReg;    // IDE sector count register
   BYTE bSectorNumberReg;   // IDE sector number register
   BYTE bCylLowReg;         // IDE low order cylinder value
   BYTE bCylHighReg;        // IDE high order cylinder value
   BYTE bDriveHeadReg;      // IDE drive/head register
   BYTE bCommandReg;        // Actual IDE command.
   BYTE bReserved;          // reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;

//  SENDCMDINPARAMS contains the input parameters for the 
//  Send Command to Drive function.
typedef struct _SENDCMDINPARAMS
{
   DWORD     cBufferSize;   //  Buffer size in bytes
   IDEREGS   irDriveRegs;   //  Structure with drive register values.
   BYTE bDriveNumber;       //  Physical drive number to send 
                            //  command to (0,1,2,3).
   BYTE bReserved[3];       //  Reserved for future expansion.
   DWORD     dwReserved[4]; //  For future use.
   BYTE      bBuffer[1];    //  Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

// Status returned from driver
typedef struct _DRIVERSTATUS
{
   BYTE  bDriverError;  //  Error code from driver, or 0 if no error.
   BYTE  bIDEStatus;    //  Contents of IDE Error register.
                        //  Only valid when bDriverError is SMART_IDE_ERROR.
   BYTE  bReserved[2];  //  Reserved for future expansion.
   DWORD  dwReserved[2];  //  Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

// Structure returned by PhysicalDrive IOCTL for several commands
typedef struct _SENDCMDOUTPARAMS
{
   DWORD         cBufferSize;   //  Size of bBuffer in bytes
   DRIVERSTATUS  DriverStatus;  //  Driver status structure.
   BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

BYTE 	IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
BOOL 	DoIDENTIFY (HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD);
char 	*ConvertToString (DWORD diskdata [256], int firstIndex, int lastIndex);
/*************************************************************end for getdiskn************************************************************/

//定义工艺过程结构体 
struct  struProcessLOG	  
{
char	PN[50];				//PN
char	SN[50];		   		//模块的序列号
char	Operator[10];		//操作员序号
int		ResultsID;			//Results表的index
char   	Log_Action[50];		//过程定义
char	Action_Time[50];	//记录时间
char	Parameter[50];		//参数类型的定义
char	P_Value[50];		//参数值
char	Comments[4096];		//备注 
char	SoftVersion[50];	//软件版本
char	StationID[50];		//工位计算机名称  
int		RunTime;			//测试程序运行时间
int		RunCount;			//当前工位运行次数 
} my_struProcessLOG;

extern int 	 __stdcall DLLIMPORT MyDLLFloattoInt(float numeric);						//浮点型数据转换到标准32位整型
extern float __stdcall DLLIMPORT MyDLLInttoFloat(int numeric);  						//标准32位整型数据转换到浮点型

/*******************************************************************/   
/*
函 数 名 称：	MyDLLAddConnection  
功 能 描 述：	建立网络映射盘符 
参 数 说 明：	LocalName:	盘符  比如"H:" 
             	RemotePath: 网络共享路径 \\192.168.1.11\test1\test2 注意要加转义字符
				lpUserName：用户名
				lpPassword：密码
返回值 说明：	连接成功返回0，否则为非0  
/*******************************************************************/  
extern int 	__stdcall DLLIMPORT MyDLLAddConnection(IN LPSTR LocalName, IN LPSTR RemotePath, IN LPCSTR lpUserName, IN LPCSTR lpPassword);

extern int 	__stdcall DLLIMPORT MyDLLCancelConnection(IN LPSTR LocalName); 
extern int  __stdcall DLLIMPORT MyDLLBase64_encode(const char *data_in, char *data_out);
extern int  __stdcall DLLIMPORT MyDLLBase64_decode(const char *data_in, char *data_out); 
extern int  __stdcall DLLIMPORT MyDLLReadPhysicalDrive(char *HardDriveSerialNumber); 	//0:fail;1:OK, 读取硬盘物理地址，只在WindowsXP使用过，Vista下由于安全原因可能会被禁止，没有测试过
extern BOOL __stdcall DLLIMPORT MyDLLGetComputerName(char *ComputerName); 				//读取计算机名称
extern BOOL __stdcall DLLIMPORT MyDLLGetServerIP(char *ServerIP);						//REGKEY_HKCU, "Software\\Superxon", "Server" 
extern BOOL __stdcall DLLIMPORT MyDLLGetRegInfo(const char *DiskSN, char *RegInfo);		//根据输入的DiskSN计算校验字符串RegInfo
extern int	__stdcall DLLIMPORT MyDLLCheckLicense(void);
extern BOOL __stdcall DLLIMPORT MyDLLCalSystemDate(void);								//重MyDLLGetServerIP函数中获取SQLserver的IP，根据此IP作为系统的标准时间校准
extern void	__stdcall DLLIMPORT MyDLLCheckSN(char *s);									//将多余的字符删除(a-z, A-Z, 0-9),并将SN转化为大写,支持字符串个数为128 
extern void __stdcall DLLIMPORT MyDLLCheckStr_Ex(char *s);								//将多余的字符删除(a-z, A-Z, 0-9),并将SN转化为大写,支持字符串个数为128, 包含小数点.  
extern void __stdcall DLLIMPORT MyDLLGETPartNumber(int panel, int control);				//从exe目录下的data文件的PartNumber.txt获取PN并赋值到panel指定的界面的control的ring控件上     											
extern void __stdcall DLLIMPORT MyDLLGETPartNumber_ex(int panel, int control, char file[]);			//从指定文件获取已“;”符号分割的信息并赋值到panel指定的界面的control的ring控件上     											
																
//080715:添加注册功能函数,使用这部分函数必须要在工程目录中添加文件license.uir和license.h
/*************************************************************Login*************************************************************/
struct  struLicense
{
char username[50];	//程序用户名称
char password[50];	//程序用户密码
char power[1024];	//程序用户权限
} my_struLicense; 

int panelHandle_log; 
int	g_passwordICtrlID;
int panPW;
int g_pwCtrlID_old;
int g_pwCtrlID_new; 
int g_pwCtrlID_new2; 

extern BOOL __stdcall DLLIMPORT MyDLL_License_Load(void); 			//load注册界面
extern BOOL __stdcall DLLIMPORT MyDLL_License_Check(char *username, char *password, char *power);	//检查注册信息，并返回账号的信息
extern BOOL __stdcall DLLIMPORT MyDLL_License_Close(void); 			//关闭注册界面
extern int  __stdcall DLLIMPORT MyDLL_License_Entry_Username(void); //Username上输入回车 
extern int  __stdcall DLLIMPORT MyDLL_License_Entry_Password(void); //Password上输入回车  
extern int __stdcall DLLIMPORT MyDLL_License_PW_Open(void); 		//打开密码更新界面
extern int __stdcall DLLIMPORT MyDLL_License_PW_Close(void); 		//关闭密码更新界面
extern int __stdcall DLLIMPORT MyDLL_License_PW_Update(void); 		//提交密码更新
extern void __stdcall DLLIMPORT MyDLL_License_PW_Entry (int control);//在密码更新界面上输入回车

//090327 添加注册控制变量
#define  LICENSE_ENABLE      1
#define  LICENSE_DISENABLE   0
BOOL	 license_status;
/*************************************************************Login*************************************************************/ 

/*******************************************************************/   
/*090327 添加CRC32校验值计算函数   
函 数 名 称：	MyDLLCRC32  
功 能 描 述：	执行对数据段的CRC32循环冗余校验  
参 数 说 明：	buffer[in]:待校验数据  
             	size[in]:  待校验数据长度  
返回值 说明：	循环冗余校验结果  
/*******************************************************************/  
extern unsigned long __stdcall DLLIMPORT MyDLLCRC32(unsigned char *buffer, unsigned long size);

extern void __stdcall DLLIMPORT MyDLLFormatDate(char *s);			//转换日期格式 04-03-2009 13:29:42 变为 2009-04-03 13:29:42， 便于写入数据库后进行日期比较

/*******************************************************************/   
/*090629 添加SEVB序列号读取函数   
函 数 名 称：	MyDLL_GET_SEVB_SN  
功 能 描 述：	根据输入的SEVB类型，能直接将获取到的SEVB序号赋值指定的控件 
参 数 说 明：	SEVB_TYPE:SEVB类型，暂时可支持SEVB1、SEVB5、SEVB6、SEVB12（如果选用SEVB12，只筛选出设备编号SBERT开头的设备序列号
             	panel：界面ID 
				control：控件ID 可支持控件类型ring or list
返回值 说明：	函数运行结果，0表示函数运行成功，<0表示函数运行失败  
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLL_GET_SEVB_SN(const int SEVB_TYPE, int panel, int control); 

/*******************************************************************/   
/*090820 添加CRC16校验值计算函数   
函 数 名 称：	MyDLLCRC16  
功 能 描 述：	执行对数据段的CRC16循环冗余校验  
参 数 说 明：	buffer[in]:待校验数据  
             	size[in]:  待校验数据长度  
返回值 说明：	循环冗余校验结果  
/*******************************************************************/  
extern unsigned short __stdcall DLLIMPORT MyDLLCRC16(unsigned char *buffer, unsigned long size);

/*******************************************************************/   
/*
函数名称：	MyDLLHEXstrtoINTnum
创建日期：	2009-10-28
创建作者：	Roger.Li
功能描述：	将16进制字符串改为64bit整数型数字，最多支持16个（0~9，A~F）ASCII字符  0xFFFFFFFFFFFFFFFF， 
参数说明：	*str [in] 输入字符串  
            *num [out]转换结果 
返回值说明：0函数运行成功，非0函数运行错误，-2输入字符串长度超出16个字符
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLLHEXstrtoINTnum(char *str, unsigned long long *num);

/*******************************************************************/   
/*
函数名称：	MyDLL_TRIM
创建日期：	2009-12-09
创建作者：	Roger.Li
功能描述：	将输入字符串中的空格字符去掉，空格后的字符前移 
参数说明：	*s [in and out] 输入字符串和转换结果  
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLL_TRIM(char *s);								

//获取本机系统时间
extern int __stdcall DLLIMPORT MyDLL_GetDateTime(char *DateTime);  
//获取服务器系统时间
extern int __stdcall DLLIMPORT MyDLLGetSystemDate(char *DateTime);

enum enum_LIFE
{   
LIFE_FIBER_STAND,
LIFE_FIBER,
LIFE_SFP,
LIFE_SFF
} LIFE_TYPE; 

#define  FIBER_RUNTIME_MAX   	300 		//光纤一次性可使用次数最大值 
#define  FIBER_RUNTIME_HIS_MAX	1500		//光纤总共可使用次数最大值

#define  SFP_RUNTIME_MAX   		15000 		//SFP一次性可使用次数最大值 
#define  SFP_RUNTIME_HIS_MAX	15000		//SFP总共可使用次数最大值

#define  SFF_RUNTIME_MAX   		5000 		//SFF一次性可使用次数最大值 
#define  SFF_RUNTIME_HIS_MAX	5000		//SFF总共可使用次数最大值

/*******************************************************************/   
/*
函数名称：	MyDLL_FiberManage_Init
创建日期：	2010-05-05
创建作者：	Roger.Li
功能描述：	检查输入的fiber编号是否可以使用
参数说明：	hdbc [in] 数据库连接句柄 
            *fibersn [int] fiber编号
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLL_FiberManage_Init (int hdbc, char *fibersn);
/*扩展函数说明，增加寿命管理类型参数，扩展函数最为基础单元，原函数调用扩展函数，以保持版本兼容性*/
extern int __stdcall DLLIMPORT MyDLL_FiberManage_Init_EX (int hdbc, char sn[], enum enum_LIFE LIFE_TYPE);

/*******************************************************************/   
/*
函数名称：	MyDLL_FiberManage_Run
创建日期：	2010-05-05
创建作者：	Roger.Li
功能描述：	更新输入的fiber编号的状态
参数说明：	hdbc [in] 数据库连接句柄 
            *fibersn [int] fiber编号
			*runtime [int] 当前光纤已使用次数
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLL_FiberManage_Run (int hdbc, char *fibersn, int *runtime);
extern int __stdcall DLLIMPORT MyDLL_FiberManage_Run_EX (int hdbc, char sn[], int *runtime, enum enum_LIFE LIFE_TYPE); 

/*******************************************************************/   
/*
函数名称：	MyDLL_FiberManage_End
创建日期：	2010-05-05
创建作者：	Roger.Li
功能描述：	恢复输入的fiber编号的状态 到Active
参数说明：	hdbc [in] 数据库连接句柄 
            *fibersn [int] fiber编号
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLL_FiberManage_End (int hdbc, char *fibersn);
extern int __stdcall DLLIMPORT MyDLL_FiberManage_End_EX (int hdbc, char sn[], enum enum_LIFE LIFE_TYPE); 

/*******************************************************************/   
/*
函数名称：	MyDLLReadModuleSN
创建日期：	2010-05-21
创建作者：	Roger.Li
功能描述：	读取模块的序列号
参数说明：	handle [in] 设备句柄 
            *modulesn [out]   module SN
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLLReadModuleSN (int handle, char *modulesn);

/*******************************************************************/   
/*
函数名称：	MyDLLDeleteSpace
创建日期：	2010-06-18
创建作者：	Roger.Li
功能描述：	将输入字符串中的空格字符全部去掉 
参数说明：	*str [in and out] 输入字符串和转换结果  
返回值说明：0函数运行成功，非0函数运行错误
/*******************************************************************/ 
//MyDLLDeleteSpace已被函数MyDLL_TRIM代替
//extern int __stdcall DLLIMPORT MyDLLDeleteSpace(char *str);

extern int __stdcall DLLIMPORT MyDLLGetProductLine (int *ProductLineID);

/***************************账号管理部分定义***************************************/
#define  ACCOUNT_TYPE_MAX	 7

static ViString AccountTypeArray[] = {
"",
"Operator",
"Firmware",
"Admin",
"Fiber",
"Audit",
"Repair"
};

static ViString AccountTypeArray_chs[] = {
"",
"操作员",
"固件管理员",
"高级管理员",
"光纤管理员",
"审核员",
"维修员"
};
/***************************账号管理部分定义***************************************/ 

/***************************设备管理部分定义***************************************/
#define  INSTRUMENT_TYPE_MAX	 9  

enum enum_INSTR_TYPE
{   
INSTR_TYPE_NONE,
INSTR_TYPE_PM,
INSTR_TYPE_ATT, 
INSTR_TYPE_DCA, 
INSTR_TYPE_BERT, 
INSTR_TYPE_SEVB5, 
INSTR_TYPE_SEVB6,
INSTR_TYPE_SPECTRUM,   
INSTR_TYPE_FIBER
}; 

static ViString InstrumentTypeArray[] = {
"",
"光功率计",
"光衰减器",
"示波器",
"误码仪",
"SEVB5",
"SEVB6",
"光谱仪",   
"光纤"
};

#define  INSTRUMENT_STATUS_MAX	 4 

static ViString InstrumentStatusArray[] = {
"",
"使用",
"空闲",
"维修"
};
/***************************设备管理部分定义***************************************/ 

#define  CUSTOMER_MAX	 20 

static ViString CustomerNameArray[] = {
"",
"UT",
"ZT",
"FH",
"HW",
"OP",
"AT",
"AL",
"RH",
"01",
"DY", 
"SUPERXON",
"ZY",
"FLEX",
"AP",
"DZ",
"AD",
"MC",
"MI",
"DEONET"
};

//产品类型定义
#define  ProductTYPE_MAX	 13 

static ViString ProductTypeArray[] = {
"",
"EPON OLT",
"GPON OLT",
"EPON ONU_P",			//pigtail
"EPON ONU_R", 			//receptacle
"GPON ONT",
"GBE",
"SDH",
"XFP",
"10G OLT",
"10G ONU",
"SFP_PLUS",
"XGPON_ONU"
};
//产品类型定义

/***************************工序部分定义***************************************/
#define  PROCESS_TYPE_MAX	 50

enum enum_PROCESS_TYPE
{   
PROCESS_TYPE_NONE,
PROCESS_TYPE_OSA_APD,
PROCESS_TYPE_OSA_APD_QA, 
PROCESS_TYPE_TRX_Assemble,
PROCESS_TYPE_TRX_ChangeBatch, 
PROCESS_TYPE_TRX_DownFirmware, 
PROCESS_TYPE_TRX_E2Check, 
PROCESS_TYPE_OSA_Fiberlink,
PROCESS_TYPE_TRX_Flunk,   
PROCESS_TYPE_TRX_FQC, 
PROCESS_TYPE_OSA_FQC,
PROCESS_TYPE_OSA_LIV, 
PROCESS_TYPE_OSA_LIV_QA, 
PROCESS_TYPE_OSA_LIV_TC1, 
PROCESS_TYPE_OSA_LIV_TC1_QA, 
PROCESS_TYPE_OSA_LIV_TC2, 
PROCESS_TYPE_OSA_LIV_TC2_QA,
PROCESS_TYPE_TRX_Obsolete,  
PROCESS_TYPE_OSA_RXTrack, 
PROCESS_TYPE_OSA_TXTrack,
PROCESS_TYPE_TRX_Penalty,  
PROCESS_TYPE_TRX_PowerRepeat, 
PROCESS_TYPE_TRX_PowerStability, 
PROCESS_TYPE_TRX_PowerTest,
PROCESS_TYPE_TRX_QA_10GTest,
PROCESS_TYPE_TRX_QA_E2Check,
PROCESS_TYPE_TRX_QA_Testing, 
PROCESS_TYPE_TRX_QA_Testing_H,  
PROCESS_TYPE_TRX_QA_Testing_L,  
PROCESS_TYPE_OSA_ROSA,
PROCESS_TYPE_OSA_ROSA_QA,
PROCESS_TYPE_TRX_RXTesting, 
PROCESS_TYPE_TRX_RXTesting_H,
PROCESS_TYPE_TRX_RXTesting_L,
PROCESS_TYPE_TRX_System,
PROCESS_TYPE_TRX_10GTest,
PROCESS_TYPE_TRX_10GTest_H,
PROCESS_TYPE_TRX_10GTest_L,
PROCESS_TYPE_TRX_Testing,
PROCESS_TYPE_TRX_Testing_H,
PROCESS_TYPE_TRX_Testing_L,
PROCESS_TYPE_TRX_10GTestRx,
PROCESS_TYPE_TRX_TrackingSN,
PROCESS_TYPE_TRX_10GTune,
PROCESS_TYPE_TRX_TuneRX,
PROCESS_TYPE_TRX_Tuning,
PROCESS_TYPE_TRX_Tuning_H,
PROCESS_TYPE_TRX_Tuning_L,
PROCESS_TYPE_TRX_Vapd_Tune,
PROCESS_TYPE_TRX_OQA, 
PROCESS_TYPE_OSA_ChangeBatch,
PROCESS_TYPE_OSA_Reform,
PROCESS_TYPE_testAginFront,
PROCESS_TYPE_testAginBack,
PROCESS_TYPE_TunReliability,
PROCESS_TYPE_TestReliability,
PROCESS_TYPE_TestUpstream  
}; 

static ViString ProcessTypeArray[] = {
"",
"APD_OQC",
"APD_OQC-QA",
"ASSEMBLE",
"CHANGE BATCH",
"DOWNLOAD FIRMWARE",   
"EEPROM CHECK",
"FIBERLINK",
"FLUNK",
"FQC",
"FQC_OSA",   
"LIV",
"LIV-QA",
"LIV_TC1",
"LIV_TC1-QA",
"LIV_TC2",   
"LIV_TC2-QA",
"OBSOLETE",
"OSA RX TRACK",
"OSA TX TRACK",
"PENALTY",   
"POWERREPEAT",
"POWERSTABILITY",
"POWER TEST",
"QATEST_10G",
"QAE2CHECK",
"QATESTING",
"QATESTING_HIGH", 
"QATESTING_LOW", 
"ROSA SENS TEST",   
"ROSA SENS TEST-QA",
"RXTESTING",
"RXTESTING_HIGH", 
"RXTESTING_LOW",
"SYSTEM",
"TEST_10G",
"TEST_10G_HIGH",
"TEST_10G_LOW",
"TESTING",
"TESTING_HIGH",
"TESTING_LOW",
"TESTRX_10G",
"TRACKING SN",   
"TUN_10G",
"TUN_RX",
"TUNING",
"TUNING_HIGH",
"TUNING_LOW",
"VAPD TUNING",
"OQA",
"CHANGE_BATCH_OSA",
"REFORM_OSA",
"AginFront_Test",
"AginBack_Test",
"TUN_Reliability",
"TEST_Reliability", 
"Test_Upstream"
};
/***************************工序部分定义***************************************/ 

extern int __stdcall DLLIMPORT MyDLLGetOAServer (char server[]);  

/*******************************************************************/   
/*
函数名称：	MyDLLGetInnerSN
创建日期：	2011-08-09
创建作者：	Roger.Li
功能描述：	读取模块的内部识别编码，默认读取地址A2 table0【240-255】，
当产品类型是PN_TYPE_GPON_OLT_SFP，先读取A0【96-103】，如果包含信息“ALCATEL”，将A0【240-255】内容作为InnerSN,否则读取A2 table0【240-255】
当产品类型是PN_TYPE_XFP，切换读取地址到A0 table1【240-255】 
默认产品类型录入值可以是-1
参数说明：	handle [in] 设备句柄 
            sn[]   [out] 识别码
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLLGetInnerSN (int handle, enum ePN_TYPE pn_type, char sn[]);

/*
函数名称：	MyDLLGetPCBA
创建日期：	2012-07-06
创建作者：	Roger.Li
功能描述：	读取模块的内部pcba批次，默认读取地址A2 table0【224-239】，
当产品类型是PN_TYPE_XFP或PN_TYPE_10G_EPON_OLT，返回异常，因为目前暂不支持
默认产品类型录入值可以是-1
参数说明：	handle [in] 设备句柄 
            sn[]   [out] 识别码
返回值说明：0函数运行成功，非0函数运行错误，
*/
int __stdcall DLLIMPORT MyDLLGetPCBA (int handle, enum ePN_TYPE pn_type, char pcba[]);

/*******************************************************************/   
/*
函数名称：	MyDLLSetSysTimeFormat
创建日期：	2011-08-18
创建作者：	Roger.Li
功能描述：	修改系统时间格式为yyyy-MM-dd HH:mm:ss   
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
extern int __stdcall DLLIMPORT MyDLLSetSysTimeFormat (void);  

/*******************************************************************/   
/*
函数名称：	MyDLL_8472_GetRxPWR
创建日期：	2011-09-18
创建作者：	Roger.Li
功能描述：	按照8472协议读取A2的收端监控值，已经过校准系数修正  
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/  
int __stdcall DLLIMPORT MyDLL_8472_GetRxPWR (int inst, float *RxPWR); 
int __stdcall DLLIMPORT MyDLL_8472_GetIbias (int inst, float *val);
int __stdcall DLLIMPORT MyDLL_8472_GetTemperatrue (int inst, float *val);
int __stdcall DLLIMPORT MyDLL_8472_GetTemper_NoAck (int inst, float *val);					//读A2温度，不报noack错误，用于检测模块温度是否可以测试，或模块是否在测试板上，访问异常时返回温度999  
int __stdcall DLLIMPORT MyDLL_8472_GetTxPWR (int inst, float *val); 
int __stdcall DLLIMPORT MyDLL_8472_GetVcc (int inst, float *val);
int __stdcall DLLIMPORT MyDLL_8472_GetRxPWR_EED (int inst, float *RxPWR, int EED_Data[]); 	//带入EED文件参数 
int __stdcall DLLIMPORT MyDLL_8472_GetTxPWR_EED (int inst, float *val, int EED_Data[]);		//带入EED文件参数 

/*******************************************************************/   														   
/*
函数名称：	MyDLLGetServerType
创建日期：	2012-03-22
创建作者：	Roger.Li
功能描述：	读取注册表配置，获取生产数据库类型，值分别为sql和oracle，大小写不敏感
[HKEY_CURRENT_USER\Software\superxon]
"server_type"="sql" 
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/ 
enum eServerType
{  
SERVER_SQL,
SERVER_ORACLE
};

int __stdcall DLLIMPORT MyDLLGetServerType (enum eServerType *server_type); 

/*******************************************************************/   
/*
函数名称：	MyDLL_DB_Init
创建日期：	2012-04-18
创建作者：	Roger.Li
功能描述：	数据库链接函数
1、先关闭指定hdbc句柄
2、设置DB_INIT_MULTITHREADED
3、根据数据库类型配置（调用MyDLLGetServerType）创建链接
返回值说明：0函数运行成功，非0函数运行错误，
/*******************************************************************/ 
int __stdcall DLLIMPORT MyDLL_DB_Init (int *hdbc); 

/*******************************************************************/   
/*
函数名称：	MyDLL_DBOA_Init
创建日期：	2012-12-13
创建作者：	Roger.Li
功能描述：	OA数据库链接函数
1、先关闭指定hdbc句柄
2、设置DB_INIT_MULTITHREADED
返回值说明：0 函数运行成功，非0函数运行错误，
            -2 读取注册表的OA数据库IP错误  
/*******************************************************************/ 
int __stdcall DLLIMPORT MyDLL_DBOA_Init (int *hdbc);

#endif   
