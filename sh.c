#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

#define BUFFER_SIZE 128

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char BUFFER[BUFFER_SIZE];
  char promptBuff[PROMPTMAX];
  char **envi;
  glob_t globBuf;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while ( go )
  {
    /* print your prompt */
    printf("\n%s [%s]> ", prompt, pwd);
    /* get command line and process */
    if(fgets(BUFFER, BUFFER_SIZE, stdin) != NULL){
	    int lineLen = strlen(BUFFER);
	    if(BUFFER[lineLen-1] == '\n'){
	      BUFFER[lineLen-1] = 0;
	    }
	    strcpy(commandline, BUFFER);
    }
    /* check for each built in command and implement */
    int i = 0;
    char* com = strtok(commandline, " ");
    command = com;
    memset(args, '\0', MAXARGS*sizeof(char*));
    while(com){
	    args[i] = com;
	    com = strtok(NULL, " ");
	    i++;
    }
    if(command != NULL){
      if(strcmp(command,"exit") == 0){
	      break;
	    }
      else if((command[0] == '/') | (command[0] == '.') & (command[1] == '/') | ((command[1] == '.') & (command[2] == '/'))){	  
	      if( access(command, X_OK) == -1){
	        printf("\nUnable to access %s", command);
	        perror("Error ");
	      }
        else{
	        printf("\nExecuted path %s\n", command);
	        executeCommand(command, args, status);
	      }
	    }
      else if(strcmp(command, "which" ) == 0){
	      for( int i = 1; args[i] != NULL; i++){
	        commandpath = which(args[i], pathlist);
	        printf("\n%s",commandpath);
	        free(commandpath);
	      }
	    }
      else if(strcmp(command, "where" ) == 0){
	      for( int i = 1; args[i] != NULL; i++){
	        commandpath = where(args[i], pathlist);
	        free(commandpath);
	      }
	    }
      else if(strcmp(command, "list") == 0){
	      if(args[1] == NULL){
	      list(pwd);
	      }else{
	        int i = 1;
	        while(args[i]){
	          if(access(args[i], X_OK) == -1){
		          perror("\nError ");
	          }
            else{
		        printf("\n%s:\n", args[i]);
		        list(args[i]);
	          }
	          i++;
	        }
	      }
	    }
      else if(strcmp(command, "cd" ) == 0){
	      if(args[1] == NULL){
	        strcpy(owd, pwd);
	        strcpy(pwd, homedir);
	        chdir(pwd);
	      }
        else if(strcmp(args[1], "-") == 0){
	        p = pwd;
	        pwd = owd;
	        owd = p;
	        chdir(pwd);
	      }else if(args[1] != NULL && args[2] == NULL){		
	        if(chdir(args[1]) == -1){
	          perror("Error ");
	        }
          else{
	          memset(owd, '\0', strlen(owd));
	          memcpy(owd, pwd, strlen(pwd));  
	          getcwd(pwd, PATH_MAX+1);
	        }	    
	      }
      }
      else if( strcmp(command, "pid") == 0){
	      printf("\nPID: %d", getpid());
	    }
	    else if(strcmp(command, "pwd") == 0){
	      printf("\nPWD: %s", pwd);
	    }
      else if( strcmp(command, "kill") == 0){
	      if(args[1] == NULL){
	        printf("\nNo argument input for kill");
	      }
        else if(args[2] == NULL){
	        int temppId = -1;
	        sscanf(args[1], "%d", &temppId);
	        if(temppId != -1){
	          if(kill(temppId, 15) == -1){
		          perror("Error ");
	          }
	        }
          else{
	          printf("\nEntered invalid PID: Not a number!");
	        }
	      }
        else if(args[3] == NULL){
	        int temppId = -1;
	        int sig = 0;
	        sscanf(args[2], "%d", &temppId);
	        sscanf(args[1], "%d", &sig);
	        if(temppId != -1 && sig < 0){
	          if(temppId == getpid() && sig == -1){
		          free(owd);  
		          free(pwd);
		          free(prompt);  
		          free(args);
		          free(commandline);
		          deletepath(&pathlist);
		          pathlist = NULL;
	          }
	          if( kill(temppId, abs(sig)) == -1){
		          perror("Error ");
	          }
	        }
          else{
	          printf("\nInvalid arguments for kill");
	        }
	      }
	    }
      else if(strcmp(command, "prompt") == 0){
	      if(args[1] == NULL){
	        printf("/nInput prompt prefix: ");
	      if(fgets(promptBuff, PROMPTMAX, stdin) != NULL){
	        int lineLen = strlen(promptBuff);
	      if(promptBuff[lineLen-1] == '\n'){
		      promptBuff[lineLen-1] = 0; 
	      }
	        strtok(promptBuff, " ");
	        strcpy(prompt, promptBuff);
	      }
	      }
        else{
	        strcpy(prompt, args[1]);
	      }
	    }
      else if(strcmp(command, "printenv") == 0){
	      if(args[1] == NULL){
	        printenv(envi);
	      }
        else if(args[2] == NULL){
	        printf("\n%s\n", getenv(args[1]));
	      }else{
	        printf("\nprintenv: Too many arguments.");
	      } 
	    }
      else if(strcmp(command, "setenv") == 0){
	      if(args[1] == NULL){
	        printenv(envi);
	      }
        else if(args[2] == NULL && (strcmp(args[1], "PATH") == 0 || strcmp(args[1], "HOME") == 0)){
	        printf("\nPlease do not set PATH or HOME to empty\n");
	      }
        else if(args[2] == NULL){
	        if(setenv(args[1] , "", 1) == -1){
	          perror("Error ");}
	      }
        else if(args[3] == NULL){
	        if(setenv(args[1], args[2],1) == -1){
	          perror("Error ");
	        }else{
	          if(strcmp(args[1], "PATH")==0){
		          deletepath(&pathlist);
		          pathlist = NULL; 
	          }
	          if(strcmp(args[1], "HOME") == 0){
		        homedir = args[2];
	          }
	        }
	      }
        else{
	        printf("\nsetenv: Too many arguments.");
	      }
	    }
  }
  }
  free(owd);  
  free(pwd);
  free(prompt);  
  free(args);
  free(commandline);
  deletepath(&pathlist);
  pathlist = NULL;
  exit(0);
  return 0;
}


