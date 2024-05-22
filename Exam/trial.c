#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 256

int main(int argc, char* argv[]){

int len, s, server_port;
char buf[MAX_LINE];
struct hostent* hp;
struct sockaddr_in sin;
char *host;

if(argc == 3){

server_port = atoi(argv[2]);
host = argv[1];

}

hp = gethostbyname(host);
if(!hp) printf("err host");

bzero((char *)&sin,sizeof(sin));
sin.sin_family = AF_INET;
bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
sin.sin_port = htons(server_port);

s = socket(PF_INET, SOCK_STREAM, 0);
if(s<0) printf("err socket");

if(connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) printf("err connect");

printf("listening\n");

strcpy(buf, "start");
buf[MAX_LINE - 1] = '\0';
len = strlen(buf) + 1;
send(s, buf, len, 0);
printf("Send: %s\n", buf);

bzero(buf, sizeof(buf));
recv(s, buf, sizeof(buf), 0);
printf("Recieved: %s\n", buf);

return 0;
}