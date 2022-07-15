#!/bin/sh

#Este es un script simple para generar el archivo binario con el formato apropiado y subirlo al micro
arm-none-eabi-objcopy -O binary spi-mems.elf spi-mems.bin
st-flash --reset write spi-mems.bin 0x8000000
