#define F_CPU 8000000UL

#include <avr/io.h>		// Ports and stuff
#include <util/delay.h>	// _delay_ms
#include <string.h>		// memcpy
#include "main.h"
#include "USI_TWI_Slave.h"


int main(void)
{
	DDRB = 
		(1 << SDA) |
		(1 << SCL) |
		(0 << BAT) |  // Might not be necessary?
		(1 << TRIGGER_L);
	ADC_init();
	USI_TWI_Slave_Initialize(TWI_ADDR);
	TWI_Command_t command;
	float bat_voltage;
	char float_bytes[sizeof(float)];
	PORTB |= (1 << TRIGGER_L);
	_delay_ms(1000);
	PORTB &= ~(1 << TRIGGER_L);
	_delay_ms(1000);
	while(1)
	{
		command = USI_TWI_Receive_Byte();
		switch (command)
		{
			case POLL:
				bat_voltage = ADC_read();
				memcpy(float_bytes, &bat_voltage, sizeof(float));
				for (unsigned char i = 0; i < sizeof(float); i++)
				{
					USI_TWI_Transmit_Byte(float_bytes[i]);
				}
				break;
			default:
				break;
		}
	}
}

void ADC_init(void)
{
	ADMUX = 
		(0 << REFS1) |  // Use VCC for reference voltage
		(0 << REFS0) |
		(0 << ADLAR) |  // Left mode, see notes
		(0 << REFS2) |  // VCC ref cont.
		(0 << MUX3)  |  // Single ended input for ADC2 (PB4)
		(0 << MUX2)  |
		(1 << MUX1)  |
		(0 << MUX0);
	ADCSRA = 
		(1 << ADEN)  |  // Enable ADC
		// Just write 0 to the status bits
		(1 << ADPS2) |  // Set prescaler to 128, at F_CPU = 8MHz the ADC runs at 62.5kHz
		(1 << ADPS1) |
		(1 << ADPS0);
}

float ADC_read(void)
{
	/*
	8 bit sampling has a resolution of ~20mV between 0-5V
	10 bit sampling has a resolution of ~5mv between 0-5V
	Float has 4 bytes and is more than enough
	*/
	return 5.5;  // TODO this is temporary
	ADCSRA |= (1 << ADSC);  // Start a conversion
	while (ADCSRA & (1 << ADSC));  // Wait for conversion to complete
	uint16_t adc_val = ADCL | (ADCH << 8);
	float bat_voltage = 1.0 * adc_val / (1 << 10) * 5.0 * ((R1 + R2) / R2);  // Get input voltage as float, then reverse the voltage divider to get battery voltage.  10 bit resolution with 5V reference
	return bat_voltage;
}
