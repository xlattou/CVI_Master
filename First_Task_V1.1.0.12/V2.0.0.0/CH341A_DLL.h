//V14.0 //20100106
//注意，如果是VC++调用，则全局变量前面要加“extern”，
//比如“int I2C_position;”，就要变成“extern int I2C_position;”
//同时在主程序的.C文件中定义这些全局变量并赋初值。
//注意，如果是CVI调用，则无需改变，仅需在主程序的.C文件中给这些全局变量赋初值。

#ifndef _CH341A_DLL_H_
#define _CH341A_DLL_H_ 

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall GetDLLVersion_DLL (void);

//get dll file version
void __stdcall GetDLLVersion2_DLL (char *ver); 


int __stdcall F320StreamC2(int iIndex, unsigned char iCommand, int iWriteLength, int iReadLength, unsigned char *ioBuffer);

/*====================================================================================*/
// USB 
/*====================================================================================*/ 
unsigned char OutBuf[300], InBuf[300]; 
int USBStatus;
//int USBiMode;// 位1 位0: I2C 速度/SCL 频率, 00=低速20KHz,01=标准100KHz,10=快速400KHz,11=高速750KHz 
int USBHandle; // host设备序号，代入下面函数的iIndex
/*约定USBHandle
CH341，0x000 <= USBHandle <= 0x0FF; (典型值:0x000，0x001，……) 
LPT，  0x100 <= USBHandle <= 0x3FF; (典型值:0x378，0x278)
F320， 0x800 <= USBHandle <= 0x9FF; (典型值:0x800，0x801，……) 
其中，
F320的0x800偏移量是人为加的，为的是用户编程时区别于CH341的index；
而用户调用DLL时，DLL将把这个0x800的偏移量自动减去。
*/


void __stdcall SetUSBiMode_DLL (int iMode);		  //CH341 I2C速率; I2C接口速度/SCL频率, 00=低速/20KHz,01=标准/100KHz(默认值),10=快速/400KHz,11=高速/750KHz

int __stdcall InitialUSB_DLL (int iIndex);
int __stdcall CloseUSB_DLL (int iIndex);

int __stdcall GetUSBHostDescr(  // 读取设备描述符
				int	iIndex,  // 指定CH341设备序号
				unsigned char *OutBuf);  // 指向一个足够大的缓冲区,用于保存描述符


int __stdcall GetUSBHostConfigDescr(  // 读取配置描述符
				int	iIndex,  // 指定CH341设备序号
				unsigned char *OutBuf);  // 指向一个足够大的缓冲区,用于保存描述符

int __stdcall USBHost_GetStatus_DLL (int iIndex, unsigned int *iStatus);


int __stdcall USBHost_GetInput_DLL (int iIndex, unsigned int *iStatus);
// 位7-位0对应CH341的D7-D0引脚
// 位8对应CH341的ERR#引脚, 位9对应CH341的PEMP引脚, 位10对应CH341的INT#引脚, 位11对应CH341的SLCT引脚, 位23对应CH341的SDA引脚
// 位13对应CH341的BUSY/WAIT#引脚, 位14对应CH341的AUTOFD#/DATAS#引脚,位15对应CH341的SLCTIN#/ADDRS#引脚


int __stdcall USBHost_SetD5D0_DLL (int iIndex, int iSetDirOut, int iSetDataOut);
// 设置D5-D0各引脚的I/O方向,某位清0则对应引脚为输入,某位置1则对应引脚为输出,并口方式下默认值为0x00全部输入
// 设置D5-D0各引脚的输出数据,如果I/O方向为输出,那么某位清0时对应引脚输出低电平,某位置1时对应引脚输出高电平


int __stdcall USBHost_ResetDevice_DLL(int iIndex);


int __stdcall I2C_HOST_INITIALIZATION_DLL (int iIndex);
//没找到host就返回-1，否则返回0

int __stdcall I2C_SLAVE_SEARCH_DLL (int iIndex, int device_addr);
//没找到host就返回-1，否则返回0

int __stdcall I2C_BYTE_CURRENT_ADDRESS_READ_DLL (int iIndex, int device_addr, unsigned char *rom_value);
//单字节立即读

