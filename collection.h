#ifndef COLLECTION_H
#define COLLECTION_H


void get_collections(char *host, int port) ;
void get_collection(char *host, int port, int id);
void add_collection(char *host, int port, char *payload, int *movie_ids, int num_movies);
void delete_collection(char *host, int port, int id) ;
int add_movie_to_collection(char *host, int port, int collection_id, int movie_id) ;
void delete_movie_from_collection(char *host, int port, int collection_id, int movie_id);
#endif