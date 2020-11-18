#include "remote_control.h"

#include <stdint.h>
#include <string.h>
#include "host.h"
#include "action_layer.h"

#define MAX_COMMAND_LENGTH (32)
  
typedef struct {
  uint8_t data[MAX_COMMAND_LENGTH];
  uint8_t write;
} cmdbuffer_t;

/**************
 * CMD buffer *
 *************/
inline uint8_t* cmdbuf_write_pointer(cmdbuffer_t *buffer, uint8_t *size) {
  *size = MAX_COMMAND_LENGTH - buffer->write;
  return buffer->data + buffer->write;
}

// unsafe to push beyond limit
inline void cmdbuf_commit(cmdbuffer_t *buffer, uint8_t size) {
  buffer->write += size;
}

inline uint8_t cmdbuf_available(cmdbuffer_t *buffer) {
  return MAX_COMMAND_LENGTH - buffer->write;
}

inline uint8_t cmdbuf_size(cmdbuffer_t *buffer) {
  return buffer->write;
}

inline uint8_t cmdbuf_at(cmdbuffer_t *buffer, uint8_t index) {
  return buffer->data[index];
}

inline void cmdbuf_splice(cmdbuffer_t *buffer, uint8_t size) {
  if (buffer->write != size) {
    memcpy(buffer->data, buffer->data + size, size);
  }
  buffer->write -= size;
}

inline uint8_t cmdbuf_clear(cmdbuffer_t *buffer) {
  buffer->write = 0;
  return MAX_COMMAND_LENGTH;
}

static uint8_t ok_response[3] = {'O', 'K', '\n'};
// send ok response
static void push_ok(void) {
  host_send_serial(ok_response, 3);
}

static uint8_t err_response[6] = {'E', 'R', 'R', ':', '0', '\n'};
// send error response
static void push_err(SerialCommandError err) {
  err_response[4] = (uint8_t)err + '0';
  host_send_serial(err_response, 6);
}

static cmdbuffer_t cmd_buffer;

// setup buffers etc for operation
void serial_control_init(void) {
  cmdbuf_clear(&cmd_buffer);
}

static uint8_t read_arg8(uint8_t *cmd, uint8_t size, bool *error) {
  if (size == 0) {
    *error = true;
    return 0;
  }
  uint16_t result = 0;
  for(int i=0; i < size; i++) {
    uint8_t next = cmd[i];
    if (next < '0' || next > '9') {
      *error = true;
      return 0;
    }
    result += (next - '0');
    if (result > 255) {
      *error = true;
      return 0;
    }
  }
  return result;
}

static SerialCommandError process_layer_cmd(uint8_t *cmd, uint8_t size) {
  bool err = false;
  if (size==0) {
    return BAD_ARGUMENT;
  }
  uint8_t operation = cmd[0];
  uint8_t layer = read_arg8(cmd+1, size-1, &err);
  if (err) {
    return BAD_ARGUMENT;
  } else {
    switch(operation) {
    case 'S': // set
      layer_on(layer);
      break;
    case 'R': // reset
      layer_off(layer);
      break;
    case 'T': // toggle
      layer_invert(layer);
      break;
    default:
      return BAD_ARGUMENT;
    }
  }
  push_ok();
  return CMD_OK;
}

// handle command in buffer by delegating to handle function per type
static void process_cmd(uint8_t *cmd, uint8_t size) {
  if (size < 3) {
    push_err(FRAME);
    return;
  }
  if (cmd[0] != 'K' && cmd[1] != 'B') {
    push_err(FRAME);
    return;
  }
  SerialCommandError err;
  uint8_t action = cmd[2];
  switch(action) {
  case 'L':
    err = process_layer_cmd(cmd + 3, size - 3);
    break;
  default:
    err = BAD_COMMAND;
  }
  if (err) {
    push_err(BAD_COMMAND);
  }
}

// process incoming/outgoing data
void serial_control_task(void) {
  // process commands
  uint8_t remaining;
  uint8_t *buffer = cmdbuf_write_pointer(&cmd_buffer, &remaining);
  if (remaining == 0) {
    cmdbuf_clear(&cmd_buffer);
    buffer = cmdbuf_write_pointer(&cmd_buffer, &remaining);
    push_err(OVERFLOW);
  }
  uint8_t received = host_receive_serial(buffer, remaining);
  if (received > 0) {
    cmdbuf_commit(&cmd_buffer, received);
  }

  uint8_t first = 0; // unprocessed character
  uint8_t cmd_size = cmdbuf_size(&cmd_buffer);
  for(int i=0; i<cmd_size; i++) {
    uint8_t next_char = cmdbuf_at(&cmd_buffer, i);
    if (next_char == '\n' || next_char == '\r') {
      if (i - first > 0) {
	process_cmd(cmd_buffer.data + first, i-first);
      }
      first = i+1;
    }
  }
  // discard processed part
  if (first > 0) {
    cmdbuf_splice(&cmd_buffer, first);
  }
}