int __stdcall I2C_BYTE_READ_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value);
//单字节指定地址读

int __stdcall I2C_BYTEs_READ_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr);
//多字节指定首地址连续读
//注意，rom_value_arr[256]数组要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据

int __stdcall I2C_BYTE_CURRENT_ADDRESS_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, float T_wait);
//偏移量立即写


int __stdcall I2C_BYTE_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait);
//单字节指定首地址写

int __stdcall I2C_BYTEs_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait);
//多字节指定首地址连续写 
//注意，rom_value_arr[256]要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。
//注意，对于EEPROM，一般地只能8byte连写；而对于7020，一般就没有最大长度的限制。

int __stdcall I2C_2BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2);
//双字节指定首地址读 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1

int __stdcall I2C_4BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3, unsigned char *rom_value4); 
//四字节指定首地址读 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1，rom_value3的地址是rom_startaddress+2，rom_value4的地址是rom_startaddress+3

int __stdcall I2C_2BYTEs_WRITE_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value1, unsigned char rom_value2, float T_wait);
//双字节指定首地址写 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1

int __stdcall I2C_4BYTEs_WRITE_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, float T_wait); 
//四字节指定首地址写 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1，rom_value3的地址是rom_startaddress+2，rom_value4的地址是rom_startaddress+3


/*============F320================================*/
void __stdcall RESET_F320_DLL (int iIndex);
//复位F320
int __stdcall GetF320Numbers(int *lpdwNumDevices); 
//读取当前挂上PC的F320的数量
int __stdcall GET_Version_F320_DLL (int iIndex, unsigned char *FVER);
//读取F320的firmware版本号
int __stdcall GetF320SerialNumber(int iIndex, unsigned char *SN); 
//读取F320的SN[16]
int __stdcall SetF320SerialNumber(int iIndex, unsigned char *SN); 
//设置F320的SN[16]

//传一个F320转I2C的速率给DLL内部的变量F320I2CRate: 0=50KHz, 1=100KHz, 2=150KHz, others: Fi2c=24MHz/3/(256-I2CRate) 
//然后调用函数I2C_HOST_INITIALIZATION_DLL可将I2C速率进行设置
void __stdcall SetF320I2CRate_DLL (int I2CRate);

//传一个I2C读时序的ReSTART之前加一个STOP的标志
//然后调用函数I2C_HOST_INITIALIZATION_DLL可率进行设置
void __stdcall SetF320I2CSTOPbeforeReSTART_DLL (int I2CSTOPbeforeReSTART); 

//复位主I2C，并设置速率F320I2CRate: 0=50KHz, 1=100KHz, 2=150KHz, others: Fi2c=24MHz/3/(256-I2CRate)
//并设置是否在主I2C读时序的ReSTART之前加一个STOP
int __stdcall I2C_HOST_RESET_F320_DLL (int iIndex, int I2CRate, int I2CSTOPbeforeReSTART); 

int __stdcall F320ExecuteCmd_DLL(int iIndex, unsigned char iCommand, int iWriteLength, int iReadLength, unsigned char *ioBuffer);
//执行F320命令

int __stdcall F320_SPI_Init_DLL(int iIndex, int SPI_Rate, int SCK_PHASE, int SCK_POLARITY);
//F320ExecuteCmd(0, SPI_RST, 3, 0, ioArray) 复位并重新设置SPI的速率，相位，极性 
//SPI_Rate：速率，计算公式SCK = 12000/( ioArray[0]+1)K，如ioArray[0] = 119，则SPI速率为100K
//SCK_PHASE：相位，0：在SCK周期的第一个边沿采样数据，1：在SCK周期的第二个边沿采样数据
//SCK_POLARITY：bit0是SCK极性，0：SCK在空闲状态时处于低电平，1：SCK在空闲状态时处于高电平
//              bit7是CS产生方式，0：需要用户调用SetF320_SPI_NSS_DLL()函数更改，1：每次发起SPI操作时自动置低置高
//默认设置为100K，第一个边沿采样数据，SCK在空闲状态时处于低电平

