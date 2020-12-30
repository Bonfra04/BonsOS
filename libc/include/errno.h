#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int* _errno(void);
void _set_errno(int value);
int _get_errno(void);

#define errno (*_errno())

#ifdef __cplusplus
}
#endif