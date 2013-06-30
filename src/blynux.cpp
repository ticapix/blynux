#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>

static int g_verbose_flag = 0;

static void logging(int level, const char* prefix, const char *format, ...);
static void logging_va(int level, const char* prefix, const char *format,
		va_list args);

#define LOG(...) {logging(0, "", __VA_ARGS__);}
#define ERROR_LOG(...) {logging(1, "EE", __VA_ARGS__);}
#define WARNING_LOG(...) {logging(2, "WW", __VA_ARGS__);}
#define INFO_LOG(...) {logging(3, "II", __VA_ARGS__);}
#define DEBUG_LOG(...) {logging(4, "DD", __VA_ARGS__);}

#define DEV_PID 0x0001
#define DEV_VID 0x1130

#define HELP "\
--verbose, -v      :-vvvv for max verbosity\n\
--device, -d       :device number (default: 0)\n\
--color, -c        :WHITE, CYAN, MAGENTA, BLUE, YELLOW, GREEN, RED, OFF\n\
--help, -h\n"

static const struct option g_long_options[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "verbose", no_argument, NULL, 'v' }, { "device", required_argument,
				NULL, 'd' }, { "color", required_argument, NULL, 'c' }, { 0, 0,
				0, 0 } };

static const char* g_short_options = "vld:c:h";

static const struct {
	const char* name;
	int value;
} g_colors[] = { { "WHITE", 0x8 }, { "CYAN", 0x9 }, { "MAGENTA", 0xa }, {
		"BLUE", 0xb }, { "YELLOW", 0xc }, { "GREEN", 0xd }, { "RED", 0xe }, {
		"OFF", 0xf }, { 0, 0 } };

void logging(int level, const char* prefix, const char *format, ...) {
	va_list args;
	va_start(args, format);
	logging_va(level, prefix, format, args);
	va_end(args);
}

static void logging_va(int level, const char* prefix, const char *format,
		va_list args) {
	if (level < g_verbose_flag)
		return;
	printf("%s ", prefix);
	vprintf(format, args);
}

//void printdev(libusb_device *dev) {
//	libusb_device_descriptor desc;
//	int r = libusb_get_device_descriptor(dev, &desc);
//	if (r < 0) {
//		std::cout << "failed to get device descriptor" << std::endl;
//		return;
//	}
//	std::cout << "Number of possible configurations: "
//			<< (int) desc.bNumConfigurations << "  ";
//	std::cout << "Device Class: " << (int) desc.bDeviceClass << "  ";
//	std::cout << "VendorID: " << desc.idVendor << "  ";
//	std::cout << "ProductID: " << desc.idProduct << std::endl;
//	libusb_config_descriptor *config;
//	libusb_get_config_descriptor(dev, 0, &config);
//	std::cout << "Interfaces: " << (int) config->bNumInterfaces << " ||| ";
//	const libusb_interface *inter;
//	const libusb_interface_descriptor *interdesc;
//	const libusb_endpoint_descriptor *epdesc;
//	for (int i = 0; i < (int) config->bNumInterfaces; i++) {
//		inter = &config->interface[i];
//		std::cout << "Number of alternate settings: " << inter->num_altsetting
//				<< " | ";
//		for (int j = 0; j < inter->num_altsetting; j++) {
//			interdesc = &inter->altsetting[j];
//			std::cout << "Interface Number: "
//					<< (int) interdesc->bInterfaceNumber << " | ";
//			std::cout << "Number of endpoints: "
//					<< (int) interdesc->bNumEndpoints << " | ";
//			for (int k = 0; k < (int) interdesc->bNumEndpoints; k++) {
//				epdesc = &interdesc->endpoint[k];
//				std::cout << "Descriptor Type: "
//						<< (int) epdesc->bDescriptorType << " | ";
//				std::cout << "EP Address: " << (int) epdesc->bEndpointAddress
//						<< " | ";
//			}
//		}
//	}
//	std::cout << std::endl << std::endl << std::endl;
//	libusb_free_config_descriptor(config);
//}

bool isBuddy(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		WARNING_LOG("Failed to get device descriptor");
		return false;
	}
	return desc.idVendor == DEV_VID && desc.idProduct == DEV_PID;
}

void setColor(libusb_device *device, int color_mask) {
	libusb_device_handle* dev_handle = NULL;
	if (libusb_open(device, &dev_handle) < 0) {
		ERROR_LOG("Error on opening handle");
		return;
	}

	int intf_num = 1;
	if (libusb_kernel_driver_active(dev_handle, intf_num) == 1) { //find out if kernel driver is attached
		INFO_LOG("Kernel Driver Active\n");
		if (libusb_detach_kernel_driver(dev_handle, intf_num) == 0) //detach it
			INFO_LOG("Kernel Driver Detached!");
	}
	if (libusb_claim_interface(dev_handle, intf_num) < 0) { //claim interface
		ERROR_LOG("Cannot Claim Interface");
		return;
	}
	unsigned char data[8] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x40, 0x02, 0x0f};
	data[7] = (color_mask << 4) | data[7];

	uint8_t bmRequestType = 0x21;
	uint8_t bRequest = 9;
	uint16_t wValue = 0x0200;
	uint16_t wIndex = 1;
	uint16_t wLength = 8;
	unsigned int timeout = 1000 * 2; // 2 secs

	if (wLength
			!= libusb_control_transfer(dev_handle, bmRequestType, bRequest,
					wValue, wIndex, data, wLength, timeout)) {
		ERROR_LOG("Write Control Error\n");
	}
	if (libusb_release_interface(dev_handle, 1) != 0) { //release the claimed interface
		ERROR_LOG("Cannot Release Interface");
	}
	libusb_close(dev_handle); //close the device we opened
}

int setColorOnDevice(int device_number, int color_mask) {
	int cpt = 0;
	libusb_device **list;
	libusb_context *ctx = NULL;
	libusb_init(&ctx); //initialize a library session
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	if (cnt < 0) {
		ERROR_LOG("no usb device found");
		cnt = 0;
	}
	for (ssize_t i = 0; i < cnt; i++) {
		if (isBuddy(list[i])) {
			if (cpt++ == device_number) {
				setColor(list[i], color_mask);
			}
		}
	}
	libusb_free_device_list(list, 0); //free the list, unref the devices in it
	libusb_exit(ctx); //close the session
	return cpt;
}

int main(int argc, char* argv[]) {
	int device = 0;
	int color = -1;
	int c;
	while (true) {
		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long(argc, argv, g_short_options, g_long_options,
				&option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			++g_verbose_flag;
			break;

		case 'h':
			LOG("Usage: %s\n%s", argv[0], HELP);
			exit(EXIT_SUCCESS);
			break;

		case 'd':
			device = atoi(optarg);
			INFO_LOG("asking for device %d\n", device);
			break;

		case 'c':
			for (int i = 0; g_colors[i].name != 0; ++i) {
				if (strcasecmp(optarg, g_colors[i].name) == 0) { // match
					color = g_colors[i].value;
					INFO_LOG("asking for color %s\n", g_colors[i].name);
					break;
				}
			}
			break;

		case '?':
			break;

		default:
			ERROR_LOG("Unknown error while parsing command line. Exiting.");
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (color == -1) {
		WARNING_LOG("no color specified. Read help --help.\n");
		exit(EXIT_FAILURE);
	}
	setColorOnDevice(device, color);
	return 0;
}