int __stdcall SetF320_SPI_NSS_DLL(int iIndex, int NSS); 
//F320ExecuteCmd(0, SPI_NSS, 1, 0, NSS);设置NSS电平，0：低电平；1：高电平

int __stdcall SetF320_SPI_CS9_CS0_DLL(int iIndex, int CS9_CS0); 
//F320ExecuteCmd(iIndex, SET_CS, 1, 0, CS9_CS0);设置CS9_CS0电平，CS9_CS0的bit9..bit0分别对应GPIO的CS9_CS0电平

int __stdcall SPI_BYTEs_READWRITE_F320_DLL (int iIndex, int iRWLength, unsigned char *iBuffer, unsigned char *oBuffer);
//F320转SPI读写，iRWLength最大约280

int __stdcall SetF320_FPGA_PROGB_DLL(int iIndex, int PROGB);
//设置FPGA_PROGB电平，0：低电平；1：高电平

int __stdcall GET_TM_F320_DLL (int iIndex, unsigned char *TM);
//读取F320点灯LED的时间，TM的1LSB=1s
int __stdcall SET_TM_F320_DLL (int iIndex, unsigned char TM);
//设置F320点灯LED的时间，TM的1LSB=1s

int __stdcall SEL_SPIIIC_F320_DLL (int iIndex, unsigned char SI);
//设置F320输出SEL_SPIIIC管脚的电平，低电平选中SPI，高电平选中I2C

int __stdcall SET_FS_F320_DLL (int iIndex, unsigned char FS);
//针对evb5设置F320输出FS管脚的电平.
//针对evb11设置F320输出FS管脚的电平：
//    该命令后面跟1个数据，数据解释如下：
//    0x00：选择晶振输出频率115.52MHz
//    0x01: 选择晶振输出频率161.13281MHz
//    0x02: 选择晶振输出频率167.33165MHz
//    0x03: 选择晶振输出频率173.37075MHz
//    其他数据无效，保持上一次频率选择结果

int __stdcall SDN_VCC_F320_DLL (int iIndex, unsigned char VCC);
//    该命令后面跟一个数据，数据解释如下：
//    数据的bit0：控制OLT电源的开断，1为打开，0为关断
//    数据的bit1：控制ONU电源的开断，1为打开，0为关断
//    其它bit无效，默认全部开电

int __stdcall SEL_IIC_F320_DLL (int iIndex, unsigned char IIC);
//    该命令后面跟一个数据，数据解释如下：
//    数据的bit0：OLT IIC总线使能，1为使能，0禁止
//    数据的bit1：ONU1 IIC总线使能，1为使能，0禁止
//    数据的bit2：BERT IIC总线使能，1为使能，0禁止
//    数据的bit3：ARM IIC总线使能，1为使能，0禁止
//    其它bit无效，默认全部使能

int __stdcall EPP_BYTEs_WRITE_F320_DLL (int iIndex, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr);
//EPP多字节指定首地址连续写 
//注意，rom_value_arr[256]要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。

int __stdcall EPP_BYTEs_READ_F320_DLL  (int iIndex, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr);
//EPP多字节指定首地址连续读
//注意，rom_value_arr[256]数组要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据


//SEVB0027-4001 时钟信号切换板 通道切换函数
//Channel参数0-7
int __stdcall SEL_CLK_F320_DLL (int iIndex, unsigned char Channel);   
int __stdcall CLK_Get_F320_DLL (int iIndex, unsigned char *Channel);

/*====================================================================================*/
// firmware downloader
/*====================================================================================*/ 
char FirmwareFile[300], EEPROMFile[300];
int RemainEEPROM, WRITEorVERIFY;
unsigned char Data_Array[16*1024*1024/8]; 
unsigned char EEPROM_Array[62*1024]; 

int __stdcall READ_FIRMWARE_DLL(char *hexFile, unsigned char *Data_Array);
int __stdcall ERASE_PAGE0_DLL (int iIndex, int MCU_position);
int __stdcall START_DLL (int iIndex);
int __stdcall RUN_DLL(int iIndex);
int __stdcall ERASE_DLL(int iIndex, int RemainEEPROM);
int __stdcall WRITEorVERIFY_DLL(int iIndex, int RemainEEPROM, int WRITEorVERIFY, unsigned char *Data_Array); 
int __stdcall I2C_MasterWriteSlave_DLL(int iIndex, int my_device_addr);
int __stdcall I2C_MasterReadSlave_DLL (int iIndex, int my_device_addr);
/*====================================================================================*/
// IIC_UART
/*====================================================================================*/ 

