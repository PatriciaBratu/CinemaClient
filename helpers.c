#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"


#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    char *buffer = NULL;
    size_t buffer_size = 0;
    int header_end = -1;
    int content_length = 0;

    // Citește până la terminatorul de header
    while (1) {
        int bytes = read(sockfd, response, BUFLEN);
        if (bytes < 0) {
            error("ERROR reading response from socket");
        }
        if (bytes == 0) {
            break;
        }

        // Realocă bufferul
        buffer = realloc(buffer, buffer_size + bytes);
        memcpy(buffer + buffer_size, response, bytes);
        buffer_size += bytes;

        // Caută sfârșitul header-ului
        for (size_t i = 0; i + HEADER_TERMINATOR_SIZE <= buffer_size; i++) {
            if (memcmp(buffer + i, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE) == 0) {
                header_end = (int)(i + HEADER_TERMINATOR_SIZE);
                break;
            }
        }

        if (header_end >= 0) {
            // Caută Content-Length (case insensitive)
            for (size_t i = 0; i + CONTENT_LENGTH_SIZE < buffer_size; i++) {
                if (strncasecmp(buffer + i, CONTENT_LENGTH, CONTENT_LENGTH_SIZE) == 0) {
                    content_length = strtol(buffer + i + CONTENT_LENGTH_SIZE, NULL, 10);
                    break;
                }
            }
            break;
        }
    }

    // Continuă citirea până avem tot corpul
    size_t total_size = (size_t)(header_end + content_length);
    while (buffer_size < total_size) {
        int bytes = read(sockfd, response, BUFLEN);
        if (bytes < 0) {
            error("ERROR reading response from socket");
        }
        if (bytes == 0) {
            break;
        }

        buffer = realloc(buffer, buffer_size + bytes);
        memcpy(buffer + buffer_size, response, bytes);
        buffer_size += bytes;
    }

    // Adaugă null terminator
    buffer = realloc(buffer, buffer_size + 1);
    buffer[buffer_size] = '\0';

    return buffer;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}
