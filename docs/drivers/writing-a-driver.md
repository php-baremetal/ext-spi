---
eyebrow: 'ext-spi · Drivers'
lede:    'A driver is a bm-lib package with a C file: a descriptor of type spi_driver_desc_t, registered into spi_drivers. ext-spi turns it into a PHP class.'
see_also:
  - { href: './registry.md', meta: 'Drivers' }
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/esp32-display-st77916', meta: 'Driver', label: 'esp32-display-st77916 (a managed-io driver)' }
  - { href: 'package:php-baremetal/bm-common', meta: 'Library', label: 'bm-common (bm_err.h)' }
prev: { label: 'The spi_drivers registry', href: './registry.md' }
next: { label: 'Read a SPI flash ID', href: '../recipes/raw-spi-device.md' }
---

# Writing a driver

A driver gives one chip a PHP class. It is a `bm-lib` package that requires ext-spi and registers a descriptor
symbol into the `spi_drivers` registry. A driver that talks through ext-spi's transfer function includes only
`bm_spi_driver.h` (plus Zend headers for its methods): no ESP-IDF, so the same source builds on any platform
that has a spi HAL.

## The package

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/xyz-driver",
  "description": "XYZ sensor driver for ext-spi (Baremetal\\Spi\\Driver\\Xyz).",
  "version": "1.0.0",
  "type": "bm-lib",
  "license": "MIT",
  "require": {
    "php": ">=8.3",
    "php-baremetal/ext-spi": "^1.0"
  },
  "extra": {
    "baremetal": {
      "register": { "spi_drivers": ["xyz_driver_desc"] }
    }
  }
}
```
<!-- @endcode-block -->

Sources default to `src/*.c`; no `CMakeLists.txt`. The symbol in `register` must be a non-static `const
spi_driver_desc_t` defined in those sources.

## The C file

A complete driver with a constructor, an identity check and one method. The register layout is a placeholder
for your chip's.

<!-- @code-block language="c" label="src/xyz.c" -->
```c
#include "bm_spi_driver.h"
#include "zend_exceptions.h"

#define XYZ_READ      0x80   /* bit 7 set: read */
#define XYZ_ID_REG    0x0F
#define XYZ_ID_VALUE  0x33
#define XYZ_DATA_REG  0x28   /* 2 bytes, little-endian */

extern const spi_driver_desc_t xyz_driver_desc;

static bm_err_t xyz_read(spi_dev_t *d, uint8_t reg, uint8_t *out, size_t n)
{
    uint8_t tx[8] = { XYZ_READ | reg };
    uint8_t rx[8] = { 0 };
    bm_err_t e = spi_dev_transfer(d, tx, rx, n + 1);
    if (e == BM_OK) {
        memcpy(out, rx + 1, n);
    }
    return e;
}

static bm_err_t xyz_init(spi_dev_t *d)
{
    uint8_t id = 0;
    bm_err_t e = xyz_read(d, XYZ_ID_REG, &id, 1);
    if (e != BM_OK) {
        return e;
    }
    return id == XYZ_ID_VALUE ? BM_OK : BM_ERR_NOT_FOUND;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_xyz_ctor, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, bus, Baremetal\\Spi\\Bus, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cs, IS_LONG, 0, "10")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hz, IS_LONG, 0, "4000000")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xyz_value, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Xyz, __construct)
{
    spi_driver_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &xyz_driver_desc);
}

PHP_METHOD(Xyz, value)
{
    ZEND_PARSE_PARAMETERS_NONE();
    spi_dev_t *d = spi_device_this(ZEND_THIS);
    if (!d) {
        RETURN_THROWS();
    }
    uint8_t b[2];
    if (xyz_read(d, XYZ_DATA_REG, b, 2) != BM_OK) {
        zend_throw_exception(zend_ce_exception, "XYZ read failed", 0);
        RETURN_THROWS();
    }
    RETURN_LONG((int16_t) (b[0] | (b[1] << 8)));
}

