/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-spi                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/* Neutral SPI transport: the error type and raw transfer a driver uses; the spi-hal package implements it. */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "bm_err.h"   /* bm_err_t and the BM_ERR_* codes (php-baremetal/bm-common) */

/* The per-device registry entry; opaque to drivers (its layout is the backend's concern). */
typedef struct spi_dev spi_dev_t;

/* Full-duplex raw transfer on a raw device: tx and/or rx may be given, n bytes each way. */
bm_err_t spi_dev_transfer(spi_dev_t *d, const uint8_t *tx, uint8_t *rx, size_t n);
