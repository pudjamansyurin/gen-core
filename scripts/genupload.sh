#!/bin/bash
source .env
source "./gencrc.sh"

echo "FTP binary uploading..."
$CURL -T $CRC_APP_FILE "ftp://ftp.garda-energi.com/$DEV/" --user "$FTP_USERNAME:$FTP_PASSWORD" || exit 1

echo "FTP done, ready for FOTA."

