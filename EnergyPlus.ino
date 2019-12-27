/*
******************************************************************************************************************************************************************
*
* Energy+ 433.92MHz power plugs
* Compatible with remotes like 50121 and power plugs like 51107
* Sold by Tokmanni in Finland
* 
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is just an implementation to capture and repeat the commands from Energy+ remotes. No effort to reverse engineer
* the codes has been made. Remotes send a different code on every press, indicating some type of timestamp or rolling code
* + checksum. What's strange is that one captured code works every time, thus it's not your typical rolling code.
* 
* 
* HOW TO USE
* 
* Capture your remote controls with RemoteCapture.ino and copy paste the 25 bit commands to EnergyPlus.ino for
* sendEnergyPlusCommand(). More info about this provided in RemoteCapture.ino.
* 
* 
* HOW TO USE WITH EXAMPLE COMMANDS
* 
* 1. Set the plug into pairing mode by holding down its power button for about 3 seconds (LED begins to blink).
* 2. Transmit ON command, eg. "sendEnergyPlusCommand(BUTTON_A_ON_SEQ1, BUTTON_A_ON_SEQ2);" and the LED will stop blinking.
* 
* 
* PROTOCOL DESCRIPTION
* 
* Tri-state bits are used.
* A single command is: 2 AGC bits + 6 times 25 command tribits (sequence 1) + 1 AGC bit + 4 times 33 command tribits (sequence 2)
* The second sequence command bits are not actually required at all and you could just transmit 33 zeroes.
*
* All sample counts below listed with a sample rate of 44100 Hz (sample count / 44100 = microseconds).
*
* So from start to finish:
* Initial AGC: HIGH of approx. 18 samples = 408 us
* AGC: LOW of approx. 97 samples = 2222 us
* 
* 1st command sequence of 25 data bits, repeated 6 times, with pulse lengths:
* SHORT: approx. 15 samples = 340 us
* LONG: approx. 53 samples = 1202 us
* 
* AGC: LOW of approx. 315 samples = 7143 us
* 
* 2nd command sequence of 33 data bits, repeated 4 times, with pulse lengths (this sequence is not required):
* SHORT: approx. 21 samples = 476 us
* LONG: approx. 67 samples = 1520 us
* 
* Data bits:
* Data 0 = short HIGH, long LOW (wire 100)
* Data 1 = long HIGH, short LOW (wire 110)
* 
* 
* HOW THIS WAS STARTED
* 
* Project started with a "poor man's oscilloscope": 433.92MHz receiver unit (data pin) -> 10K Ohm resistor -> USB sound card line-in.
* Try that at your own risk. Power to the 433.92MHz receiver unit was provided by Arduino (connected to 5V and GND).
*
* To view the waveform Arduino is transmitting (and debugging timings etc.), I found it easiest to directly connect the digital pin (13)
* from Arduino -> 10K Ohm resistor -> USB sound card line-in. This way the waveform was very clear.
* 
* Note that a PC sound cards may capture the waveform "upside down" (phase inverted). You may need to apply Audacity's Effects -> Invert
* to get the HIGHs and LOWs correctly.
* 
******************************************************************************************************************************************************************
*/



// Example commands to try (or just capture your own remotes with RemoteCapture.ino).
// 2nd sequence commands are not actually required; you could just send zeroes and the switches will still work.
#define BUTTON_A_ON_SEQ1                  "1101100100110111001100000"
#define BUTTON_A_ON_SEQ2                  "110110000101001101101101100100110"
#define BUTTON_A_OFF_SEQ1                 "1101011110010101001000000"
#define BUTTON_A_OFF_SEQ2                 "000110010011111101101000110100110"

#define BUTTON_B_ON_SEQ1                  "1101111101100100011101000"
#define BUTTON_B_ON_SEQ2                  "101110010111111110000111100101010"
#define BUTTON_B_OFF_SEQ1                 "1101111010101001100101000"
#define BUTTON_B_OFF_SEQ2                 "100110010100101001101111100101010"

#define BUTTON_C_ON_SEQ1                  "1101001010110011101111000"
#define BUTTON_C_ON_SEQ2                  "010010001011110110101001110110110"
#define BUTTON_C_OFF_SEQ1                 "1101011110010101001011000"
#define BUTTON_C_OFF_SEQ2                 "110000001110110010000001100110110"

#define TRANSMIT_PIN                       13   // We'll use digital 13 for transmitting
#define REPEAT_COMMAND                      1   // How many times to repeat the same command: original remotes repeat the whole sequence only once (1st sequence 6 times, 2nd sequence 4 times)
#define DEBUG                           false   // Do note that if you print serial output during transmit, it will cause delay and commands may fail

