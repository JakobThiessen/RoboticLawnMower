/*!
*  @file pca9685.cpp
*
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pca9685.h"

/*!
*  @brief  Setups the I2C interface and hardware
*/
void pca9685_init(struct pca9685_dev *dev)
{	
	// set the default internal frequency
	dev->oscillator_freq = FREQUENCY_OSCILLATOR;
	
	pca9685_reset(dev);

	// set a default frequency
	pca9685_setPWMFreq(1000, dev);
	
	pca9685_setModeReg(PCA9685_MODE_1, MODE_1_AI, dev);
	
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_ALLCALLADR, (uint8_t*)&dev->chip_id, 1);
}

/*!
*   @brief  Helper to set pin PWM output. Sets pin without having to deal with
* on/off tick placement and properly handles a zero value as completely off and
* 4095 as completely on.  Optional invert parameter supports inverting the
* pulse for sinking to ground.
*   @param  num One of the PWM output pins, from 0 to 15
*   @param  val The number of ticks out of 4096 to be active, should be a value
* from 0 to 4095 inclusive.
*   @param  invert If true, inverts the output, defaults to 'false'
*/
void pca9685_setPin(uint8_t num, uint16_t val, bool invert, struct pca9685_dev *dev)
{
	// Clamp value between 0 and 4095 inclusive.
	if(val > 4095)	val = 4095;
	
	if (invert)
	{
		if (val == 0)
		{
			// Special value for signal fully on.
			pca9685_setPWM(num, 4096, 0, dev);
		}
		else if (val == 4095)
		{
			// Special value for signal fully off.
			pca9685_setPWM(num, 0, 4096, dev);
		}
		else
		{
			pca9685_setPWM(num, 0, 4095 - val, dev);
		}
	}
	else
	{
		if (val == 4095)
		{
			// Special value for signal fully on.
			pca9685_setPWM(num, 4096, 0, dev);
		}
		else if (val == 0)
		{
			// Special value for signal fully off.
			pca9685_setPWM(num, 0, 4096, dev);
		}
		else
		{
			pca9685_setPWM(num, 0, val, dev);
		}
	}
}

/*!
*  @brief  Sends a reset command to the PCA9685 chip over I2C
*/
void pca9685_reset(struct pca9685_dev *dev)
{
	uint8_t data = MODE_1_RESTART;
	dev->rslt = dev->write(dev->i2c_addr, PCA9685_MODE_1, &data, 1);
	dev->delay_ms(10);
}

/*!
*  @brief  Puts board into sleep mode
*/
void pca9685_sleep(struct pca9685_dev *dev)
{
	uint8_t awake = 0;
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_MODE_1, &awake, 1);
	
	uint8_t sleep = awake | MODE_1_SLEEP;	// set sleep bit high
	
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &sleep, 1);
	dev->delay_ms(5);						// wait until cycle ends for sleep to be active
}

/*!
*  @brief  Wakes board from sleep
*/
void pca9685_wakeup(struct pca9685_dev *dev)
{
	uint8_t sleep = 0;
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_MODE_1, &sleep, 1);
	uint8_t wakeup = sleep & ~MODE_1_SLEEP; // set sleep bit low
		
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &wakeup, 1);
}

/*!
*  
*/
void pca9685_setExtClk(uint8_t prescale, struct pca9685_dev *dev)
{
	uint8_t oldmode = 0;
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_MODE_1, &oldmode, 1);
	
	uint8_t newmode = (oldmode & ~MODE_1_RESTART) | MODE_1_SLEEP; // sleep
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &newmode, 1); // go to sleep, turn off internal oscillator

	// This sets both the SLEEP and EXTCLK bits of the MODE1 register to switch to
	// use the external clock.
	newmode = (newmode |= MODE_1_EXTCLK);
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &newmode, 1);

	dev->rslt += dev->write(dev->i2c_addr, PCA9685_PRESCALE, &prescale, 1); // set the prescaler

	dev->delay_ms(5);
	
	// clear the SLEEP bit to start
	newmode = (newmode & ~MODE_1_SLEEP) | MODE_1_RESTART | MODE_1_AI;
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &newmode, 1);
}

