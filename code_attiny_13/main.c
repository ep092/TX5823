#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


//user defined:
#define IR_LED_PORT PORTB
#define IR_LED_DDR DDRB
#define IR_LED_PIN PB3

#define PWM_IN_PORT PORTB
#define PWM_IN_DDR DDRB
#define PWM_IN_PIN PB1

#define SPI_DATA_PORT PORTB
#define SPI_DATA_DDR DDRB
#define SPI_DATA_PIN PB0

#define SPI_LE_PORT PORTB
#define SPI_LE_DDR DDRB
#define SPI_LE_PIN PB4

#define SPI_CLOCK_PORT PORTB
#define SPI_CLOCK_DDR DDRB
#define SPI_CLOCK_PIN PB2


//non user defined:
#define IR_LED_ON 		IR_LED_PORT |= (1 << IR_LED_PIN)
#define IR_LED_OFF 		IR_LED_PORT &= ~(1 << IR_LED_PIN)
#define SPI_DATA_LOW 		SPI_DATA_PORT &= ~(1 << SPI_DATA_PIN)
#define SPI_DATA_HIGH 		SPI_DATA_PORT |= (1 << SPI_DATA_PIN)
#define SPI_LE_LOW 		SPI_LE_PORT &= ~(1 << SPI_LE_PIN)
#define SPI_LE_HIGH 		SPI_LE_PORT |= (1 << SPI_LE_PIN)
#define SPI_CLOCK_LOW 		SPI_CLOCK_PORT &= ~(1 << SPI_CLOCK_PIN)
#define SPI_CLOCK_HIGH 		SPI_CLOCK_PORT |= (1 <<SPI_CLOCK_PIN)

// Channels to sent to the SPI registers
const uint16_t channelTable[] PROGMEM = {
  // Channel 1 - 8
  0x2A05,    0x299B,    0x2991,    0x2987,    0x291D,    0x2913,    0x2909,    0x289F,    // Band A
  0x2903,    0x290C,    0x2916,    0x291F,    0x2989,    0x2992,    0x299C,    0x2A05,    // Band B
  0x2895,    0x288B,    0x2881,    0x2817,    0x2A0F,    0x2A19,    0x2A83,    0x2A8D,    // Band E
  0x2906,    0x2910,    0x291A,    0x2984,    0x298E,    0x2998,    0x2A02,    0x2A0C  // Band F / Airwave
};

/*
// Channels with their Mhz Values
const uint16_t channelFreqTable[] PROGMEM = {
  // Channel 1 - 8
  5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A
  5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B
  5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E
  5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880  // Band F / Airwave
};
*/


volatile uint8_t pwm_value = 0 ;

void canon_shutter_now(void){
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

void canon_shutter_delayed(void){
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


void serial_send_bit_1(void){
	SPI_CLOCK_LOW;
	_delay_us(1);
	SPI_DATA_HIGH;
	_delay_us(1);
	SPI_CLOCK_HIGH;
	_delay_us(1);
	SPI_CLOCK_LOW;
	_delay_us(1);
}

void serial_send_bit_0(void)
{
	SPI_CLOCK_LOW;
	_delay_us(1);
	SPI_DATA_LOW;
	_delay_us(1);
	SPI_CLOCK_HIGH;
	_delay_us(1);
	SPI_CLOCK_LOW;
	_delay_us(1);
}

void serial_enable_low(void){
	_delay_us(1);
	SPI_LE_LOW;
	_delay_us(1);
}

void serial_enable_high(void){
	_delay_us(1);
	SPI_LE_HIGH;
	_delay_us(1);
}

void set_channel(uint8_t channel){
	uint8_t i;
	uint16_t channelData;

	//channelData = pgm_read_word(&channelTable[channel]);
	channelData = channelTable[channel];	
	//channelData = pgm_read_word_near(channelTable + channel);

	// 25 bits of data
	// Order: A0-3, !R/W, D0-D19
	// A0=0, A1=0, A2=0, A3=1, RW=0, D0-19=0
 
	// 20 bytes of register data are sent, but the MSB 4 bits are zeros
	// register address = 0x1, write, data0-15=channelData data15-19=0x0

	serial_enable_high();
	_delay_us(1);
	serial_enable_low();
	
	// register 0x1
	serial_send_bit_1();
	serial_send_bit_0();
	serial_send_bit_0();
	serial_send_bit_0();

	// write to register
	serial_send_bit_1();

	// D0-D15
	for (i = 16; i > 0; i--){
	// check if bit is high or low
	if (channelData & 0x1){
		serial_send_bit_1();
	}
	else{
		serial_send_bit_0();
	}

	// Shift bits along to check the next one
	channelData >>= 1;
	}

	// Remaining D16-D19
	for (i = 4; i > 0; i--){
		serial_send_bit_0();
	}

	// Finished clocking data in
	serial_enable_high();
	_delay_us(1);

}

void int0_init(void){
	MCUCR |= (1<<ISC00);	//Any logical change on INT0 generates an interrupt request
	GIMSK |= (1<<INT0);	//External Interrupt Request 0 Enable
}

void timer_init(void){
	TCCR0A |= (1<<WGM01); //mode of operation CTC
	TCCR0B |= (1<<CS01) | (1<<CS00); //prescaler 64
}

void init (void){
	IR_LED_DDR |= (1<<IR_LED_PIN);
	PWM_IN_PORT |= (1<<PWM_IN_PIN);
	SPI_DATA_DDR |= (1<<SPI_DATA_PIN);
	SPI_LE_DDR |= (1<<SPI_LE_PIN);
	SPI_CLOCK_DDR |= (1<<SPI_CLOCK_PIN);
	serial_enable_high();
	SPI_CLOCK_LOW;
	
}

int main(void) {
	init();
	timer_init();
	int0_init();
	canon_shutter_now();
	_delay_ms(1000);
	pwm_value = 0;
	_delay_ms(100);
	
	while (1){
		//for (int i = 0; i<32; i++){
			_delay_ms(9000);
			set_channel(20);
			canon_shutter_now();
		//}
	}	
}

ISR ( INT0_vect) {
	uint8_t counter = TCNT0; //buffer timervalue  
	TCNT0 = 0; //reset timer
	if (TIFR0 & (1<<TOV0)){	//timeroverflow
		TIFR0 |= (1<<TOV0);
		if (!(PWM_IN_PORT & (1<<PWM_IN_PIN))){
			counter = 0;
		}
	} else {
		pwm_value = counter;
	}		
}