#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>

#define SERVER_PORT 21

char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int main(int argc, char *argv[])
{

    // RECEBE E SEPARA OS PARAMETROS DO URL
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s http://username:password@ftp.example.com/path/to/file.txt\n", argv[0]);
        exit(-1);
    }
    char url[] = "ftp://username:password@ftp.example.com/path/to/file.txt/sarampo";
    char protocol[6], username[256], password[256], hostname[256], path[1024], ficheiro[256];
    int porta = 21;

    if (strstr(argv[1], "@") != NULL)
    {
        sscanf(argv[1], "%5[^:]://%255[^:]:%255[^@]@%255[^/]/%1023[^\n]", protocol, username, password, hostname, path);
        printf("Username: %s\n", username);
        printf("Password: %s\n", password);
        printf("Hostname: %s\n", hostname);
        printf("Path: %s\n", path);
    }

    else
    {
        sscanf(argv[1], "%5[^:]://%255[^/]/%1023[^\n]", protocol, hostname, path);
        printf("Hostname: %s\n", hostname);
        printf("Path: %s\n", path);
        strcpy(username, "anonymous");
        strcpy(password, "anonymous");
    }

    struct hostent *h = gethostbyname(hostname);

    if (h == NULL)
    {
        herror("gethostbyname");
        exit(-1);
    }
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));

    int sockfd, sockdt, rc;
    struct sockaddr_in server_addr, data_addr;
    char response[4020];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting");
        exit(1);
    }

    while (recv(sockfd, response, sizeof(response), 0))
    {
        printf("%s\n", response);
        if (strstr(response, "220 "))
        {
            break;
        }
    }

    char user[256];
    strcpy(user, "USER ");
    strcat(user, username);
    strcat(user, "\r\n");
    char autentica[100];
    send(sockfd, user, strlen(user), 0);
    recv(sockfd, autentica, sizeof(autentica), 0);
    printf(">>%s", user);
    printf("%s\n", autentica);

    char login[256];
    char pass[256];
    strcpy(pass, "PASS ");
    strcat(pass, password);
    strcat(pass, "\r\n");
    send(sockfd, pass, strlen(pass), 0);
    recv(sockfd, login, sizeof(pass), 0);
    printf(">>%s", pass);
    printf("%s\n", login);

    char cd[100];
    const char ch = '/';
    char *ret;
    ret = strrchr(path, ch);
    ret++;
    printf("%s\n", ret);

    strcpy(ficheiro, ret); // <-- ADD THIS LINE

    char **tokens;
    char caminho[256];
    tokens = str_split(path, '/');
    if (tokens)
    {
        int i;

        for (i = 0; *(tokens + i); i++)
        {
            if (strcmp(ret, *(tokens + i)) == 0)
            {
                break;
            }
            strcpy(caminho, "CWD ");
            strcat(caminho, *(tokens + i));
            strcat(caminho, "\r\n");
            send(sockfd, caminho, strlen(caminho), 0);
            recv(sockfd, cd, sizeof(cd), 0);
            printf(">>%s", caminho);
            printf("%s\n", cd);
            free(*(tokens + i));
            memset(caminho, 0, strlen(caminho));
        }
        printf("\n");
        free(tokens);
    }

    char type[256];
    send(sockfd, "TYPE I\r\n", 8, 0);
    recv(sockfd, type, sizeof(type), 0);
    printf(">>TYPE I\r\n");
    printf("%s\n", type);

    char size[256];
    strcpy(size, "SIZE ");
    strcat(size, ret);
    strcat(size, "\r\n");

    char tamanho[256];
    send(sockfd, size, strlen(size), 0);
    recv(sockfd, tamanho, sizeof(tamanho), 0);
    printf(">>%s", size);
    printf("%s\n", tamanho);

    char passmode[256];
    send(sockfd, "PASV\r\n", 6, 0);
    recv(sockfd, passmode, sizeof(passmode), 0);
    printf(">>PASV\r\n");
    printf("%s\n", passmode);
    char reter[256];
    strcpy(reter, "RETR ");
    strcat(reter, ret);
    strcat(reter, "\r\n");

    int ip1, ip2, ip3, ip4, porta1, porta2;
    sscanf(passmode, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &porta1, &porta2);
    int portafinal = porta1 * 256 + porta2;
    printf("Porta=%i\n", portafinal);
    send(sockfd, reter, strlen(reter), 0);
    printf(">>%s", reter);

    sockdt = socket(AF_INET, SOCK_STREAM, 0);
    if (sockdt == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
    data_addr.sin_port = htons(portafinal);

    if (connect(sockdt, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
    {
        perror("Error connecting");
        exit(1);
    }

    char download[100];

    FILE *fp = fopen(ficheiro, "wb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    char buffer[4096];
    int num_bytes = 0;
    while ((num_bytes = recv(sockdt, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, num_bytes, fp);
    }

    fclose(fp);
    char fecha[100];
    recv(sockfd, fecha, sizeof(fecha), 0);
    printf("%s\n", fecha);
    char fecha2[100];
    recv(sockfd, fecha2, sizeof(fecha), 0);
    printf("%s\n", fecha2);

    if (close(sockdt) < 0)
    {
        perror("close()");
        exit(-1);
    }

    char adeus[100];
    send(sockfd, "QUIT\r\n", 6, 0);
    recv(sockfd, adeus, sizeof(adeus), 0);
    printf(">>quit\r\n");
    printf("%s\n", adeus);
    if (close(sockfd) < 0)
    {
        perror("close()");
        exit(-1);
    }

    return 0;
}
