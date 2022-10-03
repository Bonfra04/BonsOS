#include "escape.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define ESCAPE_INVALID (escape_t){.type = ESC_NONE }

static escape_t read_csi()
{
    escape_t escape = { .type = ESC_CSI };

    char params[2][21];
    memset(params, 0, sizeof(params));
    uint8_t current_param = 0;

    char ch;
    while((ch = fgetc(stdin)) != EOF)
    {
        if(isdigit(ch))
            strcat(params[current_param], (char[]){ ch, '\0'});
        else if(ch == ';')
            current_param++;
        else
            switch (ch)
            {        
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'J': case 'K': case 'S': case 'T': case 'm':
                escape.csi.type = ch;
                escape.csi.param0 = isdigit(params[0][0]) ? strtoi(params[0], NULL, 10) : 1;
                goto end;

            case 'H': case 'f':
            {
                escape.csi.type = ch;
                escape.csi.param0 = strtoi(params[0], NULL, 10);
                escape.csi.param1 = strtoi(params[1], NULL, 10);
                goto end;
            }

            case 'i': case 'n':
            {
                escape.csi.type = ch;
                escape.csi.param0 = isdigit(params[0][0]) ? params[0][0] - '0' : 0;
                goto end;
            }

            default:
                return ESCAPE_INVALID;
            }
    }

    end:
    return escape;
}

escape_t readescape()
{
    switch (fgetc(stdin))
    {
        // TODO: all of them
        case 'N': break;
        case 'O': break;
        case 'P': break;
        case '[': return read_csi();
        case '\\': break;
        case ']': break;
        case 'X': break;
        case '^': break;
        case '_': break;
        default: return ESCAPE_INVALID;
    }

    return ESCAPE_INVALID;
}

static void print_csi(const escape_t* escape)
{
    switch (escape->csi.type)
    {
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'G': case 'J': case 'K': case 'S': case 'T': case 'm':
    case 'i': case 'n':
        fprintf(stdout, "%d", escape->csi.param0);
        break;

    case 'H': case 'f':
        fprintf(stdout, "%d;%d", escape->csi.param0, escape->csi.param1);
        break;
    }
    fputc(escape->csi.type, stdout);
}

void printescape(const escape_t* escape)
{
    fputc(0x1b, stdout);
    fputc(escape->type, stdout);

    switch (escape->type)
    {
    // TODO: all of them
    case ESC_SS2: break;
    case ESC_SS3: break;
    case ESC_DCS: break;
    case ESC_CSI: print_csi(escape); break;
    case ESC_ST: break;
    case ESC_OSC: break;
    case ESC_SOS: break;
    case ESC_PM: break;
    case ESC_APC: break;
    }
}

escape_keyboard_t escape_simpify(const escape_t* escape)
{
    if(escape->type != ESC_CSI)
        return ESCKB_NONE;

    switch (escape->csi.type)
    {
    case 'A': return ESCKB_ARROW_UP;
    case 'B': return ESCKB_ARROW_DOWN;
    case 'C': return ESCKB_ARROW_RIGHT;
    case 'D': return ESCKB_ARROW_LEFT;
    default: return ESCKB_NONE;
    }
}
