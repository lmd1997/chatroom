/*************************************************************************
> File Name: cs1.c
> Author: liangmengdi
> Mail: llmmdd1997130@126.com
> Created Time: 2016年08月08日 星期一 16时59分46秒
************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<signal.h>
#include<sys/msg.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<time.h>


#define SERV_PORT 4507
#define LISTENQ 12
#define BUFSIZE 1024
#define PMMESSAGE 100

#define REGISTER 0 //注册
#define LOGIN 1 //登录
#define PMM 2 //私聊
#define EXIT 3//退出
#define ALLCHAT 4//群聊
#define INSERT 5//添加好友
#define INSERTBOX 6 //消息盒子
#define LOOKFRIEND 7 //查看好友
#define DELETEFRIEND 8 //删除好友
#define CHATRECORD 9 //聊天记录
#define NOTEXIT 10 //离线消息


/*自定义错误函数*/
void myerror(const char*string,int line)
{
    fprintf(stderr,"line:%d ",line);
    perror(string);
    exit(1);
}

 char mn[50]={0};///////////////////////////////////////////////////
/*此结构体保存了用户的注册信息*/
struct usermessage                              
{
    char usrname[40];                             //保存注册用户姓名   
    char password[10];                            //保存注册用户密码
    struct usermessage *next;                   
};
struct usermessage *phead; //链表头结点


/*此结构体保存了客户端传来的信息*/
struct message
{
    char idname[40]; //用户端姓名  
    char password[10];//密码
    char recvename[40]; //目标用户姓名
    char message[PMMESSAGE];
    int flag;//参数
    int number; // 群聊人数
    char allchat[40][40];
    char insert[40];
    int boxsign;
    char getboxname[40];
    char deletename[40];
    char recordname[40];
};

struct information
{
    char name[40];
    struct information *next;
};
struct information *ehead;

struct online //在线结构体
{
    char name[40]; //姓名
    int fd; //套接字
    struct online *next;
};
struct online *ahead;//在线链表头指针

struct mainmessage //发送至客户端结构体
{
    char chatname[40]; //保存了发消息者姓名
    char rechatname[40];//保存了接收者姓名
    char message[PMMESSAGE]; //保存了聊天内容
    char sign[10];
    char getrequest[40];
    char a[40][40]; // 将文件中的姓名存入链表
    int n;//标志文件中有多少个人
    char b[100][100];
    int m;
    int insertn;
    char c[100][100];
    int l;
};


/*读取文件中的注册信息*/
void readzc()
{
    phead=(struct usermessage*)malloc(sizeof(struct usermessage));
    struct usermessage *a;
    struct usermessage *b;
    a=phead;
    FILE *fp;
    if((fp = fopen("zc.txt","rt")) == NULL)
    {
        fp=fopen("zc.txt","a+");
    }
    else{
        while(!feof(fp))
        {
            b=(struct usermessage*)malloc(sizeof(struct usermessage));
            fscanf(fp,"%s %s \n",b->usrname,b->password);
            a->next=b;
            a=b;
        }
        a->next = NULL;
    }
    fclose(fp);
}

void writezc()
{
    FILE *fp;
    struct usermessage *q;
    q=phead;
    if((fp=fopen("zc.txt","w")) == NULL)
    {
        printf("文件无法打开\n");
    }
    for(q=phead->next;q;q=q->next)
    {
        fprintf(fp,"%s %s \n",q->usrname,q->password);
    }
    fclose(fp);
}


void read_file(char *x)
 {

     int fd;
     char path[40];
     char s[40];
     struct information *a;
     struct information *b;
     struct information *p;
    
     a=ehead;
     getcwd(path,sizeof(path));
     strcat(path,"/");
     strcat(path,x);
     strcat(path,"/list.txt");
     fd=open(path,O_RDWR);
     while(read(fd,s,sizeof(s))>0)
     {
         b=(struct information*)malloc(sizeof(struct information));
         s[strlen(s)]='\0';
         strcpy(b->name,s);
         while(a->next!=NULL)
         {
             a=a->next;
         }
         a->next=b;
         b->next=NULL;
     }
     close(fd);
 }



