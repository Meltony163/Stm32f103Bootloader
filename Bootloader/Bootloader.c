#include"Bootloader.h"
#include"STD_TYPES.h"

static uint8 Global_u8arrBuffer[BUFFER_SIZE];

/**
 * @brief   Jumps to the user application located at a specific address in flash memory.
 * 
 * This function retrieves the initial stack pointer and reset handler address of the user application
 * from a predefined memory location (flash memory). It then reconfigures the stack pointer (MSP),
 * resets the RCC configuration to the default state, and jumps to the user application's reset handler.
 */
static void JumbToUserApplication(void)
{
    // Step 1: Retrieve the Main Stack Pointer (MSP) from the user application's vector table
    uint32 Local_u32AppMsp = *((volatile uint32*)FLASH_SECTOR2_BASE_ADDRESS);
    // The MSP is located at the start of the vector table (first 4 bytes)

    // Step 2: Retrieve the reset handler address from the user application's vector table
    uint32 Local_u32AppAddress = *((volatile uint32*)(FLASH_SECTOR2_BASE_ADDRESS + 4));
    // The reset handler address is the second entry in the vector table (next 4 bytes)

    // Step 3: Create a function pointer to the user application's reset handler
    void (*ResetHandler_Address)(void) = (void (*)(void))Local_u32AppAddress;
    // Cast the retrieved address to a function pointer type

    // Step 4: Set the MSP to the value retrieved from the user application's vector table
    __set_MSP(Local_u32AppMsp);
    // Configure the MSP register to the application's stack pointer to ensure correct stack usage

    // Step 5: De-initialize the system peripherals to reset the system state
    HAL_RCC_DeInit();  
    // Resets the clock configuration and all peripherals, preparing for the user application

    // Step 6: Jump to the user application's reset handler
    ResetHandler_Address();  
    // This calls the user application's reset handler, starting its execution
}

/**
 * @brief Prints a formatted message using UART for debugging.
 *        The message format is similar to the printf style.
 * @param Format: A string that contains the format specifiers for the variable arguments.
 * @param ...: Variable arguments that match the format specifiers in the Format string.
 * 
 * @note This function uses UART to transmit the message when DEBUG_METHODE is set to UART_DEBUG.
 */
static void PrintMessage(const char* Format, ...)
{
    // Buffer to store the formatted message (maximum size 100 characters).
    char Local_u8arrMessage[100] = {0}; 
    
    // Variable argument list to handle the multiple arguments passed to the function.
    va_list Local_arg;
    
    // Initializes the va_list variable to retrieve the additional arguments.
    va_start(Local_arg, Format); 
    
    // Creates the formatted string and stores it in Local_u8arrMessage using the provided format.
    vsprintf(Local_u8arrMessage, Format, Local_arg); 
    
    // Check if the debug method is set to UART, then send the message via UART.
    #if (DEBUG_METHODE == UART_DEBUG)
    HAL_UART_Transmit(DEBUGING_PORT, (const uint8*) Local_u8arrMessage, sizeof(Local_u8arrMessage), HAL_MAX_DELAY);
    #endif
    
    // Cleans up the variable argument list.
    va_end(Local_arg); 
}

BL_status BL_enGetCoomand()
{
	BL_status Local_enBlStatus=BL_ACK;
	HAL_StatusTypeDef Local_enUartStatus=HAL_OK;
	uint8 Local_u8DataSize=0;
	memset(Global_u8arrBuffer,0,BUFFER_SIZE);
	Local_enUartStatus=HAL_UART_Receive(COMMUNICATION_PORT,Global_u8arrBuffer, 1, HAL_MAX_DELAY);
	if(Local_enUartStatus==HAL_OK)
	{
		Local_u8DataSize=Global_u8arrBuffer[0];
		Local_enUartStatus=HAL_UART_Receive(COMMUNICATION_PORT,Global_u8arrBuffer+1, Local_u8DataSize, HAL_MAX_DELAY);
		if(Local_enUartStatus==HAL_OK)
		{
    switch (Global_u8arrBuffer[1]) 
			{
        case CBL_GET_VER_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling GET Version Command");
            #endif
            Bootloader_Get_Version(Global_u8arrBuffer);
            break;
        case CBL_GET_HELP_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling GET Help Command");
            #endif
            Bootloader_Get_Help(Global_u8arrBuffer);
            break;
        case CBL_GET_CID_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling GET Chip ID Command");
            #endif
            Bootloader_Get_Chip_Identification_Number(Global_u8arrBuffer);
            break;
        case CBL_GET_RDP_STATUS_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling GET Read Protection Status Command");
            #endif
            Bootloader_Read_Protection_Level(Global_u8arrBuffer);
            break;
        case CBL_GO_TO_ADDR_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling Go to Address Command");
            #endif
            Bootloader_Jump_To_Address(Global_u8arrBuffer);
            break;
        case CBL_FLASH_ERASE_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling Flash Erase Command");
            #endif
            Bootloader_Erase_Flash(Global_u8arrBuffer);
            break;
        case CBL_MEM_WRITE_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling Memory Write Command");
            #endif
            Bootloader_Memory_Write(Global_u8arrBuffer);
            break;
        case CBL_CHANGE_ROP_Level_CMD:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Handling Change ROP Level Command");
            #endif
            Bootloader_Change_Read_Protection_Level(Global_u8arrBuffer);
            break;
        default:
            #if (DEBUG_STATUS == ENABLED)
                PrintMessage("Unknown Command");
            #endif
						Local_enBlStatus=BL_NACK;
            break;
			}
		}
		else
		{
			Local_enBlStatus=BL_NACK;
		}
			
	}
	else
	{
		Local_enBlStatus=BL_NACK;
	}
	return Local_enBlStatus;
}

