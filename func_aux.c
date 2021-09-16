/**
* @file func_aux.c
* @brief Projeto de PA
* @date 2020
* @author Marta Rosa Borralho dos Santos - 2181351         Patricia Alexandre PereiraPaulo - 2191239
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "debug.h"
#include "memory.h"
#include "args.h"
#include "func_aux.h"

int nExecutedCmd = 0;
int nExecutedStdout = 0;
int nExecutedStderr = 0;

void execCommandLine(char *rest){

    int n = 0; //number of words separated by " "
    pid_t pid;
    char **argsAux = NULL;
    char *token;
    int stoudOrSterrIndex = 0;
    int execOption = 1;
    FILE *outputFile;
    
    if(rest == "\n"){
        printf("You've only input an ENTER\n");
        return;
    }
    
    for (int i = 1; i < strlen(rest); i++)
	{
		if (strcmp(rest, "bye") == 0)
		{
			printf("[INFO] bye command detected. Terminating nanoShell.\n");
			exit(0); //exit successfully
		}
	}

    while((token = strtok_r(rest," ",&rest))){

        //checking for non permited characters in each command line
        for(size_t i = 0; i<strlen(token);i++){
            verifyCharacters(token[i],token);
        }

        //Check point for redirectioning characters
        if(execOption == 1){
            execOption = checkForStdOutAndStdError(token);
        }
        
        if(execOption != 1){
            stoudOrSterrIndex = n;
        }

        //Alocating memory for command read
        argsAux = realloc(argsAux, sizeof(char*)*n);
        if(argsAux == NULL){
            ERROR(5, " alocating memory");
        }
  
        argsAux[n] = token;
        n++;            

        argsAux = realloc(argsAux, sizeof(char*)*(n+1));

        if(argsAux == NULL){
            ERROR(5, " alocating memory");
        }

        argsAux[n] = 0;

    }

    //Increment stdout counter
    if(execOption == 2 || execOption == 3){
        nExecutedStdout++;
    }

    //Increment stderr counter
    if(execOption == 4|| execOption == 5){
        nExecutedStderr++;
    }

    //Increment command counter
    nExecutedCmd++; 

    switch (pid = fork()) {
        case -1:
            /* Error */
            ERROR(1, "Error on the execution of fork()");
            /* 'break' is meaningless here since 'ERROR' already exits */

        case 0:
            /* Son */
            //printf("execOption-%d\n",execOption);

            switch(execOption){

                case 1:  //No options, executes normally
                    printf("[INFO] '%s' executed\n", argsAux[0]);
                    execvp(argsAux[0], argsAux);
                    break;
                    

                case 2:  //>
                    printf("[INFO] stdout redirected to '%s'\n", *(argsAux+stoudOrSterrIndex));
                    outputFile = freopen (*(argsAux+stoudOrSterrIndex),"w",stdout);
                    *(argsAux+stoudOrSterrIndex-1) = '\0';
                    printf("-------------------------------------------------");
                    execvp(argsAux[0],argsAux);
                    fclose(outputFile);                    
                    break;

                case 3:  //>> (append)
                    printf("[INFO] stdout redirected and appended to '%s'\n", *(argsAux+stoudOrSterrIndex));
                    outputFile = freopen (*(argsAux+stoudOrSterrIndex),"a",stdout);
                    *(argsAux+stoudOrSterrIndex-1) = '\0';
                    printf("-------------------------------------------------");
                    execvp(argsAux[0],argsAux);
                    fclose(outputFile);
                    break;

                case 4:  //2> stderr
                    printf("[INFO] stderr redirected to '%s'\n", *(argsAux+stoudOrSterrIndex));
                    outputFile = freopen (*(argsAux+stoudOrSterrIndex),"w",stderr);
                    *(argsAux+stoudOrSterrIndex-1) = '\0';
                    printf("-------------------------------------------------");
                    execvp(argsAux[0],argsAux);
                    fclose(outputFile);
                    break;

                case 5:  //2>> stderr
                    printf("[INFO] stderr redirected and appended to '%s'\n",*(argsAux+stoudOrSterrIndex));
                    outputFile = freopen (*(argsAux+stoudOrSterrIndex),"a",stderr);
                    *(argsAux+stoudOrSterrIndex-1) = '\0';
                    printf("-------------------------------------------------");
                    execvp(argsAux[0],argsAux);
                    fclose(outputFile);
                    break;

                default:
                    ERROR(1, "Something went wrong");
            } 

            ERROR(1, " Execvp went wrong.");
        
    default:
        /* Dad */
        wait(NULL);
        //printf("End of execution\n");
        
        break;
    }
    free(argsAux);
}

