/*  
A basic receiver using the nRF24L01 module to receive 4 channels and convert them to PPM.
nRF24L01:(1-GND,2-VCC,3-D8,4-D10,5-D13,6-D11,7-D12,8-N/A)
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h> 
#include "printf.h"

////////////////////// PPM CONFIGURATION//////////////////////////
#define channel_number 6  //set the number of channels
#define sigPin 2  //set PPM signal output pin on the arduino
#define PPM_FrLen 27000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 400  //set the pulse length
//////////////////////////////////////////////////////////////////


int ppm[channel_number];

const uint64_t pipeIn = 0xE8E8F0F0E1LL;

RF24 radio(8,10);
int value = 0;
// The sizeof this struct should not exceed 32 bytes
struct MyData 
{
  byte X;
  byte Y;
  byte T;
  byte YA;
  byte R;
  byte H;
};

MyData data;

void resetData() 
{
  data.T = 0;
  data.X = 128;
  data.Y = 128;
  data.YA = 128;
  data.R = 128;
  data.H = 0;
  setPPMValuesFromData();
}

void setPPMValuesFromData()
{
  ppm[0] = map(data.T, 0, 255, 1000, 2000);
  ppm[1] = map(data.YA, 0, 255, 1000, 2000);
  ppm[2] = map(data.X, 0, 255, 1000, 2000);
  ppm[3] = map(data.Y, 0, 255, 1000, 2000);  
  ppm[4] = map(data.R, 0, 255, 1000, 2000);
  ppm[5] = map(data.H, 0, 12, 1000, 2000);
}
/**************************************************/

void setupPPM()
{
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, 0);  //set the PPM signal pin to the default state (off)

  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;  // compare match register (not very important, sets the timeout for the first interrupt)
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void setup()
{ 
  setupPPM();
  Serial.begin(9600);
  radio.begin();
  printf_begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS); // Both endpoints must have this set the same
  radio.setChannel(108); //2.508 Ghz 
  radio.openReadingPipe(0,pipeIn);
  radio.startListening();
  radio.printDetails();
  resetData(); 
}

unsigned long lastRecvTime = 0;
void recvData()
{  
  while ( radio.available() ) 
  {        
    radio.read(&data, sizeof(MyData));
    lastRecvTime = millis();
  }
}
void loop()
{
  recvData();
  unsigned long now = millis();
  if ( now - lastRecvTime > 1000 ) 
  {
    // signal lost?
    resetData();
  }
  else
  {
    setPPMValuesFromData();
  }
}

/**************************************************/

//#error This line is here to intentionally cause a compile error. Please make sure you set clockMultiplier below as appropriate, then delete this line.
#define clockMultiplier 2 // set this to 2 if you are using a 16MHz arduino, leave as 1 for an 8MHz arduino

ISR(TIMER1_COMPA_vect){
  static boolean state = true;

  TCNT1 = 0;

  if ( state ) {
    //end pulse
    PORTD = PORTD & ~B00000100; // turn pin 2 off. Could also use: digitalWrite(sigPin,0)
    OCR1A = PPM_PulseLen * clockMultiplier;
    state = false;
  }
  else {
    //start pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    PORTD = PORTD | B00000100; // turn pin 2 on. Could also use: digitalWrite(sigPin,1)
    state = true;

    if(cur_chan_numb >= channel_number) {
      cur_chan_numb = 0;
      calc_rest += PPM_PulseLen;
      OCR1A = (PPM_FrLen - calc_rest) * clockMultiplier;
      calc_rest = 0;
    }
    else {
      OCR1A = (ppm[cur_chan_numb] - PPM_PulseLen) * clockMultiplier;
      calc_rest += ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