void write_file(char *x)
{
    char z[40];
    char path[40];
    int fd;
    struct information * a = ehead->next;
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,x);
    strcat(path,"/list.txt");
    fd=open(path,O_RDWR|O_TRUNC);
    a=ehead->next;
    while(a)
    {
        strcpy(z,a->name);
        write(fd,z,sizeof(z));
        memset(&z,0,sizeof(z));
        a=a->next;
    }
    close(fd);
}

void destroy_linklist()
{
    struct information *a;
    struct information *t;
    a = ehead->next;
    if(a == NULL)  return ;
    while(a->next!=NULL)
    {
        t = a;
        a = a->next;
        free(t);
    }
    free(a);
    ehead->next= NULL;
    
}
 
 void delete(struct message recvbuf)
 {   char path[40];
     int fd;
     char t[40];
     char x[40];
     strcpy(x,recvbuf.idname);
     struct information *p;
     struct information *q;
     char m[40];
     strcpy(m,recvbuf.deletename);
     read_file(x);
     p=ehead->next;
     q=ehead;

     strcpy(t,recvbuf.idname);
     strcat(t,"删除了好友");
     strcat(t,recvbuf.deletename);

     strcpy(mn,t);//////////////////////////////////////////
      getcwd(path,sizeof(path));
     strcat(path,"/");
     strcat(path,"rizhi.txt");
     fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
     write(fd,mn,sizeof(mn));
     memset(mn,0,sizeof(mn));///////////////////////////
     memset(t,0,sizeof(t));/////////////////////////////////

     while(p!=NULL)
     {
         if(strcmp(p->name,m) == 0)
         {
             q->next=p->next;
             free(p);
             break;        
         }
         q=p;
         p=p->next;
     }
     write_file(x);
     destroy_linklist();
 }

/*用户注册函数*/
void zhuce(int connfd,struct message recvbuf)
{  readzc();
 struct usermessage *z;
 struct mainmessage x;
 z=phead->next;
 while(z!=NULL)
 {
     if(strcmp(z->usrname,recvbuf.idname) == 0)
     {
         send(connfd,"o",sizeof("o"),0);
         return;
     }
     z=z->next;
 }
    char s[50];//////////////////////////////////////////
    struct usermessage *p; 
    struct usermessage *a;
    char dirname[40]; //保存目录名
    char path[40]; // 保存当前路径
    char tmp[40];
    char tmp1[40];
    int fd;
    a=phead;
    p=(struct usermessage*)malloc(sizeof(struct usermessage));
    strcpy(p->usrname,recvbuf.idname);
    strcpy(dirname,recvbuf.idname);
    strcpy(p->password,recvbuf.password);
    send(connfd,"p",sizeof("p"),0);
    strcpy(s,recvbuf.idname);///////////////////////////////
    strcat(s,"进行了注册");//////////////////////////////
    strcpy(mn,s);//////////////////////////////////////////
     getcwd(path,sizeof(path));
 
     strcat(path,"/");
     strcat(path,"rizhi.txt");
     fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
     write(fd,mn,sizeof(mn));
    memset(mn,0,sizeof(mn));///////////////////////////
    while(a->next!=NULL)
    {
        a=a->next;
    }
    a->next=p;
    p->next=NULL;
    writezc();
    /*每注册一个用户创建一个目录*/
    mkdir(dirname,0777); //创建目录
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,dirname);
    strcpy(tmp,path);
    strcat(tmp,"/list.txt");
    strcpy(tmp1,path);
    strcat(tmp1,"/box.txt");
    open(tmp,O_RDWR|O_CREAT,0777);
    open(tmp1,O_RDWR|O_CREAT,0777);
}

