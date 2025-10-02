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

char *admin_cookie = NULL;
int admin_logged = 0;
char *user_cookie = NULL;
char *jwt_token = NULL;


void response_succ_or_fail(char* response) {
    char *status_line = strtok(response, "\r\n");
    if (status_line != NULL) {
        int status_code;
        // Extragem codul de status (de forma: HTTP/1.1 201 Created)
        if (sscanf(status_line, "HTTP/%*s %d", &status_code) == 1) {
            if (status_code >= 200 && status_code < 300) {
                printf("SUCCESS: ");
                return;
            } else {
                printf("ERROR: ");
            }
        } else {
            printf("ERROR: ");  // Dacă nu putem extrage codul
        }
    } else {
        printf("ERROR: ");  // Dacă nu există linia de status
    }
}
void login_admin(char *host, int port, char* payload){
    if(admin_logged != 0){
        printf("ERROR: Already logged in!\n");
        return;
    }
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
            char *message = compute_post_request(host,
                                           "/api/v1/tema/admin/login",
                                           "application/json",
                                           &payload, 1,
                                           NULL, 0);
            send_to_server(sockfd, message);
            char * response = receive_from_server(sockfd);
            char *cookie_start = strstr(response, "Set-Cookie: ");
            if (cookie_start != NULL) {
                cookie_start += strlen("Set-Cookie: "); // mută pointerul după "Set-Cookie: "
                char *cookie_end = strstr(cookie_start, "\r\n");
                if (cookie_end != NULL) {
                    int len = cookie_end - cookie_start;
                    admin_cookie = (char *)malloc(len + 1);
                    strncpy(admin_cookie, cookie_start, len);
                    admin_cookie[len] = '\0';
                    admin_logged = 1;
    }
}
            response_succ_or_fail(response);
            printf("%s\n", response); // Afișează răspunsul serverului
            
            free(message);
            free(response);
            close_connection(sockfd);

}

void add_user(char *host, int port, char* payload){
    if (admin_cookie == NULL) {
        printf("ERROR: Nu sunteți autentificat ca admin.\n");
        return;
    }
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    char *cookies[1];
    cookies[0] = admin_cookie;
            char *message = compute_post_request(host,
                                           "/api/v1/tema/admin/users",
                                           "application/json",
                                           &payload, 1,
                                           cookies, 1);
            send_to_server(sockfd, message);
            char * response = receive_from_server(sockfd);
            response_succ_or_fail(response);
            printf("%s\n", response); // Afișează răspunsul serverului
            
            free(message);
            free(response);
            close_connection(sockfd);

}


char *get_json_body(char *http_response) {
    char *body = strstr(http_response, "\r\n\r\n");
    if (body == NULL) return NULL;
    return body + 4; // Sărim peste "\r\n\r\n"
}

void parse_and_print_users(const char *json_string) {
    JSON_Value *root_value = json_parse_string(json_string);
    if (root_value == NULL) {
        fprintf(stderr, "Failed to parse JSON.\n");
        return;
    }

    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Array *users_array = json_object_get_array(root_object, "users");

    if (users_array == NULL) {
        fprintf(stderr, "No 'users' array found in JSON.\n");
        json_value_free(root_value);
        return;
    }

    size_t count = json_array_get_count(users_array);
    for (size_t i = 0; i < count; i++) {
        JSON_Object *user_obj = json_array_get_object(users_array, i);
        const char *username = json_object_get_string(user_obj, "username");
        const char *password = json_object_get_string(user_obj, "password");

        if (username && password) {
            printf("#%zu: %s:%s\n", i+1, username, password);
        }
    }

    json_value_free(root_value);
}


void get_users(char * host, int port){
    if (admin_cookie == NULL) {
        printf("FAIL: Nu sunteți autentificat ca admin.\n");
        return;
    }
    
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    char *cookies[1];
    cookies[0] = admin_cookie;
    char *message = compute_get_request(host, "/api/v1/tema/admin/users", NULL, cookies, 1);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    printf("SUCCES: lista users\n");
    char *json_body = get_json_body(response);
    if (json_body) {
        parse_and_print_users(json_body);
    }
    close_connection(sockfd);
    free(message);
    free(response);
}


void delete_user(char *host, int port, char *username) {
    if (admin_cookie == NULL) {
        printf("ERROR: Nu sunteți autentificat ca admin.\n");
        return;
    }

    // Deschide conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    char *cookies[1];
    cookies[0] = admin_cookie;

    char *url = malloc(100 * sizeof(char));
    sprintf(url, "/api/v1/tema/admin/users/%s", username);

    // Creează cererea DELETE
    char *message = compute_delete_request(host, url, NULL, cookies, 1);

    if (message != NULL) {
        send_to_server(sockfd, message);
        char *response = receive_from_server(sockfd);

        response_succ_or_fail(response);
        printf("%s\n", response);

        // Dacă ștergerea a fost cu succes, curățăm cookie-ul și tokenul
        // if (strstr(response, "HTTP/1.1 200 OK") != NULL) {
            if (user_cookie) {
                free(user_cookie);
                user_cookie = NULL;
            }
            if (jwt_token) {
                free(jwt_token);
                jwt_token = NULL;
            }
        // }

        free(response);
    }

    // Cleanup
    free(url);
    free(message);
    close_connection(sockfd);
}



void logout_admin(char *host, int port) {
    if (admin_cookie == NULL) {
        printf("ERROR: Nu sunteți autentificat ca admin.\n");
        return;
    }

    // Deschide conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Creează vectorul de cookie-uri
    char *cookies[1];
    cookies[0] = admin_cookie;

    // Creează cererea GET pentru logout
    char *message = compute_get_request(host,
                                        "/api/v1/tema/admin/logout",
                                        NULL,  // fără parametri
                                        cookies,
                                        1);    // 1 cookie

    // Trimite cererea la server
    send_to_server(sockfd, message);

    // Primește răspunsul
    char *response = receive_from_server(sockfd);

    // Afișează răspunsul
    response_succ_or_fail(response);
    printf("%s\n", response);

    // Curăță
    free(message);
    free(response);
    close_connection(sockfd);

    // Șterge cookie-ul din memorie
    free(admin_cookie);
    admin_cookie = NULL;
    admin_logged = 0;
}