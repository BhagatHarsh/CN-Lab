#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 256

int main(int argc, char* argv[]) {

int len,s;
struct hostent* hp;
struct sockaddr_in sin;
char buf[MAX_LINE];
char* host;
unsigned short server_port;
FILE* fp;

if(argc == 3){

host = argv[1];
server_port = atoi(argv[2]);

}

hp = gethostbyname(host);

if(!hp) { printf("err host"); exit(1); }

bzero((char*)&sin, sizeof(sin));
sin.sin_family = AF_INET;
bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
sin.sin_port = htons(server_port);

s = socket(PF_INET, SOCK_STREAM, 0);

if(s < 0) { printf("err socket"); exit(1); }

if(connect(s, (struct sockaddr*)&sin, sizeof(sin))<0) { printf("err connect"); close(s); exit(1); }

strcpy(buf, "start");
buf[MAX_LINE-1] = '\0';
len = strlen(buf) + 1;
printf("sending\n");
send(s, buf, len, 0);
printf("send %s\n",buf);

bzero(buf, sizeof(buf));
recv(s,buf,sizeof(buf),0);

printf("recieved %s\n",buf);

int n = htonl(2140084);
send(s, &n, sizeof(n), 0);
printf("send: %d\n",n);

n = 0;
recv(s, &n, sizeof(n), 0);
printf("recived: %d\n",ntohl(n));

fp = fopen("sample.txt", "wb");

if(fp == NULL){
    printf("Err file not found!\n");
    close(s);
    exit(1);
}

int bytes_recieved;

while((bytes_recieved = recv(s, buf, sizeof(buf), 0)) > 0){
    fwrite(buf, 1, bytes_recieved, fp);
}


strcpy(buf, "bye");
buf[MAX_LINE-1] = '\0';
len = strlen(buf) + 1;
printf("sending\n");
send(s, buf, len, 0);
printf("send %s\n",buf);

bzero(buf, sizeof(buf));
recv(s,buf,sizeof(buf),0);

printf("recieved %s\n",buf);

close(s);
return 0;
}
