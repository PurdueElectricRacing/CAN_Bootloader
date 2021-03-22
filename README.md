# CAN Bootloader for STM32L4/F4 Devices
This bootloader will use a connected CAN bus to flash new application firmware to the device.This project was built on top to the PlatformIO project in order to simplify the build system and CMSIS dependency.

## Useful Links
- [STM32L43xx Reference Manual](https://www.st.com/resource/en/reference_manual/dm00151940-stm32l41xxx-42xxx-43xxx-44xxx-45xxx-46xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf) for Peripheral Info
- [STM32F429 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-415-stm32f407-417-stm32f427-437-and-stm32f429-439-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf) for Peripheral Info
---
## Getting Started


### Install PlatformIO
Before you clone this project, install PlatformIO (PIO) from one of the following ways:
#### 1. PIO IDE
PIO has a great extension for VSCode and you can do all of the development work from there. A guide from PlatformIO can be found [here](https://docs.platformio.org/en/latest/integration/ide/pioide.html), there is also support for CLion.
If you install PIO with this option, you will also be automatically installing the PIO CLI as the editor extensions just provide a nice interaction layer to the PIO tools.

#### 2. PIO CLI
If you want to use your own editor, PIO also has a command line interface for working with PIO projects, information can be found [here](https://docs.platformio.org/en/latest/core/installation.html) for setting that up.

### Clone & Build the project
    cd <per_git_directory>
    git clone git@github.com:PurdueElectricRacing/CAN_Bootloader.git
    cd CAN_Bootloader
    pio build --environment nucleo_l432kc

A good resource for learning how to use PlatformIO is from their documentation, the [Tutorials and Examples](https://docs.platformio.org/en/latest/tutorials/index.html) page has a lot of good content. Most of the videos on YouTube are Arduino-based projects, but all of the pio commands will be very similar to this project

## Unit Testing
PIO comes with easy integration with the [Unity](http://www.throwtheswitch.org/unity) unit testing framework for C. The `test` directory contains modules that can be run with the `pio test -e native` command. This will compile the `test\<module>\test_<component>.c` for your "native" desktop environment and does not require a microcontroller. 
Future unit tests can be created for execution on actual ARM hardware, but a large portion of state machine/data structure code can be tested on your local machine.   