/*用户登录函数*/
void denglu(int connfd,struct message recvbuf)
{
    int fd;
    char path[40];
    struct online *z;
    z=ahead->next;
    while(z!=NULL)
    {
        if(strcmp(z->name,recvbuf.idname) == 0)
        {
            send(connfd,"x",sizeof("x"),0);
            return;
        }
        z=z->next;
    }

    char s[50];////////////////////////////////
    int n=0;//////////////////////////////////////
    struct usermessage *q; //定义链表指针寻找目标用户名密码
    struct online *p;
    struct online *a;
    p=(struct online*)malloc(sizeof(struct online));//存储在线用户申请空间
    q=phead->next; //指向注册链表头结点
    int i;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    a=ahead;
    while(q)
    {
        if(strcmp(q->usrname,recvbuf.idname) == 0&&strcmp(q->password,recvbuf.password)==0)//对比目标信息与结构体储存信息
        {
            n=1;///////////////////////////
            strcpy(s,recvbuf.idname);///////////////////////////////
            strcat(s,"成功登录");//////////////////////////////
            strcpy(mn,s);//////////////////////////////////////////
             getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
            memset(mn,0,sizeof(mn));///////////////////////////

            send(connfd,"y",sizeof("y"),0); // 若找到发送信号给客户端
            pthread_mutex_lock(&mutex);   //上锁
            strcpy(p->name,q->usrname); //将在线用户姓名存入
            p->fd=connfd; //将对应套接字存入
            while(a->next!=NULL)
            {
                a=a->next;
            }
            a->next=p;
            p->next=NULL;  //此时在线用户信息已存入链表
            pthread_mutex_unlock(&mutex);
            return;          //退出函数
        }
        q=q->next;
    }
    n=2;
    send(connfd,"n",sizeof("n"),0);//若未找到发送信号给客户端  
    strcpy(s,recvbuf.idname);///////////////////////////////
    strcat(s,"登录失败");//////////////////////////////
    strcpy(mn,s);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
    memset(mn,0,sizeof(mn));///////////////////////////  
}

/*私聊函数*/
void siliao(int connfd,struct message recvbuf)
{
    time_t timep;
    time_t timeq;
    time_t timee;
    char x[100];
    char t[50];///////////////////////////// 
    int fd,fd_rizhi;
    char path[40];
    char s[40];
    char l[100];
    struct mainmessage chat;
    struct online *p;
    memset(&chat,0,sizeof(chat));
    p=ahead->next;
    strcpy(chat.sign,"siliao");
    strcpy(chat.chatname,recvbuf.idname);
    strcpy(chat.rechatname,recvbuf.recvename);
    strcpy(chat.message,recvbuf.message);

    /**************************************************/
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.idname);
    strcat(path,"/");
    memset(s,0,sizeof(s));
    strcpy(s,recvbuf.recvename);
    strcat(s,".txt");
    strcat(path,s);
    fd=open(path,O_RDWR|O_APPEND|O_CREAT,0777);
    
    
    memset(x,0,sizeof(x));
    time(&timeq);
    strcpy(x,recvbuf.idname);
    strcat(x,"对");
    strcat(x,recvbuf.recvename);
    strcat(x,"说了");
    strcat(x,recvbuf.message);
    strcat(x,"     ");
    strcat(x,ctime(&timeq));
    write(fd,x,sizeof(x));
    memset(s,0,sizeof(s));
    memset(path,0,sizeof(path));
    close(fd);


    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.recvename);
    strcat(path,"/");
    
    memset(s,0,sizeof(s));
    strcpy(s,recvbuf.idname);
    strcat(s,".txt");
    strcat(path,s);
    fd=open(path,O_RDWR|O_APPEND|O_CREAT,0777);
    
    memset(x,0,sizeof(x));
    time(&timee);
    strcpy(x,recvbuf.idname);
    strcat(x,"对");
    strcat(x,recvbuf.recvename);
    strcat(x,"说了");
    strcat(x,recvbuf.message);
    strcat(x,"     ");
    strcat(x,ctime(&timee));
    write(fd,x,sizeof(x));
    memset(s,0,sizeof(s));
    memset(path,0,sizeof(path));
    close(fd);

    while(p!=NULL)
    {
      
        if(strcmp(p->name,chat.rechatname) == 0)
        {
            strcpy(t,recvbuf.idname);///////////
            strcat(t,"对");////////////
            strcat(t,recvbuf.recvename);/////////
            strcat(t,"说了");///////////
            strcat(t,recvbuf.message);////////////
            strcpy(mn,t);//////////////////////////////////////////
            getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
            close(fd_rizhi);
            memset(mn,0,sizeof(mn));///////////////////////////
            memset(t,0,sizeof(t));////////////////////

            if( send(p->fd,&chat,sizeof(chat),0) <0)
            {
                myerror("personchat",__LINE__);
            }
            return;
            memset(&chat,0,sizeof(chat));
        }        
        else
        {
            p=p->next;
        }
    }       

    strcpy(chat.chatname,"服务器");
    strcpy(chat.message,"好友未上线\n");
    send(connfd,&chat,sizeof(chat),0);


    time(&timep);
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.recvename);
    strcat(path,"/");
    strcat(path,"lixian.txt");
    fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
    strcpy(l,recvbuf.idname);
    strcat(l,"发来消息：");
    strcat(l,recvbuf.message);
    strcat(l,"      ");
    strcat(l,ctime(&timep));
    write(fd,l,sizeof(recvbuf.message));
    close(fd);
    strcpy(t,recvbuf.idname);////////////////////////
    strcat(t,"未将消息发送成功");/////////////////////
    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
            close(fd_rizhi);
    memset(mn,0,sizeof(mn));///////////////////////////

}

/*退出函数*/
void Exit(struct message recvbuf)
{
    char path[40];
    int fd;   

    char t[40];//////////////////
    struct online *p;
    struct online *q;
    p=ahead;
    if(p->next == NULL)  return ;
    strcpy(t,recvbuf.idname);
    strcat(t,"下线");

    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
    memset(mn,0,sizeof(mn));///////////////////////////
    memset(t,0,sizeof(t));/////////////////////////////////
    while(strcmp(p->name,recvbuf.idname)!=0&&p!=0)
    {
        q=p;
        p=p->next;
    }
    q->next=p->next;
    free(p);
}

/*群聊函数*/
void qunliao(int connfd,struct message recvbuf)
{
    char x[100];
    char c[100];
    time_t timep;
    time_t timeq;
    char t [50];//////////////////////////////
    struct mainmessage chats;
    struct online *p;
    p=ahead->next;
    int n=0;
    memset(&chats,0,sizeof(chats));
    int i;
    char s[10][10];
    char path[40];
    int fd,fd_rizhi;
    char d[40];
    
    strcpy(t,recvbuf.idname);///////
    strcat(t,"发送了群聊消息");///////////////////////
    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
            close(fd_rizhi);
    memset(mn,0,sizeof(mn));///////////////////////////
    memset(t,0,sizeof(t));/////////////////////// 
    for(i=0;i<recvbuf.number;i++)
    {
        strcpy(s[i],recvbuf.allchat[i]);
    }
    for(i=0;i<recvbuf.number;i++)
    {
        p=ahead->next;
        while(p)
        {
            if(strcmp(p->name,s[i]) == 0)
            {
                strcpy(t,recvbuf.idname);////////////////
                strcat(t,"对");///////////////////////
                strcat(t,s[i]);/////////////////////
                strcat(t,"说");////////////////////
                strcat(t,recvbuf.message);///////////////
                strcpy(mn,t);//////////////////////////////////////////
                 getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
                close(fd_rizhi);
                memset(mn,0,sizeof(mn));///////////////////////////
                memset(t,0,sizeof(t));///////////////     

                time(&timep);
                getcwd(path,sizeof(path));
                strcat(path,"/");
                strcat(path,recvbuf.idname);
                strcat(path,"/");
                strcpy(d,s[i]);
                strcat(d,".txt");
                strcat(path,d);
                fd=open(path,O_RDWR|O_APPEND|O_CREAT,0777);
                memset(x,0,sizeof(x));
                strcpy(x,recvbuf.idname);
                strcat(x,"对");
                strcat(x,s[i]);
                strcat(x,"说了");
                strcat(x,":");
                strcat(x,recvbuf.message);
                strcat(x,"     ");
                strcat(x,ctime(&timep));
                write(fd,x,sizeof(recvbuf.message));
                memset(d,0,sizeof(d));
                memset(path,0,sizeof(path));
                close(fd);

                getcwd(path,sizeof(path));
                strcat(path,"/");
                strcat(path,s[i]);
                strcat(path,"/");
                strcpy(d,recvbuf.idname);
                strcat(d,".txt");
                strcat(path,d);
                fd=open(path,O_RDWR|O_APPEND|O_CREAT,0777);
                memset(c,0,sizeof(c));
                time(&timeq);
                strcpy(c,recvbuf.idname);
                strcat(c,"对");
                strcat(c,s[i]);
                strcat(c,"说了");
                strcat(c,":");
                strcat(c,recvbuf.message);
                strcat(c,"     ");
                strcat(c,ctime(&timeq));
                write(fd,c,sizeof(recvbuf.message));
                memset(d,0,sizeof(d));
                memset(path,0,sizeof(path));
                close(fd);

                memset(&chats,0,sizeof(chats));
                strcpy(chats.sign,"qunliao");
                strcpy(chats.chatname,recvbuf.idname);
                strcpy(chats.message,recvbuf.message);
                send(p->fd,&chats,sizeof(chats),0);
                n=1;
                break;
            }
            else
            {
                p=p->next;
            }
            n=0;
        }
        if(n==0)
        {
            strcpy(chats.sign,"qunliao");
            strcpy(chats.chatname,"服务器");
            strcpy(chats.message,"未上线");
            strcpy(t,recvbuf.idname);
            strcat(t,"向");
            strcat(t,s[i]);
            strcat(t,"发送消息失败");
            strcpy(mn,t);//////////////////////////////////////////
             getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
            memset(mn,0,sizeof(mn));///////////////////////////
            send(connfd,&chats,sizeof(chats),0);
        }
    }
}

