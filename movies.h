#ifndef MOVIES_H
#define MOVIES_H


void get_movies(char *host, int port);
void get_movie(char *host, int port, int id);
void add_movie(char *host, int port, char *payload);
void delete_movie(char *host, int port, int id);
void update_movie(char *host, int port, char *payload, int id);


#endif