#!/bin/sh
make uImage LOADADDR=0x30008000 -j8
cp arch/arm/boot/uImage ~/tftpboot/uImage

