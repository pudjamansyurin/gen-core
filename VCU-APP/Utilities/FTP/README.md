cd /home/geni/STM32CubeIDE/workspace_1.3.0/F407/Debug
sudo stm32flash -w F407.bin -v -S 0x8020000 -g 0x8020000 /dev/ttyUSB0 

st-flash erase
st-flash write HUB.bin 0x8020000

hexdump -v -e '1/4 "%08x\n"' APP.bin
