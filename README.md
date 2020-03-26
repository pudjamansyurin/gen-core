# VCU / HUB

## Software Progress:
- [x] Add IWDG to VCU 
- [ ] Handle migration from STM32F407VG to STM32F423VH 
- [ ] Use AES features in new STM32F423VH 
- [x] ~~Decrease System Clock to 100MHz~~ 
- [x] Edit file database.c data type to structure 
- [x] Move all constant (config variable) to config.h file 
- [x] ~~Increase the baud-rate of all communication protocols~~ 
- [x] Pass pointer of chars from Simcom_Response() to Simcom_Send() 
- [x] De-structure "stm32f4_discovery_audio.c" and "stm32f4_discovery.c" 
- [ ] Configure UBLOX if you order the IC separately, so build the library for it 
- [x] Handle AT Command of new SIM5300e 
- [x] SIM5300e TCP communication 
- [ ] SIM5300e SSL communication 
- [x] Add SIMCOM signal strength CAN and the display 
- [x] Handle command from Server to IOT 
- [x] ~~MQTT Protocol with backend server~~ 
- [x] ~~Try to lock SIMCOM on 3G network, then fallback to 2G if failed continuously.~~ 
- [x] Handle interrupted restart of SIMCOM module 
- [x] Day names in RTC is not editable yet 
- [x] Handle SIMCOM logs array when offline 
- [x] ~~EEPROM Emulation ? How about external EEPROM modules?~~
- [ ] Handle DTR pin for SIM5300e (sleep mode) 
- [x] Handle HMI Secondary CAN 
- [x] Handle RTC reset when (no BMS & Li-ION empty), need sync RTC 
- [x] Standardize frame data which sent to server, ignore ATrack frame. 
- [x] Change HAL_Delay() to osDelay() 
- [ ] Something wrong with I2S DMA Data Width (it should be Half-Word, not Byte) 
- [x] ~~Remove DMA_handler files~~ 
- [x] Remove any un-necessary array, use pointer if possible for simplicity 
- [ ] Slow down report time when no BMS 
- [x] Calibrate audio volume 
- [ ] Use hierarchy algorithm to handle simcom error 
- [x] Encode Command Frame from Server to IOT 
- [x] Calibrate simcom signal level from db to linear 
- [ ] Handle "Phone Plugged Status" from HMI-2 
- [x] ~~Set audio volume to max at first, then set to default. Because the external speaker has AGC.~~
- [x] Check ACK on every REPORT / RESPONSE frame. If not receive ACK response, resend (2 times), then restart the SIMCOM if still failed. 
- [x] Check COMMAND before clearing the Buffer 
- [ ] Lock all global variable to minimize RTOS problem. 
- [ ] Scale down "BMS Voltage", and set warning on 20% value.

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
- [ ] Remove un-necessary GPIO pin to HMI-2 (replaced by CAN) 
- [ ] Make 6 pin connector same: Fingerprint & CAN
- [ ] Check crystals footprint: HCLK, RTC
- [ ] Audio: Change jack connector
- [ ] Li-ion Charger: B+ only for: MCU, GPS, SIMCOM & CAN
- [ ] Li-ion Charger: Add switch to disable the Li-ion battery
- [ ] Gyro: Add separate 3v3 regulator
- [ ] Gyro: Connect INT pin to MCU (give jumper)
- [ ] Gyro: Connect AD0 pin to GND (the problem)
- [ ] Gyro: Separate Q4 GND and GYRO_GND
- [ ] GPS: Give jumper for serial to MCU
- [ ] GPS: Give jumper for V_BCKP, it can use VDD / VCC
- [ ] GPS: Give jumper for GPS_IDD, it can use B+ or 5V from usb2serial


## RF PCB Guidelines : 
- http://iot-bits.com/simple-rf-pcb-layout-tips-tricks/ 
- http://www.pcbtechguide.com/2010/07/wifi-module-layout-guidelines.html 
- http://www.summitdata.com/blog/parasitic-effects-rf-design/ 

## Sub-Modules Progress:
| No | Sub Module                  | Chip           | ST-Peripheral  | Firmware(%)  | Hardware     | Note		  							|
|:--:|-----------------------------|----------------|:--------------:|:------------:|:------------:|----------------------------------------|
|  1 | IoT                         | SIM5300e       | USART1		 |      95      | On Progress  | Waitting: Real server 					|
|  2 | GPS                         | Ublox NEO-6M   | USART2		 |      100     |   	✔ 	   | **Done**: Use long-cable antenna		|
|  3 | Gyroscope & Accelerometer   | MPU6050        | I2C3			 |      100     |  		✔	   | **Done**: AD0 pin should be grounded	|
|  4 | Keyless/RF                  | nRF24L01 (semi)| SPI1			 |      50      |       ✔      | **Done**: Use semi module				|
|  5 | Fingerprint                 | FZ3387         | UART4			 |      100     | On Progress  | 										|
|  6 | RTC                         | ST-RTC		    | -				 |      100     |       ✔      | **Done**								|
|  7 | Li-ION Charger & Protection | TP4056 & DW01A | -				 |      100     |       ✔      | **Done**								|
|  8 | Artificial Audio            | CS43L22        | I2C1, I2S3   	 | 	    100     |              |										|
|  9 | CAN Transceiver             | SN65HVD230     | CAN1			 |      100     |       ✔      | Done: Need to be validated				|
| 10 | EEPROM                      | 24AA32A        | I2C2			 |      100     |       ✔      | **Done**								|
| 11 | Handlebar/Switch            | ST-EXTI        | PE			 |      100     |       ✔      | Done: Need to be validated				|
| 12 | Backup Battery Monitor      | ST-ADC			| ADC1			 | 		0		| 		✔ 	   | Pending: Auxiliary						|
| 13 | Encryption IoT & RF		   | ST-AES			| -  			 | 		0 		| 		-	   | Waitting: Server & pocket keyless		|
| 14 | Firmware upgrade OTA (FOTA) | ST-FLASH		| - 			 |      0 		| 		-      | Waitting: Auxiliary					|