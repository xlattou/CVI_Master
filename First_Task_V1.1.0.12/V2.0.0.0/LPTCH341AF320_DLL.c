/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/
#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h> 
#include "CH341A_DLL.h"   
#include "USB_I2C_DEVICE.h"
#include "LPT_I2C_DEVICE.h"
#include "F320_I2C_DEVICE.h"
#include "SiUSBXp.h" 		//F320的头文件

#define MAX_BYTENUMBER 62*1024 		//最多处理62k byte， Limited by ADuC70xxBCPZxxI
#define MAX_ROWNUMBER MAX_BYTENUMBER/16  	//62k byte 除以 每行16byte 所得的最大行数=3968
#define MAX_ONEPAGEDATANUMBER 248  	//每页包含的Byte个数 =248
#define MAX_PAGENUMBER MAX_BYTENUMBER/MAX_ONEPAGEDATANUMBER //62k byte 除以 每页包含的Byte个数 所得的最大页数 =256

int DLLVersion = 16;         //20131226
#define SOFTVER  "1.1.0.2"   //20131226  

int USBiMode=1; 	//CH341 I2C速率; I2C接口速度/SCL频率, 00=低速/20KHz,01=标准/100KHz(默认值),10=快速/400KHz,11=高速/750KHz
int F320I2CRate=1; 	//F320 I2C速率; 2CRate: 0=50KHz, 1=100KHz, 2=150KHz, others: Fi2c=24MHz/3/(256-I2CRate) 
int F320I2CSTOPbeforeReSTART=0; //=0，在主I2C读时序的ReSTART之前不加STOP；=1，在主I2C读时序的ReSTART之前加一个STOP
/*---------------------------------------------------------------------------*/
/* This is the DLL's main entry-point.  When the OS loads or unloads this    */
/* DLL, it will call this function with the apropriate event.                */ 
/*---------------------------------------------------------------------------*/
int __stdcall DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
        {
        case DLL_PROCESS_ATTACH:
            
            /* Respond to DLL loading by initializing the RTE */
            if (InitCVIRTE (hinstDLL, 0, 0) == 0) 
                return 0;
            break;
        case DLL_PROCESS_DETACH:
            
            /* Respond to DLL unloading by closing the RTE for its use */
            if (!CVIRTEHasBeenDetached ())
                CloseCVIRTE (); 
            break;
        }
    
    /* Return 1 to indicate successful initialization */
    return 1;
}

/*---------------------------------------------------------------------------*/
/* This sample exported function will be available to callers of our DLL.    */
/*---------------------------------------------------------------------------*/
void __stdcall DLLFunc1 (void)
{
    MessagePopup ("Inside DLLFunc1", "Hi!");
}

int __stdcall GetDLLVersion_DLL (void)
{
	return DLLVersion;
}

void __stdcall GetDLLVersion2_DLL (char *ver)
{
	strcpy (ver, SOFTVER);
	
	return ;
}

/*---------------------------------------------------------------------------*/
/* This sample exported function will be available to callers of our DLL.    */
/*---------------------------------------------------------------------------*/

/******************************************************************************************************/


int __stdcall I2C_HOST_INITIALIZATION_DLL (int iIndex)
//没找到host就返回-1，否则返回0
//如果USB转I2C电路板，则I2C_position=2; 
//unsigned int my_iIndex   
// 指定CH341 USB设备序号
//unsigned int my_iMode	
// 指定CH341模式,见下行
// 位1 位0: I2C 速度/SCL 频率, 00=低速20KHz,01=标准100KHz,10=快速400KHz,11=高速750KHz
// 位2: SPI 的I/O 数/IO 引脚, 0=单入单出(4 线接口),1=双入双出(5 线接口)
// 其它保留,必须为0
{ 
	int error;
	void *hUSBDevice;

	if (iIndex<0) return -1;//none
	
	//CH341A
	if ( (iIndex>=0) && (iIndex<=0xFF) )	
	{	error = InitialUSB(iIndex, USBiMode); //打开USB句柄
		if (error == 0)//成功
		{   CloseUSB(iIndex); //关闭USB句柄 
		//	return 0;			//陈涛删掉这里，需要确认是否有问题 Eric.Yao 20140603
		}
		else   return -1;
	}
	
	//LPT
	if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
	{	error = InitialLPT (iIndex);
		if (error) //失败
		{
			return -1;
		}
	}
	
	//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //人为减去0x800这个偏移量
		USBStatus = SI_Open(iIndex, &hUSBDevice); //打开USB句柄 
		if (USBStatus==SI_SUCCESS) //成功
	  	{	
			SI_Close(hUSBDevice);  //关闭USB句柄 
	  		I2C_HOST_RESET_F320_DLL(iIndex, F320I2CRate, F320I2CSTOPbeforeReSTART); //复位F320的I2C Host
			
	  		Delay(0.1);//等待复位重启
	  	}			
		else  return -1; 
	}

	//成功
	return 0;
}

int __stdcall I2C_SLAVE_SEARCH_DLL (int iIndex, int device_addr)
//没找到slave就返回-1，否则返回0 
{	int error;

  		if (iIndex<0) return -1;
		
		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_SLAVE_SEARCH_USB(iIndex, USBiMode, device_addr); //打开USB句柄
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_SLAVE_SEARCH_LPT (iIndex, device_addr);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_SLAVE_SEARCH_F320 (iIndex, device_addr);
			return error; 
		}
		
		return 0;
}

int __stdcall I2C_BYTE_CURRENT_ADDRESS_READ_DLL (int iIndex, int device_addr, unsigned char *rom_value)
//单字节立即读
{ int error;
		
		if (iIndex<0) return -1;

		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_BYTE_CURRENT_ADDRESS_READ_USB(iIndex, USBiMode, device_addr, rom_value); 
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_BYTE_CURRENT_ADDRESS_READ_LPT (iIndex, device_addr, rom_value);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_BYTE_CURRENT_ADDRESS_READ_F320 (iIndex, device_addr, rom_value);
			return error; 
		}
		
		return 0;
}

int __stdcall I2C_BYTE_CURRENT_ADDRESS_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, float T_wait)
{ int error;
		
		if (iIndex<0) return -1;

		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	//error = I2C_BYTE_CURRENT_ADDRESS_READ_USB(iIndex, USBiMode, device_addr, rom_value); 
			return -1; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	//error = I2C_BYTE_CURRENT_ADDRESS_READ_LPT (iIndex, device_addr, rom_value);
			return -1; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_BYTE_CURRENT_ADDRESS_WRITE_F320 (iIndex, device_addr, rom_startaddress, T_wait);
			return error; 
		}
		
		return 0;
}

int __stdcall I2C_BYTE_READ_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value)
//单字节指定地址读
{ int error;
		
		if (iIndex<0) return -1;

		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_BYTE_READ_USB(iIndex, USBiMode, device_addr, rom_startaddress, rom_value); 
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_BYTE_READ_LPT (iIndex, device_addr, rom_startaddress, rom_value);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_BYTE_READ_F320 (iIndex, device_addr, rom_startaddress, rom_value);
			return error; 
		}

		return 0;

}

int __stdcall I2C_BYTEs_READ_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
//多字节指定首地址连续读
//注意，rom_value_arr[]数组要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据
{ 		
		int error;
		if (iIndex<0) return -1;
		
		//CH341
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_BYTEs_READ_USB(iIndex, USBiMode, device_addr, rom_startaddress, rom_Length, rom_value_arr); 
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_BYTEs_READ_LPT (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_BYTEs_READ_F320 (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr);
			return error; 
		}
		
		return 0;

}

int __stdcall I2C_BYTE_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait)
//单字节指定地址写
{ int error;
  unsigned char rom_value_arr[256];
  
		if (iIndex<0) return -1;
		
		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_BYTE_WRITE_USB(iIndex, USBiMode, device_addr, rom_startaddress, rom_value, T_wait); 
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_BYTE_WRITE_LPT (iIndex, device_addr, rom_startaddress, rom_value, T_wait);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
			error = I2C_BYTE_WRITE_F320 (iIndex, device_addr, rom_startaddress, rom_value, T_wait);
			return error; 
		}
		
		return 0;

}

int __stdcall I2C_BYTEs_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait)
//多字节指定首地址连续写 
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。
//注意，对于EEPROM，一般地只能8byte连写；而对于7020，一般就没有最大长度的限制。
{ int error;

  
		if (iIndex<0) return -1;
		
		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_BYTEs_WRITE_USB(iIndex, USBiMode, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait); 
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_BYTEs_WRITE_LPT (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //人为减去0x800这个偏移量
		{	error = I2C_BYTEs_WRITE_F320 (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			return error; 
		}
		}

		return 0;

}

int __stdcall I2C_2BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2)
//双字节指定首地址读 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1
{ int error;
  unsigned char rom_value_arr[256];
  			if (iIndex<0) return -1;
			error = I2C_BYTEs_READ_DLL (iIndex, device_addr, rom_startaddress, 2, rom_value_arr);

			*rom_value1 = rom_value_arr[rom_startaddress]; 
			*rom_value2 = rom_value_arr[rom_startaddress+1]; 
			
			return error;
}

int __stdcall I2C_4BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3, unsigned char *rom_value4)
//四字节指定首地址读 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1，rom_value3的地址是rom_startaddress+2，rom_value4的地址是rom_startaddress+3
{ int error;
  unsigned char rom_value_arr[256];
  		if (iIndex<0) return -1;
		error = I2C_BYTEs_READ_DLL (iIndex, device_addr, rom_startaddress, 4, rom_value_arr);

		*rom_value1 = rom_value_arr[rom_startaddress]; 
		*rom_value2 = rom_value_arr[rom_startaddress+1]; 
		*rom_value3 = rom_value_arr[rom_startaddress+2]; 
		*rom_value4 = rom_value_arr[rom_startaddress+3]; 
			
			return error;
}

int __stdcall I2C_2BYTEs_WRITE_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value1, unsigned char rom_value2, float T_wait)
//双字节指定首地址写 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1
{ int error;
  unsigned char rom_value_arr[256];
  		if (iIndex<0) return -1;
		rom_value_arr[rom_startaddress]   = rom_value1;
		rom_value_arr[rom_startaddress+1] = rom_value2;
		error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, 2, rom_value_arr, T_wait);

	return error;
}

int __stdcall I2C_4BYTEs_WRITE_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, float T_wait)
//四字节指定首地址写 
//注意， rom_value1的地址是rom_startaddress，rom_value2的地址是rom_startaddress+1，rom_value3的地址是rom_startaddress+2，rom_value4的地址是rom_startaddress+3
{ int error;
  unsigned char rom_value_arr[256];
  		if (iIndex<0) return -1;
		rom_value_arr[rom_startaddress]   = rom_value1;
		rom_value_arr[rom_startaddress+1] = rom_value2;
		rom_value_arr[rom_startaddress+2] = rom_value3;
		rom_value_arr[rom_startaddress+3] = rom_value4;
		error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, 4, rom_value_arr, T_wait);

	return error;
}

