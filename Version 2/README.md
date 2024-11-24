# Version 2 (The Research Paper)

The version 2 of the project was based on ESP32 microcontroller. It is built using ESP32 microcontroller with PZEM-004T sensor. It is IoTenabled to provide real-time monitoring and analysis of power consumption through a web dashboard, mobile app, or a dedicated receiver. With extensive safety features, such as a glass fuse, a high-pitched alarm, and an automated power cutoff, the proposed system allows homes and businesses to optimize 
energy use and save expenses without sacrificing safety. 

<p align="center">
  <img src="https://github.com/user-attachments/assets/1eece639-d179-4592-9b79-54f70497c5ea" />
</p>

The proposed system contains two circuits, one for the transmitter and one for the receiver.

## Transmitter:

- The transmitter is the heart of the complete operation performed by the proposed system.
  
- The transmitter serves the dual purpose; as power socket for AC devices as well as a multimeter.
  
- It takes the mains supply from any socket and routes it to the connected appliance, plugged into the transmitter’s socket, via the PZEM-004T sensor. The sensor then measures the current through the appliance, voltage drop across it with the frequency, power drawn by the appliance and the power factor.
  
- The sensor then feeds these readings into the ESP32, via the Rx and Tx terminal, where the data is interpreted. This data then is sent to the Blynk IoT server using the internal WiFi provisioning.

<p align="center">
  <img src="https://github.com/user-attachments/assets/f22a1371-12f1-4011-89ec-b3d5ff413058" />
</p>

## Receiver:

- The receiver is a glorified Monitor acting as a wireless display for the “Multimeter” which is the transmitter in the proposed system.

- It consists of ESP32 for processing the data received from the transmitter, and two 16x2 I2C LCDs for display.
  
- The data received from the transmitter circuit is processed, interpreted and displayed on the LCDs using the I2C communication bus.

Also, it connects to the Blynk IoT server via the internal Wi-Fi provisioning.

<p align="center">
  <img src="https://github.com/user-attachments/assets/84b7f292-52a3-4ab5-94c4-5bc800701b15" />
</p>
