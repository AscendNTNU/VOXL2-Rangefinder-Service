/*******************************************************************************
 * Copyright 2022 ModalAI Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * 4. The Software is used solely in conjunction with devices provided by
 *    ModalAI Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#ifndef VOXL_IO_I2C_H
#define VOXL_IO_I2C_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief      Maximum I2C bus identifier. Default is 15 for a total of 6 busses.
 *             This can be increased by the user for special cases.
 */
#define I2C_MAX_BUS 15

/**
 * @brief      size of i2c buffer in bytes for writing to registers. Only
 *             increase if you know what you are doing.
 */
#define I2C_BUFFER_SIZE 128

/**
 * @brief      Initializes a bus and sets it to talk to a particular device
 *             address.
 *
 * @param[in]  bus      The bus
 * @param[in]  devAddr  The device address
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_init(int bus, uint8_t devAddr);

/**
 * @brief      Closes an I2C bus
 *
 * @param[in]  bus   The bus
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_close(int bus);

/**
 * @brief      Changes the device address the bus is configured to talk to.
 *
 *             Actually changing the device address in the I2C driver requires a
 *             system call and is relatively slow. This function records which
 *             device address the bus is set to and will only make the system
 *             call if the requested address is different than the set address.
 *             This makes it safe to call this function repeatedly with no
 *             performance penalty.
 *
 * @param[in]  bus      The bus
 * @param[in]  devAddr  The new device address
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_set_device_address(int bus, uint8_t devAddr);

/**
 * @brief      Reads a single byte from a device register.
 *
 *             This sends the device address and register address to be read
 *             from before reading the response, works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[out] data     The data pointer to write response to.
 *
 * @return     number of bytes read, 1 on success or -1 on failure
 */
int voxl_i2c_read_byte(int bus, uint8_t regAddr, uint8_t *data);

/**
 * @brief      Reads multiple bytes from a device register.
 *
 *             This sends the device address and register address to be read
 *             from before reading the response, works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[in]  count   number of bytes to read
 * @param[out] data     The data pointer to write response to.
 *
 * @return     returns number of bytes read or -1 on failure
 */
int voxl_i2c_read_bytes(int bus, uint8_t regAddr, size_t count,  uint8_t *data);

