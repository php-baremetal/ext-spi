---
eyebrow: 'ext-spi · PHP API'
lede:    'Baremetal\Spi\Device is one chip on a bus, selected by its CS pin: full-duplex transfers, writes and reads of raw bytes.'
see_also:
  - { href: './bus.md', meta: 'PHP API' }
  - { href: '../recipes/raw-spi-device.md', meta: 'Recipe' }
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
prev: { label: 'Bus', href: './bus.md' }
next: { label: 'Errors and limits', href: '../reference/errors-and-limits.md' }
---

# Device

A `Device` addresses one chip-select pin on a `Bus`. Get one from `Bus::device()` or construct it. Every driver
class (`Baremetal\Spi\Driver\St77916`, …) extends `Device`.

The object is a handle into a table that lives for the whole run: dropping it never touches the hardware, and
asking for the same CS again returns the same device.

## Constructor

<!-- @method name="new Device(Bus $bus, int $cs, int $hz = 1000000, int $mode = 0)" visibility="public" -->

<!-- @params -->
<!-- @param name="$bus" type="Bus" required -->
The bus the chip is wired to.
<!-- @endparam -->
<!-- @param name="$cs" type="int" required -->
GPIO number of the chip-select line, driven by the SPI peripheral around each transfer.
<!-- @endparam -->
<!-- @param name="$hz" type="int" default="1000000" -->
SCLK frequency for this device. `0` means 1 MHz. Ignored when the device already exists.
<!-- @endparam -->
<!-- @param name="$mode" type="int" default="0" -->
SPI mode, 0 to 3 (clock polarity and phase). Ignored when the device already exists.
<!-- @endparam -->
<!-- @endparams -->

Neither `$hz` nor `$mode` is range-checked by ext-spi; the HAL rejects values it cannot use. Throws
`\Exception` `cannot add SPI device cs=N: ...` when a driver already holds the CS, when all 8 device slots are
in use, or when the HAL cannot attach the device.

## Methods

<!-- @method name="Device::transfer(string $tx): string" returns="string" visibility="public" -->

Full duplex: clocks out every byte of `$tx` and returns the bytes clocked in at the same time, same length. Put
a command and its response in one call, padding `$tx` with dummy bytes for the response: CS stays active for
the whole call.

<!-- @method name="Device::write(string $data): void" returns="void" visibility="public" -->

Clocks out the bytes of `$data` and discards what comes back.

<!-- @method name="Device::read(int $length): string" returns="string" visibility="public" -->

Clocks out `$length` zero bytes and returns the `$length` bytes received. `\ValueError` when `$length` is not
greater than 0. A chip that needs a command before its data needs `transfer()` instead.

An empty string to `transfer()` or `write()` does nothing. A call longer than 32768 bytes fails on ESP32 with
`ESP_ERR_INVALID_SIZE`; split it.

## Errors

A failed transfer throws `\Exception` with the operation and the HAL's error name, for example
`SPI write failed: ESP_ERR_INVALID_SIZE`. On a driver whose descriptor sets `managed_io` (the driver drives the
wire itself, like `St77916`), the inherited `transfer()`, `write()` and `read()` always fail with
`ESP_ERR_INVALID_STATE`. Full list in [Errors and limits](../reference/errors-and-limits.md).

## Example

<!-- @code-block language="php" label="read a register" -->
```php
<?php
use Baremetal\Spi\Bus;

$bus  = new Bus(sclk: 12, mosi: 11, miso: 13);
$chip = $bus->device(cs: 10, hz: 4_000_000, mode: 3);

// many sensors: bit 7 set means "read", then the register address
$rx = $chip->transfer(chr(0x80 | 0x0F) . "\x00");
printf("WHO_AM_I = 0x%02X\n", ord($rx[1]));

$chip->write(chr(0x20) . chr(0x47));      // register 0x20 = 0x47
```
<!-- @endcode-block -->
