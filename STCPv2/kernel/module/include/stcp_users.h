#ifndef STCP_USERS_H
#define STCP_USERS_H

struct stcp_sock;

int stcp_users_init(void);
void stcp_users_exit(void);
void stcp_user_register(struct stcp_sock *ssk);
void stcp_user_unregister(struct stcp_sock *ssk);

#endif
