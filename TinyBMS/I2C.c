/*
Code inspired by AVR312 example code.  Updated to be GCC-AVR/Atmel Studio compatible, fixed bugs, and rewrote so that I get a better understanding of AVR312, USI, and TWI/I2C.

TODO a real ring buffer would be neat to implement
*/
#include <avr/interrupt.h>
#include <stdbool.h>
#include "I2C.h"

// Static and inline function and variable declarations
static inline void SET_USI_TO_SEND_ACK(void);
static inline void SET_USI_TO_READ_ACK(void);
static inline void SET_USI_TO_TWI_START_CONDITION_MODE(void);
static inline void SET_USI_TO_SEND_DATA(void);
static inline void SET_USI_TO_READ_DATA(void);

static I2C_overflow_state_t I2C_overflow_state;
static uint8_t RX_buf[I2C_BUFFER_LEN];
static volatile uint8_t RX_head;
static volatile uint8_t RX_tail;
static uint8_t TX_buf[I2C_BUFFER_LEN];
static volatile uint8_t TX_head;
static volatile uint8_t TX_tail;



// Debug stuff

#define F_CPU 8000000UL
#include <util/delay.h>

static inline void flash(void)
{
	PORTB |= (1 << PB3);
	_delay_ms(10);
	PORTB &= ~(1 << PB3);
	_delay_ms(10);
}


// End debug stuff



void I2C_init(void)  // Start as a slave
{
	I2C_flush();
	I2C_DDR &= ~(1 << I2C_SDA);  // SDA Input
	I2C_PORT |= (1 << I2C_SDA);
	I2C_DDR |= (1 << I2C_SCL);   // SCL Output
	I2C_PORT |= (1 << I2C_SCL);  
	USICR = 
		(1 << USISIE) |  // Start condition interrupt enabled
		(0 << USIOIE) |  // Counter overflow interrupt disabled
		(1 << USIWM1) |  // Two wire mode, no overflow hold
		(0 << USIWM0) |
		(1 << USICS1) |  // Set clock to external positive edge
		(0 << USICS0) |
		(0 << USICLK) |
		(0 << USITC);    // Don't toggle the clock pin
	USISR =
		(1 << USISIF)  |  // Clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(0 << USICNT3) |  // Clear the counter
		(0 << USICNT2) |
		(0 << USICNT1) |
		(0 << USICNT0);
}

uint8_t I2C_read(void)
{
	while(RX_head == RX_tail);  // Wait for something to be received
	RX_tail = (RX_tail + 1) & I2C_BUFFER_LEN;
	return RX_buf[RX_head];
}

void I2C_write(uint8_t data)
{
	uint8_t tmp_TX_head = (TX_head + 1) & I2C_BUFFER_LEN;
	while(tmp_TX_head == TX_tail);  // Wait for there to be room in the buffer for another byte
	TX_buf[tmp_TX_head] = data;
	TX_head = tmp_TX_head;
}

void I2C_flush(void)
{
	RX_head = 0;
	RX_tail = 0;
	TX_head = 0;
	TX_tail = 0;
}

bool I2C_available(void)
{
	return (RX_head != RX_tail);
}

ISR(USI_START_vect)
{
	flash();  // Never goes off
	I2C_DDR &= ~(1 << I2C_SDA);  // Set SDA to input
	while( (I2C_PIN & (1 << I2C_SCL)) & !(USISR & (1 << USIPF)) );  // Wait for SCL to go low after triggering the start condition, unless there's a stop condition  TODO I don't like the stop condition check here
	USICR =
		(1 << USISIE) |  // Start condition interrupt enabled
		(1 << USIOIE) |  // Counter overflow interrupt enabled now
		(1 << USIWM1) |  // Two wire mode, overflow hold enabled
		(1 << USIWM0) |
		(1 << USICS1) |  // Set clock to external positive edge
		(0 << USICS0) |
		(0 << USICLK) |
		(0 << USITC);    // Don't toggle the clock pin
	USISR = 
		(1 << USISIF)  |  // Clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(0 << USICNT3) |  // Clear the counter
		(0 << USICNT2) |
		(0 << USICNT1) |
		(0 << USICNT0);
}

