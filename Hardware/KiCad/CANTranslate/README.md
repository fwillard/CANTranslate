# CANTranslate – Hardware Design

## Overview

The hardware design for **CANTranslate** facilitates physical interfacing and electrical translation between multiple CAN bus systems. This includes power management, bus isolation, transceivers, and microcontroller integration for reliable embedded CAN communication.

## Features

* Single SN65HVD230 CAN transceiver (CAN 2.0)&#x20;
* STM32F103C8-based microcontroller
* Power input via USB or 5V pin on the CAN header
* Power source selection using TPS2116DRL (USB VBUS prioritized)
* Debug interface (SWD)

## Hardware Design Files

The following files are included in the `/Hardware/KiCad/CANTranslate` directory:

* `CANTranslate.kicad_pro`
* `CANTranslate.kicad_sch` – Schematic file
* `CANTranslate.kicad_pcb` – PCB layout
* `CustomParts.kicad_sym` – Custom symbol library
* `KMR223G LFG.STEP` – 3D model of tactile switch
* `fabrication-toolkit-options.json` – Fabrication preferences
* `fp-lib-table` – Footprint library table
* `sym-lib-table` – Symbol library table

## Power Requirements
* Power Inputs:
    * 5V from USB VBUS (primary source, with ESD protection)

    * 5V from the CAN header power pin (secondary source, no ESD protection)

* Power Selection:
    * The board uses a TPS2116DRL power multiplexer to automatically select the power source. When both USB and CAN header power are connected, the TPS2116DRL prioritizes USB VBUS, ensuring safe and seamless switching without back-feeding either source.

* Voltage Regulation:
    * The selected 5V supply is regulated down to 3.3V using an AMS1117-3.3 linear voltage regulator, providing stable power for the STM32F103C8 MCU and the SN65HVD230 CAN transceiver.

* Protection Features:
    * The USB power input includes ESD protection to safeguard against electrostatic discharge events.

## Toolchain

* **EDA Software**: KiCad 9.x

## Licensing

Hardware design files are released under the **CERN Open Hardware License v2** - Permissive
