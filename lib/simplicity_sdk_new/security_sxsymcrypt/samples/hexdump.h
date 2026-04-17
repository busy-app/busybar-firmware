#ifndef HEXDUMP_HEADERFILE
#define HEXDUMP_HEADERFILE

#include "env/io.h"


static inline void displayhex(unsigned char v)
{
    char c[2] = {0};

    for (int i = 4; i >= 0; i -= 4) {
        unsigned char nibble = (v >> i) & 0xF;
        *c = '0' + nibble;
        if (*c > '9')
            *c +=  'a' - '9' - 1;
        displaymsg(c);
    }
}

static inline void hexdump(const char *name, const char *v, size_t len)
{
    (void)name;
    (void)v;
    (void)len;

    displaymsg("\n");
    displaymsg(name);
    displaymsg(":\n ");
    for (size_t i = 0; i < len; i++) {
        displayhex((unsigned char)*v++);
        if (i%16==15)
            displaymsg("\n ");
        if (i%16==7)
            displaymsg(" ");
    }
    displaymsg("\n");
}

#endif