/**
 * @brief  Retrieves the bootloader version and sends it to the host if CRC verification passes.
 * 
 * This function first verifies the integrity of the received data using the CRC. If the CRC check 
 * passes, it sends an acknowledgment (ACK) to the host, followed by the bootloader version details 
 * (vendor ID, software major version, minor version, and patch version) via UART. If the CRC check 
 * fails, it does not send any data.
 * 
 * @param  Host_Buffer: A pointer to the buffer containing the received data and CRC.
 * @retval None
 */
static void Bootloader_Get_Version(uint8_t *Host_Buffer)
{
    // Step 1: Verify CRC of the received data
    if (PASSED == CRC_enVerify(Host_Buffer)) 
    {
        // Step 2: If CRC passes, send an acknowledgment with a size of 4 bytes
        SendAck(4);

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Sending Version"); // Print debug message indicating version is being sent
        #endif

        // Step 3: Prepare the version information to be sent
        uint8 Local_u8arrMessage[4] = {CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION, CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};

        // Step 4: Transmit the version information via UART
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)Local_u8arrMessage, 4, HAL_MAX_DELAY);
    }
    else
    {
        // Step 5: If CRC fails, you may want to send a NACK or handle it accordingly
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not sending version"); // Optional debug message for failed CRC
        #endif
        SendNAck();
    }
}


/**
 * @brief   Handles the "Get Help" command from the bootloader host.
 * 
 * This function is responsible for processing the "Get Help" command sent from the host.
 * It verifies the integrity of the received data using CRC, sends an acknowledgment, 
 * and transmits a list of supported commands to the host over UART.
 * 
 * @param   Host_Buffer Pointer to the buffer containing the received data from the host.
 * 
 * @note    The function assumes UART communication is available and uses blocking mode for data transmission.
 */
static void Bootloader_Get_Help(uint8_t *Host_Buffer)
{
    // Step 1: Verify CRC of the received data
    if (PASSED == CRC_enVerify(Host_Buffer))  // Check if CRC verification is successful
    {
        // Step 2: If CRC passes, send an acknowledgment with a size of 12 bytes
        SendAck(12);  // Send an acknowledgment to the host indicating readiness to send 12 bytes

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Sending Commands");  // Print debug message indicating that commands are being sent
        #endif

        // Step 3: Prepare the list of commands to be sent to the host
        uint8 Local_u8arrMessage[8] = {
            CBL_GET_VER_CMD,             // Command 1: Get version
            CBL_GET_HELP_CMD,            // Command 2: Get help
            CBL_GET_CID_CMD,             // Command 3: Get Chip ID
            CBL_GET_RDP_STATUS_CMD,      // Command 4: Get Read Protection Status
            CBL_GO_TO_ADDR_CMD,          // Command 5: Go to specific address
            CBL_FLASH_ERASE_CMD,         // Command 6: Flash erase
            CBL_MEM_WRITE_CMD,           // Command 7: Memory write
            CBL_CHANGE_ROP_Level_CMD     // Command 8: Change Read Out Protection Level
        };

        // Step 4: Transmit the array of commands via UART to the host
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)Local_u8arrMessage, 12, HAL_MAX_DELAY); 
        // Send the 12 command bytes using UART, blocking until transmission is complete
    }
    else  // If CRC check fails
    {
        // Step 5: If CRC fails, handle the error by sending a NACK or any custom error handling
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not sending Commands");  // Optional debug message for failed CRC
        #endif

        SendNAck();  // Send a negative acknowledgment (NACK) to the host indicating failure
    }
}

