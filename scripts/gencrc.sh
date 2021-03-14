#!/bin/bash

source .env

echo "Files preparing..."
rm -rf $APP_BIN_FILE $CRC_APP_FILE $CRC_STR_FILE $CRC_BIN_FILE
cp $SRC_BIN_FILE $APP_BIN_FILE

echo "Checksum calculating..."
CRC_RESULT=$(java -jar ./jacksum.jar -a crc32_mpeg2 -E hexup -F "#CHECKSUM" $APP_BIN_FILE)
echo $CRC_RESULT > $CRC_STR_FILE
$XXD -r -p $CRC_STR_FILE | $XXD -e -g4 | $XXD -r > $CRC_BIN_FILE

echo "Combine checksum to binary file..."
cat $CRC_BIN_FILE $APP_BIN_FILE > $CRC_APP_FILE

echo "CRC calculation done."

