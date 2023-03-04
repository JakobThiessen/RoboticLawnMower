/*
 * i2c.c
 *
 * Created: 12/11/2020 9:10:52 AM
 *  Author: JThiessen
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "i2c.h"

enum {
	I2C_INIT = 0,
	I2C_ACKED,
	I2C_NACKED,
	I2C_READY,
	I2C_ERROR
};

void I2C_0_init(enum PORTMUX_TWI0_enum pinmux, uint32_t baudrate)
{	
	/* Select I2C pins default --> PA2 (SDA)/PA3 (SCL) */
	PORTMUX.TWIROUTEA = pinmux;
	    
	/* Master Baud Rate Control */
	TWI0.MBAUD = TWI_BAUD((baudrate), 0);
	    
	/* Enable TWI */
	TWI0.MCTRLA = TWI_ENABLE_bm;
	    
	/* Initialize the address register */
	TWI0.MADDR = 0x00;
	    
	/* Initialize the data register */
	TWI0.MDATA = 0x00;
	    
	/* Set bus state idle */
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

void I2C_0_transmittingAddrPacket(uint8_t slaveAddres, uint8_t directionBit)
{
	TWI0.MADDR = I2C_SET_ADDR_POSITION(slaveAddres) + directionBit;
	while (!I2C_0_SLAVE_RESPONSE_ACKED)
	{
		;
	}
}

uint8_t I2C_0_receivingDataPacket(void)
{
	while (!I2C_0_DATA_RECEIVED)
	{
		;
	}

	return TWI0.MDATA;
}

void I2C_0_sendMasterCommand(uint8_t newCommand)
{
	TWI0.MCTRLB |=  newCommand;
}

void I2C_0_setACKAction(void)
{
	TWI0.MCTRLB &= !TWI_ACKACT_bm;
}

void I2C_0_setNACKAction(void)
{
	TWI0.MCTRLB |= TWI_ACKACT_bm;
}

uint8_t i2c_0_WaitW(void)
{
	uint8_t state = I2C_INIT;
	do
	{
		if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			if(!(TWI0.MSTATUS & TWI_RXACK_bm))
			{
				/* slave responded with ack - TWI goes to M1 state */
				state = I2C_ACKED;
			}
			else
			{
				/* address sent but no ack received - TWI goes to M3 state */
				state = I2C_NACKED;
			}
		}
		else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

uint8_t i2c_0_WaitR(void)
{
	uint8_t state = I2C_INIT;
	
	do
	{
		if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			state = I2C_READY;
		}
		else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

/* Returns how many bytes have been sent, -1 means NACK at address, 0 means slave ACKed to slave address */
int8_t I2C_0_sendData(uint8_t address, uint8_t *pData, uint8_t len)
{
	int8_t retVal = -1;
	 
	/* start transmitting the slave address */
	TWI0.MADDR = I2C_SET_ADDR_POSITION(address) & ~0x01;
	if(i2c_0_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData != NULL))
	{
		while(len--)
		{
			TWI0.MDATA = *pData;
			if(i2c_0_WaitW() == I2C_ACKED)
			{
				pData++;
				continue;
			}
			else // did not get ACK after slave address
			{
				retVal = -1;
				break;
			}
		}
	}
	 
	return retVal;
}
 
/* Returns how many bytes have been received, -1 means NACK at address */
int8_t I2C_0_getData(uint8_t address, uint8_t *pData, uint8_t len)
{
	int8_t retVal = -1;
	 
	/* start transmitting the slave address */
	TWI0.MADDR = I2C_SET_ADDR_POSITION(address) | 0x01;
	if(i2c_0_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData !=NULL ))
	{
		while(len--)
		{
			if(i2c_0_WaitR() == I2C_READY)
			{
				*pData = TWI0.MDATA;
				TWI0.MCTRLB = (len == 0)? TWI_ACKACT_bm | TWI_MCMD_STOP_gc : TWI_MCMD_RECVTRANS_gc;
				pData++;
				continue;
			}
			else
			break;
		}
	}
	 
	return retVal;
}

void I2C_0_endSession(void)
{
	TWI0.MCTRLB = TWI_MCMD_STOP_gc;
}
 
 /* Scans for I2C slave devices on the bus - that have an address within the 
 *specified range [addr_min, addr_max]
 */
void I2C_0_scan(uint8_t addr_min, uint8_t addr_max)
{
    uint8_t slave_address, ret;
    printf("\n\r I2C_0 Scan started from 0x%02X to 0x%02X:\n\r", addr_min, addr_max);
    for (slave_address = addr_min; slave_address <= addr_max; slave_address++)
    {
		//printf(".");
        ret = I2C_0_sendData(slave_address, NULL, 0);
        I2C_0_endSession();
        if(ret != 0xFF)
        {
			printf("Scanning slave address = 0x%02X", (int)slave_address);
            printf(" --> slave ACKED\n\r");
        }
    }
    printf("\n\r I2C Scan ended\n\r");
}



