# Control Energy+ (Tokmanni) 433.92MHz power plugs from Arduino

# How to use
1. Load up RemoteCapture.ino and plug a 433.92MHz receiver to digital pin 2.
2. Open up Tools -> Serial Monitor in Arduino IDE and start pressing the remote controller (button).
3. Copy paste the 25 bit commands to EnergyPlus.ino for sendEnergyPlusCommand().


# How to use with example commands
1. Set the power plug into pairing mode by holding down its power button for 3 seconds until the LED starts blinking.
2. Transmit ON command, eg. "sendEnergyPlusCommand(BUTTON_A_ON_SEQ1, BUTTON_A_ON_SEQ2);", which stops the LED from blinking (indicating successful pairing).


# Note about the second command sequence
RemoteCapture.ino only reads the first (25 bits long) command sequence at the moment, as the second (33 bits long) sequence is not required for controlling the switches. EnergyPlus.ino is, however, written to transmit both sequences should any device require it in the future.

