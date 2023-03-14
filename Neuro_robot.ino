//#include <SoftwareSerial.h>
#include "Otto.h"
Otto Otto;

#define LeftLeg 8 // left leg pin, servo[0]
#define LeftFoot 9 // left foot pin, servo[2]
#define RightLeg 10 // right leg pin, servo[1]
#define RightFoot 11 // right foot pin, servo[3]



//SoftwareSerial BT(2,3); //Rx/Tx
#define BAUDRATE 57600


// system variables
byte  generatedChecksum = 0;
byte  checksum = 0; 
int   payloadLength = 0;
byte  payloadData[64] = {0};
byte  attention = 0;
byte  meditation = 0;
boolean bigPacket = false;
int nCntAtt = 0;
int nCntMed = 0;

void setup()   {
 // BT.begin(BAUDRATE);               // Software serial port  (ATMEGA328P)
  Serial.begin(BAUDRATE);
  Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true, 13);
  Otto.home();
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

////////////////////////////////
// Read data from Serial UART //
////////////////////////////////
byte ReadOneByte() {
  int ByteRead;
  int nCount = 0;
  while(!Serial.available()) {
    nCount++;
    // hard reset, if MW unaviable
    if (nCount > 1000) {
      return 0;
    }
  }
  ByteRead = Serial.read();
  return ByteRead;
}

/////////////
//MAIN LOOP//
/////////////
void loop() {

  attention = 0;
  meditation = 0;
  bigPacket = false; 

  // Look for sync bytes
  if (ReadOneByte() == 170) {
    if (ReadOneByte() == 170) {
      payloadLength = ReadOneByte();

      //Payload length can not be greater than 169
      if (payloadLength > 169) return;

      generatedChecksum = 0;        
      for (int i = 0; i < payloadLength; i++) {  
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }   
      checksum = ReadOneByte();                      //Read checksum byte from stream      
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

      if (checksum == generatedChecksum) {    
        attention = 0;
        meditation = 0;
        bigPacket = false;  
        
        // Parse the payload
        for (int i = 0; i < payloadLength; i++) {    
          switch (payloadData[i]) {
          case 2:
            i++;            
            bigPacket = true;            
            break;
          case 4:
            i++;
            attention = payloadData[i];                        
            break;
          case 5:
            i++;
            meditation = payloadData[i];
            break;
          case 0x80:
            i = i + 3;
            break;
          case 0x83:
            i = i + 25;      
            break;
          default:
            break;
          }
        }

        // *** Add your code here ***
        if (bigPacket) {
          // put to OLED
       /*   Serial.print("Attention ");
          Serial.println(attention);
          Serial.print("Meditation ");
          Serial.println(meditation);*/
          //move forward
          if (meditation > attention) {
            nCntMed++;
            nCntAtt = 0;
            digitalWrite(4, LOW);
            digitalWrite(5, HIGH);
            if (nCntMed > 3) {
              //Serial.println("move forward");
             Otto.walk(3, 750, 1);
            }
          }
          //move backward
          if (meditation < attention) {
            nCntMed = 0;
            nCntAtt++;
            digitalWrite(4, HIGH);
            digitalWrite(5, LOW);
            if (nCntAtt > 3) {
        //      Serial.println("move back");
             Otto.walk(3, 750, -1);
            }
          }
        }
      }
    }
  }
  
}