static const zend_function_entry xyz_methods[] = {
    PHP_ME(Xyz, __construct, arginfo_xyz_ctor,  ZEND_ACC_PUBLIC)
    PHP_ME(Xyz, value,       arginfo_xyz_value, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

const spi_driver_desc_t xyz_driver_desc = {
    .name = "xyz",
    .php_class = "Xyz",
    .default_cs = 10,
    .default_hz = 4000000,
    .default_mode = 3,
    .init = xyz_init,
    .methods = xyz_methods,
};
```
<!-- @endcode-block -->

PHP then has `Baremetal\Spi\Driver\Xyz`, with `value()` plus every `Device` method. `new Xyz($bus)` uses CS 10
at 4 MHz in mode 3.

<!-- @callout variant="warning" title="Always define __construct" -->
The method table must contain a `__construct`, normally one that calls `spi_driver_ctor()` with the
descriptor. Without it the class inherits `Device::__construct`, which attaches a raw device: `init` never runs.
<!-- @endcallout -->

## Descriptor fields

`spi_driver_desc_t` is declared in `spi_driver_desc.h`. Leave unused fields zeroed (designated initialisers do
that). ext-spi 1.0 reads these:

| Field | Meaning |
| --- | --- |
| `name` | Registry name, used in constructor errors, e.g. `"st77916"`. |
| `php_class` | Short class name; the class is `Baremetal\Spi\Driver\<php_class>`. Must be unique among installed drivers; the full name must fit in 95 characters. |
| `capability` | Interface FQN, or `NULL`. Only `"Baremetal\\Output\\Display"` is honoured; any other value is ignored and the class implements nothing. |
| `default_cs` | CS pin when the constructor gets a negative one. `-1` means no default. |
| `default_hz` | Clock when the constructor gets 0 or less; 0 here means 1 MHz. |
| `default_mode` | SPI mode 0 to 3. Always used: the shared constructor has no `$mode` argument. |
| `managed_io` | `false`: ext-spi attaches a HAL device on the CS pin, and `spi_dev_transfer()` works. `true`: no HAL device is attached; the driver drives the wire itself (for example through a platform LCD library), and the inherited `transfer()`, `write()` and `read()` fail with `ESP_ERR_INVALID_STATE`. |
| `state_size` | Bytes allocated (zeroed) per device before `init`. `bm_spi_driver.h` has no accessor for this block; a driver that needs it includes `spi_registry.h` and reads `d->state`. |
| `init` | Called once when the device is first created, on the PHP task, after the HAL device is attached. Return `BM_OK`, or an error code the constructor reports (`BM_ERR_NOT_FOUND` for a wrong chip identity). |
| `methods` | The class's method table (`zend_function_entry[]`). |

Declared but not read by ext-spi 1.0: `deinit` (devices are never torn down). Filling it is harmless.

## Functions a driver may call

From `bm_spi.h` and `bm_spi_driver.h`:

| Function | Does |
| --- | --- |
| `spi_dev_transfer(d, tx, rx, n)` | One full-duplex transfer of `n` bytes, CS held for all of it. `tx` or `rx` may be `NULL`. Returns `bm_err_t`; `BM_ERR_INVALID_STATE` on a managed-io device. |
| `spi_driver_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &desc)` | The shared constructor: `(Bus $bus, int $cs = default, int $hz = default)`, attaches the device, runs `init`, throws `cannot init NAME (cs=N): ...` on failure. |
| `spi_device_this(ZEND_THIS)` | The `spi_dev_t *` behind the object; throws and returns `NULL` if the object is not initialised. |

Error codes are in `bm_err.h` from bm-common (`BM_OK` is 0). The transfer runs on the calling PHP task; ext-spi
adds no locking (on ESP32, `spi_master` serialises devices that share a host).

## Managed-io drivers

A driver that hands the wire to a platform library sets `managed_io = true` and becomes platform-specific: it
requires the platform's HAL package and lists the ESP-IDF components it uses. The ST77916 driver is one:

<!-- @code-block language="json" label="esp32-display-st77916 composer.json" -->
```json
{
  "type": "bm-lib",
  "require": {
    "php": ">=8.3",
    "php-baremetal/ext-spi": "^1.0",
    "php-baremetal/esp32-spi-hal": "^1.0"
  },
  "extra": {
    "baremetal": {
      "idf-requires": ["esp_lcd", "esp_driver_gpio"],
      "register": { "spi_drivers": ["st77916_driver_desc"] }
    }
  }
}
```
<!-- @endcode-block -->

Such a driver includes `php_spi.h` (or `spi_registry.h`) to reach the bus (`d->bus->host`, the pins) and its
state, and may write its own constructor that calls `spi_dev_get(bus, cs, hz, mode, &desc, &err)` to reserve
the slot before bringing the chip up.

## Try it in a project

Keep the driver next to the project as a path repository; conductor links it into `.phpbm/packages/`.

<!-- @code-block language="json" label="project composer.json" -->
```json
{
  "name": "acme/xyz-demo",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true },
    { "type": "path", "url": "extensions/xyz-driver" }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "acme/xyz-driver": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "espressif-esp32-s3-devkitc-1-n8r8",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Spi\Bus;
use Baremetal\Spi\Driver\Xyz;

$xyz = new Xyz(new Bus(sclk: 12, mosi: 11, miso: 13));   // the pins your chip is wired to
echo $xyz->value(), "\n";
```
<!-- @endcode-block -->

Then `phpbm flash` and `phpbm monitor`. A wrong chip identity shows up as
`cannot init xyz (cs=10): ESP_ERR_NOT_FOUND`.
