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

void get_collections(char *host, int port) {
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
                                         "/api/v1/tema/library/collections",
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
JSON_Array *coll = json_object_get_array(root_obj, "collections");
if (!coll) {
    printf("FAIL: Cheia 'movies' lipsește sau nu este un array.\n");
    json_value_free(root_value);
    free(message);
    free(response);
    close_connection(sockfd);
    return;
}

size_t count = json_array_get_count(coll);
printf("SUCCESS: Lista colectiilor :\n", count);

for (size_t i = 0; i < count; i++) {
    JSON_Object *movie = json_array_get_object(coll, i);
    const char *title = json_object_get_string(movie, "title");
    int id = (int)json_object_get_number(movie, "id");
    printf("#%zu: %s \n", id, title);

}

json_value_free(root_value);
free(message);
free(response);
close_connection(sockfd);

}

void get_collection(char *host, int port, int id){
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la library (JWT lipsește).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    char *headers[1];
    char auth_header[LINELEN];
    sprintf(auth_header, "Authorization: Bearer %s", jwt_token);
    headers[0] = auth_header;

    char url[LINELEN];
    sprintf(url, "/api/v1/tema/library/collections/%d", id);

    char *message = compute_get_request2(host, url, NULL, NULL, 0, headers, 1);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (!response) {
        printf("FAIL: Nu s-a primit niciun răspuns de la server.\n");
        free(message);
        close_connection(sockfd);
        return;
    }

    if (strstr(response, "HTTP/1.1 200 OK") == NULL) {
        printf("FAIL: %.*s\n", (int)(strchr(response, '\n') - response), response);
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    char *json_start = strstr(response, "{");
    if (!json_start) {
        printf("Eroare: Nu s-a primit un corp de răspuns valid.\n");
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    // Parsează JSON-ul
    JSON_Value *root_val = json_parse_string(json_start);
    if (!root_val || json_value_get_type(root_val) != JSONObject) {
        printf("Eroare: JSON invalid sau răspuns greșit.\n");
        if (root_val) json_value_free(root_val);
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    JSON_Object *root_obj = json_value_get_object(root_val);
    const char *title = json_object_get_string(root_obj, "title");
    const char *owner = json_object_get_string(root_obj, "owner");

    printf("SUCCESS: Detalii colecție\n");
    printf("title: %s\n", title ? title : "N/A");
    printf("owner: %s\n", owner ? owner : "N/A");

    JSON_Array *movies = json_object_get_array(root_obj, "movies");
    if (movies) {
        for (size_t i = 0; i < json_array_get_count(movies); i++) {
            JSON_Object *movie = json_array_get_object(movies, i);
            int movie_id = (int)json_object_get_number(movie, "id");
            const char *movie_title = json_object_get_string(movie, "title");
            if (movie_title) {
                printf("#%d: %s\n", movie_id, movie_title);
            }
        }
    }

    json_value_free(root_val);
    free(message);
    free(response);
    close_connection(sockfd);
}



void add_collection(char *host, int port, char *payload, int *movie_ids, int num_movies) {
    if (jwt_token == NULL) {
        printf("ERROR: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Headerul Authorization
    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1] = { auth_header };

    // Construim cererea POST
    char *message = compute_post_request2(
        host,
        "/api/v1/tema/library/collections",
        "application/json",
        &payload, 1,
        headers, 1
    );

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (!response) {
        printf("ERROR: Nu s-a primit răspuns de la server.\n");
        free(message);
        close_connection(sockfd);
        return;
    }

    // Verificare status HTTP
    if (strstr(response, "HTTP/1.1 201") == NULL) {
        printf("ERROR: %.*s\n", (int)(strchr(response, '\n') - response), response);
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    // Caută începutul JSON-ului
    char *json_start = strstr(response, "{");
    if (!json_start) {
        printf("ERROR: Răspuns invalid de la server (fără JSON).\n");
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    JSON_Value *root_val = json_parse_string(json_start);
    JSON_Object *root_obj = json_value_get_object(root_val);
    int collection_id = (int)json_object_get_number(root_obj, "id");

    if (collection_id == 0) {
        printf("ERROR: Nu s-a putut extrage ID-ul colecției.\n");
        json_value_free(root_val);
        free(message);
        free(response);
        close_connection(sockfd);
        return;
    }

    printf("SUCCESS: Colecție adăugată\n");

    // Adaugă fiecare film în colecție
    for (int i = 0; i < num_movies; i++) {
        // add_movie_to_collection(host, port, collection_id, movie_ids[i]);
        int result = add_movie_to_collection(host, port, collection_id, movie_ids[i]);
    if (result == -1) {
        printf("ERROR: Adăugarea colecției eșuată. Șterg colecția...\n");
        // Poți chiar trimite DELETE pentru colecție
        return;
    }
    }
    // Cleanup
    json_value_free(root_val);
    free(message);
    free(response);
    close_connection(sockfd);

}


void delete_collection(char *host, int port, int id) {
    if (jwt_token == NULL) {
        printf("ERROR: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
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
    sprintf(url, "/api/v1/tema/library/collections/%d", id);
    
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

int add_movie_to_collection(char *host, int port, int collection_id, int movie_id) {
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return -1;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1] = { auth_header };

    char url[100];
    snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%d/movies", collection_id);

    char payload[100];
    snprintf(payload, sizeof(payload), "{\"id\":%d}", movie_id);
    char *body_data[1] = { payload };

    char *message = compute_post_request2(host, url, "application/json", body_data, 1, headers, 1);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    if (!response || strstr(response, "error") || strstr(response, "404 Not Found")) {
        printf("FAIL: Nu s-a putut adăuga filmul %d la colecție.\n", movie_id);
        free(message);
        if (response) free(response);
        close_connection(sockfd);
        return -1;
    }

    response_succ_or_fail(response);

    free(message);
    free(response);
    close_connection(sockfd);
    return 0;
}



void delete_movie_from_collection(char *host, int port, int collection_id, int movie_id) {
    if (jwt_token == NULL) {
        printf("FAIL: Nu aveți acces la bibliotecă (JWT token lipsă).\n");
        return;
    }

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Construim Authorization: Bearer <token>
    char auth_header[LINELEN];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", jwt_token);
    char *headers[1] = { auth_header };

    // Construim URL-ul dinamic
    char url[100];
    snprintf(url, sizeof(url),
             "/api/v1/tema/library/collections/%d/movies/%d", collection_id, movie_id);

    // Construim payload-ul JSON
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"id\":%d}", movie_id);
    char *body_data[1] = { payload };

    // Construim mesajul POST
    char *message = compute_delete_request2(host,
                                          url,
                                          "application/json",
                                          body_data, 1,
                                          headers, 1);

    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    response_succ_or_fail(response); // afisează SUCCES / FAIL
    printf("%s\n", response);        // opțional: pentru debugging

    // Cleanup
    free(message);
    free(response);
    close_connection(sockfd);
}