/*
EnergyPulseReader.ino
2013-09-16 <jpa458 at gmail dot com> http://opensource.org/licenses/mit-license.php

Energy Meter LED Pulse Counter using JeeNode v6 and JeeLab Lux Plug containing a TSL2561. 

The sketch detects the LED pulse and increments a counter. Every so often the counter is broadcast using the JeeNode RF12B module

I actually wanted to use the interrupt mechanism built into the TSL2561 chip but can't get that working yet.
I could also have used an LDR with a voltage divider and an interrupt - the TSL2561 is probably overkill - but this got me up and running with the standard hardware.

The only difficulty is to calibrate the software cycle with the duration of the pulse.
With my energy meter (ISKRA ME161) and a cycle of 50ms, the led is lit for 6-8 cycles during a pulse.

We loop every 50ms(LOOP_TIME) and increment the counter (triggerCount) for every light reading above a certain threshold (CHECK_LIGHT_LEVEL).
I am useing the IR reading of the TSL2561.

I consider 4 (VALID_PULSE_COUNT_THRESHOLD) software cycles enough to confirm I am reading an actual pulse.
After that I ignore any further readings for another 4 cycles (IGNORE_LED_LIT_THRESHOLD).
At which point I clear the triggerCount and start all over again.
I introduced this mechanism to manage minor inconsitencies in the duration of the LED pulse. 

Once we have been counting for a given period (MEASUREMENT_PERIOD) we broadcast the pulse count

Finally, my JeeNode is running off a power supply so there are no power saving steps - such as powering down the RF12B. 
If you are running off a battery you probably want to be doing this.

*/

#include <JeeLib.h>
#include <Time.h>    
PortI2C myBus (1);
LuxPlug sensor (myBus, 0x39);

#define DEBUG  0

//light level that triggers a pulse
const int CHECK_LIGHT_LEVEL=80;

//this is our pulse count
int pulseCount=0;

//how long ago did we broadcast the last pulse count
time_t lastSentTimestamp;

//how often do we broadcast the pulse count (in seconds)
const int MEASUREMENT_PERIOD= 60*5;//5mn

//software loop delay in ms
const int LOOP_TIME=50;

//count the number of times we "think" a pulse is firing
int triggerCount=0;

//how many triggerCounts does it take to confirm a pulse
const int VALID_PULSE_COUNT_THRESHOLD=4;

//
//Once we confirm a pulse we want to ignore further readings for a short while
//
//How many software loops do we ignore the "lit" reading 
const int IGNORE_LED_LIT_THRESHOLD = 4;
//keep track of how many loops we have been ignoring the led 
int ignoreLedLitCycleCount=0;
//enables/disables the clearing mechanism.
boolean ignoreLedLit=false;

void setup () {
    #if DEBUG
      Serial.begin(57600);
      Serial.println("\n[EnergyPulseSensor]");
    #endif

    rf12_initialize(1, RF12_868MHZ, 100);
    sensor.begin();
    delay(500); 
    lastSentTimestamp= now();
}

void loop () {    
    
  delay(LOOP_TIME);
  
  if (ignoreLedLit && ignoreLedLitCycleCount < IGNORE_LED_LIT_THRESHOLD) {
    #if DEBUG
      Serial.println("Ignoring LED");
    #endif
    
    ignoreLedLitCycleCount++;
  } else if (ignoreLedLit) {
      #if DEBUG
        Serial.println("Stop ignoring LED");
      #endif
      
      ignoreLedLit=false;
      ignoreLedLitCycleCount=0;
  } else {   
      #if DEBUG
        Serial.print("Time since last :");
        Serial.println(now()-lastSentTimestamp);
      #endif

      const word* photoDiodes = sensor.getData();

      #if DEBUG
        Serial.print("IR light level :");
        Serial.println(photoDiodes[1]);
      #endif

      if (photoDiodes[1] > CHECK_LIGHT_LEVEL) {
        #if DEBUG
          Serial.println("Level is high enough.");
        #endif
        triggerCount++;
      }
      
      if (VALID_PULSE_COUNT_THRESHOLD < triggerCount) {
        #if DEBUG
          Serial.println("Reading is valid.");
        #endif
        
        pulseCount++;
        triggerCount=0;
        ignoreLedLit=true;//start ignoring LED for a bit
      }
       
      if(now() - lastSentTimestamp >= MEASUREMENT_PERIOD ){
        #if DEBUG
          Serial.print("PULSE COUNT:");
          Serial.println(pulseCount);
        #endif

        // actual packet sent: broadcast to all, current counter
        rf12_sendNow(0, &pulseCount, sizeof pulseCount);
        rf12_sendWait(1);
        pulseCount=0;          
        lastSentTimestamp = now();
      }
   }   

}