ISR(USI_OVF_vect)
{
	switch(I2C_overflow_state)
	{
		case CHECK_ADDRESS:
			if((USIDR == 0) || ((USIDR >> 1) == I2C_ADDRESS))  // Check address.  Right shift because the lowest bit is the data direction bit
			{
				if(USIDR & 0x01)  // Data direction bit says to write
				{
					I2C_overflow_state = SEND_DATA;
				}
				else  // Data direction bit says to read
				{
					I2C_overflow_state = REQUEST_DATA;
					SET_USI_TO_SEND_ACK();
				}
			}
			else
			{
				SET_USI_TO_TWI_START_CONDITION_MODE();
			}
			break;
		
		case CHECK_REPLY_FROM_SEND_DATA:
			if(USIDR)  // If NACK, the master does not want more data
			{
				SET_USI_TO_TWI_START_CONDITION_MODE();
				return;
			}
			// Go to SEND_DATA now without breaking
			
		case SEND_DATA:
			if(TX_head != TX_tail)  // If the buffer has contents, then load the contents into USIDR
			{
				TX_tail = (TX_tail + 1) & I2C_BUFFER_LEN;
				USIDR = TX_buf[TX_tail];
				I2C_overflow_state = REQUEST_REPLY_FROM_SEND_DATA;
				SET_USI_TO_SEND_ACK();
			}
			else  // Empty buffer, just reset
			{
				SET_USI_TO_TWI_START_CONDITION_MODE();
				return;
			}
			break;
			
		case REQUEST_REPLY_FROM_SEND_DATA:
			I2C_overflow_state = CHECK_REPLY_FROM_SEND_DATA;
			SET_USI_TO_READ_ACK();
			break;
		
		case REQUEST_DATA:
			I2C_overflow_state = GET_DATA_AND_SEND_ACK;
			SET_USI_TO_READ_ACK();
			break;
			
		case GET_DATA_AND_SEND_ACK:
			RX_head = (RX_head + 1) & I2C_BUFFER_LEN;
			RX_buf[RX_head] = USIDR;
			I2C_overflow_state = REQUEST_DATA;
			SET_USI_TO_SEND_ACK();
			break;
	}
}

// The following were preprocessor function macros in the original AVR312
static inline void SET_USI_TO_SEND_ACK(void)
{
	USIDR = 0;
	I2C_DDR |= (1 << I2C_SDA);  // SDA as output
	USISR =
		(0 << USISIF)  |  // DON'T clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(1 << USICNT3) |  // Prepare counter to shift 1 bit
		(1 << USICNT2) |
		(1 << USICNT1) |
		(0 << USICNT0);
}

static inline void SET_USI_TO_READ_ACK(void)
{
	I2C_DDR &= ~(1 << I2C_SDA);  // SDA as input
	USIDR = 0;
	USISR =
		(0 << USISIF)  |  // DON'T clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(1 << USICNT3) |  // Prepare counter to shift 1 bit
		(1 << USICNT2) |
		(1 << USICNT1) |
		(0 << USICNT0);
}

static inline void SET_USI_TO_TWI_START_CONDITION_MODE(void)
{
	USICR =
		(1 << USISIE) |  // Start condition interrupt enabled
		(0 << USIOIE) |  // Counter overflow interrupt disabled
		(1 << USIWM1) |  // Two wire mode, no overflow hold
		(0 << USIWM0) |
		(1 << USICS1) |  // Set clock to external positive edge
		(0 << USICS0) |
		(0 << USICLK) |
		(0 << USITC);    // Don't toggle the clock pin
	USISR =
		(0 << USISIF)  |  // DON'T clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(1 << USICNT3) |  // Prepare counter to shift 1 bit
		(1 << USICNT2) |
		(1 << USICNT1) |
		(0 << USICNT0);
}

static inline void SET_USI_TO_SEND_DATA(void)
{
	I2C_DDR |= (1 << I2C_SDA);  // SDA as output
	USISR =
		(0 << USISIF)  |  // DON'T clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(0 << USICNT3) |  // Prepare counter to shift 8 bits
		(0 << USICNT2) |
		(0 << USICNT1) |
		(0 << USICNT0);
}

static inline void SET_USI_TO_READ_DATA(void)
{
	I2C_DDR &= ~(1 << I2C_SDA);  // SDA as input
	USISR =
		(0 << USISIF)  |  // DON'T clear start condition interrupt flag
		(1 << USIOIF)  |  // Clear overflow interrupt flag
		(1 << USIPF)   |  // Clear stop condition flag
		(1 << USIDC)   |  // Clear data collision flag
		(0 << USICNT3) |  // Prepare counter to shift 8 bits
		(0 << USICNT2) |
		(0 << USICNT1) |
		(0 << USICNT0);
}