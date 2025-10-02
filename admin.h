#ifndef ADMIN_H
#define ADMIN_H

extern char *admin_cookie ;
extern int admin_logged ;
extern char *user_cookie;
extern char *jwt_token;

void response_succ_or_fail(char* response);
void login_admin(char *host, int port, char* payload);
void add_user(char *host, int port, char* payload);
char *get_json_body(char *http_response);
void parse_and_print_users(const char *json_string);
void get_users(char * host, int port);
void delete_user(char *host, int port, char *username);
void logout_admin(char *host, int port);



#endif