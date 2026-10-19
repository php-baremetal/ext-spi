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

/* The bus/device tables and the driver-facing transfer, over the neutral spi HAL. */
#include "bm_spi_hal.h"

#include <string.h>
#include <stdlib.h>

#define MAX_BUSES    2
#define MAX_DEVICES  8

static spi_bus_t s_buses[MAX_BUSES];
static spi_dev_t s_devs[MAX_DEVICES];

void spi_registry_init(void)
{
    memset(s_buses, 0, sizeof(s_buses));
    memset(s_devs, 0, sizeof(s_devs));
}

static void set_err(bm_err_t *err, bm_err_t e)
{
    if (err) {
        *err = e;
    }
}

spi_bus_t *spi_bus_get(int host, int sclk, int mosi, int miso, int data2, int data3, bm_err_t *err)
{
    set_err(err, BM_OK);

    const int want[5] = { sclk, mosi, miso, data2, data3 };
    for (int i = 0; i < MAX_BUSES; i++) {
        spi_bus_t *b = &s_buses[i];
        if (!b->in_use) {
            continue;
        }
        const int have[5] = { b->sclk, b->mosi, b->miso, b->data2, b->data3 };
        if (b->host == host) {
            /* Never hand back a host wired with other pins. */
            if (memcmp(want, have, sizeof(want)) != 0) {
                set_err(err, BM_ERR_BUS_PINS);
                return NULL;
            }
            return b;
        }
        /* Another host may not take a pin of an open bus (-1 = line unused). */
        for (int w = 0; w < 5; w++) {
            for (int h = 0; h < 5; h++) {
                if (want[w] >= 0 && want[w] == have[h]) {
                    set_err(err, BM_ERR_BUS_PINS);
                    return NULL;
                }
            }
        }
    }

    spi_bus_t *slot = NULL;
    for (int i = 0; i < MAX_BUSES; i++) {
        if (!s_buses[i].in_use) {
            slot = &s_buses[i];
            break;
        }
    }
    if (!slot) {
        set_err(err, BM_ERR_NO_MEM);
        return NULL;
    }

    memset(slot, 0, sizeof(*slot));
    slot->host = host;
    slot->sclk = sclk;
    slot->mosi = mosi;
    slot->miso = miso;
    slot->data2 = data2;
    slot->data3 = data3;
    slot->quad = (data2 >= 0 && data3 >= 0);
    bm_err_t e = bm_spi_hal_bus_open(slot);
    if (e != BM_OK) {
        memset(slot, 0, sizeof(*slot));
        set_err(err, e);
        return NULL;
    }
    slot->in_use = true;
    return slot;
}

spi_dev_t *spi_dev_find(spi_bus_t *bus, int cs)
{
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (s_devs[i].in_use && s_devs[i].bus == bus && s_devs[i].cs == cs) {
            return &s_devs[i];
        }
    }
    return NULL;
}

spi_dev_t *spi_dev_get(spi_bus_t *bus, int cs, uint32_t hz, uint8_t mode,
                       const spi_driver_desc_t *driver, bm_err_t *err)
{
    set_err(err, BM_OK);

    spi_dev_t *found = spi_dev_find(bus, cs);
    if (found) {
        if (found->driver != driver) {
            set_err(err, BM_ERR_CS_IN_USE);
            return NULL;
        }
        return found;
    }

    spi_dev_t *slot = NULL;
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!s_devs[i].in_use) {
            slot = &s_devs[i];
            break;
        }
    }
    if (!slot) {
        set_err(err, BM_ERR_NO_MEM);
        return NULL;
    }

    memset(slot, 0, sizeof(*slot));
    slot->bus = bus;
    slot->cs = cs;
    slot->hz = hz ? hz : SPI_DEFAULT_HZ;
    slot->mode = mode;
    slot->driver = driver;

    bool raw_io = !(driver && driver->managed_io);
    if (raw_io) {
        bm_err_t e = bm_spi_hal_dev_open(slot);
        if (e != BM_OK) {
            memset(slot, 0, sizeof(*slot));
            set_err(err, e);
            return NULL;
        }
    }

    if (driver && driver->state_size) {
        slot->state = calloc(1, driver->state_size);
        if (!slot->state) {
            if (raw_io) {
                bm_spi_hal_dev_close(slot);
            }
            memset(slot, 0, sizeof(*slot));
            set_err(err, BM_ERR_NO_MEM);
            return NULL;
        }
    }

    slot->in_use = true;
    if (driver && driver->init) {
        bm_err_t e = driver->init(slot);
        if (e != BM_OK) {
            free(slot->state);
            if (raw_io) {
                bm_spi_hal_dev_close(slot);
            }
            memset(slot, 0, sizeof(*slot));
            set_err(err, e);
            return NULL;
        }
    }
    return slot;
}

bm_err_t spi_dev_transfer(spi_dev_t *d, const uint8_t *tx, uint8_t *rx, size_t n)
{
    if (!d->hal) {
        return BM_ERR_INVALID_STATE;
    }
    return bm_spi_hal_transfer(d, tx, rx, n);
}