/**
 * @brief   Handles the "Get Chip Identification Number" command from the bootloader host.
 * 
 * This function processes the command to retrieve the Chip Identification Number (IDCODE) 
 * from the microcontroller. It verifies the CRC of the received data, sends an acknowledgment, 
 * and transmits the chip ID via UART.
 * 
 * @param   Host_Buffer Pointer to the buffer containing the received data from the host.
 */
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer)
{
    // Step 1: Verify the CRC of the received data from the host
    if (PASSED == CRC_enVerify(Host_Buffer))  // Check if the CRC verification is successful
    {
        // Step 2: If CRC passes, send an acknowledgment with a size of 2 bytes (IDCODE size)
        SendAck(2);  // Send acknowledgment to indicate readiness to send 2 bytes of Chip ID

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Sending Chip Identification Number");  // Optional debug message for sending Chip ID
        #endif

        // Step 3: Retrieve the Chip Identification Number (IDCODE)
        uint16 Local_u16ChipIdentificationCode = (uint16)((DBGMCU->IDCODE) & IDCODE_MASK);
        // The Chip ID is retrieved by masking the DBGMCU->IDCODE register

        // Step 4: Transmit the Chip Identification Number via UART
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)&Local_u16ChipIdentificationCode, 2, HAL_MAX_DELAY); 
        // Send the 2-byte Chip ID over UART
    }
    else  // If the CRC check fails
    {
        // Step 5: Handle CRC failure by sending a NACK to the host
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not sending Chip Identification Number");  // Optional debug message for failed CRC
        #endif

        SendNAck();  // Send a negative acknowledgment (NACK) to the host
    }
}



/**
 * @brief  This function reads the current read protection (RDP) level of the flash memory.
 *         It verifies the CRC of the received data before retrieving the protection level.
 *         If the CRC verification is successful, it sends an acknowledgment (ACK) and the RDP level.
 *         If the CRC fails, it sends a negative acknowledgment (NACK).
 * @param  Host_Buffer: Pointer to the buffer received from the host.
 * @retval None
 */
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer)
{
    // Step 1: Verify the CRC of the received data from the host
    if (PASSED == CRC_enVerify(Host_Buffer))  // Check if the CRC verification is successful
    {
        // Step 2: If CRC passes, send an acknowledgment with a size of 1 byte (since RDPLevel is 1 byte)
        SendAck(1);  // Send acknowledgment to indicate readiness to send RDP level

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Getting Protection Level");  // Debug message indicating protection level retrieval
        #endif

        // Step 3: Retrieve the current Read Protection Level (RDP)
        uint8 Local_u8Message;
        
        // Initialize the configuration structure to read the Option Bytes
        FLASH_OBProgramInitTypeDef Local_stConfig;
        
        // Retrieve the Option Bytes configuration, which contains the RDP level
        HAL_FLASHEx_OBGetConfig(&Local_stConfig);
        
        // Store the RDP level into Local_u8Message
        Local_u8Message = Local_stConfig.RDPLevel;

        // Step 4: Transmit the Read Protection Level (1 byte) via UART
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)&Local_u8Message, 1, HAL_MAX_DELAY); 
        // Send the 1-byte RDP level over UART
    }
    else  // If the CRC check fails
    {
        // Step 5: Handle CRC failure by sending a NACK to the host
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not Getting Protection Level");  // Debug message for failed CRC
        #endif

        SendNAck();  // Send a negative acknowledgment (NACK) to the host
    }
}

/**
 * @brief  This function handles the process of verifying the host-provided data, validating the address,
 *         and performing a jump to the specified address (if valid) in the STM32 bootloader.
 * 
 * @param  Host_Buffer: A pointer to the buffer that contains the data received from the host. The address to jump to 
 *                      is stored in this buffer at an offset of 2 bytes.
 */

