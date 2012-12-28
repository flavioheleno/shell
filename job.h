#ifndef __JOB_H__
#define __JOB_H__

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>

#include "command.h"

enum job_result {
	JOB_SUCCESS,
	JOB_FAILED,
	JOB_FORCED,
	JOB_NONE
};

enum job_state {
	JOB_STOPPED,
	JOB_EXEC_FG,
	JOB_EXEC_BG,
	JOB_TERMINATED
};

struct job {
	pid_t pid;
	command application;
	enum job_result result;
	enum job_state state;
};

struct jobs {
	struct job job;
	struct jobs *next;
};

//limpa a lista de 'jobs', removendo 'jobs' já terminados
void cleanJobs(struct jobs **jobs);
//força o fim da execução de todos os 'jobs' existentes
void killJobs(struct jobs **jobs);
//cria um 'job' na lista
pid_t createJob(command app, struct jobs **jobs);
//retorna o último 'job' criado
struct job *lastJob(struct jobs *jobs);
//executa um 'job' existente na lista em foreground
void foregroundJob(pid_t id, struct jobs **jobs);
//executa um 'job' existente na lista em background
void backgroundJob(pid_t id, struct jobs **jobs);
//pausa um 'job' existente na lista, que esteja em execução
void pauseJob(pid_t id, struct jobs **jobs);
//força o fim da execução de um 'job' existente na lista
void terminateJob(pid_t id, struct jobs **jobs);
//destrói um 'job' existente na lista
void destroyJob(pid_t id, struct jobs **jobs);
//retorna o estado atual de um 'job'
enum job_state stateJob(pid_t id, struct jobs *jobs);
//retorna o resultado da saída de um 'job'
enum job_result resultJob(pid_t id, struct jobs *jobs);

#endif
