/*LPTorUSB תI2C �豸��������ͷ�ļ�*/
#include "CH341DLL.H" 
#include "USB_I2C_DEVICE.h" 


extern int PCHost_I2C_BYTE_CURRENT_ADDRESS_READ (int iIndex, int device_addr, BYTE *rom_value);
//���ֽ�������

extern int PCHost_I2C_BYTE_READ (int iIndex, int device_addr, int rom_StartAddress, BYTE *rom_value); 
//���ֽ�ָ����ַ��

extern int PCHost_I2C_BYTEs_READ (int iIndex, int device_addr, int rom_StartAddress, int rom_Length, BYTE *rom_value_arr);
//���ֽ�ָ���׵�ַ������
//ע�⣬rom_value_arr[]����Ҫȫ������ȥ��������rom_value_arr[rom_StartAddress]��rom_value_arr[rom_StartAddress+rom_Length-1]�������

extern int PCHost_I2C_BYTE_WRITE (int iIndex, int device_addr, int rom_StartAddress, BYTE rom_value, float T_wait); 
//���ֽ�ָ����ַд

extern int PCHost_I2C_BYTEs_WRITE (int iIndex, int device_addr, int rom_StartAddress, int rom_Length, BYTE *rom_value_arr, float T_wait);
//���ֽ�ָ���׵�ַ����д 
//ע�⣬rom_value_arr[]Ҫȫ������ȥ��������rom_value_arr[rom_StartAddress]��rom_value_arr[rom_StartAddress+rom_Length-1]������ݡ�
//ע�⣬����EEPROM��һ���ֻ��8byte��д��������7020��һ���û����󳤶ȵ����ơ�

//extern int PCHost_I2C_TwoBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//ע�⣬rom_value_arr[]Ҫȫ������ȥ��������rom_value_arr[rom_StartAddress]��rom_value_arr[rom_StartAddress+1]������ݡ�

//extern int PCHost_I2C_FourBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//ע�⣬rom_value_arr[]Ҫȫ������ȥ��������rom_value_arr[rom_StartAddress]��rom_value_arr[rom_StartAddress+3]������ݡ�

//extern int PCHost_I2C_EightBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//ע�⣬rom_value_arr[]Ҫȫ������ȥ��������rom_value_arr[rom_StartAddress]��rom_value_arr[rom_StartAddress+7]������ݡ�

