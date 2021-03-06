/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

void terminate(char *line);
int commandes_internes(char**cmd);

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	  identically to the standard execution scheme:
	  parsecmd, then fork+execvp, for a single command.
	  pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);

	return 0;
}

/*Fonctions perso*/
//Fonction qui execute une ligne de commande
void execute_cmd(int bg, char **cmd){
		if (commandes_internes(cmd)!=1){
				pid_t process=fork();
				if (process==0){
						int flag=execvp(cmd[0],cmd);
						if (flag==-1){
								printf("%s\n",strerror(errno));
								terminate(0);
						}
				}
				else {
						if (bg!=1){
								wait(&process);
						}
				}
		}
}

// Commandes internes
int commandes_internes (char** cmd ){
	//siginfo_t *status;
	//ligne de commande change directory ... Bonus
	if(strcmp(cmd[0],"cd")==0 )
	{
			int flag;
			if(cmd[1])
			{
					flag=chdir(cmd[1]);
					if(flag==-1) perror("Erreur de la commande chdir.");
			}
			else printf("Syntaxe : cd argument missing");
			return 1;
	}
	//ligne de commande jobs
	if( strcmp(cmd[0],"jobs")==0 )
	{
			printf("Non ... là vraiment je vois pas comment faire");
			return 1;
	}
	return 0;
}

/*Fin fonctions perso*/

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}


int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line=0;
		int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {

			terminate(0);
		}



		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			//Execution de la commande
			execute_cmd(l->bg,cmd);
			// Affichage de la commande
			printf("seq[%d]: ", i);
        for (j=0; cmd[j]!=0; j++) {
                printf("'%s' ", cmd[j]);
        }
			printf("\n");
		}
	}

}
