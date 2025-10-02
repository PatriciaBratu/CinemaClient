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
#include "user.h"
#include "movies.h"
#include "collection.h"






int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char *host = "63.32.125.183";
    int port = 8081;

    char *command = (char*) malloc(256 * sizeof(char));
    int ok =1;
    while(ok){
        scanf(" %[^\n]", command);

        if(strcmp(command, "exit") == 0) {
            ok = 0;
        } else if (strcmp(command, "login_admin") == 0) {
            char *username = (char*)malloc(100 * sizeof(char));
            char *password = (char*)malloc(100 * sizeof(char));
            printf("username=");
            scanf(" %[^\n]", username);
            printf("password=");
            scanf(" %[^\n]", password);
            char payload[256];
            snprintf(payload, sizeof(payload),
                     "{\"username\":\"%s\", \"password\":\"%s\"}",
                     username, password);       
            login_admin(host, port, payload);
            
        } else if(strcmp(command, "add_user") == 0) {
           
            char *username = (char*)malloc(100 * sizeof(char));
            char *password = (char*)malloc(100 * sizeof(char));
            printf("username=");
            scanf(" %[^\n]", username);
            printf("password=");
            scanf(" %[^\n]", password);
            char payload[256];
            snprintf(payload, sizeof(payload),
                     "{\"username\":\"%s\", \"password\":\"%s\"}",
                     username, password);       
            add_user(host, port, payload);
        }else if(strcmp(command, "get_users") == 0) {      
            get_users(host, port);
        }else if(strcmp(command, "delete_user") == 0) {
            char *username = (char*)malloc(100 * sizeof(char));
            printf("username=");
            scanf(" %[^\n]", username);
            delete_user(host, port, username);
        }else if(strcmp(command, "logout_admin") == 0) {
            
            logout_admin(host, port);
        } else if (strcmp(command, "login") == 0) {
            char *username = (char*)malloc(100 * sizeof(char));
            char *password = (char*)malloc(100 * sizeof(char));
            char *admin = (char *)malloc(100*sizeof(char));
            printf("admin_username=");
            scanf(" %[^\n]", admin);
            printf("username=");
            scanf(" %[^\n]", username);
            printf("password=");
            scanf(" %[^\n]", password);
            char payload[256];
            snprintf(payload, sizeof(payload),
                     "{\"admin_username\":\"%s\",\"username\":\"%s\", \"password\":\"%s\"}",
                     admin, username, password);       
            login(host, port, payload);
            
        }else if(strcmp(command, "get_access") == 0) {
            
            get_access(host, port);
        }else if(strcmp(command, "get_movies") == 0) {
            
            get_movies(host, port);
        }else if(strcmp(command, "get_movie") == 0) {
            int id;
            printf("id=");
            scanf(" %d",&id);
            get_movie(host, port, id);
        }else if(strcmp(command, "add_movie") == 0) {
            char *title = (char*)malloc(100 * sizeof(char));
            char *description = (char*)malloc(100 * sizeof(char));
            int year;
            float rating;
            printf("title=");
            scanf(" %[^\n]", title);
            printf("year=");
            scanf(" %d", &year);
            printf("description=");
            scanf(" %[^\n]", description);
            printf("rating=");
            scanf(" %f", &rating);
            char payload[256];
            snprintf(payload, sizeof(payload),
         "{\"title\":\"%s\",\"year\":%d,\"description\":\"%s\",\"rating\":%.1f}",
         title, year, description, rating);            
            add_movie(host, port, payload);
        }else if(strcmp(command, "delete_movie") == 0) {
            int id;
            printf("id=");
            scanf(" %d",&id);
            delete_movie(host, port, id);
        }else if(strcmp(command, "update_movie") == 0) {
            char *title = (char*)malloc(100 * sizeof(char));
            char *description = (char*)malloc(100 * sizeof(char));
            int year;
            float rating;
            int id;
            printf("id=");
            scanf(" %d", &id);
            printf("title=");
            scanf(" %[^\n]", title);
            printf("year=");
            scanf(" %d", &year);
            printf("description=");
            scanf(" %[^\n]", description);
            printf("rating=");
            scanf(" %f", &rating);
            char payload[256];
            snprintf(payload, sizeof(payload),
         "{\"title\":\"%s\",\"year\":%d,\"description\":\"%s\",\"rating\":%.1f}",
         title, year, description, rating);            
            update_movie(host, port, payload, id);
        }else if(strcmp(command, "get_collections") == 0) {
            
            get_collections(host, port);
        }else if(strcmp(command, "get_collection") == 0) {
            int id;
            printf("id=");
            scanf(" %d",&id);
            get_collection(host, port, id);
        }else if (strcmp(command, "add_collection") == 0) {
            
        
            char title[100];
int num_movies;
int movie_ids[20];  // presupunem max 20 filme

printf("title=");
scanf(" %[^\n]", title);
printf("num_movies=");
scanf("%d", &num_movies);

for (int i = 0; i < num_movies; i++) {
    printf("movie_id[%d]=", i);
    scanf("%d", &movie_ids[i]);
}

char payload[256];
snprintf(payload, sizeof(payload), "{\"title\":\"%s\"}", title);

// Apelează funcția
add_collection(host, port, payload, movie_ids, num_movies);

        }else if(strcmp(command, "delete_collection") == 0) {
            int id;
            printf("id=");
            scanf(" %d",&id);
            delete_collection(host, port, id);
        }else if(strcmp(command, "add_movie_to_collection") == 0) {
            int collection_id;
            int movie_id;
            printf("collection_id=");
            scanf(" %d",&collection_id);
            printf("movie_id=");
            scanf("%d", &movie_id);
            add_movie_to_collection(host, port, collection_id, movie_id);
        }else if(strcmp(command, "delete_movie_from_collection") == 0) {
            int collection_id;
            int movie_id;
            printf("collection_id=");
            scanf(" %d",&collection_id);
            printf("movie_id=");
            scanf("%d", &movie_id);
            delete_movie_from_collection(host, port, collection_id, movie_id);
        }else if(strcmp(command, "logout") == 0){
            logout_user(host, port);
        }else if(strcmp(command, "exit") == 0){
            exit(0);
        }else {
            printf("Unknown\n");
        }
        
    }
    

    return 0;
}


