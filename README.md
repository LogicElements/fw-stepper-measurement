# Precise RTD Simulator

This repo contains firmware and utilities for Precise RTD Simulator
 - Application firmware
 - Bootloader
 - Python class
 - Test suite
 - Documentation

# Register map

Modbus register map and internal memory structure is managed using Excel generator
./Documents/PrecRtdSim.xlsm. This worksheet contains several VBA scripts that
translates defined variables into firmware local storage definition, firmware modbus
register map, python class definition and json register map definition.

Some documentation can be found in ./Documents/wiki.


> [!NOTE]
> Opening excel worksheet with macros may show some kind of warning

# Programming bootloader

In order to flash bootloader firmware with serial number, proceed as follows:
- connect power and st-link
- edit ./Binary/STLink-program.bat with serial number prefix and binary names
- run `./Binary/STLink-program.bat x` where `x` is serial number to program 

Any further debugging can be done using IDE.

# Python tools

There are python support tools in the directory ./Tests/prec-rtd-dim/.
In order to use VisualModbus application, proceed as follows:

- make sure python >= 3.7 is installed and accessible via PATH
- install requirements `pip install -r requirements.txt`
- edit communication settings in ComSettings.json (mainly COM port)
- run visual application `python VisualRtdSim.py`