static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer)
{
    // Step 1: Verify the CRC of the received data from the host
    if (PASSED == CRC_enVerify(Host_Buffer))  // Check if the CRC verification is successful
    {
        uint8 Local_u8Message = ADDRESS_IS_INVALID;  // Initialize message to indicate invalid address
        
        // Step 2: If CRC passes, send an acknowledgment to indicate readiness to send 2 bytes of Chip ID
        SendAck(1);  // Send acknowledgment to the host

        // Step 3: Retrieve the address to jump to, which is 32 bits (4 bytes) starting from the 3rd byte of Host_Buffer
        uint32 Local_u32Address = *((uint32*)(Host_Buffer + 2));

        // Step 4: Validate the address - ensure it falls within valid Flash or SRAM address ranges
        if (((Local_u32Address >= FLASH_START_ADDRESS) && (Local_u32Address <= FLASH_END_ADDRESS)) ||
            ((Local_u32Address >= SRAM_START_ADDRESS) && (Local_u32Address <= SRAM_END_ADDRESS)))
        {
            // Step 5: Create a function pointer and set it to the address retrieved, adding +1 for Thumb mode
            void (*Local_fpAddress)(void) = (void (*)(void))(Local_u32Address + 1);

            Local_u8Message = ADDRESS_IS_VALID;  // Update message to indicate valid address

            // Send the valid address acknowledgment to the host
            HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)&Local_u8Message, 1, HAL_MAX_DELAY);

            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("Jumping TO The Address");  // Optional debug message for jumping
            #endif

            // Step 6: Jump to the retrieved address (execute function at that address)
            Local_fpAddress();
        }
        else
        {
            // Address is outside valid ranges (Flash/SRAM)
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("Invalid Address");  // Optional debug message for invalid address
            #endif
            
            // Send an invalid address acknowledgment to the host
            HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)&Local_u8Message, 1, HAL_MAX_DELAY);
        }
    }
    else  // If the CRC check fails
    {
        // Step 7: Handle CRC failure by sending a NACK to the host
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Not Jumping To The Address");  // Optional debug message for failed CRC
        #endif

        SendNAck();  // Send a negative acknowledgment (NACK) to the host
    }
}

/**
 * @brief Erases flash memory pages based on the received command from the host.
 *
 * This function first verifies the integrity of the received data using CRC. 
 * If the CRC verification is successful, it proceeds to erase the specified flash pages 
 * and sends an acknowledgment back to the host. If the CRC verification fails, it sends a NACK.
 *
 * @param Host_Buffer Pointer to the buffer containing the command and parameters from the host.
 *                    The buffer is expected to contain:
 *                    - Host_Buffer[2]: Starting page address for the erase operation.
 *                    - Host_Buffer[3]: Number of pages to erase.
 */
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer) {
    // Step 1: Verify CRC of the received data
    if (PASSED == CRC_enVerify(Host_Buffer)) {
        // Step 2: If CRC passes, send an acknowledgment with a size of 4 bytes
        SendAck(1);

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Erasing Flash"); // Print debug message indicating that the flash is being erased
        #endif

        // Step 3: Call EraseFlashPages to perform the flash erase operation
        uint8_t Local_u8Message = EraseFlashPages(Host_Buffer[2], Host_Buffer[3]);

        // Step 4: Transmit the result of the erase operation via UART
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8_t*)&Local_u8Message, 1, HAL_MAX_DELAY);
    } 
    else {
        // Step 5: If CRC fails, send a NACK or handle it accordingly
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not Erasing Flash"); // Optional debug message for failed CRC
        #endif
        SendNAck(); // Send negative acknowledgment to the host
    }
}


/**
 * @brief  This function writes data from the host to the flash memory of the device.
 *         It verifies the CRC of the received data before proceeding with the flash write.
 *         If the CRC passes, the flash memory is written; otherwise, it sends a negative acknowledgment.
 * @param  Host_Buffer: Pointer to the buffer containing the data to be written to flash.
 *                      The buffer includes the address and the data to be written.
 * @retval None
 */
static void Bootloader_Memory_Write(uint8_t *Host_Buffer)
{
    // Step 1: Verify CRC of the received data to ensure its integrity
    if (PASSED == CRC_enVerify(Host_Buffer)) {
        // Step 2: If CRC verification passes, send an acknowledgment (ACK) with a size of 1 byte
        SendAck(1);

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Writing Flash"); // Print debug message indicating the flash write process is starting
        #endif
        
        // Step 3: Extract the target flash address from the received buffer
        //         The address is located starting from byte 2 in the buffer.
        uint32 Local_u32Address = *((uint32*)(Host_Buffer + 2));

        // Step 4: Call WriteFlash to write the data to flash memory
        //         Host_Buffer+7: Points to the data to be written
        //         Host_Buffer[6]: Specifies the length of the data
        //         Local_u32Address: The flash memory address where the data will be written
        uint8_t Local_u8Message = WriteFlash(Host_Buffer + 7, Host_Buffer[6], Local_u32Address);

        // Step 5: Transmit the result of the flash write operation via UART
        //         This sends back a status byte indicating whether the write operation was successful
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8_t*)&Local_u8Message, 1, HAL_MAX_DELAY);
    } 
    else {
        // Step 6: If CRC verification fails, send a negative acknowledgment (NACK) to the host
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not Writing Flash"); // Debug message indicating CRC failure
        #endif

        // Send a NACK to notify the host that the data was corrupted and not written
        SendNAck();
    }
}


