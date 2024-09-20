# Version 1 (The Internship)

In this project, I tried to design a monitoring project using simple components using which the power consumption of different electrical appliances can be monitored and also switched on or off. The project is based on Arduino and we have used Arduino UNO for the project and are going to Monitor Home Appliances using Bluetooth Module and appropriate sensors for current and voltage sensing. 

Oorja is built around an Arduino Uno board. There exist two separate circuits one for Transmitter and one for Receiver.

Transmitter :-

> The transmitter acts as a combination of power supply for the AC devices and as a multimeter.

> It uses ACS712 Current Sensor and ZMP101B Voltage Sensor to measure the current through the appliance connected and the voltage drop across it and in turn calculate the power consumed by the appliance.

> These sensors outputs the readings into the “analog” Pins of the Arduino where then the data is interpreted and transmitted to the receiver via the HC05 Bluetooth module in a form which is easily readable and displayable using the 16x2 LCD.

Receiver :-

> The receiver is basically a glorified Monitor which acts as a wireless display for the “Multimeter” that is the transmitter.

> It consists of 16x2 I2C LCD display and HC05 Bluetooth Module.

> The data received from the transmitter circuit is interpreted and displayed on the 16x2 LCD using the I2C communication bus.


Also the receiver has the capability to send the transmitter either an ON or an OFF signal in order to wirelessly switch the Home appliance which is interpreted at the transmitter end and implemented using a 5v relay module.
