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
 * Baremetal\Spi\Bus: a SPI master host with its shared pins, keyed by host. device() adds a child with
 * its own CS/clock/mode. Pass data2/data3 for a four-line (QSPI) bus.
 */
#include "php_spi.h"
#include "bm_spi_hal.h"
#include "zend_exceptions.h"

zend_class_entry *spi_bus_ce;
static zend_object_handlers spi_bus_handlers;

extern void spi_device_object_wrap(zval *out, spi_dev_t *dev);

static zend_object *spibus_obj_create(zend_class_entry *ce)
{
    spi_bus_object *o = zend_object_alloc(sizeof(spi_bus_object), ce);
    zend_object_std_init(&o->std, ce);
    object_properties_init(&o->std, ce);
    o->std.handlers = &spi_bus_handlers;
    o->bus = NULL;
    return &o->std;
}

static void spibus_obj_free(zend_object *obj)
{
    zend_object_std_dtor(obj);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_bus_ctor, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, sclk, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, mosi, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, miso, IS_LONG, 0, "-1")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data2, IS_LONG, 0, "-1")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data3, IS_LONG, 0, "-1")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, host, IS_LONG, 0, "1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_bus_device, 0, 1, Baremetal\\Spi\\Device, 0)
    ZEND_ARG_TYPE_INFO(0, cs, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hz, IS_LONG, 0, "1000000")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

PHP_METHOD(SpiBus, __construct)
{
    zend_long sclk, mosi, miso = -1, data2 = -1, data3 = -1, host = 1;
    ZEND_PARSE_PARAMETERS_START(2, 6)
        Z_PARAM_LONG(sclk)
        Z_PARAM_LONG(mosi)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(miso)
        Z_PARAM_LONG(data2)
        Z_PARAM_LONG(data3)
        Z_PARAM_LONG(host)
    ZEND_PARSE_PARAMETERS_END();

    bm_err_t err = BM_OK;
    spi_bus_t *bus = spi_bus_get((int) host, (int) sclk, (int) mosi, (int) miso,
                                 (int) data2, (int) data3, &err);
    if (!bus) {
        zend_throw_exception_ex(zend_ce_exception, 0,
            "cannot open SPI host %d: %s", (int) host, bm_spi_hal_err_name(err));
        RETURN_THROWS();
    }
    spi_bus_object_from(Z_OBJ_P(ZEND_THIS))->bus = bus;
}

PHP_METHOD(SpiBus, device)
{
    zend_long cs, hz = SPI_DEFAULT_HZ, mode = 0;
    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_LONG(cs)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(hz)
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    spi_bus_object *o = spi_bus_object_from(Z_OBJ_P(ZEND_THIS));
    if (!o->bus) {
        zend_throw_exception(zend_ce_exception, "Spi\\Bus is not initialised", 0);
        RETURN_THROWS();
    }

    bm_err_t err = BM_OK;
    spi_dev_t *dev = spi_dev_get(o->bus, (int) cs, (uint32_t) hz, (uint8_t) mode, NULL, &err);
    if (!dev) {
        zend_throw_exception_ex(zend_ce_exception, 0,
            "cannot add SPI device cs=%d: %s", (int) cs, bm_spi_hal_err_name(err));
        RETURN_THROWS();
    }
    spi_device_object_wrap(return_value, dev);
}

static const zend_function_entry spi_bus_methods[] = {
    PHP_ME(SpiBus, __construct, arginfo_bus_ctor,   ZEND_ACC_PUBLIC)
    PHP_ME(SpiBus, device,      arginfo_bus_device, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

void spi_bus_class_register(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Baremetal\\Spi", "Bus", spi_bus_methods);
    spi_bus_ce = zend_register_internal_class(&ce);
    spi_bus_ce->create_object = spibus_obj_create;

    memcpy(&spi_bus_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    spi_bus_handlers.offset = XtOffsetOf(spi_bus_object, std);
    spi_bus_handlers.free_obj = spibus_obj_free;
}
