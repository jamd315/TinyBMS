#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>

// Ports, pins, settings
#define I2C_DDR		DDRB
#define I2C_PORT	PORTB
#define I2C_PIN		PINB
#define I2C_SDA		PB0
#define I2C_SCL		PB2

#define I2C_ADDRESS		0x22
#define I2C_BUFFER_LEN	4

// Function prototypes
void I2C_init(void);
uint8_t I2C_read(void);
void I2C_write(uint8_t data);
void I2C_flush(void);
bool I2C_available(void);

// Enums
typedef enum 
{
	CHECK_ADDRESS					= 0,
	SEND_DATA						= 1,
	REQUEST_REPLY_FROM_SEND_DATA	= 2,
	CHECK_REPLY_FROM_SEND_DATA		= 3,
	REQUEST_DATA					= 4,
	GET_DATA_AND_SEND_ACK			= 5
} I2C_overflow_state_t;

#endif /* I2C_H_ */