/*LPTorUSB 转I2C 设备驱动程序头文件*/
#include "CH341DLL.H" 
#include "USB_I2C_DEVICE.h" 


extern int PCHost_I2C_BYTE_CURRENT_ADDRESS_READ (int iIndex, int device_addr, BYTE *rom_value);
//单字节立即读

extern int PCHost_I2C_BYTE_READ (int iIndex, int device_addr, int rom_StartAddress, BYTE *rom_value); 
//单字节指定地址读

extern int PCHost_I2C_BYTEs_READ (int iIndex, int device_addr, int rom_StartAddress, int rom_Length, BYTE *rom_value_arr);
//多字节指定首地址连续读
//注意，rom_value_arr[]数组要全部传进去，仅更新rom_value_arr[rom_StartAddress]到rom_value_arr[rom_StartAddress+rom_Length-1]这段数据

extern int PCHost_I2C_BYTE_WRITE (int iIndex, int device_addr, int rom_StartAddress, BYTE rom_value, float T_wait); 
//单字节指定地址写

extern int PCHost_I2C_BYTEs_WRITE (int iIndex, int device_addr, int rom_StartAddress, int rom_Length, BYTE *rom_value_arr, float T_wait);
//多字节指定首地址连续写 
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_StartAddress]到rom_value_arr[rom_StartAddress+rom_Length-1]这段数据。
//注意，对于EEPROM，一般地只能8byte连写；而对于7020，一般就没有最大长度的限制。

//extern int PCHost_I2C_TwoBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_StartAddress]到rom_value_arr[rom_StartAddress+1]这段数据。

//extern int PCHost_I2C_FourBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_StartAddress]到rom_value_arr[rom_StartAddress+3]这段数据。

//extern int PCHost_I2C_EightBYTE_WRITE (int device_addr, int rom_StartAddress, BYTE *rom_value_arr, float T_wait);
//注意，rom_value_arr[]要全部传进去，仅更新rom_value_arr[rom_StartAddress]到rom_value_arr[rom_StartAddress+7]这段数据。

