
# Buttmote

The aim of our project was to create a remote controller that will detect when a person sits or gets up on a sofa, turn on and off TV with an IR signal and control smart lights with user defined settings through WiFi using UDP packets containing a JSON message.

## Table of Contents

1. [How it works](#how-it-works)

2. [Technologies](#technologies)

3. [Enclosure](#enclosure)

4. [Electrical design](#electrical-design)

5. [Credits](#credits)

## How it works

The device is a made from 2 parts. A casing that is the brains and pressure mat designed to go under sofa cushion.

<img src="img/result1.png" width=250px>
<img src="img/result2.png" width=300px>

The device has these functionalities:

* Pressure mat: Detects when person has sat down on sofa
* IR reciever & sender: You can teach IR signals from your remote to be stored in device. This way the device can open up TV and for example your favorite channel when you sit down.
* Light controls: The device controls smart lights with 3 states that user can modify easily with buttons.
* LED: Indicator for user to see if device is functioning properly

Device has 3 phases:

* Standing
* Sitting
* Standup

### Standing

This is the normal mode. Lights are on, tv is off. (NOTE: user can modify light states and brightness on each phase. For clarity in this document Standing = full brightness, Sitting = lights off, Standup = dimmed lights)

### Sitting

When person sits down the pressure sensor is triggered and device goes into sitting mode. Here TV is turned on, lights are turned off and if person has favorite TV channel, TV is turned to that channel.

### Standup

This is the middle phase where there has been person sitting and then person stands up. We understand that people might want to go get snacks/drinks so it would be anyoying for TV to turn off and lights on. That is why we have customizable Standup phase where in user set time (for example 15 mins) the device will not turn TV off and it will turn on the lights with a dimm brightness, so that person can see to walk but doesent get blasted with light.

## Technologies

In this project we used the following technologies:

* KiCad: Electrical design
* Fusion 360: 3D modeling
* Creality K1 3D printer
* ESP32 Dev Board
* IR Receiver & Emitter
* Pressure Sensor
* Smart Light
* FreeRTOS
* WiFi & UDP

## Enclosure

The enclosure is a two part design and it is held together with four hidden M2 allen screws that thread into brass inserts. The screws also hold the circuit board in place as it is sandwiched between the two halves of the enclosure. The buttons use a compliant mechanism to improve the feel and to prevent accidental presses, as the tactile switches on the circuit board are quite sensitive. The enclosure has openings for the IR LED, IR receiver, indicator LED, power switch, USB cable and pressure sensor connector and those were glued in place.Labels for the buttons were embossed to the top surface for better user experience. The enclosure has cooling slots on its sides to ensure sufficient cooling of the chip and they also improve the looks. Silicone pads were glued to the bottom to help keep the device stationary during use.

<img src="img/enclosure.png" width=400px>


## Electrical design

The device is powered through a USB cable, which also serves as the interface for programming the ESP32. A solderable micro USB connector was connected to the USB connector on the board and the wires from the USB cable were soldered to it, 5V wire through the power switch. The IR receiver is powered with 3.3V and the data pin is connected to pin 12 on the ESP. The IR LED is connected to pin 4 and it has a 200 ohm resistor to limit the current. The indicator LED is connected to pins 25, 32 and 33 and there are 100 ohm resistors to prevent drawing too much current from the pins. There is 16 a 2 kohm potentiometer on the ground connection to be able to adjust the brightness of the LED to a suitable level. There are 4 buttons on the circuit board and those connect to pins 16, 17, 22 and 23 on the ESP. One of these was not used in the final design. The buttons have 10 kohm pullups and 100 nF capacitors for debouncing. The design was hand soldered to a FR-4 prototype board using through-hole components and 30 AWG silicone wire.

<img src="img/electricalDesign.png" width=800px>

## Credits

* [Josia Orava](https://github.com/JosiaOrava)
* [Miro Tuovinen](https://github.com/1UPNuke)
* [Jussi Enne](https://github.com/dredxster)
* [Ariana Karadzha](https://github.com/feepa)
