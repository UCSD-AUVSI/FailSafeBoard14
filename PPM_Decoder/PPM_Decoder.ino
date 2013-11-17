/**
 * This program puts the servo values into an array,  reagrdless of channel
 * number, polarity, ppm frame length, etc...
 */

/////////////////////////CONFIGURATION/////////////////////////////////////////
#define PPM_Pin 2  //this must be 2 or 3
/////////////////////////CONFIGURATION/////////////////////////////////////////

int ppm[16];  //array for storing up to 16 servo signals

void setup(){
	Serial.begin(9600);
	pinMode(PPM_Pin, INPUT);
	attachInterrupt(PPM_Pin - 2, read_ppm, FALLING);
	TCCR1A = 0;  //reset timer1
	TCCR1B = 0;
	TCCR1B |= (1 << CS11);  //set timer1 to increment every 0,5 us
}

void loop(){
	//You can delete everithing inside loop() and put your own code here
	int count;
	while(ppm[count] != 0){  //print out the servo values
		Serial.print(ppm[count]);
		Serial.print("  ");
		count++;
	}
	Serial.println("");
	delay(100);  //you can even use delays!!!
}

void read_ppm(){  //leave this alone
	static unsigned int pulse;
	static unsigned long counter;
	static byte channel;
	counter = TCNT1;
	TCNT1 = 0;
	if(counter < 1020){  //must be a pulse if less than 510us
		pulse = counter;
	}else if(counter > 6000){  //sync pulses over 1910us
		channel = 0;
	}else{  //servo values between 510us and 2420us will end up here
		ppm[channel] = (counter + pulse) / 2;
		channel++;
	}
}
