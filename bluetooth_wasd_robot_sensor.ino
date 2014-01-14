/*


-----------------------------------------------
HMC5883 Interface code provided from:
An Arduino code example for interfacing with the HMC5883 

by: Jordan McConnell
 SparkFun Electronics
 created on: 6/30/11
 license: OSHW 1.0, http://freedomdefined.org/OSHW

-----------------------------------------------
Teensy Modifications from:
Robert Wozniak

Teensy 3.1
Analog input pin 18 I2C SDA on GY-MCU 
Analog input pin 19 I2C SCL on GY-MCU
*/

#include <Wire.h> //I2C Arduino Library

#define address 0x1E //0011110b, I2C 7bit address of HMC5883

//HMC5883 Sensor Setup
const int threshold = 100;
int16_t x_int,y_int,z_int;

const int ledPin = 13;
uint8_t count =0; 

//Motor Controller assignment and setup
byte right_f, left_f, right_r, left_r;
const int lfpin = 17;
const int lrpin = 16;
const int rfpin = 14;
const int rrpin = 15;

const int speed_rpin = 23;
const int speed_lpin = 22;

int rscale = 255;
int lscale = 255;
int speed_r, speed_l;




void setup() 
{
        //Define Input/Output channels and Start Serial
        pinMode(ledPin, OUTPUT);
        Serial.begin(115200);
        Serial1.begin(115200);
        pinMode(rfpin, OUTPUT);
        pinMode(rrpin, OUTPUT);
        pinMode(lfpin, OUTPUT);
        pinMode(lrpin, OUTPUT);
        pinMode(speed_rpin, OUTPUT);
        pinMode(speed_lpin, OUTPUT);
        
        Wire.begin();
        
        //Put the HMC5883 IC into the correct operating mode
        Wire.beginTransmission(address); //open communication with HMC5883
        
        Wire.send(0x00); //select Config A register
        Wire.send(0x70); //avg 8 samples, 15hz data rate normal bias config  <0 11 100 00>
        
        Wire.send(0x01); //select Config B register
        Wire.send(0x80); //set gain to 440  <100 00000>  (1090 is default)
        
        Wire.send(0x02); //select mode register
        Wire.send(0x00); //continuous measurement mode
        
        Wire.endTransmission();
        
        //Tell the HMC5883 where to begin reading data
        Wire.beginTransmission(address);
        Wire.send(0x03); //select register 3, X MSB register
        Wire.endTransmission();
        

        //Assign Initial values to motor channels
        right_f = 0;
        right_r = 0;
        left_f = 0;
        left_r = 0;
        
        speed_r = 0;
        speed_l = 0;
        
        digitalWrite(rfpin, right_f);
        digitalWrite(rrpin, right_r);
        digitalWrite(lfpin, left_f);
        digitalWrite(lrpin, left_r);
        analogWrite(speed_rpin, speed_r);
        analogWrite(speed_lpin, speed_l);
        
        //3 Blinks at startup, followed by 1.5 second pause
        digitalWrite(ledPin, HIGH);   // set the LED on
        delay(150);                  // wait for a second
        digitalWrite(ledPin, LOW);    // set the LED off
        delay(150);                  // wait for a second
        digitalWrite(ledPin, HIGH);   // set the LED on
        delay(150);                  // wait for a second
        digitalWrite(ledPin, LOW);    // set the LED off
        delay(150);                  // wait for a second
        digitalWrite(ledPin, HIGH);   // set the LED on
        delay(150);                  // wait for a second
        digitalWrite(ledPin, LOW);    // set the LED off
        delay(1500);                  // wait for a second
  
        
        //Read data from each axis, 2 registers per axis
        Wire.requestFrom(address, 6);
        if(6<=Wire.available())
        {
          x_int = Wire.receive()<<8; //X msb
          x_int |= Wire.receive(); //X lsb
          z_int = Wire.receive()<<8; //Z msb
          z_int |= Wire.receive(); //Z lsb
          y_int = Wire.receive()<<8; //Y msb
          y_int |= Wire.receive(); //Y lsb
        }
  
        
}