/*====================================================================================*/
/*====================================================================================*/ 
/*====================================================================================*/ 
/*====================================================================================*/ 
  
int __stdcall READ_FIRMWARE_DLL(char *hexFile, unsigned char *Data_Array)
{  int error;
   static int g_exeHandle;  
   char exeFile[MAX_PATHNAME_LEN];
   char str[255]="NotePad.exe";
   char Driver_name[MAX_PATHNAME_LEN], Directory_name[MAX_PATHNAME_LEN], File_name[MAX_PATHNAME_LEN];
   int  stat, Driver_number;

   int rom_addr, rom_Length;
   unsigned char rom_value, rom_value_arr[256], device_addr, rom_startaddress;
   char rom_value_str[50],title_str[50];
   int temp_int0, temp_int1, temp_int2, temp_int3, temp_int4, temp_int5, temp_int6;
   char ch;  
   float T_wait;//100ms, wait for 50ms+39ms flash time
   
   FILE *fp;
   unsigned int LL, AAAA, TT, DD, CC;
   unsigned int extended_linear_address=0, extended_segment_address=0, absolute_address=0;
   unsigned char DD_byte[16], checksum, checksum_error;
   int i, temp_int, EffectiveRowNumber, CurrentPage;
   char RowLabel[16];
   //unsigned char address_mode=0;//0:no extend address; 4: extend linear address active; 2: extend segment address active
   unsigned char BeCompatibleWithADuCHEXFormat;//0:not compatible with ADuC HEX format; 1: compatible;
   unsigned int Roger_address1, Roger_address0;

																

		//转换hex文件到Data_Array[]=============================================================
			//打开hex文件
			DisableBreakOnLibraryErrors (); 
			fp=fopen(hexFile,"rt");
			EnableBreakOnLibraryErrors ();
			if (fp==NULL) 
			{	return -3;  }
		
			absolute_address=0;
			EffectiveRowNumber=0;//数据在hex文件中共占多少行
			checksum_error=0;	 //如果当前行的checksum不符，则累加1
			for (i=0; i<MAX_BYTENUMBER; i++) Data_Array[i]=0xff; //初始化62k数组数据
			
			//判断当前文件是否是ADuC兼容格式的HEX文件，判断标准是看文件第一行是否是扩展地址。
			//51核的hex第一行就不是扩展地址。当然这个判断标准不是很科学
			if (!feof (fp)) //如果文件尚未读完
			{	fscanf(fp,":%2X%4X%2X",&LL,&AAAA,&TT); //读取每行固有的“行头”“:LLAAAATT”
				//BeCompatibleWithADuCHEXFormat = ((TT == 4) || (TT == 2));
				BeCompatibleWithADuCHEXFormat = ((TT == 4) || (TT == 2) || (TT == 5));
				fseek (fp, 0, SEEK_SET); //回到文件首
			}

			extended_linear_address=0;
			extended_segment_address=0;
			while(!feof (fp) && BeCompatibleWithADuCHEXFormat && (EffectiveRowNumber<MAX_ROWNUMBER)) 
			//如果文件尚未读完，并且是ADuC兼容格式的HEX文件,并且有效数据的行数未超过MAX_ROW
			{	fscanf(fp,":%2X%4X%2X",&LL,&AAAA,&TT); //读取每行固有的“行头”“:LLAAAATT”
				
				//根据TT判断后续数据性质
				switch (TT)
				{
				case 0: //data record
					//获取绝对地址，但注意到最大62k，所以低16bit就够了，根本用不到高16bit的地址值。
					absolute_address = extended_linear_address + extended_segment_address + AAAA;
					//获取该行LL个数据，计算checksum。注意此时Data_Array的序号是 i+(absolute_address&0xffff)
					checksum = LL + (AAAA>>8) + (AAAA & 0xff) + TT;
					for (i=0; i<LL; i++) //读取LL个数据
					{	fscanf(fp,"%2X", &temp_int);
						Data_Array[i+(absolute_address&0xffff)]=temp_int;//给表格数据赋值
						checksum = checksum + temp_int;
					}
					checksum = 0x01 + ~checksum; 
					fscanf(fp,"%2X\n",&CC); //读取CC和回车符，文件指针指向下一行
					//校验数据
					if (checksum != CC) checksum_error++; 					
					//行数累计1
					EffectiveRowNumber++;
				break;

				case 1: //end-of-file record
					fscanf(fp,"%2X\n",&CC); //读取CC和回车符，文件指针指向下一行
				break;
				
				case 2: //extended segment address record
					fscanf(fp,"%4X",&extended_segment_address);
					fscanf(fp,"%2X\n",&CC); //读取CC和回车符，文件指针指向下一行
					extended_segment_address = extended_segment_address <<4;
				break;

				case 3: //Start Segment Address Record, ignored
					//直接读到回车换行
					ch=fgetc(fp); 
					while ((ch!=EOF) && (ch!='\n')) {ch = fgetc(fp);}
				break;
				
				case 4: //extended linear address record
					fscanf(fp,"%4X\n",&extended_linear_address);
					fscanf(fp,"%2X\n",&CC); //读取CC和回车符，文件指针指向下一行
					extended_linear_address = extended_linear_address <<16;
				break;

				case 5: //Start Linear Address Record, ignored
					//直接读到回车换行
					ch=fgetc(fp); 
					while ((ch!=EOF) && (ch!='\n')) {ch = fgetc(fp);}
				break;
				}//end of switch (TT)
		 	}//end of while

			//关闭hex文件
			error=fclose(fp); 	
			if (error) 
			{	return -4; }

			//分析文件
			//SetCtrlVal (panel, TOPPANEL_NUM_ROW_CHECKSUM_ERR, checksum_error);
			//SetCtrlVal (panel, TOPPANEL_NUM_EXTLINADD, extended_linear_address); 
			//SetCtrlVal (panel, TOPPANEL_NUM_EXTSEGADD, extended_segment_address); 
			if ((checksum_error==0) && (EffectiveRowNumber<=MAX_ROWNUMBER) && (EffectiveRowNumber>0) && BeCompatibleWithADuCHEXFormat)
			{	return 0;  }
			else
			{	
				if (BeCompatibleWithADuCHEXFormat==0) 
				{	return -5;  }
				else  //注意如果(BeCompatibleWithADuCHEXFormat==0)那么就不会读文件和分析文件，也就不会出现checksum出错和行数超大的错误
				{
					if (checksum_error>0) 
					{	return -6;  }
					if (EffectiveRowNumber>MAX_ROWNUMBER) 
					{	return -7;  }
					if (EffectiveRowNumber==0) 
					{	return -8;  }
				} //end of else
			} //end of else

	  return 0;  
}

