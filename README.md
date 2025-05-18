# CANTranslate

[![License: MIT & CERN-OHL-P-2.0](https://img.shields.io/badge/License-MIT%20%26%20CERN--OHL--P--2.0-blue.svg)](LICENSE.MD)

## Overview

CANTranslate is a bridge solution that enables seamless communication ArduPilot devices using DroneCAN protocol and ODrive motor controllers using CANSimple protocol. It translates CAN bus messages between these two protocols in real-time, allowing for integration of these otherwise incompatible systems.

## Features

- Bidirectional translation between DroneCAN and CANSimple protocols
- Custom STM32F103 Based Hardware
- Support for critical message types from both protocols

## Hardware

CANTranslate runs on a custom PCB based on:

- STM32F103C8 microcontroller
- SN65HVD230 CAN Transciever

## Software

The project is written in C with the following components:

- Protocol parsers for both DroneCAN and CANSimple
- Message translation logic
- STM32 HAL CAN peripheral

## Protocol Support

### DroneCAN Messages

- [X] Node status messages
- [ ] Actuator commands 
- [ ] Sensor feedback

### CANSimple Messages

- [ ] Motor commands
- [ ] Encoder feedback
- [ ] Controller status
- [ ] Error codes

## License

This project uses dual licensing:

- **Software**: The software is licensed under the MIT License.
- **Hardware**: The hardware design files (schematics, PCB layouts, etc.) are licensed under the CERN Open Hardware License v2 - Permissive.

See the `LICENSE.md` file for the complete license text for both components.

## Contact

For questions and support, please open an issue on the GitHub repository.