/**
 * @brief  Bootloader function to change the Read-Out Protection (ROP) level based on the host request.
 *         This function verifies the received data's CRC, acknowledges the host, retrieves the chip ID,
 *         and transmits the new ROP level or a failure message via UART.
 * @param  Host_Buffer: Pointer to the buffer that holds the received data from the host, including the new ROP level.
 * @retval None
 */
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer)
{
    // Step 1: Verify the CRC of the received data from the host
    if (PASSED == CRC_enVerify(Host_Buffer))  // Check if the CRC verification is successful
    {
        // Step 2: If CRC passes, send an acknowledgment with a size of 1 byte
        SendAck(1);  // Send acknowledgment to indicate readiness to receive the ROP level

        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("Changing ROP Level");  // Debug message to indicate the start of ROP level change
        #endif

        // Step 3: Change the ROP level based on the data from the host
        uint8 Local_u8Message = ChangeROPLevel(Host_Buffer[2]);  // Pass the ROP level from the host buffer

        // Step 4: Transmit the status of ROP change operation via UART
        HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)&Local_u8Message, 1, HAL_MAX_DELAY); 
        // Send the 1-byte status (ROP change success or failure) over UART
    }
    else  // If the CRC check fails
    {
        // Step 5: Handle CRC failure by sending a negative acknowledgment (NACK) to the host
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC Failed, not Changing ROP Level");  // Debug message to indicate CRC failure
        #endif

        SendNAck();  // Send a negative acknowledgment (NACK) to the host indicating CRC failure
    }
}


/**
 * @brief  Verifies the CRC (Cyclic Redundancy Check) of data received from the host.
 * 
 * This function calculates the CRC for the data received in the `Host_Buffer` and compares it 
 * to the CRC sent by the host (appended at the end of the data). The size of the data, including
 * the CRC, is determined by the first byte of the `Host_Buffer`.
 * 
 * @param  Host_Buffer: A pointer to the buffer containing the received data and the appended CRC.
 * @retval CRC_status: Returns PASSED if the calculated CRC matches the host's CRC, otherwise FAILED.
 */
static CRC_status CRC_enVerify(uint8 *Host_Buffer)
{
    // Initialize status as PASSED
    CRC_status Local_enStatus = PASSED;

    // Calculate the total length of data, including CRC
    uint8 Local_u8DataLength = Host_Buffer[0] + 1; 

    // Extract the CRC value sent by the host from the buffer (last 4 bytes)
    uint32 Local_u32HostCrc = *((uint32*)(Host_Buffer + Local_u8DataLength - CRC_SIZE)); 

    // Variable to store calculated CRC
    uint32 Local_u32McuCrc = 0;

    // Temporary variable to hold data for CRC calculation
    uint32 Local_u32Data = 0;

    // Counter to loop through the data bytes (excluding CRC)
    uint8 Local_u8Counter = 0;

    // Loop through the buffer and accumulate CRC for each byte
    for (Local_u8Counter = 0; Local_u8Counter < Local_u8DataLength - CRC_SIZE; Local_u8Counter++) 
    {
        Local_u32Data = (uint32)Host_Buffer[Local_u8Counter]; // Cast the byte to 32-bit
        Local_u32McuCrc = HAL_CRC_Accumulate(CRC_ENGINE, (uint32_t*)&Local_u32Data, 1); // Accumulate CRC
    }

    // Reset the CRC engine data register after calculation
    __HAL_CRC_DR_RESET(CRC_ENGINE);

    // Compare the CRC received from the host with the one calculated by MCU
    if (Local_u32HostCrc != Local_u32McuCrc)
    {
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC_FAILED"); // Print debug message if CRC check fails
        #endif
        Local_enStatus = FAILED; // Mark status as failed
    }
    else
    {
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("CRC_PASSED"); // Print debug message if CRC check passes
        #endif
    }

    return Local_enStatus; // Return the CRC status
}

/**
 * @brief  Sends an acknowledgment (ACK) message to the host.
 * 
 * This function sends a message that consists of the acknowledgment byte (ACK) followed by 
 * a size byte that specifies the expected size of the data. The message is transmitted 
 * via the UART communication port using HAL_UART_Transmit.
 * 
 * @param  Copy_u8Size: The size of the acknowledgment message to be sent.
 * @retval None
 */