/**
 * @brief      Reads a single word (16 bits) from a device register.
 *
 *             This sends the device address and register address to be read
 *             from before reading the response, works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[out] data     The data pointer to write response to.
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_read_word(int bus, uint8_t regAddr, uint16_t *data);

/**
 * @brief      Reads multiple words (16 bytes each) from a device register.
 *
 *             This sends the device address and register address to be read
 *             from before reading the response, works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[in]  count    Number of 16-bit words to read, NOT number of bytes to read
 * @param[out] data     The data pointer to write response to.
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_read_words(int bus, uint8_t regAddr, size_t count, uint16_t* data);


/**
 * @brief      Writes a single byte to a specified register address.
 *
 *             This sends the device address and register address followed by
 *             the actual data to be written. Works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[in]  data     Single byte to be writen
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_write_byte(int bus, uint8_t regAddr, uint8_t data);

/**
 * @brief      Writes multiple bytes to a specified register address.
 *
 *             This sends the device address and register address followed by
 *             the actual data to be written. Works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address to write to
 * @param[in]  count    The number of bytes to write
 * @param      data     pointer to user's data to be writen
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_write_bytes(int bus, uint8_t regAddr, size_t count, uint8_t* data);


/**
 * @brief      Writes a single word (16 bits) to a specified register address.
 *
 *             This sends the device address and register address followed by
 *             the actual data to be written. Works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address to write to
 * @param[in]  data     16-bit word to be written
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_write_word(int bus, uint8_t regAddr, uint16_t data);

/**
 * @brief      Writes multiple words (16 bits each) to a specified register
 *             address.
 *
 *             This sends the device address and register address followed by
 *             the actual data to be written. Works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[in]  count    Number of 16-bit words to write, NOT number of bytes
 * @param[in]  data     The data
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_write_words(int bus, uint8_t regAddr, size_t count, uint16_t* data);



/**
 * @brief      Sends exactly user-defined data without prepending a register
 *             address.
 *
 *             Instead of automatically sending a device address before the data
 *             which is typical for reading/writing registers, the
 *             voxl_i2c_send_bytes function send only the data given by the data
 *             argument. This is useful for more complicated IO such as
 *             uploading firmware to a device.
 *
 * @param[in]  bus     The bus
 * @param[in]  count   Number of bytes to send
 * @param[in]  data    The data
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_send_bytes(int bus, size_t count, uint8_t* data);

/**
 * @brief      Sends exactly user-defined data without prepending a register
 *             address.
 *
 *             Instead of automatically sending a device address before the data
 *             which is typical for reading/writing registers, the
 *             voxl_i2c_send_bytes function send only the data given by the data
 *             argument. This is useful for more complicated IO such as
 *             uploading firmware to a device.
 *
 * @param[in]  bus   The bus
 * @param[in]  data  The data
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_send_byte(int bus, uint8_t data);

/**
 * @brief      Locks the bus so other threads in the process know the bus is in
 *             use.
 *
 *             Locking a bus is similar to locking a mutex, it is a way for
 *             threads to communicate within one process when sharing a bus.
 *             This, however, is not a hard lock in the sense that it does not
 *             block and does not stop any of the other functions in this API
 *             from being called. It only serves as a flag that can be checked
 *             between threads if the user chooses to do so. This is encouraged
 *             in multithraded applications to prevent timing-sensitive i2c
 *             communication from being interrupted but is not enforced.
 *
 *             All read/write functions in this API will lock the bus during the
 *             transaction and return the lockstate to what it was at the
 *             beginning of the transaction. Ideally the user should lock the
 *             bus themselves before a sequence of transactions and unlock it
 *             afterwards.
 *
 * @param[in]  bus   The bus ID
 *
 * @return     Returns the lock state (0 or 1) when this function is called, or -1 on
 *             error.
 */
int voxl_i2c_lock_bus(int bus);

/**
 * @brief      Unlocks a bus to indicate to other threads in the process that
 *             the bus is now free.
 *
 *             see voxl_i2c_lock_bus for further description.
 *
 * @param[in]  bus   The bus ID
 *
 * @return     Returns the lock state (0 or 1) when this function is called, or -1 on
 *             error.
 */
int voxl_i2c_unlock_bus(int bus);

/**
 * @brief      Fetches the current lock state of the bus.
 *
 * @param[in]  bus   The bus ID
 *
 * @return     Returns 0 if unlocked, 1 if locked, or -1 on error.
 */
int voxl_i2c_get_lock(int bus);

/**
 * @brief      Gets file descriptor.
 *
 *
 * @param[in]  bus      The bus
 *
 * @return     returns file descriptor of the specified bus or -1 on failure
 */
int voxl_i2c_get_fd(int bus);










/**
 * @brief      Writes multiple bytes to a specified 16-bit register address.
 *
 *             This sends the device address and register address followed by
 *             the actual data to be written. Works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address to write to
 * @param[in]  count    The number of bytes to write
 * @param      data     pointer to user's data to be writen
 *
 * @return     0 on success or -1 on failure
 */
int voxl_i2c_reg16_write_bytes(int bus, uint16_t regAddr, size_t count, uint8_t* data);


/**
 * @brief      Reads multiple bytes from a device register.
 *
 *             This sends the device address and register address to be read
 *             from before reading the response, works for most i2c devices.
 *
 * @param[in]  bus      The bus
 * @param[in]  regAddr  The register address
 * @param[in]  count   number of bytes to read
 * @param[out] data     The data pointer to write response to.
 *
 * @return     returns number of bytes read or -1 on failure
 */
int voxl_i2c_reg16_read_bytes(int bus, uint16_t regAddr, size_t count,  uint8_t *data);


#ifdef __cplusplus
}
#endif

#endif  // VOXL_IO_I2C_H