int __stdcall IIC_Uart_Open(int iIndex,int rom_Length,unsigned char *rom_value_arr); //打开串口
int __stdcall IIC_Uart_Close(int iIndex,int rom_Length,unsigned char *rom_value_arr); //关闭串口 
int __stdcall IIC_Uart_Read (int iIndex, int imode,int Read_Length, int *BytesReturned, unsigned char *rom_value_arr);

/*====================================================================================*/
// PDI 接口
/*====================================================================================*/ 
int __stdcall   F320_PDI_Get_deviceID_DLL(int iIndex, unsigned char *rom_value_arr);
int __stdcall   F320_PDI_Earse_NVM_Page_DLL(int iIndex,int rom_PageAddress);
int __stdcall   F320_PDI_Earse_NVM_All_DLL(int iIndex);
int __stdcall   F320_PDI_Write_NVM_DLL(int iIndex,int rom_startaddress,int iRWLength, unsigned char *iBuffer);
int __stdcall   F320_PDI_Read_NVM_DLL(int iIndex, int rom_startaddress,int rom_Length, unsigned char *rom_value_arr);	  
int __stdcall   F320_PDI_Earse_EEP_Page_DLL(int iIndex,int rom_PageAddress);  
int __stdcall   F320_PDI_Earse_EEP_All_DLL(int iIndex);  
int __stdcall   F320_PDI_Write_EEP_DLL(int iIndex,int rom_startaddress,int iRWLength, unsigned char *iBuffer);
int __stdcall   F320_PDI_Read_EEP_DLL(int iIndex,int rom_startaddress,int rom_Length, unsigned char *rom_value_arr);

int __stdcall   F320_PDI_Program_BOD_DLL(int iIndex,unsigned char *rom_value_arr);
int __stdcall   F320_PDI_Get_BOD_DLL(int iIndex,unsigned char *rom_value_arr);  


/*====================================================================================*/
// EEPROM 
/*====================================================================================*/ 
#pragma pack (1) /* set maximum alignment to 1 */

typedef signed char         INT8S;               //8bit
typedef unsigned char       INT8U;               //8bit
typedef unsigned char       BYTE;                //8bit 
typedef unsigned short int  INT16U;              //16bit
typedef signed short int    INT16S;              //16bit
typedef unsigned int        INT32U;              //32bit
typedef signed int          INT32S;              //32bit  

/*====================================================================================*/
// EEPROM A0
/*====================================================================================*/ 
struct InfCompCodesDef
{
INT8U bit0:1;			 //1X Copper Passive                      
INT8U bit1:1;			 //1X Copper Active
INT8U bit2:1;		     //1X LX
INT8U bit3:1;			 //1X SX
INT8U bit4:1;			 //10G Base-SR
INT8U bit5:1;			 //10G Base-LR  
INT8U bit6:1;			 //10G Base-LRM  
INT8U bit7:1;			 //Unallocated 
};

struct ESCON_SONETCCDef
{
INT8U bit0:1;			 //OC-48, short reach                      
INT8U bit1:1;			 //OC-48, intermediate reach
INT8U bit2:1;		     //OC-48, long reach
INT8U bit3:1;			 //SONET reach specifier bit1-bit2
INT8U bit4:1;			 //SONET reach specifier
INT8U bit5:1;			 //OC-192, short reach
INT8U bit6:1;			 //ESCON SMF, 1310nm Laser
INT8U bit7:1;			 //ESCON MMF, 1310nm LED
};

struct SONETCCDef
{
INT8U bit0:1;			 //OC-3, short reach                     
INT8U bit1:1;			 //OC-3, single mode, inter. reach
INT8U bit2:1;		     //OC-3, single mode, long reach
INT8U bit3:1;			 //Reserved
INT8U bit4:1;            //OC-12, short reach
INT8U bit5:1;			 //OC-12, single mode, inter. reach
INT8U bit6:1;			 //OC-12, single mode, long reach
INT8U bit7:1;			 //Reserved
};

