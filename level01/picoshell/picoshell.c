#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int    picoshell(char **cmds[])
{
	int		fds[2];
	int		pid;
	int		prev_fd = -1;
	int		i = 0;

	while (cmds[i])
	{
		if (cmds[i + 1] && pipe(fds) == -1)
			return (1);

		pid = fork();
		if (pid == 0) // child
		{
			// if not first cmd
			if (prev_fd != -1)
			{
				dup2(prev_fd, STDIN_FILENO);
				close(prev_fd);
			}

			// if not last cmd
			if (cmds[i + 1])
			{
				close(fds[0]);
				dup2(fds[1], STDOUT_FILENO);
				close(fds[1]);
			}
			execvp(cmds[i][0], cmds[i]);
			exit(1); // fallback
		}
		// if not first cmd
		if (prev_fd != -1)
			close(prev_fd);

		// if not last cmd
		if (cmds[i + 1])
		{
			close(fds[1]);
			prev_fd = fds[0];
		}
		i++;
	}

	int		status;
	int		exit_code = 0;
	while (wait(&status) != -1)
	{
		if (WIFEXITED(status) && WIFSIGNALED(status) != 0)
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
