#include "get_path.h"
#include <glob.h>

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void executeCommand(char *command, char** args, int status);
void executeGlob(int cardIndex, char *commandpath, struct pathelement * pathlist, char **args, glob_t globbuf, int status);

#define PROMPTMAX 32
#define MAXARGS 10
