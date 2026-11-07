# Changelog

## 1.1.0

- Added: ide/ext-spi.stub, the PHP declarations of the package for IDEs (conductor copies it into vendor/php-baremetal/ide-stubs/).
- Added: `spi_driver_desc_t.class_init`, an optional hook called with the driver's class once it is registered, so a driver can add class constants.
- Changed: ext-spi no longer registers Baremetal\Output\Display; ext-display does (a driver that declares the capability finds it registered, or gets a marker interface as for any capability).

## 1.0.2

- Fixed: `new Bus()`, `Bus::device()`, `new Device()` and the driver constructors cast `$host`, `$cs`, `$hz`
  and `$mode` without checking them, so values such as `mode: 256` or `hz: -1` wrapped around. They now
  throw `\ValueError` for a host outside 1 to 255, a CS that is neither a GPIO number nor -1, a clock outside
  0 to 80 MHz (0 is the default) and a mode outside 0 to 3.
- Fixed: a driver whose `capability` was not `Baremetal\Output\Display` implemented nothing. Any interface
  name now works, as in ext-i2c.
- Fixed: a driver class name longer than 96 characters (with its namespace) read past a fixed buffer.
- Fixed: `clone` on a `Bus` or a device read outside the object and could crash the firmware. It now throws
  `Error`.

## 1.0.1

- Changed: the author's contact email.
- Docs: projects use `"sdk": "esp32@^2.0"`.

## 1.0.0

Initial release.
