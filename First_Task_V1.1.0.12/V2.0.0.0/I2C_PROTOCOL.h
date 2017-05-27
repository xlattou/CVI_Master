/************** Static Function Declarations **************/

/************** Global Variable Declarations **************/
/************** Global Function Declarations **************/
extern void I2C_START(int LPTHandle);
extern unsigned char I2C_GET_ACK(int LPTHandle);
extern void I2C_WRITE_WORD_VALUE(int LPTHandle, unsigned char rom_value);
extern unsigned char I2C_READ_WORD_VALUE(int LPTHandle);
extern void I2C_SET_ACK(int LPTHandle);
extern void I2C_SET_NONE_ACK(int LPTHandle);
extern void I2C_STOP(int LPTHandle);


extern int I2C_BYTE_CURRENT_ADDRESS_READ (int LPTHandle, unsigned char device_addr, unsigned char *rom_value);
extern int I2C_BYTE_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value);
extern int I2C_2BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1);
extern int I2C_4BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3);
extern int I2C_8BYTEs_READ (int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char *rom_value0, unsigned char *rom_value1, unsigned char *rom_value2, unsigned char *rom_value3, 
			unsigned char *rom_value4, unsigned char *rom_value5, unsigned char *rom_value6, unsigned char *rom_value7);
extern int I2C_BYTEs_READ  (int LPTHandle, unsigned char device_addr, unsigned char rom_startAddress, int rom_Length, unsigned char *rom_value_arr);

extern int I2C_BYTE_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_addr, unsigned char rom_value, float T_wait);
extern int I2C_2BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_addr, 
					unsigned char rom_value1, unsigned char rom_value2, float T_wait);
extern int I2C_4BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_startAddress, 
     unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, float T_wait);
extern int I2C_8BYTEs_WRITE(int LPTHandle, unsigned char device_addr, unsigned char rom_startAddress, 
     unsigned char rom_value1, unsigned char rom_value2, unsigned char rom_value3, unsigned char rom_value4, 
     unsigned char rom_value5, unsigned char rom_value6, unsigned char rom_value7, unsigned char rom_value8, float T_wait);
extern int I2C_BYTEs_WRITE (int LPTHandle, unsigned char device_addr, unsigned char rom_startAddress, int rom_Length, unsigned char *rom_value_arr, float T_wait);


