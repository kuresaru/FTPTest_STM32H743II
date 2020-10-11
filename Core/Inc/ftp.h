#ifndef __FTP_H
#define __FTP_H
#include <stdint.h>

enum ftp_status_code
{
    NONE = 0,
    USER,
    PASS,
    NOOP,
    PASV,
    RETR,
};
extern enum ftp_status_code ftp_status;

void ftp_init();
void ftp_getfile(char *filename, void *filebuf, uint16_t buflen, uint16_t *read);

#endif