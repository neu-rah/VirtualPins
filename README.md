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

### Possibilities

The virtualization of pins and protocol abstration is similar (at a lower level) to a network, where the arduino acts as a client (master) driving some hardware server (Slave) over a network medium (SPI, I2C, etc..).

With this base concept we can then make the arduino MCU act as a network server (slave) publishing its own harware pins over a network to be used by another client.

The slave only runs the VPortServer sketch while the client has the specific hardware driver and uses it as if it was wired to direct hardware pins.

This is valid on the arduino environment for libraries that use the arduino API to change pin status.
For libraries that do directport IO its a matter of converting them to use the VirtualPins port IO.

### Getting wild

We can use an arduino VPortServer listening to the serial port (not implemented yet) and the port the arduino environment to the PC (a board with no physical pins) but able to use virtual pins mapped to the arduino.
With this my PC can link in the LiquidCristal library, map it to virtual pins that correspond to the server wiring and have my PC printing to the Arduino LCD or reading pin status.

A server can also publish its own virtual pins...

### keeping it simple

This is a low level harware network, no arbitration is provided at this level (other than the present on the used medium), so this is not indicated for concurrent access to a device.

I leave that for another layer.

## Implementation

Added files:

- virtual_pins.h
- virtual_pins.cpp

to cores/arduino

### changes to wiring_digital.c file

Changes that have to be done in what concerns performance impact, as you can see its minimal.

**on pinMode:**

```c++
#ifdef USE_VIRTUAL_PINS
  if (pin>=NUM_DIGITAL_PINS) {//then its a virtual pin...
    vpins_mode(port);
  }
#endif
```

**on digitalWrite:**

```c++
#ifdef USE_VIRTUAL_PINS
  if (pin>=NUM_DIGITAL_PINS) {//then its a virtual pin...
    vpins_out(port);
  }
#endif
```

**on digitalRead:**

```c++
#ifdef USE_VIRTUAL_PINS
  if (pin>=NUM_DIGITAL_PINS)//then its a virtual pin...
    vpins_in(port);
#endif
```

### changes to main.c

initialize virtual pins maps

**on main function:**

```c++
#ifdef USE_VIRTUAL_PINS
  vpins_init();//initialize port/pin maps
#endif
```

### virtual_pins.h

#### extra RAM used

**12 bytes for 4 extra ports, 32 IO Pins**

```c++
extern char vpins_data[VPINS_SZ];//and this is the memory for it (12 bytes= 32in+32out)
```

**8 Bytes for the routing or 4 virtual ports**

This eventually can be made static.

```c++
portBranch* tree[branchLimit]={NOBRANCH,NOBRANCH,NOBRANCH,NOBRANCH};
```

#### Startup impact

Just the initialization of that 12 bytes.

```c++
void vpins_init() {
	if (portBranch::running()) return;
	for(char n=0;n<VPINS_SZ;n++)
		vpins_data[n]=0;
	portBranch::vpins_running=true;
}
```

### changes to variants/standard/pins_arduino.h

This part was coded for an older IDE, it still works but I have not revided it.

Total 160 bytes on flash for extending the maps with more 32 IO pins.

Here are most of the PROGMEM memory impact of virtual-pins changes.

I've been sticking to existing maps and ways of doing things (at least on the original IDE).

But i'm not dure if a more independent way of doing the map would be prefereable for porting this to other devices like due, stm, esp, etc... eventually making the list of virtual pins dynamic.

As it it makes easy to use virtual pins even if you use the arduino maps.

Also I have some spacing on ports maps to avoid bumping with existing ports on processors like 2560... but I'm not sure if it is still needed.

**extended port_to_mode_PGM map with:**
_24 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) DDR_VPA,
	(uint16_t) DDR_VPB,
	(uint16_t) DDR_VPC,
	(uint16_t) DDR_VPD,
#endif
```

**on port_to_output_PGM map:**
_24 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	NOT_A_PORT,//5
	NOT_A_PORT,//6
	NOT_A_PORT,//7
	NOT_A_PORT,//8
	NOT_A_PORT,//9
	NOT_A_PORT,//10
	NOT_A_PORT,//11
	NOT_A_PORT,//12
	(uint16_t) PORT_VPA,//13
	(uint16_t) PORT_VPB,//14
	(uint16_t) PORT_VPC,//15
	(uint16_t) PORT_VPD,//16
#endif
```

**on port_to_input_PGM map:**
_24 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	NOT_A_PORT,//5
	NOT_A_PORT,//6
	NOT_A_PORT,//7
	NOT_A_PORT,//8
	NOT_A_PORT,//9
	NOT_A_PORT,//10
	NOT_A_PORT,//11
	NOT_A_PORT,//12
	(uint16_t) PIN_VPA,//13
	(uint16_t) PIN_VPB,//14
	(uint16_t) PIN_VPC,//15
	(uint16_t) PIN_VPD,//16
#endif //USE_VIRTUAL_PINS
```

**on digital_pin_to_port_PGM map:**
_32 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	VPA,
	VPA,
	VPA,
	VPA,
	VPA,
	VPA,
	VPA,
	VPA,

	VPB,
	VPB,
	VPB,
	VPB,
	VPB,
	VPB,
	VPB,
	VPB,

	VPC,
	VPC,
	VPC,
	VPC,
	VPC,
	VPC,
	VPC,
	VPC,

	VPD,
	VPD,
	VPD,
	VPD,
	VPD,
	VPD,
	VPD,
	VPD,
#endif //USE_VIRTUAL_PINS
```

**on digital_pin_to_bit_mask_PGM map:**
_32 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	_BV(0), /* 20, PORT_VPA */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 28, port PORT_VPB */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 36, port PORT_VPC */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 44, port PORT_VPD */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
#endif
```

**on digital_pin_to_timer_PGM map:**
_32 bytes flash_

```c++
#ifdef USE_VIRTUAL_PINS
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
#endif
```
