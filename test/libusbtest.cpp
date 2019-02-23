#include <libusb-1.0/libusb.h>
#include <stdio.h>

static int ezusb_read(libusb_device_handle *device, const char *label,
	uint8_t opcode, uint32_t addr, const unsigned char *data, size_t len)
{
	int status;

	printf("%s, addr 0x%08x len %4u (0x%04x)\n", label, addr, (unsigned)len, (unsigned)len);
	status = libusb_control_transfer(device,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		opcode, addr & 0xFFFF, addr >> 16,
		(unsigned char*)data, (uint16_t)len, 1000);
	if (status != (signed)len) {
		if (status < 0)
			printf("%s: %s\n", label, libusb_error_name(status));
		else
			printf("%s ==> %d\n", label, status);
	}
	return (status < 0) ? 1 : 0;
}

void libusb_test(){
	libusb_init(NULL);
	libusb_device_handle* handle = NULL;
	handle = libusb_open_device_with_vid_pid (NULL, 0x16c0, 0x05e4);//16c0:05e4
	if (!handle) {
		fprintf(stderr, "Error finding USB device\n");
	}

	uint8_t data[8] = {0};

	ezusb_read(handle, "vusb", 0x00, 0, data, 8);

	libusb_close(handle);
	libusb_exit(NULL);
	return;
}
