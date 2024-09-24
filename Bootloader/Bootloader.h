#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/********************************************Library Include Start********************************************/
// Standard Libraries
#include <stdio.h>
#include "STD_TYPES.h"   // Standard type definitions
#include <string.h>      // String manipulation functions
#include <stdarg.h>      // Variable argument list handling
#include "usart.h"       // USART communication functions
#include "crc.h"         // CRC calculation functions
/********************************************Library Include End********************************************/

/**************************************Bootloader Macros Declaration Start**************************************/
// Define ports for debugging and communication
#define DEBUGING_PORT                          &huart2      // Debugging port
#define COMMUNICATION_PORT                     &huart3      // Communication port for bootloader commands

// Define CRC engine for data integrity checks
#define CRC_ENGINE                             &hcrc        // CRC handler

// Acknowledgment constants
#define NACK                                   0xAB        // Negative acknowledgment
#define ACK                                    0xCD        // Positive acknowledgment

// Debugging settings
#define UART_DEBUG                             1u          // UART debugging enabled
#define BUFFER_SIZE                             200u        // Buffer size for data transmission

// Flash memory base addresses
#define FLASH_SECTOR2_BASE_ADDRESS             0x08008000U // Base address for sector 2 of Flash memory

// Status definitions
#define ENABLED                                 1u          // Enabled state
#define DISABLED                                2u          // Disabled state

// Debugging method settings
#define DEBUG_METHODE                          UART_DEBUG   // Method for debugging
#define DEBUG_STATUS                            ENABLED      // Debugging status

// Flash Memory Address Range
#define FLASH_START_ADDRESS                    0x08000000U  // Start address of Flash memory
#define FLASH_END_ADDRESS                      0x0801FFFFU  // End address of Flash memory

// SRAM Address Range
#define SRAM_START_ADDRESS                     0x20000000U  // Start address of SRAM
#define SRAM_END_ADDRESS                       0x20004FFFU  // End address of SRAM

// Address validity checks
#define ADDRESS_IS_INVALID                     0x00         // Address is invalid
#define ADDRESS_IS_VALID                       0x01         // Address is valid

// Bootloader Command Definitions
#define CBL_GET_VER_CMD                        0x10         // Command to get bootloader version
#define CBL_GET_HELP_CMD                       0x11         // Command to get help information
#define CBL_GET_CID_CMD                        0x12         // Command to get Chip Identification Number
#define CBL_GET_RDP_STATUS_CMD                 0x13         // Command to get Read Protection Status
#define CBL_GO_TO_ADDR_CMD                     0x14         // Command to jump to a specified address
#define CBL_FLASH_ERASE_CMD                    0x15         // Command to erase flash memory
#define CBL_MEM_WRITE_CMD                      0x16         // Command to write to memory
#define CBL_CHANGE_ROP_Level_CMD               0x21         // Command to change Read Out Protection Level

#define IDCODE_MASK                            0xFFF        // Mask for ID code

// CRC size definition
#define CRC_SIZE                               4u           // Size of CRC

// Bootloader Version Information
#define CBL_VENDOR_ID                          100          // Vendor ID
#define CBL_SW_MAJOR_VERSION                   1            // Major version of the software
#define CBL_SW_MINOR_VERSION                   1            // Minor version of the software
#define CBL_SW_PATCH_VERSION                   0            // Patch version of the software

// Flash Memory Specifications
#define FLASH_BASE_ADDRESS                     0x08000000   // Base address of flash memory
#define FLASH_LAST_ADDRESS                     0x0800FFFF   // Last address of flash memory
#define PAGE_SIZE                              0x00000400   // Size of a single page (1 KB or 1024 bytes)

#define CBL_FLASH_MASS_ERASE                   0xFF        // Command to perform a mass erase of flash
/***************************************Bootloader Macros Declaration End***************************************/

/*************************************Bootloader DataType Declaration Start*************************************/

// Bootloader acknowledgment status
typedef enum
{
    BL_NACK,      // Negative acknowledgment
    BL_ACK,       // Positive acknowledgment
} BL_status;

// CRC verification status
typedef enum
{
    PASSED,       // CRC check passed
    FAILED,       // CRC check failed
} CRC_status;

// Flash memory erase status
typedef enum
{
    INVALID_PAGE_NUMBER,        // Invalid page number specified
    INVALID_PAGE_ADDRESS,       // Invalid page address specified
    UNSUCCESSFUL_ERASE = 0x03, // Erase operation failed
    SUCCESSFUL_ERASE = 0x02,   // Erase operation succeeded
} FLASH_erase_status;

// Flash memory write status
typedef enum
{
    UNSUCCESSFUL_WRITE,         // Write operation failed
    SUCCESSFUL_WRITE,           // Write operation succeeded
} FLASH_write_status;

// Read Out Protection level change status
typedef enum
{
    ROP_LEVEL_CHANGE_INVALID,   // Invalid level change request
    ROP_LEVEL_CHANGE_VALID,     // Valid level change request
} FLASH_CHANGE_PROTECTION_status;
/**************************************Bootloader DataType Declaration End**************************************/


/*************************************Bootloader Function Declaration Start*************************************/

// Function to print debug messages
static void PrintMessage(const char* Format, ...);

// Function to get command from the host
BL_status BL_enGetCommand();

// Function to jump to the user application
static void JumbToUserApplication();

// Bootloader command functions
static void Bootloader_Get_Version(uint8_t *Host_Buffer); // Get bootloader version
static void Bootloader_Get_Help(uint8_t *Host_Buffer);    // Get help information
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer); // Get chip ID
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer); // Read protection level
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer); // Jump to specified address
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);   // Erase flash memory
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);   // Write data to memory
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer); // Change read protection level

// Function to verify CRC
static CRC_status CRC_enVerify(uint8 *Host_Buffer);

// Functions for sending acknowledgment
static void SendAck(uint8 Copy_u8Size);   // Send positive acknowledgment
static void SendNAck();                     // Send negative acknowledgment

// Flash memory operation functions
static FLASH_erase_status EraseFlashPages(uint32_t Copy_u32PageAddress, uint32_t Copy_u32NumberOfPages); // Erase specified flash pages
static FLASH_write_status WriteFlash(uint8* Host_Buffer, uint8 Copy_u8Length, uint32 Copy_u32StartAddress); // Write to flash memory
static FLASH_CHANGE_PROTECTION_status ChangeROPLevel(uint8 Copy_u8ROPLevel); // Change read out protection level
/**************************************Bootloader Function Declaration End**************************************/


#endif
