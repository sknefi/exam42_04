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
int ft_popen(const char *file, char *const argv[], char type)
{
	int		fds[2]; // fds[read, write]
	int		pid;

	if (!file || !argv || (type != 'w' && type != 'r'))
		return (-1);
	if (pipe(fds) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), -1);
	if (pid == 0) //child
	{
		if (type == 'r')
			dup2(fds[1], STDOUT_FILENO);
		else // type == 'w'
			dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
		close(fds[1]);
		execvp(file, argv);
		// end of program (or exit) defaultly closes standard fds that means fd on index 0-2
		// no need to close them manualy
		exit(42); // exit child
	}
	// main process
	if (type == 'r')
		return (close(fds[1]), fds[0]);
	else // type == 'w'
		return (close(fds[0]), fds[1]);
}


/**  === SUBJECT PART (this will be provided on exam) === */
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