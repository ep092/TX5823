#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

//user defined:
#define IR_LED_PORT PORTB
#define IR_LED_PIN PB3

#define PWM_IN_PORT PORTB
#define PWM_IN_PIN PB1

#define SPI_DATA_PORT PORTB
#define SPI_DATA_PIN PB0

#define SPI_LE_PORT PORTB
#define SPI_LE_PIN PB4

#define SPI_CLOCK_PORT PORTB
#define SPI_CLOCK_PIN PB2


//non user defined:
#define IR_LED_ON
#define IR_LED_OFF



void canon_shutter_now(void)
{
 for(int i=0; i<16; i++) { 
    IR_LED_ON;
    _delay_us(11);
    IR_LED_OFF;
    _delay_us(11);
  } 
  _delay_us(7330); 
  for(int i=0; i<16; i++) { 
    IR_LED_ON;
    _delay_us(11);
    IR_LED_OFF;
    _delay_us(11);
  }
}

void canon_shutter_delayed(void)
{
  for(int i=0; i<16; i++) { 
    IR_LED_ON;
    _delay_us(11);
    IR_LED_OFF;
    _delay_us(11);
  } 
  _delay_us(5360); 
  for(int i=0; i<16; i++) { 
    IR_LED_ON;
    _delay_us(11);
    IR_LED_OFF;
    _delay_us(11);
  }
}

int main(void) {
}