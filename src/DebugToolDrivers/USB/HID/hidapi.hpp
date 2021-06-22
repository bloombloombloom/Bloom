#pragma once

#include <libusb-1.0/libusb.h>
#include <hidapi/hidapi.h>

/*
 * The code below was extracted from the HIDAPI library. Third-party license may apply here.
 *
 * https://github.com/signal11/hidapi
 */
struct hid_device_
{
    // Handle to the actual device.
    libusb_device_handle* device_handle;

    // Endpoint information
    int input_endpoint;
    int output_endpoint;
    int input_ep_max_packet_size;

    // The interface number of the HID
    int interface;

    // Indexes of Strings
    int manufacturer_index;
    int product_index;
    int serial_index;

    // Whether blocking reads are used
    int blocking; // boolean

    // Read thread objects
    pthread_t thread;
    pthread_mutex_t mutex; // Protects input_reports
    pthread_cond_t condition;
    pthread_barrier_t barrier; // Ensures correct startup sequence
    int shutdown_thread;
    int cancelled;
    struct libusb_transfer* transfer;

    // List of received input reports.
    struct input_report* input_reports;
};
