#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host);
void *senda(void *hos);

/* proxy validator
usage $ gcc fastChecker.c -o fastChecker; ./fastChecker proxy.list
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
		printf("something null, quitting");
    	return -1;
	}
	printf("host[%s] port[%s]\n", host, port);
	struct sockaddr_in *remote;
	int sock;
	int tmpres;
	char *ip;
	char *get;
	char buf[BUFSIZ+1];

	sock = create_tcp_socket();
	//ip = get_ip(host);
	//fprintf(stderr, "IP is %s\n", ip);
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton(AF_INET, host, (void *)(&(remote->sin_addr.s_addr)));
	if( tmpres < 0)
	{
  		perror("Can't set remote->sin_addr.s_addr");
		exit(1);
	}
	else if(tmpres == -1)
	{
  		fprintf(stderr, "%s is not a valid IP address\n", ip);
	}
	remote->sin_port = htons(atoi(port));

	if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
	{
		perror("Could not connect");
	}
	get = build_get_query(host);
	fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

	//Send the query to the server
	int sent = 0;
	while(sent < strlen(get))
	{
		tmpres = send(sock, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1)
		{
			// perror("Can't send query");
		}
	 	sent += tmpres;
	}
	//now it is time to receive the page
	memset(buf, 0, sizeof(buf));
	int htmlstart = 0;
	char * htmlcontent;
	while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0)
	{
		if(htmlstart == 0)
  		{
			htmlcontent = strstr(buf, "\r\n\r\n");
			if(htmlcontent != NULL)
			{
				htmlstart = 1;
				htmlcontent += 4;
			}
		}
		else
		{
  			htmlcontent = buf;
		}
		if(htmlstart)
  		{
			fprintf(stdout, htmlcontent);
  		}
		printf("got %s", htmlcontent);
		memset(buf, 0, tmpres);
	}
	if(tmpres < 0)
	{
	  perror("Error receiving data");
	}
	free(get);
	free(remote);
	free(ip);
	close(sock);
	printf("exiting");
}


int create_tcp_socket()
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Can't create TCP socket");
	}

	return sock;
}


char *get_ip(char *host)
{
	struct hostent *hent;
	int iplen = 15; //XXX.XXX.XXX.XXX
	char *ip = (char *)malloc(iplen+1);
	memset(ip, 0, iplen+1);
	if((hent = gethostbyname(host)) == NULL)
	{
  		herror("Can't get IP");
		exit(1);
	}
	if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
	{
  		perror("Can't resolve host");
 		exit(1);
	}

	return ip;
}

char *build_get_query(char *host)
{
	char *query;
	char *req;
	req = (char *) malloc ( 1024 + 1 );
	strcpy(req, "http://www.google.com/humans.txt");
	char *tpl = "GET http://www.google.com/humans.txt HTTP/1.0\r\n\r\n";
	query = (char *)malloc(strlen(host)+strlen(req)+strlen(tpl)-5);
	sprintf(query, tpl,  host);

	return query;
}

