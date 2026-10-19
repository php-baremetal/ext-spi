---
eyebrow: 'ext-spi · PHP API'
lede:    'Baremetal\Spi\Bus is a handle onto one SPI host and its pins. Same host, same bus: constructing it twice with the same pins is safe and cheap.'
see_also:
  - { href: './device.md', meta: 'PHP API' }
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
  - { href: 'package:php-baremetal/esp32-spi-hal', meta: 'HAL', label: 'esp32-spi-hal' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Device', href: './device.md' }
---

# Bus

`Baremetal\Spi\Bus` opens a SPI master host on a clock pin and its data pins. The bus lives for the whole run:
a second `new Bus()` on the same host with the same pins returns the same underlying bus, so libraries can each
construct their own `Bus` object without coordinating.

## Constructor

<!-- @method name="new Bus(int $sclk, int $mosi, int $miso = -1, int $data2 = -1, int $data3 = -1, int $host = 1)" visibility="public" -->

<!-- @params -->
<!-- @param name="$sclk" type="int" required -->
GPIO number of the clock line.
<!-- @endparam -->
<!-- @param name="$mosi" type="int" required -->
GPIO number of MOSI. On a QSPI bus this is data line 0.
<!-- @endparam -->
<!-- @param name="$miso" type="int" default="-1" -->
GPIO number of MISO, or `-1` for a write-only bus. On a QSPI bus this is data line 1.
<!-- @endparam -->
<!-- @param name="$data2" type="int" default="-1" -->
QSPI data line 2. The bus is four-line only when both `$data2` and `$data3` are given.
<!-- @endparam -->
<!-- @param name="$data3" type="int" default="-1" -->
QSPI data line 3.
<!-- @endparam -->
<!-- @param name="$host" type="int" default="1" -->
The platform's SPI host number, the key of the bus. On ESP32 chips `1` is `SPI2_HOST` and `2` is `SPI3_HOST`.
<!-- @endparam -->
<!-- @endparams -->

Throws `\Exception` `cannot open SPI host N: ...` when:

- the host is already open with other pins, or one of the pins belongs to a bus open on another host
  (`the host or a pin is already used by an SPI bus open with other pins`);
- both bus slots are in use (`ESP_ERR_NO_MEM`);
- the HAL cannot initialise the host, for example because the firmware already uses it (on ESP32,
  `spi_bus_initialize` fails with `ESP_ERR_INVALID_STATE`).

<!-- @callout variant="warning" title="Hosts the firmware may already use" -->
On ESP32-S3 boards the SDK's microSD storage in SPI mode runs on `SPI3_HOST` (host `2`), and the W5500 network
runs on `SPI2_HOST` (host `1`). With microSD storage keep the default host 1; with W5500 networking pass
`host: 2`. A board that uses both has no general-purpose SPI host left for ext-spi.
<!-- @endcallout -->

On ESP32 the host is set up with DMA (channel picked automatically) and a maximum transfer of 32768 bytes.

## device()

<!-- @method name="Bus::device(int $cs, int $hz = 1000000, int $mode = 0): Device" returns="Device" visibility="public" -->

Attaches the chip whose chip select is GPIO `$cs`, clocked at `$hz` in SPI mode `$mode` (0 to 3), and returns a
raw `Device`. Same as `new Device($bus, $cs, $hz, $mode)`.

Devices are kept per bus and CS pin: asking again for a CS returns the existing device (the new `$hz` and
`$mode` are ignored). If a driver class already holds that CS, it throws
`cannot add SPI device cs=N: chip select already used by another driver on this bus`.

<!-- @code-block language="php" label="two chips on one bus" -->
```php
<?php
use Baremetal\Spi\Bus;

$bus   = new Bus(sclk: 12, mosi: 11, miso: 13);
$flash = $bus->device(cs: 10, hz: 10_000_000);        // mode 0
$adc   = $bus->device(cs: 9, hz: 1_000_000, mode: 1);
```
<!-- @endcode-block -->

The bus has no other methods. There is no `close()`: a bus stays open until the board restarts.
