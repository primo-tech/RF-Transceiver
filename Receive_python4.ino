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
int c[26];
int n = 0;
int X,Y,T,YA,W1,W2,R,S;
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
  data.X = 128;
  data.Y = 128;
  data.YA = 128;
  data.R = 128;
  data.H = 0;
}

void setup()
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  
  radio.begin();
  printf_begin();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS); // Both endpoints must have this set the same
  radio.setChannel(108); //2.508 Ghz 
  radio.setRetries(5,15);
  radio.openWritingPipe(pipeOut);
  radio.printDetails();
  resetData();
}

bool State = 0;

void loop()
{
  if (State == 0)
  {
    data.YA = saturation(map(analogRead(A2),20,980,0,255));  
    
    data.T = saturation(map(analogRead(A3),100,950,0,255));
    
    data.X = saturation(map(analogRead(A1),100,900,0,255));
    
    data.Y = saturation(map(analogRead(A0),900,100,0,255));
    
    data.H = 1*100;
    data.H += 0*10;
    data.H += 0*1;

    data.R = 0;
    
    radio.write(&data, sizeof(MyData));
    Serial.println(data.T);
    if (data.T < 25 && data.Y > 225)
    {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);                       // wait for a second
      State = 1;
    }
  }
  
  if(State == 1)
  {
    if(Serial.available())
    {
      a = Serial.read();
      c[n] = (int) a -48;
      n++;
        
      if (a == '>')
      {
        n = 0;
        
        YA = c[1]*100;
        YA += c[2]*10;
        YA += c[3]*1;
        data.YA = saturation(map(YA,100,355,0,255));

        T = c[5]*100;
        T += c[6]*10;
        T += c[7]*1;
        data.T = saturation(map(T,100,355,0,255));
        
        X = c[9]*100;
        X += c[10]*10;
        X += c[11]*1;
        data.X = saturation(map(X,100,355,0,255));
          
        Y = c[13]*100;
        Y += c[14]*10;
        Y += c[15]*1;
        data.Y = saturation(map(Y,100,355,0,255));
          
        data.H = c[17]*100;
        data.H += c[18]*10;
        data.H += c[19]*1;

        data.R = c[21]*1000;
        data.R += c[22]*100;
        data.R += c[23]*10;
        data.R += c[24]*1;

        radio.write(&data, sizeof(MyData));

        if (data.H == 9)
        {
          digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(1000);                       // wait for a second
          digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
          delay(1000);                       // wait for a second
          State = 0;
        }
      }
    }
    else
    {
      State = 0;
    }
  }
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