struct ETHERNETCCDef
{
INT8U bit0:1;			 //1000BASE-SX                  
INT8U bit1:1;			 //1000BASE-LX
INT8U bit2:1;		     //1000BASE-CX
INT8U bit3:1;			 //1000BASE-T
INT8U bit4:1;            //100BASE-LX/LX10
INT8U bit5:1;			 //100BASE-FX
INT8U bit6:1;			 //BASE-BX10
INT8U bit7:1;			 //BASE-PX
};

struct FibreChannelDef
{
INT8U bit0:1;			 //Electrical inter-enclosure (EL)                  
INT8U bit1:1;			 //Longwave laser (LC)
INT8U bit2:1;		     //Reserved
INT8U bit3:1;			 //medium distance (M)
INT8U bit4:1;            //long distance (L)
INT8U bit5:1;			 //intermediate distance (I)
INT8U bit6:1;			 //short distance (S)
INT8U bit7:1;			 //very long distance (V)
};

struct TransTecDef
{
INT8U bit0:1;			 //Reserved   
INT8U bit1:1;			 //Copper FC-BaseT
INT8U bit2:1; 			 //Copper Passive
INT8U bit3:1;		     //Copper Active
INT8U bit4:1;            //Longwave laser (LL)
INT8U bit5:1;			 //Shortwave laser with OFC (SL)
INT8U bit6:1;			 //Shortwave laser w/o OFC (SN)
INT8U bit7:1;			 //Electrical intra-enclosure (EL)
};


struct TransMediaDef
{
INT8U bit0:1;			 //Single Mode (SM)   
INT8U bit1:1;			 //Reserved
INT8U bit2:1; 			 //Multi-Mode, 50um (M5)
INT8U bit3:1;		     //Multi-Mode, 62.5um (M6)
INT8U bit4:1;            //Video Coax (TV)
INT8U bit5:1;			 //Miniature Coax (MI)
INT8U bit6:1;			 //Shielded Twisted Pair (TP)
INT8U bit7:1;			 //Twin Axial Pair (TW)
};


struct FibreChannelSpeedDef
{
INT8U bit0:1;			 //100 MBytes/sec 
INT8U bit1:1;			 //Reserved
INT8U bit2:1; 			 //200 MBytes/sec
INT8U bit3:1;		     //Reserved
INT8U bit4:1;            //400 MBytes/sec
INT8U bit5:1;			 //Reserved
INT8U bit6:1;			 //800 MBytes/sec
INT8U bit7:1;			 //Reserved
};


struct StatCtrlBitsDef
{
INT8U  dataReady:1;
INT8U  LOS:1;
INT8U  TX_Fault:1;
INT8U  softRateSelect:1;
INT8U  rateSelectState:1;
INT8U  reserved:1;
INT8U  softTxDis:1;	
INT8U  txDisableState:1;
};

struct Options64Def
{
INT8U bit0:1;			 //Linear Receiver Output Implemented
INT8U bit1:1;			 //Power Class Declaration
INT8U bit2:1; 			 //Cooled Transceiver Declaration
INT8U bit3_7:5;			 //Reserved 
};


struct Options65Def
{
INT8U bit0:1;			 //Reserved
INT8U bit1:1;			 //Rx_LOS
INT8U bit2:1; 			 //Loss Inverted
INT8U bit3:1;		     //TX_FAULT
INT8U bit4:1;            //TX_DISABLE
INT8U bit5:1;			 //RATE_SELECT
INT8U bit6_7:2;			 //Reserved
};

struct DiagnosticMonitorDef
{
INT8U bit0_1:2;			 //Reserved
INT8U bit2:1; 			 //Address Modes
INT8U bit3:1;		     //Received power type
INT8U bit4:1;            //Externally calibrated
INT8U bit5:1;			 //Internally calibrated
INT8U bit6:1;			 //If Digital diagnostic monitoring, Must be "0"
INT8U bit7:1;			 //If Digital diagnostic monitoring, Must be "1"
};

