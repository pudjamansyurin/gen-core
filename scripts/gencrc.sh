#!/bin/bash
source .env

echo "Removing old files..."
rm -rf dist/
mkdir dist
sleep 2

echo "Calculating CRC..."
CRC_RESULT=$($JACKSUM -a crc32_mpeg2 -E hexup -F "#CHECKSUM" $APP_BIN_FILE)
echo $CRC_RESULT > $CRC_STR_FILE
$XXD -r -p $CRC_STR_FILE | $XXD -e -g4 | $XXD -r > $CRC_BIN_FILE
sleep 2

echo "Inserting CRC to bin file..."
cat $CRC_BIN_FILE $APP_BIN_FILE > $CRC_APP_FILE
sleep 2 

echo "CRC = 0x$CRC_RESULT"
sleep 2

