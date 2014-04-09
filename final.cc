//Project- 1 : Operating systems - Multi-threaded web server//
//Srinivasan rengarajan - 50097996//
/*The following code is developed based on HTTP/1.0 request parameters specified by RFC 1945 regulations*/
/*The status codes in effect are:
404 - FILE NOT FOUND
200 - STATUS OK
302  - FILE MOVED PERMANENTLY
*/
#include<iostream>
#include<fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<pthread.h>
#include<malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<time.h>
#include<semaphore.h>


using namespace std;

#define LENGTH 1024;//size of the filename BUFFER
#define MAX 10000;
//Initialize conditional variables//
pthread_t t_worker;


int sleep_time =60;
int help_flag=0,debug_flag=0,log_flag=0,sched_flag=0,dir_flag=0,n_threads =4;//Define debug,help and logfile flags//



struct req_parameters {   
   int connfd;   
   int size;//refers to file size
   char buffer[1024]; //message buffer//
   char filename[1024];//refers to filename//
   int  type; //GET or HEAD request//
   int  status; //error/retrieved status //
}req,req2;   
using namespace std;

req_parameters data[10000];
//The queue data structure is implemented as an array//
//queue insertion functions//

int numberofrequests=0;

int front=-1,rear=-1;
void insertintoqueue(req_parameters r);
req_parameters deleteelementfromqueue(int);
req_parameters returnelement();


//Define the Listening thread function//
void *listen(void *arg)
{

req_parameters req;
int sockfd = socket(AF_INET, SOCK_STREAM,0);  
int code;
socklen_t clilen;
char ch[128], time_arrival[128]; //listening thread calculates time of arrival//
unsigned int acceptid;
char *file_name, *fname;
struct sockaddr_in cli_addr;        
clilen = sizeof(cli_addr);
struct stat st; //To calculate the file size//
off_t f_size;
char buffer[1024], in_buf[1024];
listen(sockfd,10); 
while(true)//Listens indefinitely//
{        
acceptid= accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
cout<<"Socket retrieve success"<<endl;
//code adopted from stackoverflow.com//
                time_t now;
                time(&now);
                struct tm * ct=localtime(&now); 
                struct timeval tv;
                strftime(ch, 128, "[%d/%b/%Y : %H:%M:%S %z]", ct);
                snprintf(time_arrival, 128, ch, tv.tv_usec); 
//code adopted from stackoverflow.com//
                memset(in_buf, 0, sizeof(in_buf));
                code = recv(acceptid,in_buf,1024,0);
/*sample code for parsing an incoming request
char str = GET /index.html HTTP/1.0
char s[2] = " ";
   char *token;
   strtok(str, s);
   token = strtok(NULL, " ") ; 
   printf("%s\n", token);
*/
strtok(in_buf, " ");
file_name = strtok(NULL, " ");   
char s[2]=" ";
char *token;
token = strtok(in_buf,s);
if(strcmp(token,"GET")==0)
req.type = 1;//GET
else
req.type =0; //HEAD
if(file_name!=NULL)
{
if (stat(fname, &st) == 0)
{f_size=st.st_size;}
}
req.size = f_size;
strcpy(req.filename,file_name);
strcpy(req.buffer,in_buf);
req.status=0;//defined later in worker thread//
req.connfd = acceptid;

insertintoqueue(req);
numberofrequests++;

}//end of while... Listens continuously and inserts into queue//
}                  

void *scheduling_thread(void *arg)
{

pthread_mutex_t sthread_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
int x, sid;
req_parameters req2;
int freethreads= n_threads; //Initialize to 4//
cout<<"thread Status- Number of free threads is"<<freethreads<<endl;
int acceptfd, n;

  if (sched_flag==0)
  {
   
pthread_mutex_lock(&sthread_mutex);
pthread_mutex_lock(&qmutex);  
   cout<<"FCFS scheduling"<<endl;
   req= returnelement(); //simple FCFS scheduling//
pthread_mutex_unlock(&qmutex);
pthread_mutex_unlock(&sthread_mutex); 
cout<<req.connfd<<endl;   
freethreads--; //request will be assigned to worker threads// 
  }
  else if (sched_flag==1)
{
  cout<<"SJF scheduling"<<endl;
  int min=data[1].size,sid=0;
  for (int i=0;i<=numberofrequests;i++)
  {
    if (data[i].size>min)
    { 
    min=data[i].size;
    x=i; //get the index with the minimum value//
    }
  }


pthread_mutex_lock(&sthread_mutex);
pthread_mutex_lock(&qmutex);  
req2 = data[x];
pthread_mutex_unlock(&qmutex);
pthread_mutex_unlock(&sthread_mutex); 

}
   
   sid =data[x].connfd; //corresponds to the request connection id 

   cout<<req2.connfd<<endl;
}

