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
#include "SiUSBXp.h" 		//F320��ͷ�ļ�

#define MAX_BYTENUMBER 62*1024 		//��ദ��62k byte�� Limited by ADuC70xxBCPZxxI
#define MAX_ROWNUMBER MAX_BYTENUMBER/16  	//62k byte ���� ÿ��16byte ���õ��������=3968
#define MAX_ONEPAGEDATANUMBER 248  	//ÿҳ������Byte���� =248
#define MAX_PAGENUMBER MAX_BYTENUMBER/MAX_ONEPAGEDATANUMBER //62k byte ���� ÿҳ������Byte���� ���õ����ҳ�� =256

int DLLVersion = 16;         //20131226
#define SOFTVER  "1.1.0.2"   //20131226  

int USBiMode=1; 	//CH341 I2C����; I2C�ӿ��ٶ�/SCLƵ��, 00=����/20KHz,01=��׼/100KHz(Ĭ��ֵ),10=����/400KHz,11=����/750KHz
int F320I2CRate=1; 	//F320 I2C����; 2CRate: 0=50KHz, 1=100KHz, 2=150KHz, others: Fi2c=24MHz/3/(256-I2CRate) 
int F320I2CSTOPbeforeReSTART=0; //=0������I2C��ʱ���ReSTART֮ǰ����STOP��=1������I2C��ʱ���ReSTART֮ǰ��һ��STOP
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
//û�ҵ�host�ͷ���-1�����򷵻�0
//���USBתI2C��·�壬��I2C_position=2; 
//unsigned int my_iIndex   
// ָ��CH341 USB�豸���
//unsigned int my_iMode	
// ָ��CH341ģʽ,������
// λ1 λ0: I2C �ٶ�/SCL Ƶ��, 00=����20KHz,01=��׼100KHz,10=����400KHz,11=����750KHz
// λ2: SPI ��I/O ��/IO ����, 0=���뵥��(4 �߽ӿ�),1=˫��˫��(5 �߽ӿ�)
// ��������,����Ϊ0
{ 
	int error;
	void *hUSBDevice;

	if (iIndex<0) return -1;//none
	
	//CH341A
	if ( (iIndex>=0) && (iIndex<=0xFF) )	
	{	error = InitialUSB(iIndex, USBiMode); //��USB���
		if (error == 0)//�ɹ�
		{   CloseUSB(iIndex); //�ر�USB��� 
		//	return 0;			//����ɾ�������Ҫȷ���Ƿ������� Eric.Yao 20140603
		}
		else   return -1;
	}
	
	//LPT
	if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
	{	error = InitialLPT (iIndex);
		if (error) //ʧ��
		{
			return -1;
		}
	}
	
	//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
		USBStatus = SI_Open(iIndex, &hUSBDevice); //��USB��� 
		if (USBStatus==SI_SUCCESS) //�ɹ�
	  	{	
			SI_Close(hUSBDevice);  //�ر�USB��� 
	  		I2C_HOST_RESET_F320_DLL(iIndex, F320I2CRate, F320I2CSTOPbeforeReSTART); //��λF320��I2C Host
			
	  		Delay(0.1);//�ȴ���λ����
	  	}			
		else  return -1; 
	}

	//�ɹ�
	return 0;
}

int __stdcall I2C_SLAVE_SEARCH_DLL (int iIndex, int device_addr)
//û�ҵ�slave�ͷ���-1�����򷵻�0 
{	int error;

  		if (iIndex<0) return -1;
		
		//CH341A
		if ( (iIndex>=0) && (iIndex<=0xFF) )	
		{	error = I2C_SLAVE_SEARCH_USB(iIndex, USBiMode, device_addr); //��USB���
			return error; 
		}
		
		//LPT
		if ( (iIndex>=0x100) && ((iIndex<0x400)) ) 
		{	error = I2C_SLAVE_SEARCH_LPT (iIndex, device_addr);
			return error; 
		}

		//F320
		if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
			error = I2C_SLAVE_SEARCH_F320 (iIndex, device_addr);
			return error; 
		}
		
		return 0;
}

