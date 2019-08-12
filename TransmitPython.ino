/*  
Transceiver using the nRF24L01 module to send and receive 4 channels and convert them to PPM.
nRF24L01:(1-GND,2-VCC,3-D9,4-D10,5-D13,6-D11,7-D12,8-N/A)
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h> 
#include "printf.h"

char a;
int c[16];
int n = 0;
int X,Y,T,W1,W2,R,S;
int h,i;

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
  data.T = 128;
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
  if(Serial.available())
  {
     a = Serial.read();
     c[n] = (int) a -48;
     n++;
        
     if (a == '>')
     {
        n = 0;
     
        X = c[1]*100;
        X += c[2]*10;
        X += c[3]*1;
        data.X = map(X,100,355,0,255);
          
        Y = c[5]*100;
        Y += c[6]*10;
        Y += c[7]*1;
        data.Y = map(Y,100,355,0,255);
        
        R = c[9]*100;
        R += c[10]*10;
        R += c[11]*1;
        data.R = map(R,100,355,0,255);
          
        data.H = c[13]*100;
        data.H += c[14]*10;
        data.H += c[15]*1;

        data.T = map(analogRead(A0),0,1023,0,255);
        data.YA = map(analogRead(A1),65,1023,0,255);

        radio.write(&data, sizeof(MyData));
      }
   }
}
