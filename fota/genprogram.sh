#!/bin/bash

source .prog.env

echo "SWD connecting..."
sleep 1
$CLI -c port=SWD 

echo "Full flash erasing..."
sleep 1
$CLI -c port=SWD --erase all
 
echo "Bootloader downloading..."
sleep 1
$CLI -c port=SWD --write $BL_BIN_FILE $BL_START_ADDR 

echo "Application downloading..."
sleep 1
$CLI -c port=SWD --write $APP_BIN_FILE $APP_START_ADDR 
 
echo "Application:CRC writing..."
sleep 1
#APP_CRC=0x$(cat $CRC_TXT_FILE)
#$CLI -c port=SWD -w32 $APP_CRC_ADDR $APP_CRC
$CLI -c port=SWD --write $CRC_BIN_FILE $APP_CRC_ADDR

echo "Application:Size writing..."
sleep 1
APP_SIZE=$(printf "0x%08x" `stat -c %s "$APP_BIN_FILE"`)
$CLI -c port=SWD -w32 $APP_SIZE_ADDR $APP_SIZE

echo "Application running..."
sleep 1
$CLI -c port=SWD -swv freq=$SYS_FREQ_MHZ portnumber=0 "swv.log" 

