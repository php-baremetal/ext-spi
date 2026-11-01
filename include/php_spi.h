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

/*
 * Internal glue shared by the spi extension's C files: the class entries, the object wrappers (each a
 * handle into a registry entry) and the registration entry points called from MINIT.
 */
#pragma once

#include "php.h"
#include "spi_registry.h"
#include "bm_spi_driver.h"   /* spi_device_this + spi_driver_ctor (the driver-facing SDK) */

extern zend_module_entry spi_module_entry;

extern zend_class_entry *spi_bus_ce;
extern zend_class_entry *spi_device_ce;

typedef struct {
    spi_bus_t   *bus;
    zend_object  std;
} spi_bus_object;

typedef struct {
    spi_dev_t   *dev;
    zend_object  std;
} spi_device_object;

/* Argument checks for a device: CS a GPIO number or -1, clock 0 (the default) to 80 MHz, mode 0 to 3.
 * false (with a ValueError) otherwise. */
static inline bool spi_check_device_args(uint32_t cs_arg, zend_long cs, uint32_t hz_arg, zend_long hz,
                                         uint32_t mode_arg, zend_long mode)
{
    if (cs < -1 || cs > 255) {
        zend_argument_value_error(cs_arg, "must be a GPIO number or -1");
        return false;
    }
    if (hz < 0 || hz > (zend_long) SPI_MAX_HZ) {
        zend_argument_value_error(hz_arg, "must be between 0 and 80000000 (Hz; 0 is the default)");
        return false;
    }
    if (mode < 0 || mode > 3) {
        zend_argument_value_error(mode_arg, "must be an SPI mode from 0 to 3");
        return false;
    }
    return true;
}

static inline spi_bus_object *spi_bus_object_from(zend_object *o)
{
    return (spi_bus_object *)((char *)o - XtOffsetOf(spi_bus_object, std));
}

static inline spi_device_object *spi_device_object_from(zend_object *o)
{
    return (spi_device_object *)((char *)o - XtOffsetOf(spi_device_object, std));
}

void spi_bus_class_register(void);
void spi_device_class_register(void);

/* Register the capability interface Output\Display. */
void spi_capabilities_register(void);

/* Register the selected driver classes (Baremetal\Spi\Driver\*), each extending Device. */
void spi_driver_classes_register(void);