static void SendAck(uint8 Copy_u8Size)
{
    // Send an acknowledgment message with the size of the ACK
    uint8 Local_u8arrMessage[2] = {ACK, Copy_u8Size};

    // Transmit the ACK message via UART, blocking until the transmission is complete
    HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)Local_u8arrMessage, 2, HAL_MAX_DELAY);
}

/**
 * @brief  Sends a negative acknowledgment (NACK) message to the host.
 * 
 * This function sends a negative acknowledgment (NACK) to indicate that the received data 
 * was not processed correctly. The message is transmitted via the UART communication port 
 * using HAL_UART_Transmit.
 * 
 * @retval None
 */
static void SendNAck()
{
    // Send a negative acknowledgment (NACK) message via UART, blocking until complete
    HAL_UART_Transmit(COMMUNICATION_PORT, (const uint8*)NACK, 1, HAL_MAX_DELAY);
}

/**
 * @brief Erases specified flash memory pages.
 *
 * This function can perform either a mass erase of the entire flash memory or erase a specified number of pages.
 *
 * @param Copy_u32PageAddress The starting address of the page to erase or a predefined constant for mass erase.
 * @param Copy_u32NumberOfPages The number of pages to erase (only applicable if not performing a mass erase).
 *
 * @return FLASH_status Status of the erase operation:
 *         - SUCCESSFUL_ERASE: Erase operation was successful.
 *         - UNSUCCESSFUL_ERASE: Erase operation failed.
 *         - INVALID_PAGE_NUMBER: The specified page number is invalid.
 *         - INVALID_PAGE_ADDRESS: The specified page address is invalid.
 */
static FLASH_erase_status EraseFlashPages(uint32_t Copy_u32PageAddress, uint32_t Copy_u32NumberOfPages) {
    HAL_StatusTypeDef Local_enFlashstatus = HAL_OK; // Status of HAL flash operations
    FLASH_erase_status Local_enStatus = SUCCESSFUL_ERASE;  // Initialize status to successful erase
    uint32_t Local_u32FaultyPageAddress;             // Variable to store faulty page address during erase

    // Check if a mass erase is requested
    if (Copy_u32PageAddress == CBL_FLASH_MASS_ERASE) {
        FLASH_EraseInitTypeDef Local_stFlashConfig;
        Local_stFlashConfig.TypeErase = FLASH_TYPEERASE_MASSERASE; // Set erase type to mass erase
        Local_stFlashConfig.Banks = FLASH_BANK_1;                  // Select flash bank to erase
        Local_enFlashstatus=HAL_FLASH_Unlock();                                         // Unlock flash memory for writing
        Local_enFlashstatus = HAL_FLASHEx_Erase(&Local_stFlashConfig, (uint32_t*)&Local_u32FaultyPageAddress); // Perform mass erase
        Local_enFlashstatus=HAL_FLASH_Lock();                                          // Lock flash memory after operation

        // Check if the mass erase was successful
        if ((Local_enFlashstatus == HAL_OK) && (Local_u32FaultyPageAddress == 0xFFFFFFFF)) {
            Local_enStatus = SUCCESSFUL_ERASE; // Mass erase successful
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("SUCCESSFUL_MASS_ERASE"); // Print debug message for successful erase
            #endif
        } else {
            Local_enStatus = UNSUCCESSFUL_ERASE; // Mass erase failed
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("UNSUCCESSFUL_MASS_ERASE"); // Print debug message for unsuccessful erase
            #endif
        }
    }
    // Check if the passed address is valid for page erase
    else if ((Copy_u32PageAddress >= FLASH_BASE_ADDRESS) && (Copy_u32PageAddress <= FLASH_LAST_ADDRESS) && (Copy_u32PageAddress % PAGE_SIZE == 0)) {
        // Check if the requested number of pages does not exceed the flash memory limit
        if ((Copy_u32PageAddress + (Copy_u32NumberOfPages - 1) * PAGE_SIZE) <= FLASH_LAST_ADDRESS) {
            FLASH_EraseInitTypeDef Local_stFlashConfig;
            Local_stFlashConfig.TypeErase = FLASH_TYPEERASE_PAGES; // Set erase type to page erase
            Local_stFlashConfig.NbPages = Copy_u32NumberOfPages;   // Set number of pages to erase
            Local_stFlashConfig.PageAddress = Copy_u32PageAddress; // Set the starting page address
            Local_enFlashstatus=HAL_FLASH_Unlock();                                      // Unlock flash memory for writing
            Local_enFlashstatus = HAL_FLASHEx_Erase(&Local_stFlashConfig, (uint32_t*)&Local_u32FaultyPageAddress); // Perform page erase
            Local_enFlashstatus=HAL_FLASH_Lock();                                       // Lock flash memory after operation

            // Check if the page erase was successful
            if ((Local_enFlashstatus == HAL_OK) && (Local_u32FaultyPageAddress == 0xFFFFFFFF)) {
                Local_enStatus = SUCCESSFUL_ERASE; // Page erase successful
                #if (DEBUG_STATUS == ENABLED)
                PrintMessage("SUCCESSFUL_ERASE"); // Print debug message for successful erase
                #endif
            } else {
                Local_enStatus = UNSUCCESSFUL_ERASE; // Page erase failed
                #if (DEBUG_STATUS == ENABLED)
                PrintMessage("UNSUCCESSFUL_ERASE"); // Print debug message for unsuccessful erase
                #endif
            }
        } else {
            Local_enStatus = INVALID_PAGE_NUMBER; // Requested number of pages exceeds memory limit
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("INVALID_PAGE_NUMBER"); // Print debug message for invalid page number
            #endif
        }
    } else {
        Local_enStatus = INVALID_PAGE_ADDRESS; // Invalid page address provided
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("INVALID_PAGE_ADDRESS"); // Print debug message for invalid page address
        #endif
    }

    return Local_enStatus; // Return the status of the erase operation
}

