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
