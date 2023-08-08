/***************************************************************************//**
 *   @file   max14906.c
 *   @brief  Source file of MAX14906 Driver.
 *   @author Ciprian Regus (ciprian.regus@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "max14906.h"
#include "no_os_util.h"
#include "no_os_alloc.h"

/**
 * @brief Compute the CRC5 value for an array of bytes when writing to MAX14906
 * @param data - array of data to encode
 * @return the resulted CRC5
 */
static uint8_t max14906_crc_encode(uint8_t *data)
{
	uint8_t crc5_start = 0x1f;
	uint8_t crc5_poly = 0x15;
	uint8_t crc5_result = crc5_start;
	uint8_t extra_byte = 0x00;
	uint8_t data_bit;
	uint8_t result_bit;

	/*
	 * This is a custom implementation of a CRC5 algorithm, detailed here:
	 * https://www.analog.com/en/app-notes/how-to-program-the-max14906-quadchannel-industrial-digital-output-digital-input.html
	 */

	for (int i = 0; i < 8; i++) {
		data_bit = (data[0] >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	for (int i = 0; i < 8; i++) {
		data_bit = (data[1] >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	for (int i = 0; i < 3; i++) {
		data_bit = (extra_byte >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	return crc5_result;
}

/**
 * @brief Compute the CRC5 value for an array of bytes when reading from MAX14906
 * @param data - array of data to decode
 * @return the resulted CRC5
 */
static uint8_t max14906_crc_decode(uint8_t *data)
{
	uint8_t crc5_start = 0x1f;
	uint8_t crc5_poly = 0x15;
	uint8_t crc5_result = crc5_start;
	uint8_t extra_byte = 0x00;
	uint8_t data_bit;
	uint8_t result_bit;

	/*
	 * This is a custom implementation of a CRC5 algorithm, detailed here:
	 * https://www.analog.com/en/app-notes/how-to-program-the-max14906-quadchannel-industrial-digital-output-digital-input.html
	 */
	for (int i = 2; i < 8; i++) {
		data_bit = (data[0] >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	for (int i = 0; i < 8; i++) {
		data_bit = (data[1] >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	for (int i = 0; i < 3; i++) {
		data_bit = (extra_byte >> (7 - i)) & 0x01;
		result_bit = (crc5_result & 0x10) >> 4;
		if (data_bit ^ result_bit)
			crc5_result = crc5_poly ^ ((crc5_result << 1) & 0x1f);
		else
			crc5_result = (crc5_result << 1) & 0x1f;
	}

	return crc5_result;
}

/**
 * @brief Write the value of a device register
 * @param desc - device descriptor for the MAX14906
 * @param addr - address of the register
 * @param val - value of the register
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_reg_write(struct max14906_desc *desc, uint32_t addr, uint32_t val)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.bytes_number = MAX14906_FRAME_SIZE,
		.cs_change = 1,
	};

	desc->buff[0] = no_os_field_prep(MAX14906_CHIP_ADDR_MASK, desc->chip_address);
	desc->buff[0] |= no_os_field_prep(MAX14906_ADDR_MASK, addr);
	desc->buff[0] |= no_os_field_prep(MAX14906_RW_MASK, 1);
	desc->buff[1] = val;

	if (desc->crc_en) {
		xfer.bytes_number++;
		desc->buff[2] = max14906_crc_encode(desc->buff);
	}

	return no_os_spi_transfer(desc->comm_desc, &xfer, 1);
}

/**
 * @brief Read the value of a device register
 * @param desc - device descriptor for the MAX14906
 * @param addr - address of the register
 * @param val - value of the register
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_reg_read(struct max14906_desc *desc, uint32_t addr, uint32_t *val)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.rx_buff = desc->buff,
		.bytes_number = MAX14906_FRAME_SIZE,
		.cs_change = 1,
	};
	uint8_t crc;
	int ret;

	if (desc->crc_en)
		xfer.bytes_number++;

	memset(desc->buff, 0, xfer.bytes_number);
	desc->buff[0] = no_os_field_prep(MAX14906_CHIP_ADDR_MASK, desc->chip_address);
	desc->buff[0] |= no_os_field_prep(MAX14906_ADDR_MASK, addr);
	desc->buff[0] |= no_os_field_prep(MAX14906_RW_MASK, 0);

	if (desc->crc_en)
		desc->buff[2] = max14906_crc_encode(&desc->buff[0]);

	ret = no_os_spi_transfer(desc->comm_desc, &xfer, 1);
	if (ret)
		return ret;

	if (desc->crc_en) {
		crc = max14906_crc_decode(&desc->buff[0]);
		if (crc != desc->buff[2])
			return -EINVAL;
	}

	*val = desc->buff[1];

	return 0;
}

/**
 * @brief Update the value of a device register (read/write sequence).
 * @param desc - device descriptor for the MAX14906
 * @param addr - address of the register
 * @param mask - bit mask of the field to be updated
 * @param val - value of the masked field. Should be bit shifted by using
 * 		 no_os_field_prep(mask, val)
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_reg_update(struct max14906_desc *desc, uint32_t addr,
			uint32_t mask, uint32_t val)
{
	int ret;
	uint32_t reg_val = 0;

	ret = max14906_reg_read(desc, addr, &reg_val);
	if (ret)
		return ret;

	reg_val &= ~mask;
	reg_val |= mask & val;

	return max14906_reg_write(desc, addr, reg_val);
}

/**
 * @brief Read the (voltage) state of a channel (works for both input or output).
 * @param desc - device descriptor for the MAX14906
 * @param ch - channel index (0 based).
 * @param val - binary value representing a channel's voltage level (0 or 1).
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_ch_get(struct max14906_desc *desc, uint32_t ch, uint32_t *val)
{
	int ret;

	if (ch >= MAX14906_CHANNELS)
		return -EINVAL;

	ret = max14906_reg_read(desc, MAX14906_DOILEVEL_REG, val);
	if (ret)
		return ret;

	*val = no_os_field_get(MAX14906_DOI_LEVEL_MASK(ch), *val);

	return 0;
}

/**
 * @brief Write the (voltage) state of a channel (only for output channels).
 * @param desc - device descriptor for the MAX14906
 * @param ch - channel index (0 based).
 * @param val - binary value representing a channel's voltage level (0 or 1).
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_ch_set(struct max14906_desc *desc, uint32_t ch, uint32_t val)
{
	if (ch >= MAX14906_CHANNELS)
		return -EINVAL;

	return max14906_reg_update(desc, MAX14906_SETOUT_REG,
				   MAX14906_HIGHO_MASK(ch), (val) ?
				   MAX14906_HIGHO_MASK(ch) : 0);
}

/**
 * @brief Configure a channel's function.
 * @param desc - device descriptor for the MAX14906
 * @param ch - channel index (0 based).
 * @param function - channel configuration (input, output or high-z).
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_ch_func(struct max14906_desc *desc, uint32_t ch,
		     enum max14906_function function)
{
	int ret;

	if (function == MAX14906_HIGH_Z) {
		ret = max14906_reg_update(desc, MAX14906_CONFIG_DO_REG, MAX14906_DO_MASK(ch),
					  no_os_field_prep(MAX14906_DO_MASK(ch),
							  MAX14906_PUSH_PULL));
		if (ret)
			return ret;

		return max14906_reg_update(desc, MAX14906_SETOUT_REG,
					   MAX14906_CH_DIR_MASK(ch),
					   no_os_field_prep(MAX14906_CH_DIR_MASK(ch), 1));
	}

	if (function == MAX14906_IN) {
		ret = max14906_reg_update(desc, MAX14906_CONFIG_DO_REG, MAX14906_DO_MASK(ch),
					  no_os_field_prep(MAX14906_DO_MASK(ch),
							  MAX14906_HIGH_SIDE));
		if (ret)
			return ret;
	}

	return max14906_reg_update(desc, MAX14906_SETOUT_REG, MAX14906_CH_DIR_MASK(ch),
				   no_os_field_prep(MAX14906_CH_DIR_MASK(ch), function));
}

/**
 * @brief Configure the current limit for output channels
 * @param desc - device descriptor for the MAX14906
 * @param ch - channel index (0 based).
 * @param climit - current limit value.
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_climit_set(struct max14906_desc *desc, uint32_t ch,
			enum max14906_climit climit)
{
	return max14906_reg_update(desc, MAX14906_CONFIG_CURR_LIM, MAX14906_CL_MASK(ch),
				   no_os_field_prep(MAX14906_CL_MASK(ch), climit));
}

/**
 * @brief Read an output channel's current limit
 * @param desc - device descriptor for the MAX14906
 * @param ch - channel index (0 based).
 * @param val - current limit value.
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_climit_get(struct max14906_desc *desc, uint32_t ch,
			enum max14906_climit *climit)
{
	uint32_t reg_val;
	int ret;

	if (!climit)
		return -EINVAL;

	ret = max14906_reg_read(desc, MAX14906_CONFIG_CURR_LIM, &reg_val);
	if (ret)
		return ret;

	*climit = no_os_field_get(MAX14906_CL_MASK(ch), reg_val);

	return 0;
}

/**
 * @brief Initialize and configure the MAX14906 device
 * @param desc - device descriptor for the MAX14906 that will be initialized.
 * @param param - initialization parameter for the device.
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_init(struct max14906_desc **desc,
		  struct max14906_init_param *param)
{
	struct max14906_desc *descriptor;
	uint32_t reg_val;
	int ret;
	int i;

	descriptor = no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor)
		return -ENOMEM;

	ret = no_os_spi_init(&descriptor->comm_desc, param->comm_param);
	if (ret)
		goto err;

	descriptor->crc_en = param->crc_en;

	ret = no_os_gpio_get_optional(&descriptor->enable, param->enable_param);
	if (!ret) {
		ret = no_os_gpio_set_value(descriptor->enable, NO_OS_GPIO_HIGH);
		if (ret)
			goto gpio_err;
	}

	/* Clear the latched faults generated at power up */
	ret = max14906_reg_read(descriptor, MAX14906_OVR_LD_REG, &reg_val);
	if (ret)
		goto gpio_err;

	ret = max14906_reg_read(descriptor, MAX14906_OPN_WIR_FLT_REG, &reg_val);
	if (ret)
		goto gpio_err;

	ret = max14906_reg_read(descriptor, MAX14906_SHD_VDD_FLT_REG, &reg_val);
	if (ret)
		goto gpio_err;

	ret = max14906_reg_read(descriptor, MAX14906_GLOBAL_FLT_REG, &reg_val);
	if (ret)
		goto gpio_err;

	for (i = 0; i < MAX14906_CHANNELS; i++) {
		ret = max14906_ch_func(descriptor, i, MAX14906_HIGH_Z);
		if (ret)
			goto gpio_err;

		ret = max14906_climit_set(descriptor, i, MAX14906_CL_130);
		if (ret)
			goto gpio_err;
	}

	*desc = descriptor;

	return 0;

gpio_err:
	no_os_gpio_remove(descriptor->enable);
	no_os_spi_remove(descriptor->comm_desc);
err:
	no_os_free(descriptor);

	return ret;
}

/**
 * @brief Free the resources allocated during init and place all the channels in high-z.
 * @param desc - device descriptor for the MAX14906 that will be initialized.
 * @return 0 in case of success, negative error code otherwise
 */
int max14906_remove(struct max14906_desc *desc)
{
	int ret;

	if (!desc)
		return -ENODEV;

	for (int i = 0; i < MAX14906_CHANNELS; i++) {
		ret = max14906_ch_func(desc, i, MAX14906_HIGH_Z);
		if (ret)
			return ret;
	}

	ret = no_os_spi_remove(desc->comm_desc);
	if (ret)
		return ret;

	if (desc->enable) {
		ret = no_os_gpio_set_value(desc->enable, NO_OS_GPIO_HIGH_Z);
		if (ret)
			return ret;
	}

	no_os_free(desc);

	return 0;
}