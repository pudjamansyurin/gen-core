#!/bin/bash

source ./credential.env

SRC_BIN_FILE="../VCU-APP/Release/VCU-APP.bin"
APP_BIN_FILE="APP.bin"
CRC_STR_FILE="CRC.txt"
CRC_BIN_FILE="CRC.bin"
CRC_APP_FILE="CRC_APP.bin"

echo "Files preparing..."
rm -rf $APP_BIN_FILE $CRC_APP_FILE $CRC_STR_FILE $CRC_BIN_FILE
cp $SRC_BIN_FILE $APP_BIN_FILE
#objcopy -I binary -O binary --reverse-bytes=4 $SRC_BIN_FILE "tmp.bin"

echo "Checksum calculating..."
CRC_RESULT=$(java -jar ./jacksum.jar -a crc32_mpeg2 -E hexup -F "#CHECKSUM" $APP_BIN_FILE)
echo $CRC_RESULT > $CRC_STR_FILE
#xxd -r -p $CRC_STR_FILE $CRC_BIN_FILE
xxd -r -p $CRC_STR_FILE | xxd -e -g4 | xxd -r > $CRC_BIN_FILE

echo "Combine checksum to binary file..."
cat $CRC_BIN_FILE $APP_BIN_FILE > $CRC_APP_FILE

echo "FTP binary uploading..."
curl -T $CRC_APP_FILE ftp://ftp.genmotorcycles.com/vcu/ --user "$USERNAME:$PASSWORD" || exit 1

echo "FTP done, ready for FOTA."

