#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include<threads.h>
#include<string.h>
#include <fcntl.h>

#define PORT 4444

struct arg_struct {
    int arg1;
    struct sockaddr_in arg2;
};

// struct for storing process info... Im storing just the pid, process name and user and kernel time.
struct process_info{
  int pid;
  char process_name[50];
  unsigned long user_time;
  unsigned long kernel_time;
  unsigned long total_time;
};


// processing string to get the desired values.. tokens separated by space..
void process_string(char buffer[], struct process_info *process)
{
  int j=0, token_number=0,space=0;
  char token[50];
    for(int i=0;i<=(strlen(buffer));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(buffer[i]==' '||buffer[i]=='\0')
        {
            token[j]='\0';
            token_number++;

            //pid was the first token
            if(token_number==1)
            {
              process->pid = atoi(token);
            }

            //process name was the 2nd token
            if(token_number==2)
            {
              strcpy(process->process_name, token); 
              if(process->process_name[strlen(process->process_name)-1]!=')')
              {
                space = 1;
              }
            }
            if(token_number == 3 && space == 1)
            {
              strcat(process->process_name,"_");
              strcat(process->process_name,token);
              space = 0;
              token_number--;
            }

            //user time was the 14th token
            if(token_number==14)
            {
              char* string_left;
              process->user_time = strtoul(token, &string_left, 10);

            }

            //kernel time was the 15th token
            if(token_number==15)
            {
              char* string_left;
              process->kernel_time = strtoul(token, &string_left, 10);
              process->total_time = process->user_time + process->kernel_time;
            }

            strcpy(token,"");
            j=0;    //for next word, init index to 0
        }
        else
        {
            token[j]=buffer[i];
            j++;
        }
    }
}
 

 // sorting the struct array according to the total time in descending order......

void sort(struct process_info all_processes[2000], int total_process)
{
    int i, j;
    struct process_info temp;
    
    for (i = 0; i < total_process - 1; i++)
    {
        for (j = 0; j < (total_process - 1-i); j++)
        {
            if (all_processes[j].total_time < all_processes[j + 1].total_time)
            {
                temp = all_processes[j];
                all_processes[j] = all_processes[j + 1];
                all_processes[j + 1] = temp;
            } 
        }
    }
}


// just for displaying all processes.....
void display(struct process_info all_processes[2000], int total_process)
{
    int i;
    
    printf("\nProcess Info: ");
    for (i = 0; i < total_process; i++)
    {
        printf("\nProcess: %d %s %lu %lu %lu", all_processes[i].pid,all_processes[i].process_name, all_processes[i].user_time, all_processes[i].kernel_time, all_processes[i].total_time);
    } 
}

void file_creation(struct process_info all_processes[2000], int N, char path[1000])
{
  FILE * fptr;

  char file_name[1000];
  strcpy(file_name,"server_side_");
  strcat(file_name,path);
  strcat(file_name,".txt");

  fptr = fopen(file_name, "w");
  printf("\n\n-------\n\nTop Processes--> \n ");
    for(int i=0;i<N;i++)
    {
        fprintf(fptr, "Pid:%d,Process_name:%s,User_time:%lu,Kernel_time:%lu,total_time:%lu ", all_processes[i].pid, all_processes[i].process_name, all_processes[i].user_time, all_processes[i].kernel_time, all_processes[i].total_time);
        printf("-> Pid: %d, Process Name: %s, user_time: %lu, kernel_time: %lu, total_time: %lu \n", all_processes[i].pid, all_processes[i].process_name, all_processes[i].user_time, all_processes[i].kernel_time, all_processes[i].total_time);
    }
    fprintf(fptr,"\n");
    printf("\nFile made at server end.....\n");
  fclose(fptr);
}

