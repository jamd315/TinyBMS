#ifndef DEBUG_STATE_H_
#define DEBUG_STATE_H_

#include <avr/io.h>

#define DEBUG_LED_0 PB1
#define DEBUG_LED_1 PB3
#define DEBUG_LED_2 PB4

typedef enum {
	STARTUP_COMPLETE = 0b001,
	ISR_TRIGGER      = 0b010
} Debug_State_t;

void Debug_LED_init(void);
void Debug_LED_show(Debug_State_t state);


#endif /* DEBUG_STATE_H_ */