---
eyebrow: 'ext-spi · Drivers'
lede:    'ext-spi owns the spi_drivers registry. Driver packages register a descriptor into it, and each descriptor becomes a PHP class under Baremetal\Spi\Driver.'
see_also:
  - { href: './writing-a-driver.md', meta: 'Drivers' }
  - { href: 'package:php-baremetal/esp32-display-st77916#docs/overview.md', meta: 'Driver', label: 'esp32-display-st77916' }
prev: { label: 'Errors and limits', href: '../reference/errors-and-limits.md' }
next: { label: 'Writing a driver', href: './writing-a-driver.md' }
---

# The spi_drivers registry

ext-spi declares one registry in its `composer.json`:

<!-- @code-block language="json" label="ext-spi composer.json, extra.baremetal" -->
```json
{
  "module": "spi",
  "priority": 100,
  "registries": {
    "spi_drivers": { "type": "spi_driver_desc_t", "header": "spi_driver_desc.h" }
  }
}
```
<!-- @endcode-block -->

A driver package registers into it by naming its descriptor symbol:

<!-- @code-block language="json" label="esp32-display-st77916 composer.json, extra.baremetal" -->
```json
{ "register": { "spi_drivers": ["st77916_driver_desc"] } }
```
<!-- @endcode-block -->

## What happens at build and startup

<!-- @steps -->
- **The build collects the entries**

  The SDK driver generates a table from every package in the project that registers into `spi_drivers`,
  ordered by package and then symbol, NULL-terminated, with a count. With no driver installed the table is
  empty and ext-spi still builds.

- **ext-spi registers the classes**

  At module startup, for each descriptor ext-spi registers `Baremetal\Spi\Driver\<php_class>` as a subclass of
  `Baremetal\Spi\Device`, with the descriptor's method table. If the descriptor's `capability` is
  `Baremetal\Output\Display`, the class implements that interface.

- **PHP uses the class**

  The driver's constructor reserves the device slot for its CS pin on the bus, allocates the driver state and
  runs the driver's `init` once. A later construction on the same bus and CS returns the same device.
<!-- @endsteps -->

So a project gets a driver class just by requiring the driver package: nothing to configure, and no class to
enable at runtime. Classes of drivers that are not installed do not exist.

## Drivers available today

| Package | Class | Default CS, clock | Capability | Platform |
| --- | --- | --- | --- | --- |
| [esp32-display-st77916](package:php-baremetal/esp32-display-st77916) | `St77916` | GPIO 21, 40 MHz QSPI | `Baremetal\Output\Display` | ESP32 (uses `esp_lcd`) |

`St77916` has its own constructor (bus, CS, reset and backlight pins, size, orientation) and the methods
`fill()`, `rect()`, `rgb()` and `wake()`. See its package documentation.

## Rules the build enforces

- Only ext-spi may declare `spi_drivers`; a second package declaring it is a build error.
- A symbol registered twice, an invalid C name, or a registry that no installed package declares is a build
  error that names the package.
- The header `spi_driver_desc.h` comes from ext-spi's `include/`, so a driver needs no include path of its own.
