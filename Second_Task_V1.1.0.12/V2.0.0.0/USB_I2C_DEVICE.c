#include <ansi_c.h>  
#include <windows.h>    
#include <userint.h>
#include <utility.h>    
#include "CH341DLL.H"  


int OpenUSB(ULONG iIndex)
{	if(CH341OpenDevice(iIndex) == INVALID_HANDLE_VALUE)
	{	CH341ResetDevice(iIndex);				// ��λUSB
		CH341SetTimeout(iIndex, 0x2710, 0x2710);	// ָ��USB��ʱʱ��,1S

		if(CH341OpenDevice(iIndex) == INVALID_HANDLE_VALUE)
			return -1;  
	}		
	return 0; 
}


int CloseUSB(ULONG iIndex)
{
	CH341CloseDevice(iIndex);
	return 0;	
}


int InitialUSB(ULONG iIndex, ULONG iMode)
{

	if(OpenUSB(iIndex)<0)//open USB error
		return -1;  
	
	if (!CH341SetStream(iIndex,iMode))			//����ͨѶ����
	{	CH341ResetDevice(iIndex);				// ��λUSB
		CH341SetTimeout(iIndex, 0x2710, 0x2710);	// ָ��USB��ʱʱ��,1S
		if(OpenUSB(iIndex)<0)//open USB error
			return -1;
		if (!CH341SetStream(iIndex,iMode))			//����ͨѶ����
			return -1;  
	}
	
	return 0;
}

int I2C_SLAVE_SEARCH_USB (int iIndex, ULONG iMode, int device_addr)
//û�ҵ�slave�ͷ���-1�����򷵻�0 
{	int existed;
	unsigned char OutBuf[300], InBuf[300]; 
	int Status;
	
  		if (iIndex<0) return -1;
		existed =0;
				InitialUSB(iIndex, iMode); //��USB��� 
					OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
					OutBuf[1] = 0;	//reg_add = 0
					Status = CH341StreamI2C(iIndex,2,OutBuf,0,InBuf); 	//дI2C��������ַ��ƫ�Ƶ�ַ
					OutBuf[0] ++;					//e.g. A1,for I2C read 
					Status = CH341StreamI2C(iIndex,1,OutBuf,2,InBuf);	//restart����2byte����
					
					if ( (Status == 1) && ((InBuf[0] != 0xff) || (InBuf[1] != 0xff)) )   //found
					{
				    	existed =0;
					} 
					else
					{
				    	existed =-1;
					} 
				CloseUSB(iIndex);
		
		return existed;
}


int I2C_BYTE_CURRENT_ADDRESS_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, BYTE *rom_value)
//������
{
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE + 0x1;	//e.g. A1,for I2C read
	Status = CH341StreamI2C(iIndex,1,OutBuf,1,InBuf); 	//дI2C��������ַ��ƫ�Ƶ�ַ
	if (Status==0) { return -1; }
	*rom_value = InBuf[0];
	
 	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
	
  return 0;
}


int I2C_BYTEs_READ_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr)
//ע�⣬rom_value_arr[256]��Ҫȫ������������������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]�������
{
	BYTE rom_value;
 	int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 	int error, Status, i;
 	unsigned char OutBuf[300], InBuf[300];  
 
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	Status = CH341StreamI2C(iIndex,2,OutBuf,0,InBuf); 	//дI2C��������ַ��ƫ�Ƶ�ַ
	OutBuf[0] ++;					//e.g. A1,for I2C read 
	Status = CH341StreamI2C(iIndex,1,OutBuf,rom_Length,InBuf);	//restart��������
	if (Status==0) { return -1; }


	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
	
	for (i=0; i<rom_Length; i++)
		rom_value_arr[rom_startaddress + i] = InBuf[i];
  
  return 0;
}

int I2C_BYTE_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE *rom_value)
{int error;
 unsigned char rom_value_arr[256];
 
	error = I2C_BYTEs_READ_USB  (iIndex, 1, device_addr, rom_startaddress, 1, rom_value_arr);
	*rom_value = rom_value_arr[rom_startaddress];

  return error;
}


int I2C_BYTEs_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr, float T_wait)
//ע�⣬ֻ֧�����8byte��д�� ��ʼ��ֵַӦ����8�ı���
//ע�⣬rom_value_arr[256]��Ҫȫ������������������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������� 
{BYTE rom_value;
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300]; 
 
 	
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	for (i=0;i<rom_Length;i++)
		OutBuf[i+2] = rom_value_arr[rom_startaddress+i];
	Status = CH341StreamI2C(iIndex,rom_Length+2,OutBuf,0,InBuf);
	if (Status<0) { return -1; }
	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
    Delay(T_wait);
  
  return 0;
}

