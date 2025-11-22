#include <unistd.h>
#include <stdlib.h>

/**  === TESTING PART (to test your code before submitting it) === */
#define BUFFER_SIZE 10

char	*ft_strdup(char *src)
{
	char	*dest;
	int		i;

	i = 0;
	while (src[i])
		i++;
	dest = (char *)malloc(sizeof(char) * (i + 1));
	i = 0;
	while (src[i])
	{
	   dest[i] = src[i];
	   i++;
	}
	dest[i] = '\0';
	return (dest);
}

char	*get_next_line(int fd)
{
	static char	buffer[BUFFER_SIZE];
	char		line[70000];
	static int	buffer_read;
	static int 	buffer_pos;
	int			i;

	i = 0;
	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	while (1)
	{
		if (buffer_pos >= buffer_read)
		{
			buffer_read = read(fd, buffer, BUFFER_SIZE);
			buffer_pos = 0;
			if (buffer_read <= 0)
				break ;
		}
		line[i++] = buffer[buffer_pos++];
		if (buffer[buffer_pos] == '\n')
			break ;
	}
	line[i] = '\0';
	if (i == 0)
		return (NULL);
	return (ft_strdup(line));
}

void	ft_putstr(char *str)
{
	int		i;

	i = 0;
	while (str[i])
		i++;
	write(STDOUT_FILENO, str, i);
}

/**  === SUBJECT PART (to pass this subject this is enough) === */
int	ft_popen(const char *file, char *const argv[], char type)
{
	int	fds[2];
	pid_t pid;

	if (!file || !argv || (type != 'r' && type != 'w'))
		return (-1);
	if (pipe(fds) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
	{
		close(fds[0]);
		close(fds[1]);
		return (-1);
	}
	if (pid == 0)
	{
		if (type == 'r')
		{
			if (dup2(fds[1], STDOUT_FILENO) == -1)
				exit(1);
		}
		else
		{
			if (dup2(fds[0], STDIN_FILENO) == -1)
				exit(1);
		}
		close(fds[0]);
		close(fds[1]);
		execvp(file, argv);
		exit(1);
	}
	if (type == 'r')
	{
		close(fds[1]);
		return fds[0];
	}
	else
	{
		close(fds[0]);
		return fds[1];
	}
}

int main()
{
    int  fd;
    char *line;

    fd = ft_popen("ls", (char *const []){"ls", NULL}, 'r');
    while ((line = get_next_line(fd)))
	{
        ft_putstr(line);
		free(line);
	}
    return (0);
}