#ifndef MAIN_H_
#define MAIN_H_

// Useful macros
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// Pins, for both SPI and I2C
#define DI PB0
#define SDA PB0
#define DO PB1
#define USCK PB2
#define SCL PB2
#define TRIGGER_L PB3
#define BAT PB4  // ADC2

// Resistor values in the voltage divider
#define R1 1000000
#define R2 430000

// Function declarations
void ADC_init(void);
float ADC_read(void);
void flash_n(uint8_t n_times);
void flash_byte(uint8_t data);

#endif /* MAIN_H_ */