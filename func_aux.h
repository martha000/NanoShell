#ifndef _FUNC_AUX_
#define _FUNC_AUX_

extern int continues;
extern char buffer[80];

void execCommandLine(char *rest);
void verifyCharacters(char character,char *rest);
void treat_signal(int sig, siginfo_t *siginfo, void *context);
int checkForStdOutAndStdError(char *rest);

#endif