/*添加好友函数*/
void insert(int connfd,struct message recvbuf)
{
    char t[40];///////////////////////////////
    struct online *p;
    p=ahead->next;
    struct mainmessage a;
    int fd;
    char path[40];
    
    read_file(recvbuf.idname);
    struct information *e;
    struct mainmessage w;
    e=ehead->next;
    while(e!=NULL)
    {
        if(strcmp(e->name,recvbuf.insert) == 0)
        {
            memset(&w,0,sizeof(w));
            strcpy(w.sign,"insert");
            w.insertn=1;
            send(connfd,&w,sizeof(w),0);
            return;
        }
        e=e->next;
    }


   w.insertn=2;
    while(p)
    {
        if(strcmp(p->name,recvbuf.insert) == 0)
        {
            strcpy(t,recvbuf.idname);////////////////////////
            strcat(t,"请求添加");//////////////////////
            strcat(t,recvbuf.insert);///////////////////
            strcat(t,"为好友");/////////////////////////////
            strcpy(mn,t);//////////////////////////////////////////
             getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
            memset(mn,0,sizeof(mn));///////////////////////////
            close(fd);

            strcpy(a.sign,"insert");
            strcpy(a.chatname,recvbuf.idname);
            strcpy(a.message,"请求添加您为好友");
            strcat(a.chatname,"请求添加您为好友");
            strcpy(a.getrequest,a.chatname);
            send(p->fd,&a,sizeof(a),0);
            return;

        }
        p=p->next;
    }

    strcpy(a.sign,"insert");
    strcpy(a.chatname,"服务器");
    strcpy(a.message,"好友未上线");
    strcpy(t,recvbuf.idname);/////////////////////////////
    strcat(t,"请求添加的好友未上线");///////////////////////////
    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
    memset(mn,0,sizeof(mn));///////////////////////////
    close(fd);

    send(connfd,&a,sizeof(a),0);
}

