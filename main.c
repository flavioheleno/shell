#include <stdio.h>
#include <stdio_ext.h>
#include <signal.h>

#include "command.h"
#include "job.h"

//lista de 'jobs'
struct jobs *jobs = NULL;
//job corrente
pid_t job_pid = 0;
//controle do signal
int handler_action = 0;

void signalHandler(int signal) {
	struct job *j;
	if (!job_pid) {
		cleanJobs(&jobs);
		j = lastJob(jobs);
		if (j) {
			job_pid = j->pid;
		}
	}
	//força a impressão do buffer de saída
	fflush(stdout);
	switch (signal) {
		//ctrl+c
		case SIGINT:
			if (job_pid) {
				terminateJob(job_pid, &jobs);
				printf("\n[%d]\tterminado\t%s\n", job_pid, "");
				job_pid = 0;
			} else {
				handler_action = 1;
			}
			break;
		//ctrl+z
		case SIGTSTP:
			if (job_pid) {
				pauseJob(job_pid, &jobs);
				printf("\n[%d]\tpausado\t%s\n", job_pid, "");
			} else {
				handler_action = 2;
			}
			break;
		//child process stop/terminate
		case SIGCHLD:
			if (job_pid) {
				if (handler_action == 1) {
					terminateJob(job_pid, &jobs);
					printf("\n[%d]\tterminado\t%s\n", job_pid, "");
					job_pid = 0;
				} else if (handler_action == 2) {
					pauseJob(job_pid, &jobs);
					printf("\n[%d]\tpausado\t%s\n", job_pid, "");
				}
				handler_action = 0;
			}
			break;
		//default:
			//printf("SIGNAL: %d\n", signal);
	}
	//printf("job_pid: %d\n", job_pid);
	//remove os 'jobs' terminados da lista
	cleanJobs(&jobs);
	printf("\n");
}