int __stdcall ERASE_PAGE0_DLL (int iIndex, int MCU_position)
{int error;
 char str[256];
 unsigned char Firmware_version;
 float T_wait=0.0; //无需等待
 int error_counter=0;
 int PN;
 unsigned char rom_value, rom_value_arr[256], device_addr, rom_startaddress;
 

		switch (MCU_position)
		{
			case 0: //Module
				error_counter=0;
			  	T_wait = 0.100; //100ms
			  	device_addr = 0xA2;  
				rom_startaddress = 123;
				rom_value_arr[rom_startaddress  ] = 'G';
				rom_value_arr[rom_startaddress+1] = 'L';
				rom_value_arr[rom_startaddress+2] = 'X';
				rom_value_arr[rom_startaddress+3] = 'Y';
				error = I2C_BYTEs_WRITE_DLL(iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//写密码解放B2
				if (error<0) 
				{  	return -1;  }
				else
				{   T_wait = 1.0; //1s
					device_addr = 0xA4; 
					error = I2C_BYTE_WRITE_DLL (iIndex, device_addr, 0, 'E', T_wait); //set flag to tell MCU to erase page0
					if (error<0) 
					{  	return -2;  }
				}
			break;

			case 1: //EVB 
				error_counter=0;
			  	T_wait = 0.100; //100ms
			  	device_addr = 0xC2;  
				rom_startaddress = 123;
				rom_value_arr[rom_startaddress  ] = 'G';
				rom_value_arr[rom_startaddress+1] = 'L';
				rom_value_arr[rom_startaddress+2] = 'X';
				rom_value_arr[rom_startaddress+3] = 'Y';
				error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//写密码解放B2
				if (error<0) 
				{  	return -1;  }
				else
				{   T_wait = 1.0; //1s
					device_addr = 0xC4; //EVB is 0xC4!!!  
					error = I2C_BYTE_WRITE_DLL (iIndex, device_addr, 0, 'E', T_wait); //set flag to tell MCU to erase page0
					if (error<0) 
					{  	return -2;  }
				}
			break;

			case 2: //MAB
				error_counter=0;
			  	T_wait = 0.100; //100ms
			  	device_addr = 0xC2;  
				rom_startaddress = 123;
				rom_value_arr[rom_startaddress  ] = 'G';
				rom_value_arr[rom_startaddress+1] = 'L';
				rom_value_arr[rom_startaddress+2] = 'X';
				rom_value_arr[rom_startaddress+3] = 'Y';
				error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//写密码解放B2
				if (error<0) 
				{  	return -1;  }
				else
				{   T_wait = 1.0; //1s
					device_addr = 0xC6; //SET is 0xC6!!!
					error = I2C_BYTE_WRITE_DLL (iIndex, device_addr, 0, 'E', T_wait); //set flag to tell MCU to erase page0
					if (error<0) 
					{  	return -2;  }
				}
			break;
		}
	return 0;	
}
		
int __stdcall START_DLL (int iIndex)
{unsigned char device_addr, rom_startaddress=0x04, rom_Length;
 char rom_value_arr[256], str[256]="";
 int error, rom_addr, i;
 
			device_addr=0x04;
			rom_startaddress=0x08;
			rom_Length=24;
			error = I2C_BYTEs_READ_DLL (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr); 
			if (error<0)
			{	return -1;  }
			
			for (i=0; i<rom_Length; i++)  str[i]=rom_value_arr[i+rom_startaddress];
			str[rom_Length-1]=0;
			
			if (str[0] != 'A')  //2006-04-01
			{	return -2;  }
			
	return 0;
}
		
int __stdcall RUN_DLL(int iIndex)
{
 int error, i;
 unsigned char Start_ID[2], NoOfDataBytes, Data[300]={0}, Checksum, j;
 int ackerror=0;//equal to 0 means i2c operation normally
 double T_wait=0.1; //Twr=100ms 

 int rom_addr, rom_Length;
 unsigned char rom_value, rom_value_arr[256], device_addr, rom_startaddress;

 			for (i=0; i<256; i++) Data[i]=0;

			device_addr = 0x04;
			Start_ID[0] = 0x07;
			Start_ID[1] = 0x0E;
			NoOfDataBytes = 0x05;
			Data[1] = 'R';  //CMD: "R"
			Data[2] = 0x00; //Address: h
			Data[3] = 0x08; //Address: u
			Data[4] = 0x00; //Address: m
			Data[5] = 0x01; //Address: l 
			//a RUN from address 0x80001 will run code after performing a software reset, 
			//setting back all register to their default value.
			Checksum = NoOfDataBytes;
			for (i=1; i<=NoOfDataBytes; i++) Checksum+=Data[i];
			Checksum = 0x00 - Checksum;
			
			device_addr = 0x04;
			rom_startaddress = Start_ID[0];
			rom_value_arr[rom_startaddress] = Start_ID[1];
			rom_value_arr[rom_startaddress+1] = NoOfDataBytes;

			for (i=1; i<=NoOfDataBytes; i++)
			{	rom_value_arr[rom_startaddress+1+i] = Data[i];  }
			rom_value_arr[rom_startaddress+1+NoOfDataBytes+1] = Checksum;
			rom_Length = 1 + 1 + NoOfDataBytes + 1; // Start_ID[1] + NoOfDataBytes + Bytes + Checksum

			error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			if (error) return -1;

			//对702x返回的0x06的响应必需读回，不然下一次写不进
			error = I2C_BYTE_CURRENT_ADDRESS_READ_DLL (iIndex, device_addr, &rom_value);
			if (error || (rom_value != 0x06)) return -2;
			return 0; 
}


int __stdcall ERASE_DLL(int iIndex, int RemainEEPROM)
{
 int error, i;
 unsigned char Start_ID[2], NoOfDataBytes, Data[300]={0}, Checksum, j;
 int ackerror=0;//equal to 0 means i2c operation normally
 double T_wait=0.1; //Twr=100ms 

 int rom_addr, rom_Length;
 unsigned char rom_value, rom_value_arr[256], device_addr, rom_startaddress;

			for (i=0; i<256; i++) Data[i]=0;
			
			device_addr = 0x04;
			Start_ID[0] = 0x07;
			Start_ID[1] = 0x0E;
			NoOfDataBytes = 5 + 1;
			Data[1] = 'E';  //CMD: "E"
			Data[2] = 0x00; //Address: h
			Data[3] = 0x00; //Address: u
			Data[4] = 0x00; //Address: m
			Data[5] = 0x00; //Address: l

			//如果要保留0x8A000-0x8F7FF区域的EEPROM数据，仅刷0x80000-0x89FFF，共80*512 Byte；
			//如果仅更新0x80000-0x83FFF区域共16K的代码区，共32*512Byte即共32 pages；
			//如果仅更新0x80000-0x89FFF区域共40K的代码区，共80*512Byte即共80 pages；
			if (RemainEEPROM)	
				//Data[6] = 32; 	//32 pages
				Data[6] = 80; 	//80 pages
			else
				Data[6] = 0x00; //all pages			
			Checksum = NoOfDataBytes;							 
			for (i=1; i<=NoOfDataBytes; i++) Checksum= Checksum + Data[i];
			Checksum = 0x00 - Checksum;
			
			device_addr = 0x04;
			rom_startaddress = Start_ID[0];
			rom_value_arr[rom_startaddress] = Start_ID[1];
			rom_value_arr[rom_startaddress+1] = NoOfDataBytes;
			for (i=1; i<=NoOfDataBytes; i++)
			{	rom_value_arr[rom_startaddress+1+i] = Data[i];  }
			rom_value_arr[rom_startaddress+1+NoOfDataBytes+1] = Checksum;
			rom_Length = 1 + 1 + NoOfDataBytes + 1; // Start_ID[1] + NoOfDataBytes + Bytes + Checksum

			error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			if (error) return -1;

			//waiting 3.5 seconds for 7020 erase
			Delay(4.0); 			
			
			//对702x返回的0x06的响应必需读回，不然下一次写不进
			error = I2C_BYTE_CURRENT_ADDRESS_READ_DLL (iIndex, device_addr, &rom_value);
			if (error || (rom_value != 0x06)) return -2;

			return 0; 
}


int my_OnePage_WRITEorVERIFY(int iIndex, unsigned char WRITEorVERIFY, int CurrentPageNumner, unsigned char *Data_Array) //WRITEorVERIFY=1时表示当前按下的键是“Verify Flash”键 
{  
 unsigned char Start_ID[2], Data[300]={0}, Checksum;
 int error, i;
 int CurrentDataNumber; 
 double T_wait=0.0; //无需等待
 int rom_addr, rom_Length;
 unsigned char rom_value, rom_value_arr[300];
 int device_addr, rom_startaddress, NoOfDataBytes;

		//if (Page_Array[CurrentPageNumner]==1) //当该页的有效标志为1才写
		{   //SetCtrlVal (panel, TOPPANEL_TEXTBOX, "."); ProcessDrawEvents ();
		
		    device_addr = 0x04;
			Start_ID[0] = 0x07;
			Start_ID[1] = 0x0E;
			NoOfDataBytes = 5 + MAX_ONEPAGEDATANUMBER;//每页248个数据，加上CMD和地址有5个byte
			
			if (WRITEorVERIFY==0)//当前按下的键是“Write Flash”键；
				Data[1] = 'W';  //CMD: "W"
			else				 //当前按下的键是“Verify Flash”键；数据8bit分高3低5bit倒序
				Data[1] = 'V';  //CMD: "V"
			Data[2] = 0x00; //Address: h
			Data[3] = 0x00; //Address: u
			Data[4] = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER) / 256; //Address: m
			Data[5] = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER) % 256; //Address: l
			for (i=0; i<MAX_ONEPAGEDATANUMBER; i++) 
			{	CurrentDataNumber = CurrentPageNumner * MAX_ONEPAGEDATANUMBER + i;
				if (WRITEorVERIFY==0)//当前按下的键是“Write Flash”键；
					Data[6+i] = Data_Array[CurrentDataNumber];
				else				 //当前按下的键是“Verify Flash”键；数据8bit分高3低5bit倒序
					Data[6+i] = ((Data_Array[CurrentDataNumber] & 0x1f) << 3) + (Data_Array[CurrentDataNumber] >> 5);
			}
			Checksum = NoOfDataBytes;
			for (i=1; i<=NoOfDataBytes; i++) Checksum+=Data[i];
			Checksum = 0x00 - Checksum;
 
			 
			device_addr = 0x04;
			rom_startaddress = Start_ID[0];
			rom_value_arr[rom_startaddress] = Start_ID[1];
			rom_value_arr[rom_startaddress+1] = NoOfDataBytes;
			for (i=1; i<=NoOfDataBytes; i++)
			{	rom_value_arr[rom_startaddress+1+i] = Data[i];  }
			rom_value_arr[rom_startaddress+1+NoOfDataBytes+1] = Checksum;
			rom_Length = 1 + 1 + NoOfDataBytes + 1; // Start_ID[1] + NoOfDataBytes + Bytes + Checksum

			error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			if (error) return -1;
 			if (iIndex>=0x800) //这些Delay是必要的，否则要出错。只有并口操作本来慢才不用延时的
 				Delay(0.02);
 			if (iIndex<=0x0ff)
 				Delay(0.01);
			//对702x返回的0x06的响应必需读回，不然下一次写不进
			error = I2C_BYTE_CURRENT_ADDRESS_READ_DLL (iIndex, device_addr, &rom_value);
			if (error || (rom_value != 0x06)) return -2;
 			if (iIndex>=0x800)
 				Delay(0.01);
			
		}//end of "if (Page_Array[CurrentPageNumner]==1)"	
	  return 0;
}

int __stdcall WRITEorVERIFY_DLL(int iIndex, int RemainEEPROM, int WRITEorVERIFY, unsigned char *Data_Array) //WRITEorVERIFY=1时表示当前按下的键是“Verify Flash”键
{int CurrentPageNumner, CurrentDataNumber;
 unsigned char device_addr, rom_startaddress, rom_Length;
 unsigned char rom_value_arr[256];
 int error, rom_addr, i;
 unsigned char Start_ID[2], NoOfDataBytes, Data[300]={0}, Checksum;
 int ackerror=0;//equal to 0 means i2c operation normally
 double T_wait=0.0; //无需等待 
 char str[256]=""; 
 int Current_MAX_PAGENUMBER;
 unsigned int Page_Array[MAX_PAGENUMBER];	//有效页标志，如果该页包含有效数据，则等于1，否则=0;  
 
		//分析有效页标志，如果该页包含有效数据，则等于1，否则=0; 注意一页有248个数据
		for (i=0; i<MAX_PAGENUMBER; i++) Page_Array[i]=0;//初始化所有有效页标志为无效；
		for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)
		{	for (i=0; i<MAX_ONEPAGEDATANUMBER; i++)
			{	CurrentDataNumber= CurrentPageNumner*MAX_ONEPAGEDATANUMBER+i;
				if (Data_Array[CurrentDataNumber]!=0xFF)//当前data数据不为0xff就说明该页包含合法数据
				{Page_Array[CurrentPageNumner]=1;//置该页标志为有效 
				 break;
				}
			}
		}

		//如果仅更新0x80000-0x83FFF区域共16K代码区区，共32*512 Byte；
		//注意到下载firmware时，我规定一次页写操作里的有效数据只有248个byte
		//所以需要发起页写操作的次数是32*512/248=66.064516129032258064516129032258，
		//取整用66，尚剩余16个byte没有写入，即0x83FF0-0x83FFF没有写入；但无大碍，此16byte几乎用不到。
		//于是代码区以上的区域，有效页标志都可以清零，表示无需写入。

		//如果仅更新0x80000-0x89FFF区域共40K的代码区，共80*512Byte；
		//注意到下载firmware时，我规定一次页写操作里的有效数据只有248个byte
		//所以需要发起页写操作的次数是80*512/248=165.16129032258064516129032258065，
		//取整用165，尚剩余40个byte没有写入，即0x89FD8-0x89FFF没有写入；但无大碍，此40byte几乎用不到。
		//于是代码区以上的区域，有效页标志都可以清零，表示无需写入。
		
		if (RemainEEPROM)	
		{	for (CurrentPageNumner=165; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)
				Page_Array[CurrentPageNumner]=0;//置该页标志为无效 
		}


		//逐页把数据写入FLASH，当该页的有效标志为1才写，否则不写。
		CurrentPageNumner=0;
		CurrentDataNumber=0;
		//先不写第0页，使80014h地址的数据保持为0xffffffff; 以免在写过程中出错后I2C无法再次访问
		for (CurrentPageNumner=1; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)  
		{ if (Page_Array[CurrentPageNumner]==1) //当该页的有效标志为1才写
  		  {
			error = my_OnePage_WRITEorVERIFY(iIndex, WRITEorVERIFY, CurrentPageNumner, Data_Array);
			if (error)
			{ //sprintf(str, "Error occur when attempt to operate the FLASH at page No.%d.\n", CurrentPageNumner);
			  //SetCtrlVal (panel, TOPPANEL_TEXTBOX, str); 
			  return -1;
			}
		  }
		}	
		//最后写第0页
		CurrentPageNumner=0; 
		{ if (Page_Array[CurrentPageNumner]==1) //当该页的有效标志为1才写
    	  {
			error = my_OnePage_WRITEorVERIFY(iIndex, WRITEorVERIFY, CurrentPageNumner, Data_Array);
			if (error)
			{ //sprintf(str, "Error occur when attempt to operate the FLASH at page No.%d.\n", CurrentPageNumner);
			  //SetCtrlVal (panel, TOPPANEL_TEXTBOX, str); 
			  return -2;
			}
		  }
		}	
		return 0;

}


