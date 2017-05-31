/*USB תI2C �豸��������ͷ�ļ�
����������������������������������������������������������
����������������������������������������������������������
*/
/*
extern unsigned int iIndex;   // ָ��CH341 �豸���
extern unsigned int iMode;	// ָ��ģʽ,������
					// λ1 λ0: I2C �ٶ�/SCL Ƶ��, 00=����20KHz,01=��׼100KHz,10=����400KHz,11=����750KHz
					// λ2: SPI ��I/O ��/IO ����, 0=���뵥��(4 �߽ӿ�),1=˫��˫��(5 �߽ӿ�)
					// ��������,����Ϊ0
*/
extern int InitialUSB(ULONG iIndex, ULONG iMode);
extern int CloseUSB(ULONG iIndex);
extern int I2C_SLAVE_SEARCH_USB (int iIndex, ULONG iMode, int device_addr);

extern int I2C_BYTE_CURRENT_ADDRESS_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, BYTE *rom_value);
//������
extern int I2C_BYTE_READ_USB (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE *rom_value);
//�����
extern int I2C_BYTEs_READ_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr);
extern int I2C_BYTEs_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, int rom_Length, BYTE *rom_value_arr, float T_wait);
//ע�⣬I2C_BYTEs_WRITE_USB()������ֻ֧�����8byte��д�� ��ʼ��ֵַrom_startaddressӦ����8�ı�����
//����������rom_Length���ֵ��8�����ң�rom_value_arr[256]��Ҫȫ����������
//��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������ݡ�
extern int I2C_BYTE_WRITE_USB  (ULONG iIndex, ULONG iMode, int device_addr, int rom_startaddress, BYTE rom_value, float T_wait);
//ע�⣬I2C_BYTEs_READ_USB()������rom_value_arr[256]��Ҫȫ����������
//��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������ݡ�


