/**************************************************
 * Interfacing with keyboard via CDC class device *
 *************************************************/
#ifndef CDC_H
#define CDC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// send individual characters, used by print functions
void cdc_sendchar(uint8_t data);
  
// send block of data, used by serial interface. data is placed
// in ring buffer for attempt to send at the end of the next loop
uint8_t send_serial(uint8_t *buffer, uint8_t size);

// receive block of data, used by serial interface
// data is read directly from cdc interface into provided buffer
uint8_t receive_serial(uint8_t *buffer, uint8_t size);

// serial end loop task as used by lufa main loop
void serial_usb_task(void);

bool cdc_configure_endpoint(void);
void cdc_control_request(void);

#ifdef __cplusplus
}
#endif

#endif