/*====================================================================================*/
/*====================================================================================*/ 
/*====================================================================================*/ 
/*====================================================================================*/ 


//===================================================================================//
void __stdcall A0_transform(unsigned char *rom_value_arr, unsigned char *A0_str)
{ char str[10000], str1[10000]; 
  union uA0 A0;
  int i;
  	
  	
  	for (i=0; i<256; i++)
		A0.pStrA0[i] = rom_value_arr[i];

	memset (A0_str, 0, sizeof(A0_str));

//byte0, identifier  
	sprintf(str,"\nA0[00] = 0x%02X (Type of serial transceiver)\n", A0.sStrA0.identifier);
	switch(A0.sStrA0.identifier)
	{
		case 0:
			strcat(str, "Unspecified\n");
			break;
		case 1:
			strcat(str, "GBIC\n");
			break;
		case 2:
			strcat(str, "Module soldered to motherboard\n");
			break;
		case 3:
			strcat(str, "SFP\n");
			break;
		case 4:
			strcat(str, "300 pin XBI\n");
			break;
		case 5:
			strcat(str, "Xenpak\n");
			break;
 		case 6:
			strcat(str, "XFP\n");
			break;
		case 7:
			strcat(str, "XFF\n");
			break;
 		case 8:
			strcat(str, "XFP-E\n");
			break;
		case 9:
			strcat(str, "XPak\n");
			break;
		case 10:
			strcat(str, "X2\n");
			break;
		case 11:
			strcat(str, "DWDM-SFP\n");
			break;
		case 12:
			strcat(str, "QSFP\n");
			break;
 		default:
			strcat(str, "Reserved\n");
			break;
	}
	strcat(A0_str, str); 		

//byte1, extIdentifier  
	sprintf(str,"\nA0[01] = 0x%02X (Extended identifier of type of serial transceiver)\n", A0.sStrA0.extIdentifier);
	switch(A0.sStrA0.extIdentifier)
	{
		case 0:
			strcat(str, "MOD_DEF\n");
			break;
		case 1:
			strcat(str, "MOD_DEF1\n");
			break;
		case 2:
			strcat(str, "MOD_DEF2\n");
			break;
		case 3:
			strcat(str, "MOD_DEF3\n");
			break;
		case 4:
			strcat(str, "MOD_DEF4\n");
			break;
		case 5:
			strcat(str, "MOD_DEF5\n");
			break;
		case 6:
			strcat(str, "MOD_DEF6\n");
			break;
		case 7:
			strcat(str, "MOD_DEF7\n");
			break;
 		default:
			strcat(str, "Unallocated\n");
			break;
	}
	strcat(A0_str, str);
			  
//byte2, connector  
	sprintf(str,"\nA0[02] = 0x%02X (Code for connector type)\n", A0.sStrA0.connector);
	switch(A0.sStrA0.connector)
	{
		case 0:
			strcat(str, "Unspecified\n");
			break;
		case 1:
			strcat(str, "SC\n");
			break;
		case 2:
			strcat(str, "Copper 1\n");
			break;
		case 3:
			strcat(str, "Copper 2\n");
			break;
		case 4:
			strcat(str, "BNC/TNC\n");
			break;
		case 5:
			strcat(str, "Coaxil headers\n");
			break;
		case 6:
			strcat(str, "FiberJack\n");
			break;
		case 7:
			strcat(str, "LC\n");
			break;
 		case 8:
			strcat(str, "MT_RJ\n");
			break;
 		case 9:
			strcat(str, "MU\n");
			break;
		case 10:
			strcat(str, "SG\n");
			break;
		case 11:
			strcat(str, "Optical Pigtail\n");
			break;
		case 32:
			strcat(str, "HSSDC II\n");
			break;
		case 33:
			strcat(str, "Copper Pigtail\n");
			break;
		case 34:
			strcat(str, "RJ45\n");
			break;
		default:
			strcat(str, "Unallocated\n");
			break;
	}
	strcat(A0_str, str);
		      
//byte3, InfCompCodesDef  
	sprintf(str,"\nA0[03] = 0x%02X (10G Ethernet/Infiniband Compliance Codes)\n", *(INT8U *)(&A0.sStrA0.infCompCodes));
	if (A0.sStrA0.infCompCodes.bit0==1)//bit0
		strcat(str, "1X Copper Passive\n");
	if (A0.sStrA0.infCompCodes.bit1==1) //bit1
		strcat(str, "1X Copper Active\n");
	if (A0.sStrA0.infCompCodes.bit2==1) //bit2
		strcat(str, "1X LX\n");
	if (A0.sStrA0.infCompCodes.bit3==1) //bit3
		strcat(str, "1X SX\n");
	if (A0.sStrA0.infCompCodes.bit4==1) //bit4  												        {strcpy(str,"03(Infiniband Compliance Codes)\n 10G Base-SR\n");
		strcat(str, "10G Base-SR;"); 
	if (A0.sStrA0.infCompCodes.bit5==1) //bit5  								
		strcat(str, "10G Base-LR\n");
	if (A0.sStrA0.infCompCodes.bit6==1) //bit6 									 
		strcat(str, "10G Base-LRM\n");
	if (A0.sStrA0.infCompCodes.bit7==1) //bit7 									 
		strcat(str, "Unallocated\n");
	strcat(A0_str, str);
		      

//byte4, ESCON_SONETCCDef		   
	sprintf(str,"\nA0[04] = 0x%02X (Part of ESCON/SONET Compliance Codes)\n", *(INT8U *)(&A0.sStrA0.escon_SonetCC));
	if (A0.sStrA0.escon_SonetCC.bit0==1)//bit0
		strcat(str, "OC-48,short reach\n");
	if (A0.sStrA0.escon_SonetCC.bit1==1) //bit1
		strcat(str, "OC-48,intermediate reach\n");
	if (A0.sStrA0.escon_SonetCC.bit2==1) //bit2
		strcat(str, "OC-48,long reach\n");
	if (A0.sStrA0.escon_SonetCC.bit3==1) //bit3
		strcat(str, "OC-48,long reach\n");
	//bit3-4 not handled
	if (A0.sStrA0.escon_SonetCC.bit5==1) 
		strcat(str, "OC-192, short reach2\n"); 
	if (A0.sStrA0.escon_SonetCC.bit6==1) 
		strcat(str, "ESCON SMF, 1310nm Laser\n");
	if (A0.sStrA0.escon_SonetCC.bit7==1) 
		strcat(str, "ESCON MMF, 1310nm LED\n");
	strcat(A0_str, str); 
  
			  
//byte5, SONET Compliance Codes (continue)
	sprintf(str,"\nA0[05] = 0x%02X (SONET Compliance Codes (continue))\n", *(INT8U *)(&A0.sStrA0.sonetCC));
	if (A0.sStrA0.sonetCC.bit0==1) 
		strcat(str, "OC-3, short reach\n");
	if (A0.sStrA0.sonetCC.bit1==1 ) 
		strcat(str, "OC-3, single mode, inter reach\n");
	if (A0.sStrA0.sonetCC.bit2==1 ) 
		strcat(str, "OC-3, single mode, long reach\n");
	if (A0.sStrA0.sonetCC.bit3==1 ) 
		strcat(str, "Unallocated\n");
	if (A0.sStrA0.sonetCC.bit4==1) 
		strcat(str, "OC-12, short reach\n");
	if (A0.sStrA0.sonetCC.bit5==1) 
		strcat(str, "OC-12, single mode, inter reach\n");
	if (A0.sStrA0.sonetCC.bit6==1) 
		strcat(str, "OC-12, single mode, long reach\n");
	if (A0.sStrA0.sonetCC.bit7==1) 
		strcat(str, "Unallocated\n");
	strcat(A0_str, str); 
             
//Ethernet Compliance Codes	  byte6
	sprintf(str,"\nA0[06] = 0x%02X (Ethernet Compliance Codes)\n", *(INT8U *)(&A0.sStrA0.ethernetCC));
	if(A0.sStrA0.ethernetCC.bit0==1 ) 
		strcat(str, "1000BASE-SX\n");
	if (A0.sStrA0.ethernetCC.bit1==1 ) 
		strcat(str, "1000BASE-LX\n");
	if (A0.sStrA0.ethernetCC.bit2==1)
		strcat(str, "1000BASE-CX\n");
	if (A0.sStrA0.ethernetCC.bit3==1 ) 
		strcat(str, "1000BASE-T\n");
	if (A0.sStrA0.ethernetCC.bit4==1 ) 
		strcat(str, "100BASE-LX/LX10\n");
	if (A0.sStrA0.ethernetCC.bit5==1 ) 
		strcat(str, "100BASE-FX\n");
	if (A0.sStrA0.ethernetCC.bit6==1 ) 
		strcat(str, "BASE-BX10\n");
	if (A0.sStrA0.ethernetCC.bit7==1 ) 
		strcat(str, "BASE-PX\n");
	strcat(A0_str, str); 
			 					
// Fibre Channel linkLength	byte7
	sprintf(str,"\nA0[07] = 0x%02X (Fibre Channel linklength/transmitter technology)\n", *(INT8U *)(&A0.sStrA0.fibreChannel));
	if (A0.sStrA0.fibreChannel.bit0==1) 
		strcat(str, "Unallocated\n");
	if (A0.sStrA0.fibreChannel.bit1==1) 
		strcat(str, "Longwave laser (LC)\n");
	if (A0.sStrA0.fibreChannel.bit2==1) 
		strcat(str, "Electrical inter-enclosure (EL)\n");
	if (A0.sStrA0.fibreChannel.bit3==1)
		strcat(str, "medium distance (M)\n");
	if (A0.sStrA0.fibreChannel.bit4==1)
		strcat(str, "long distance (L)\n");
	if (A0.sStrA0.fibreChannel.bit5==1)
		strcat(str, "intermediate distance (I)\n");
	if (A0.sStrA0.fibreChannel.bit6==1)
		strcat(str, "short distance (S)\n");
	if (A0.sStrA0.fibreChannel.bit7==1) 
		strcat(str, "very long distance (V)\n");
	strcat(A0_str, str); 

			           
// Fibre Channel transTec	byte8
	sprintf(str,"\nA0[08] = 0x%02X (Fibre Channel transmitter technology)\n", *(INT8U *)(&A0.sStrA0.transTec));
	if (A0.sStrA0.transTec.bit0==1) 
		strcat(str, "Unallocated\n");
	if (A0.sStrA0.transTec.bit1==1) 
		strcat(str, "Copper FC-BaseT\n");
	if (A0.sStrA0.transTec.bit2==1) 
		strcat(str, "Copper Passive\n");
	if ( A0.sStrA0.transTec.bit3==1)
		strcat(str, "Copper Active\n");
	if ( A0.sStrA0.transTec.bit4==1) 
		strcat(str, "Longwave laser (LL)\n");
	if ( A0.sStrA0.transTec.bit5==1 ) 
		strcat(str, "Shortwave laser with OFC (SL)\n");
	if ( A0.sStrA0.transTec.bit6==1) 
		strcat(str, "Shortwave laser w/o OFC (SN)\n");
	if ( A0.sStrA0.transTec.bit7==1) 
		strcat(str, "Electrical intra-enclosure (EL)\n");
	strcat(A0_str, str); 
				
//  Fibre Channel transmission media    byte9
	sprintf(str,"\nA0[09] = 0x%02X (Fibre Channel transmission media)\n",*(INT8U *)(& A0.sStrA0.transMedia));
	if (A0.sStrA0.transMedia.bit0==1 )  
		strcat(str, "Single Mode(SM)\n");
	if (A0.sStrA0.transMedia.bit1==1 )
		strcat(str, "Unallocated\n");   
	if (A0.sStrA0.transMedia.bit2==1) 
		strcat(str, "Multi-Mode, 50um (M5)\n");
	if (A0.sStrA0.transMedia.bit3==1) 
		strcat(str, "Multi-Mode,62.5um (M6)\n");
	if (A0.sStrA0.transMedia.bit4==1)
		strcat(str, "Video Coax(TV)\n");
	if (A0.sStrA0.transMedia.bit5==1 ) 
		strcat(str, "Miniature Coax(MI)\n");
	if (A0.sStrA0.transMedia.bit6==1) 
		strcat(str, "ShieldedTwisted Pair(TP)\n");
	if (A0.sStrA0.transMedia.bit7==1)
		strcat(str, "Twin Axial Pair(TW)\n");
	strcat(A0_str, str);
             
			                                   
//Fibre Channel speed   byte10
	sprintf(str,"\nA0[10] = 0x%02X (Fibre Channel speed)\n", *(INT8U *)(&A0.sStrA0.fibreChannelSpeed));
	if ( A0.sStrA0.fibreChannelSpeed.bit0==1 )    
		strcat(str, "100 MBytes/sec\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit1==1 )
		strcat(str, "Unallocated\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit2==1 )    
		strcat(str, "200 MBytes/sec\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit3==1 )
		strcat(str, "Unallocated\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit4==1 )
		strcat(str, "400 MBytes/sec\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit5==1 )
		strcat(str, "Unallocated\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit6==1 )
		strcat(str, "800 MBytes/sec\n");   
	if ( A0.sStrA0.fibreChannelSpeed.bit7==1 )
		strcat(str, "1200 MBytes/sec\n");   
	strcat(A0_str, str);
			                                   


//Encoding    byte 11
	sprintf(str,"\nA0[11] = 0x%02X (Code for serial encoding algorithm)\n", A0.sStrA0.encoding);
	switch (A0.sStrA0.encoding)
	{
		case 0:
			strcat(str, "Unspecified\n");
			break;
		case 1:
			strcat(str, "8B10B\n");
			break;
		case 2:
			strcat(str, "4B5B\n");
			break;
		case 3:
			strcat(str, "NRZ\n");
			break;
		case 4:
			strcat(str, "Manchester\n");
			break;
		case 5:
			strcat(str, "SONET Scrambled\n");
			break;
		case 6:
			strcat(str, "64B/66B\n");
			break;
		default:
			strcat(str, "Unspecified\n");
			break; 
	}
	strcat(A0_str, str);
           

//BR, Nominal  byte12
	sprintf (str, "\nA0[12] = 0x%02X (Nominal bit rate)\n",A0.sStrA0.nominalBitRate);
	switch (A0.sStrA0.nominalBitRate)
    {
		case 0:           
			strcat(str, "nominalBitRate\n");
			break; 
		case 1:           
			strcat(str, "100 Mb/s or 125 Mb/s\n");
			break; 
		case 2:           
			strcat(str, "155 Mb/s or 200 Mb/s\n");
			break; 
		case 6:           
			strcat(str, "622 Mb/s\n");
			break; 
		case 11:           
			strcat(str, "1.0625 Gb/s\n");
			break; 
 		case 12: 
			strcat(str, "1.25 Gb/s (0CH)\n");
			break; 
		case 13:           
			strcat(str, "1.25 Gb/s (0DH)\n");
			break;
		case 21:           
			strcat(str, "2.125 Gb/s\n");
			break;
		case 25:           
			strcat(str,"2.5 Gb/s\n");
			break;
		case 42:           
			strcat(str, "4.25 Gb/s (2AH)\n");
			break;
		case 43:           
			strcat(str, "4.25 Gb/s (2BH)\n");
			break;
		case 100:           
			strcat(str, "10 Gb/s\n");
		case 103:           
			strcat(str, "10.3 Gb/s\n");
			break;
		default:
			if (A0.sStrA0.nominalBitRate * 100 >= 1000)
				sprintf (str1, "%.2f Gb/s\n", (float)A0.sStrA0.nominalBitRate * 100./1000.);
			else
				sprintf (str1, "%d Mb/s\n", A0.sStrA0.nominalBitRate * 100.);
		break;
	 }	
	strcat(A0_str, str);
			 
		
//Rate Identifier   byte 13
	sprintf (str, "\nA0[13] = 0x%02X (Type of rate select functionality)\n",A0.sStrA0.rateIdentifier);
	switch (A0.sStrA0.rateIdentifier)
	{
		case 0:
			strcat(str, "Unspecified\n");
			break;
		case 1:
			strcat(str, "4/2/1G Rate_Select & AS0/AS1\n");
			break;
		case 2:
			strcat(str, "8/4/2G Rx Rate_Select\n");
			break;
		case 3:
			strcat(str, "8/4/2G Independent Rx & Tx Rate_select\n");
			break;
		case 4:
			strcat(str, "8/4/2G Tx Rate_Select\n");
			break;
		default:
			strcat(str, "Unspecified\n");
			break; 
	}                   
	strcat(A0_str, str);
		
//9/125  byte 14
	sprintf (str, "\nA0[14] = 0x%02X (Link length supported for 9/125, units of km)\n%d km\n", A0.sStrA0.length9im_km, A0.sStrA0.length9im_km);
	strcat(A0_str, str);

//9/125  byte 15
	sprintf (str, "\nA0[15] = 0x%02X (Link length supported for 9/125, units of 100m)\n%d m\n", A0.sStrA0.length9im, A0.sStrA0.length9im*100);
	strcat(A0_str, str);

//50/125  byte 16
	sprintf (str, "\nA0[16] = 0x%02X (Link length supported for 50/125, units of 10m)\n%d m\n", A0.sStrA0.length50im, A0.sStrA0.length50im*10);
	strcat(A0_str, str);
				
//62.5/125  byte 17
	sprintf (str, "\nA0[17] = 0x%02X (Link length supported for 62.5/125, units of 10m)\n%d m\n", A0.sStrA0.length62p5im, A0.sStrA0.length62p5im*10);
	strcat(A0_str, str);
			 
//copper  byte 18
	sprintf (str, "\nA0[18] = 0x%02X (Link length supported for copper, units of meters)\n%d m\n", A0.sStrA0.lengthCopper, A0.sStrA0.lengthCopper);
	strcat(A0_str, str);
				
//lengthOM3  byte 19
	sprintf (str, "\nA0[19] = 0x%02X (Link length supported for 50 OM3 fiber, units of 10m)\n%d m\n", A0.sStrA0.lengthOM3, A0.sStrA0.lengthOM3*10);
	strcat(A0_str, str);
			 
//vendorName  byte 20-35
	strncpy (str1, A0.sStrA0.vendorName, 16);
	str1[16]=0;
	sprintf (str, "\nA0[20-35] = %s (Vendor name)\n", str1);
	strcat(A0_str, str);
				
//Reserved  byte 36
	sprintf (str, "\nA0[36] = 0x%02X (Reserved)\n",A0.sStrA0.reserved3);
	strcat(A0_str, str);
			 
//vendorOUI  byte 37-39
	strncpy (str1, A0.sStrA0.vendorOUI, 3);
	str1[3]=0;
	sprintf (str, "\nA0[37-39] = %s (Vendor IEEE company ID)\n", str1);
	strcat(A0_str, str);
			 

//vendorPN  byte 40-55
	strncpy (str1, A0.sStrA0.vendorPN, 16);		  
	str1[16]=0;
	sprintf (str, "\nA0[40-55] = %s (Part number provided by vendor)\n", str1);
	strcat(A0_str, str);
				
//vendorRev  byte 56-59
	strncpy (str1, A0.sStrA0.vendorRev, 4);		  
	str1[4]=0;
	sprintf (str, "\nA0[56-59] = %s (Revision level for part number provided by vendor)\n", str1);
	strcat(A0_str, str);
			 
//Laser wavelength  byte 60-61      
	sprintf (str, "\nA0[60-61] = 0x%02X-0x%02X (Laser wavelength)\n%d\n", A0.sStrA0.laserWaveLength[0], A0.sStrA0.laserWaveLength[1], (A0.sStrA0.laserWaveLength[0]*256+A0.sStrA0.laserWaveLength[1]));
	strcat(A0_str, str); 	 
		
//Reserved  byte 62      
	sprintf (str, "\nA0[62] = 0x%02X (Reserved)\n",A0.sStrA0.reserved4);
	strcat(A0_str, str);
				
//cc_Base  byte 63      
	sprintf (str, "\nA0[63] = 0x%02X (Check code for Base ID Fields)\n",A0.sStrA0.cc_Base);
	strcat(A0_str, str);
	
	
//Option values   byte 64
	sprintf (str, "\nA0[64] = 0x%02X (Indicates which optional transceiver signals)\n", *(INT8U *)(&A0.sStrA0.options64));
	if (A0.sStrA0.options64.bit0==1 )  
		strcat(str, "Linear Receiver Output Implemented\n");   
	if (A0.sStrA0.options64.bit1==1 ) 
		strcat(str, "Power Class Declaration\n");   
	if ( A0.sStrA0.options64.bit2==1)
		strcat(str, "Cooled Transceiver Declaration\n");   
	strcat(A0_str, str);
	

				
//Option values   byte 65 
	sprintf (str, "\nA0[65] = 0x%02X (Indicates which optional transceiver signals (continue))\n", *(INT8U *)(&A0.sStrA0.options65));
	if ( A0.sStrA0.options65.bit1==1 )    
		strcat(str, "Rx_LOS\n");   
	if ( A0.sStrA0.options65.bit2==1 )    
		strcat(str, "Signal Detect\n");   
	if ( A0.sStrA0.options65.bit3==1 )    
		strcat(str, "TX_FAULT\n");   
	if ( A0.sStrA0.options65.bit4==1 )    
		strcat(str, "TX_DISABLE\n");   
	if ( A0.sStrA0.options65.bit5==1 )    
		strcat(str, "RATE_SELECT\n");   
	strcat(A0_str, str);
	
	
//maxBiteRate byte 66
	sprintf (str, "\nA0[66] = 0x%02X (Upper bit rate margin)\n",A0.sStrA0.maxBiteRate);
	strcat(A0_str, str);
				
//minBiteRate byte 67
	sprintf (str, "\nA0[67] = 0x%02X (Lower bit rate margin)\n",A0.sStrA0.minBiteRate);
	strcat(A0_str, str);
		
//Serial number  byte 68-83       
	strncpy (str1, A0.sStrA0.vendorSN, 16);		  
	str1[16]=0;
	sprintf (str, "\nA0[68-83] = %s (Serial number provided by vendor)\n", str1);
	strcat(A0_str, str);
						  
//Vendor's manufacturing date  byte 84-91       
	strncpy (str1, A0.sStrA0.dateCode, 8);		  
	str1[8]=0;
	sprintf (str, "\nA0[84-91] = %s (Vendor's manufacturing date code)\n", str1);
	strcat(A0_str, str);
				
//Diagnostic Monitoring Type   byte92    
	sprintf (str, "\nA0[92] = 0x%02X (Indicates which type of diagnostic monitoring)\n", *(INT8U *)(&A0.sStrA0.diagMonitorType));
    if ( A0.sStrA0.diagMonitorType.bit2==1 )    
		strcat(str, "Address change\n");   
	if ( A0.sStrA0.diagMonitorType.bit3==1 )    
		strcat(str, "Average power\n");
	else
	if ( *(INT8U *)(&A0.sStrA0.diagMonitorType) !=0)
		strcat(str, "OMA\n");
	if ( A0.sStrA0.diagMonitorType.bit4==1 )    
		strcat(str, "Externally calibrated\n");   
	if ( A0.sStrA0.diagMonitorType.bit5==1 )    
		strcat(str, "Internally calibrated\n");   
	if ( A0.sStrA0.diagMonitorType.bit6==1 )    
		strcat(str, "Digital diagnostic monitoring implemented\n");   
	if ( A0.sStrA0.diagMonitorType.bit7==1 )    
		strcat(str, "Reserved for legacy diagnostic implementations\n");   
	strcat(A0_str, str);

// Enhanced Options      byte 93
	sprintf(str,"\nA0[93] = 0x%02X (Enhanced Options)\n", *(INT8U *)(&A0.sStrA0.enhancedOptions));
	if ( A0.sStrA0.enhancedOptions.bit0==1 )    
		strcat(str, "Unallocated\n");   
	if ( A0.sStrA0.enhancedOptions.bit1==1 )    
		strcat(str, "Optional Rate Select control implemented per SFF-8431\n");   
	if ( A0.sStrA0.enhancedOptions.bit2==1 ) 
		strcat(str, "Optional Application Select control implemented per SFF-8079\n");   
	if ( A0.sStrA0.enhancedOptions.bit3==1 ) 
		strcat(str, "Optional soft RATE_SELECT control and monitoring implemented\n");   
	if ( A0.sStrA0.enhancedOptions.bit4==1 ) 
		strcat(str, "Optional soft RX_LOS monitoring implemented\n");   
	if ( A0.sStrA0.enhancedOptions.bit5==1 ) 
		strcat(str, "Optional soft TX_FAULT monitoring implemented\n");   
	if ( A0.sStrA0.enhancedOptions.bit6==1 ) 
		strcat(str, "Optional soft TX_DISABLE control and monitoring implemented\n");   
	if ( A0.sStrA0.enhancedOptions.bit7==1 ) 
		strcat(str, "Optional Alarm/warning flags implemented for all monitored quantities\n");   
	strcat(A0_str, str);

				
//  SFF-8472 Compliance    byte94
	sprintf(str,"\nA0[94] = 0x%02X (Indicates which revision of SFF-8472 the transceiver complies with)\n",A0.sStrA0.sff_8472Compliance);
	switch (A0.sStrA0.sff_8472Compliance)
	{
		case 0:
			strcat(str, "DD not included\n");
			break;
		case 1:
			strcat(str, "Rev 9.3 of SFF-8472\n");
			break;
		case 2:
			strcat(str, "Rev 9.5 of SFF-8472\n");
			break;
		case 3:
			strcat(str, "Rev 10.0 of SFF-8472\n");
			break;
		default:
			strcat(str, "Unspecified\n");
			break; 
	}      
	strcat(A0_str, str);

//  Checksum   byte95 				
	sprintf (str, "\nA0[95] = 0x%02X (Check code for the Extended ID Fields)\n",A0.sStrA0.cc_Ext);
	strcat(A0_str, str);
				
}

int __stdcall I2C_MasterReadSlave_DLL (int iIndex, int my_device_addr) 
//firmware规定，如果正确，则A4.sStrA4.I2Ccommand=0，否则=1；所以该函数正确执行将返回0！
{
/*
  int error, i;
  int device_addr, reg_add, rom_Length;
  unsigned char rom_value_arr[256];
  //float T_wait=0.1;

		//写4个byte的命令字到A4.sStrA4.I2Ccommand，A4.sStrA4.I2Cslave_address，A4.sStrA4.I2Cslave_address，A4.sStrA4.I2Cslave_address
			A4.sStrA4.I2Ccommand       = 'R'; 

			device_addr = my_device_addr; 
			reg_add =(int)(&A4.sStrA4.I2Ccommand) - (int)(&A4.sStrA4.FirstByte); 
			rom_value_arr[reg_add] = A4.sStrA4.I2Ccommand;
			rom_value_arr[reg_add+1] = A4.sStrA4.I2Cslave_address;
			rom_value_arr[reg_add+2] = A4.sStrA4.I2Creg_start_add;
			rom_value_arr[reg_add+3] = A4.sStrA4.I2Creg_length;
			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, 4, rom_value_arr, 0.00);//写RAM，可以不等待
			if (error<0) {return -1;}

			Delay(0.1); //因为最多9.4*5=47ms就进入IRQ，随后发起的读DS1856最多需要10ms，所以等100ms已经足够7020发起并结束I2C操作
		
		//读入I2Creg_length个数据和ErrorCode 
 			error = I2C_BYTEs_READ_DLL (device_addr, 0, 30, A4.pStrA4);
			if (error<0) {return -1;}

	return A4.sStrA4.I2Ccommand;
*/
	return 0;
}

int __stdcall I2C_MasterWriteSlave_DLL (int iIndex, int my_device_addr) 
//firmware规定，如果正确，则A4.sStrA4.I2Ccommand=0，否则=1；所以该函数正确执行将返回0！ 
{/*
  int error, i;
  int device_addr, reg_add, rom_Length;
  //float T_wait = 0.1;
  
		//先写I2Creg_length个数据到A4.sStrA4.I2Creg_dat_arr[]
			device_addr = my_device_addr; 
			reg_add =(int)(&A4.sStrA4.I2Creg_dat_arr[0]) - (int)(&A4.sStrA4.FirstByte); 

			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, A4.sStrA4.I2Creg_length, A4.pStrA4, 0.00);//写RAM，可以不等待 
			if (error<0) {return -1;}
			

		//写4个byte的命令字到A4.sStrA4.I2Ccommand，A4.sStrA4.I2Cslave_address，A4.sStrA4.I2Cslave_address，A4.sStrA4.I2Cslave_address
			A4.sStrA4.I2Ccommand       = 'W'; 
			reg_add =(int)(&A4.sStrA4.I2Ccommand) - (int)(&A4.sStrA4.FirstByte); 

			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, 4, A4.pStrA4, 0.00); //写RAM，可以不等待
			if (error<0) {return -1;}
													 
			Delay(0.1); //因为最多9.4*5=47ms就进入IRQ，随后发起的写DS1856最多需要10ms，所以等100ms已经足够7020发起并结束I2C操作
			
		//读入I2Creg_length个数据和ErrorCode 
 			error = I2C_BYTEs_READ_DLL (device_addr, 0, 30, A4.pStrA4);
			if (error<0) {return -1;}

	return A4.sStrA4.I2Ccommand;
*/
	return 0;
}

/*=============CH341===========================================================================*/
void __stdcall SetUSBiMode_DLL (int iMode)
{
	USBiMode = iMode;
}

/*=============F320===========================================================================*/
void __stdcall SetF320I2CRate_DLL (int I2CRate) 
{
	 F320I2CRate = I2CRate;
}

void __stdcall SetF320I2CSTOPbeforeReSTART_DLL (int I2CSTOPbeforeReSTART) 
{
	 F320I2CSTOPbeforeReSTART = I2CSTOPbeforeReSTART;
}

int __stdcall I2C_HOST_RESET_F320_DLL (int iIndex, int I2CRate, int I2CSTOPbeforeReSTART)
{
	unsigned char  ioBuffer[300];

	ioBuffer[0] = I2CRate;
	ioBuffer[1] = I2CSTOPbeforeReSTART;
	
	return F320ExecuteCmd(iIndex, IIC_RST, 2, 0, ioBuffer);
}

void __stdcall RESET_F320_DLL (int iIndex)
{
	F320ExecuteCmd(iIndex, MCU_RST, 0, 0, NULL);    
}

int __stdcall GetF320Numbers(int *lpdwNumDevices) //读取当前挂上PC的F320的数量
{
	SI_STATUS  status;

	status = SI_GetNumDevices(lpdwNumDevices); 
	
	return  status;
}

int __stdcall F320ExecuteCmd_DLL(int iIndex, unsigned char iCommand, int iWriteLength, int iReadLength, unsigned char *ioBuffer)
{
	return F320ExecuteCmd(iIndex, iCommand, iWriteLength, iReadLength, ioBuffer); 
}

int __stdcall GetF320SerialNumber(int iIndex, unsigned char *SN) //读取当前挂上PC的F320的SN
{unsigned char  ioBuffer[300];
 int error;

	error =  F320ExecuteCmd(iIndex, GET_SN, 0, 16, SN);
	SN[16]=0;
	
	return error;
}

int __stdcall SetF320SerialNumber(int iIndex, unsigned char *SN) //设置当前挂上PC的F320的SN
{unsigned char  ioBuffer[300];

	return F320ExecuteCmd(iIndex, SET_SN, 16, 0, SN);  
}

int __stdcall F320_SPI_Init_DLL(int iIndex, int SPI_Rate, int SCK_PHASE, int SCK_POLARITY) 
//F320ExecuteCmd(0, SPI_RST, 3, 0, ioArray) 复位并重新设置SPI的速率，相位，极性 
//ioArray[0]：速率，计算公式SCK = 12000/( ioArray[0]+1)K，如ioArray[0] = 119，则SPI速率为100K
//ioArray[1]：相位，0：在SCK周期的第一个边沿采样数据，1：在SCK周期的第二个边沿采样数据
//ioArray[1]：极性，0：SCK在空闲状态时处于低电平，1：SCK在空闲状态时处于高电平
//默认设置为100K，第一个边沿采样数据，SCK在空闲状态时处于低电平
{unsigned char  ioBuffer[300];

	ioBuffer[0] = SPI_Rate;
	ioBuffer[1] = SCK_PHASE;
	ioBuffer[2] = SCK_POLARITY;
	return F320ExecuteCmd(iIndex, SPI_RST, 3, 0, ioBuffer);
}

int __stdcall SetF320_FPGA_PROGB_DLL(int iIndex, int PROGB) 
//F320ExecuteCmd(0, SPI_NSS, 1, 0, NSS);设置NSS电平，0：低电平；1：高电平
{unsigned char  ioBuffer[300];

	ioBuffer[0] = PROGB;
	return F320ExecuteCmd(iIndex, SET_PG, 1, 0, ioBuffer);
}

int __stdcall SetF320_SPI_NSS_DLL(int iIndex, int NSS) 
//F320ExecuteCmd(0, SPI_NSS, 1, 0, NSS);设置NSS电平，0：低电平；1：高电平
{unsigned char  ioBuffer[300];

	ioBuffer[0] = NSS;
	return F320ExecuteCmd(iIndex, SPI_NSS, 1, 0, ioBuffer);
}

int __stdcall SetF320_SPI_CS9_CS0_DLL(int iIndex, int CS9_CS0) 
//F320ExecuteCmd(iIndex, SET_CS, 1, 0, CS9_CS0);设置CS9_CS0电平，CS9_CS0的bit9..bit0分别对应GPIO的CS9_CS0电平
{unsigned char  ioBuffer[300];

	ioBuffer[0] = CS9_CS0 & 0xff;
	ioBuffer[1] = (CS9_CS0 >> 8) & 0xff;
	
	return F320ExecuteCmd(iIndex, SET_CS, 2, 0, ioBuffer);
}

int __stdcall SPI_BYTEs_READWRITE_F320_DLL (int iIndex, int iRWLength, unsigned char *iBuffer, unsigned char *oBuffer)
{SI_STATUS  status;

	status = SPI_BYTEs_READWRITE_F320(iIndex, iRWLength, iBuffer, oBuffer);
	return status;
}


int __stdcall GET_Version_F320_DLL (int iIndex, unsigned char *FVER)
{unsigned char  ioBuffer[300];
 int error;

	error = F320ExecuteCmd(iIndex, GET_FVER, 0, 1, ioBuffer); 
	*FVER = ioBuffer[0];
	return error;
}

int __stdcall GET_TM_F320_DLL (int iIndex, unsigned char *TM)
{unsigned char  ioBuffer[300];
 int error;

	error = F320ExecuteCmd(iIndex, GET_TM, 0, 1, ioBuffer); 
	*TM = ioBuffer[0]; 
	return error;
}

int __stdcall SET_TM_F320_DLL (int iIndex, unsigned char TM)
{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= TM;
	error = F320ExecuteCmd(iIndex, SET_TM, 1, 0, ioBuffer); 
	return error;
}

int __stdcall SET_FS_F320_DLL (int iIndex, unsigned char FS)
//    该命令后面跟1个数据，数据解释如下：
//    0x00：选择晶振输出频率115.52MHz
//    0x01: 选择晶振输出频率161.13281MHz
//    0x02: 选择晶振输出频率167.33165MHz
//    0x03: 选择晶振输出频率173.37075MHz
//    其他数据无效，保持上一次频率选择结果
{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= FS;
	error = F320ExecuteCmd(iIndex, SET_FS, 1, 0, ioBuffer); 
	return error;
}

int __stdcall SEL_SPIIIC_F320_DLL (int iIndex, unsigned char SI)
{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= SI;
	error = F320ExecuteCmd(iIndex, SEL_SPIIIC, 1, 0, ioBuffer); 
	return error;
}

int __stdcall SDN_VCC_F320_DLL (int iIndex, unsigned char VCC)
//    该命令后面跟一个数据，数据解释如下：
//    数据的bit0：控制OLT电源的开断，1为打开，0为关断
//    数据的bit1：控制ONU电源的开断，1为打开，0为关断
//    其它bit无效，默认全部开电
{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= VCC;
	error = F320ExecuteCmd(iIndex, SDN_VCC, 1, 0, ioBuffer); 
	return error;
}

int __stdcall SEL_IIC_F320_DLL (int iIndex, unsigned char IIC)
//    该命令后面跟一个数据，数据解释如下：
//    数据的bit0：OLT IIC总线使能，1为使能，0禁止
//    数据的bit1：ONU1 IIC总线使能，1为使能，0禁止
//    数据的bit2：BERT IIC总线使能，1为使能，0禁止
//    数据的bit3：ARM IIC总线使能，1为使能，0禁止
//    其它bit无效，默认全部使能

{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= IIC;
	error = F320ExecuteCmd(iIndex, SEL_IIC, 1, 0, ioBuffer); 
	return error;
}

//20090909
int __stdcall EPP_BYTEs_WRITE_F320_DLL (int iIndex, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
{SI_STATUS  status;
 unsigned char ioBuffer[300];
 int i;
 
	ioBuffer[0] = rom_startaddress;   //写数据的第一个字节必须为首地址
	for (i=0; i<rom_Length; i++)
		ioBuffer[i+1] = rom_value_arr[rom_startaddress+i];
		
	status = F320StreamEPP(iIndex-0x800, EPP_W, rom_Length, 0, ioBuffer); //iDataLen为要读的数据实际长度，支持256字节操作 
	
	return status;
}

int __stdcall EPP_BYTEs_READ_F320_DLL (int iIndex, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
{SI_STATUS  status;
 unsigned char ioBuffer[300];
 int i;
 
	ioBuffer[0] = rom_startaddress;   //写数据的第一个字节必须为首地址
	for (i=0; i<rom_Length; i++)
		ioBuffer[i+1] = rom_value_arr[rom_startaddress+i];
		
	status = F320StreamEPP(iIndex-0x800, EPP_R, 0, rom_Length, ioBuffer); ////iDataLen为要读的数据实际长度，支持256字节操作

	for (i=0; i<rom_Length; i++)
	rom_value_arr[rom_startaddress + i] = ioBuffer[i];
	
	return status;
}




/* USB-C2  */
int __stdcall F320StreamC2(int iIndex, unsigned char iCommand, int iWriteLength, int iReadLength, unsigned char *ioBuffer)
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;

	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = iCommand;
		
		if(iCommand == C2_RST || iCommand == C2_INIT || iCommand == C2_ERSCP)
		{
			UsbPacket.error_code = 0;
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}
		else if(iCommand == C2_GETID && iReadLength == 1)
		{
			UsbPacket.error_code = 0;
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + 1, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + 1 || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}

			memcpy(ioBuffer, UsbPacket.io_dat, 1);
		}
		else if(iCommand == C2_ERSPG)
		{
			UsbPacket.dat_length[1] = 2;
			UsbPacket.dat_length[0] = 0;
			memcpy(UsbPacket.io_dat, ioBuffer, 2);

			UsbPacket.error_code = 0;
		
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + 2, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + 2)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}
		else if(iCommand == C2_WRPG && iWriteLength == 256)
		{
			UsbPacket.dat_length[1] = 0;
			UsbPacket.dat_length[0] = 1;
			memcpy(UsbPacket.io_dat, ioBuffer, 256 + 2);
			
			UsbPacket.error_code = 0;
		
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + 256 + 2, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + 256 + 2)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		}
		else if(iCommand == C2_RDPG && iReadLength == 256)
		{
			UsbPacket.dat_length[1] = 0;
			UsbPacket.dat_length[0] = 1;
			memcpy(UsbPacket.io_dat, ioBuffer, 2);
			
			UsbPacket.error_code = 0;
		
			SI_SetTimeouts(0, dwTimeout);
			status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + 2, &BytesWritten, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + 2)
			{
				SI_Close(UsbHandle);
				return -1;
			}
		
			SI_SetTimeouts(dwTimeout, 0);
			status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + 256, &BytesReturned, NULL);
			SI_SetTimeouts(tmpReadTO, tmpWriteTO);
			if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH + 256 || UsbPacket.error_code != 0)
			{
				SI_Close(UsbHandle);
				return -1;
			}
			
			memcpy(ioBuffer, UsbPacket.io_dat, 256);
		}
		else
		{
			SI_Close(UsbHandle);
			return -1;
		}

		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}

int __stdcall SEL_CLK_F320_DLL (int iIndex, unsigned char Channel)
{
	unsigned char  ioBuffer[300];
	int error;

	ioBuffer[0]= Channel;
	error = F320ExecuteCmd(iIndex, CLK_SEL, 1, 0, ioBuffer); 
	
	return error;
}

int __stdcall CLK_Get_F320_DLL(int iIndex, unsigned char *Channel) 
{
	unsigned char  ioBuffer[300];
	int error;

	error =  F320ExecuteCmd(iIndex, GET_CLK_CH, 0, 1, ioBuffer);
	*Channel=ioBuffer[0];
	
	return error;
}

int __stdcall IIC_Uart_Open(int iIndex,int rom_Length,BYTE *rom_value_arr) //打开串口
{
	unsigned char  ioBuffer[300];
	if (iIndex<0) return -1;
		//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //人为减去0x800这个偏移量
		memcpy(ioBuffer, rom_value_arr, rom_Length); 
	    return F320ExecuteCmd(iIndex, IIC_UART_OPEN, rom_Length, 0, ioBuffer); 
	}
	return -1; 
}

