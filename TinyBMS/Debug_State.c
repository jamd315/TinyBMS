#include "Debug_State.h"

void Debug_LED_init(void)
{
	PORTB |= (1 << DEBUG_LED_0) | (1 << DEBUG_LED_1) | (1 << DEBUG_LED_2);
}

void Debug_LED_show(Debug_State_t state)
{
	PORTB &= ~((1 << DEBUG_LED_0) | (1 << DEBUG_LED_1) | (1 << DEBUG_LED_2));
	if(state & 1)
		PORTB |= (1 << DEBUG_LED_0);
	if(state & 2)
		PORTB |= (1 << DEBUG_LED_1);
	if(state & 4)
		PORTB |= (1 << DEBUG_LED_2);
}