#define F_CPU 8000000UL

#include <avr/io.h>		 // Ports and stuff
#include <util/delay.h>	 // _delay_ms
#include <string.h>		 // memcpy
#include "main.h"
#include "ADC.h"         // ADC_init, ADC_read
#include "usiTwiSlave.h" // 


int main(void)
{
	DDRB = 
		(0 << BAT) |  // Might not be necessary?
		(1 << TRIGGER_L);
	//ADC_init();
	usiTwiSlaveInit(0x22);
	//TWI_Command_t command;
	while(1)
	{
		usiTwiTransmitByte(0b11110101);
	}
}

void flash(void)
{
	PORTB |= (1 << TRIGGER_L);
	_delay_ms(1000);
	PORTB &= ~(1 << TRIGGER_L);
	_delay_ms(1000);
}

float Bat_read(void)
{
	return 5.2;  // Temporary
	uint16_t adc_val = ADC_read();
	float bat_voltage = 1.0 * adc_val / (1 << 10) * 5.0 * ((R1 + R2) / R2);  // Get input voltage as float, then reverse the voltage divider to get battery voltage.  10 bit resolution with 5V reference
	return bat_voltage;
}
