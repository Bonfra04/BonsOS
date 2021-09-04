#include "event.h"
#include <syscalls.h>

#define MSG_EVENT 0xFF

typedef struct event_msg
{
    uint64_t wnd_id;
    uint8_t msg_id;
} __attribute__ ((packed)) event_msg_t;

void event_send(const window_t* window, uint8_t event_id)
{
    if(window == NULL)
        return;

    msg_t msg;
    msg.id = MSG_EVENT;
    event_msg_t* event_msg = (event_msg_t*)msg.data;
    event_msg->wnd_id = window->id;
    event_msg->msg_id = event_id;
    msg_send(window->owner, &msg);
}