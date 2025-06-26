/*
 * Mihashi USB Device Descriptors
 * USB MIDI Device descriptors for Mihashi
 */

#include "tusb.h"
#include "mihashi_dual_usb.h"

//--------------------------------------------------------------------
// Device Descriptor
//--------------------------------------------------------------------
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = MIHASHI_USB_VID,
    .idProduct          = MIHASHI_USB_PID,
    .bcdDevice          = MIHASHI_USB_DEVICE_VER,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &desc_device;
}

//--------------------------------------------------------------------
// Configuration Descriptor
//--------------------------------------------------------------------
enum {
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
#define EPNUM_MIDI_OUT      0x01
#define EPNUM_MIDI_IN       0x81

uint8_t const desc_configuration[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------
char const* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
    "Mihashi Dev Project",         // 1: Manufacturer
    "Mihashi USB MIDI Bridge",     // 2: Product
    "MDB001",                      // 3: Serials, should use chip ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        // Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}