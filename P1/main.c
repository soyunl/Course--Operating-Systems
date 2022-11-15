#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

Node* head = NULL;

void func_BG(char **cmd){
  //concatenating ./ + name of the process
  char *argv_cmd[] = {"bg",cmd[1],NULL};
  pid_t retcode = fork();
  char filename[100];
  strcpy(filename,"./");
  strcat(filename,cmd[1]);

  // in child process
  if (retcode == 0) {
    if (execvp(filename,argv_cmd)==-1) {
      perror("error on execvp");
      exit(0);
    }
  }
  // in parent process
  else if (retcode > 0) {
    head = add_newNode(head,retcode,filename);

  // error handling
  } else {
    printf("fork() error occured\n");
  }
}

void func_BGlist(char **cmd){
	//calling printList from linked_list.c
  printList(head);
}


void func_BGkill(char * str_pid){
	//calling kill() with SIGTERM and delete its node
  kill(atoi(str_pid),SIGTERM);
  head = deleteNode(head,atoi(str_pid));
  printf("%s has been killed\n",str_pid);
}


void func_BGstop(char * str_pid){
//calling kill() with SIGSTOP
  kill(atoi(str_pid),SIGSTOP);
  printf("%s has been stopped\n",str_pid);

}


void func_BGstart(char * str_pid){
	//calling kill() with SIGCONT
  kill(atoi(str_pid),SIGCONT);
  printf("%s has been continued\n",str_pid);
}


void func_pstat(char * str_pid){
	// calling /proc/pid/stat
  char statPath[100];
  strcpy(statPath,"/proc/");
  strcat(statPath,str_pid);
  strcat(statPath,"/stat");
  FILE* fp1 = fopen(statPath,"r");
  
  int pid;
  char comm[100];
  char state;
  fscanf(fp1, "%d %s %c\n", &pid, comm, &state);
  printf("comm:\t%s\n",comm);
  printf("state:\t%c\n",state);

  int count = 0;
  char line[1000];
  while (fgets(line,sizeof(line),fp1)!=NULL) {
    const char s[2] = " ";
    char *token;
    token = strtok(line,s);
    while (token!=NULL) {
      count++;
      if (count==11) {
        printf("utime:\t%s\n",token);
      } else if (count==12) {
        printf("stime:\t%s\n",token);
      } else if (count==21) {
        printf("rss:\t%s\n",token);
      }
      token = strtok(NULL,s);
    }
  }
  fclose(fp1);

  // calling /proc/pid/status
  char statusPath[100];
  strcpy(statusPath,"/proc/");
  strcat(statusPath,str_pid);
  strcat(statusPath,"/status");
  char line2[1000];
  int count2 = 0;
  FILE* fp2 = fopen(statusPath,"r");
  
  while (fgets(line2,sizeof(line2),fp2)!=NULL) {
    count2++;
    if (count2==35) {
      printf("%s",line2);
    } else if (count2==36) {
      printf("%s",line2);
    }
  }
  fclose(fp2);
}

 
int main(){
    char user_input_str[50];
    
    while (true) {
      printf("Pman: > ");
      fgets(user_input_str, 50, stdin);
      printf("User input: %s \n", user_input_str);
      char * ptr = strtok(user_input_str, " \n");
      if(ptr == NULL){
        continue;
      }
      char * lst[50];
      int index = 0;
      lst[index] = ptr;
      index++;
      while(ptr != NULL){
        ptr = strtok(NULL, " \n");
        lst[index]=ptr;
        index++;

      }
      if (strcmp("bg",lst[0]) == 0){
        func_BG(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        func_BGlist(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        func_BGkill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        func_BGstop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        func_BGstart(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        func_pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
        printf("Bye Bye \n");
        exit(0);
      } else {
        printf("Invalid input\n");
      }
    }

  return 0;
}

