#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

/**  === SUBJECT PART (to pass this subject this is enough) === */
int picoshell(char **cmds[])
{
	pid_t   pid;
	int     fds[2];
	int     status;
	int     prev_fd		= -1; // previous read fd (pipe read end -> fds[0])
	int     exit_code 	= 0;
	int     i 			= 0;

	while (cmds[i])
	{
		if (cmds[i + 1] && pipe(fds) == -1)
			return (1);

		pid = fork();
		if (pid == 0) // child
		{
			// If not first command then read from prev pipe
			if (prev_fd != -1)
			{
				if (dup2(prev_fd, STDIN_FILENO) == -1)
					exit(1);
				close(prev_fd);
			}

			// If not last command then write to next pipe
			if (cmds[i + 1])
			{
				close(fds[0]);
				if (dup2(fds[1], STDOUT_FILENO) == -1)
					exit(1);
				close(fds[1]);
			}

			execvp(cmds[i][0], cmds[i]); // this should exit with execution exit status
			exit(1); // fallback - error cmd not found
		}

		// parent
		if (prev_fd != -1)
			close(prev_fd);
		if (cmds[i + 1])
		{
			close(fds[1]);
			prev_fd = fds[0];
		}

		i++;
	}

	while (wait(&status) != -1) // while there is a child
	{
		// WIFEXITED	- bool: terminated normally, used exit() or _exit()
		// WEXITSTATUS	- int: returns exit status that was used to terminate program
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			exit_code = 1;
	}

	return (exit_code);
}

/**  === TESTING PART (to test your code before submitting it) === */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static int	count_cmds(int argc, char **argv)
{
	int	count = 1;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "|") == 0)
			count++;
	}
	return (count);
}

int	main(int argc, char **argv)
{
	if (argc < 2)
		return (fprintf(stderr, "Usage: %s cmd1 [args] | cmd2 [args] ...\n", argv[0]), 1);

	int	cmd_count = count_cmds(argc, argv);
	char	***cmds = calloc(cmd_count + 1, sizeof(char **));
	if (!cmds)
		return (perror("calloc"), 1);

	int	i = 1, j = 0;
	while (i < argc)
	{
		int	len = 0;
		while (i + len < argc && strcmp(argv[i + len], "|") != 0)
			len++;
		cmds[j] = calloc(len + 1, sizeof(char *));
		if (!cmds[j])
			return (perror("calloc"), 1);
		for (int k = 0; k < len; k++)
			cmds[j][k] = argv[i + k];
		cmds[j][len] = NULL;
		i += len + 1;
		j++;
	}
	cmds[cmd_count] = NULL;

	int	ret = picoshell(cmds);

	// Clean up
	for (int i = 0; cmds[i]; i++)
		free(cmds[i]);
	free(cmds);

	return (ret);
}
