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

#define TIME_BETWEEN_PHOTOS 3000

//settings for pwm signal
#define PWM_VALUE_0_MIN	105	//min length for the signal (pulse) to be detectet as "middle"
#define PWM_VALUE_0_MAX 135	//max length for the signal (pulse) to be detectet as "middle"
#define PWM_VALUE_MIN 75	//min length of the signal (pulse)
#define PWM_VALUE_MAX 175	//max length of the signal (pulse)


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
	// CH 1 CH 2 	CH 3 	CH 4 	CH 5 	CH 6 	CH 7 	CH 8
	0x7981, 0x758D, 0x7199, 0x6DA5, 0x69B1, 0x65BD, 0x6209, 0x5E15, // Band A
	0x5F9D, 0x6338, 0x6713, 0x6AAE, 0x6E89, 0x7224, 0x75BF, 0x799A, // Band B
	0x5A21, 0x562D, 0x5239, 0x4E85, 0x7D35, 0x8129, 0x851D, 0x8911, // Band E
	0x610C, 0x6500, 0x68B4, 0x6CA8, 0x709C, 0x7490, 0x7884, 0x7C38, // Band F / Airwave
	0x510A, 0x5827, 0x5F84, 0x66A1, 0x6DBE, 0x751B, 0x7C38, 0x8395  // Band R / Raceband
};

volatile uint8_t pwm_value = 0 ;
volatile uint8_t new_data = 0 ;

void canon_shutter_now(void){
	for(int i=0; i<16; i++) { 
		IR_LED_ON;
		_delay_us(15);
		IR_LED_OFF;
		_delay_us(15);
		
	} 
	_delay_us(7330); 
	for(int i=0; i<16; i++) { 
		IR_LED_ON;
		_delay_us(15);
		IR_LED_OFF;
		_delay_us(15);
		
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

	channelData = pgm_read_word_near(channelTable + channel);
	// 25 bits of data
	// Order: A0-3, !R/W, D0-D19
	// A0=1, A1=0, A2=0, A3=0, RW=1, D0-19=0 

	// 20 bytes of register data are sent, but the MSB 4 bits are zeros
	// register address = 0x1, write, data0-15=channelData data15-19=0x4

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
	serial_send_bit_0();
	serial_send_bit_0();
	serial_send_bit_1();
	serial_send_bit_0();

	// Finished clocking data in
	serial_enable_high();
	_delay_us(1);
}

void int0_init(void){
	MCUCR |= (1<<ISC00);	//Any logical change on INT0 generates an interrupt request
	GIMSK |= (1<<INT0);	//External Interrupt Request 0 Enable
}

void timer_init(void){
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
	uint8_t pwm_pulse_now;
	uint8_t pwm_pulse_last;
	uint8_t channel = 8;		//channel at startup
	init();
	timer_init();
	int0_init();
	set_channel(channel);
	sei();
	canon_shutter_now();
	_delay_ms(1000);
	pwm_value = 0;
	new_data = 0;
	_delay_ms(100);
	while (1){
		//wait for new data and detect timeout at pwm_in
		for(uint8_t i = 0; new_data==0 && i<200; i++){
			_delay_ms(1);
		}
		if (new_data) {
			pwm_pulse_last = pwm_pulse_now;
			pwm_pulse_now = pwm_value;
			if (pwm_pulse_now > PWM_VALUE_MIN && pwm_pulse_now < PWM_VALUE_0_MIN){		//switch channel
				if (pwm_pulse_last > PWM_VALUE_0_MIN && PWM_VALUE_0_MAX < 180){		//detect "falling edge"
					channel++;
					set_channel(channel);	
					_delay_ms(200); 
				}
			
			} else if(pwm_pulse_now > PWM_VALUE_0_MAX && pwm_pulse_now < PWM_VALUE_MAX){	//do photo
				while (pwm_value >PWM_VALUE_0_MAX && pwm_value < PWM_VALUE_MAX){	
					canon_shutter_now();
					_delay_ms(TIME_BETWEEN_PHOTOS);			//time between pictures
				}
			}
			
		}else{
			pwm_value = 0;
			pwm_pulse_last = 0;
			pwm_pulse_now = 0;
		}
		
		new_data = 0;
	}	
}

//saves the length of the pwm_pulse to pwm_value
ISR ( INT0_vect) {
	uint8_t counter = TCNT0; //buffer timervalue  
	TCNT0 = 0; //reset timer
	if (TIFR0 & (1<<TOV0)){	//timeroverflow
		TIFR0 = _BV(TOV0);
		if (PINB & (1<<PWM_IN_PIN)){
			pwm_value = 0;
			new_data = 1;
		}
	} else {

		pwm_value = counter;
		new_data = 1;
	}		
}