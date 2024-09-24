# STM32F103 Bootloader

A simple bootloader for STM32F103 microcontrollers that allows firmware updates over UART.

## Features

- Firmware update via UART.
- Compatible with STM32F103 series microcontrollers.
- Lightweight and easy to integrate.
- Customizable settings for different applications.

## Getting Started

### Prerequisites

- STM32F103 microcontroller.
- STM32CubeIDE or a compatible development environment.
- UART communication setup.

### Installation

1. **Clone the repository:**

   ```bash
   git clone https://github.com/Meltony163/Stm32f103Bootloader.git
   ```

2. **Open the project in STM32CubeIDE.**

3. **Build the project** and upload it to your STM32F103 microcontroller using your preferred method (ST-Link, USB, etc.).

### Flashing the Bootloader

To flash the bootloader onto your STM32F103:

1. Connect your microcontroller to your PC using a programmer (like ST-Link).
2. In STM32CubeIDE, select your target microcontroller and upload the bootloader firmware.

## Usage

To use the bootloader for firmware updates:

1. **Connect your STM32F103** microcontroller to your PC via UART.
2. Use a terminal application (like PuTTY or Tera Term) to send the firmware update command through the UART interface.
3. The bootloader will receive the new firmware and program it into flash memory.

### UART Command Protocol

- **Command Format:** 
  - `0xAA` - Start byte
  - `0x01` - Command type (e.g., firmware update)
  - `[data]` - Firmware data
  - `0x55` - End byte

## Configuration

### Configurable Parameters

You may need to adjust the configuration files based on your specific hardware setup. Check the `config.h` file for the following parameters:

- **BAUD_RATE**: Set the UART baud rate for communication.
- **FLASH_SIZE**: Define the size of the flash memory on your microcontroller.
- **FIRMWARE_START_ADDRESS**: Specify the address where the firmware should be programmed in flash.

## Troubleshooting

If you encounter issues while using the bootloader, consider the following:

- Ensure that the UART connection is properly established.
- Check the baud rate settings to ensure they match on both the microcontroller and the terminal application.
- Verify that the correct firmware is being sent.

## Contributing

Contributions are welcome! If you have suggestions for improvements or encounter any bugs, please open an issue or submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [STM32 Documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32f1-series.html)
- Community contributions and feedback.
