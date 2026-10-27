---
eyebrow: 'Package · Extension'
lede:    'A SPI master for PHP on the board: open a host on your pins (QSPI included), exchange raw bytes with a chip, or use a driver class such as a display. Neutral extension; conductor adds the SPI HAL for your platform.'
see_also:
  - { href: './usage/bus.md', meta: 'PHP API' }
  - { href: './drivers/writing-a-driver.md', meta: 'Drivers' }
  - { href: 'package:php-baremetal/esp32-spi-hal', meta: 'HAL', label: 'esp32-spi-hal' }
  - { href: 'package:php-baremetal/esp32-display-st77916#docs/overview.md', meta: 'Driver', label: 'esp32-display-st77916' }
  - { href: 'package:php-baremetal/bm-common', meta: 'Library', label: 'bm-common' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'Bus', href: './usage/bus.md' }
---

# SPI bus and devices

ext-spi lets the PHP script on your board talk to SPI chips. `Baremetal\Spi\Bus` opens one SPI host on a clock
pin and its data pins: MOSI and MISO for plain SPI, plus `data2` and `data3` for a four-line QSPI bus.
`Baremetal\Spi\Device` addresses one chip on that bus by its chip-select pin and moves raw bytes. Chips with a
driver package get their own class under `Baremetal\Spi\Driver\`, which extends `Device` and adds typed
methods (a display's `fill()`, `rect()`, `wake()`).

## Requirements

| Requires | Why |
| --- | --- |
| `php-baremetal/spi-hal` (virtual) | The platform side: host setup, device attach, transfers. On ESP32 it is `php-baremetal/esp32-spi-hal` (platform `esp32`, over `esp_driver_spi`). |
| `php-baremetal/bm-common` | The shared `bm_err.h` error codes. |

You never require the HAL yourself. Conductor reads the platform from `extra.baremetal.sdk` (`esp32@^2.0` gives
`esp32`) and adds `php-baremetal/esp32-spi-hal` while Composer resolves. The extension's PHP module is `spi`.

## Install

<!-- @steps -->
- **Install conductor once per machine**

  `composer global require php-baremetal/conductor` keeps the plugin active during the first solve of a new
  project.

- **Create the project**

  `phpbm init my-display` writes `composer.json` with conductor (`^1.0`, allowed as a plugin), the package
  registry, and the SDK, board, runtime and storage you pick.

- **Require the extension and any drivers**

  `composer require php-baremetal/ext-spi php-baremetal/esp32-display-st77916`

- **Build and flash**

  `phpbm flash` builds the firmware and flashes it; `phpbm monitor` opens the serial console.
<!-- @endsteps -->

The resulting `composer.json`, for a Waveshare ESP32-S3-Touch-LCD-1.85B (an ST77916 round panel on a QSPI bus):

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/my-display",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-sys": "^1.0",
    "php-baremetal/ext-spi": "^1.0",
    "php-baremetal/esp32-display-st77916": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "waveshare-esp32-s3-touch-lcd-1.85b",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

`ext-sys` is there for `delay()` in the script below; ext-spi does not need it.

## A first script

`project-src/index.php` for the `init-loop` runtime: the top level runs once, then `loop($tick)` runs repeatedly.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Spi\Bus;
use Baremetal\Spi\Driver\St77916;

// QSPI: mosi is data line 0, miso data line 1
$bus = new Bus(sclk: 40, mosi: 46, miso: 45, data2: 42, data3: 41);
$lcd = new St77916($bus, cs: 21, rst: 3, bl: 5);

function loop(int $tick): void
{
    global $lcd;

    $colors = [$lcd->rgb(255, 0, 0), $lcd->rgb(0, 255, 0), $lcd->rgb(0, 0, 255)];
    $lcd->fill($colors[$tick % 3]);
    delay(1000);
}
```
<!-- @endcode-block -->

## PHP API at a glance

| Name | Kind | Meaning |
| --- | --- | --- |
| `Baremetal\Spi\Bus` | class | One SPI host and its SCLK and data pins; `device()`. |
| `Baremetal\Spi\Device` | class | One chip on a bus, keyed by its CS pin: `transfer()`, `write()`, `read()`. |
| `Baremetal\Spi\Driver\*` | classes | One per installed driver package, each extends `Device`. |
| `Baremetal\Output\Display` | interface | Capability marker a display driver class implements. Always defined, no methods. |

The extension defines no functions and no constants. Failures throw `\Exception` with a readable message;
an invalid `read()` length throws `\ValueError`. See [Errors and limits](./reference/errors-and-limits.md).

## Drivers

ext-spi declares the registry `spi_drivers` (element type `spi_driver_desc_t`, header `spi_driver_desc.h`).
Driver packages (`bm-lib`) register a descriptor into it with `extra.baremetal.register`, and at module startup
ext-spi registers one PHP class per descriptor. Available today:

| Package | Class | Capability |
| --- | --- | --- |
| [esp32-display-st77916](package:php-baremetal/esp32-display-st77916) | `Baremetal\Spi\Driver\St77916` | `Baremetal\Output\Display` |

See [The spi_drivers registry](./drivers/registry.md) and, to add a chip, [Writing a driver](./drivers/writing-a-driver.md).

## Notes

- Buses and devices live for the whole run. `new Bus()` on the same host returns the same bus, and a device
  object is a handle: dropping it never touches the wire. This also holds across requests in the `web-server`
  runtime, so a display stays initialised between requests.
- Every transfer runs on the calling PHP task and blocks until it completes. Chip select is held for the whole
  of one `transfer()`, `write()` or `read()` call.
- ext-spi does nothing special for the `power-save` option. A driver may need its own recovery after light
  sleep (the ST77916 driver has `wake()`).
- The default host is `1`, which is `SPI2_HOST` on ESP32 chips. The SDK's microSD storage (SPI mode) uses
  `SPI3_HOST` and the W5500 network uses `SPI2_HOST`; pick a host they do not use. See
  [Bus](./usage/bus.md).
