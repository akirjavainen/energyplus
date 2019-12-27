 /*
******************************************************************************************************************************************************************
*
* Energy+ protocol remote control capture
* Compatible with remotes like 50121 and power plugs like 51107
* Sold by Tokmanni in Finland
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
* 
* Use this code to capture the commands from your remotes. Outputs to serial
* (Tools -> Serial Monitor). At the moment this code only captures the
* first 25 bit command sequence, as the latter 33 bits are not actually
* required for commands to work.
* 
* The trouble with capturing both sequences is that pulseIn(), Arduino or 
* the receivers I'm using cannot seem to switch fast enough from listening
* to LOW pulses (AGC) to HIGH (command bits). There are of course multiple
* ways to get around this and once I figure a nice and clean way to implement
* it, I'll update the code. For now, I opted for cleaner code.
* 
* 
* HOW TO USE
* 
* Plug a 433.92MHz receiver to digital pin 2 and start pressing buttons
* from your original remotes (copy pasting them to EnergyPlus.ino).
*
******************************************************************************************************************************************************************
*/



// Plug your 433.92MHz receiver to digital pin 2:
#define RECEIVE_PIN   2

// Enable debug mode if there's no serial output or if you're modifying this code for
// another protocol/device. However, note that serial output delays receiving, causing
// data bit capture to fail. So keep debug disabled unless absolutely required:
#define DEBUG         false
#define ADDITIONAL    false    // Display some additional info after capture



// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  int i = 0;
  unsigned long t = 0;
  String command = "";



  // *********************************************************************
  // Wait for the AGC:
  // *********************************************************************
  // HIGH between 300-500 us
  // *********************************************************************

  while (t < 300 || t > 500) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // / Waits for HIGH and times it

    if (DEBUG) { // For finding AGC timings
      Serial.println(t);
    }
  }

  if (DEBUG) {
    Serial.print("AGC: ");
    Serial.println(t);
    //return; // If modifying this code for another protocol, stop here
  }


  
  // *********************************************************************
  // Command bits, locate them simply by HIGH waveform spikes:
  // *********************************************************************
  // First command sequence (the one we're reading with this code):
  // 0 = 250-450 us
  // 1 = 1000-1300 us
  // *********************************************************************
  // Second command sequence (we're ignoring this for now):
  // 0 = 350-570 us
  // 1 = 1400-1650 us
  // *********************************************************************

  while (i < 25) {
    t = pulseIn(RECEIVE_PIN, HIGH, 1000000); // Waits for HIGH and times it
  
    if (DEBUG) {
      Serial.print(t);
      Serial.print(": ");
    }
  
    if (t > 250 && t < 450) { // Found 0
      command += "0";
      if (DEBUG) Serial.println("0");
        
    } else if (t > 1000 && t < 1300) { // Found 1
      command += "1";
      if (DEBUG) Serial.println("1");
        
    } else { // Unrecognized bit, finish
      if (ADDITIONAL) {
        Serial.print("INVALID TIMING: ");
        Serial.println(t);
      }
      
      i = 0;
      break;
    }
  
    i++;
  }


 
  // *********************************************************************
  // Done! Display results:
  // *********************************************************************  

  // Correct data bits length is 25 or 33 bits, dismiss bad captures:
  if (command.length() != 25 && command.length() != 33) {
  
    if (ADDITIONAL) {
      Serial.print("Bad capture, invalid command length ");
      Serial.println(command.length());
      Serial.println();
    }
    
  } else {
      Serial.println("Command is: " + command);

      // We'll pause here to avoid accidentally reading the second
      // command sequence:
      Serial.println("Pausing for 1 second...");
      delay(1000);
      Serial.println("OK, receiving again!");
      Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
