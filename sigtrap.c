/*
 * Do nothing until the specified signal is received, then exec to the specified
 * command
 *
 * Usage: sigtrap <signal> <command> [arguments...]
 *
 * Supported signals: USR1, USR2, HUP, INT, ILL, BUS, FPE, URG, PWR, SYS
 *
 * Compile with: gcc -o sigtrap sigtrap.c -liberty
 *
 * Author: Taylor Hedberg <tmhedberg@gmail.com>
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libiberty.h>

static char **cmd_argv;

void handle(const int sig)
{
	static const char *err_msg_pre = "Couldn't exec to ";

	char *err_msg;
	sigset_t *set;

	printf("Received signal %d\n", sig);

	set = malloc(sizeof (sigset_t));
	sigemptyset(set);
	if (sigaddset(set, sig)) {
		perror("Couldn't add signal to unblock set");
		exit(EXIT_FAILURE);
	}
	if (sigprocmask(SIG_UNBLOCK, set, NULL)) {
		perror("Couldn't modify signal mask");
		exit(EXIT_FAILURE);
	}

	execvp(cmd_argv[0], cmd_argv);

	err_msg = malloc(strlen(err_msg_pre) + strlen(cmd_argv[0]));
	strcpy(err_msg, err_msg_pre);
	strcat(err_msg, cmd_argv[0]);
	perror(err_msg);
	exit(EXIT_FAILURE);
}

int main(const int argc, char *argv[])
{
	struct sigaction *sact;
	char *signame;
	size_t signame_len;
	int signum;
	char *str;

	if (argc < 3) {
		fprintf(stderr, "Usage: sigtrap <signal> <command> [arguments...]\n");
		exit(EXIT_FAILURE);
	}

	for (str = argv[1]; *str != '\0'; ++str)
		*str = toupper(*str);
	if (!strcmp(argv[1], "USR1"))
		signum = SIGUSR1;
	else if (!strcmp(argv[1], "USR2"))
		signum = SIGUSR2;
	else {
		signame_len = sizeof argv[1] + 3;
		signame = malloc(signame_len);
		snprintf(signame, signame_len, "SIG%s", argv[1]);
		signum = strtosigno(signame);
		free(signame);
	}
	cmd_argv = &argv[2];

	sact = malloc(sizeof (struct sigaction));
	sact->sa_handler = handle;
	sigemptyset(&sact->sa_mask);
	sact->sa_flags = SA_RESETHAND;
	if (sigaction(signum, sact, NULL)) {
		perror("Couldn't register signal handler");
		exit(EXIT_FAILURE);
	}
	free(sact);

	printf("Waiting for SIG%s...\n", argv[1]);
	pause();
}
