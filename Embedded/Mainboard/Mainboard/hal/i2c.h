/*
 * i2c.h
 *
 * Created: 12/11/2020 9:11:06 AM
 *  Author: JThiessen
 */ 


#ifndef I2C_H_
#define I2C_H_

#define TWI0_BAUD(F_SCL, T_RISE)                        ((((((float)F_CPU / (float)F_SCL)) - 10 - ((float)F_CPU * T_RISE))) / 2)

#define I2C_SCL_FREQ                                    100000

#define I2C_DIRECTION_BIT_WRITE                         0
#define I2C_DIRECTION_BIT_READ                          1
#define I2C_SET_ADDR_POSITION(address)                  (address << 1)
#define I2C_SLAVE_RESPONSE_ACKED                        (!(TWI_RXACK_bm & TWI0.MSTATUS))
#define I2C_DATA_RECEIVED                               (TWI_RIF_bm & TWI0.MSTATUS)

#define CREATE_16BIT_VARIABLE(HIGH_BYTE, LOW_BYTE)      ((HIGH_BYTE << 8) | (LOW_BYTE & 0xFF))

void	I2C_0_init(void);

void	I2C_0_transmittingAddrPacket(uint8_t slaveAddres, uint8_t directionBit);
uint8_t	I2C_0_receivingDataPacket(void);
void	I2C_0_sendMasterCommand(uint8_t newCommand);
void	I2C_0_setACKAction(void);
void	I2C_0_setNACKAction(void);

int8_t	I2C_0_sendData(uint8_t address, uint8_t *pData, uint8_t len); // returns how many bytes have been sent, -1 means NACK at address
int8_t	I2C_0_getData(uint8_t address, uint8_t *pData, uint8_t len); // returns how many bytes have been received, -1 means NACK at address
void	I2C_0_endSession(void);

void	I2C_0_scan(uint8_t addr_min, uint8_t addr_max);

#endif /* I2C_H_ */