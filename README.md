# ATTINY Power switch
This programmable MOSFET high side on/off power switch is based on the ATTINY85 chip and is intended for battery powered devices.

- 1-3 Lipo cells 
- Max operating voltage 16.0V (18V never exceed)
- Max current: needs to be tested, but up to 5A should be very safe. See MOSFET data sheet for info.
- Power consumption in "off" mode with BOD disabled: 3uA
- Power consumption in "off" mode with BOD enabled: 20uA

### Configurable features in config.h file:
- LED behaviour
- Low battery voltage threshold, LED warning
- Very low battery voltage threshold, power off
- Button press and hold for a set time for power on
- Button press and hold for set time for power off
- Timer, delayed power off
- Timer, after power has been on for set time, switch power off
- PB3 pin signal (to for example a Raspberry Pi) initiating controlled shut-down, then switch the power off after a set time delay
-  +lots of other possibilities...

### Battery/voltage features using button switch:
- Auto calibrating internal voltmeter
- Auto set number of cells


