/*USB 转I2C 设备驱动程序头文件
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
*/
/*
extern unsigned int iIndex;   // 指定CH341 设备序号
extern unsigned int iMode;	// 指定模式,见下行
					// 位1 位0: I2C 速度/SCL 频率, 00=低速20KHz,01=标准100KHz,10=快速400KHz,11=高速750KHz
					// 位2: SPI 的I/O 数/IO 引脚, 0=单入单出(4 线接口),1=双入双出(5 线接口)
					// 其它保留,必须为0
*/
extern int InitialUSB(ULONG iIndex, ULONG iMode);
extern int CloseUSB(ULONG iIndex);
extern int I2C_SLAVE_SEARCH_USB (int iIndex, ULONG iMode, int device_addr);

extern int I2C_BYTE_CURRENT_ADDRESS_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, BYTE *rom_value);
//立即读
extern int I2C_BYTE_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE *rom_value);
//随机读
extern int I2C_BYTEs_READ_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr);
extern int I2C_BYTEs_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr, float T_wait);
//注意，I2C_BYTEs_WRITE_USB()函数，只支持最大8byte连写， 起始地址值rom_startaddress应该是8的倍数，
//数据流长度rom_Length最大值是8。并且，rom_value_arr[256]，要全部传进来，
//但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。
extern int I2C_BYTE_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE rom_value, float T_wait);
//注意，I2C_BYTEs_READ_USB()函数，rom_value_arr[256]，要全部传进来，
//但仅更新rom_value_arr[rom_startaddress]到rom_value_arr[rom_startaddress+rom_Length-1]这段数据。