struct EnhancedOptionsDef
{
INT8U bit0:1;			 //Reserved
INT8U bit1:1;			 //Optional Rate
INT8U bit2:1; 			 //Optional Application
INT8U bit3:1;		     //RATE_SELECT
INT8U bit4:1;            //RX_LOS
INT8U bit5:1;			 //TX_FAULT
INT8U bit6:1;			 //TX_DISABLE
INT8U bit7:1;			 //Optional Alarm/warning flags
};

struct strA0 		  
{
  //BASE ID FIELDS
  
      //Type of serial transceiver 
  INT8U  identifier;	                 //pStrA0[0]
      //Extended identifier of type of serial transceiver
  INT8U  extIdentifier;	                 //pStrA0[1]
      //Code for connector type
  INT8U  connector;		                 //pStrA0[2]
      //Code for electronic compatibility or optical compatibility
  //INT8U  transceiver[8];		         //pStrA0[3~10]
  //Infiniband Compliance Codes
  struct InfCompCodesDef infCompCodes;
  //Part of SONET Compliance Codes
  struct ESCON_SONETCCDef escon_SonetCC;
  //Part of SONET Compliance Codes
  struct SONETCCDef sonetCC;
  //Gigabit Ethernet Compliance Codes
  struct ETHERNETCCDef ethernetCC;
  //Fiber Channel link length & part of transmitter technology
  struct FibreChannelDef fibreChannel;
  //Part of Fiber Channel transmitter technology
  struct TransTecDef transTec;
  //Fiber Channel Transmission media
  struct TransMediaDef transMedia;
  //Fiber Channel speed
  struct FibreChannelSpeedDef fibreChannelSpeed;
  
      //Code for serial encoding algorithm
  INT8U  encoding;		                 //pStrA0[11]
      //Nominal bit rate, units of 100 MBits/sec
  INT8U  nominalBitRate;	             //pStrA0[12] 	
      //Reserved for SFF-8079
  INT8U  rateIdentifier;		         //pStrA0[13]
      //Link length supported for 9/125 ìm fiber, units of km
  INT8U  length9im_km;		             //pStrA0[14]
      //Link length supported for 9/125 ìm fiber, units of 100m
  INT8U  length9im;		             //pStrA0[15]
      //Link length supported for 50/125 ìm fiber, units of 10m
  INT8U  length50im;		             //pStrA0[16]
      //Link length supported for 62.5/125 ìm fiber, units of 10m
  INT8U  length62p5im;	                 //pStrA0[17]
      //Link length supported for copper, units of meters
  INT8U  lengthCopper;	                 //pStrA0[18]
      //Reserved
  INT8U	 lengthOM3;		                 //pStrA0[19]
      //Vendor name
  INT8U  vendorName[16];		             //pStrA0[20~35]
      //Reserved
  INT8U  reserved3;		                 //pStrA0[36]
      //Vendor IEEE company ID
  INT8U  vendorOUI[3];		             //pStrA0[37~39]
      //Part number provided by vendor
  INT8U  vendorPN[16];		             //pStrA0[40~55]
      //Revision level for part number provided by vendor
  INT8U  vendorRev[4];		             //pStrA0[56~59]
      //Laser wavelength
  INT8U laserWaveLength[2];             //pStrA0[60~61]
      //Reserved
  INT8U  reserved4;                      //pStrA0[62]
      //Check code for Base ID Fields
  INT8U  cc_Base;		                 //pStrA0[63]
  
      //EXTENDED ID FIELDS
  
      //Indicates which optional transceiver signals are implemented
  struct Options64Def options64;		//pStrA0[64]
  struct Options65Def options65;		//pStrA0[65]
      //Upper bit rate margin, units of %
  INT8U  maxBiteRate;                    //pStrA0[66]
      //Lower bit rate margin, units of %
  INT8U  minBiteRate;                    //pStrA0[67]
      //Serial number provided by vendor
  INT8U vendorSN[16];                    //pStrA0[68~83]	
      //Vendor’s manufacturing date code
  INT8U dateCode[8];                     //pStrA0[84~91]
      //Indicates which type of diagnostic monitoring is implemented in the transceiver
  struct DiagnosticMonitorDef diagMonitorType; //pStrA0[92]
      //Indicates which optional enhanced features are implemented in the transceiver
  struct EnhancedOptionsDef  enhancedOptions; //pStrA0[93]
      //Indicates which revision of SFF-8472 the transceiver complies with
  INT8U sff_8472Compliance;              //pStrA0[94]
      //Check code for the Extended ID Fields		
  INT8U cc_Ext;                          //pStrA0[95]
  
