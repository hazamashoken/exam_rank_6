#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        write(2, "Wrong number of arguments\n", 27);
        exit (1);
    }
    int clientSockets[128];
    int next_id = 0;
    fd_set activeSockets, readySockets;
    char buffer[200000];
    char *heapbuf;
    char *msg;
    char tmp[200000 + 42];
    int id = 1;
    int clientid[128 + 3];
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        write(2, "Fatal error\n", 13);
        exit(1);
    }
    struct sockaddr_in serverAddress = {0};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(atoi(argv[1]));
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        write(2, "Fatal error\n", 13);
        exit(1);
    }
    if (listen(serverSocket, 128) < 0)
    {
        write(2, "Fatal error\n", 13);
        exit(1);
    }
    FD_ZERO(&activeSockets);
    FD_SET(serverSocket, &activeSockets);
    int maxSocket = serverSocket;
    while (1)
    {
        readySockets = activeSockets;
        if (select(maxSocket + 1, &readySockets, NULL, NULL, NULL) < 0)
        {
            write(2, "Fatal error\n", 13);
            exit(1);
        }
        for (int socketId = 0; socketId <= maxSocket; socketId++)
        {
            if (FD_ISSET(socketId, &readySockets))
            {
                bzero(buffer, sizeof(buffer));
                bzero(tmp, sizeof(tmp));
                if (socketId == serverSocket)
                {
                    int clientSocket = accept(serverSocket, NULL, NULL);
                    if (clientSocket < 0)
                    {
                        write(2, "Fatal error\n", 13);
                        exit(1);
                    }
                    FD_SET(clientSocket, &activeSockets);
                    clientid[clientSocket] = id++;
                    maxSocket = (clientSocket > maxSocket) ? clientSocket : maxSocket;
                    sprintf(buffer, "server: client %d just arrived\n", id - 1);
                    for (int i = 0; i < next_id; i++)
                        if (clientSockets[i] != socketId)
                            send(clientSockets[i], buffer, strlen(buffer), 0);
                    clientSockets[next_id++] = clientSocket;
                }
                else
                {
                    int bytesRead = recv(socketId, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead <= 0)
                    {
                        sprintf(tmp, "server: client %d just left\n", clientid[socketId]);
                        for (int i = 0; i < next_id; i++)
                        {
                            if (clientSockets[i] != socketId)
                                send(clientSockets[i], tmp, strlen(tmp), 0);
                            else
                                clientSockets[i] = -1;
                        }
                        close(socketId);
                        FD_CLR(socketId, &activeSockets);
                    }
                    else
                    {
                        buffer[bytesRead] = '\0';
                        while (strlen(buffer))
                        {
                            heapbuf = ft_strdup(buffer);
                            extract_message(&heapbuf, &msg);
                            sprintf(tmp, "client %d: %s", clientid[socketId], msg);
                            for (int i = 0; i < next_id; i++)
                                if (clientSockets[i] != socketId)
                                    send(clientSockets[i], tmp, strlen(tmp), 0);
                            free(msg);
                            msg = NULL;
                            strcpy(buffer, heapbuf);
                            free(heapbuf);
                            heapbuf = NULL;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
