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


void get_movies(char *host, int port) {
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la library (JWT lipsește).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Header pentru autorizare
    char auth_header[LINELEN];
    sprintf(auth_header, "Authorization: Bearer %s", jwt_token);
    char *headers[1] = { auth_header };

    char *message = compute_get_request2(host,
                                         "/api/v1/tema/library/movies",
                                         NULL,
                                         NULL, 0,
                                         headers, 1);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    char *json_body = get_json_body(response);

    if (!json_body) {
        printf("FAIL: Răspuns invalid (nu s-a găsit corp JSON).\n");
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    // Parsează JSON
    JSON_Value *root_value = json_parse_string(json_body);
if (!root_value || json_value_get_type(root_value) != JSONObject) {
    printf("FAIL: Răspuns invalid (nu e un obiect JSON valid).\n");
    if (root_value) json_value_free(root_value);
    free(message);
    free(response);
    close_connection(sockfd);
    return;
}

JSON_Object *root_obj = json_value_get_object(root_value);
JSON_Array *movies = json_object_get_array(root_obj, "movies");
if (!movies) {
    printf("FAIL: Cheia 'movies' lipsește sau nu este un array.\n");
    json_value_free(root_value);
    free(message);
    free(response);
    close_connection(sockfd);
    return;
}

size_t count = json_array_get_count(movies);
printf("SUCCESS: Lista filmelor :\n", count);

for (size_t i = 0; i < count; i++) {
    JSON_Object *movie = json_array_get_object(movies, i);
    const char *title = json_object_get_string(movie, "title");
    int id = (int)json_object_get_number(movie, "id");
    printf("#%zu: %s\n", id, title);

}

json_value_free(root_value);
free(message);
free(response);
close_connection(sockfd);

}



void get_movie(char *host, int port, int id){
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la library (JWT lipsește).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Header personalizat: Authorization: Bearer <jwt_token>
    char *headers[1];
    char auth_header[LINELEN];
    sprintf(auth_header, "Authorization: Bearer %s", jwt_token);
    headers[0] = auth_header;
    char url[LINELEN];
    sprintf(url, "/api/v1/tema/library/movies/%d", id);

    char *message = compute_get_request2(host,
                                        url,
                                        NULL,
                                        NULL, 0, // fără cookie-uri
                                        headers, 1);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    // response_succ_or_fail(response);
    // printf("%s\n", response);

    char *json_start = strstr(response, "{");
    if (json_start) {
        printf("%s\n", json_start);
    } else {
        printf("Eroare: Nu s-a primit un corp de răspuns valid.\n");
    }

    free(message);
    free(response);
    close_connection(sockfd);
}



void add_movie(char *host, int port, char *payload) {
    if (jwt_token == NULL) {
        printf("ERROR: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Construim headerul de autorizare: "Authorization: Bearer <token>"
    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1];
    headers[0] = auth_header;

    // Construim cererea POST
    char *message = compute_post_request2(host,
                                         "/api/v1/tema/library/movies",
                                         "application/json", 
                                         &payload, 1,    // payload JSON
                                         headers, 1);    // headerul cu Bearer JWT

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    response_succ_or_fail(response);
    printf("%s\n", response); // opțional: pentru debugging

    // Cleanup
    free(message);
    free(response);
    close_connection(sockfd);
}
void delete_movie(char *host, int port, int id) {
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return;
    }
    // Deschide conexiunea
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    // char *cookies[1];
    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1];
    headers[0] = auth_header;
    char *url = malloc(100 * sizeof(char));
    // cookies[0] = admin_cookie;
    sprintf(url, "/api/v1/tema/library/movies/%d", id);
    
    // Creează cererea DELETE
    char *message = compute_delete_request2(host,
                                            url,// Folosește username-ul în URL
                                            NULL, NULL, 0,
                                            headers, 
                                            1);  // Folosește cookie-ul admin pentru autentificare
                                            
                                        
    if(message != NULL){                                        

    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    
    
    response_succ_or_fail(response);
    printf("%s\n", response);  
    free(response);

    }
    // Curăță memoria
    free(message);
    close_connection(sockfd);
    // }
}
void update_movie(char *host, int port, char *payload, int id) {
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Construim headerul de autorizare: "Authorization: Bearer <token>"
    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1];
    headers[0] = auth_header;

    char *url = malloc(100 * sizeof(char));
    // cookies[0] = admin_cookie;
    sprintf(url, "/api/v1/tema/library/movies/%d", id);

    // Construim cererea POST
    char *message = compute_put_request2(host,
                                         url,
                                         "application/json", 
                                         &payload, 1,    // payload JSON
                                         headers, 1);    // headerul cu Bearer JWT

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    response_succ_or_fail(response);
    printf("%s\n", response); // opțional: pentru debugging

    // Cleanup
    free(message);
    free(response);
    close_connection(sockfd);
}