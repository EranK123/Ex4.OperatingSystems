/*
** server.c -- a stream socket server demo
*/
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
using namespace std;

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10 

// -------------- Malloc Free Imp ----------- //


typedef struct _mem_dictionary
{
    void *addr;
    size_t size;
    int freed;
} mem_dictionary;
size_t dictionary_ct=0;
mem_dictionary* dictionary;
void *malloc(size_t size)
{
     void *return_ptr = NULL;
     int i;

     if (dictionary == NULL) {
         dictionary =(mem_dictionary*) sbrk(4096 * sizeof(mem_dictionary));
         memset(dictionary, 0, 4096 * sizeof(mem_dictionary));
     }
     if(dictionary_ct>=4096 * sizeof(mem_dictionary))
     {
         dictionary_ct=0;
     }

     for (i = 0; i < dictionary_ct; i++)
         if (dictionary[i].size >= size
          && dictionary[i].freed)
     {
         dictionary[i].freed = 0;
         return dictionary[i].addr;
     }

     return_ptr = sbrk(size);

     dictionary[dictionary_ct].addr = return_ptr;
     dictionary[dictionary_ct].size = size;
     dictionary[dictionary_ct].freed = 0;
     dictionary_ct++;

     return return_ptr;
}

void free(void *ptr)
{
    int i;

    if (!dictionary)
        return;

    for (i = 0; i < dictionary_ct; i++ )
    {
        if (dictionary[i].addr == ptr)
        {
            dictionary[i].freed = 1;
            return;
        }
    }
}



// -------------- Stack Imp ----------- //

struct Node {
   string data;
   struct Node *next;
};
struct Node* top = NULL;
void push(string val) {
   struct Node* newnode = (struct Node*) malloc(sizeof(struct Node));
   newnode->data = val;
   newnode->next = top;
   top = newnode;
}
void pop() {
   if(top==NULL)
   cout<<"Stack Underflow"<<endl;
   else {
    struct Node* temp = top;
    top = top->next;
    free(temp);
   }
}

string peek(){
    cout << "OUTPUT: ";
    if(top != NULL){
    return top->data + "\n";
    }
    return "Stack is empty\n";
}

string display() {
   string s; 
   struct Node* ptr;
   if(top==NULL)
   return "\n";
   else {
      ptr = top;
      while (ptr != NULL) {
         s += ptr->data + " ";
         ptr = ptr->next;
      }
   }
   s += '\n';
   return s;
}

// -------------- Server Imp ----------- //

char client_message[1024];
pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER;
void * socketThread(void *arg)
{
  pthread_mutex_lock(&lock_);
  int newSocket = *((int *)arg);
   recv(newSocket , client_message , 1024 , 0);
   string commnad = client_message;
   if(commnad.substr(0, 4).compare("PUSH") == 0){
       push(commnad.substr(5, commnad.length()));
   }else if(commnad.substr(0, 3).compare("POP") == 0){
       pop();
   }else if(commnad.substr(0, 3).compare("TOP") == 0){
       string data = "OUTPUT: " + peek();
       send(newSocket, data.c_str(), strlen(data.c_str()), 0);
   }
   else if(commnad.substr(0, 4).compare("DISP") == 0){
        //  string out = "Stack elements are: " + display() + '\n';
        string out = "Stack elements are: " + display();
         send(newSocket, out.c_str(), strlen(out.c_str()), 0);
   }  

  pthread_mutex_unlock(&lock_);
  sleep(1);
  close(newSocket);
  pthread_exit(NULL);

}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // printf("server: waiting for connections...\n");
    pthread_t tid[60];
    int i = 0;
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        // printf("server: got connection from %s\n", s);

        if( pthread_create(&tid[i++], NULL, socketThread, &new_fd) != 0 ){
        printf("Failed to create thread\n");
        }

        if( i >= 50)
        {
            i = 0;
                 
          while(i < 50)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
    }

  return 0;    
    }
  
