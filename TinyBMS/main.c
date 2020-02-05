#define F_CPU 8000000UL

#include <avr/io.h>		 // Ports and stuff
#include <util/delay.h>	 // _delay_ms
#include <string.h>		 // memcpy
#include "main.h"
#include "I2C.h"
#include "ADC.h"         // ADC_init, ADC_read
#include "Debug_State.h"


int main(void)
{
	Debug_LED_init();
	ADC_init();
	I2C_init();
	Debug_LED_show(STARTUP_COMPLETE);
	while(1)
	{
		//Debug_LED_show(I2C_read());
	}
}

float Bat_read(void)
{
	return 5.2;  // Temporary
	uint16_t adc_val = ADC_read();
	float bat_voltage = 1.0 * adc_val / (1 << 10) * 5.0 * ((R1 + R2) / R2);  // Get input voltage as float, then reverse the voltage divider to get battery voltage.  10 bit resolution with 5V reference
	return bat_voltage;
}
