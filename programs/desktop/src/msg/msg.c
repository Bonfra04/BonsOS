#include "msg.h"
#include "../window/window.h"

#define MSG_CREATE_WND 1
#define RSP_CREATE_WND 2
#define MSG_RESIZE_WND 3
#define RSP_RESIZE_WND 4

typedef struct create_wnd_msg
{
    uint8_t flags;
} __attribute__ ((packed)) create_wnd_msg_t;

typedef struct create_wnd_rsp
{
    uint64_t wnd_id;
} __attribute__ ((packed)) create_wnd_rsp_t;

typedef struct resize_wnd_msg
{
    uint64_t wnd_id;
    size_t width, height;
} __attribute__ ((packed)) resize_wnd_msg_t;

typedef struct resize_wnd_rsp
{
    void* framebuffer;
} __attribute__ ((packed)) resize_wnd_rsp_t;

static void handle_create_wnd(uint64_t sender, msg_t* msg)
{
    create_wnd_msg_t* create_wnd_msg = (create_wnd_msg_t*)msg->data;
    msg_t response;
    response.id = RSP_CREATE_WND;
    create_wnd_rsp_t* create_wnd_rsp = (create_wnd_rsp_t*)response.data;
    create_wnd_rsp->wnd_id = window_create(create_wnd_msg->flags);
    msg_send(sender, &response);
}

static void handle_resize_wnd(uint64_t sender, msg_t* msg)
{
    resize_wnd_msg_t* resize_wnd_msg = (resize_wnd_msg_t*)msg->data;
    msg_t response;
    response.id = RSP_RESIZE_WND;
    resize_wnd_rsp_t* resize_wnd_rsp = (resize_wnd_rsp_t*)response.data;
    resize_wnd_rsp->framebuffer = window_resize(resize_wnd_msg->wnd_id, resize_wnd_msg->width, resize_wnd_msg->height);
    msg_send(sender, &response);
}

void msg_process(uint64_t sender, msg_t* msg)
{
    switch (msg->id)
    {
        case MSG_CREATE_WND:
            handle_create_wnd(sender, msg);
            break;
        case MSG_RESIZE_WND:
            handle_resize_wnd(sender, msg);
            break;
    }    
}