#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *ft_strdup(char *str)
{
    if (!str)
        return 0;
    char *ret = malloc(strlen(str) + 1);
    if (!ret)
        return 0;
    strcpy(ret, str);
    return ret;
}

void    error(char *str)
{
    write(2,  str, strlen(str));
    exit(1);
}

int main(int ac, char **av)
{
    if (ac < 2)
        error("Wrong number of arguments\n");
    char buf[200000];
    char tmp[200000 + 42];
    char *heap;
    char *msg;
    int next_id = 0;
    fd_set active, ready;
    int clientSockets[128 + 3];
    int clientIds[128 + 3];
    int serverSocket = socket(AF_INET, SOCK_STREAM , 0);
    if (serverSocket < 0)
        error("Fatal error\n");
    struct sockaddr_in socketAddress = {0};
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    socketAddress.sin_port = htons(atoi(av[1]));
    if (bind(serverSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
        error("Fatal error\n");
    if (listen(serverSocket, 128) < 0)
        error("Fatal error\n");
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 0;
    FD_ZERO(&active);
    FD_SET(serverSocket, &active);
    int maxSocket = serverSocket;
    while(1)
    {
        ready = active;
        if (select(maxSocket + 1, &ready, 0, 0, &tv) < 0)
            error("Fatal error\n");
        for (int socketId = 0; socketId <= maxSocket; socketId++)
        {
            if (FD_ISSET(socketId, &ready))
            {
                bzero(buf, sizeof(buf));
                bzero(tmp, sizeof(tmp));
                if (socketId == serverSocket)
                {
                    int clientSocket = accept(serverSocket, 0 , 0);
                    if (clientSocket < 0)
                        error("Fatal error\n");
                    FD_SET(clientSocket, &active);
                    clientIds[clientSocket] = next_id + 1;
                    maxSocket = (clientSocket > maxSocket) ? clientSocket : maxSocket;
                    sprintf(tmp, "server: client %d just arrived\n", next_id);
                    for (int i = 0; i <= maxSocket; i++)
                        if (clientSockets[i] != socketId)
                            send(clientSockets[i], tmp, strlen(tmp), 0);
                    clientSockets[clientSocket] = clientSocket;
                    next_id++;
                }
                else
                {
                    int read = recv(socketId, buf, sizeof(buf) - 1, 0);
                    if (read <= 0)
                    {
                        sprintf(tmp, "server: client %d just left\n", clientIds[socketId]);
                        for (int i = 0; i <= maxSocket; i++)
                        {
                            if(clientSockets[i] != socketId)
                                send(clientSockets[i], tmp, strlen(tmp), 0);
                            else
                                clientSockets[i] = -1;
                        }
                        close(socketId);
                        FD_CLR(socketId, &active);
                    }
                    else
                    {
                        buf[read] = 0;
                        while (strlen(buf))
                        {
                        heap = ft_strdup(buf);
                        extract_message(&heap, &msg);
                        sprintf(tmp, "client %d: %s", clientIds[socketId], msg);
                        for (int i = 0; i <= maxSocket; i++)
                            if(clientSockets[i] != socketId)
                                send(clientSockets[i], tmp, strlen(tmp), 0);
                        free(msg);
                        msg = 0;
                        strcpy(buf, heap);
                        free(heap);
                        heap = 0;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
