# VCU / HUB

## Software Progress:
- [x] Add IWDG to VCU 
- [x] ~~Decrease System Clock to 100MHz~~ 
- [x] Edit file database.c data type to structure 
- [x] Move all constant (config variable) to config.h file 
- [x] ~~Increase the baud-rate of all communication protocols~~ 
- [x] Pass pointer of chars from Simcom_Response() to Simcom_Send() 
- [x] De-structure "stm32f4_discovery_audio.c" and "stm32f4_discovery.c" 
- [x] Handle AT Command of new SIM5300e 
- [x] SIM5300e TCP communication 
- [x] Add SIMCOM signal strength CAN and the display 
- [x] Handle command from Server to IOT 
- [x] ~~MQTT Protocol with backend server~~ 
- [x] ~~Try to lock SIMCOM on 3G network, then fallback to 2G if failed continuously.~~ 
- [x] Handle interrupted restart of SIMCOM module 
- [x] Day names in RTC is not editable yet 
- [x] Handle SIMCOM logs array when offline 
- [x] ~~EEPROM Emulation ? How about external EEPROM modules?~~
- [x] Handle HMI Secondary CAN 
- [x] Handle RTC reset when (no BMS & Li-ION empty), need sync RTC 
- [x] Standardize frame data which sent to server, ignore ATrack frame. 
- [x] Change HAL_Delay() to osDelay() 
- [x] ~~Remove DMA_handler files~~ 
- [x] Remove any un-necessary array, use pointer if possible for simplicity 
- [x] Calibrate audio volume 
- [x] Encode Command Frame from Server to IOT 
- [x] Calibrate simcom signal level from db to linear 
- [x] ~~Set audio volume to max at first, then set to default. Because the external speaker has AGC.~~
- [x] Check ACK on every REPORT / RESPONSE frame. If not receive ACK response, resend (2 times), then restart the SIMCOM if still failed. 
- [x] Check COMMAND before clearing the Buffer 
- [x] Slow down report time when no BMS (5x longer)
- [x] Fix SIMCOM auto restart when receive Command from server
- [x] Handle "Phone Plugged Status" from HMI-2 
- [x] Handle DTR pin for SIM5300e (sleep mode) 
- [x] Use hierarchy algorithm to handle simcom error 
- [x] Use double buffering for ADC-DMA (HT & TC)
- [x] Fix moving average divider before full
- [x] Fix Odometer estimation based on GPS speed
- [x] Handle TCP Closed backend, it should not auto reset
- [x] Calibrate the ADC of Backup Battery Monitor
- [x] Flush BMS data when timeout
- [x] Migrate Firmware RTOSv1 to RTOSv2
- [x] RTOS: add master thread
- [x] Use Trip A & B
- [x] Sync variable name for HMI-1 and VCU
- [x] ~~Scale down "BMS Voltage", and set warning on 20% value.~~
- [x] Fix CRC not valid problem between VCU and Server
- [x] Change Notify() to RTOSv2 API
- [x] ~~Fix duplicate variables~~
- [x] use EVT_ERROR & EVENT_ERROR for ThreadFlag and EventFlag
- [x] Move routines in CommandTask to its own Task
  - [x] Implement ThreadFlags to handle that
- [x] Simplify Log Debug as a function
- [x] Build dedicated AT Command library for SIMCOM
- [x] Simplify SIMCOM library
- [x] Re-calibrate I2S PLL Clock after migration to F423
- [x] Record RTOS high water mark stack space
- [x] Check SYS_LED blinking continuously
- [x] Re-Calculate CRC for looped NACK in Response Frame
- [x] Handle NO Driver ID when vehicle OFF
- [x] Handle migration from STM32F407VG to STM32F423VH 
  - [x] Use AES features in new STM32F423VH 
  - [x] Use AES for NRF24LE1 communication
  - [x] User RNG to generate random AES-KEY
  - [x] Use NRF-FLASH to store permanent AES-KEY
  - [x] Build communication handshake between NRF & VCU
