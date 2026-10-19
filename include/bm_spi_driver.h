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

/* Driver-facing SDK: all a SPI device driver includes, with nothing backend-specific (no ESP-IDF). */
#pragma once

#include "php.h"
#include "bm_spi.h"
#include "spi_driver_desc.h"

/* The device entry behind $this (a Device or a driver instance); throws and returns NULL if unset. */
spi_dev_t *spi_device_this(zval *zthis);

/* Shared driver constructor: (Baremetal\Spi\Bus $bus, int $cs = default, int $hz = default). */
void spi_driver_ctor(INTERNAL_FUNCTION_PARAMETERS, const spi_driver_desc_t *desc);
