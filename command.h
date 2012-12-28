#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
	//entrada original
	char *input;
	//parâmetros para a aplicação externa, vindos do comando lido
	char **argv;
	//número de parâmetros para a aplicação externa, vindos do comando lido
	int argc;
	//variáveis de ambiente
	char **env;
	//controle de foreground/background
	int foreground;
} command;

//inicializa a memória usada pelo parser
void initCommand(command *application);
//separa o comando de seus parâmetros
void parseCommand(char *input, command *application, char **env);
//copia a estrutura do comando
void copyCommand(command *dest, command source);
//libera a memória usada pelo parser
void freeCommand(command *application);
//faz a busca pelo comando no diretório corrente e nos diretórios do sistema (/bin; /sbin; /usr/bin; /usr/sbin)
void findCommand(command *app, char *workdir);

#endif
