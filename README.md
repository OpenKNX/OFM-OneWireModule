OpenKNX - OneWireModule
===

Implementation of a knx 1-wire module with up to 90 channels based on the [knx stack](https://github.com/OpenKNX/knx), a fork from [thelsing](https://github.com/thelsing/knx).

Application description (including a feature overview) can be found [here](https://github.com/OpenKNX/OFM-OneWireModule/blob/v1/doc/Applikationsbeschreibung-Wire.md).

It supports the RP2040 version of the stack.

It is a PlatformIO project and needs a working ETS >=5.7 installed on the same PC.


### How to use this module

Uses 12 shared (static) KOs 0-11.
Uses 1 KO per channel

Designed for up to 90 channels. 

Depends on OGM-SensorDevices module.
Needs 
    -D WIREMODULE
in platformio.ini