/**
 * @brief  This function writes data to flash memory in 16-bit half-word units.
 *         It handles both even and odd lengths of data, ensuring correct flash programming.
 *         The function performs boundary checks, unlocks the flash memory, writes data, 
 *         and then locks the flash again.
 * @param  Host_Buffer: Pointer to the buffer holding the data to be written to flash memory.
 * @param  Copy_u8Length: The length of the data to be written in bytes.
 * @param  Copy_u32StartAddress: The starting address in flash memory where data will be written.
 * @retval FLASH_write_status: Returns SUCCESSFUL_WRITE if successful, otherwise UNSUCCESSFUL_WRITE.
 */
static FLASH_write_status WriteFlash(uint8* Host_Buffer, uint8 Copy_u8Length, uint32 Copy_u32StartAddress)
{
    // Status variable to indicate if the write operation was successful or not.
    FLASH_write_status Local_stErrState = SUCCESSFUL_WRITE;
    // Status variable to track the result of HAL flash operations.
    HAL_StatusTypeDef Local_enFlashstatus = HAL_OK;

    // Check if the start address is within the valid flash memory range
    // and if the total length of data to write fits within the allowed address range.
    if (((Copy_u32StartAddress) >= FLASH_BASE_ADDRESS) && 
        ((Copy_u32StartAddress + Copy_u8Length) <= FLASH_LAST_ADDRESS))
    {
        uint16 Local_u16Counter = 0;   // Counter for iterating through the buffer
        uint16 Local_u16Data = 0;      // Temporary variable to hold 16-bit data

        // Unlock the flash memory to allow writing
        Local_enFlashstatus = HAL_FLASH_Unlock();  

        // Check if the flash was successfully unlocked and if the data length is odd
        if ((HAL_OK == Local_enFlashstatus) && (Copy_u8Length % 2 != 0))
        {
            // Handle the last byte if the length is odd, as flash writes must be done in 16-bit chunks.
            // Load the last byte into a 16-bit variable and mask the higher byte to preserve only the lower byte.
            Local_u16Data = *((uint16 *)(Host_Buffer + Copy_u8Length - 1));
            Local_u16Data = Local_u16Data & 0xFF; // Mask the upper byte (keep only lower 8 bits)
            
            // Write the last byte to flash memory as a 16-bit half-word (only lower byte will be valid)
            Local_enFlashstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Copy_u32StartAddress + Copy_u8Length - 1, Local_u16Data);
            Copy_u8Length--;  // Decrease the length by 1 to make it even
        }

        // Loop through the buffer and write data in 16-bit chunks (two bytes at a time)
        for (Local_u16Counter = 0; ((Local_enFlashstatus == HAL_OK) && (Local_u16Counter < Copy_u8Length)); Local_u16Counter += 2)
        {
            // Read two consecutive bytes from the buffer and combine them into a 16-bit half-word
            Local_u16Data = *((uint16 *)(Host_Buffer + Local_u16Counter));
            
            // Write the 16-bit half-word to the flash memory at the corresponding address
            Local_enFlashstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Copy_u32StartAddress + Local_u16Counter, Local_u16Data);
        }

        // Lock the flash memory again after writing to prevent accidental writes
        HAL_FLASH_Lock();

        // Check if the entire flash write operation was successful
        if (HAL_OK == Local_enFlashstatus)
        {
            Local_stErrState = SUCCESSFUL_WRITE;  // Mark the operation as successful
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("Successful Write");  // Debug message indicating successful write
            #endif
        }
        else
        {
            Local_stErrState = UNSUCCESSFUL_WRITE;  // Mark the operation as failed
            #if (DEBUG_STATUS == ENABLED)
            PrintMessage("Unsuccessful Write");  // Debug message indicating failure in writing
            #endif
        }
    }
    else
    {
        // If the start address or address range is invalid, return error status
        Local_stErrState = UNSUCCESSFUL_WRITE;
        #if (DEBUG_STATUS == ENABLED)
        PrintMessage("INVALID_ADDRESS");  // Debug message indicating invalid flash address range
        #endif
    }

    // Return the final status of the flash write operation
    return Local_stErrState;
}