- [x] Re-swap the CAN data between SwitchModulControl & MixedData
- [?] Handle add new fingerprint error when use different finger for confirmation.
- [ ] Add new fingerprint can only be executed when KNOB is ON (HMI1 ON)
- [ ] E-Scooter Diagram:
  - [?] HMI.Primary Fingerprint indicator blink when Scanning
  - [?] Try to add more than 5 fingerprint user
  - [ ] Handle keyless lost
  	- [ ] Only when knob ON
    - [ ] HMI.Primary Keyless blink
    - [ ] Send event group to Server
  - [ ] Add finger throttle  and events, max 6 times failed within 30 second
  - [?] Keyless.BEEP turn on Horn & Sein Lamp (toggle the HORN_PWR)
  - [ ] Handle Starter Button
  - [ ] Handle Gyroscope crash & fall to switched OFF BMS & MCU
  - [?] Handle BMS.Fan when BMS temperature is overheat	
  - [?] Handle SOLENOID_PWR by Keyless.Seat
  - [?] VCU Lost Mode / Independent (NO BMS more thant 2 minutes)
    - [?] Send event "Unauthorized Battery Removal" 
    - [ ] Sleep every 1 hour, just wakeup to Send Data
    - [ ] When wakeup, only turn ON: VCU, GPS & SIMCOM
  - [x] ~~MCU Power control with GPIO ?~~
    - [x] Controlled by BMS_WAKEUP (by 63V power)
  - [x] SteerLock & Kickstand use GPIO of KNOB_IRQ
  - [ ] MCU related
    - [ ] Handle sound volume based on MCU RPM
    - [ ] Drive mode handle MCU related parameters
    - [ ] HMI.Primary 'warning' & 'overheat' based on MCU
- [ ] Make routine to check SIMCOM internet package
- [x] Build bootloader for FOTA
- [x] Merge bootloader & application project
- [x] ~~Separate audio assets with FTP download~~.
  - [x] ~~Use last 2 sector of FLASH for its space~~. 
- [ ] Something wrong with I2S DMA Data Width (it should be Half-Word, not Byte) 
- [ ] SIM5300e SSL communication, maybe MbedTLS can be used.
- [x] ~~Lock all global variable to minimize RTOS problem.~~ 
- [ ] RTOS: give timeout for any osWaitForever
- [x] ~~Configure UBLOX if you order the IC separately, so build the library for it~~
  - [x] ~~Library: https://github.com/sparkfun/SparkFun_Ublox_Aacrduino_Library~~
- [x] Simplify CANBUS library with pointer data

## Hardware Progress:
- [x] Add SMD Fuse 
- [x] ~~Research Low Power STM32 for IoT~~ 
- [x] Change STM32 from LQFP100 (to many unused pins), and has AES feature 
- [x] Control HMI Primary power (by MOSFET) 
- [x] Remove un-necessary LED 
- [x] Watch capacitor position 
- [x] Change component position in schematic as it used to be 
- [x] Check decouple capacitor type (tantalum) for SIMCOM and other vcc modules 
- [x] See the RF PCB design guide rules 
- [x] Add holes for screw 
- [x] Add Solder Jumper for every module 
- [x] Remove RTC battery, can you? 
- [x] Make serial jumper for ublox (using U-CENTER), for programming the IC 
- [x] Add optional solder jumper for JACK connector 
- [x] B- of Li-ION on schematic ? 
- [x] Change all led to side (like ATrack) 
- [x] Add Idd jumper to check the current for each sub module 
- [x] Add 2 GPIO for HMI Secondary: WAKE (OUT), SHUTDOWN (OUT) 
- [x] Add  2 GPIO for HMI Secondary: Phone Connected (IN), RTC Daylight (OUT) 
- [x] Handle interrupt of fingerprint using PNP Transistor as switch. 
- [x] Add DTR pin for SIM5300e 
- [x] Check TX/RX cross over for UART 
- [x] Change VCU GPIOS pin in excell 
- [x] Change INT_FINGER to EXT_FINGER 
- [x] Add solder jumper to switch between MCU 
- [x] Check all transistor for power control (PNP or NPN) 
- [x] MOSFET for Power Control not same 
- [x] Change all Power Control transistor to EN pin of each Regulator 
- [x] Find best SIMCOM Regulator 
- [x] ~~Check resistor in every bias transistor~~ 
- [x] Check without CR2032 battery, but use Li-iON instead 
- [x] Add voltage divider after fuse to check BMS power 
- [x] Add uFL connector directly for each RF Antenna 
- [x] Add alternative built-in NRF24L01 module 
- [x] All GPIO Output to HMI2 should be 3.3v max 
- [x] Handle "Spare Pins" converter mosfet/optocoppler 
- [x] Handle 1 GPIO_IN left optocopler 
- [x] Add big decouple capacitor for SIMCOM pwr, NRF pwr (it fixes auto restart) 
- [x] Thinner the PCB height 
- [x] Add "label name" for all solder jumper 
- [x] Add marker for Pick & Place tool 
- [x] Add many via (through hole) for best RF performance  