/*消息盒子*/
void insertbox(int connfd,struct message recvbuf)
{
    char t[40];
    char path[40];
    int fd,fd_rizhi;
    if(recvbuf.boxsign == 1)
    {

        read_file(recvbuf.idname);
        struct information *e;
        struct mainmessage w;
        e=ehead->next;
        while(e!=NULL)
        {
            if(strcmp(e->name,recvbuf.getboxname) == 0)
            {
                return;
            }
            e=e->next;
        }

        strcpy(t,recvbuf.idname);///////////////////////////
        strcat(t,"添加");//////////////////
        strcat(t,recvbuf.getboxname);///////////////
        strcat(t,"为好友");//////////////////////////
        strcpy(mn,t);//////////////////////////////////////////
         getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));

        memset(mn,0,sizeof(mn));///////////////////////////
        memset(t,0,sizeof(t));/////////////////////////////////
        close(fd_rizhi);

        getcwd(path,sizeof(path));
        strcat(path,"/");
        strcat(path,recvbuf.idname);
        strcat(path,"/list.txt");
        fd=open(path,O_RDWR|O_APPEND);
        write(fd,recvbuf.getboxname,sizeof(recvbuf.getboxname));
        close(fd);
        getcwd(path,sizeof(path));
        strcat(path,"/");
        strcat(path,recvbuf.getboxname);
        strcat(path,"/list.txt");
        fd=open(path,O_RDWR|O_APPEND);
        write(fd,recvbuf.idname,sizeof(recvbuf.idname));
        close(fd);
    }
    else if(recvbuf.boxsign == 2)
    {
        strcpy(t,recvbuf.idname);///////////////////////////
        strcat(t,"拒绝添加");//////////////////
        strcat(t,recvbuf.getboxname);///////////////
        strcat(t,"为好友");//////////////////////////
        strcpy(mn,t);//////////////////////////////////////////
         getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
        memset(mn,0,sizeof(mn));///////////////////////////
        memset(t,0,sizeof(t));/////////////////////////////////
        close(fd_rizhi);
        
    }
}

void lookfriend(int connfd,struct message recvbuf)
{
    char t[40];
    int ret;
    int fd,fd_list;
    int i;
    int n;
    int size;
    struct mainmessage p;
    char path[40];
    char a[40][40];
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.idname);
    strcat(path,"/list.txt");
    fd_list=open(path,O_RDWR);
    strcpy(t,recvbuf.idname);
    strcat(t,"查看了好友");

    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd,mn,sizeof(mn));
            close(fd);
    memset(mn,0,sizeof(mn));///////////////////////////
    memset(t,0,sizeof(t));/////////////////////////////////
    
    n = 0;
    while(1)
    {
        
        if((size = read(fd_list,a[n],sizeof(a[n]))) == -1 )
            myerror("read",__LINE__);
        if(size == 0)
            break;
        strcpy(p.a[n],a[n]);
        n++;
    }
    close(fd_list);
    strcpy(p.sign,"look");
    p.n=n;

    send(connfd,&p,sizeof(p),0);    
}

void liaotianjilu(int connfd,struct message recvbuf)
{
    char t[40];
    char a[100][100];
    char path[40];
    char s[40];
    struct mainmessage p;
    int fd,fd_rizhi;
    int n;
    int size;
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.idname);
    strcat(path,"/");
    strcpy(s,recvbuf.recordname);
    strcat(s,".txt");
    strcat(path,s);
    fd=open(path,O_RDWR|O_CREAT|O_APPEND);
    strcpy(t,recvbuf.idname);/////////
    strcat(t,"查看了与");////////////
    strcat(t,recvbuf.recordname);
    strcat(t,"的聊天记录");///////////

    strcpy(mn,t);//////////////////////////////////////////
     getcwd(path,sizeof(path));

            strcat(path,"/");
            strcat(path,"rizhi.txt");
            fd_rizhi=open(path,O_CREAT|O_RDWR|O_APPEND,0777);
            write(fd_rizhi,mn,sizeof(mn));
            close(fd_rizhi);
    memset(mn,0,sizeof(mn));///////////////////////////
    memset(t,0,sizeof(t));/////////////////////////////////


    n = 0;
    while(1)
    {
        size = read(fd,a[n],sizeof(a[n]));
        if(size == 0)
        break;
        strcpy(p.b[n],a[n]);
        n++;       
    }
    close(fd);
    strcpy(p.sign,"chatrecord");
    p.m=n;
    send(connfd,&p,sizeof(p),0);    
}