int __stdcall IIC_Uart_Close(int iIndex,int rom_Length,BYTE *rom_value_arr) //关闭串口
{
	unsigned char  ioBuffer[300];
	if (iIndex<0) return -1;
		//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //人为减去0x800这个偏移量
		memcpy(ioBuffer, rom_value_arr, rom_Length); 
	    return F320ExecuteCmd(iIndex, IIC_UART_CLOSE, rom_Length, 0, ioBuffer);
	}
	return -1;  
}

int __stdcall IIC_Uart_Read (int iIndex, int imode,int Read_Length, int *BytesReturned, unsigned char *rom_value_arr)  //读串口
{
	unsigned char  ioBuffer[300];
	int error;
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesRead;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;
	
	if (iIndex<0) return -1;
		//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		status = SI_SUCCESS;
		if(0==imode)
        {
			iIndex = iIndex-0x800; //人为减去0x800这个偏移量
	        status = SI_Open(iIndex, &UsbHandle);
		}
	    if(status == SI_SUCCESS)
	    {
		   
			SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
			
		    UsbPacket.op_command = IIC_UART_READ;
			
			UsbPacket.dat_length[1] = 2;
			UsbPacket.dat_length[0] = 0;
			
			ioBuffer[1]=Read_Length % 256;
		    ioBuffer[0]=Read_Length / 256;   
			memcpy(UsbPacket.io_dat, ioBuffer, 2);
		    
		    UsbPacket.error_code = 0;
		
		    SI_SetTimeouts(0, dwTimeout);
		    status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + 2, &BytesWritten, NULL);
		    SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		    if((status != SI_SUCCESS) || (BytesWritten != (HEADER_LENGTH + 2)))
		    {
			    SI_Close(UsbHandle);
			    return -1;
		    }

		    SI_SetTimeouts(dwTimeout, 0);
		    status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH + Read_Length, &BytesRead, NULL);
		    SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		    if(status != SI_SUCCESS  || UsbPacket.error_code != 0)
		    {
			    SI_Close(UsbHandle);
			    return -1;
		    }
			*BytesReturned =BytesRead-HEADER_LENGTH;   
		    memcpy(rom_value_arr, UsbPacket.io_dat, *BytesReturned);

			if(imode)
			{
				return 0;  
			}
			else
			{
		        status = SI_Close(UsbHandle);
		        if(status != SI_SUCCESS)
		        {
			        return -1;
		        }
		        else
		        {
			       return 0;
		        }  
			}
	    }
	    else
	    {
		    return -1;
	    }
	}
	return  -1;
}
int __stdcall   F320_PDI_Get_deviceID_DLL(int iIndex, unsigned char *rom_value_arr)
{
	
	return F320ExecuteCmd(iIndex, PDI_GET_DEVICE_ID, 0, 3, rom_value_arr);
   	
}