int __stdcall I2C_BYTE_CURRENT_ADDRESS_READ_DLL (int iIndex, int device_addr, unsigned char *rom_value)
//���ֽ�������
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
			error = I2C_BYTE_CURRENT_ADDRESS_WRITE_F320 (iIndex, device_addr, rom_startaddress, T_wait);
			return error; 
		}
		
		return 0;
}

int __stdcall I2C_BYTE_READ_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value)
//���ֽ�ָ����ַ��
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
			error = I2C_BYTE_READ_F320 (iIndex, device_addr, rom_startaddress, rom_value);
			return error; 
		}

		return 0;

}

int __stdcall I2C_BYTEs_READ_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
//���ֽ�ָ���׵�ַ������
//ע�⣬rom_value_arr[]����Ҫȫ������ȥ��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]�������
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
			error = I2C_BYTEs_READ_F320 (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr);
			return error; 
		}
		
		return 0;

}

int __stdcall I2C_BYTE_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait)
//���ֽ�ָ����ַд
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
			error = I2C_BYTE_WRITE_F320 (iIndex, device_addr, rom_startaddress, rom_value, T_wait);
			return error; 
		}
		
		return 0;

}

int __stdcall I2C_BYTEs_WRITE_DLL (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait)
//���ֽ�ָ���׵�ַ����д 
//ע�⣬rom_value_arr[]Ҫȫ������ȥ��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������ݡ�
//ע�⣬����EEPROM��һ���ֻ��8byte��д��������7020��һ���û����󳤶ȵ����ơ�
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
		{	iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
		{	error = I2C_BYTEs_WRITE_F320 (iIndex, device_addr, rom_startaddress, rom_Length, rom_value_arr, T_wait);
			return error; 
		}
		}

		return 0;

}

int __stdcall I2C_2BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2)
//˫�ֽ�ָ���׵�ַ�� 
//ע�⣬ rom_value1�ĵ�ַ��rom_startaddress��rom_value2�ĵ�ַ��rom_startaddress+1
{ int error;
  unsigned char rom_value_arr[256];
  			if (iIndex<0) return -1;
			error = I2C_BYTEs_READ_DLL (iIndex, device_addr, rom_startaddress, 2, rom_value_arr);

			*rom_value1 = rom_value_arr[rom_startaddress]; 
			*rom_value2 = rom_value_arr[rom_startaddress+1]; 
			
			return error;
}

int __stdcall I2C_4BYTEs_READ_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3, unsigned char *rom_value4)
//���ֽ�ָ���׵�ַ�� 
//ע�⣬ rom_value1�ĵ�ַ��rom_startaddress��rom_value2�ĵ�ַ��rom_startaddress+1��rom_value3�ĵ�ַ��rom_startaddress+2��rom_value4�ĵ�ַ��rom_startaddress+3
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
//˫�ֽ�ָ���׵�ַд 
//ע�⣬ rom_value1�ĵ�ַ��rom_startaddress��rom_value2�ĵ�ַ��rom_startaddress+1
{ int error;
  unsigned char rom_value_arr[256];
  		if (iIndex<0) return -1;
		rom_value_arr[rom_startaddress]   = rom_value1;
		rom_value_arr[rom_startaddress+1] = rom_value2;
		error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, rom_startaddress, 2, rom_value_arr, T_wait);

	return error;
}