int I2C_BYTE_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE rom_value, float T_wait)
{
 int rom_addr; // if define rom_addr as BYTE, the   for (rom_addr=0; rom_addr<=0xff; rom_addr++)  will be a dead-circle
 int error, Status, i;
 unsigned char OutBuf[300], InBuf[300];  
 
 	
	error = InitialUSB(iIndex, iMode);
	if(error<0) { return -1; }
	
	OutBuf[0] = device_addr&0xFE;	//e.g. A0,for I2C write
	OutBuf[1] = rom_startaddress;	//reg_add = rom_startaddress
	OutBuf[2] = rom_value;
	Status = CH341StreamI2C(iIndex,3,OutBuf,0,InBuf);
	if (Status<0) { return -1; }
	error = CloseUSB(iIndex);	
	if(error<0) { return -1; }
    Delay(T_wait);
  
  return 0;
}








int __stdcall InitialUSB_DLL(int iIndex) //��USB���
{
 return InitialUSB(iIndex, 1);
}

int __stdcall CloseUSB_DLL(int iIndex) //�ر�USB���
{if (iIndex<0) return -1;
 return CloseUSB(iIndex);
}


int __stdcall GetUSBHostDescr(  // ��ȡ�豸������
				int	iIndex,  // ָ��CH341�豸���
				unsigned char *OutBuf) // ָ��һ���㹻��Ļ�����,���ڱ���������
{int length=10;
 int error;
    if (iIndex<0) return -1;
	
	//InitialUSB(iIndex, 1); //��USB���
	if (CH341GetDeviceDescr(iIndex,  OutBuf, &length)) 
		error=0;
	else
		error=-1;
	//CloseUSB(iIndex); //�ر�USB���     
	
	return error;
}



int __stdcall GetUSBHostConfigDescr(  // ��ȡ����������
				int	iIndex,  // ָ��CH341�豸���
				unsigned char *OutBuf)  // ָ��һ���㹻��Ļ�����,���ڱ���������
{int length=10;
 int error;
    if (iIndex<0) return -1;

	//InitialUSB(iIndex, 1); //��USB���
	if (CH341GetConfigDescr(iIndex,  OutBuf, &length)) 
		error=0;
	else
		error=-1;
	//CloseUSB(iIndex); //�ر�USB���     

	return error;
}
				
int __stdcall USBHost_GetStatus_DLL (int iIndex, unsigned int *iStatus)
//
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //��USB���
	error = CH341GetStatus(iIndex, iStatus);
	//CloseUSB(iIndex); //�ر�USB���     
 return error;        	
}

int __stdcall USBHost_GetInput_DLL (int iIndex, unsigned int *iStatus)
// λ7-λ0��ӦCH341��D7-D0����
// λ8��ӦCH341��ERR#����, λ9��ӦCH341��PEMP����, λ10��ӦCH341��INT#����, λ11��ӦCH341��SLCT����, λ23��ӦCH341��SDA����
// λ13��ӦCH341��BUSY/WAIT#����, λ14��ӦCH341��AUTOFD#/DATAS#����,λ15��ӦCH341��SLCTIN#/ADDRS#����
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //��USB���
	error = CH341GetInput(iIndex, iStatus);
	//CloseUSB(iIndex); //�ر�USB���     
	
	return  error;       	
}

int __stdcall USBHost_SetD5D0_DLL (int iIndex, int iSetDirOut, int iSetDataOut)
// ����D5-D0�����ŵ�I/O����,ĳλ��0���Ӧ����Ϊ����,ĳλ��1���Ӧ����Ϊ���,���ڷ�ʽ��Ĭ��ֵΪ0x00ȫ������
// ����D5-D0�����ŵ��������,���I/O����Ϊ���,��ôĳλ��0ʱ��Ӧ��������͵�ƽ,ĳλ��1ʱ��Ӧ��������ߵ�ƽ
{int error;

	if (iIndex<0) return -1;
	//InitialUSB(iIndex, 1); //��USB���
	error = CH341Set_D5_D0(iIndex,  iSetDirOut,  iSetDataOut );
	//CloseUSB(iIndex); //�ر�USB���     
	
	return  error;       	
}

int __stdcall USBHost_ResetDevice_DLL(int iIndex)
{
	if (iIndex<0) return -1;
	return CH341ResetDevice(iIndex);
}
