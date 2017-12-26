/*************************************************************************
	> File Name: cs2.c
	> Author: liangmengdi
	> Mail: llmmdd1
    997130@126.com
	> Created Time: 2016年08月08日 星期一 17时40分30秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <unistd.h>

#define BUFSIZE 1024 //缓冲区大小
#define PMMESSAGE 100 //聊天内容区域大小

#define REGISTER 0 //注册
#define LOGIN 1 //登录
#define PMM 2 //私聊
#define EXIT 3//退出
#define ALLCHAT 4//群聊
#define INSERT 5 //添加好友
#define INSERTBOX 6 //消息盒子
#define LOOKFRIEND 7 //查看好友
#define DELETEFRIEND 8 //删除好友
#define CHATRECORD 9 //聊天记录
#define NOTEXIT 10 //离线消息
/*自定义错误函数*/
void myerror(const char *string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(string);
    exit(1);
}

char mainname[40];  //本客户端姓名
struct message
{
    char name[40];
    char password[10];
    char recvname[40];
    char message[100];
    int flag;
    int number;//群聊人数
    char allchat[40][40]; //储存群聊用户姓名
    char insert[40];
    int boxsign;//消息盒子信号
    char getboxname[40];
    char deletename[40];
    char recordname[40];
};

struct mainmessage //接收消息结构体
{
    char chatname[40];
    char rechatname[40];
    char message[PMMESSAGE];
    char sign[10];
    char getrequest[40];
    char a[40][40];
    int n;
    char b[100][100];
    int m;
    int insertn;
    char c[100] [100];
    int l;
     
};

int flag_box = 0;
char getrequest [40] = {0};

/*好友管理结构体*/
struct information
{
    char name[40];
    struct information *next;
};
struct information *bhead;

/*聊天记录结构体*/
struct txt
{
    char message[50];
    struct txt *next;
};
struct txt *chead;


 /*注册函数*/
 void zhuce( int connfd )
 {
     char d[40];
     struct message a;
     memset(&a,0,sizeof(a));
     a. flag=REGISTER; //赋参数
     printf("请输入您的注册信息\n");
     printf("用户名:");
     scanf("%s",a.name);
     printf("\033[;37m密码:\033[8m");
     scanf("%s",a.password);
     printf("\033[0m");
     if(send(connfd,&a,sizeof(a),0)<0) //将注册信息发送至服务器端
     {
         myerror("send",__LINE__);
     }
     memset(&a,0,sizeof(a));//清空缓存
     recv(connfd,&d,sizeof(d),0);
     if(strcmp(d,"o") == 0)
     {
         printf("无法注册\n");
         return;
     }
     if(strcmp(d,"p")==0)
     {
         printf("注册成功\n");
     }
 }
 /*退出函数*/
 void Exit(int connfd)
 {
     struct message d;
     memset(&d,0,sizeof(d));
     d.flag=EXIT;
     strcpy(d.name,mainname);
     if(send(connfd,&d,sizeof(d),0)<0)//将退出参数发送至服务器端
     {
         myerror("exit",__LINE__);
     }
     memset(&d,0,sizeof(d));
 }

 /*私聊函数*/
 void siliao(int connfd)
 {
     int fd;
     char path[40];
     struct message b;
     memset(&b,0,sizeof(b));
     b.flag=PMM;
     strcpy(b.name,mainname);
     printf("请输入要对话的好友：\n");
     scanf("%s",b.recvname);//对方用户姓名
     printf("请输入聊天内容：\n");
     scanf("%s",b.message);//聊天内容
    if(send(connfd,&b,sizeof(b),0) < 0)//发送至服务器端
     {
         myerror("send1",__LINE__);
     }       
 }

/*群聊函数*/
 void qunliao(int connfd)
 {
     int i;
     struct message e;
     memset(&e,0,sizeof(e));
     e.flag=ALLCHAT;
     strcpy(e.name,mainname);
     printf("请选择群聊的人数:\n");
     scanf("%d",&e.number);
     printf("请选择要发送消息的用户：\n");
     for(i=0;i<e.number;i++)
     {
         scanf("%s",e.allchat[i]);
     }
     printf("请输入聊天内容:\n");
     scanf("%s",e.message);
     if(send(connfd,&e,sizeof(e),0) <0)
     {
         myerror("allchatsend",__LINE__);
     }
     memset(&e,0,sizeof(e));
 }

 /*添加好友函数*/
 void insert(int connfd)
 {
     struct message f;
     memset(&f,0,sizeof(f));
     f.flag=INSERT;
     strcpy(f.name,mainname);
     printf("请选择您要添加的好友\n");
     scanf("%s",f.insert);
     send(connfd,&f,sizeof(f),0);
 }

