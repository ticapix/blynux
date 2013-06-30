#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#define ERROR_LOG(...) {printf("EE "); printf(__VA_ARGS__);}

#define PID 0x0001
#define VID 0x1130

//namespace std {
void printdev(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		std::cout << "failed to get device descriptor" << std::endl;
		return;
	}
	std::cout << "Number of possible configurations: "
			<< (int) desc.bNumConfigurations << "  ";
	std::cout << "Device Class: " << (int) desc.bDeviceClass << "  ";
	std::cout << "VendorID: " << desc.idVendor << "  ";
	std::cout << "ProductID: " << desc.idProduct << std::endl;
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	std::cout << "Interfaces: " << (int) config->bNumInterfaces << " ||| ";
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	for (int i = 0; i < (int) config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		std::cout << "Number of alternate settings: " << inter->num_altsetting
				<< " | ";
		for (int j = 0; j < inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			std::cout << "Interface Number: "
					<< (int) interdesc->bInterfaceNumber << " | ";
			std::cout << "Number of endpoints: "
					<< (int) interdesc->bNumEndpoints << " | ";
			for (int k = 0; k < (int) interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				std::cout << "Descriptor Type: "
						<< (int) epdesc->bDescriptorType << " | ";
				std::cout << "EP Address: " << (int) epdesc->bEndpointAddress
						<< " | ";
			}
		}
	}
	std::cout << std::endl << std::endl << std::endl;
	libusb_free_config_descriptor(config);
}

bool isBuddy(libusb_device *dev) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		std::cout << "failed to get device descriptor" << std::endl;
		return false;
	}
	return desc.idVendor == VID && desc.idProduct == PID;
}

enum colors {
	WHITE = 0x8,
	CYAN = 0x9,
	MAGENTA = 0xa,
	BLUE = 0xb,
	YELLOW = 0xc,
	GREEN = 0xd,
	RED = 0xe,
	OFF = 0xf
};

void findBuddy() {
	// discover devices
	libusb_device **list;
	libusb_context *ctx = NULL; //a libusb session
	libusb_init(&ctx); //initialize a library session

	ssize_t cnt = libusb_get_device_list(ctx, &list);
	int i;
	if (cnt < 0) {
		ERROR_LOG("no usb device found");
		cnt = 0;
	}
	for (i = 0; i < cnt; i++) {
		if (isBuddy(list[i])) {
			int r = 0;
			printdev(list[i]);
			libusb_device_handle* dev_handle = NULL;
			if (libusb_open(list[i], &dev_handle) < 0)
				ERROR_LOG("error on opening handle");

			if (libusb_kernel_driver_active(dev_handle, 1) == 1) { //find out if kernel driver is attached
				std::cout << "Kernel Driver Active" << std::endl;
				if (libusb_detach_kernel_driver(dev_handle, 1) == 0) //detach it
					std::cout << "Kernel Driver Detached!" << std::endl;
			}
			r = libusb_claim_interface(dev_handle, 1); //claim interface 0 (the first) of device (mine had jsut 1)
			if (r < 0) {
				std::cout << "Cannot Claim Interface" << std::endl;
				return;
			}
			std::cout << "Claimed Interface" << std::endl;

			std::cout << "Writing Data..." << std::endl;
			unsigned char data[8] = { 0x55, 0x53, 0x42, 0x43, 0x00, 0x40, 0x02,	0x0f };

			unsigned char c7 = OFF;
			{
				data[7] = (c7 << 4) | 0xf;

						uint8_t bmRequestType = 0x21;
						uint8_t bRequest = 9;
						uint16_t wValue = 0x0200;
						uint16_t wIndex = 1;
						uint16_t wLength = 8;
						unsigned int timeout = 1000 * 2; // 2 secs

						r = libusb_control_transfer(dev_handle, bmRequestType,
								bRequest, wValue, wIndex, data, wLength,
								timeout);
						printf("res: %d\n", r);
						if (r == wLength)
							std::cout << "Writing Successful!" << std::endl;
						else {
							std::cout << "Write Error" << std::endl;
							break;
						}

						usleep(1000 * 100);
//						sleep(1);
						if (c7 == 255)
							break;
					}

			r = libusb_release_interface(dev_handle, 1); //release the claimed interface
			if (r != 0) {
				std::cout << "Cannot Release Interface" << std::endl;
				return;
			}
			std::cout << "Released Interface" << std::endl;

			libusb_close(dev_handle); //close the device we opened
		}
	}
	libusb_free_device_list(list, 1); //free the list, unref the devices in it
	libusb_exit(ctx); //close the session
}

//}

int main() {
	findBuddy();
	return 0;
}
