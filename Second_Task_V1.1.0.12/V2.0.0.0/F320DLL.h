
#define MCU_RST    0x00
#define GET_FVER   0x01
#define GET_DVER   0x02

#define SET_SN     0x20
#define GET_SN     0x21

#define IIC_RST    0x80
#define IIC_SCH    0x81
#define IIC_R      0x82
#define IIC_W      0x83

#define SPI_RST    0xA0
#define SPI_NSS    0xA1
#define SPI_R      0xA2
#define SPI_W      0xA3
#define SPI_RW     0xA4

extern  HANDLE F320OpenDevice(ULONG iIndex);
extern  BOOL   F320CloseDevice(HANDLE UsbHandle);
extern  BOOL   F320StreamSPI(ULONG iIndex, ULONG iRWLength, PVOID iBuffer, PVOID oBuffer);
extern  BOOL   F320StreamI2C(ULONG iIndex, ULONG iWriteLength, PVOID iWriteBuffer, ULONG iReadLength, PVOID oReadBuffer);
extern  BOOL   F320ExecuteCmd(ULONG iIndex, BYTE iCommand, ULONG iWriteLength, ULONG iReadLength, PVOID ioBuffer);

