/******************************************************************************
 * This is the code for the Fail Safe Board.
 * Author: Nathan Hui
 * Date: 12/10/13
 *****************************************************************************/

/////////////////////////CONFIGURATION/////////////////////////////////////////
#define PPM_FrLen 22500		// set the PPM frame length (us)
#define PPM_PulseLen 300	// set the pulse length (us)
#define CH_AIL 0			// Set Roll PPM channel
#define CH_ELE 1			// Set Pitch PPM channel
#define CH_THR 2			// Set Throttle PPM channel
#define CH_RUD 3			// Set Yaw PPM channel
#define CH_MOD 7			// Set Mode PPM channel
#define CH_AUX 5			// Set Aux PPM channel
#define RC_RTH 600			// Set RTH mode length
/////////////////////////CONFIGURATION/////////////////////////////////////////

////////////////////////PROGRAM SPECIFIC CONSTANTS/////////////////////////////
#define SAVAIL 0			// Serial Available bit
#define RCHLTH 1			// RC PPM Health (1 is bad health, 0 is good)
#define FSMOD1 2			// Failsafe Mode 1
#define FSMOD2 3			// Failsafe Mode 1
////////////////////////PROGRAM SPECIFIC CONSTANTS/////////////////////////////

int rc_ppm[8];
int s_ppm[8] = {900, 900, 900, 900, 900, 900, 900, 900};
char flags = 0;	// bit 0 is Serial Available, bit 1 is Bad RC PPM Health
/***********************FLAG DESCRIPTIONS**************************************
 * 0	Serial Available
 * 1	Bad RC PPM Health
 * 2	Failsafe Mode 1
 * 3	Failsafe Mode 2
 *****************************************************************************/
int busyflag = 0;	
String inputString = "";

void setup(){
	Serial.begin(9600);
	
	// Configure GPIO
	DDRE |= (1 << DDE5);	// Set E5 as output (S_PPM OUTPUT) (Pin 3)
	DDRD &= ~(1 << DDD0);	// Set D0 as input (RC_PPM INPUT) (Pin 21)
	DDRB |= (1 << DDB7) | (1 << DDB5) | (1 << DDB4);
	// Set B7 as output (LED) (Pin 13), B5 as output (RC_PPM Health) (Pin 11),
	// B4 as Serial Comm Status (Pin 10)
	
	cli();	// disable interrupts
	
	// Configure timers and interrupts
	// Using timer1 for RC_PPM, timer3 for S_PPM
	
	// S_PPM Setup
	TCCR3A = 0;	// Clear timer 3 control regsiter a
	OCR3A = 1000;	// compare match register, set to dummy value to start
	TCCR3B = (1 << WGM32) | (1 << CS31);
	// turn on Clear Timer on Compare mode, set 8 prescaler: 0.5 us @ 16 MHz
	TIMSK3 |= (1 << OCIE3A);	// enable timer compare interrupt
	
	// RC_PPM Setup
	TIMSK1 |= (1 << TOIE1);	// enable overflow interrupt
	TCCR1A = 0;	// Clear timer 1 control register a
	TCCR1B = 0 | (1 << WGM11) | (1 << CS11);
	// use CTC mode, set 8 prescaler: 0.5 us @ 16 MHz
	EIMSK |= (1 << INT0);	// enable extermal interrupt on pin 2
	EICRA |= (1 << ISC01);	// Trigger INT0 on falling edge
	
	// FS_CLK Setup
	TCCR4A = 0;	// Clear control setup
	OCR4A = 46875;	// 3 seconds at 1024 prescaler
	TCCR4B = (1 << WGM42);	// turn on CTC mode
	TIMSK4 |= (1 << OCIE4A);	// enable timer compare interrupt
		
	sei();	// reenable interrupts
}

void loop(){
	while(Serial.available()){
		char inChar = (char)Serial.read();
		inputString += inChar;
		flags |= (1 << 0);
	}	
	
	if(flags){
		// PORTB |= (1 << ON_TIME_PIN);
	}else{
		// PORTB &= ~(1 << ON_TIME_PIN);
	}
	if(flags & (1 << SAVAIL)){
		char readChar = inputString.charAt(0);
		switch(readChar){
			case 97:
				PORTB |= (1 << PORTB4);
				break;
			case 98:
				PORTB &= ~(1 << PORTB4);
				break;
			default:
				PORTB &= ~(1 << PORTB4);
				break;
		}
		flags ^= (1 << SAVAIL);
		inputString = "";
	}
	if(flags & (1 << RCHLTH)){	// RC_PPM Health Bad
		PORTB |= (1 << PORTB5);	// if flag is set, set port high
	}else{
		PORTB &= ~(1 << PORTB5);	// if flag not set, set port low
	}
	if(flags & (1 << FSMOD1)){	// Failsafe Mode 1
		s_ppm[0] = rc_ppm[0];	// copy previous state
		s_ppm[1] = rc_ppm[1];
		s_ppm[2] = rc_ppm[2];
		s_ppm[3] = rc_ppm[3];
		s_ppm[4] = rc_ppm[4];
		s_ppm[5] = rc_ppm[5];
		s_ppm[6] = rc_ppm[6];
		s_ppm[7] = rc_ppm[7];
	}
	if(flags & (1 << FSMOD2)){	// Failsafe Mode 2
		s_ppm[CH_MOD] = RC_RTH;	// RTH!
	}
	
	delay(10);
}

ISR(INT0_vect){	// Falling edge interrupt for RC_PPM
	static unsigned int pulse;
	static unsigned long counter;
	static byte channel;
	
	counter = TCNT1;	// Get current time
	TCNT1 = 0;	// Reset timer
	flags &= ~(1 << RCHLTH);	// set good RC PPM Health flag
	flags &= ~(1 << FSMOD1);	// Clear FS mode
	flags &= ~(1 << FSMOD2);	// Clear FS mode
	if(counter < 1020){	// must be a pulse if less than 510 us
		pulse = counter;
	}else if(counter > 6000){	// sync pulses over 3000us
		channel = 0;
	}else{	// servo values between 510 us and 3000 us
		rc_ppm[channel] = (counter + pulse) / 2;
		channel++;
	}
}

ISR(TIMER4_COMPA_vect){	// 3 seconds for failsafe 1
	flags |= (1 << FSMOD2);	// set new mode
	flags ^= (1 << FSMOD1);
}

ISR(TIMER1_OVF_vect){
	flags |= (1 << RCHLTH) | (1 << FSMOD1);	// set bad RC PPM Health flag
	TCCR4B |= (1 << CS42) | (1 << CS40);	// set watchdog timer on
}

ISR(TIMER3_COMPA_vect){
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
}
