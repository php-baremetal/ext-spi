# ext-spi

A SPI master for the PHP that runs on your board. `Baremetal\Spi\Bus` opens a SPI host on your pins (four data
lines for QSPI), `Baremetal\Spi\Device` exchanges raw bytes with one chip on it, and device-driver packages add
typed classes under `Baremetal\Spi\Driver\` (for example `St77916`, a QSPI round display).

ext-spi is platform-neutral: it requires the virtual `php-baremetal/spi-hal`, and conductor adds the HAL for your
SDK platform (`php-baremetal/esp32-spi-hal` on ESP32).

## Install

In a project created with `phpbm init`:

```bash
composer require php-baremetal/ext-spi
```

Add a device driver the same way; a driver requires ext-spi, so it is enough on its own:

```bash
composer require php-baremetal/esp32-display-st77916
```

## Example

```php
<?php
use Baremetal\Spi\Bus;

$bus   = new Bus(sclk: 12, mosi: 11, miso: 13);
$flash = $bus->device(cs: 10, hz: 10_000_000);

$rx = $flash->transfer("\x9F\x00\x00\x00");          // JEDEC ID command + 3 dummy bytes
printf("JEDEC id: %s\n", bin2hex(substr($rx, 1)));
```

## Documentation

- [Overview](docs/overview.md): requirements, install, a first project, limits.
- [Bus](docs/usage/bus.md) and [Device](docs/usage/device.md): the PHP API.
- [Errors and limits](docs/reference/errors-and-limits.md)
- [The spi_drivers registry](docs/drivers/registry.md) and [Writing a driver](docs/drivers/writing-a-driver.md)
- [Read a SPI flash ID](docs/recipes/raw-spi-device.md)

## License

MIT.
