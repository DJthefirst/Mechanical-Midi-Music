/*
 * Teensy41_USBName.c
 *
 * Overrides the Teensy USB product-name descriptor so the device appears
 * with the project's CFG_DEVICE_NAME in the host OS device list.
 *
 * The Teensy core declares  usb_string_product_name  as a weak symbol
 * in usb_desc.c.  We provide a strong definition here.
 *
 * This file MUST be compiled as C (not C++) because it relies on GCC's
 * flexible-array-member initialiser extension.
 */

#if (defined(ARDUINO_TEENSY41) || defined(__IMXRT1062__)) && defined(CFG_MMM_NETWORK_USB)

#include <usb_names.h>

#ifndef CFG_DEVICE_NAME
#define CFG_DEVICE_NAME "MMM Device"
#endif

/*
 * Helper: extract character i from the string literal, or 0 if i is past
 * the end.  GCC constant-folds this at compile time.
 */
#define CH(i) ((uint16_t)((i) < sizeof(CFG_DEVICE_NAME) - 1 ? CFG_DEVICE_NAME[i] : 0))

/*
 * Strong override of the weak symbol from the Teensy core.
 * bLength       = 2 + string_length * 2   (USB string descriptor header)
 * bDescriptorType = 3                       (USB STRING descriptor)
 * wString[]     = UTF-16LE characters
 *
 * We expand up to 32 characters.  Entries beyond the actual name length
 * are zero and are ignored because bLength limits the descriptor size.
 */
struct usb_string_descriptor_struct usb_string_product_name = {
    2 + (sizeof(CFG_DEVICE_NAME) - 1) * 2,
    3,
    {
        CH( 0), CH( 1), CH( 2), CH( 3), CH( 4), CH( 5), CH( 6), CH( 7),
        CH( 8), CH( 9), CH(10), CH(11), CH(12), CH(13), CH(14), CH(15),
        CH(16), CH(17), CH(18), CH(19), CH(20), CH(21), CH(22), CH(23),
        CH(24), CH(25), CH(26), CH(27), CH(28), CH(29), CH(30), CH(31)
    }
};

#undef CH

#endif /* TEENSY41 && CFG_MMM_NETWORK_USB */
