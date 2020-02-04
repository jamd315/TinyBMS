#include <avr/io.h>
#include "ADC.h"

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

uint16_t ADC_read(void)
{
	/*
	8 bit sampling has a resolution of ~20mV between 0-5V
	10 bit sampling has a resolution of ~5mv between 0-5V
	Float has 4 bytes and is more than enough
	*/
	ADCSRA |= (1 << ADSC);  // Start a conversion
	while (ADCSRA & (1 << ADSC));  // Wait for conversion to complete
	uint16_t adc_val = ADCL | (ADCH << 8);
	return adc_val;
}