/*消息盒子*/
 void box(int connfd)
 {
     if(flag_box == 0)
     {
         printf("没有消息！\n");
         return ;
     }
     struct message h;
     memset(&h,0,sizeof(h));
     h.flag=INSERTBOX;
     int n;
     char s [40];
     unsigned int i;
     unsigned int j;
     unsigned int k;
     i=strlen(getrequest);
     j=strlen("请求添加您为好友");
     k=i-j;
     strncpy(s,getrequest,k);
     s[strlen(s)]='\0';
     printf("%s\n",getrequest);
     printf("1.同意\n");
     printf("2.拒绝\n");
     scanf("%d",&n);
     h.boxsign=n;
     strcpy(h.name,mainname);
     strcpy(h.getboxname,s);
     send(connfd,&h,sizeof(h),0);
     memset(getrequest,0,sizeof(getrequest));
     flag_box = 0;
 }
 

/*查看好友*/
void show(int connfd)
{
    struct message j;   
    memset(&j,0,sizeof(j));
    j.flag=LOOKFRIEND;
    strcpy(j.name,mainname);
    send(connfd,&j,sizeof(j),0);
    usleep(100000);
}

/*删除好友*/
void delete(int connfd)
{
    struct message k;
    memset(&k,0,sizeof(k));
    k.flag=DELETEFRIEND;
    printf("请输入要删除的好友\n");
    scanf("%s",k.deletename);
    strcpy(k.name,mainname);
    send(connfd,&k,sizeof(k),0);
}
 void liaotianjilu(int connfd)
 {
     struct message l;
     memset(&l,0,sizeof(l));
     l.flag=CHATRECORD;
     strcpy(l.name,mainname);
     printf("请输入要查看聊天记录好友：\n");
     scanf("%s",l.recordname);
    send(connfd,&l,sizeof(l),0);
 }

 void lixianxiaoxi(int connfd)
 {
     struct message z;
     memset(&z,0,sizeof(z));
     z.flag=NOTEXIT;
     strcpy(z.name,mainname);
     send(connfd,&z,sizeof(z),0);
 }


 /*登录函数*/
 void denglu(int connfd)
 {
     int fd;
     char path[40];
     char s[40];
     struct message c;
     memset(&c,0,sizeof(c));
     c.flag=LOGIN;//赋参数
     char recve[BUFSIZE];//接受服务器端验证登录信息后反馈的信息
     printf("姓名：");
     scanf("%s",c.name);
     strcpy(mainname,c.name);   //登录后将登录名赋给全局变量mainname
     printf("\033[;37m密码:\033[8m");
     scanf("%s",c.password);
     printf("\033[0m");
     send(connfd,&c,sizeof(c),0);//将登录信息发送至服务器端
     memset(&c,0,sizeof(c));//清空缓存
     recv(connfd,recve,sizeof(recve),0);
     if(strcmp(recve,"x") == 0)
     {
         printf("该用户已经登录，您无法登录\n");
         return;
     }
     

     if(strcmp(recve,"y") == 0)
    {
         void *clientthread(void* arg)
         {
             while(1)
             {
                 int ret;
                 struct mainmessage recvbuf;
                 ret=recv(connfd,&recvbuf,sizeof(recvbuf),0);
                 if(ret == 0){
                     printf("服务器退出\n");
                     exit(0);
                 }
                 if(strcmp(recvbuf.sign,"siliao") == 0)
                 {
                    
                     printf("%s发来消息：%s\n",recvbuf.chatname,recvbuf.message);
                
                 } 
                 if(strcmp(recvbuf.sign,"qunliao") == 0)
                 {
                    
                     printf("%s发来消息：%s\n",recvbuf.chatname,recvbuf.message);
                 }
                 if(strcmp(recvbuf.sign,"insert") == 0)
                 {
                     if(recvbuf.insertn==1)
                     {
                         printf("该用户已存在不可添加\n");
                         continue;
                     }
                     printf("%s\n",recvbuf.chatname);
                     strcpy(getrequest,recvbuf.getrequest);
                     flag_box = 1;
                 }
                 if(strcmp(recvbuf.sign,"look")==0)
                 {
                     int i;
                     printf("您的好友有：\n");
                     for(i=0;i<recvbuf.n;i++)
                     {
                         printf("%s\n",recvbuf.a[i]);
                     }
                 }
                 if(strcmp(recvbuf.sign,"chatrecord")==0)
                 {
                     int j;
                     for(j=0;j<recvbuf.m;j++)
                     {
                         printf("%s\n",recvbuf.b[j]);
                     }
                 }

                 if(strcmp(recvbuf.sign,"lixian") == 0)
                 {
                     int k;
                     for(k=0;k<recvbuf.l;k++)
                     {
                         printf("%s\n",recvbuf.c[k]);
                     }
                 }
             }
         }
         pthread_t pid;
         pthread_create(&pid,0,clientthread,&connfd);
         char choice31[100];
         int choice11;
         do
         {
             printf("                                                                                             \n");
             printf("                                ****************************                                 \n");
             printf("                                       1、好友管理                                           \n");
             printf("                                       2、聊天管理                                           \n");
             printf("                                       3、聊天记录查看                                       \n");
             printf("                                       4、消息盒子                                           \n");
             printf("                                       5、离线消息                                           \n");
             printf("                                ***************************                                  \n");
             printf("                                                                                             \n");
             printf("请输入您的选项：\n");
             scanf("%s",choice31);
             if(strlen(choice31)==1)  choice11=choice31[0]-48;
             else choice11=9;
             switch(choice11)
             {
                 case 1:
                 {
                     char choice41[100];
                     int choice51;
                     do
                     {
                         printf("好友管理-输入0退出\n");
                         printf("1、添加好友\n");
                         printf("2、删除好友\n");
                         printf("3、查看好友\n");
                         scanf("%s",choice41);
                         if(strlen(choice41) == 1) choice51=choice41[0]-48;
                         else
                            choice51=7;
                         switch(choice51)
                         {
                             case 1:
                             insert(connfd);
                             break;
                             case 2:
                             delete(connfd);
                             break;
                             case 3:
                             show(connfd);
                             break;
                             case 0:
                             break;
                         }

                     }while(choice51!=0);
                     break;
                 }
                 case 2:
                 {
                     char choice2[100];
                     int  choice21;
                     do
                     {
                         printf("聊天管理-输入0退出\n");
                         printf("1、私聊\n");
                         printf("2、群聊\n");
                         scanf("%s",choice2);
                         if(strlen(choice2) == 1 )    choice21 = choice2[0] - 48;
                         else 
                            choice21 = 4;
                         switch(choice21)
                         {
                             case 1:
                             siliao(connfd);
                             break;
                             case 2:
                             qunliao(connfd);
                             break;
                         }
                     }while(choice21!=0);
                     break;
                 }
                 case 3:
                 liaotianjilu(connfd);
                 break;
                 case 4:
                 box(connfd);
                 break;
             case 5:
                 lixianxiaoxi(connfd);
                 break;
                 case 0:
                 Exit(connfd);
                 exit(0);

             }
         }while(choice11!=0);
     }

      if(strcmp(recve,"n") == 0)
     {
         printf("登录信息错误\n");
         return;
     }
 }

