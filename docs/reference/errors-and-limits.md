---
eyebrow: 'ext-spi · Reference'
lede:    'Every exception ext-spi throws, and the fixed sizes behind them.'
see_also:
  - { href: '../usage/bus.md', meta: 'PHP API' }
  - { href: '../usage/device.md', meta: 'PHP API' }
prev: { label: 'Device', href: '../usage/device.md' }
next: { label: 'The spi_drivers registry', href: '../drivers/registry.md' }
---

# Errors and limits

ext-spi throws plain `\Exception` for runtime failures and `\ValueError` for invalid arguments; it defines no
exception classes of its own. Messages end with the HAL's error name (on ESP32 an `esp_err_t` name such as
`ESP_ERR_NO_MEM`, or one of the two php-baremetal names below).

## Exceptions

| Message | Thrown by | Cause |
| --- | --- | --- |
| `cannot open SPI host N: ...` | `new Bus()` | Host or pin conflict, no free bus slot, or the HAL could not initialise the host. |
| `cannot add SPI device cs=N: ...` | `Bus::device()`, `new Device()` | The CS is held by a driver, no free device slot, or the HAL could not attach the device. |
| `cannot init NAME (cs=N): ...` | a driver constructor that uses the shared one | The CS is taken, no free slot, or the driver's `init` failed. |
| `SPI transfer failed: ...`, `SPI write failed: ...`, `SPI read failed: ...` | `transfer()`, `write()`, `read()` | The transfer failed, was too long, or the device is driven by a managed-io driver. |
| `Spi\Bus is not initialised`, `Spi\Device is not initialised` | any method | The object was created without its constructor running (for example a subclass that skipped it). |

`\ValueError` is thrown by `read()` for a `$length` that is not greater than 0. Driver methods throw their own
`\Exception` messages (for example `st77916 init failed: ...`).

The php-baremetal error names: `chip select already used by another driver on this bus` and
`the host or a pin is already used by an SPI bus open with other pins`.

## Limits

| Limit | Value | Source |
| --- | --- | --- |
| Open buses | 2 | ext-spi bus table |
| Devices (raw and driver, all buses) | 8 | ext-spi device table |
| Default device clock | 1 MHz (drivers set their own) | ext-spi, driver descriptor |
| Bytes per call | 32768 | esp32-spi-hal |
| Queued transactions per device | 4 (calls are synchronous, so one is in flight) | esp32-spi-hal |

Tables are never freed: buses and devices stay open until the board restarts, across requests in the
`web-server` runtime too. The SPI hosts the firmware itself uses (microSD storage, W5500 network) are not
available to `Bus`; see [Bus](../usage/bus.md).

## Where the work runs

Every transfer runs on the PHP task that calls it and blocks until the hardware is done; ext-spi uses no
executor task and no background sampling. On ESP32 the HAL keeps each device's transaction descriptor in
internal RAM, and `spi_master` copies PHP strings (which live in PSRAM) through a DMA-capable buffer. SPI
transfers do not touch the flash, so they are allowed on the PHP task.