void loop() 
{
          char incomingByte;
          int16_t x,y,z,xx,yy,zz; //triple axis data
          
        
          //Tell the HMC5883 where to begin reading data
          Wire.beginTransmission(address);
          Wire.send(0x03); //select register 3, X MSB register
          Wire.endTransmission();
          
         
         //Read data from each axis, 2 registers per axis
          Wire.requestFrom(address, 6);
          if(6<=Wire.available())
          {
            xx = Wire.receive()<<8; //X msb
            xx |= Wire.receive(); //X lsb
            zz = Wire.receive()<<8; //Z msb
            zz |= Wire.receive(); //Z lsb
            yy = Wire.receive()<<8; //Y msb
            yy |= Wire.receive(); //Y lsb
          }
          
          x = xx - x_int;
          y = yy - y_int;
          z = zz - z_int;
          
          if(count > 15)
          {
            count = 0;
            //Print out values of each axis
            Serial.print("x: ");
            Serial.print(x);
            Serial.print("  y: ");
            Serial.print(y);
            Serial.print("  z: ");
            Serial.println(z);
          
          }
          else
          {
            count++;
          }

          //Read from Bluetooth Transmitter (Serial1) and Write to Arduino Serial Monitor Window (Serial)
          if (Serial1.available() > 0) 
          {
            incomingByte = char(Serial1.read());
            if((incomingByte == 'w')||(incomingByte == 'W'))
            //Forward
            {
              right_f = 1;
              left_f  = 1;
              right_r = 0;
              left_r  = 0;
              speed_r = 255*(rscale)/255;
              speed_l = 200*(lscale)/255;
              Serial1.println("Forward");
            }
            else if((incomingByte == 'a')||(incomingByte == 'A'))
            //Left Turn in Place
            {
              right_f = 1;
              left_f  = 0;
              right_r = 0;
              left_r  = 1;
              speed_r = 255*rscale/255;
              speed_l = 255*lscale/255;
              Serial1.println("Left");
            }
            else if((incomingByte == 's')||(incomingByte == 'S'))
            //Backward
            {
              right_f = 0;
              left_f  = 0;
              right_r = 1;
              left_r  = 1;
              speed_r = 255*rscale/255;
              speed_l = 255*lscale/255;
              Serial1.println("Backward");
            }
            else if((incomingByte == 'd')||(incomingByte == 'D'))
            //Right Turn in Place
            {
              right_f = 0;
              left_f  = 1;
              right_r = 1;
              left_r  = 0;
              speed_r = 255*rscale/255;
              speed_l = 255*lscale/255;
              Serial1.println("Right");
            }
            else if((incomingByte == 'x')||(incomingByte == 'X'))
            //Brake
            {
              right_f = 1;
              left_f  = 1;
              right_r = 1;
              left_r  = 1;
              speed_r = 0;
              speed_l = 0;
              Serial1.println("Stop");
            }
            else if((incomingByte == 'r')||(incomingByte == 'R'))
            //Reset Magnetometer (WOZ)
            {
              //Tell the HMC5883 where to begin reading data
              Wire.beginTransmission(address);
              Wire.send(0x03); //select register 3, X MSB register
              Wire.endTransmission();
              //Read data from each axis, 2 registers per axis
              Wire.requestFrom(address, 6);
              if(6<=Wire.available())
              {
                x_int = Wire.receive()<<8; //X msb
                x_int |= Wire.receive(); //X lsb
                z_int = Wire.receive()<<8; //Z msb
                z_int |= Wire.receive(); //Z lsb
                y_int = Wire.receive()<<8; //Y msb
                y_int |= Wire.receive(); //Y lsb
              }
              Serial1.println("Reset");
            }
            digitalWrite(rfpin, right_f);
            digitalWrite(rrpin, right_r);
            digitalWrite(lfpin, left_f);
            digitalWrite(lrpin, left_r);
            analogWrite(speed_rpin, speed_r);
            analogWrite(speed_lpin, speed_l);
            
              
            // Print to Arduino Serial Monitor Window
            Serial.print("UART received: ");
            Serial.println(incomingByte);
            //Serial.write(incomingByte);
            //Serial.println();
                  
            // Echo to Bluetooth Receiver
            Serial1.print("UART received:");
            Serial1.println(incomingByte);
            //Serial1.write(incomingByte);
            //Serial1.println();
    }
  
    if (abs(x) > threshold)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      digitalWrite(ledPin, LOW);
    }
      
      
       if (abs(y) > threshold)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      digitalWrite(ledPin, LOW);
    } 
      
      
      if (abs(x) > threshold)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      digitalWrite(ledPin, LOW);
    }  
    delay(20);

}
