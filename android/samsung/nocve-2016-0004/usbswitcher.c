/*
 * usbswitcher.c, by @joystick and @rpaleari
 *
 * Search for an attached USB Samsung device and switch it to a specific USB
 * configuration (#2). Requires libusb [1] (use version 0.1).
 *
 * References:
 * [1] http://libusb.org/
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "usb.h"

/* This VID/PID pair is OK for all the Samsung phones we tested. However, you
   may need to tune them for other device models. */
#define SAMSUNG_VENDOR_ID  0x04e8
#define SAMSUNG_PRODUCT_ID 0x6860

void _log(char tag, const char *fmt, ...) {
  char msg[1024];

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);

  fprintf(stderr, "[%c] %s\n", tag, msg);
}

#define error(fmt, ...) _log('!', fmt, ## __VA_ARGS__)
#define info(fmt, ...) _log('*', fmt, ## __VA_ARGS__)

struct usb_device *find_device() {
  struct usb_bus *bus;
  struct usb_device *dev;
  for (bus=usb_busses; bus != NULL; bus=bus->next) {
    for (dev=bus->devices; dev; dev=dev->next) {
      if ((dev->descriptor.idVendor == SAMSUNG_VENDOR_ID) &&
	  (dev->descriptor.idProduct == SAMSUNG_PRODUCT_ID))
	return dev;
    }
  }
  return NULL;
}

int main() {
  int r;
  struct usb_device *dev;
  usb_dev_handle *udev;

  usb_init();
  usb_find_busses();
  usb_find_devices();

  dev = find_device();
  if (!dev) {
    error("Device not found");
    exit(-1);
  }
  info("Device found, %d configuration(s)", dev->descriptor.bNumConfigurations);

  assert(dev->descriptor.bNumConfigurations == 2);

  udev = usb_open(dev);
  if (!udev) {
    error("Can't open device");
    exit(-1);
  }
  info("Device opened, Switching to configuration #2");

  usb_reset(udev);
  r = usb_set_configuration(udev, 2);
  if (r != 0) {
    error("Configuration switch failed");
    usb_close(udev);
    exit(-1);
  }

  info("Configuration switched!");
  usb_close(udev);

  return 0;
}
