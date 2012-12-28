#include "job.h"

void cleanJobs(struct jobs **jobs) {
	struct jobs *j;
	j = *jobs;
	while (j) {
		if ((j->job.result == JOB_FAILED) || (j->job.state == JOB_TERMINATED)) {
			destroyJob(j->job.pid, jobs);
		}
		j = j->next;
	}
}

void killJobs(struct jobs **jobs) {
	struct jobs *j;
	j = *jobs;
	while (j) {
		if (j->job.state != JOB_TERMINATED) {
			terminateJob(j->job.pid, jobs);
		}
		j = j->next;
	}
}

void waitJob(struct job *job) {
	int status, child_wait;
	do {
		//aguarda até que o processo filho termine para então liberar o processo principal (mesmo groupid)
		child_wait = waitpid(job->pid, &status, 0);
		if ((job->state == JOB_EXEC_FG) || (job->state == JOB_EXEC_BG)) {
			if (child_wait == job->pid) {
				if (!status) {
					job->result = JOB_SUCCESS;
				} else {
					job->result = JOB_FAILED;
				}
				job->state = JOB_TERMINATED;
			}
		}
	} while (job->state == JOB_EXEC_FG);
}

//função que executa o aplicativo externo
void execCommand(struct job *job) {
	int e = 0;
	//struct para manipulação de signals
	struct sigaction signals;
	//cria um processo filho pra executar o aplicativo externo
	job->pid = fork();
	//executa o processo filho
	if (job->pid == 0) {
		setpgid(0, 0);
	
		signals.sa_handler = SIG_DFL;
		sigfillset(&signals.sa_mask);
		signals.sa_flags = 0;
		//restaura a configuração de signals
		sigaction(SIGCHLD, &signals, NULL);
		sigaction(SIGTSTP, &signals, NULL);
		sigaction(SIGINT, &signals, NULL);
		sigaction(SIGTERM, &signals, NULL);

		//executa o comando do usuário
		e = execve(job->application.argv[0], job->application.argv, job->application.env);
		//se o retorno foi -1, significa que o comando não foi encontrado no PATH do sistema
		if (e == -1) {
			job->result = JOB_FAILED;
			job->state = JOB_TERMINATED;
		}
	//erro no fork()
	} else if (job->pid < 0) {
		exit(EXIT_FAILURE);
	//processo pai
	} else {
		if (job->application.foreground) {
			waitJob(job);
		}
	}
}

struct job *selectJob(pid_t id, struct jobs *jobs) {
	struct jobs *j;
	int flag = 0;
	j = jobs;
	while ((!flag) && (j)) {
		if (j->job.pid == id) {
			flag = 1;
		} else {
			j = j->next;
		}
	}
	if (flag) {
		return &j->job;
	} else {
		return NULL;
	}
}

pid_t createJob(command app, struct jobs **jobs) {
	struct jobs *n;
	n = (struct jobs *)malloc(sizeof(struct jobs));
	initCommand(&n->job.application);
	copyCommand(&n->job.application, app);
	//parseCommand(app.input, &n->job.application, app.env);
	//se um path completo não foi especificado, busca pelos diretórios do sistema
	if (!(strchr(n->job.application.argv[0], '/'))) {
		findCommand(&n->job.application, getcwd(NULL, 0));
	}
	n->job.result = JOB_NONE;
	if (app.foreground) {
		n->job.state = JOB_EXEC_FG;
	} else {
		n->job.state = JOB_EXEC_BG;
	}
	n->next = *jobs;
	*jobs = n;
	execCommand(&n->job);
	return n->job.pid;
}

struct job *lastJob(struct jobs *jobs) {
	if (jobs) {
		return &jobs->job;
	} else {
		return NULL;
	}
}

void foregroundJob(pid_t id, struct jobs **jobs) {
	struct job *job;
	//printf("foregroundJob(%d)\n", id);
	job = selectJob(id, *jobs);
	if (job) {
		if (job->state != JOB_TERMINATED) {
			printf("job: %s\n", job->application.input);
			if (job->state == JOB_STOPPED) {
				kill(job->pid, SIGCONT);
			}
			job->state = JOB_EXEC_FG;
			waitJob(job);
		}
	}
}

void backgroundJob(pid_t id, struct jobs **jobs) {
	struct job *job;
	//printf("backgroundJob(%d)\n", id);
	job = selectJob(id, *jobs);
	if (job) {
		if (job->state != JOB_TERMINATED) {
			printf("job: %s\n", job->application.input);
			if (job->state == JOB_STOPPED) {
				kill(job->pid, SIGCONT);
			}
			job->state = JOB_EXEC_BG;
		}
	}
}

void pauseJob(pid_t id, struct jobs **jobs) {
	struct job *job;
	//printf("pauseJob(%d)\n", id);
	job = selectJob(id, *jobs);
	//if ((job) && (job->state == JOB_EXEC_FG)) {
	if (job) {
		if (job->state != JOB_TERMINATED) {
			job->state = JOB_STOPPED;
			kill(job->pid, SIGTSTP);
		}
	}
}

void terminateJob(pid_t id, struct jobs **jobs) {
	struct job *job;
	//printf("terminateJob(%d)\n", id);
	job = selectJob(id, *jobs);
	if (job) {
		if (job->state != JOB_TERMINATED) {
			job->result = JOB_FORCED;
			job->state = JOB_TERMINATED;
			kill(job->pid, SIGQUIT);
		}
	}
}

void destroyJob(pid_t id, struct jobs **jobs) {
	struct jobs *j, *a = NULL;
	int flag = 0;
	j = *jobs;
	while ((!flag) && (j)) {
		if (j->job.pid == id) {
			if (a) {
				a->next = j->next;
			} else {
				*jobs = j->next;
			}
			freeCommand(&j->job.application);
			free(j);
			flag = 1;
		} else {
			a = j;
			j = j->next;
		}
	}
}

enum job_state stateJob(pid_t id, struct jobs *jobs) {
	struct job *job;
	job = selectJob(id, jobs);
	if (job) {
		return job->state;
	} else {
		return -1;
	}
}

enum job_result resultJob(pid_t id, struct jobs *jobs) {
	struct job *job;
	job = selectJob(id, jobs);
	if (job) {
		return job->result;
	} else {
		return -1;
	}
}
