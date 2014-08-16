#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <syslog.h>

#define LOCALPORT 3000
#define BUFFER_SIZE 512
#define PING_TIME 30
//#define DEBUG
static const char *cmd[] = {
    "wdctl",
    "remote.sh",
    "uptime",
    "get_wifidog_conf.sh",
    "free",
    "autoupgrade.sh",
    "whitelist.sh",
    "do_remote_sh.sh",
    "NULL"
};
struct send_pak {
    char gwid[32];
    char code[6];
    char *msg;

};
struct recv_pak {
    char cmd[100];
    char callback_port[5];
};
char gwid[50];
char send_buff[BUFFER_SIZE];
struct send_pak *send_pak = (struct send_pak *) send_buff;
char *do_cmd(const char *command)
{
    static char newcmd[200];
    static char buffer[1024];
    memset(newcmd, 0, 200);
    memset(buffer, 0, 1024);
    strncpy(newcmd, command, 190);
    strcat(newcmd," 2>&1");
	syslog(LOG_INFO, "docmd=%s",newcmd);
    FILE *pp = popen(newcmd,"r");
    if(!pp)
    {
        perror("popen Error\n");
        return NULL;
    }
    int count = fread(buffer, 1, 1024, pp);
    buffer[count]=0;
    pclose(pp);
    return buffer;
}
void init()
{
    openlog("udphd", LOG_PID|LOG_CONS, LOG_USER);
    char *temp = NULL; 
 #ifdef DEBUG
    char x[32]="12345678901234567890123456789012";
    temp = x;
#else
	temp = do_cmd(". /lib/ralink.sh ;ralink_get_yunwifi_str |tr '\\n' '\\0'");
    while(strlen(temp) != 32)
    {
		syslog(LOG_ERR,"can't get gwid try 5s later.\n");
        sleep(5);
		temp = do_cmd(". /lib/ralink.sh ;ralink_get_yunwifi_str |tr '\\n' '\\0'");
    }
#endif
    strcpy(send_pak->gwid, temp);
    strcpy(send_pak->code,"  ping");
    syslog(LOG_INFO, "Gwid=%s",send_pak->gwid);
}
void recv_process(int sock)
{
    struct sockaddr_in addr;
    static char buff[BUFFER_SIZE];
    int len = sizeof(addr);
    int recv_len;
    do
    {
        memset(buff, 0, BUFFER_SIZE);
        recv_len = recvfrom(sock, buff, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &len);
//        printf("recv return !\n");
        if(recv_len > 0)
        {
            buff[recv_len] = '\0';
            struct recv_pak * p = (struct recv_pak *)buff;
            if(recv_len > 100)
            {
                int i=0;
                for(i=0;cmd[i];++i)
                {
                    if(strncmp(p->cmd, cmd[i], strlen(cmd[i])) == 0)
                    {
                        syslog(LOG_INFO,"%s loop\n",cmd[i]);
                        break;
                    }
                }
                addr.sin_port = htons(atoi(p->callback_port));
                syslog(LOG_INFO,"Recv from %s %d length:%d:%s--%s\n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),recv_len,p->cmd,p->callback_port);
                if(cmd[i])
                {
                    char *result_p = do_cmd(p->cmd);
//                    puts(result_p);
                    sendto(sock, result_p, strlen(result_p), 0, (struct sockaddr *)&addr, sizeof(addr));
                }
                else
                {
                    syslog(LOG_ERR,"Unsupported Command!\n");
                }
            }
            else 
            {
                syslog(LOG_INFO,"Recv from %s %d length:%d:%s\n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),recv_len,p->cmd);
            }
                

        }
    }while(recv_len>0);
    perror("sock has closed");
    close(sock);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s ip port\n", argv[0]);
        exit(1);
    }
    printf("Hd udp client\n");
    init();
    struct sockaddr_in addr;
    int sock;

    if ( (sock=socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3000);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        perror("bind error");
        exit(1);
    }

    pid_t recv_pid = fork();
    if(recv_pid == 0)
    {
        printf("son process\n");
        recv_process(sock);
    }
    else
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(atoi(argv[2]));
        struct hostent *hostinfo;
 //       char buff[512]="{'gwid':'1234567890'}";
        while (1)
        {
 //           gets(buff);
            hostinfo = gethostbyname(argv[1]);
            while(hostinfo == NULL)
            {            
                sleep(5);
                hostinfo = gethostbyname(argv[1]);
            }
            addr.sin_addr.s_addr = *(unsigned int *)hostinfo->h_addr_list[0];
            int n;
            n = sendto(sock, send_buff, 512, 0, (struct sockaddr *)&addr, sizeof(addr));
            if (n < 0)
            {
                perror("sendto");
                close(sock);
                break;
            }
            sleep(PING_TIME);
        }
    }
    close(sock);
    return 0;
}
