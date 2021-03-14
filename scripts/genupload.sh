#!/bin/bash

source .env
source .upload.env

source "./gencrc.sh"

echo "FTP binary uploading..."
$CURL -T $CRC_APP_FILE ftp://ftp.genmotorcycles.com/vcu/ --user "$USERNAME:$PASSWORD" || exit 1

echo "FTP done, ready for FOTA."