char *which(char *command, struct pathelement *pathlist ){
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
  char CAT_BUFFER[BUFFER_SIZE];
  struct pathelement *current = pathlist;
  DIR *dr;
  struct dirent *de;
  //Go though all our PATH_ENVs
  while (current != NULL) {
    char *path = current->element;
    //vars for looking though the directories
    dr = opendir(path);
    if(dr){
      //in each path, look at all of it's files
      while ((de = readdir(dr)) != NULL) {
        //for each file in the directory, check if it's the one we want
        if (strcmp(de->d_name, command) == 0) {
        //cat together the full filename
        strcpy(CAT_BUFFER, path);
        strcat(CAT_BUFFER, "/");
        strcat(CAT_BUFFER, de->d_name);
        //create a string pointer and return it
        int len = (int) strlen(CAT_BUFFER);
        char *p = (char *) malloc(len);
        strcpy(p, CAT_BUFFER);
        closedir(dr);
       return p;
        }
      }
    }
    closedir(dr);
    current = current->next;
  }
    //Return null if we haven't found one 
  return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  char CAT_BUFFER[BUFFER_SIZE];
  struct pathelement *current = pathlist;
  DIR *dr;
  struct dirent *de;
  strcpy(CAT_BUFFER, "");
  while (current != NULL) {
    char *path = current->element;
    dr = opendir(path);
    if(dr){
      while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, command) == 0) {
          strcat(CAT_BUFFER, path);
          strcat(CAT_BUFFER, "/");
          strcat(CAT_BUFFER, de->d_name);
          strcat(CAT_BUFFER, "\n");
        }
      }
    }
    closedir(dr);
    current = current->next;
  }
    int len = (int) strlen(CAT_BUFFER);
    char *p = (char *) malloc(len);
    CAT_BUFFER[len - 1] = '\0';
    strcpy(p, CAT_BUFFER);
    return p;
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
  DIR *dr;
  struct dirent *de;
  dr = opendir(dir);
  if (dr == NULL) {
    printf("Cannot open %s\n", dir);
  } 
  else {
    while ((de = readdir(dr)) != NULL) {
      printf("%s\n", de->d_name);
    }
  }
closedir(dr);
} /* list() */

void printenv(char ** envp){
    int i = 0;
    while(envp[i]!=NULL){
      printf("%s\n",envp[i]);
      i++;
    }
}

void executeCommand(char *commandpath, char** args, int status){
  if(commandpath == NULL){
    fprintf(stderr, "%s: Command not found.\n", args[0]); 
  }
  else{
	  pid = fork();
	  if(pid == 0){ 
	    execve(commandpath,args,NULL);
	    exit(EXIT_FAILURE);
	  }
    else if(pid < 0){
	  }else{
	    do{
	      waitpid(pid, &status, WUNTRACED);
	    }
      while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
}
void executeGlob(int cardIndex, char *commandpath, struct pathelement *pathlist, char **args, glob_t globBuf, int status){
      globBuf.gl_offs = cardIndex;
      glob(args[cardIndex], GLOB_DOOFFS, NULL, &globBuf);
      for (int i = 0; i < cardIndex; i++){
	      globBuf.gl_pathv[i] = calloc(sizeof(char), strlen(args[i])+1 );
	      strcpy( globBuf.gl_pathv[i], args[i]);	
      }
      commandpath = which(globBuf.gl_pathv[0], pathlist);
      executeCommand(commandpath, globBuf.gl_pathv, status);
      free(commandpath);
      for (int i = 0; i < cardIndex; i++){
	      free(globBuf.gl_pathv[i]);
      }
      globfree(&globBuf);
}