#include "command.h"

void initCommand(command *application) {
	application->input = NULL;
	application->argv = NULL;
	application->argc = 0;
	application->env = NULL;
	application->foreground = 0;
}

void parseCommand(char *input, command *application, char **env) {
	char *tmp;
	application->input = (char *)malloc(sizeof(char) * strlen(input));
	strcpy(application->input, input);
	if (strchr(input, '&')) {
		application->foreground = 0;
	} else {
		application->foreground = 1;
	}
	tmp = strtok(input, " ");
	application->argc = 0;
	application->argv = (char **)malloc(sizeof(char *));
	while (tmp) {
		if (strcmp(tmp, "&")) {
			application->argv[application->argc] = (char *)malloc(sizeof(char) * strlen(tmp));
			strcpy(application->argv[application->argc], tmp);
			application->argc++;
			application->argv = (char **)realloc(application->argv, (sizeof(char *) * (application->argc + 1)));
		}
		tmp = strtok(NULL, " ");
	}
	application->argv[application->argc] = NULL;
	application->env = env;
}

void copyCommand(command *dest, command source) {
	int i;
	//copia o input
	dest->input = (char *)malloc((sizeof(char) * strlen(source.input)));
	strcpy(dest->input, source.input);
	//copia o argc
	dest->argc = source.argc;
	//copia o argv
	dest->argv = (char **)malloc((sizeof(char *) * (source.argc + 1)));
	for (i = 0; i < source.argc; i++) {
		dest->argv[i] = (char *)malloc((sizeof(char) * strlen(source.argv[i])));
		strcpy(dest->argv[i], source.argv[i]);
	}
	dest->argv[dest->argc] = NULL;
	//copia o env
	dest->env = source.env;
}

void freeCommand(command *application) {
	if (application->input) {
		free(application->input);
		application->input = NULL;
	}
	if (application->argc) {
		while (application->argc) {
			application->argc--;
			if (application->argv[application->argc]) {
				free(application->argv[application->argc]);
			}
		}
		if (application->argv) {
			free(application->argv);
			application->argv = NULL;
		}
	}
	application->env = NULL;
}

//verifica a existência do aplicativo
int isCommand(char *command) {
	struct stat st;
	return !stat(command, &st);
}

//adiciona o path ao comando
char *createCommand(char *app, char *path) {
	char *cmd;
	cmd = (char *)malloc((sizeof(char) * (strlen(app) + strlen(path) + 2)));
	strcpy(cmd, path);
	strcat(cmd, "/");
	strcat(cmd, app);
	return cmd;
}

void findCommand(command *app, char *workdir) {
	char *cmd = NULL, *path[5] = {workdir, "/bin", "/sbin", "/usr/bin", "/usr/sbin"};
	int i = 0, found = 0;
	while ((!found) && (i < 5)) {
		if (cmd) {
			free(cmd);
		}
		cmd = createCommand(app->argv[0], path[i]);
		found = isCommand(cmd);
		i++;
	}
	if (found) {
		free(app->argv[0]);
		app->argv[0] = cmd;
	} else {
		free(cmd);
	}
}