int __stdcall   F320_PDI_Get_BOD_DLL(int iIndex, unsigned char *rom_value_arr)
{
	
	return F320ExecuteCmd(iIndex, PDI_GET_DEVICE_BOD, 0, 6, rom_value_arr);
   	
}

int __stdcall   F320_PDI_Program_BOD_DLL(int iIndex,unsigned char *rom_value_arr)
{
	
	return F320ExecuteCmd(iIndex, PDI_WRITE_DEVICE_BOD, 2, 0, rom_value_arr);
   	
}


int __stdcall   F320_PDI_Earse_NVM_Page_DLL(int iIndex,int rom_PageAddress)    
{
	unsigned char  ioBuffer[300];
	
	ioBuffer[0]= (rom_PageAddress >> 8) & 0xFF;
	ioBuffer[1]= rom_PageAddress & 0xFF;
	return F320ExecuteCmd(iIndex, PDI_EARSE_NVM_PAGE, 2, 0, ioBuffer);
}

int __stdcall   F320_PDI_Earse_NVM_All_DLL(int iIndex)
{
	unsigned char  ioBuffer[300];
	
	return F320ExecuteCmd(iIndex, PDI_ERASE_NVM_ALL, 0, 0, ioBuffer);
}

int __stdcall   F320_PDI_Write_NVM_DLL(int iIndex,int rom_startaddress,int iRWLength, unsigned char *iBuffer)
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;
	
	if(iRWLength > 280)
	{
		return -1;
	}

	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = PDI_WRITE_NVM;
		UsbPacket.dat_length[1] = (iRWLength+4) % 256;
		UsbPacket.dat_length[0] = (iRWLength+4) / 256;

		UsbPacket.io_dat[0]=  (rom_startaddress >> 8) & 0xFF; 
		UsbPacket.io_dat[1]=  rom_startaddress  & 0xFF; 
		UsbPacket.io_dat[2]=  (iRWLength >> 8) & 0xFF; 
		UsbPacket.io_dat[3]=  iRWLength  & 0xFF; 
		
		
		memcpy(UsbPacket.io_dat+4, iBuffer, iRWLength);
		UsbPacket.error_code = 0;
		
		if(((HEADER_LENGTH + iRWLength+4) % 64) == 0)
			{
				UsbPacket.io_dat[iRWLength+4] = 0xAA;
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength + 5, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength + 5)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			else
			{
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength + 4, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength + 4)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH  || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return -1;
		}
		
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
	
}

