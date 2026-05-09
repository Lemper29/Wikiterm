#define _XOPEN_SOURCE 700

#include "display.h"
#include <stdio.h>

void display_text(const char *text)
{
    if (!text) return;
    
    FILE *less = popen("less -R", "w");
    if (less) {
        fprintf(less, "%s\n", text);
        pclose(less);
    } else {
        puts(text);
    }
}
