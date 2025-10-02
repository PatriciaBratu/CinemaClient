#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "admin.h"



void login(char *host, int port, char* payload){
    if (user_cookie != NULL) {
        printf("ERROR: Sunteți deja autentificat, trebuie sa va delogati.\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    char *cookies[1];
    cookies[0] = admin_cookie;
            char *message = compute_post_request(host,
                                           "/api/v1/tema/user/login",
                                           "application/json",
                                           &payload, 1,
                                           NULL, 0);
            send_to_server(sockfd, message);
            char * response = receive_from_server(sockfd);
            char *cookie_start = strstr(response, "Set-Cookie: ");
            if (cookie_start != NULL) {
                cookie_start += strlen("Set-Cookie: ");
                char *cookie_end = strstr(cookie_start, "\r\n");
                if (cookie_end != NULL) {
                    int len = cookie_end - cookie_start;
                    user_cookie = (char *)malloc(len + 1);
                    strncpy(user_cookie, cookie_start, len);
                    user_cookie[len] = '\0';
                }
            }
            response_succ_or_fail(response);
            printf("%s\n", response);
            free(message);
            free(response);
            close_connection(sockfd);

}


void get_access(char *host, int port) {
    if (user_cookie == NULL) {
        printf("ERROR: Nu sunteți autentificat ca user.\n");
        return;
    }

    // Deschide conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Creează vectorul de cookie-uri
    char *cookies[1];
    cookies[0] = user_cookie;

    // Creează cererea GET pentru logout
    char *message = compute_get_request(host,
                                        "/api/v1/tema/library/access",
                                        NULL,  // fără parametri
                                        cookies,
                                        1);    // 1 cookie

    // Trimite cererea la server
    send_to_server(sockfd, message);

    // Primește răspunsul
    char *response = receive_from_server(sockfd);
    char *token_start = strstr(response, "{\"token\":\"");
    if (token_start != NULL) {
        token_start += strlen("{\"token\":\"");
        char *token_end = strchr(token_start, '"');
        if (token_end != NULL) {
            int len = token_end - token_start;
            jwt_token = malloc(len + 1);
            strncpy(jwt_token, token_start, len);
            jwt_token[len] = '\0';
        }
    }

    // Afișează răspunsul
    response_succ_or_fail(response);
    printf("%s\n", response);

    // Curăță
    free(message);
    free(response);
    close_connection(sockfd);

   
}

void logout_user(char *host, int port) {
    if (user_cookie == NULL) {
        printf("ERROR: Nu sunteți autentificat.\n");
        return;
    }

    // Deschide conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Creează vectorul de cookie-uri
    char *cookies[1];
    cookies[0] = user_cookie;

    // Creează cererea GET pentru logout
    char *message = compute_get_request(host,
                                        "/api/v1/tema/user/logout", 
                                        NULL,  // fără parametri
                                        cookies,
                                        1);    // 1 cookie

    // Trimite cererea la server
    send_to_server(sockfd, message);

    // Primește răspunsul
    char *response = receive_from_server(sockfd);
    // printf("%s\n", response);
    // Afișează răspunsul
    response_succ_or_fail(response);
    printf("%s\n", response);  // opțional: pentru debugging

    // Curăță
    free(message);
    free(response);
    close_connection(sockfd);

    // Șterge cookie-ul și tokenul
    free(user_cookie);
    user_cookie = NULL;

    if (jwt_token) {
        free(jwt_token);
        jwt_token = NULL;
    }
}