int __stdcall   F320_PDI_Read_NVM_DLL(int iIndex,int rom_startaddress,int rom_Length, unsigned char *rom_value_arr)
{
	unsigned char  ioBuffer[300];
	int error;

	ioBuffer[0]=  (rom_startaddress >> 8) & 0xFF; 
	ioBuffer[1]=  rom_startaddress  & 0xFF; 
	
	ioBuffer[2]=  (rom_Length >> 8) & 0xFF; 
	ioBuffer[3]=  rom_Length  & 0xFF; 
	
	error = F320ExecuteCmd(iIndex, PDI_READ_NVM, 4, rom_Length, ioBuffer);
	if(error == 0)
	{
		memcpy(rom_value_arr, ioBuffer, rom_Length);
		return 0;
	}
	else return  -1;
	
}

int __stdcall   F320_PDI_Earse_EEP_Page_DLL(int iIndex,int rom_PageAddress)    
{
	unsigned char  ioBuffer[300];
	
	ioBuffer[0]= (rom_PageAddress >> 8) & 0xFF;
	ioBuffer[1]= rom_PageAddress & 0xFF;
	return F320ExecuteCmd(iIndex, PDI_ERASE_EEPROM_PAGE, 2, 0, ioBuffer);
}

int __stdcall   F320_PDI_Earse_EEP_All_DLL(int iIndex)
{
	unsigned char  ioBuffer[300];
	
	return F320ExecuteCmd(iIndex, PDI_ERASE_EEPROM_ALL, 0, 0, ioBuffer);
}

