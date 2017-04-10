/**************************

  Copyright (c) 2014 Rui Azevedo

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

**********************
Virtual Pins Extension
enumerating external pins for use as regular pins
virtualize device connection to avoid library change due to change in connecting media.
*/

#include "Arduino.h"

#define USE_VIRTUAL_PINS//allow disabling of this feature for lib test purposes

#ifdef USE_VIRTUAL_PINS
	#ifndef VIRTUAL_PINS_DEF
		#define VIRTUAL_PINS_DEF

		#define VPINS_OPTIMIZE_SPEED
		//#define VPINS_OPTIMIZE_RAM

		//number of 8bit ports to use
		#define VPINS_PORTS 4
		//allocated memory size (in bytes)
		#define PORTREGSZ 3
		#define VPINS_SZ (VPINS_PORTS*PORTREGSZ)//we are using 3 bytes per port
		extern char vpins_data[VPINS_SZ];//and this is the memory for it (12 bytes= 32in+32out)
		//max number of protocol stacks
		#define branchLimit 8
		#define NOT_A_BRANCH -1
		#ifdef VPINS_OPTIMIZE_SPEED
			extern char port_to_branch[];
		#endif

		#define DDR_VPA (vpins_data+0)
		#define PORT_VPA (vpins_data+1)
		#define PIN_VPA (vpins_data+2)
		#define DDR_VPB (vpins_data+3)
		#define PORT_VPB (vpins_data+4)
		#define PIN_VPB (vpins_data+5)
		#define DDR_VPC (vpins_data+6)
		#define PORT_VPC (vpins_data+7)
		#define PIN_VPC (vpins_data+8)
		#define DDR_VPD (vpins_data+9)
		#define PORT_VPD (vpins_data+10)
		#define PIN_VPD (vpins_data+11)

		#define VP0 20
		#define VP1 21
		#define VP2 22
		#define VP3 23
		#define VP4 24
		#define VP5 25
		#define VP6 26
		#define VP7 27

		#define VP8 28
		#define VP9 29
		#define VP10 30
		#define VP11 31
		#define VP12 31
		#define VP13 33
		#define VP14 34
		#define VP15 35

		#define VP16 36
		#define VP17 37
		#define VP18 38
		#define VP19 39
		#define VP20 40
		#define VP21 41
		#define VP22 42
		#define VP23 43

		#define VP24 44
		#define VP25 45
		#define VP26 46
		#define VP27 47
		#define VP28 48
		#define VP29 49
		#define VP30 50
		#define VP31 51

		#define VPA 13
		#define VPB 14
		#define VPC 15
		#define VPD 16

		//utility macros
		#define vp_on(x) digitalWrite(x,1)
		#define vp_off(x) digitalWrite(x,0)
		#define vp_tog(x) digitalWrite(x,!digitalRead(x))
		#define vp_pulse(x) {vp_tog(x);vp_tog(x);}
		#define NATIVE_PORTS digitalPinToPort(NUM_DIGITAL_PINS)
		#define NATIVE_PORT(x) (x<=digitalPinToPort(NUM_DIGITAL_PINS)))

		//Virtual pin numbers by using virtual ports
		//virtual pin 35 = VP15 = VP(VPA,15) = VP(VPB,7) = vpA(15) = vpB(7)
		#define VP(port,pin) (NUM_DIGITAL_PINS+(port-VPA)*8+pin)
		#define vpA(pin) (VP(VPA,pin))
		#define vpB(pin) (VP(VPB,pin))
		#define vpC(pin) (VP(VPC,pin))
		#define vpD(pin) (VP(VPD,pin))

		#ifdef USE_VIRTUAL_PINS
			#ifdef __cplusplus
			extern "C" {
			#endif
			//void vpins_begin(char lp,char regsz);
			void vpins_init();
			void vpins_mode(char port);
			void vpins_in(char port);
			void vpins_out(char port);
			void vpins_io(char port);//use portmap to dispatch network port (includes SPI)
			#ifdef __cplusplus
			}
			#endif
		#endif

	#endif

	#ifdef __cplusplus
		#ifndef VIRTUAL_PINS_CPP_DEF
			#define VIRTUAL_PINS_CPP_DEF

			#define NOBRANCH ((portBranch*)0)
			class portBranch;
			//extern portBranch* tree[branchLimit];
			//extern portBranch* debug_branch;

			class portBranch {
			friend void vpins_init();
			protected:
				static bool vpins_running;
			public:
				char index;
				bool active;//branch mounted ok?
				char size;//number of ports on this chain (must be sequential)
				char localPort;//local port nr, it can be a virtual port :D
				portBranch(char port, char sz);
				//portBranch(char sz);
				virtual ~portBranch();
				inline static bool running() {return portBranch::vpins_running;}
				inline bool hasPort(char port) {return port>=localPort && port<(localPort+size);}
				//inline
				int pin(int p);// {return 20+((localPort-VPA)<<3)+p;}//NUM_DIGITAL_PINS not available here? damn weird compiling schema!
				/*static inline int freePort() {
					for(int i=0;i<ports_limit;i++) if (port_to_Branch[i]}*/
				static char getBranchId(char port);
				static portBranch& getBranch(char port);
				//this functions kick data in/out of the virtual ports
				//on SPI (and duplex protocols) io is always called
				//on other protocols we have advantage of calling either in or out
				//port mode is only sent out for network ports
				//default branch type does nothing (ideal for representing reah hardware ports)
				virtual void mode();
				virtual void in();
				virtual void out();
				virtual void io();
			};

		#endif
	#endif
#endif
