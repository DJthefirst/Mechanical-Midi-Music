/*
 * Teensy41_USBPid.c
 *
 * Overrides the Teensy USB Product ID (PID) so each device can be
 * uniquely identified by a DAW or host application.
 *
 * The Teensy core's device_descriptor[] lives in RAM (non-const) but is
 * static to usb_desc.c.  We reach it through the extern
 * usb_descriptor_list[0].addr pointer and patch the PID bytes before
 * usb_init() is called.
 *
 * Startup order in the Teensy 4 core (startup.c):
 *   startup_middle_hook()  ←  we patch the descriptor here
 *   usb_init()             ←  USB stack reads the (now-patched) descriptor
 *
 * This file MUST be compiled as C (not C++).
 *
 * Usage – add to the network_usb section of your .ini config:
 *   -D CFG_USB_MIDI_PID=0x0500
 */

#if (defined(ARDUINO_TEENSY41) || defined(__IMXRT1062__)) \
    && defined(CFG_MMM_NETWORK_USB) && defined(CFG_USB_MIDI_PID)

#define USB_DESC_LIST_DEFINE
#include <usb_desc.h>

/*
 * USB device descriptor byte offsets (USB 2.0 spec, Table 9-8):
 *   Bytes  8– 9 : idVendor  (LSB, MSB)
 *   Bytes 10–11 : idProduct (LSB, MSB)
 */
#define USB_DEV_DESC_PID_OFFSET 10

/*
 * startup_middle_hook() is a weak symbol in the Teensy core that runs
 * after RAM initialisation but before usb_init().
 */
void startup_middle_hook(void) {
    uint8_t *desc = (uint8_t *)usb_descriptor_list[0].addr;
    desc[USB_DEV_DESC_PID_OFFSET]     = (CFG_USB_MIDI_PID) & 0xFF;
    desc[USB_DEV_DESC_PID_OFFSET + 1] = ((CFG_USB_MIDI_PID) >> 8) & 0xFF;
}

#endif /* TEENSY41 && CFG_MMM_NETWORK_USB && CFG_USB_MIDI_PID */
