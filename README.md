# Arduino-VirtualPins proposal

## Resume

This is my proposal for slight change on arduino digital pins handling in order to provider user with virtual-pins.

This implementation is Arduino environment specific, but the concept of virtual pins is generic and can be applyed to all digital electronics and drivers that uses bit setting and with proper care to all port IO and Buses (SPI, I2C, Serial, Etc..)

Implementing abstraction of hardware connection making it seamless of the medium. With virtual pins we can map pins of I2C or SPI chips into the arduino environment and use them as common arduino pins.

This abstrations allows the usage of libraries that are build for direct pin interfacing like the standard arduino Liquid Cristal library to use an LCD wired over shift-registers or I2C bus, with no need to change the library.

**Blinking a LED wired on a shift register**

```c++
#include <Arduino.h>

/*
Virtual pins library
blinking a led wired over an SPI shift register
*/

#include <SPI.h>// <---<< we will use this midia
#include <VPinsSPI.h>//<-- with this driver
#include <debug.h>

#define STCP 9//stcp or latch pin
//use virtual pins over SPI shift registers as VPA (Virtual Port A)
SPIBranch spi(SPI,STCP,VPA,2);//<-- map 2 virtual ports starting at A (VPA+VPB)

//now we have an extra 16 output pins
//ranging from 20 to 35 (virtual pins)
#define LEDPIN 27

void setup() {
  SPI.begin();//<-- initialize the media
  pinMode(LEDPIN,OUTPUT);//<-- using a virtual pins the same way as an internal
  //we can pass it around to libraries, no need to recode your library.
}

void loop() {
  digitalWrite(LEDPIN,0);
  delay(500);
  digitalWrite(LEDPIN,1);
  delay(500);
}
```

## Possibilities

The virtualization of pins and protocol abstration is similar (at a lower level) to a network, where the arduino acts as a client (master) driving some hardware server (Slave) over a network medium (SPI, I2C, etc..).

With this base concept we can then make the arduino MCU act as a network server (slave) publishing its own harware pins over a network to be used by another client.

The slave only runs the VPortServer sketch while the client has the specific hardware driver and uses it as if it was wired to direct hardware pins.

This is valid on the arduino environment for libraries that use the arduino API to change pin status.
For libraries that do directport IO its a matter of converting them to use the VirtualPins port IO.

## Getting wild

We can use an arduino VPortServer listening to the serial port (not implemented yet) and the port the arduino environment to the PC (a board with no physical pins) but able to use virtual pins mapped to the arduino.
With this my PC can link in the LiquidCristal library, map it to virtual pins that correspond to the server wiring and have my PC printing to the Arduino LCD or reading pin status.

A server can also publish its own virtual pins...

## keeping it simple

This is a low level harware network, no arbitration is provided at this level (other than the present on the used medium), so this is not indicated for concurrent access to a device.

I leave that for another layer.
