#include "msg.h"
#include "../window/window.h"
#include "../renderer/renderer.h"

#define MSG_CREATE_WND  1
#define RSP_CREATE_WND  2
#define MSG_RESIZE_WND  3
#define RSP_RESIZE_WND  4
#define MSG_MOVE_WND    5
#define MSG_SCREEN_SIZE 6
#define RSP_SCREEN_SIZE 7

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

typedef struct move_wnd_msg
{
    uint64_t wnd_id;
    size_t x, y;
} __attribute__ ((packed)) move_wnd_msg_t;

typedef struct screen_size_rsp
{
    size_t width, height;
} __attribute__ ((packed)) screen_size_rsp_t;

static void handle_create_wnd(uint64_t sender, msg_t* msg)
{
    create_wnd_msg_t* create_wnd_msg = (create_wnd_msg_t*)msg->data;
    msg_t response;
    response.id = RSP_CREATE_WND;
    create_wnd_rsp_t* create_wnd_rsp = (create_wnd_rsp_t*)response.data;
    create_wnd_rsp->wnd_id = window_create(sender, create_wnd_msg->flags);
    msg_send(sender, &response);
}

static void handle_resize_wnd(uint64_t sender, msg_t* msg)
{
    resize_wnd_msg_t* resize_wnd_msg = (resize_wnd_msg_t*)msg->data;
    msg_t response;
    response.id = RSP_RESIZE_WND;
    resize_wnd_rsp_t* resize_wnd_rsp = (resize_wnd_rsp_t*)response.data;
    void* fb = window_resize(resize_wnd_msg->wnd_id, resize_wnd_msg->width, resize_wnd_msg->height);
    resize_wnd_rsp->framebuffer = map_mem(fb, 0);
    msg_send(sender, &response);
}

static void handle_move_wnd(uint64_t sender, msg_t* msg)
{
    move_wnd_msg_t* move_wnd_msg = (move_wnd_msg_t*)msg->data;
    window_move(move_wnd_msg->wnd_id, move_wnd_msg->x, move_wnd_msg->y);
}

static void handle_screen_size(uint64_t sender)
{
    msg_t response;
    response.id = RSP_SCREEN_SIZE;
    screen_size_rsp_t* screen_size_rsp = (screen_size_rsp_t*)response.data;
    screen_size_rsp->width = display_width();
    screen_size_rsp->height = display_height();
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
        case MSG_MOVE_WND:
            handle_move_wnd(sender, msg);
            break;
        case MSG_SCREEN_SIZE:
            handle_screen_size(sender);
            break;
    }    
}