// If you wish to use PORTB commands instead of digitalWrite, these are for Arduino Uno digital 13:
#define D13high | 0x20; 
#define D13low  & 0xDF; 

// Timings in microseconds (us). Get sample count by zooming all the way in to the waveform with Audacity.
// Calculate microseconds with: (samples / sample rate, usually 44100 or 48000) - ~15-20 to compensate for delayMicroseconds overhead.
// Sample counts listed below with a sample rate of 44100 Hz:
#define EP_INIT_AGC_PULSE                     400   // 18 samples
#define EP_SEQ1_AGC_PULSE                     2210  // 97 samples
#define EP_SEQ2_AGC_PULSE                     7130  // 315 samples

#define EP_SEQ1_PULSE_SHORT                   335   // 15 samples
#define EP_SEQ1_PULSE_LONG                    1190  // 53 samples
#define EP_SEQ2_PULSE_SHORT                   465   // 21 samples
#define EP_SEQ2_PULSE_LONG                    1510  // 67 samples

#define EP_SEQ1_ARRAY_SIZE                    25    // First sequence length
#define EP_SEQ2_ARRAY_SIZE                    33    // Second sequence length


// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  Serial.begin(9600); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {

  sendEnergyPlusCommand(BUTTON_A_ON_SEQ1, BUTTON_A_ON_SEQ2);
  delay(3000);
  sendEnergyPlusCommand(BUTTON_A_OFF_SEQ1, BUTTON_A_OFF_SEQ2);
  delay(3000);

}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendEnergyPlusCommand(char* sequence1, char* sequence2) {

  if (sequence1 == NULL || sequence2 == NULL) {
    errorLog("sendEnergyPlusCommand(): Command array pointer was NULL, cannot continue.");
    return;
  }

  // Prepare for transmitting and check for validity
  pinMode(TRANSMIT_PIN, OUTPUT); // Prepare the digital pin for output
  
  if (strlen(sequence1) != EP_SEQ1_ARRAY_SIZE) {
    errorLog("sendEnergyPlusCommand(): Invalid command length (sequence 1 needs to be 25 bits long), cannot continue.");
    return;
  }
  if (strlen(sequence2) != EP_SEQ2_ARRAY_SIZE) {
    errorLog("sendEnergyPlusCommand(): Invalid command length (sequence 2 needs to be 33 bits long), cannot continue.");
    return;
  }
  
  // Repeat the whole command:
  for (int i = 0; i < REPEAT_COMMAND; i++) {

    transmitHigh(EP_INIT_AGC_PULSE);

    // Repeat 1st sequence 6 times:
    for (int i = 0; i < 6; i++) {
      doEnergyPlusTribitSend(sequence1, EP_SEQ1_ARRAY_SIZE, EP_SEQ1_AGC_PULSE, EP_SEQ1_PULSE_SHORT, EP_SEQ1_PULSE_LONG);
    }

    // Repeat 2nd sequence 4 times:
    for (int i = 0; i < 4; i++) {
      doEnergyPlusTribitSend(sequence2, EP_SEQ2_ARRAY_SIZE, EP_SEQ2_AGC_PULSE, EP_SEQ2_PULSE_SHORT, EP_SEQ2_PULSE_LONG);
    }
  }

  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // disrupting most appliances operating on the 433.92MHz band:
  digitalWrite(TRANSMIT_PIN, LOW);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void doEnergyPlusTribitSend(char* command, int command_length, int pulse_agc, int pulse_short, int pulse_long) {

  // Starting (AGC) bit:
  transmitLow(pulse_agc);

  // Transmit command:
  for (int i = 0; i < command_length; i++) {

      // If current bit is 0, transmit short HIGH, long LOW (100):
      if (command[i] == '0') {
        transmitHigh(pulse_short);
        transmitLow(pulse_long);
      }

      // If current bit is 1, transmit long HIGH, short LOW (110):
      if (command[i] == '1') {
        transmitHigh(pulse_long);
        transmitLow(pulse_short);
      }   
   }

  if (DEBUG) {
    Serial.println();
    Serial.print("Transmitted ");
    Serial.print(command_length);
    Serial.println(" bits.");
    Serial.println();
  }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitHigh(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, HIGH);
  //PORTB = PORTB D13high; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmitLow(int delay_microseconds) {
  digitalWrite(TRANSMIT_PIN, LOW);
  //PORTB = PORTB D13low; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void errorLog(String message) {
  Serial.println(message);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
