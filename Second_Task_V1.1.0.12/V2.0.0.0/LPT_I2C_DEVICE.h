/*LPT 转I2C 设备驱动程序头文件
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
*/
extern int InitialLPT(int LPTHandle);
extern int I2C_SLAVE_SEARCH_LPT (int LPTHandle, int device_addr);
extern int I2C_BYTE_CURRENT_ADDRESS_READ_LPT (int LPTHandle, int device_addr, unsigned char *rom_value);
extern int I2C_BYTE_READ_LPT (int LPTHandle, int device_addr, int rom_startaddress, unsigned char *rom_value);
extern int I2C_BYTEs_READ_LPT (int LPTHandle, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr);
extern int I2C_BYTEs_WRITE_LPT  (int LPTHandle, int device_addr, int rom_startaddress, int rom_Length, unsigned char  *rom_value_arr, float T_wait);
extern int I2C_BYTE_WRITE_LPT  (int LPTHandle, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait);