int __stdcall   F320_PDI_Write_EEP_DLL(int iIndex,int rom_startaddress,int iRWLength, unsigned char *iBuffer)
{
	HANDLE     UsbHandle;
	SI_STATUS  status;
	USB_PACKET UsbPacket;
	DWORD      BytesWritten;
	DWORD      BytesReturned;
	DWORD      tmpReadTO;
	DWORD      tmpWriteTO;
	
	if(iRWLength > 280)
	{
		return -1;
	}

	status = SI_Open(iIndex, &UsbHandle);
	if(status == SI_SUCCESS)
	{
		SI_GetTimeouts(&tmpReadTO, &tmpWriteTO);
		UsbPacket.op_command = PDI_WRITE_EEPROM;
		UsbPacket.dat_length[1] = (iRWLength+4) % 256;
		UsbPacket.dat_length[0] = (iRWLength+4) / 256;
		

		UsbPacket.io_dat[0]=  (rom_startaddress >> 8) & 0xFF; 
		UsbPacket.io_dat[1]=  rom_startaddress  & 0xFF; 
		
		UsbPacket.io_dat[2]=  (iRWLength >> 8) & 0xFF; 
		UsbPacket.io_dat[3]=  iRWLength  & 0xFF; 
		
		
		memcpy(UsbPacket.io_dat+4, iBuffer, iRWLength);
		UsbPacket.error_code = 0;
		
		if(((HEADER_LENGTH + iRWLength + 4) % 64) == 0)
			{
				UsbPacket.io_dat[iRWLength+4] = 0xAA;
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength + 5, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength + 5)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
			else
			{
				SI_SetTimeouts(0, dwTimeout);
				status = SI_Write(UsbHandle, &UsbPacket, HEADER_LENGTH + iRWLength+4, &BytesWritten, NULL);
				SI_SetTimeouts(tmpReadTO, tmpWriteTO);
				if(status != SI_SUCCESS || BytesWritten != HEADER_LENGTH + iRWLength + 4)
				{
					SI_Close(UsbHandle);
					return -1;
				}
			}
		
		SI_SetTimeouts(dwTimeout, 0);
		status = SI_Read(UsbHandle,	&UsbPacket,	HEADER_LENGTH, &BytesReturned, NULL);
		SI_SetTimeouts(tmpReadTO, tmpWriteTO);
		if(status != SI_SUCCESS || BytesReturned != HEADER_LENGTH || UsbPacket.error_code != 0)
		{
			SI_Close(UsbHandle);
			return -1;
		}
		status = SI_Close(UsbHandle);
		if(status != SI_SUCCESS)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
	
}

int __stdcall   F320_PDI_Read_EEP_DLL(int iIndex,int rom_startaddress,int rom_Length, unsigned char *rom_value_arr)
{
	unsigned char  ioBuffer[300];
	int error;
	ioBuffer[0]=  (rom_startaddress >> 8) & 0xFF; 
	ioBuffer[1]=  rom_startaddress  & 0xFF; 
	
	ioBuffer[2]=  (rom_Length >> 8) & 0xFF; 
	ioBuffer[3]=  rom_Length  & 0xFF; 
	
	error = F320ExecuteCmd(iIndex, PDI_READ_EEPROM, 4, rom_Length, ioBuffer);
	if((error == 0) &&( rom_startaddress == ioBuffer[2]*128+ioBuffer[3]))
	{
		memcpy(rom_value_arr, ioBuffer, rom_Length);
		return 0;
	}
	else return  -1;
}
