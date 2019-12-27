#!/usr/bin/python

"""
* Energy+ power plugs
* Sold by Tokmanni
*
* Code by Antti Kirjavainen (antti.kirjavainen [_at_] gmail.com)
*
* This is a Python implementation for Energy+ devices, for
* the Raspberry Pi. Plug your transmitter to BOARD PIN 16 (BCM/GPIO23).
* This script only transmits the first 25 bit command sequence.
*
* HOW TO USE
* ./EnergyPlus.py [25-bit_binary_string]
*
* More info on the protocol in EnergyPlus.ino and RemoteCapture.ino here:
* https://github.com/akirjavainen/energyplus
*
"""

import time
import sys
import os
import RPi.GPIO as GPIO


TRANSMIT_PIN = 16  # BCM PIN 23 (GPIO23, BOARD PIN 16)
REPEAT_COMMAND = 6


# Microseconds (us) converted to seconds for time.sleep() function:
EP_INIT_AGC_PULSE = 0.0004
EP_SEQ1_AGC_PULSE = 0.00221

EP_SEQ1_PULSE_SHORT = 0.000335
EP_SEQ1_PULSE_LONG = 0.00119

EP_COMMAND_BIT_ARRAY_SIZE = 25


# ------------------------------------------------------------------
def sendEnergyPlusCommand(command):

    if len(str(command)) is not EP_COMMAND_BIT_ARRAY_SIZE:
        print("Your (invalid) command was", len(str(command)), "bits long.")
        print
        printUsage()

    # Prepare:
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(TRANSMIT_PIN, GPIO.OUT)

    transmitHigh(EP_INIT_AGC_PULSE)  # Initial AGC (only before first command repetition)

    # Send command:
    for t in range(REPEAT_COMMAND):
        doEnergyPlusTribitSend(command)

    # Disable output to transmitter and clean up:
    exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def doEnergyPlusTribitSend(command):

    # AGC bits:
    transmitLow(EP_SEQ1_AGC_PULSE)  # AGC

    for i in command:

        if i == '0':  # HIGH-LOW-LOW
            transmitHigh(EP_SEQ1_PULSE_SHORT)
            transmitLow(EP_SEQ1_PULSE_LONG)

        elif i == '1':  # HIGH-HIGH-LOW
            transmitHigh(EP_SEQ1_PULSE_LONG)
            transmitLow(EP_SEQ1_PULSE_SHORT)

        else:
            print("Invalid character", i, "in command! Exiting...")
            exitProgram()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitHigh(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.HIGH)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def transmitLow(delay):
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    time.sleep(delay)
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def printUsage():
    print("Usage:")
    print(os.path.basename(sys.argv[0]), "[command_string]")
    print
    print("Correct command length is", EP_COMMAND_BIT_ARRAY_SIZE, "bits.")
    print
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
def exitProgram():
    # Disable output to transmitter and clean up:
    GPIO.output(TRANSMIT_PIN, GPIO.LOW)
    GPIO.cleanup()
    exit()
# ------------------------------------------------------------------


# ------------------------------------------------------------------
# Main program:
# ------------------------------------------------------------------
if len(sys.argv) < 2:
    printUsage()

sendEnergyPlusCommand(sys.argv[1])

