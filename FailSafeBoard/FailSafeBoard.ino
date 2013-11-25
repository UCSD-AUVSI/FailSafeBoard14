/******************************************************************************
 * This is the code for the Fail Safe Board.
 * Author: Nathan Hui
 *****************************************************************************/

/////////////////////////CONFIGURATION/////////////////////////////////////////
#define PPM_FrLen 22500		// set the PPM frame length (us)
#define PPM_PulseLen 300	// set the pulse length (us)
#define S_PPM_PIN 3			// set PPM signal output pin
#define RC_PPM_PIN 21		// set PPM signal input pin
#define CH_THROTTLE 2		// Set Throttle PPM channel
#define CH_YAW 0			// Set Yaw PPM channel
#define CH_PITCH 1			// Set Pitch PPM channel
#define CH_ROLL 3			// Set Roll PPM channel
#define CH_AUX 4			// Set Aux PPM channel
/////////////////////////CONFIGURATION/////////////////////////////////////////
 

int rc_ppm[8];
int s_ppm[8];
int flags = 0;	// bit 0 is Serial Available
String inputString = "";

void setup(){
	Serial.begin(9600);
	pinMode(S_PPM_PIN, OUTPUT);
	pinMode(RC_PPM_PIN, INPUT);
	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);
	digitalWrite(13, LOW);
	cli();	// disable interrupts
	TCCR1A = 0;	// Clear timer 1 control register a
	TCCR1B = 0;	// Clear timer 1 control register b
	TCCR3A = 0;	// Clear timer 3 control regsiter a
	TCCR3B = 0;	// Clear timer 3 control register b
	
	// Configure timers
	// Using timer1 for RC_PPM, timer3 for S_PPM
	OCR3A = 100;	// compare match register, set to dummy value to start
	TCCR3B |= (1 << WGM12);	// turn on Clear Timer on Compare mode
	TCCR3B |= (1 << CS11);	// 8 prescaler: 0.5 us @ 16 MHz
	TIMSK3 |= (1 << OCIE1A);	// emable timer compare interrupt
	
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
}

void loop(){
	if(inputString.length())
		Serial.println(inputString);
	inputString = "";
	/* if(flags){
		PORTB |= (1 << PORTB7);
	}else{
		PORTB &= ~(1 << PORTB7);
	}
	if(flags & (1 << 1)){
		char readChar = inputString.charAt(0);
		switch(readChar){
			case 97:
				digitalWrite(12, HIGH);
				//PORTB |= (1 << PORTB6);
				break;
			case 98:
				digitalWrite(12, LOW);
				//PORTB &= ~(1 << PORTB6);
				break;
			default:
				digitalWrite(12, LOW);
				//PORTB &= ~(1 << PORTB6);
				break;
		}
		flags ^= (1 << 1);
	} */
	delay(10);
}

void SerialEvent(){
	while(Serial.available()){
		char inChar = (char)Serial.read();
		inputString += inChar;
		//flags |= (1 << 1);
	}
}

ISR(INT0_vect){	// Falling edge interrupt for RC_PPM
	static unsigned int pulse;
	static unsigned long counter;
	static byte channel;
	
	counter = TCNT1;	// Get current time
	TCNT1 = 0;	// Reset timer
	if(counter < 1020){	// must be a pulse if less than 510 us
		pulse = counter;
	}else if(counter > 6000){	// sync pulses over 3000us
		channel = 0;
	}else{	// servo values between 510 us and 3000 us
		rc_ppm[channel] = (counter + pulse) / 2;
		channel++;
	}
}

ISR(TIMER3_COMPA_vect){	
	static boolean state = true;	// set initial state
	
	TCNT3 = 0;	// reset timer
	
	if(state){	// start pulse
		// digitalWrite(S_PPM_PIN, LOW);
		PORTE &= ~(1 << PORTE5);
		OCR3A = 600;	// timeout at 300 us
		state = false;
	}else{	// end pulse and calculate when to start next
		static byte cur_chan_num;
		static unsigned int calc_rest;
		
		// digitalWrite(S_PPM_PIN, HIGH);
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
