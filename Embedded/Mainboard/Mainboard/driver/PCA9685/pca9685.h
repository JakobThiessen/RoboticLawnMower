/*!
 *  @file PCA9685
 *
 */
 
#ifndef _PCA9685_H
#define _PCA9685_H


// REGISTER ADDRESSES
#define PCA9685_MODE_1			0x00	/**< Mode Register 1 */
#define PCA9685_MODE_2			0x01	/**< Mode Register 2 */
#define PCA9685_SUBADR_1		0x02	/**< I2C-bus subaddress 1 */
#define PCA9685_SUBADR_2		0x03	/**< I2C-bus subaddress 2 */
#define PCA9685_SUBADR_3		0x04	/**< I2C-bus subaddress 3 */
#define PCA9685_ALLCALLADR		0x05	/**< LED All Call I2C-bus address */

#define PCA9685_LED_0_ON_L		0x06	/**< LED0 on tick, low byte*/
#define PCA9685_LED_0_ON_H		0x07	/**< LED0 on tick, high byte*/
#define PCA9685_LED_0_OFF_L		0x08	/**< LED0 off tick, low byte */
#define PCA9685_LED_0_OFF_H		0x09	/**< LED0 off tick, high byte */

#define PCA9685_LED_1_ON_L		0x0A
#define PCA9685_LED_1_ON_H		0x0B
#define PCA9685_LED_1_OFF_L		0x0C
#define PCA9685_LED_1_OFF_H		0x0D
#define PCA9685_LED_2_ON_L		0x0E
#define PCA9685_LED_2_ON_H		0x0F
#define PCA9685_LED_2_OFF_L		0x10
#define PCA9685_LED_2_OFF_H		0x11
#define PCA9685_LED_3_ON_L		0x12
#define PCA9685_LED_3_ON_H		0x13
#define PCA9685_LED_3_OFF_L		0x14
#define PCA9685_LED_3_OFF_H		0x15
#define PCA9685_LED_4_ON_L		0x16
#define PCA9685_LED_4_ON_H		0x17
#define PCA9685_LED_4_OFF_L		0x18
#define PCA9685_LED_4_OFF_H		0x19
#define PCA9685_LED_5_ON_L		0x1A
#define PCA9685_LED_5_ON_H		0x1B
#define PCA9685_LED_5_OFF_L		0x1C
#define PCA9685_LED_5_OFF_H		0x1D
#define PCA9685_LED_6_ON_L		0x1E
#define PCA9685_LED_6_ON_H		0x1F
#define PCA9685_LED_6_OFF_L		0x20
#define PCA9685_LED_6_OFF_H		0x21
#define PCA9685_LED_7_ON_L		0x22
#define PCA9685_LED_7_ON_H		0x23
#define PCA9685_LED_7_OFF_L		0x24
#define PCA9685_LED_7_OFF_H		0x25
#define PCA9685_LED_8_ON_L		0x26
#define PCA9685_LED_8_ON_H		0x27
#define PCA9685_LED_8_OFF_L		0x28
#define PCA9685_LED_8_OFF_H		0x29
#define PCA9685_LED_9_ON_L		0x2A
#define PCA9685_LED_9_ON_H		0x2B
#define PCA9685_LED_9_OFF_L		0x2C
#define PCA9685_LED_9_OFF_H		0x2D
#define PCA9685_LED_10_ON_L		0x2E
#define PCA9685_LED_10_ON_H		0x2F
#define PCA9685_LED_10_OFF_L	0x30
#define PCA9685_LED_10_OFF_H	0x31
#define PCA9685_LED_11_ON_L		0x32
#define PCA9685_LED_11_ON_H		0x33
#define PCA9685_LED_11_OFF_L	0x34
#define PCA9685_LED_11_OFF_H	0x35
#define PCA9685_LED_12_ON_L		0x36
#define PCA9685_LED_12_ON_H		0x37
#define PCA9685_LED_12_OFF_L	0x38
#define PCA9685_LED_12_OFF_H	0x39
#define PCA9685_LED_13_ON_L		0x3A
#define PCA9685_LED_13_ON_H		0x3B
#define PCA9685_LED_13_OFF_L	0x3C
#define PCA9685_LED_13_OFF_H	0x3D
#define PCA9685_LED_14_ON_L		0x3E
#define PCA9685_LED_14_ON_H		0x3F
#define PCA9685_LED_14_OFF_L	0x40
#define PCA9685_LED_14_OFF_H	0x41
#define PCA9685_LED_15_ON_L		0x42
#define PCA9685_LED_15_ON_H		0x43
#define PCA9685_LED_15_OFF_L	0x44
#define PCA9685_LED_15_OFF_H	0x45