int main(int argc,char *argv[])
{
    int i;
    int serv_port;
    int serv_address;
    struct sockaddr_in serv_addr;
    int connfd;
    bhead=(struct information*)malloc(sizeof(struct information));
    chead=(struct txt*)malloc(sizeof(struct txt));
    if(argc!=5)
    {
        printf("Usage [-p],[serv_port] [-a] [serv_address]\n");
        exit(1);
    }

    /*初始化服务器端地址结构*/
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;

    /*从命令行获取服务器的端口与地址*/
    for(i=0;i<argc;i++)
    {
        if(strcmp("-p",argv[i]) == 0)
        {
            serv_port = atoi(argv[i+1]);
            if(serv_port<0||serv_port>65535)
            {
                printf("invalid serv_addr.sin_port\n");
                exit(1);
            }
            else{
                serv_addr.sin_port=htons(serv_port);
            }
            continue;
        }
        if(strcmp("-a",argv[i]) == 0)
        {
            if(inet_aton(argv[i+1],&serv_addr.sin_addr) == 0)
               {
                   printf("invalid serve ip adress\n");
                   exit(1); 
               }
            continue;
        }

    }
    /*检查是否少输了某项参数*/
    if(serv_addr.sin_port == 0||serv_addr.sin_addr.s_addr == 0)
    {
        printf("Usage:[-p] [serv_addr.sin_port] [-a] [serv_address]\n");
        exit(1);
    }
    /*创建一个套接字*/
    connfd = socket(AF_INET,SOCK_STREAM,0);
    if(connfd<0)
    {
        myerror("socket",__LINE__);
    }

    if(connect(connfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))<0)
    {
        myerror("connect",__LINE__);
    }
  
    /*登录界面*/
    char choice61[100];
    int choice71;
    do
    {
        printf("                                     *****************************                                 \n");
        printf("                                           欢迎来到聊天室                                          \n");
        printf("                                             1、注册                                               \n");
        printf("                                             2、登录                                               \n");
        printf("                                             3、退出                                               \n");
        printf("                                     ****************************                                  \n");
        printf("                                                                                                   \n");
        printf("请输入选项：");
        scanf("%s",choice61);
        if(strlen(choice61) == 1) choice71=choice61[0]-48;
        else
        choice71=9;
        switch(choice71)
        {
            case 1: // 注册
            {
                zhuce(connfd);
                break;
            }
            case 2: //登录界面
            {
                denglu(connfd);
                break;
            }    
            case 3://退出
            Exit(connfd);
            break;
        }
    }while(choice71!=3);

    return 0;
}