/************************************************************************/
/* TWI 1                                                                */
/************************************************************************/

void I2C_1_init(enum PORTMUX_TWI1_enum pinmux, uint32_t baudrate)
{	
	/* Select I2C pins default --> PA2 (SDA)/PA3 (SCL) */
	PORTMUX.TWIROUTEA = pinmux;
	    
	/* Master Baud Rate Control */
	TWI1.MBAUD = TWI_BAUD(baudrate, 0);
	    
	/* Enable TWI */
	TWI1.MCTRLA = TWI_ENABLE_bm;
	    
	/* Initialize the address register */
	TWI1.MADDR = 0x00;
	    
	/* Initialize the data register */
	TWI1.MDATA = 0x00;
	    
	/* Set bus state idle */
	TWI1.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

void I2C_1_transmittingAddrPacket(uint8_t slaveAddres, uint8_t directionBit)
{
	TWI1.MADDR = I2C_SET_ADDR_POSITION(slaveAddres) + directionBit;
	while (!I2C_1_SLAVE_RESPONSE_ACKED)
	{
		;
	}
}

uint8_t I2C_1_receivingDataPacket(void)
{
	while (!I2C_1_DATA_RECEIVED)
	{
		;
	}

	return TWI1.MDATA;
}

void I2C_1_sendMasterCommand(uint8_t newCommand)
{
	TWI1.MCTRLB |=  newCommand;
}

void I2C_1_setACKAction(void)
{
	TWI1.MCTRLB &= !TWI_ACKACT_bm;
}

void I2C_1_setNACKAction(void)
{
	TWI1.MCTRLB |= TWI_ACKACT_bm;
}

uint8_t I2C_1_WaitW(void)
{
	uint8_t state = I2C_INIT;
	do
	{
		if(TWI1.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			if(!(TWI1.MSTATUS & TWI_RXACK_bm))
			{
				/* slave responded with ack - TWI goes to M1 state */
				state = I2C_ACKED;
			}
			else
			{
				/* address sent but no ack received - TWI goes to M3 state */
				state = I2C_NACKED;
			}
		}
		else if(TWI1.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

uint8_t I2C_1_WaitR(void)
{
	uint8_t state = I2C_INIT;
	
	do
	{
		if(TWI1.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
		{
			state = I2C_READY;
		}
		else if(TWI1.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
		{
			/* get here only in case of bus error or arbitration lost - M4 state */
			state = I2C_ERROR;
		}
	} while(!state);
	
	return state;
}

/* Returns how many bytes have been sent, -1 means NACK at address, 0 means slave ACKed to slave address */
int8_t I2C_1_sendData(uint8_t address, uint8_t *pData, uint8_t len)
{
	int8_t retVal = -1;
	 
	/* start transmitting the slave address */
	TWI1.MADDR = I2C_SET_ADDR_POSITION(address) & ~0x01;
	if(I2C_1_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData != NULL))
	{
		while(len--)
		{
			TWI1.MDATA = *pData;
			if(I2C_1_WaitW() == I2C_ACKED)
			{
				pData++;
				continue;
			}
			else // did not get ACK after slave address
			{
				retVal = -1;
				break;
			}
		}
	}
	 
	return retVal;
}
 
/* Returns how many bytes have been received, -1 means NACK at address */
int8_t I2C_1_getData(uint8_t address, uint8_t *pData, uint8_t len)
{
	int8_t retVal = -1;
	 
	/* start transmitting the slave address */
	TWI1.MADDR = I2C_SET_ADDR_POSITION(address) | 0x01;
	if(I2C_1_WaitW() != I2C_ACKED)
	return retVal;

	retVal = 0;
	if((len != 0) && (pData !=NULL ))
	{
		while(len--)
		{
			if(I2C_1_WaitR() == I2C_READY)
			{
				*pData = TWI1.MDATA;
				TWI1.MCTRLB = (len == 0)? TWI_ACKACT_bm | TWI_MCMD_STOP_gc : TWI_MCMD_RECVTRANS_gc;
				pData++;
				continue;
			}
			else
			break;
		}
	}
	 
	return retVal;
}

void I2C_1_endSession(void)
{
	TWI1.MCTRLB = TWI_MCMD_STOP_gc;
}
 
 /* Scans for I2C slave devices on the bus - that have an address within the 
 *specified range [addr_min, addr_max]
 */
void I2C_1_scan(uint8_t addr_min, uint8_t addr_max)
{
    uint8_t slave_address, ret;
    printf("\n\r I2C_1 Scan started from 0x%02X to 0x%02X:\n\r", addr_min, addr_max);
    for (slave_address = addr_min; slave_address <= addr_max; slave_address++)
    {
		//printf(".");
        ret = I2C_1_sendData(slave_address, NULL, 0);
        I2C_1_endSession();
        if(ret != 0xFF)
        {
			printf("Scanning slave address = 0x%02X", (int)slave_address);
            printf(" --> slave ACKED\n\r");
        }
    }
    printf("\n\r I2C Scan ended\n\r");
}