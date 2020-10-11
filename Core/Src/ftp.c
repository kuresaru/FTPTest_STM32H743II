#include "ftp.h"
#include "lwip.h"
#include "ip_addr.h"
#include "tcp.h"
#include <string.h>

#define FTP_DEBUG 0  // 输出有问题

#if FTP_DEBUG
#include "cmsis_os.h"
extern osMessageQId printQueueHandle;
#endif

enum ftp_status_code ftp_status = NONE;

static struct tcp_pcb *svr_pcb;
static ip_addr_t svr_addr;
static struct tcp_pcb *data_pcb;
static ip_addr_t ftp_pasv_addr;
static uint16_t ftp_pasv_port;
char *ftp_filename;
void *ftp_filebuf;
uint16_t ftp_filebuf_len;
uint16_t *ftp_fileread;

static err_t ftp_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    return err;
}

static void ftp_send(struct tcp_pcb *pcb, char *cmd)
{
    size_t len = strlen(cmd);
#if FTP_DEBUG
    printf(cmd);
#endif
    tcp_write(pcb, cmd, len, TCP_WRITE_FLAG_COPY);
}

static err_t ftp_data_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *buf, err_t err)
{
    if (buf)
    {
        if (ftp_filebuf)
        {
            *ftp_fileread = ftp_filebuf_len < buf->len ? ftp_filebuf_len : buf->len;
            memcpy(ftp_filebuf, buf->payload, *ftp_fileread);
        }
    }
    return err;
}

static err_t ftp_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *buf, err_t err)
{
    if (buf)
    {
        char *ret = (char *)buf->payload;
#if FTP_DEBUG
        uint16_t i;
        // debug print
        for (i = 0; i < buf->len; i++)
            osMessagePut(printQueueHandle, *((uint8_t *)buf->payload + i), 0);
#endif
        if (ftp_status == NONE && !memcmp(ret, "220 ", 4))
        {
            ftp_status = USER;
            ftp_send(pcb, "USER anonymous\r\n");
        }
        else if (ftp_status == USER && !memcmp(ret, "331 ", 4))
        {
            ftp_status = PASS;
            ftp_send(pcb, "PASS\r\n");
        }
        else if (ftp_status == PASS && !memcmp(ret, "230 ", 4))
        {
            ftp_status = NOOP;
        }
        else if (ftp_status == PASV && !memcmp(ret, "227 ", 4))
        {
            char ibuf[4];
            char *start = strchr(ret, '(') + 1;
            char *end = strchr(start, ',');
            uint8_t ilen = (uint8_t)(end - start);
            // h1
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_addr.addr = (atoi(ibuf) & 0xFF) << 24;
            // h2
            start = end + 1;
            end = strchr(start, ',');
            ilen = (uint8_t)(end - start);
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_addr.addr |= (atoi(ibuf) & 0xFF) << 16;
            // h3
            start = end + 1;
            end = strchr(start, ',');
            ilen = (uint8_t)(end - start);
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_addr.addr |= (atoi(ibuf) & 0xFF) << 8;
            // h4
            start = end + 1;
            end = strchr(start, ',');
            ilen = (uint8_t)(end - start);
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_addr.addr |= (atoi(ibuf) & 0xFF);
            // fix
            ftp_pasv_addr.addr = PP_HTONL(ftp_pasv_addr.addr);
            // p1
            start = end + 1;
            end = strchr(start, ',');
            ilen = (uint8_t)(end - start);
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_port = (atoi(ibuf) & 0xFF) << 8;
            // p2
            start = end + 1;
            end = strchr(start, ')');
            ilen = (uint8_t)(end - start);
            memcpy(ibuf, start, ilen);
            ibuf[ilen] = '\0';
            ftp_pasv_port |= atoi(ibuf) & 0xFF;
            // RETR
            ftp_status = RETR;
            ftp_send(pcb, "RETR ");
            ftp_send(pcb, ftp_filename);
            ftp_send(pcb, "\r\n");
        }
        else if (ftp_status == RETR && !memcmp(ret, "150 ", 4))
        {
            data_pcb = tcp_new();
            tcp_connect(data_pcb, &ftp_pasv_addr, ftp_pasv_port, &ftp_connected);
            tcp_recv(data_pcb, &ftp_data_recv);
        }
        else if (ftp_status == RETR && !memcmp(ret, "226 ", 4))
        {
            tcp_recv(data_pcb, NULL);
            tcp_close(data_pcb);
            ftp_filename = NULL;
            ftp_filebuf = NULL;
            ftp_fileread = NULL;
            ftp_status = NOOP;
        }
    }
    else
    {
        printf("ftp connection closed\r\n");
        tcp_recv(svr_pcb, NULL);
        tcp_close(svr_pcb);
    }
    return err;
}

void ftp_getfile(char *filename, void *filebuf, uint16_t buflen, uint16_t *read)
{
    if (ftp_status == NOOP)
    {
        ftp_filename = filename;
        ftp_filebuf = filebuf;
        ftp_filebuf_len = buflen;
        ftp_fileread = read;
        ftp_status = PASV;
        ftp_send(svr_pcb, "PASV\r\n");
    }
}

void ftp_init()
{
    svr_pcb = tcp_new();
    IP4_ADDR(&svr_addr, 192, 168, 3, 27);
    tcp_connect(svr_pcb, &svr_addr, 21, &ftp_connected);
    tcp_recv(svr_pcb, &ftp_recv);
}