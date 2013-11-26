/******************************************************************************
 * This is the code for the Fail Safe Board.
 * Author: Nathan Hui
 *****************************************************************************/

/////////////////////////CONFIGURATION/////////////////////////////////////////
#define PPM_FrLen 22500		// set the PPM frame length (us)
#define PPM_PulseLen 300	// set the pulse length (us)
#define S_PPM_PIN E5		// set PPM signal output pin
#define RC_PPM_PIN D0		// set PPM signal input pin
#define CH_THROTTLE 2		// Set Throttle PPM channel
#define CH_YAW 0			// Set Yaw PPM channel
#define CH_PITCH 1			// Set Pitch PPM channel
#define CH_ROLL 3			// Set Roll PPM channel
#define CH_AUX 4			// Set Aux PPM channel
#define ON_TIME_PIN PORTB6	// Set Busy Time Pin
/////////////////////////CONFIGURATION/////////////////////////////////////////
 

int rc_ppm[8];
int s_ppm[8];
int flags = 0;	// bit 0 is Serial Available, bit 1 is RC PPM Health
int busyflag = 0;	
String inputString = "";

void setup(){
	Serial.begin(9600);
	
	DDRE |= (1 << DDE5);	// Set E5 as output
	DDRD &= ~(1 << DDD0);	// Set D0 as input
	DDRB |= (1 << DDB7);	// Set B7 as output
	DDRB |= (1 << DDB6);	// Set B6 as output
	
	PORTB &= ~(1 << PORTB7);	// Turn off led (set B7 low)
	cli();	// disable interrupts
	TCCR1A = 0;	// Clear timer 1 control register a
	TCCR1B = 0;	// Clear timer 1 control register b
	TCCR3A = 0;	// Clear timer 3 control regsiter a
	TCCR3B = 0;	// Clear timer 3 control register b
	
	// Configure timers
	// Using timer1 for RC_PPM, timer3 for S_PPM
	OCR3A = 100;	// compare match register, set to dummy value to start
	TCCR3B |= (1 << WGM32);	// turn on Clear Timer on Compare mode
	TCCR3B |= (1 << CS31);	// 8 prescaler: 0.5 us @ 16 MHz
	TIMSK3 |= (1 << OCIE3A);	// enable timer compare interrupt
	
//	OCR1A = 6000;	// compare match register, set to 10000 us
	TCCR1B |= (1 << WGM12);	// use CTC mode
	TIMSK1 |= (1 << TOIE1);	// enable timer compare interrupt
	TCCR1B |= (1 << CS11);	// 8 prescaler: 0.5 us @ 16 MHz
	EIMSK |= (1 << INT0);	// enable extermal interrupt on pin 2
	EICRA |= (1 << ISC01);	// Trigger INT0 on falling edge
	sei();	// enable interrupts
	
	s_ppm[0] = 900;
	s_ppm[1] = 900;
	s_ppm[2] = 900;
	s_ppm[3] = 900;
	s_ppm[4] = 900;
	s_ppm[5] = 900;
	s_ppm[6] = 900;
	s_ppm[7] = 900;
	PORTB |= (1 << ON_TIME_PIN);	// turn on ON_TIME_PIN
}

void loop(){
	// PORTB &= ~(1 << ON_TIME_PIN);
// /*	while(Serial.available()){
		// char inChar = (char)Serial.read();
		// inputString += inChar;
		// flags |= (1 << 0);
	// }*/
// /*	if(flags){
		// PORTB |= (1 << PORTB7);
	// }else{
		// PORTB &= ~(1 << PORTB7);
	// }*/
	// if(flags & (1 << 0)){
		// char readChar = inputString.charAt(0);
// /*		switch(readChar){
			// case 97:
				// PORTB |= (1 << PORTB6);
				// break;
			// case 98:
				// PORTB &= ~(1 << PORTB6);
				// break;
			// default:
				// PORTB &= ~(1 << PORTB6);
				// break;
		// }*/
		// flags ^= (1 << 0);
		// inputString = "";
	// }
// /*	s_ppm[0] = rc_ppm[0];
	// s_ppm[1] = rc_ppm[1];
	// s_ppm[2] = rc_ppm[2];
	// s_ppm[3] = rc_ppm[3];
	// s_ppm[4] = rc_ppm[4];
	// s_ppm[5] = rc_ppm[5];
	// s_ppm[6] = rc_ppm[6];
	// s_ppm[7] = rc_ppm[7];*/
	// PORTB |= (1 << ON_TIME_PIN);
	// delay(100);
}

void SerialEvent(){
}

ISR(INT0_vect){	// Falling edge interrupt for RC_PPM
//	PORTB &= ~(1 << ON_TIME_PIN);
	static unsigned int pulse;
	static unsigned long counter;
	static byte channel;
	
	counter = TCNT1;	// Get current time
	TCNT1 = 0;	// Reset timer
	flags &= ~(1 << 1);	// set good RC PPM Health flag
	if(counter < 1020){	// must be a pulse if less than 510 us
		pulse = counter;
	}else if(counter > 6000){	// sync pulses over 3000us
		channel = 0;
	}else{	// servo values between 510 us and 3000 us
		rc_ppm[channel] = (counter + pulse) / 2;
		channel++;
	}
//	PORTB |= (1 << ON_TIME_PIN);
}

ISR(TIMER1_OVF_vect){
	PORTB &= ~(1 << ON_TIME_PIN);
	flags |= (1 << 1);	// set bad RC PPM Health flag
	PORTB |= (1 << ON_TIME_PIN);
}

ISR(TIMER3_COMPA_vect){
//	PORTB &= ~(1 << ON_TIME_PIN);
	static boolean state = true;	// set initial state
	
	TCNT3 = 0;	// reset timer
	
	if(state){	// start pulse
		PORTE &= ~(1 << PORTE5);
		OCR3A = 600;	// timeout at 300 us
		state = false;
	}else{	// end pulse and calculate when to start next
		static byte cur_chan_num;
		static unsigned int calc_rest;
		
		PORTE |= (1 << PORTE5);
		state = true;
		
		if(cur_chan_num >= 8){
			cur_chan_num = 0;	// reset frame
			calc_rest = calc_rest + 300;
			OCR3A = (22500 - calc_rest) * 2;
			calc_rest = 0;
		}else{
			OCR3A = (s_ppm[cur_chan_num] - 300) * 2;
			calc_rest = calc_rest + s_ppm[cur_chan_num];
			cur_chan_num++;
		}
	}
//	PORTB |= (1 << ON_TIME_PIN);
}
