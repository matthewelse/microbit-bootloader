# micro:bit bootloader

The current primary aim is to get a fully-functional build of the bootloader using GCC, that can be used as a drop in replacement for the existing bootloader.

However, this needs to use a single bank of flash, as well as not requiring an init packet.

The current state of the project can be built by:

```bash
cd examples/dfu/bootloader/pca10028/single_bank_ble_s110/armgcc
make
```

This will then produce a hex file in `armgcc/_build`, which you can then run through `srec_cat` to produce a drop in replacement for the binary from before.

The `srec_cat` command is only necessary if you're producing a combined binary containing an application, since it sets the bootloader settings to indicate that an application is present.

```bash
srec_cat _build/nrf51422_xxac.hex -intel \
    -exclude 0x3FC00 0x3FC20 \
    -generate 0x3FC00 0x3FC04 -constant-l-e 0x01 4 \
    -generate 0x3FC04 0x3FC08 -constant-l-e 0x00 4 \
    -generate 0x3FC08 0x3FC0C -constant-l-e 0xFE 4 \
    -generate 0x3FC0C 0x3FC20 -constant 0x00 \
    -o _build/nrf51422_xxac.hex -intel
```

