Arduino-Code-for-Nasal-Cannula in C++
**Project for Duke Engineers for International Development 2025
**
Cannula Dislodgement Monitor (Theristor-Based)
Description

This repository contains Arduino firmware for a cannula dislodgement monitoring system using a thermistor, visual indicators, an audible alarm, and a 16x2 I2C LCD. The system monitors thermistor readings in real time, establishes a calibrated baseline, and detects sustained drops in temperature consistent with cannula dislodgement.

The firmware implements debounced user input, timed calibration, threshold-based detection, latched fault handling, and continuous status reporting via both serial output and LCD display.

**System Overview**

The thermistor is sampled at fixed intervals and averaged during a calibration phase to establish a baseline reading. During normal operation, the system compares live readings against a configurable threshold derived from this baseline. If a sustained drop below the threshold is detected, the system enters a latched alarm state that requires manual user intervention to reset and recalibrate.

**Detection Logic**

A baseline thermistor value is established during a 5-second calibration period.

Live readings are compared against 99.5% of the calibrated baseline.

A dislodgement event is confirmed only after 10 consecutive readings below the threshold.

Once confirmed, the alarm and indicator LED remain active until the user initiates recalibration.

**User Interaction**

A push button is used to:

Stop an active alarm

Reset detection counters

Initiate recalibration

Button input is debounced in software to prevent false triggering.

A calibration LED provides visual feedback during baseline acquisition.

**Display Output
**
A 16x2 I2C LCD provides real-time system feedback:

Line 1:

Current raw thermistor analog reading

Line 2:

Normal operation: Count X/10

Alarm state: DISLODGED!

Serial Output

The serial interface outputs:

Calibration progress

Baseline and threshold values

Live thermistor readings

Detection counters

Alarm state messages

**This output is intended for debugging, validation, and tuning during development.
**
Hardware Configuration
Pin Assignments
Function	Pin
Thermistor Input	A0
Alarm Output	D8
Dislodgement LED	D11
Calibration LED	D12
Power LED	D13
User Button	D7
LCD Interface	I2C (SDA/SCL)
Timing Parameters
Parameter	Value
Calibration Duration	5000 ms
Sample Interval	50 ms
Monitoring Interval	500 ms
Button Debounce	50 ms
Build & Dependencies

Arduino IDE

LiquidCrystal_I2C library

Arduino-compatible microcontroller (e.g., Uno, Nano)

Ensure the LCD I2C address matches your hardware (0x27 by default).

Limitations

Uses raw ADC values rather than calibrated temperature units.

Threshold is fixed at compile time.

No persistent storage for calibration data.

Not intended for clinical or safety-critical deployment.

**Disclaimer**

This firmware is provided for educational and prototyping purposes only. It is not currently validated and approved for medical use.
**
