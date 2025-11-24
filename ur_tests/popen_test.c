// add includes, if u forgot use man in terminal (for example man dup2)


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

int ft_popen(const char *file, char *const argv[], char type)
{
	// write ur popen logic here

}


/**  === SUBJECT PART (this will be provided on exam) === */
#include <unistd.h>
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