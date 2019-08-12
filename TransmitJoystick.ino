/*  
Transceiver using the nRF24L01 module to send and receive 4 channels and convert them to PPM.
nRF24L01:(1-GND,2-VCC,3-D9,4-D10,5-D13,6-D11,7-D12,8-N/A)
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h> 
#include "printf.h"

const uint64_t pipeOut = 0xE8E8F0F0E1LL;
//const uint64_t pipeIn = 0xE8E8F0F0E1LL;
RF24 radio(9,10);
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
  data.X= 128;
  data.Y = 128;
  data.YA = 128;
  data.R = 128;
  data.H = 0;
}

void setup()
{
  Serial.begin(9600);
  radio.begin();
  printf_begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS); // Both endpoints must have this set the same
  radio.setChannel(108); //2.508 Ghz 
  radio.setRetries(5,15);
  radio.openWritingPipe(pipeOut);
  //radio.startListening();
  radio.printDetails();
  resetData();
}

void loop()
{
  data.X = saturation(map(analogRead(A1),100,900,0,255));
  data.Y = saturation(map(analogRead(A0),900,100,0,255));
  data.R = 0;
  data.T = saturation(map(analogRead(A3),100,950,0,255));
  data.YA = saturation(map(analogRead(A2),20,980,0,255));  
  data.H = 1*100;
  data.H += 0*10;
  data.H += 0*1;
  Serial.println(data.YA);
  radio.write(&data, sizeof(MyData));
}

int saturation(int x)
{
  if (x > 110 && x < 140)
  {
    x = 128;
  }
  else if(x >= 255)
  {
    x = 255;
  }
  else if(x <= 0)
  {
    x = 0;
  }
  return(x);
}

