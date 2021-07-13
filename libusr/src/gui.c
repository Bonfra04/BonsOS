#include <gui.h>
#include <syscalls.h>

#define HOST_PID 2

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

window_t create_window(const char* title, size_t width, size_t height, size_t x, size_t y, uint8_t flags)
{
    // send creation data
    msg_t msg;
    create_wnd_msg_t* data = (create_wnd_msg_t*)&msg.data;
    msg.id = MSG_CREATE_WND;
    data->flags = flags;
    msg_send(HOST_PID, &msg);

    // wait for response
    while(msg.id != RSP_CREATE_WND)
        msg_fetch(&msg);
    create_wnd_rsp_t* rsp = (create_wnd_rsp_t*)&msg.data;

    window_t window;
    window.id = rsp->wnd_id;

    // set other parameters
    window_move(window, x, y);
    window.framebuffer = window_resize(window, width, height);
    window_set_title(window, title);

    return window;
}

void window_move(window_t window, size_t x, size_t y)
{
    return;
}

void* window_resize(window_t window, size_t width, size_t height)
{
    // send size data
    msg_t msg;
    resize_wnd_msg_t* data = (resize_wnd_msg_t*)&msg.data;
    msg.id = MSG_RESIZE_WND;
    data->width = width;
    data->height = height;
    data->wnd_id = window.id;
    msg_send(HOST_PID, &msg);

    // wait for response
    while(msg.id != RSP_RESIZE_WND)
        msg_fetch(&msg);
    resize_wnd_rsp_t* rsp = (resize_wnd_rsp_t*)&msg.data;

    return 0;//rsp->framebuffer;
}

void window_set_title(window_t window, const char* title)
{
    return;
}

size_t display_width()
{
    return 100;
}

size_t display_height()
{
    return 100;
}