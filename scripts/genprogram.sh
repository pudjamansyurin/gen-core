#!/bin/bash

source .env
source .prog.env

source "./gencrc.sh"

$CLI -c port=SWD -r32 $VIN_ADDR 0x04
printf "VIN (above) readed from flash.\n"

printf "Replace VIN (in decimal) [or let empty to use old] : "
read VIN_INPUT

if [ -z $VIN_INPUT ]
then
      $CLI -c port=SWD --read $VIN_ADDR 0x04 $VIN_BIN_FILE
      printf "Use in-flash VIN, backed up to $VIN_BIN_FILE\n"
else
      printf "Use new VIN (Decimal) = $VIN_INPUT\n"
fi
sleep 2

$CLI -c port=SWD 
printf "SWD connected.\n"
sleep 2

$CLI -c port=SWD --erase all
printf "Flash erased.\n"
sleep 2
 
$CLI -c port=SWD --write $BL_BIN_FILE $BL_START_ADDR 
printf "Bootloader downloaded.\n"
sleep 2

$CLI -c port=SWD --write $APP_BIN_FILE $APP_START_ADDR 
printf "Application downloaded.\n"
sleep 2
 
$CLI -c port=SWD --write $CRC_BIN_FILE $APP_CRC_ADDR
printf "Application:CRC inserted.\n"
sleep 2

APP_SIZE=$(printf "0x%08x" `stat -c %s "$APP_BIN_FILE"`)
$CLI -c port=SWD -w32 $APP_SIZE_ADDR $APP_SIZE
printf "Application:Size inserted.\n"
sleep 2

if [ -z $VIN_INPUT ]
then
      $CLI -c port=SWD --write $VIN_BIN_FILE $VIN_ADDR 
      printf "Bootloader:Old VIN inserted.\n"
else
      VIN_HEX=$(printf "0x%08x" $VIN_INPUT)
      $CLI -c port=SWD -w32 $VIN_ADDR $VIN_HEX
      printf "Bootloader:New VIN inserted.\n"
fi
sleep 2

$CLI -c port=SWD -swv freq=$SYS_FREQ_MHZ portnumber=0 "swv.log" 

