#include "cmsis_os.h"

extern osMessageQId printQueueHandle;

int _write(int fd, char *ptr, int len)
{
    int i;
    (void)fd;
    for (i = 0; i < len; i++)
        osMessagePut(printQueueHandle, *ptr++, 0);
    return len;
}