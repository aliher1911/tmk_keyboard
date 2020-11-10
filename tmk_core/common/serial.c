#include "serial.h"

#include <stdint.h>
#include <string.h>
#include "host.h"
#include "action_layer.h"

#define RING_BUFFER_SIZE (32)

typedef struct {
  uint8_t data[RING_BUFFER_SIZE];
  uint8_t write;
  uint8_t read;
} ringbuffer_t;


#define MAX_COMMAND_LENGTH (32)
  
typedef struct {
  uint8_t data[MAX_COMMAND_LENGTH];
  uint8_t write;
} cmdbuffer_t;

/***************
 * Ring buffer *
 **************/

bool has_data(const ringbuffer_t *buffer) {
  return buffer->write != buffer->read;
}

// check available space
uint8_t available_space(const ringbuffer_t *buffer) {
  return (buffer->read - buffer->write) % RING_BUFFER_SIZE - 1;
}

// Push chars to ringbuffer, no size check is done
inline void push(ringbuffer_t *buffer, uint8_t data) {
  buffer->data[buffer->write] = data;
  buffer->write = (buffer->write + 1) % RING_BUFFER_SIZE;
}

uint8_t* read_data(ringbuffer_t *buffer, uint8_t *size) {
  if (buffer->write >= buffer->read) {
    // case where buffer is not reversed
    *size = buffer->write - buffer->read;
  } else {
    // buffer is fragmented, only return first part
    *size = RING_BUFFER_SIZE - buffer->read;
  }
  return buffer->data + buffer->read;
}

void pop(ringbuffer_t *buffer, uint8_t count) {
  buffer->read = (buffer->read + count) % RING_BUFFER_SIZE;
}

/**************
 * CMD buffer *
 *************/
inline uint8_t* write_pointer(cmdbuffer_t *buffer, uint8_t *size) {
  *size = MAX_COMMAND_LENGTH - buffer->write;
  return buffer->data + buffer->write;
}

// unsafe to push beyond limit
inline void commit(cmdbuffer_t *buffer, uint8_t size) {
  buffer->write += size;
}

inline uint8_t available_cmd_space(cmdbuffer_t *buffer) {
  return MAX_COMMAND_LENGTH - buffer->write;
}

inline uint8_t size(cmdbuffer_t *buffer) {
  return buffer->write;
}

inline uint8_t at(cmdbuffer_t *buffer, uint8_t index) {
  return buffer->data[index];
}

void splice(cmdbuffer_t *buffer, uint8_t size) {
  if (buffer->write != size) {
    memcpy(buffer->data, buffer->data + size, size);
  }
  buffer->write -= size;
}

uint8_t clear(cmdbuffer_t *buffer) {
  buffer->write = 0;
  return MAX_COMMAND_LENGTH;
}

static ringbuffer_t out_buffer;

// send ok response
void push_ok(void) {
  if (available_space(&out_buffer) > 3) {
    push(&out_buffer, 'O');
    push(&out_buffer, 'K');
    push(&out_buffer, '\n');
  }
}

// send error response
void push_err(SerialCommandError err) {
  if (available_space(&out_buffer) > 6) {
    push(&out_buffer, 'E');
    push(&out_buffer, 'R');
    push(&out_buffer, 'R');
    push(&out_buffer, ':');
    push(&out_buffer, ((uint8_t)err) + '0');
    push(&out_buffer, '\n');
  }
}

static cmdbuffer_t cmd_buffer;

// setup buffers etc for operation
void serial_control_init(void) {
  cmd_buffer.write = 0;
  out_buffer.write = 0;
  out_buffer.read = 0;
}

uint8_t read_arg8(uint8_t *cmd, uint8_t size, bool *error) {
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

void process_layer_cmd(uint8_t *cmd, uint8_t size) {
  bool err = false;
  uint8_t layer = read_arg8(cmd, size, &err);
  if (err) {
    push_err(BAD_ARGUMENT);
  } else {
    layer_on(layer);
    push_ok();
  }
}

// handle command in buffer
void process_cmd(uint8_t *cmd, uint8_t size) {
  if (size < 3) {
    push_err(FRAME);
    return;
  }
  if (cmd[0] != 'K' && cmd[1] != 'B') {
    push_err(FRAME);
    return;
  }
  uint8_t action = cmd[2];
  switch(action) {
  case 'L':
    process_layer_cmd(cmd + 3, size - 3);
    break;
  default:
    push_err(BAD_COMMAND);
  }
}

// process incoming/outgoing data
void serial_control_task(void) {
  // process commands
  uint8_t remaining;
  uint8_t *buffer = write_pointer(&cmd_buffer, &remaining);
  if (remaining == 0) {
    clear(&cmd_buffer);
    buffer = write_pointer(&cmd_buffer, &remaining);
    push_err(OVERFLOW);
  }
  uint8_t received = host_receive_serial(buffer, remaining);
  if (received > 0) {
    commit(&cmd_buffer, received);
  }

  uint8_t first = 0; // unprocessed character
  uint8_t cmd_size = size(&cmd_buffer);
  for(int i=0; i<cmd_size; i++) {
    uint8_t next_char = at(&cmd_buffer, i);
    if (next_char == '\n' || next_char == '\r') {
      if (i - first > 0) {
	process_cmd(cmd_buffer.data + first, i-first);
      }
      first = i+1;
    }
  }
  // discard processed part
  if (first > 0) {
    splice(&cmd_buffer, first);
  }
  
  // send bytes if pending
  if (has_data(&out_buffer)) {
    uint8_t size;
    uint8_t *buf = read_data(&out_buffer, &size);
    uint8_t sent = host_send_serial(buf, size);
    pop(&out_buffer, sent);
  }
}
