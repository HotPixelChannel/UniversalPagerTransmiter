# UniversalPagerTransmiter
![Preview](previ.jpg)

Universal POKSAG message transmitter for one pager or group of pagers. All work with the device is carried out according to the UART protocol and any terminal. Assembled on Arduino Nano and RF7021SE transmitter module

Based on: https://github.com/SinuXVR/arduino-pocsag-transcoder

The ADF7021 (or RF7021SE module) must operate with a 14.7456 MHz TCXO and with at least 2.5 ppm of frequency stability or better. You could use also 12.2880 MHz TCXO. Any other TCXO frequency is not supported.
For working on 145-160MHz needs be added by an external 18 nH inductor between L1 and L2 pins of ADF7021.