#define PCA9685_ALLLED_ON_L		0xFA	/**< load all the LEDn_ON registers, low */
#define PCA9685_ALLLED_ON_H		0xFB	/**< load all the LEDn_ON registers, high */
#define PCA9685_ALLLED_OFF_L	0xFC	/**< load all the LEDn_OFF registers, low */
#define PCA9685_ALLLED_OFF_H	0xFD	/**< load all the LEDn_OFF registers,high */
#define PCA9685_PRESCALE		0xFE	/**< Prescaler for PWM output frequency */
#define PCA9685_TESTMODE		0xFF	/**< defines the test mode to be entered */

// MODE1 bits
#define MODE_1_ALLCAL			0x01	/**< respond to LED All Call I2C-bus address */
#define MODE_1_SUB_3			0x02	/**< respond to I2C-bus subaddress 3 */
#define MODE_1_SUB_2			0x04	/**< respond to I2C-bus subaddress 2 */
#define MODE_1_SUB_1			0x08	/**< respond to I2C-bus subaddress 1 */
#define MODE_1_SLEEP			0x10	/**< Low power mode. Oscillator off */
#define MODE_1_AI				0x20	/**< Auto-Increment enabled */
#define MODE_1_EXTCLK			0x40	/**< Use EXTCLK pin clock */
#define MODE_1_RESTART			0x80	/**< Restart enabled */

// MODE2 bits
#define MODE_2_OUTNE_0			0x01	/**< Active LOW output enable input */
#define MODE_2_OUTNE_1			0x02	/**< Active LOW output enable input - high impedance */
#define MODE_2_OUTDRV			0x04	/**< totem pole structure vs open-drain */
#define MODE_2_OCH				0x08	/**< Outputs change on ACK vs STOP */
#define MODE_2_INVRT			0x10	/**< Output logic state inverted */

#define PCA9685_I2C_ADDRESS_0	0x40	/**< Default PCA9685 I2C Slave Address */
#define PCA9685_I2C_ADDRESS_1	0x41	/**< Default PCA9685 I2C Slave Address */
#define PCA9685_I2C_ADDRESS_2	0x42	/**< Default PCA9685 I2C Slave Address */
#define PCA9685_I2C_ADDRESS_3	0x43	/**< Default PCA9685 I2C Slave Address */
#define PCA9685_I2C_ADDRESS_4	0x44	/**< Default PCA9685 I2C Slave Address */

#define FREQUENCY_OSCILLATOR	25000000	/**< Int. osc. frequency in datasheet */

#define PCA9685_PRESCALE_MIN	3	/**< minimum prescale value --> 1526Hz maximum PWM frequency */
#define PCA9685_PRESCALE_MAX	255	/**< maximum prescale value --> 24Hz minimum PWM frequency */

#define PCA9685_FREQ_MIN	24		/**< minimum PWM frequency*/
#define PCA9685_FREQ_MAX	1526	/**< maximum PWM frequency*/


/*****************************************************************************/
/* type definitions */

typedef int8_t (*pca9685_read_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*pca9685_write_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef void (*pca9685_delay_fptr_t)(uint32_t period);

struct pca9685_dev
{
    /*! Chip Id */
    uint8_t chip_id;

    /*! Device Id */
    uint8_t id;
	
	/*! Device I2C Address */
	uint8_t i2c_addr;
	
	/*! internally tracked oscillator used for freq calculations*/
	uint32_t oscillator_freq;

	/*! result of read/write function*/
	int8_t rslt;

    /*! Read function pointer */
    pca9685_read_fptr_t read;

    /*! Write function pointer */
    pca9685_write_fptr_t write;

    /*!  Delay function pointer */
    pca9685_delay_fptr_t delay_ms;
};

void pca9685_init(struct pca9685_dev *dev);
void pca9685_setPin(uint8_t num, uint16_t val, bool invert, struct pca9685_dev *dev);
void pca9685_getPWM(uint8_t num, uint16_t *on, uint16_t *off, struct pca9685_dev *dev);
void pca9685_setPWM(uint8_t num, uint16_t on, uint16_t off, struct pca9685_dev *dev);

void pca9685_reset(struct pca9685_dev *dev);
void pca9685_sleep(struct pca9685_dev *dev);
void pca9685_wakeup(struct pca9685_dev *dev);
void pca9685_setExtClk(uint8_t prescale, struct pca9685_dev *dev);
void pca9685_setPWMFreq(float freq, struct pca9685_dev *dev);
void pca9685_setOutputMode(bool totempole, struct pca9685_dev *dev);
void pca9685_readPrescale(uint8_t *prescale, struct pca9685_dev *dev);
void pca9685_writeMicroseconds(uint8_t num, uint16_t Microseconds, struct pca9685_dev *dev);

uint8_t pca9685_getModeReg(uint8_t mode_reg, struct pca9685_dev *dev);
uint8_t pca9685_setModeReg(uint8_t mode_reg, uint8_t data, struct pca9685_dev *dev);

#endif