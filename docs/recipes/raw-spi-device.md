---
eyebrow: 'ext-spi · Recipe'
lede:    'Talk to a chip that has no driver: read the JEDEC ID and status register of an external SPI NOR flash with a raw Device.'
see_also:
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: '../drivers/writing-a-driver.md', meta: 'Drivers' }
prev: { label: 'Writing a driver', href: '../drivers/writing-a-driver.md' }
next: { label: 'No next page', href: '#' }
---

# Read a SPI flash ID

A W25Q-series SPI NOR flash breakout wired to an ESP32-S3-DevKitC-1 on the `SPI2_HOST` default pins: SCLK 12,
MOSI 11, MISO 13, CS 10. On another board or wiring, change the board id and the pins.

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/spi-flash-id",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-spi": "^1.0"
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
      "storage": "embedded",
      "options": { "restart-delay": 3600 }
    }
  }
}
```
<!-- @endcode-block -->

The script runs once and ends; `restart-delay` keeps the board from restarting for an hour, so the output stays
on the console.

## The commands

| Command | Sent | Answer |
| --- | --- | --- |
| `0x9F` JEDEC ID | 1 byte + 3 dummy bytes | manufacturer, memory type, capacity |
| `0x05` Read status 1 | 1 byte + 1 dummy byte | bit 0 is BUSY |

## The script

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Spi\Bus;

$bus   = new Bus(sclk: 12, mosi: 11, miso: 13);
$flash = $bus->device(cs: 10, hz: 10_000_000);       // mode 0

// one call per command: CS stays low while the answer is clocked in
$id = substr($flash->transfer("\x9F\x00\x00\x00"), 1);
[, $maker, $type, $capacity] = unpack('C3', $id);
printf("JEDEC id %02X %02X %02X, %d KB\n", $maker, $type, $capacity, (1 << $capacity) >> 10);

$status = ord($flash->transfer("\x05\x00")[1]);
printf("status 0x%02X (%s)\n", $status, $status & 1 ? 'busy' : 'idle');
```
<!-- @endcode-block -->

- `transfer()` returns as many bytes as it sends; the first byte arrived while the command went out, so it is
  dropped with `substr()` or skipped by index.
- Splitting a command into a `write()` and a `read()` does not work for this chip: CS goes high between two
  calls and the flash forgets the command.

## When it fails

- **`ff ff ff` or `00 00 00`**: no chip answering. Check MISO, power, and that CS is the pin you passed.
- **`cannot open SPI host 1: ...`**: the host is taken, either by another `Bus` on other pins or by the
  firmware (W5500 networking uses `SPI2_HOST`). Pass `host: 2`.
- **`cannot add SPI device cs=10: chip select already used by another driver on this bus`**: a driver class
  already owns CS 10 on this bus.
