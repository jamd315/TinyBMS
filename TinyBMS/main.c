#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "main.h"


int main(void)
{
	DDRB = 
		(0 << BAT) |
		(1 << TRIGGER_L);
	_delay_ms(1000);
	while(1)
	{
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
	ADCSRA |= (1 << ADSC);  // Start a conversion
	while (ADCSRA & (1 << ADSC));  // Wait for conversion to complete
	uint16_t adc_val = ADCL | (ADCH << 8);
	float bat_voltage = 1.0 * adc_val / (1 << 10) * 5.0 * ((R1 + R2) / R2);  // Get input voltage as float, then reverse the voltage divider to get battery voltage.  10 bit resolution with 5V reference
	return bat_voltage;
}

void flash_n(uint8_t n_times)
{
	for(int i = 0; i < n_times; i++)
	{
		sbi(PORTB, TRIGGER_L);
		_delay_ms(100);
		cbi(PORTB, TRIGGER_L);
		_delay_ms(100);
	}
}

void flash_byte(uint8_t data)
{
	for(int i = 0; i < 8; i++)
	{
		if(data & (1 << i))  // Make a long flash
		{
			sbi(PORTB, TRIGGER_L);
			_delay_ms(400);
			cbi(PORTB, TRIGGER_L);
			_delay_ms(100);
		}
		else  // Make a short flash
		{
			sbi(PORTB, TRIGGER_L);
			_delay_ms(100);
			cbi(PORTB, TRIGGER_L);
			_delay_ms(400);
		}
	}
}