/**************************************************
 * Interfacing with keyboard via CDC class device *
 *************************************************/
#include "cdc.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/Class/Device/CDCClassDevice.h>

#include "descriptor.h"
#include "ringbuf.h"

// size of ring buffer to buffer data before sending
// writing more than buffer per scan iteration would cause
// remainder to be dropped
#ifndef CDC_RING_BUFFER_SIZE
#define CDC_RING_BUFFER_SIZE (32)
#endif

// Serial device info which mirrors descriptor data
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
  {
   .Config =
   {
    .ControlInterfaceNumber         = SERIAL_CCI_INTERFACE,
    .DataINEndpoint                 =
    {
     .Address                = (ENDPOINT_DIR_IN | SERIAL_TX_EPNUM),
     .Size                   = SERIAL_TXRX_EPSIZE,
     .Banks                  = 1,
    },
    .DataOUTEndpoint                =
    {
     .Address                = (ENDPOINT_DIR_OUT | SERIAL_RX_EPNUM),
     .Size                   = SERIAL_TXRX_EPSIZE,
     .Banks                  = 1,
    },
    .NotificationEndpoint           =
    {
     .Address                = (ENDPOINT_DIR_IN | SERIAL_NOTIF_EPNUM),
     .Size                   = SERIAL_NOTIF_EPSIZE,
     .Banks                  = 1,
    },
   },
  };

static uint8_t output_data[CDC_RING_BUFFER_SIZE];
static ringbuf_t output_buf = {
    .buffer = output_data,
    .head = 0,
    .tail = 0,
    .size_mask = CDC_RING_BUFFER_SIZE - 1
};

// send individual characters, used by print functions
void cdc_sendchar(uint8_t data) {
  // ignore requests from interrupt as unsafe
  if (!(SREG & (1<<SREG_I)))
    return;

  ringbuf_put(&output_buf, data);
}

// send block of data, used by serial interface. data is placed
// in ring buffer for attempt to send at the end of the next loop
uint8_t send_serial(uint8_t *buffer, uint8_t size) {
  int sent = 0;
  for(;sent < size; sent++) {
    // using lossy head
    if (!ringbuf_put(&output_buf, buffer[sent])) {
      // if we run out of space, exit early
      break;
    }
  }
  return sent;
}

// receive block of data, used by serial interface
// data is read directly from cdc interface into provided buffer
uint8_t receive_serial(uint8_t *buffer, uint8_t size) {
  uint8_t received = 0;
  uint16_t pending_bytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
  if (pending_bytes > 0) {
    uint8_t expected = pending_bytes < size? pending_bytes : size;
    while(received < expected) {
      int16_t received_byte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
      if (received_byte >= 0) {
	// process received data
	buffer[received] = (uint8_t)received_byte;
      }
      ++received;
    }
  }

  return received;
}

// serial end loop task as used by lufa main loop
void serial_usb_task(void) {
  // send outgoing data
  if (!ringbuf_is_empty(&output_buf)) {
    uint8_t size;
    uint8_t *data = ringbuf_peek_data(&output_buf, &size);
    // send to endpoint as needed
    if (size > 0) {
      uint8_t bytes_to_send = size < (SERIAL_TXRX_EPSIZE - 1) ? size : (SERIAL_TXRX_EPSIZE - 1);
      if (CDC_Device_SendData(&VirtualSerial_CDC_Interface, (void*)data, bytes_to_send)
	  == ENDPOINT_RWSTREAM_NoError) {
	// Endpoint is already selected by write
	if (Endpoint_IsINReady() && Endpoint_BytesInEndpoint()) {
	  // Pending bytes in endpoint need to flush
	  Endpoint_ClearIN();
	}
	// Remove pending data from ringbuffer
	ringbuf_skip_data(&output_buf, bytes_to_send);
      }
    }
  }

  // Delegate to CDC loop task
  CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
}

bool cdc_configure_endpoint() {
  return CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void cdc_control_request() {
  CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