### PCB (Hardware) Revision:
- [x] Check crystals footprint: HCLK, RTC
- [x] ~~Move all solder jumpers at the top layer~~
- [x] Make screw corner hole bigger in 4 positions
- [x] ~~Add Voltage-divider for 5V line as Main Power Monitor~~
- [x] Fix Q4 & Q7 shielded pad
- [x] ~~Swap GPS & NET leds position~~
- [x] Change R67 value as R63
- [x] Change R23 value to 30k
- [x] Change C30, C31, C70, C71 value to 20pf
- [x] Change Led Color to RED (not GREEN)
- [x] Transistor Problem:
  - [x] Vbase should equal to Vemitter to turn it OFF
  - [x] Change Q3, Q5, Q9, Q13, Q6 to TRANSISTOR PNP
  - [x] Increase the base resistor to 510 Ohm
- [x] VCU-core:
  - [x] Add buzzer & the GPIO pin
  - [x] Add recharge-able coin battery for RTC
   - [x] VBAT should use this
   - [x] Use diode & resistor to limit current
  - [x] Handle "different signal" trace properly (CAN, MIC_N&P)
- [x] The Power Trace should be wider to fix voltage drop
  - [x] Trace between B+ to SIMCOM's VBAT at least 60mil
  - [x] Also trace for HMI1 & HMI2 Power Control
- [x] CAN Module:
  - [x] Give Resistor 120 Ohm between CAN_H and CAN_L (optional, just give the space)
  - [x] Add MOSFET power control and the GPIO pins
- [x] Connector:
  - [x] Remove un-necessary GPIO pin to HMI-2 (replaced by CAN) 
  - [x] Make 6 pin connector same (Fingerprint & CAN)
  - [x] Change audio jack connector
  - [x] Give more space between each connector
  - [x] Replace downloader connector with pin-header (3*2)
  - [x] Give GND pin for HMI1 & HMI2
  - [x] Put HMI1 & HMI2 side-by-side
- [x] EEPROM:
  - [x] Change the footprint
- [x] Li-ion Charger:
  - [x] **Charger is not working!!!!**
  - [x] B+ only for: MCU, GPS, SIMCOM & EEPROM
  - [x] ~~Add solder jumper (connect to B+ or 5V) for other components not listed above~~
  - [x] Add switch to disable the Li-ion battery
  - [x] Add solder switch to charge / discharge li-ion
- [x] Gyro:
  - [x] Add separate 3v3 regulator
  - [x] Connect INT pin to MCU (give jumper)
  - [x] Connect AD0 pin to GND (the problem)
  - [x] Separate Q4 GND and GYRO_GND (in schematic)
- [x] GPS:
  - [x] Add many vias arround RF trace
  - [x] Do not cover antenna ground
  - [x] Fix eagle footprint problem, the un-shielded ground near every pins
  - [x] Add Matching Network between on-board Antenna and Solder-Jumper (for next optimization)
  - [x] Add solder jumper for TX pin between VCU & Ublox chip
  - [x] Connect V_BCKP to VCC (not VDD)
  - [x] Give jumper for GPS_IDD, it can use B+ or 5V from usb2serial
  - [x] Add recharge-able coin battery
   - [x] Use diode & resistor to limit current
  - [x] Increase C90 value to 100uF (tantalum)
  - [x] Use EXTINT pin to control Sleep mode
    - See section "9.3.2.7 EXTINT pin control" in "Receiver Description" datasheet
- [x] Fingerprint:
  - [x] Change Q6 from NPN to PNP
  - [x] EXT_FINGER_TOUCH_PWR change Collector trace from B+ to Vdd