int main(int argc, char *argv[], char *env[]) {
	//flag para laço principal
	int flag = 1;
	//comando lido de stdin
	char cmd[255];
	//comando lido
	command app;
	initCommand(&app);
	//struct para manipulação de signals
	struct sigaction signals;
	struct jobs *j;
	struct job *jb;
	
	signals.sa_handler = &signalHandler;
	sigfillset(&signals.sa_mask);
	signals.sa_flags = 0;

	sigaction(SIGCHLD, &signals, NULL);
	sigaction(SIGTSTP, &signals, NULL);
	sigaction(SIGINT, &signals, NULL);
	sigaction(SIGTERM, &signals, NULL);

	//laço principal
	do {
		//inicialização do cmd
		strcpy(cmd, "");
		//força a impressão do buffer de saída
		fflush(stdout);
		//impressão inicial do terminal
		printf("AB2> ");
		//limpeza do stdin
		__fpurge(stdin);
		//leitura do comando
		scanf("%[^\n]", cmd);
		//comando embutido "quit" ou "exit"
		if ((!strcmp(cmd, "quit")) || (!strcmp(cmd, "exit"))) {
			flag = 0;
		//comando embutido "pwd"
		} else if (!strcmp(cmd, "pwd")) {
			printf("%s\n", getcwd(NULL, 0));
		//comando embutido "jobs"
		} else if (!strcmp(cmd, "jobs")) {
			//remove os 'jobs' terminados da lista
			cleanJobs(&jobs);
			if (jobs) {
				printf("Lista de 'jobs':\n");
				j = jobs;
				while (j) {
					printf("\t[%d]", j->job.pid);
					printf("\t%s", (j->job.state == JOB_STOPPED ? "Parado" : (j->job.state == JOB_TERMINATED ? "Terminado" : (j->job.state == JOB_EXEC_FG ? "Executando" : "Executando em plano de fundo"))));
					printf("\t(%s)\n", j->job.application.input);
					j = j->next;
				}
			} else {
				printf("Nenhum 'job' na lista\n");
			}
		//comando embutido "job"
		} else if (!strcmp(cmd, "job")) {
			printf("job_pid: %d\n", job_pid);
		//comando embutido "help"
		} else if (!strcmp(cmd, "help")) {
			printf("AB2 SHELL HELP\n\n");
			printf("Comandos embutidos:\n");
			printf("	help: exibe esta lista\n");
			printf("	about: exibe o 'about' do shell\n");
			printf("	pwd: mostra o diretório corrente\n");
			printf("	cd: troca o diretório corrente\n");
			printf("	quit: termina o shell\n");
			printf("	jobs: lista todos os jobs em background\n");
			printf("	bg <PID>: reinicia um job, passando sua execução para background\n");
			printf("	fg <PID>: reinicia um job, passando sua execução para foreground\n");
		//comando embutido "about"
		} else if (!strcmp(cmd, "about")) {
			printf("AB2 SHELL ABOUT\n\n");
			printf("Shell criada por:\n");
			printf("	André Vinícius Azevedo Aguilar		NUSP: 5890160\n");
			printf("	Eduardo de Freitas Alberice		NUSP: 5890006\n");
			printf("	Flávio Heleno Batista			NUSP: 5890027\n");
		//comandos embutidos com parâmetros ou comandos externos, não vazios
		} else if (strcmp(cmd, "")) {
			parseCommand(cmd, &app, env);
			//comando embutido "cd"
			if (!strcmp(app.argv[0], "cd")) {
				if (app.argc) {
					if (chdir(app.argv[1])) {
						printf("cd: %s: Diretório não encontrado\n", app.argv[1]);
					}
				}
			//comando embutido "bg"
			} else if (!strcmp(app.argv[0], "bg")) {
				if (app.argc == 2) {
					backgroundJob(atoi(app.argv[1]), &jobs);
				} else {
					if (!job_pid) {
						cleanJobs(&jobs);
						jb = lastJob(jobs);
						if (jb) {
							job_pid = jb->pid;
						}
					}
					if (job_pid) {
						backgroundJob(job_pid, &jobs);
						job_pid = 0;
					} else {
						printf("Nenhum 'job' ativo\n");
					}
				}
			//comando embutido "fg"
			} else if (!strcmp(app.argv[0], "fg")) {
				if (app.argc == 2) {
					job_pid = atoi(app.argv[1]);
					foregroundJob(job_pid, &jobs);
				} else {
					if (!job_pid) {
						cleanJobs(&jobs);
						jb = lastJob(jobs);
						if (jb) {
							job_pid = jb->pid;
						}
					}
					if (job_pid) {
						foregroundJob(job_pid, &jobs);
					} else {
						printf("Nenhum 'job' ativo\n");
					}
				}
			//comando embutido "kill"
			} else if (!strcmp(app.argv[0], "kill")) {
				if (app.argc == 2) {
					job_pid = atoi(app.argv[1]);
					terminateJob(job_pid, &jobs);
					printf("\n[%d]\tterminado\t%s\n", job_pid, "");
					job_pid = 0;
				} else {
					if (!job_pid) {
						cleanJobs(&jobs);
						jb = lastJob(jobs);
						if (jb) {
							job_pid = jb->pid;
						}
					}
					if (job_pid) {
						terminateJob(job_pid, &jobs);
						printf("\n[%d]\tterminado\t%s\n", job_pid, "");
						job_pid = 0;
					} else {
						printf("Nenhum 'job' ativo\n");
					}
				}
			//comandos externos
			} else {
				//força a impressão do buffer de saída
				fflush(stdout);
				//cria o novo job
				job_pid = 0;
				job_pid = createJob(app, &jobs);
				if (stateJob(job_pid, jobs) == JOB_TERMINATED) {
					if (resultJob(job_pid, jobs) == JOB_FAILED) {
						printf("%s: comando não encontrado\n", app.argv[0]);
					}
					job_pid = 0;
				}
			}
			freeCommand(&app);
		}
		cleanJobs(&jobs);
	} while (flag);
	if (jobs) {
		killJobs(&jobs);
		cleanJobs(&jobs);
	}
	return 0;
}
