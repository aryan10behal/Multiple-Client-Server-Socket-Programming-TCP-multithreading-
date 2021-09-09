#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 4444



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
  int j=0, token_number=0;
  char token[50];
  int space = 0;
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

void print_server_top_n_processes(char top_N_processes[50000])
{
	printf("\nServer's top N processes\n\n-->");
	for(int i=0;i<strlen(top_N_processes);i++)
	{
		if(top_N_processes[i]==' ')
			printf("\n-->");
		else
			printf("%c",top_N_processes[i]);
	}
}

void file_creation(char top_N_processes[50000], short unsigned int path_val)
{

	char path[1000];
    sprintf(path, "%d", path_val);


  FILE * fptr;

  char file_name[1000];
  strcpy(file_name,"client_side_");
  strcat(file_name,path);
  strcat(file_name,".txt");

  fptr = fopen(file_name, "w");
  fputs(top_N_processes, fptr);
  printf("\nFile made at client end.....\n");
  fclose(fptr);
}

void work_with_processes(char whole_file[50000], char buffer[1024], char top_record[200])
{

  // for reading the value of max pid stored at /proc/sys/kernel/pid_max

  int N = atoi(buffer);

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

  sprintf(top_record, "pid: %d process_name: %s user_time: %lu kernel_time: %lu total_time: %lu", all_processes[0].pid, all_processes[0].process_name, all_processes[0].user_time, all_processes[0].kernel_time, all_processes[0].total_time);
  printf("\n--> Client top process: %s",top_record);
  printf("\nTotal # of processes: %d\n", process_index);
  //display(all_processes, process_index);

}


int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char N[1024];

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");


	char top_N_records[50000];
	char top_record[200];

	while(1){
		printf("Client: # of processes--> \t");
		scanf("%s", &N[0]);
		send(clientSocket, N, strlen(N), 0);

		if(strcmp(N, "exit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		if(recv(clientSocket, top_N_records, 50000, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			print_server_top_n_processes(top_N_records);
			printf("storing top N records in client side file...");
			file_creation(top_N_records, ntohs(serverAddr.sin_port));
			work_with_processes(top_record,"1",top_record);
			printf("\nSending Clients top record..\n\n");
			send(clientSocket, top_record, strlen(top_record), 0);
			printf("<<<<----One cycle complete----->>\n\n");
			bzero(top_N_records, sizeof(top_N_records));
            bzero(top_record, sizeof(top_record));
		}

	}

	return 0;
}