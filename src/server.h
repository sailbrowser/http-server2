void server(int port, char *ip_address, int *sv, int cores);
void *thread_worker(void *arg);
void demonize(char *dir);
int get_num_cores();
