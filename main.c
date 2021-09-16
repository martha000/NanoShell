/**
* @file main.c
* @brief Projeto de PA
* @date 2020
* @author Marta Rosa Borralho dos Santos - 2181351         Patricia Alexandre Pereira Paulo - 2191239
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "debug.h"
#include "memory.h"
#include "args.h"
#include "func_aux.h"
#define MAX 256

int continues = 1;
char buffer[80];

char commandRead[MAX];
char *check;

void startingShell();

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	struct gengetopt_args_info args;

	struct sigaction act;

	time_t rawtime;
	struct tm *info;

    time( &rawtime );

    info = localtime( &rawtime );

    strftime(buffer,80,"%Y-%m-%dT%H:%M:%S%z", info); //Cannot put like the enunciado  RELATORIO WARNING

	if (cmdline_parser(argc, argv, &args))
	{
		ERROR(2, "execution of cmdline_parser\n");
	}

	//--------------------------------------------------------------------------
	act.sa_sigaction = treat_signal;

	/* Mask without signals so it doesn't block them */
	sigemptyset(&act.sa_mask);

	act.sa_flags = 0;
	act.sa_flags |= SA_SIGINFO; 
	act.sa_flags |= SA_RESTART; /* recovers blocking calls */

	/* Capture of signal: SIGUSR1 */
	if (sigaction(SIGUSR1, &act, NULL) < 0){
		ERROR(6, "sigaction - SIGUSR1");
	}

	/* Capture of signal: SIGUSR2 */
	if(sigaction(SIGUSR2, &act, NULL) < 0){
		ERROR(6, "sigaction - SIGUSR2");
	}

	/* Capture of signal: SIGINT */
	if (sigaction(SIGINT, &act, NULL) < 0){
		ERROR(6, "sigaction - SIGINT");
	}

	while(continues){
		
		// Looks for 'bye' when introduced with ./nanoShell
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "bye") == 0)
			{
				printf("[INFO] bye command detected. Terminating nanoShell.\n");
				exit(0); //exit successfully
			}
		}

		// Verifies if nonsense was written
		if ( (!(args.file_given || args.max_given || args.signalfile_given)) && argc > 1)
		{
			ERROR(2, "Invalid Option");
		}

		if (argc == 1)
		{
			while (continues)
			{
				startingShell();
			}
		
		}
		

		// For the file option
		if (args.file_given)
		{
			FILE * fp;
			char *line = NULL;
			char buf[256];
			char character = '#';
			int f = 0;

			if ((fp = fopen(argv[2], "r")) == NULL)
			{
				fprintf(stderr,"ERROR: cannot open file '%s' -- %s\n",args.file_arg, strerror(errno));
				exit(1); //exit unsuccessfully
			}
			else
			{
				printf("[INFO] executing from file '%s'\n ", args.file_arg);
				//It successfully opened the file
				// reads text until newline is encountered
				while (fgets(buf, sizeof(buf), fp)) {
						
					char *commandRead = buf;
					
					if(commandRead[0] == '\n'){
						printf("\nYou only input an ENTER\n");
					}
					else
					{
						for (size_t i = 0; i < strlen(commandRead); i++)
						{
							if(commandRead[i]=='\n'){
								commandRead[i]='\0';
								if(commandRead[0]!=character){
									f++;
									printf("\n[command #%d] %s\n", f, commandRead);
									execCommandLine(commandRead);

								}
							}
							
						}
					}
					
				}
				exit(0); // ficheiro lido successfully	
			
			}
			free(line);
			fclose(fp);
			printf("\nTHE FILE WAS CLOSED\n");
		}

		// Verify if the given max number is acceptable
		if (args.max_given)
		{
			if (args.max_arg <= 0){
				ERROR(2, "Invalid value '<int>' for -m/--max.\n");
			}
			else{

				printf("\n[INFO] terminates after %d commands", args.max_arg);

				for(int i = 0; i < args.max_arg;i++)
				{
					startingShell();		
				}
				
				printf("[END] Executed %d commands (-m %d)\n",args.max_arg,args.max_arg); 

				exit(0);
			}
		}

		if(args.signalfile_given)
		{
			FILE *signalfile;
			signalfile = fopen("signal.txt", "w");

			if (signalfile == NULL)
			{
				ERROR(3, "Failed to open the file");
			}

			printf("[INFO] created file 'signals.txt'");
			fprintf(signalfile, "kill -SIGINT %d\n", getpid()); 
			fprintf(signalfile, "kill -SIGUSR1 %d\n", getpid()); 
			fprintf(signalfile, "kill -SIGUSR2 %d\n", getpid()); 

			fclose(signalfile);

			while (continues){
				startingShell();
			}
		}

	}

	cmdline_parser_free(&args);
	return 0;
}

void startingShell() 
{

	printf("\nnanoShell$ ");

	check = (fgets(commandRead, MAX, stdin));
	if(check == NULL){
		ERROR(2,"No command detected");
	}
				
	if (continues)
	{
		if(commandRead[0] == '\n'){
			printf("\nIntroduziu apenas um ENTER\n");
		}
		else
		{
			for (size_t i = 0; i < strlen(commandRead); i++)
			{
				if(commandRead[i]=='\n')
				{
					commandRead[i]='\0';
					execCommandLine(commandRead);
				}
			}
		}
	}

}