void *worker_thread(void *arg)   
{

char inbuff[1024]="HTTP/1.0 200 OK:Content-Type:text/html";
char outbuff[1024]="HTTP/1.0 200 OK:Content-Type:image/gif";
req_parameters r;
int fileid ,acceptid;
char *file_name;
char mess[1024] = "HTTP/1.0 404 Not Found:Content-Type:text/html";
char notfound[1024] ="<html><body><h1>FILE NOT FOUND</h1></body></html>";

while(true) //respond continuously//
{
   
  
     //code adopted from stackoverflow.com - corresponds to calculation of serving time//
      req_parameters r1=req;   //fcfs//
      req_parameters r2=req2; //SJF//
      if(sched_flag==0)
      r = req;
      else
      r= req2;
      time_t now;                                        
      time(&now);
      struct tm * ct=localtime(&now); 
      char ch[128],time_serve[128];
      struct timeval tv1;
      strftime(ch, sizeof ch, "[%d/%b/%Y : %H:%M:%S %z]", ct); 
      snprintf(time_serve, sizeof time_serve, ch, tv1.tv_usec); 
      memset(outbuff, 0, sizeof(outbuff));          
      //code logic adopted from // 
      acceptid=r.connfd;
      file_name=r.filename;
      fileid = open(&file_name[1], O_RDONLY, S_IREAD | S_IWRITE);
      //http://stackoverflow.com/questions/5309471/getting-file-extension-in-c//      
const char *type = strrchr(&file_name[1],'.');
      if (fileid == -1) //If cannot open file- analogus to File Not Found Exception in Java//
      
                {
                        cout<<"File not found, send a HTTP/1.0 404 status code"<<&file_name[1]<<endl;
                        r.status =404;
                        send(acceptid, mess,   strlen(mess), 0);
                        send(acceptid, notfound, strlen(notfound), 0);
                }
        else
                       {
                       cout<<"File sent"<<endl;
                       
                       if (strcmp(type,"html")==0)
 		       {send(acceptid,inbuff ,strlen(inbuff), 0);}
                       else
                       {send(acceptid,inbuff ,strlen(outbuff), 0);//referred from the following website// 
                       }    
																	//     ////   		
                }
}
}
void printUsage();//function decleration given for User to know all the options//
/*code given by TA*/
void printUsage()
{
fprintf(stderr, "Usage: myhttpd [−d] [−h] [−l file] [−p port] [−r dir] [−t time] [−n thread_num] [−s sched]\n");
fprintf(stderr,
"\t−d : Enter debugging mode. That is, do not daemonize, only accept\n"
"\tone connection at a time and enable logging to stdout. Without\n"
"\tthis option, the web server should run as a daemon process in the\n"
"\tbackground.\n"
"\t−h : Print a usage summary with all options and exit.\n"
"\t−l file : Log all requests to the given file. See LOGGING for\n"
"\tdetails.\n"
"\t−p port : Listen on the given port. If not provided, myhttpd will\n"
"\tlisten on port 8080.\n"
"\t−r dir : Set the root directory for the http server to dir.\n"
"\t−t time : Set the queuing time to time seconds. The default should\n"
"\tbe 60 seconds.\n"
"\t−n thread_num : Set number of threads waiting ready in the execution thread pool to\n"
"\tthreadnum. The d efault should be 4 execution threads.\n"
"\t−s sched : Set the scheduling policy. It can be either FCFS or SJF.\n"
"\tThe default will be FCFS.\n");
}
/*code given by TA*/
int main(int argc, char *argv[])
{
//Test the code for scheduling//
/*req_parameters r1;
int connfd1;   
   int size1;
   char buffer1[1024];
   char filename1[1024];
   int type1;
   int status1;
cout<<"enter the connfd"<<endl;
cin>>connfd1;
cout<<"Enter the size"<<endl;
cin>>size1;
cout<<"Enter the buffer id"<<endl;
cin>>buffer1;
cout<<"enter the filename"<<endl;
cin>>filename1;
cout<<"enter the type"<<endl;
cin>>type1;
cout<<"enter the status"<<endl;
cin>>status1;
r1.connfd = connfd1;
r1.size = size1;
strcpy(r1.buffer,buffer1);
r1.type=type1;
r1.status = status1;
insertintoqueue(r1);
//deleteelementfromqueue(connfd1);
*/





//Log file code - adapted from public code given by TA//
int port = 8080;
pthread_t t_listener,t_scheduler,t_worker[6]; //max 6 worker threads//
char *logfile = NULL;
char *rootdir = "/";
int queueing_time = 60;
char *schedPolicy = "FCFS";
int c;
char *dir=0;
opterr = 0;
if (argc < 2)
{
printUsage();
exit(1);
}
while ( ( c = getopt (argc, argv, "dhl:p:r:t:n:s:") ) != -1 )
{
switch (c)
{
case 'd':
debug_flag=1;
n_threads=1;
break;
case 'h':
printUsage();
help_flag=1;
exit(1);
case 'l':
logfile = optarg;
break;
case 'p':
port = atoi(optarg);
//cout<<"Port is"<<endl<<port<<endl;
if (port < 1024)
{
fprintf(stderr, "[error] Port number must be greater than or equal to 1024.\n");
exit(1);
}
break;
case 'r':
rootdir = optarg;
break;
case 't':
sleep_time = atoi(optarg);
if (sleep_time < 1)
{
fprintf(stderr, "[error] queueing time must be greater than 0.\n");
exit(1);
}
break;
case 'n':
n_threads = atoi(optarg);
cout<<"The number of threads chosen is "<<n_threads<<endl;
if (n_threads < 1)
{
fprintf(stderr, "[error] number of threads must be greater than 0.\n");
exit(1);
}
break;
case 's':
schedPolicy = optarg;
cout<<"The scheduling policy is "<<schedPolicy<<endl;
if (schedPolicy== "FCFS")
{sched_flag=0;}
else
{sched_flag=1;}
break;
default:
printUsage();
exit(1);
}
} // while (...)
 
if (debug_flag == 1)
{
fprintf(stderr, "myhttpd logfile: %s\n", logfile);
fprintf(stderr, "myhttpd port number: %d\n", port);
fprintf(stderr, "myhttpd rootdir: %s\n", rootdir);
fprintf(stderr, "myhttpd queueing time: %d\n", queueing_time);
fprintf(stderr, "myhttpd number of threads: %d\n", n_threads);
fprintf(stderr, "myhttpd scheduling policy: %s\n", schedPolicy);
}
if(help_flag==1)                        
        {
                printUsage();
                exit(1);
        }
        else if(dir_flag==1)                        
        {
                if(chdir(dir)<0)
                        {
                                perror("\ndirectory doesnt exist");
                                exit(1);
                        }

}      

//Socket Programming - adapted from codeproject.com//
//Initialize number of threads as given by the user// 
int x = 0;

while(x <= n_threads)
{
 //pthread_create(&t_worker[x],NULL,&worker_thread ,NULL);
 x++;
}
cout<<"Worker Threads Created"<<endl; 
int listenfd = 0,connfd = 0;
  
struct sockaddr_in serv_addr;
 
char sendBuff[1025];  
int numrv;  
int ids;
 
listenfd = socket(AF_INET, SOCK_STREAM, 0);
printf("socket retrieve success\n");
  
memset(&serv_addr, '0', sizeof(serv_addr));
memset(sendBuff, '0', sizeof(sendBuff));
      
serv_addr.sin_family = AF_INET;    
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
serv_addr.sin_port = htons(8080);    
 
bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));


  
//Synchornization//
//There needs  to be a 60 sec time lag between the queuing thread and the scheduling thread//
//Once, the job is done the worker thread should signal the scheduling thread that the job is complete and increment number of free threads//


ids = listenfd;
pthread_create(&t_listener,NULL,&listen,&ids); //Create listening thread//                      
sleep(sleep_time);         //the thread sleeps for 60 sec//                                               
pthread_create(&t_scheduler,NULL,&scheduling_thread,&sched_flag);        //create scheduling thread//
pthread_join(t_listener,NULL);
pthread_join(t_scheduler,NULL);

  
  
  

return 0;

    
}


//Function definitions//
void insertintoqueue(req_parameters r)
{
 req_parameters r2 = r;
 if(rear>=9999)
{
 cout<<"queue is full,requests cannot be inserted"<<endl;
}
 else if(rear==-1 && front ==-1)
 { 
rear++;
data[rear]= r2;
cout<<"element inserted"<<endl;
}
}

req_parameters deleteelementfromqueue(int element)
{
 req_parameters r = data[front];
 int conn= element;
 if(front==rear)
 cout<<"Queue is empty"<<endl;
 else if (data[front].connfd==element)
 {
 front++;
 return data[front];
 }
}

req_parameters returnelement()
{
if(front==rear)
 cout<<"Queue is empty"<<endl;
 else
 {
 front++;
 return data[front];
 }
}


