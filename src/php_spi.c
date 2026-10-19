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
 * spi extension module: at MINIT it resets the registries and registers Baremetal\Spi\Bus,
 * Baremetal\Spi\Device, the Output\Display capability and the selected driver classes.
 */
#include "php_spi.h"

PHP_MINIT_FUNCTION(spi)
{
    spi_registry_init();
    spi_capabilities_register();
    spi_bus_class_register();
    spi_device_class_register();
    spi_driver_classes_register();
    return SUCCESS;
}

static const zend_function_entry spi_functions[] = {
    PHP_FE_END
};

zend_module_entry spi_module_entry = {
    STANDARD_MODULE_HEADER,
    "spi",
    spi_functions,
    PHP_MINIT(spi),
    NULL,   /* MSHUTDOWN */
    NULL,   /* RINIT */
    NULL,   /* RSHUTDOWN */
    NULL,   /* MINFO */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};