      //VENDOR SPECIFIC ID FIELDS
  INT8U vendorSpecReserved[32];          //pStrA0[96~127]
  											
      //Reserved for SFF8079
  INT8U sff8079Reserved[128];            //pStrA0[128~255] 
  
};		

union uA0
{ 
  struct strA0 sStrA0;
  INT8U  pStrA0[256];
};

union uA0 A0;                
void __stdcall A0_transform(unsigned char *rom_value_arr, unsigned char *A0_str);  

//////////////////////////////////////////////////////////////
/*A2*/
/////////////////////////////////////////////////////////////////
struct status_110def
{
INT8U  Data_Ready:1;
INT8U  LOS:1;
INT8U  TX_Fault:1;
INT8U  Reserved1:3;
INT8U  Soft_Tx_Dis:1;	
INT8U  Tx_Dis:1;
};

struct FLAG_112DEF
{
INT8U  Tx_P_Low:1;
INT8U  Tx_P_Hgh:1;
INT8U  Tx_Bias_Low:1;
INT8U  Tx_Bias_Hgh:1;
INT8U  vcc_Low:1;
INT8U  vcc_Hgh:1;
INT8U  Temp_Low:1;
INT8U  Temp_Hgh:1;
};

struct FLAG_113DEF
{
INT8U  Reserved:6;
INT8U  Rx_P_Low:1;
INT8U  Rx_P_Hgh:1;
};
 