void lixianxiaoxi(int connfd,struct message recvbuf)
{
    struct online *m;
    char a[100][100];
    char path[40];
    char s[40];
    struct mainmessage p;
    int fd;
    int n;
    int size;
    getcwd(path,sizeof(path));
    strcat(path,"/");
    strcat(path,recvbuf.idname);
    strcat(path,"/");
    strcat(path,"lixian.txt");
    fd=open(path,O_RDWR|O_CREAT|O_APPEND);

    n = 0;
    while(1)
    {
        size = read(fd,a[n],sizeof(a[n]));
        if(size == 0)
        break;
        strcpy(p.c[n],a[n]);
        n++;       
    }
    strcpy(p.sign,"lixian");
    p.l=n;
    if(send(connfd,&p,sizeof(p),0) < 0)
        myerror("sendlixian",__LINE__);

}

int main(int argc,char *argv[])
{
    int sockfd; //服务端套接字
    int connfd; //用户端套接字
    int optval; 
    struct sockaddr_in serv_addr;//服务端结构体
    struct sockaddr_in cli_addr;//用户端结构体
    socklen_t cli_len;
    ahead = (struct online *)malloc(sizeof(struct online));//在线链表头指针
    ehead = (struct information*)malloc(sizeof(struct information));
    ahead->next = NULL;
    /*创建服务端套接字*/
    sockfd = socket(AF_INET,SOCK_STREAM,0); 
    if(sockfd < 0)
    {
        myerror("socket",__LINE__);
    }

    /*设置套接字可以重新绑定端口*/
    optval = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&optval,sizeof(int)) < 0)
    {
        myerror("setsockopt",__LINE__);
    }

    /*初始化服务器端地址结构*/
    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*将套接字绑定到本地端口*/
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_in)) <0)
    {
        myerror("bind",__LINE__);
    }

    /*将套接字转化为监听字*/
    if(listen(sockfd,LISTENQ) < 0)
    {
        myerror("listen",__LINE__);
    }
    void * servethread(void *arg)
    {
        int connfd = *(int *) arg;
        struct message recvbuf; //储存客户端传过来的结构体
        int flag;
        int ret;
        while(1)
        {
            if( (ret = recv(connfd,&recvbuf,sizeof(recvbuf),0)) < 0)
            {
                myerror("recv",__LINE__);
            }
            if(ret == 0){

                close(connfd);
                pthread_exit(NULL);
            }
            flag=recvbuf.flag;  //解析传过来的参数
            //printf("%d %d %d\n", flag, LOGIN, LOOKFRIEND);
            if(flag==REGISTER)  //客户端传来注册需求
            {
                zhuce(connfd,recvbuf);
            }
            else if(flag==LOGIN)//客户端传来登录需求
            {
                denglu(connfd,recvbuf);
            }
            else if(flag==PMM)//客户端传来私聊需求
            {
                siliao(connfd,recvbuf);
            }
            else if(flag==EXIT)//客户端退出
            {
                Exit(recvbuf);
                close(connfd);
                pthread_exit(NULL);
            }
            else if(flag==ALLCHAT)
            {
                qunliao(connfd,recvbuf);
            }
            else if(flag==INSERT)
            {
                insert(connfd,recvbuf);
            }
            else if(flag==INSERTBOX)
            {
                insertbox(connfd,recvbuf);
            }
            else if(flag==LOOKFRIEND)
            {
                lookfriend(connfd,recvbuf);
            }
            else if(flag==DELETEFRIEND)
            {
                delete(recvbuf);
            }
            else if(flag==CHATRECORD)
            {
                liaotianjilu(connfd,recvbuf);
            }

             else if(flag==NOTEXIT)
            {
                lixianxiaoxi(connfd,recvbuf);
            }
}
close(connfd);
}
cli_len=sizeof(struct sockaddr_in);
while(1)
{ 
    connfd=accept(sockfd,(struct sockaddr*)&cli_addr,&cli_len); //另线程处于接受状态
    if(connfd==-1) //若堵塞则退出
    {
        printf("连接出错！\n");
        continue;
    }
    pthread_t pid;
    readzc();
    pthread_create(&pid,0,servethread,&connfd); //创建线程
}
return 0;
}
