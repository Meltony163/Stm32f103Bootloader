# Bootloader for STM32F103C8T6

This repository contains a bootloader implementation for the STM32F103C8T6 microcontroller, allowing for firmware updates over UART communication. The bootloader supports various commands for reading, writing, and erasing flash memory, as well as changing read protection levels.


## Features

- UART communication for firmware updates.
- Support for CRC verification to ensure data integrity.
- Commands for reading chip identification, getting bootloader version, and managing flash memory.
- Configurable read protection levels.

## Usage

1. Flash the bootloader onto your STM32F103C8T6 using a programmer (e.g., ST-Link).
2. Connect to the microcontroller via UART using a terminal program (like PuTTY or Tera Term).
3. Send commands to interact with the bootloader.

## Commands

| Command                         | Description                             |
|---------------------------------|-----------------------------------------|
| `CBL_GET_VER_CMD`              | Get bootloader version                  |
| `CBL_GET_HELP_CMD`             | Get help information                    |
| `CBL_GET_CID_CMD`              | Get Chip Identification Number          |
| `CBL_GET_RDP_STATUS_CMD`       | Get Read Protection Status              |
| `CBL_GO_TO_ADDR_CMD`           | Jump to a specified memory address      |
| `CBL_FLASH_ERASE_CMD`          | Erase specified flash memory            |
| `CBL_MEM_WRITE_CMD`            | Write data to memory                    |
| `CBL_CHANGE_ROP_Level_CMD`     | Change Read Out Protection Level        |