- [x] SIMCOM:
  - [x] Add many vias arround RF trace
  - [x] Add Matching Network between on-board Antenna and Solder-Jumper (for next optimization)
  - [x] Remove R24 & R25, instead connect "LED_NET" directly to MOSFET's gate pin.
  - [x] Add MOSFET between INT_NET_RST (VCU) and RESET (SIMCOM).
  - [x] Re-swap the USIM_VDD and USIM_DATA pin.
  - [x] Change USIM holder.
  - [x] Change LTC output from 3.8v to 4.2v
    - [x] Give solder jumper to connect LTC or B+ (directly)
    - [x] then add MOSFET/PNP power control (used RUN pin) between B+ and VBAT
  - [x] Increase C42 value to 1000uF (tantalum)
  - [x] Move SIMcard related components closer to the SIMcard-holder
- [x] AUDIO:
  - [x] Connect pin 41 to GND (bellow the chip)
  - [x] Give GND hole (un-isolated) bellow the chip.
  - [x] VL & VP pin should use its own 3v3 regulator (so add it), 
  - [x] and it must be controlled also using MOSFET or GPIO pin (directly)
- [x] Keyless:
  - [x] Increase C91 value to 100uF (tantalum)
  - [x] Preserve on-board NRF chip
  - [x] Add uFL connector for on-board chip like GPS & SIMCOM
  - [x] Change the on-board microstrip antenna to v2
  - [x] Add Solder-Jumper (for next optimization) to switch between uFL or microstrip
  
## RF PCB Guidelines : 
- http://iot-bits.com/simple-rf-pcb-layout-tips-tricks/ 
- http://www.pcbtechguide.com/2010/07/wifi-module-layout-guidelines.html 
- http://www.summitdata.com/blog/parasitic-effects-rf-design/ 

## Sub-Modules Progress:
| No | Sub Module                  | Chip           | ST-Periph.  | FW  | HW | Note		  	|
|:--:|-----------------------------|----------------|:-----------:|:---:|:--:|--------------------------|
|  1 | IoT                         | SIM5300e       | USART1	  | ✔  | ✔ | **Done**			|
|  2 | GPS                         | Ublox NEO-6M   | USART2	  | ✔  | ✔ | **Done**			|
|  3 | Gyroscope & Accelerometer   | MPU6050        | I2C3	  | ✔  | ✔ | **Done**			|
|  4 | Keyless/RF                  | nRF24L01 	    | SPI1	  | ✔  | ✔ | **Done**: Semi module	|
|  5 | Fingerprint                 | FZ3387         | UART4	  | ✔  | ✔ | **Done**			|
|  6 | RTC                         | -		    | RTC	  | ✔  | ✔ | **Done**			|
|  7 | Li-ION Charger & Protection | TP4056 & DW01A | -		  | ✔  | ✔ | **Done**			|
|  8 | Artificial Audio            | CS43L22        | I2C1, I2S3  | ✔  | ✔ | **Done**			|
|  9 | CAN Transceiver             | SN65HVD230     | CAN1	  | ✔  | ✔ | **Done**			|
| 10 | EEPROM                      | 24AA32A        | I2C2	  | ✔  | ✔ | **Done**			|
| 11 | Handlebar/Switch            | -		    | EXTI-PE	  | ✔  | ✔ | **Done**			|
| 12 | Data Validator	           | -      	    | CRC	  | ✔  | -  | **Done**			|
| 13 | Backup Battery Monitor      | -		    | ADC1	  | ✔  | ✔ | **Done** 		|
| 14 | Debugging Buzzer		   | -		    | TIM10-PWM	  | ✔  | ✔ | **Done** 		|
| 15 | Encryption RF		   | -		    | AES  	  | ✔  | -  | **Done**			|
| 16 | Encryption IoT		   | -		    | MbedTLS	  | 0%  | -  | Waiting: Real Server	|
| 17 | Fw upgrade OTA (FOTA) 	   | -		    | FLASH	  | ✔  | -  | **Done**			|
| 18 | Fw upgrade CAN (FOCAN)     | -		    | FLASH, CAN  | ✔  | -  | **Done**			|
