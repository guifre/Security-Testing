#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

int create_tcp_socket();
char *build_get_query(char *host);
void *senda(void *hos);

/* proxy validator
usage $ gcc fastChecker.c -o fastChecker -lpthread; ./fastChecker proxy.list
*/
int main(int argc, char **argv) {
	char * target;
	pthread_t thread[1000];
	int  iret1, iret2i,i;

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen(argv[1], "r");
	if (fp == NULL)
	printf("could read proxies");

	while ((read = getline(&line, &len, fp)) != -1) {
		target = (char *) malloc (1000);
		strcpy(target, line);
		pthread_create( &thread[i], NULL, *senda, (void *) target);
		i++;
	}

	int j;
	for (j=0;j<i;j++) {
		pthread_join(thread[j], NULL);
	}

	if (line)
	{
		free(line);
	}
	return 0;
}

void *senda(void *hos)
{
	char *host;
	char *port;
	host= strtok((char *)hos, ":"); //cast from void to char pointer and first occuence
	port = strtok(NULL, ":");
	port = strtok(port, "\n");

	if (port == NULL || host == NULL || strlen(port) < 2 || strlen(host) <4)
	{
    		return;
	}
	// printf("checking host[%s] port[%s]\n", host, port);
	struct sockaddr_in *remote;
	int sock;
	int tmpres;
	char *get;
	char buf[BUFSIZ+1];

	sock = create_tcp_socket();
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton(AF_INET, host, (void *)(&(remote->sin_addr.s_addr)));
	if( tmpres < 0)
	{
  		//perror("Can't set remote->sin_addr.s_addr");
	}
	else if (tmpres == -1)
	{
  		//fprintf(stderr, "%s is not a valid IP address\n", host);
	}
	remote->sin_port = htons(atoi(port));
	if (connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
	{
		//perror("Could not connect"); 
		return;
	}
	get = build_get_query(host);

	//Send the query to the server
	int sent = 0;
	while(sent < strlen(get))
	{
		tmpres = send(sock, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1)
		{
			 perror("Can't send query");
		}
	 	sent += tmpres;
	}
	//now it is time to receive the page
	memset(buf, 0, sizeof(buf));
	int htmlstart = 0;
	char *htmlcontent;
	while ((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0)
	{
		if (htmlstart == 0)
  		{
			htmlcontent = strstr(buf, "\r\n\r\n");
			if (htmlcontent != NULL)
			{
				htmlstart = 1;
				htmlcontent += 4;
			}
		}
		else
		{
  			htmlcontent = buf;
		}
		if (htmlstart)
  		{
			// fprintf(stdout, htmlcontent);
			if (strstr(htmlcontent, "Google is") != NULL)
			{
				printf("%s:%s is valid\n", host, port);
			}
  		}
		memset(buf, 0, tmpres);
	}
	if (tmpres < 0)
	{
	  perror("Error receiving data");
	}
    if (strstr(htmlcontent, "Google is built by a large team of engineers") != NULL)
    {
        printf("%s is valid\n", host);
    }
	free(get);
	free(remote);
	close(sock);
}


int create_tcp_socket()
{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Can't create TCP socket");
	}

	return sock;
}


char *build_get_query(char *host)
{
	char *query;
	char *req;
	req = (char *) malloc ( 1024 + 1 );
	strcpy(req, "http://www.google.com/humans.txt");
	char *tpl = "GET http://www.google.com/humans.txt HTTP/1.1\r\nAccept: */*\r\nMozilla/5.0 (Macintosh; Intel Mac OS X 10_8_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.115 Safari/537.36\r\n\r\n";
	query = (char *)malloc(strlen(host)+strlen(req)+strlen(tpl)-5);
	sprintf(query, tpl,  host);

	return query;
}