int __stdcall I2C_4BYTEs_WRITE_DLL(int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, float T_wait)
//���ֽ�ָ���׵�ַд 
//ע�⣬ rom_value1�ĵ�ַ��rom_startaddress��rom_value2�ĵ�ַ��rom_startaddress+1��rom_value3�ĵ�ַ��rom_startaddress+2��rom_value4�ĵ�ַ��rom_startaddress+3
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

																

		//ת��hex�ļ���Data_Array[]=============================================================
			//��hex�ļ�
			DisableBreakOnLibraryErrors (); 
			fp=fopen(hexFile,"rt");
			EnableBreakOnLibraryErrors ();
			if (fp==NULL) 
			{	return -3;  }
		
			absolute_address=0;
			EffectiveRowNumber=0;//������hex�ļ��й�ռ������
			checksum_error=0;	 //�����ǰ�е�checksum���������ۼ�1
			for (i=0; i<MAX_BYTENUMBER; i++) Data_Array[i]=0xff; //��ʼ��62k��������
			
			//�жϵ�ǰ�ļ��Ƿ���ADuC���ݸ�ʽ��HEX�ļ����жϱ�׼�ǿ��ļ���һ���Ƿ�����չ��ַ��
			//51�˵�hex��һ�оͲ�����չ��ַ����Ȼ����жϱ�׼���Ǻܿ�ѧ
			if (!feof (fp)) //����ļ���δ����
			{	fscanf(fp,":%2X%4X%2X",&LL,&AAAA,&TT); //��ȡÿ�й��еġ���ͷ����:LLAAAATT��
				//BeCompatibleWithADuCHEXFormat = ((TT == 4) || (TT == 2));
				BeCompatibleWithADuCHEXFormat = ((TT == 4) || (TT == 2) || (TT == 5));
				fseek (fp, 0, SEEK_SET); //�ص��ļ���
			}

			extended_linear_address=0;
			extended_segment_address=0;
			while(!feof (fp) && BeCompatibleWithADuCHEXFormat && (EffectiveRowNumber<MAX_ROWNUMBER)) 
			//����ļ���δ���꣬������ADuC���ݸ�ʽ��HEX�ļ�,������Ч���ݵ�����δ����MAX_ROW
			{	fscanf(fp,":%2X%4X%2X",&LL,&AAAA,&TT); //��ȡÿ�й��еġ���ͷ����:LLAAAATT��
				
				//����TT�жϺ�����������
				switch (TT)
				{
				case 0: //data record
					//��ȡ���Ե�ַ����ע�⵽���62k�����Ե�16bit�͹��ˣ������ò�����16bit�ĵ�ֵַ��
					absolute_address = extended_linear_address + extended_segment_address + AAAA;
					//��ȡ����LL�����ݣ�����checksum��ע���ʱData_Array������� i+(absolute_address&0xffff)
					checksum = LL + (AAAA>>8) + (AAAA & 0xff) + TT;
					for (i=0; i<LL; i++) //��ȡLL������
					{	fscanf(fp,"%2X", &temp_int);
						Data_Array[i+(absolute_address&0xffff)]=temp_int;//��������ݸ�ֵ
						checksum = checksum + temp_int;
					}
					checksum = 0x01 + ~checksum; 
					fscanf(fp,"%2X\n",&CC); //��ȡCC�ͻس������ļ�ָ��ָ����һ��
					//У������
					if (checksum != CC) checksum_error++; 					
					//�����ۼ�1
					EffectiveRowNumber++;
				break;

				case 1: //end-of-file record
					fscanf(fp,"%2X\n",&CC); //��ȡCC�ͻس������ļ�ָ��ָ����һ��
				break;
				
				case 2: //extended segment address record
					fscanf(fp,"%4X",&extended_segment_address);
					fscanf(fp,"%2X\n",&CC); //��ȡCC�ͻس������ļ�ָ��ָ����һ��
					extended_segment_address = extended_segment_address <<4;
				break;

				case 3: //Start Segment Address Record, ignored
					//ֱ�Ӷ����س�����
					ch=fgetc(fp); 
					while ((ch!=EOF) && (ch!='\n')) {ch = fgetc(fp);}
				break;
				
				case 4: //extended linear address record
					fscanf(fp,"%4X\n",&extended_linear_address);
					fscanf(fp,"%2X\n",&CC); //��ȡCC�ͻس������ļ�ָ��ָ����һ��
					extended_linear_address = extended_linear_address <<16;
				break;

				case 5: //Start Linear Address Record, ignored
					//ֱ�Ӷ����س�����
					ch=fgetc(fp); 
					while ((ch!=EOF) && (ch!='\n')) {ch = fgetc(fp);}
				break;
				}//end of switch (TT)
		 	}//end of while

			//�ر�hex�ļ�
			error=fclose(fp); 	
			if (error) 
			{	return -4; }

			//�����ļ�
			//SetCtrlVal (panel, TOPPANEL_NUM_ROW_CHECKSUM_ERR, checksum_error);
			//SetCtrlVal (panel, TOPPANEL_NUM_EXTLINADD, extended_linear_address); 
			//SetCtrlVal (panel, TOPPANEL_NUM_EXTSEGADD, extended_segment_address); 
			if ((checksum_error==0) && (EffectiveRowNumber<=MAX_ROWNUMBER) && (EffectiveRowNumber>0) && BeCompatibleWithADuCHEXFormat)
			{	return 0;  }
			else
			{	
				if (BeCompatibleWithADuCHEXFormat==0) 
				{	return -5;  }
				else  //ע�����(BeCompatibleWithADuCHEXFormat==0)��ô�Ͳ�����ļ��ͷ����ļ���Ҳ�Ͳ������checksum�������������Ĵ���
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
 float T_wait=0.0; //����ȴ�
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
				error = I2C_BYTEs_WRITE_DLL(iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//д������B2
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
				error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//д������B2
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
				error = I2C_BYTEs_WRITE_DLL (iIndex, device_addr, 123, 4, rom_value_arr, T_wait);//д������B2
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

			//��702x���ص�0x06����Ӧ������أ���Ȼ��һ��д����
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

			//���Ҫ����0x8A000-0x8F7FF�����EEPROM���ݣ���ˢ0x80000-0x89FFF����80*512 Byte��
			//���������0x80000-0x83FFF����16K�Ĵ���������32*512Byte����32 pages��
			//���������0x80000-0x89FFF����40K�Ĵ���������80*512Byte����80 pages��
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
			
			//��702x���ص�0x06����Ӧ������أ���Ȼ��һ��д����
			error = I2C_BYTE_CURRENT_ADDRESS_READ_DLL (iIndex, device_addr, &rom_value);
			if (error || (rom_value != 0x06)) return -2;

			return 0; 
}


int my_OnePage_WRITEorVERIFY(int iIndex, unsigned char WRITEorVERIFY, int CurrentPageNumner, unsigned char *Data_Array) //WRITEorVERIFY=1ʱ��ʾ��ǰ���µļ��ǡ�Verify Flash���� 
{  
 unsigned char Start_ID[2], Data[300]={0}, Checksum;
 int error, i;
 int CurrentDataNumber; 
 double T_wait=0.0; //����ȴ�
 int rom_addr, rom_Length;
 unsigned char rom_value, rom_value_arr[300];
 int device_addr, rom_startaddress, NoOfDataBytes;

		//if (Page_Array[CurrentPageNumner]==1) //����ҳ����Ч��־Ϊ1��д
		{   //SetCtrlVal (panel, TOPPANEL_TEXTBOX, "."); ProcessDrawEvents ();
		
		    device_addr = 0x04;
			Start_ID[0] = 0x07;
			Start_ID[1] = 0x0E;
			NoOfDataBytes = 5 + MAX_ONEPAGEDATANUMBER;//ÿҳ248�����ݣ�����CMD�͵�ַ��5��byte
			
			if (WRITEorVERIFY==0)//��ǰ���µļ��ǡ�Write Flash������
				Data[1] = 'W';  //CMD: "W"
			else				 //��ǰ���µļ��ǡ�Verify Flash����������8bit�ָ�3��5bit����
				Data[1] = 'V';  //CMD: "V"
			Data[2] = 0x00; //Address: h
			Data[3] = 0x00; //Address: u
			Data[4] = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER) / 256; //Address: m
			Data[5] = (CurrentPageNumner * MAX_ONEPAGEDATANUMBER) % 256; //Address: l
			for (i=0; i<MAX_ONEPAGEDATANUMBER; i++) 
			{	CurrentDataNumber = CurrentPageNumner * MAX_ONEPAGEDATANUMBER + i;
				if (WRITEorVERIFY==0)//��ǰ���µļ��ǡ�Write Flash������
					Data[6+i] = Data_Array[CurrentDataNumber];
				else				 //��ǰ���µļ��ǡ�Verify Flash����������8bit�ָ�3��5bit����
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
 			if (iIndex>=0x800) //��ЩDelay�Ǳ�Ҫ�ģ�����Ҫ����ֻ�в��ڲ����������Ų�����ʱ��
 				Delay(0.02);
 			if (iIndex<=0x0ff)
 				Delay(0.01);
			//��702x���ص�0x06����Ӧ������أ���Ȼ��һ��д����
			error = I2C_BYTE_CURRENT_ADDRESS_READ_DLL (iIndex, device_addr, &rom_value);
			if (error || (rom_value != 0x06)) return -2;
 			if (iIndex>=0x800)
 				Delay(0.01);
			
		}//end of "if (Page_Array[CurrentPageNumner]==1)"	
	  return 0;
}

int __stdcall WRITEorVERIFY_DLL(int iIndex, int RemainEEPROM, int WRITEorVERIFY, unsigned char *Data_Array) //WRITEorVERIFY=1ʱ��ʾ��ǰ���µļ��ǡ�Verify Flash����
{int CurrentPageNumner, CurrentDataNumber;
 unsigned char device_addr, rom_startaddress, rom_Length;
 unsigned char rom_value_arr[256];
 int error, rom_addr, i;
 unsigned char Start_ID[2], NoOfDataBytes, Data[300]={0}, Checksum;
 int ackerror=0;//equal to 0 means i2c operation normally
 double T_wait=0.0; //����ȴ� 
 char str[256]=""; 
 int Current_MAX_PAGENUMBER;
 unsigned int Page_Array[MAX_PAGENUMBER];	//��Чҳ��־�������ҳ������Ч���ݣ������1������=0;  
 
		//������Чҳ��־�������ҳ������Ч���ݣ������1������=0; ע��һҳ��248������
		for (i=0; i<MAX_PAGENUMBER; i++) Page_Array[i]=0;//��ʼ��������Чҳ��־Ϊ��Ч��
		for (CurrentPageNumner=0; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)
		{	for (i=0; i<MAX_ONEPAGEDATANUMBER; i++)
			{	CurrentDataNumber= CurrentPageNumner*MAX_ONEPAGEDATANUMBER+i;
				if (Data_Array[CurrentDataNumber]!=0xFF)//��ǰdata���ݲ�Ϊ0xff��˵����ҳ�����Ϸ�����
				{Page_Array[CurrentPageNumner]=1;//�ø�ҳ��־Ϊ��Ч 
				 break;
				}
			}
		}

		//���������0x80000-0x83FFF����16K������������32*512 Byte��
		//ע�⵽����firmwareʱ���ҹ涨һ��ҳд���������Ч����ֻ��248��byte
		//������Ҫ����ҳд�����Ĵ�����32*512/248=66.064516129032258064516129032258��
		//ȡ����66����ʣ��16��byteû��д�룬��0x83FF0-0x83FFFû��д�룻���޴󰭣���16byte�����ò�����
		//���Ǵ��������ϵ�������Чҳ��־���������㣬��ʾ����д�롣

		//���������0x80000-0x89FFF����40K�Ĵ���������80*512Byte��
		//ע�⵽����firmwareʱ���ҹ涨һ��ҳд���������Ч����ֻ��248��byte
		//������Ҫ����ҳд�����Ĵ�����80*512/248=165.16129032258064516129032258065��
		//ȡ����165����ʣ��40��byteû��д�룬��0x89FD8-0x89FFFû��д�룻���޴󰭣���40byte�����ò�����
		//���Ǵ��������ϵ�������Чҳ��־���������㣬��ʾ����д�롣
		
		if (RemainEEPROM)	
		{	for (CurrentPageNumner=165; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)
				Page_Array[CurrentPageNumner]=0;//�ø�ҳ��־Ϊ��Ч 
		}


		//��ҳ������д��FLASH������ҳ����Ч��־Ϊ1��д������д��
		CurrentPageNumner=0;
		CurrentDataNumber=0;
		//�Ȳ�д��0ҳ��ʹ80014h��ַ�����ݱ���Ϊ0xffffffff; ������д�����г����I2C�޷��ٴη���
		for (CurrentPageNumner=1; CurrentPageNumner<MAX_PAGENUMBER; CurrentPageNumner++)  
		{ if (Page_Array[CurrentPageNumner]==1) //����ҳ����Ч��־Ϊ1��д
  		  {
			error = my_OnePage_WRITEorVERIFY(iIndex, WRITEorVERIFY, CurrentPageNumner, Data_Array);
			if (error)
			{ //sprintf(str, "Error occur when attempt to operate the FLASH at page No.%d.\n", CurrentPageNumner);
			  //SetCtrlVal (panel, TOPPANEL_TEXTBOX, str); 
			  return -1;
			}
		  }
		}	
		//���д��0ҳ
		CurrentPageNumner=0; 
		{ if (Page_Array[CurrentPageNumner]==1) //����ҳ����Ч��־Ϊ1��д
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
//firmware�涨�������ȷ����A4.sStrA4.I2Ccommand=0������=1�����Ըú�����ȷִ�н�����0��
{
/*
  int error, i;
  int device_addr, reg_add, rom_Length;
  unsigned char rom_value_arr[256];
  //float T_wait=0.1;

		//д4��byte�������ֵ�A4.sStrA4.I2Ccommand��A4.sStrA4.I2Cslave_address��A4.sStrA4.I2Cslave_address��A4.sStrA4.I2Cslave_address
			A4.sStrA4.I2Ccommand       = 'R'; 

			device_addr = my_device_addr; 
			reg_add =(int)(&A4.sStrA4.I2Ccommand) - (int)(&A4.sStrA4.FirstByte); 
			rom_value_arr[reg_add] = A4.sStrA4.I2Ccommand;
			rom_value_arr[reg_add+1] = A4.sStrA4.I2Cslave_address;
			rom_value_arr[reg_add+2] = A4.sStrA4.I2Creg_start_add;
			rom_value_arr[reg_add+3] = A4.sStrA4.I2Creg_length;
			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, 4, rom_value_arr, 0.00);//дRAM�����Բ��ȴ�
			if (error<0) {return -1;}

			Delay(0.1); //��Ϊ���9.4*5=47ms�ͽ���IRQ�������Ķ�DS1856�����Ҫ10ms�����Ե�100ms�Ѿ��㹻7020���𲢽���I2C����
		
		//����I2Creg_length�����ݺ�ErrorCode 
 			error = I2C_BYTEs_READ_DLL (device_addr, 0, 30, A4.pStrA4);
			if (error<0) {return -1;}

	return A4.sStrA4.I2Ccommand;
*/
	return 0;
}

int __stdcall I2C_MasterWriteSlave_DLL (int iIndex, int my_device_addr) 
//firmware�涨�������ȷ����A4.sStrA4.I2Ccommand=0������=1�����Ըú�����ȷִ�н�����0�� 
{/*
  int error, i;
  int device_addr, reg_add, rom_Length;
  //float T_wait = 0.1;
  
		//��дI2Creg_length�����ݵ�A4.sStrA4.I2Creg_dat_arr[]
			device_addr = my_device_addr; 
			reg_add =(int)(&A4.sStrA4.I2Creg_dat_arr[0]) - (int)(&A4.sStrA4.FirstByte); 

			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, A4.sStrA4.I2Creg_length, A4.pStrA4, 0.00);//дRAM�����Բ��ȴ� 
			if (error<0) {return -1;}
			

		//д4��byte�������ֵ�A4.sStrA4.I2Ccommand��A4.sStrA4.I2Cslave_address��A4.sStrA4.I2Cslave_address��A4.sStrA4.I2Cslave_address
			A4.sStrA4.I2Ccommand       = 'W'; 
			reg_add =(int)(&A4.sStrA4.I2Ccommand) - (int)(&A4.sStrA4.FirstByte); 

			error = I2C_BYTEs_WRITE_DLL (device_addr, reg_add, 4, A4.pStrA4, 0.00); //дRAM�����Բ��ȴ�
			if (error<0) {return -1;}
													 
			Delay(0.1); //��Ϊ���9.4*5=47ms�ͽ���IRQ��������дDS1856�����Ҫ10ms�����Ե�100ms�Ѿ��㹻7020���𲢽���I2C����
			
		//����I2Creg_length�����ݺ�ErrorCode 
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

int __stdcall GetF320Numbers(int *lpdwNumDevices) //��ȡ��ǰ����PC��F320������
{
	SI_STATUS  status;

	status = SI_GetNumDevices(lpdwNumDevices); 
	
	return  status;
}

int __stdcall F320ExecuteCmd_DLL(int iIndex, unsigned char iCommand, int iWriteLength, int iReadLength, unsigned char *ioBuffer)
{
	return F320ExecuteCmd(iIndex, iCommand, iWriteLength, iReadLength, ioBuffer); 
}

int __stdcall GetF320SerialNumber(int iIndex, unsigned char *SN) //��ȡ��ǰ����PC��F320��SN
{unsigned char  ioBuffer[300];
 int error;

	error =  F320ExecuteCmd(iIndex, GET_SN, 0, 16, SN);
	SN[16]=0;
	
	return error;
}

int __stdcall SetF320SerialNumber(int iIndex, unsigned char *SN) //���õ�ǰ����PC��F320��SN
{unsigned char  ioBuffer[300];

	return F320ExecuteCmd(iIndex, SET_SN, 16, 0, SN);  
}

int __stdcall F320_SPI_Init_DLL(int iIndex, int SPI_Rate, int SCK_PHASE, int SCK_POLARITY) 
//F320ExecuteCmd(0, SPI_RST, 3, 0, ioArray) ��λ����������SPI�����ʣ���λ������ 
//ioArray[0]�����ʣ����㹫ʽSCK = 12000/( ioArray[0]+1)K����ioArray[0] = 119����SPI����Ϊ100K
//ioArray[1]����λ��0����SCK���ڵĵ�һ�����ز������ݣ�1����SCK���ڵĵڶ������ز�������
//ioArray[1]�����ԣ�0��SCK�ڿ���״̬ʱ���ڵ͵�ƽ��1��SCK�ڿ���״̬ʱ���ڸߵ�ƽ
//Ĭ������Ϊ100K����һ�����ز������ݣ�SCK�ڿ���״̬ʱ���ڵ͵�ƽ
{unsigned char  ioBuffer[300];

	ioBuffer[0] = SPI_Rate;
	ioBuffer[1] = SCK_PHASE;
	ioBuffer[2] = SCK_POLARITY;
	return F320ExecuteCmd(iIndex, SPI_RST, 3, 0, ioBuffer);
}

int __stdcall SetF320_FPGA_PROGB_DLL(int iIndex, int PROGB) 
//F320ExecuteCmd(0, SPI_NSS, 1, 0, NSS);����NSS��ƽ��0���͵�ƽ��1���ߵ�ƽ
{unsigned char  ioBuffer[300];

	ioBuffer[0] = PROGB;
	return F320ExecuteCmd(iIndex, SET_PG, 1, 0, ioBuffer);
}

int __stdcall SetF320_SPI_NSS_DLL(int iIndex, int NSS) 
//F320ExecuteCmd(0, SPI_NSS, 1, 0, NSS);����NSS��ƽ��0���͵�ƽ��1���ߵ�ƽ
{unsigned char  ioBuffer[300];

	ioBuffer[0] = NSS;
	return F320ExecuteCmd(iIndex, SPI_NSS, 1, 0, ioBuffer);
}

int __stdcall SetF320_SPI_CS9_CS0_DLL(int iIndex, int CS9_CS0) 
//F320ExecuteCmd(iIndex, SET_CS, 1, 0, CS9_CS0);����CS9_CS0��ƽ��CS9_CS0��bit9..bit0�ֱ��ӦGPIO��CS9_CS0��ƽ
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
//    ����������1�����ݣ����ݽ������£�
//    0x00��ѡ�������Ƶ��115.52MHz
//    0x01: ѡ�������Ƶ��161.13281MHz
//    0x02: ѡ�������Ƶ��167.33165MHz
//    0x03: ѡ�������Ƶ��173.37075MHz
//    ����������Ч��������һ��Ƶ��ѡ����
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
//    ����������һ�����ݣ����ݽ������£�
//    ���ݵ�bit0������OLT��Դ�Ŀ��ϣ�1Ϊ�򿪣�0Ϊ�ض�
//    ���ݵ�bit1������ONU��Դ�Ŀ��ϣ�1Ϊ�򿪣�0Ϊ�ض�
//    ����bit��Ч��Ĭ��ȫ������
{unsigned char  ioBuffer[300];
 int error;

	ioBuffer[0]= VCC;
	error = F320ExecuteCmd(iIndex, SDN_VCC, 1, 0, ioBuffer); 
	return error;
}

int __stdcall SEL_IIC_F320_DLL (int iIndex, unsigned char IIC)
//    ����������һ�����ݣ����ݽ������£�
//    ���ݵ�bit0��OLT IIC����ʹ�ܣ�1Ϊʹ�ܣ�0��ֹ
//    ���ݵ�bit1��ONU1 IIC����ʹ�ܣ�1Ϊʹ�ܣ�0��ֹ
//    ���ݵ�bit2��BERT IIC����ʹ�ܣ�1Ϊʹ�ܣ�0��ֹ
//    ���ݵ�bit3��ARM IIC����ʹ�ܣ�1Ϊʹ�ܣ�0��ֹ
//    ����bit��Ч��Ĭ��ȫ��ʹ��

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
 
	ioBuffer[0] = rom_startaddress;   //д���ݵĵ�һ���ֽڱ���Ϊ�׵�ַ
	for (i=0; i<rom_Length; i++)
		ioBuffer[i+1] = rom_value_arr[rom_startaddress+i];
		
	status = F320StreamEPP(iIndex-0x800, EPP_W, rom_Length, 0, ioBuffer); //iDataLenΪҪ��������ʵ�ʳ��ȣ�֧��256�ֽڲ��� 
	
	return status;
}

int __stdcall EPP_BYTEs_READ_F320_DLL (int iIndex, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr)
{SI_STATUS  status;
 unsigned char ioBuffer[300];
 int i;
 
	ioBuffer[0] = rom_startaddress;   //д���ݵĵ�һ���ֽڱ���Ϊ�׵�ַ
	for (i=0; i<rom_Length; i++)
		ioBuffer[i+1] = rom_value_arr[rom_startaddress+i];
		
	status = F320StreamEPP(iIndex-0x800, EPP_R, 0, rom_Length, ioBuffer); ////iDataLenΪҪ��������ʵ�ʳ��ȣ�֧��256�ֽڲ���

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

int __stdcall IIC_Uart_Open(int iIndex,int rom_Length,BYTE *rom_value_arr) //�򿪴���
{
	unsigned char  ioBuffer[300];
	if (iIndex<0) return -1;
		//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
		memcpy(ioBuffer, rom_value_arr, rom_Length); 
	    return F320ExecuteCmd(iIndex, IIC_UART_OPEN, rom_Length, 0, ioBuffer); 
	}
	return -1; 
}

int __stdcall IIC_Uart_Close(int iIndex,int rom_Length,BYTE *rom_value_arr) //�رմ���
{
	unsigned char  ioBuffer[300];
	if (iIndex<0) return -1;
		//F320
	if ( (iIndex>=0x800) && (iIndex<=0x8FF) )	
	{	
		iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
		memcpy(ioBuffer, rom_value_arr, rom_Length); 
	    return F320ExecuteCmd(iIndex, IIC_UART_CLOSE, rom_Length, 0, ioBuffer);
	}
	return -1;  
}

int __stdcall IIC_Uart_Read (int iIndex, int imode,int Read_Length, int *BytesReturned, unsigned char *rom_value_arr)  //������
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
			iIndex = iIndex-0x800; //��Ϊ��ȥ0x800���ƫ����
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
