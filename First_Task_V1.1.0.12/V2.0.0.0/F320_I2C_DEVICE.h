/*F320 תI2C �豸��������ͷ�ļ�
����������������������������������������������������������
����������������������������������������������������������
*/


#define  NUM_MAX_BYTES_BUF 280

typedef struct _USB_PACKET
{
	unsigned char op_command;
	unsigned char error_code;
	unsigned char dat_length[2];
	unsigned char io_dat[NUM_MAX_BYTES_BUF];
}USB_PACKET, *pUSB_PACKET;


#define MCU_RST    0x00
#define GET_FVER   0x01
#define GET_DVER   0x02

#define EPP_R      0x03	   //20090909
#define EPP_W      0x04	   //20090909 

#define SET_SN     0x20
#define GET_SN     0x21

#define SET_TM     0x40  //evb5
#define GET_TM     0x41  //evb5
#define SET_FS     0x42  //evb5 & evb11
#define SEL_SPIIIC 0x43  //evb5
#define SDN_VCC    0x44  //evb11
#define SEL_IIC    0x45  //evb11

#define IIC_RST    0x80
#define IIC_SCH    0x81
#define IIC_R      0x82
#define IIC_W      0x83

#define IIC_UART_OPEN  0x90
#define IIC_UART_CLOSE 0x91
#define IIC_UART_READ  0x92
#define SPI_RST    0xA0
#define SPI_NSS    0xA1
#define SPI_R      0xA2
#define SPI_W      0xA3
#define SPI_RW     0xA4
#define SPI_LDPG   0xA5
#define SPI_RDPG   0xA6

#define SPI_WREE   0xB0  
#define SPI_RDEE   0xB1

#define CLK_SEL    0xC0  	// ʱ��ͨ���л�����,����SEVB0027-0002ʱ���ź��л��� 
#define GET_CLK_CH 0xC1 	// ��ȡ��ǰ���õ�ʱ��ͨ��

#define SET_PG     0x44
#define SET_CS     0x43

#define HEADER_LENGTH (sizeof(USB_PACKET) - NUM_MAX_BYTES_BUF)

#define dwTimeout 2000

#define C2_RST     0xC0
#define C2_GETID   0xC1
#define C2_INIT    0xC2
#define C2_ERSCP   0xC3
#define C2_ERSPG   0xC4
#define C2_RDPG    0xC5
#define C2_WRPG    0xC6

#define PDI_ENABLE 0x31 // ʹ��PDI��ʹ���M��PDI����ģʽ��
#define PDI_DISABLE 0x32 // ����PDI��ʹ���M����������ģʽ��
#define PDI_GET_DEVICE_ID 0x33 // ͨ��PDI��ȡMCUʶ���
#define PDI_EARSE_NVM_PAGE 0x34 // ����NVM flashָ�����
#define PDI_ERASE_NVM_ALL 0x35 // ����NVM flash���е��
#define PDI_WRITE_NVM 0x36  // ��NVM flashָ�����_ʼ��ַд����
#define PDI_READ_NVM 0x37 // ��NVM flashָ���Ŀ�ʼ��ַ������
#define PDI_ERASE_EEPROM_PAGE 0x38 // ����EEPROMָ�����
#define PDI_ERASE_EEPROM_ALL 0x39 // ����EEPROM���е��
#define PDI_WRITE_EEPROM 0x3A // ��EEPROMָ�����_ʼ��ַд����
#define PDI_READ_EEPROM 0x3B // ��EEPROMָ���Ŀ�ʼ��ַ������
#define PDI_GET_DEVICE_BOD 0x3D     // ��FUSE����
#define PDI_WRITE_DEVICE_BOD 0x3C   // дBOD���� 


int I2C_SLAVE_SEARCH_F320 (int iIndex, int device_addr);
//��I2C���豸
extern int I2C_BYTE_CURRENT_ADDRESS_READ_F320 (int iIndex, int device_addr, unsigned char *rom_value);
//������
extern int I2C_BYTE_READ_F320 (int iIndex, int device_addr, int rom_startaddress, unsigned char *rom_value);
//�����
extern int I2C_BYTEs_READ_F320  (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr);
//ָ����ַ�ͳ��ȶ�

int I2C_BYTE_CURRENT_ADDRESS_WRITE_F320 (int iIndex, int device_addr, int rom_startaddress, float T_wait);
//����д��ֻдdevice_addr��rom_startaddress
extern int I2C_BYTEs_WRITE_F320  (int iIndex, int device_addr, int rom_startaddress, int rom_Length, unsigned char *rom_value_arr, float T_wait);
//ע�⣬I2C_BYTEs_WRITE_F320()������ֻ֧�����8byte��д�� ��ʼ��ֵַrom_startaddressӦ����8�ı�����
//����������rom_Length���ֵ��8�����ң�rom_value_arr[256]��Ҫȫ����������
//��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������ݡ�
extern int I2C_BYTE_WRITE_F320  (int iIndex, int device_addr, int rom_startaddress, unsigned char rom_value, float T_wait);
//ע�⣬I2C_BYTEs_READ_F320()������rom_value_arr[256]��Ҫȫ����������
//��������rom_value_arr[rom_startaddress]��rom_value_arr[rom_startaddress+rom_Length-1]������ݡ�

extern int SPI_BYTEs_READWRITE_F320 (int iIndex, int iRWLength, unsigned char *iBuffer, unsigned char *oBuffer);
extern int F320ExecuteCmd(ULONG iIndex, BYTE iCommand, ULONG iWriteLength, ULONG iReadLength, PVOID ioBuffer);
extern int F320StreamEPP(ULONG iIndex, BYTE iCommand, ULONG iWriteLength, ULONG iReadLength, PVOID ioBuffer); 

extern int DLLVersion;
