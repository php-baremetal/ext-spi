/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-spi                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@aurecchia.com>             |
   +----------------------------------------------------------------------+
*/

/* Neutral SPI HAL: the platform side of ext-spi, implemented by esp32-spi-hal on ESP32. */
#pragma once

#include "spi_registry.h"
#include <stddef.h>
#include <stdint.h>

/* Initialise the SPI host bus->host on bus->sclk/mosi/miso (+ data2/data3 when bus->quad). */
bm_err_t bm_spi_hal_bus_open(spi_bus_t *bus);
void bm_spi_hal_bus_close(spi_bus_t *bus);

/* Attach d->cs at d->hz / d->mode on d->bus and store the handle in d->hal. */
bm_err_t bm_spi_hal_dev_open(spi_dev_t *d);
void bm_spi_hal_dev_close(spi_dev_t *d);

/* One full-duplex transfer of n bytes on an opened device. */
bm_err_t bm_spi_hal_transfer(spi_dev_t *d, const uint8_t *tx, uint8_t *rx, size_t n);

/* Printable name of a bm_err_t. */
const char *bm_spi_hal_err_name(bm_err_t err);