struct strA2 		  
{
INT8U	reserved8[128];					//pStrA2[219~255]  
INT8U	LUTIndex;						//pStrA2[127] 
INT32U	Password;  						//pStrA2[123~126]   										
INT8U   mode;                      		//pStrA2[122]
INT8U   version;                   		//pStrA2[121]
INT8U   BEN_i;                			//pStrA2[120]
INT8U	BAK_i;                      	//pStrA2[119]      																																	
INT8U	reserved6;                  	//pStrA2[118]      																																	
struct	FLAG_113DEF FLAG_WARN_117;  	//pStrA2[117]                 																														
struct	FLAG_112DEF FLAG_WARN_116;  	//pStrA2[116]       																																
INT8U	reserved5[2];               	//pStrA2[114~115]               																															
struct	FLAG_113DEF FLAG_ALARM_113; 	//pStrA2[113]																															
struct	FLAG_112DEF FLAG_ALARM_112 ;	//pStrA2[112]    	//Optional Alarm and Warning Flag Bits                                       																																							
INT8U	PowerLeveling;              	//pStrA2[111]    	//Reserved for SFF-8079        																						
struct	status_110def status_110;   	//pStrA2[110]    	//Optional Status/Control Bits              																						
INT8U	reserved3[4];               	//pStrA2[106~109]	//Reserved for future definition of digitized analog input                   																																						
INT16U	rx_Power;                   	//pStrA2[104~105]	//Measured RX input power.           																						
INT16U	tx_Power;                   	//pStrA2[102~103]	//Measured TX output power.                                                  																						
INT16U	tx_Bias;                    	//pStrA2[100~101]	//Internally measured TX Bias Current                                        																									
INT16U	vcc;                        	//pStrA2[98~99]  	//Internally measured supply voltage in transceiver                          																																									
INT16S	temperature;                	//pStrA2[96~97]  	//Internally measured module temperature     																																																																				
INT8U	cc_Ext;                     	//pStrA2[95]     	//Check code for the Extended ID Fields           																									
INT8U	reserved2[3];               	//pStrA2[92~94]  	//Reserved                  																								
INT16S	voltageOffset;              	//pStrA2[90~91]  	//Offset of Supply Voltage Linear Calibartion      																																											
INT16U	voltageSlope;               	//pStrA2[88~89]  	//Slope of Supply Voltage Linear Calibartion        																																														
INT16S	tempOffset;                 	//pStrA2[86~87]  	//Offset of Temperature Linear Calibartion                                    																																				
INT16U	tempSlope;                  	//pStrA2[84~85]  	//Slope of Temperature Linear Calibartion                                     																																				
INT16S	tx_PwrOffset;               	//pStrA2[82~83]  	//Offset of Transmitter Coupled Output Power Linear Calibartion               																																						
INT16U	tx_PwrSlope;                	//pStrA2[80~81]  	//Slope of Transmitter Coupled Output Power Linear Calibartion                																																						
INT16S	tx_IOffset;                 	//pStrA2[78~79]  	//Offset of Laser Bias Current Linear Calibartion                             																																						
INT16U	tx_ISlope;                  	//pStrA2[76~77]  	//Slope of Laser Bias Current Linear Calibartion           																																													
float	rx_PWR0;                    	//pStrA2[72~75]  	//Polynomial Fit Coefficient of Order 0 for Rx Optical Power Calibartion.     																 
float	rx_PWR1;                    	//pStrA2[68~71]  	//Polynomial Fit Coefficient of Order 1 for Rx Optical Power Calibartion.																																							
float	rx_PWR2;                    	//pStrA2[64~67]  	//Polynomial Fit Coefficient of Order 2 for Rx Optical Power Calibartion.     																																									
float	rx_PWR3;                    	//pStrA2[60~63]  	//Polynomial Fit Coefficient of Order 3 for Rx Optical Power Calibartion.																																											
float	rx_PWR4;	                	//pStrA2[56~59]  	//Polynomial Fit Coefficient of Order 4 for Rx Optical Power Calibartion     																																																													
INT8U	reserved1[16];		        	//pStrA2[40~55]  	//Reserved for future monitored quantities 																							
INT16U	rxPowerLowWarning;           	//pStrA2[38~39]		//Reserved         																														
INT16U	rxPowerHighWarning;	         	//pStrA2[36~37]		//RX Power High Warning  																												
INT16U	rxPowerLowAlarm;	         	//pStrA2[34~35]		//RX Power Low Alarm  																											
INT16U	rxPowerHighAlarm;	         	//pStrA2[32~33]		//RX Power High Alarm 																											
INT16U	txPowerLowWarning;	         	//pStrA2[30~31]		//TX Power Low Warning   																									
INT16U	txPowerHighWarning;	         	//pStrA2[28~29]		//TX Power High Warning   																												
INT16U	txPowerLowAlarm;	         	//pStrA2[26~27]		//TX Power Low Alarm     																													
INT16U	txPowerHighAlarm;	         	//pStrA2[24~25]		//TX Power High Alarm   																														
INT16U	biasLowWarning;	             	//pStrA2[22~23]		//Bias Low Warning     																									
INT16U	biasHighWarning;	         	//pStrA2[20~21]		//Bias High Warning   																													
INT16U	biasLowAlarm;	             	//pStrA2[18~19]		//Bias Low Alarm        																										
INT16U	biasHighAlarm;	             	//pStrA2[16~17]		//Bias High Alarm       																										
INT16U	voltageLowWarning;	         	//pStrA2[14~15]		//Voltage Low Warning   																									
INT16U	voltageHighWarning;	         	//pStrA2[12~13]		//Voltage High Warning   																									
INT16U	voltageLowAlarm;	         	//pStrA2[10~11]		//Voltage Low Alarm 				
INT16U	voltageHighAlarm;	         	//pStrA2[8~9]  		//Voltage High Alarm  							
INT16S	tempLowWarning;              	//pStrA2[6~7]  		//Temperature Low Warning  																											
INT16S	tempHighWarning;             	//pStrA2[4~5] 		//Temperature High Warning  																																																				
INT16S	tempLowAlarm;	             	//pStrA2[2~3] 		//Temperature Low Alarm  																																																			
INT16S	tempHighAlarm;	             	//pStrA2[0~1] 		//Temperature High Alarm 																													 
}; 

union uA2
{ 
  struct strA2 sStrA2;
  INT8U  pStrA2[256];
};

union uA2 A2;

#ifdef __cplusplus
}
#endif

#endif  
