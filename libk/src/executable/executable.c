#include <executable/executable.h>
#include <fsys/fsys.h>
#include <memory/pfa.h>
#include <executable/elf.h>

#include <string.h>
#include <stdlib.h>

executable_t* executable_load(const char* path)
{
    file_t f = fsys_open_file(path, FSYS_READ);
    if(fsys_error(&f))
    {
        fsys_close_file(&f);
        return NULL;
    }

    fsys_set_position(&f, -1);
    size_t len = fsys_get_position(&f);
    fsys_set_position(&f, 0);

    size_t num_pages = len / PFA_PAGE_SIZE + (len % PFA_PAGE_SIZE != 0);
    uint8_t* file_data = pfa_alloc(num_pages);
    if(fsys_read_file(&f, file_data, len) != len)
    {
        pfa_free(file_data, num_pages);
        fsys_close_file(&f);
        return NULL;
    }
    fsys_close_file(&f);

    executable_t* exec = NULL;
    if(elf_check(file_data))
        exec = elf_load(file_data, num_pages);

    exec->fullpath = strdup(path);
    return exec;
}

void executable_unload(const executable_t* executable)
{
    free(executable->fullpath);

    switch (executable->format)
    {
    case EXEC_ELF:
        elf_unload(executable);
        break;

    default:
        break;
    }
}