/**
 * @brief  Change the Read-Out Protection (ROP) level of the Flash memory.
 *         This function updates the RDP level of the flash memory to the specified value
 *         by unlocking the option bytes, modifying the RDP level, and locking the option bytes again.
 * @param  Copy_u8ROPLevel: Desired ROP level to be set (0, 1, or 2).
 * @retval FLASH_CHANGE_PROTECTION_status: ROP_LEVEL_CHANGE_VALID if the operation succeeds, 
 *         ROP_LEVEL_CHANGE_INVALID if the operation fails at any step.
 */
static FLASH_CHANGE_PROTECTION_status ChangeROPLevel(uint8 Copy_u8ROPLevel)
{
    // Initialize status variables for flash operation and error state
	HAL_StatusTypeDef Local_stFlashStatus = HAL_OK;
	FLASH_CHANGE_PROTECTION_status Local_stErrState = ROP_LEVEL_CHANGE_VALID;

    // Unlock option bytes to allow programming of ROP level
	Local_stFlashStatus = HAL_FLASH_OB_Unlock();
	if (Local_stFlashStatus == HAL_OK)
	{
		#if (DEBUG_STATUS == ENABLED)
		PrintMessage("Successful_OB_Unlock");  // Debug message indicating successful option byte unlock
		#endif

        // Initialize the configuration structure for the option bytes
		FLASH_OBProgramInitTypeDef Local_stFlashConfig;
		Local_stFlashConfig.Banks = FLASH_BANK_1;  // Specify the flash bank
		Local_stFlashConfig.OptionType = OPTIONBYTE_RDP;  // Specify that the RDP option is to be programmed
		Local_stFlashConfig.RDPLevel = Copy_u8ROPLevel;  // Set the desired RDP level

        // Program the RDP level into the option bytes
		Local_stFlashStatus = HAL_FLASHEx_OBProgram(&Local_stFlashConfig);
		if (Local_stFlashStatus == HAL_OK)
		{
			#if (DEBUG_STATUS == ENABLED)
			PrintMessage("Successful_ROP_CHANGE");  // Debug message indicating successful ROP level change
			#endif

            // Launch the option byte programming operation
			HAL_FLASH_OB_Launch();

            // Lock the option bytes after the ROP level is changed
			Local_stFlashStatus = HAL_FLASH_OB_Lock();
			if (Local_stFlashStatus == HAL_OK)
			{
				#if (DEBUG_STATUS == ENABLED)
				PrintMessage("Successful_OB_Lock");  // Debug message indicating successful option byte lock
				#endif
			}
			else
			{
				#if (DEBUG_STATUS == ENABLED)
				PrintMessage("Unsuccessful_OB_Lock");  // Debug message indicating failure in locking option bytes
				#endif
				Local_stErrState = ROP_LEVEL_CHANGE_INVALID;  // Error state if locking fails
			}
		}
		else
		{
            // If ROP level programming fails, return error and lock option bytes
			Local_stErrState = ROP_LEVEL_CHANGE_INVALID;
			HAL_FLASH_OB_Lock();
			#if (DEBUG_STATUS == ENABLED)
			PrintMessage("UnSuccessful_ROP_CHANGE");  // Debug message indicating successful ROP level change
			#endif
		}
	}
	else
	{
		#if (DEBUG_STATUS == ENABLED)
		PrintMessage("Unsuccessful_OB_Unlock");  // Debug message indicating failure in unlocking option bytes
		#endif
        // Error state if unlocking option bytes fails
		Local_stErrState = ROP_LEVEL_CHANGE_INVALID;
		HAL_FLASH_OB_Lock();  // Ensure option bytes are locked even if unlock fails
	}

	return Local_stErrState;  // Return the final error state of the operation
}
