//this programm will put out a PPM signal

//////////////////////CONFIGURATION///////////////////////////////
#define chanel_number 8  //set the number of chanels
#define default_servo_value 1000  //set the default servo value
#define PPM_FrLen 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 3  //set PPM signal output pin on the arduino
#define THROTTLE 2
#define YAW 0
#define PITCH 1
#define ROLL 3
#define AUX 4
//////////////////////////////////////////////////////////////////


/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[chanel_number];

void setup(){  
  //initiallize default ppm values
  for(int i=0; i<chanel_number; i++){
    ppm[i]= default_servo_value;
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR3A = 0; // set entire TCCR1 register to 0
  TCCR3B = 0;
  
  OCR3A = 100;  // compare match register, change this
  TCCR3B |= (1 << WGM12);  // turn on CTC mode
  TCCR3B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK3 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
  
	Serial.begin(9600);

}

void loop(){
  //put main code here
  static int val = 1;

  Serial.println(" ");
  ppm[0] = 900;
  Serial.println(ppm[0]);
  ppm[1] = 902;
  Serial.println(ppm[1]);
  ppm[2] = 906;
  Serial.println(ppm[2]);
  ppm[3] = 1000;
  Serial.println(ppm[3]);
  ppm[4] = 1500;
  Serial.println(ppm[4]);
  ppm[5] = 2095;
  Serial.println(ppm[5]);
  ppm[6] = 2099;
  Serial.println(ppm[6]);
  ppm[7] = 2100;
  Serial.println(ppm[7]);
  
  
//  ppm[0] = ppm[0] + val;
//  if(ppm[0] >= 2000){ val = -1; }
//  if(ppm[0] <= 1000){ val = 1; }
	delay(1000);

}

ISR(TIMER3_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT3 = 0;
  
  if(state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR3A = PPM_PulseLen * 2;
    state = false;
  }
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= chanel_number){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PPM_PulseLen;// 
      OCR3A = (PPM_FrLen - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR3A = (ppm[cur_chan_numb] - PPM_PulseLen) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