void read_file(char whole_file[50000], char path[1000])
{

FILE * fptr;

char file_name[1000];
  strcpy(file_name,"server_side_");
  strcat(file_name,path);
  strcat(file_name,".txt");

 fptr = fopen(file_name, "r");

 fgets(whole_file, 50000, fptr);

 fclose(fptr);
}


void work_with_processes(char whole_file[50000], short unsigned int path_val, char buffer[1024])
{
    // path for file
    char path[1000];
    sprintf(path, "%d", path_val);

  int N = atoi(buffer);
  

  // for reading the value of max pid stored at /proc/sys/kernel/pid_max
  int max_pid;

  int read_max_pid = open("/proc/sys/kernel/pid_max",O_RDONLY);
  char max_pid_str[20];

  read(read_max_pid, max_pid_str,10);
  max_pid = atoi(max_pid_str); 
  close(read_max_pid);


// array of struct containing all the processes info  
  struct process_info all_processes[2000];

// count of current processes....  
  int process_index = 0; 

  for(int i = 1; i <= max_pid;i++)
  {

    // making path string to access the processes with pid i
    char path[20];
    strcpy(path,"/proc/");
    char pid[20];
    sprintf(pid, "%d", i);
    strcat(path, pid);
    strcat(path,"/stat");

    int fp = open(path,O_RDONLY);

    // if a process with given pid not found, we need to continue to next iteration.....
    if(fp < 0)
      continue;



// reading the status of process from /proc/[pid]/stat and storing it into buffer.....
    char buffer[1024];

    int end_of_file = read(fp,buffer, 1024);
    buffer[end_of_file] = '\0';

// processing the string to extract relevant information..... 
    process_string(buffer, &all_processes[process_index]);
    process_index++;

// closing descriptor....
    close(fp);
  }


// sorting all processes....
  sort(all_processes, process_index);

  printf("\nTotal # of processes: %d\n", process_index);
  display(all_processes, process_index);


  file_creation(all_processes, N, path);

  read_file(whole_file,path);

}



void* server_thread(void *arguments)
{
    

    struct arg_struct *args = arguments;
    struct sockaddr_in newAddr = args->arg2;
    int newSocket = args->arg1;


    char buffer[1024];
    char top_N_records[50000];
    char top_record[200];

    //printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
    printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    while(1){
        recv(newSocket, buffer, 1024, 0);
        if(strcmp(buffer, "exit") == 0){
            printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
            break;
        }else{
            printf("Client: %s\n", buffer);
            work_with_processes(top_N_records, ntohs(newAddr.sin_port), buffer);
            send(newSocket, top_N_records, strlen(top_N_records), 0);
            printf("%s",top_N_records);
            printf("Server Files sent...\n\n");
            recv(newSocket, top_record, 200, 0);
            printf("--> Client (%s:%d) top process:\n%s", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), top_record);
            bzero(buffer, sizeof(buffer));
            bzero(top_record, sizeof(top_record));
            bzero(top_N_records, sizeof(top_N_records));
            printf("\n\n<<<<----One cycle complete for %s: %d----->>\n\n",inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        }
    }
    close(newSocket);
}



int main(){

    int sockfd, ret;
     struct sockaddr_in serverAddr;

    int newSocket;
    struct arg_struct args;

    struct sockaddr_in newAddr;

    socklen_t addr_size;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", 4444);

    if(listen(sockfd, 3) == 0){
        printf("[+]Listening....\n");
    }else{
        printf("[-]Error in binding.\n");
    }

    pthread_t pid[5];
    int i = 0;

    while(1){
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);


        args.arg1 = newSocket;
        args.arg2 = newAddr;


        if(newSocket < 0){
            exit(1);
        }

        if(pthread_create(&pid[i++], NULL, server_thread, (void *)&args) !=0)
        {
            printf("failed to create thread");
        }

        if(i > 5)
        {
            i = 0;
            while(i<5)
            {
                pthread_join(pid[i++],NULL);
            } 
            i=0;
        }

    }

    


    return 0;
}