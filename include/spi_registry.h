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
 * The two process-lifetime tables (buses, devices), never torn down: a PHP object is a handle into them
 * and its destructor never touches the hardware, so a display survives web-server per-request teardown.
 * The `hal` handles belong to the spi HAL (bm_spi_hal.h).
 */
#pragma once

#include "bm_spi.h"            /* bm_err_t + the opaque spi_dev_t + spi_dev_transfer */
#include "spi_driver_desc.h"
#include <stdint.h>
#include <stdbool.h>

#define SPI_DEFAULT_HZ  (1000000u)

typedef struct spi_bus {
    bool   in_use;
    int    host;              /* the platform's SPI host number -- the bus key */
    int    sclk, mosi, miso;
    int    data2, data3;      /* QSPI extra lines, -1 when unused */
    bool   quad;
    void  *hal;               /* HAL bus state */
} spi_bus_t;

struct spi_dev {
    bool                     in_use;
    spi_bus_t               *bus;
    int                      cs;
    uint32_t                 hz;
    uint8_t                  mode;
    void                    *hal;         /* HAL device handle; NULL for a managed-io driver */
    const spi_driver_desc_t *driver;      /* NULL for a raw device */
    void                    *state;       /* driver per-instance state, or NULL */
};

/* Reset the tables. Runs once at MINIT; creates nothing on the wire. */
void spi_registry_init(void);

/* Idempotent lookup-or-create for the bus keyed by host. Opens the HAL bus on first use. */
spi_bus_t *spi_bus_get(int host, int sclk, int mosi, int miso, int data2, int data3, bm_err_t *err);

/* Idempotent lookup-or-create for a device keyed by (bus, cs). On first creation it opens the HAL
 * device (unless driver->managed_io), allocates driver->state_size and runs driver->init. An entry
 * already taken by another driver returns NULL with BM_ERR_CS_IN_USE. */
spi_dev_t *spi_dev_get(spi_bus_t *bus, int cs, uint32_t hz, uint8_t mode,
                       const spi_driver_desc_t *driver, bm_err_t *err);

/* The device on (bus, cs), or NULL. */
spi_dev_t *spi_dev_find(spi_bus_t *bus, int cs);