void verifyCharacters (char character,char *rest)
{
    //rest is the setence with the command to be executed
    //metacharacters detection

    switch (character)
    {
        case '*':
            printf("*\n");
            ERROR(4, "Wrong request '%s'", rest);
            break;
        
        case '?':
            printf("?\n");
            ERROR(4, "Wrong request '%s'", rest);
            break;

        case '\'':
            printf("'\n");
            ERROR(4, "Wrong request '%s'", rest); 
            break;

        case '"':
            printf("quotes\n");
            ERROR(4, "Wrong request '%s'", rest);
            break;

        case '|':
            printf("|\n");
            ERROR(4, "Wrong request '%s'", rest);
            break;

        default:
            //No metacharacters, it's okay to keep going
            break;
    }
}

void treat_signal(int sig, siginfo_t *siginfo, void *context) 
{
	(void)context;
	int aux;
	aux = errno;

	if(sig == SIGUSR1){

        printf("\nI've received the signal SIGUSR1 (%d)\n", sig);
        printf("\n%s", buffer);

	}
    else if (sig == SIGUSR2)
    {
        FILE *fptr;

        // For the date purpose
        char currentDate[80];
        time_t rawtime;
        struct tm *info;
        time( &rawtime );
        info = localtime( &rawtime );
        //

        strftime(currentDate, 80, "%Y.%m.%d_%Hh%M.%S", info);

        // Name of file
        char nameFile[] = "nanoshell_status_";
        strcat(nameFile, currentDate);
        strcat(nameFile, ".txt");
        printf("Check file %s\n", nameFile);
        //

        fptr = fopen(nameFile, "w");

        if (fptr == NULL)
        {
            ERROR(3, "Failed to open the file");
        }

        fprintf(fptr, "%d executions of applications\n %d executions with STDOUT redir \n %d executions with STDERR redir", nExecutedCmd, nExecutedStdout, nExecutedStderr); 
        
        fclose(fptr);
        

    }
    else if (sig == SIGINT)
    {
		printf("\nI've received the signal SIGINT \n");
        printf("\tPID: %ld\n", (long)siginfo->si_pid);
        exit(EXIT_SUCCESS);
        continues = 0;
    }

	errno = aux;
}

int checkForStdOutAndStdError(char *rest){
    //returns 1 if it doesn't match

    //if the length of the string is bigger than 3 it's already bigger than the possible options
    if(strlen(rest)>3){        
        return 1;
    }

    switch (rest[0])
    {
        case '>':

            // if there is only >
            if(strlen(rest)==1){
                return 2;
            }
            else
            {               
                if(strlen(rest)==2){

                    //if it's >>
                    if(rest[1]=='>'){
                        return 3;
                    }
                }
                else
                {   
                    return 1;
                }
            }
        break; 

        //2
        case '2':

            //if there is only a 2 it's wrong as it's not a valid redirection simbol
            if(strlen(rest)==1){
                return 1;
            }

            //2>
            if(strlen(rest)==2){
                if(rest[1]=='>'){
                        return 4;
                }
            }
            else{
                //2>>
                if(strlen(rest)==3){
                    if(rest[1]=='>' && rest[2]== '>'){
                        return 5;
                    }
                }
                else{
                    return 1;
                }
                return 1;
            }
            break;

        default:
            return 1;
    }
    return 1;
}