/*!
*  @brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
*  @param  freq Floating point frequency that we will attempt to match
*/
void pca9685_setPWMFreq(float freq, struct pca9685_dev *dev)
{
	uint8_t oldmode = 0;
	uint8_t newmode = 0;
	
	// Range output modulation frequency is dependant on oscillator
	if (freq < PCA9685_FREQ_MIN)	freq = PCA9685_FREQ_MIN;	// Datasheet limit
	if (freq > PCA9685_FREQ_MAX)	freq = PCA9685_FREQ_MAX;	// Datasheet limit

	float prescaleval = ((dev->oscillator_freq / (freq * 4096.0)) + 0.5) - 1;
	
	if (prescaleval < PCA9685_PRESCALE_MIN)	prescaleval = PCA9685_PRESCALE_MIN;
	if (prescaleval > PCA9685_PRESCALE_MAX)	prescaleval = PCA9685_PRESCALE_MAX;
	
	uint8_t prescale = (uint8_t)prescaleval;
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_MODE_1, (uint8_t*)&oldmode, 1);
	
	newmode = (oldmode & ~MODE_1_RESTART) | MODE_1_SLEEP; // sleep
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &newmode, 1); // go to sleep
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_PRESCALE, &prescale, 1); // set the prescaler
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &oldmode, 1); //
		
	dev->delay_ms(5);
		
	// This sets the MODE1 register to turn on auto increment.
	oldmode = (oldmode & ~MODE_1_RESTART) | MODE_1_AI;
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_1, &oldmode, 1);
}

/*!
*  @brief  Sets the output mode of the PCA9685 to either
*  open drain or push pull / totempole.
*  Warning: LEDs with integrated zener diodes should
*  only be driven in open drain mode.
*  @param  totempole Totempole if true, open drain if false.
*/
void pca9685_setOutputMode(bool totempole, struct pca9685_dev *dev)
{
	uint8_t oldmode = 0;
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_MODE_2, (uint8_t*)&oldmode, 1);
	
	uint8_t newmode;
	
	if (totempole)
	{
		newmode = oldmode | MODE_2_OUTDRV;
	}
	else
	{
		newmode = oldmode & ~MODE_2_OUTDRV;
	}
	
	dev->rslt += dev->write(dev->i2c_addr, PCA9685_MODE_2, &newmode, 1);
}

/*!
*  @brief  Reads set Prescale from PCA9685
*  @return prescale value
*/
void pca9685_readPrescale(uint8_t *prescale, struct pca9685_dev *dev)
{
	dev->rslt = dev->read(dev->i2c_addr, PCA9685_PRESCALE, prescale, 1);
}

/*!
*  @brief  Gets the PWM output of one of the PCA9685 pins
*  @param  num One of the PWM output pins, from 0 to 15
*  @return requested PWM output value
*/
void pca9685_getPWM(uint8_t num, uint16_t *on, uint16_t *off, struct pca9685_dev *dev)
{
	uint8_t data[4];
	uint8_t reg_addr = (PCA9685_LED_0_ON_L + 4 * num);

	dev->rslt = dev->read(dev->i2c_addr, reg_addr, (uint8_t*)&data, 4);
	*on = (uint16_t)data[0];
	*on |= (uint16_t)data[1] << 8;
	*off = (uint16_t)data[2];
	*off |= (uint16_t)data[3] << 8;
}

/*!
*  @brief  Sets the PWM output of one of the PCA9685 pins
*  @param  num One of the PWM output pins, from 0 to 15
*  @param  on At what point in the 4096-part cycle to turn the PWM output ON
*  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
*  @return result from endTransmission
*/
void pca9685_setPWM(uint8_t num, uint16_t on, uint16_t off, struct pca9685_dev *dev)
{
	uint8_t data[4];
	uint8_t reg_addr = (PCA9685_LED_0_ON_L + 4 * num);
	
	data[0] = (uint8_t)(on & 0x00FF);
	data[1] = (uint8_t)(on >> 8);
	data[2] = (uint8_t)(off & 0x00FF);
	data[3] = (uint8_t)(off >> 8);
	
	dev->rslt = dev->write(dev->i2c_addr, reg_addr, (uint8_t*)&data, 4);
}

/*!
*  @brief  Sets the PWM output of one of the PCA9685 pins based on the input
* microseconds, output is not precise
*  @param  num One of the PWM output pins, from 0 to 15
*  @param  Microseconds The number of Microseconds to turn the PWM output ON
*/
void pca9685_writeMicroseconds(uint8_t num, uint16_t Microseconds, struct pca9685_dev *dev)
{
	double pulse = Microseconds;
	double pulselength;
	pulselength = 1000000; 	// 1,000,000 us per second

	// Read prescale
	uint8_t prescale = 0;
	pca9685_readPrescale(&prescale, dev);

	// Calculate the pulse for PWM based on Equation 1 from the datasheet section
	// 7.3.5
	prescale += 1;
	pulselength *= prescale;
	pulselength /= dev->oscillator_freq;
	pulse /= pulselength;

	pca9685_setPWM(num, 0, pulse, dev);
}

uint8_t pca9685_getModeReg(uint8_t mode_reg, struct pca9685_dev *dev)
{
	uint8_t data = 0;
	
	dev->rslt = dev->read(dev->i2c_addr, mode_reg, (uint8_t*)&data, 1);
	
	return data;
}

uint8_t pca9685_setModeReg(uint8_t mode_reg, uint8_t data, struct pca9685_dev *dev)
{
	dev->rslt = dev->write(dev->i2c_addr, mode_reg, (uint8_t*)&data, 1);
}