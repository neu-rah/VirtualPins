# Arduino-VirtualPins proposal

## Resume

This is my proposal for slight change on arduino digital pins handling in order to provider user with virtual-pins.

This implementation is Arduino environment specific, but the concept of virtual pins is generic and can be applied to all digital electronics and drivers that uses bit setting and with proper care to all port IO and Buses (SPI, I2C, Serial, Etc..)

Implementing abstraction of hardware connection making it seamless of the medium. With virtual pins we can map pins of I2C or SPI chips into the arduino environment and use them as common arduino pins.

This abstractions allows the usage of libraries that are build for direct pin interfacing like the standard arduino Liquid Cristal library to use an LCD wired over shift-registers or I2C bus, with no need to change the library.

This implementation is for all board variants that include the hardware/arduino/avr/variants/standard/pins_arduino.h file (as a demo/test).

tested on arduino nano and raw atmega328p.

**Blinking a LED wired on a shift register**

using SPI extension sketch at: [SPI IO Extension](http://www.r-site.net/site/struct.asp?sid=264041713_83195617&lang=en&at=//op[@id=%272989%27])

![SPI IO Extension](http://www.r-site.net/media/img/pic_5620.png)

```c++
#include <Arduino.h>

/*
Virtual pins library
blinking a led wired over an SPI shift register
*/

#include <SPI.h>// <---<< we will use this media
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

The virtualization of pins and protocol abstraction is similar (at a lower level) to a network, where the arduino acts as a client (master) driving some hardware server (Slave) over a network medium (SPI, I2C, etc..).

With this base concept we can then make the arduino MCU act as a network server (slave) publishing its own hardware pins over a network to be used by another client.

The slave only runs the VPortServer sketch while the client has the specific hardware driver and uses it as if it was wired to direct hardware pins.

This is valid on the arduino environment for libraries that use the arduino API to change pin status.
For libraries that do direct port IO its a matter of converting them to use the VirtualPins port IO.

### Getting wild

We can use an arduino VPortServer listening to the serial port (not implemented yet) and the port the arduino environment to the PC (a board with no physical pins) but able to use virtual pins mapped to the arduino.
With this my PC can link in the LiquidCristal library, map it to virtual pins that correspond to the server wiring and have my PC printing to the Arduino LCD or reading pin status.

A server can also publish its own virtual pins...

### keeping it simple

This is a low level hardware network, no arbitration is provided at this level (other than the present on the used medium), so this is not indicated for concurrent access to a device.

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

This part was coded for an older IDE, it still works but I have not revised it.

Total 160 bytes on flash for extending the maps with more 32 IO pins.

Here are most of the PROGMEM memory impact of virtual-pins changes.

I've been sticking to existing maps and ways of doing things (at least on the original IDE).

But I'm not sure if a more independent way of doing the map would be preferable for porting this to other devices like due, stm, esp, etc... eventually making the list of virtual pins static user allocable instead of the fixed 4 ports.

As it is, it makes easy to use virtual pins even if you use the arduino maps.

so thing like this

```c++
DDRD|=1<<2;
DR_VPA|=1<<2;
```

are still valid for virtual ports as they are for hardware ports.

Direct IO on virtual ports stills requires a call to the ports/branch dispatch function.

Also I have some spacing on ports maps to avoid bumping with existing ports on processors like 2560... but I'm not sure if it is still needed.

Depending on the strategy selected this will probably change to more adequate maps for each board.

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

## Extension

In addition to this modifications, some libraries have to be included to interface virtual pins with medium type.

currently available:

- [VPinsSPI](https://github.com/neu-rah/VPinsSPI)
- [VPinsI2C](https://github.com/neu-rah/VPinsI2C)

Covering the cases of generic SPI IO and I2C output boards

[This board](http://www.ebay.com/itm/IIC-I2C-TWI-SPI-Serial-Interface-Board-Module-Port-for-Arduino-1602LCD-Display-/161245616356?hash=item258afcd8e4:g:nf0AAOSwZQRYgIMA) work with the generic drive, but providing a customized version allows the usage of some board features like setting pins as input or activating pull-ups.

![LCD adaptor based on PCF8574T](https://ecs7.tokopedia.net/img/product-1/2016/5/2/7117852/7117852_235e964b-1e42-4fac-a795-9c8ff42f2184.jpg)

Based on
[PCF8574T: Remote 8-bit I/O expander for I²C‑bus with interrupt](http://www.nxp.com/products/interfaces/ic-bus-portfolio/ic-general-purpose-i-o/remote-8-bit-i-o-expander-for-icbus-with-interrupt:PCF8574T)

This specialization can be provided by a derived class, specific to the hardware/family.

The same for [8-Bit I/O Expander with SPI Interface](https://www.eeweb.com/company-news/microchip/8-bit-io-expander-with-spi-interface)

So we can use a lot of devices with default media implementations and can customize specific devices to take full advantage of extra features.

Because the media types are less then the number of devices that can be attached to it and because most media can work with default implementation, I think its a good trade for not having to rewrite libraries.

**Other libs**

- [VPortServer](https://github.com/neu-rah/VPortServer)

This lib provides the avr server version, allowing the AVR to share its pins over the I2C bus.

This file was not ported from the old version yet and i expect it to be **a mess** right now.

## Considerations

Having a user specified amount of virtual ports brings along the concept of software ports. That is, ports that do not interface to hardware but instead correspond to memory to be exported from a particular sketch. Allowing data/record sharing over a network.

> This is very interesting and its just a matter of deepening a bit more on the virtual pins side and break away from the current model of following AVR maps.
> The maps mmay stioll exist for AVR's but checking pin range on entry rules out the virtual pins cases
> It is much more abrangent, because all boards define the number ou native hardware pins
> and it only move the current performance burden from the end of the functions to the beggining.
> I can live with that.

use pin 0 to denote, not used (disables future usage of pin0/reset for other purposes)

> Pin still available on drirect IO if you really nead it
> Make pinMode and digital read/write ignore the request on pin 0
> (require extra boillerplate)

signal reverse pin logic by using negative numbers

> Negate the logic on negative pins (require extra boillerplate)

Extend this to PCINT (is there an arduino standard way of doing PCInt?) if not the see [PCINT Library](https://github.com/neu-rah/PCINT)

because some of extension boards support pin direction, pull-ups, reverse logic and even interrupt lines (like the PCINT schema)

PCINT here can be software emulated for non PCINT supporting extensions like the raw shift registers.

> only if we have a standard (arduino) way of doing it

However current implementation does not allow exploitation of all the features on an abstract level. Just because we started with AVR pin/ports.

> We get what we can presserving compatibility.

What about analog inputs? there are pin extension like MCP3208, should we virtualize analog IO too?

> Maybe another day
