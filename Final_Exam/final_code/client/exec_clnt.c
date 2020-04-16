
#include <memory.h>
#include "exec.h"


static struct timeval TIMEOUT = { 25, 0 };

char **
exec_string_1(char **argp, CLIENT *clnt)
{
	static char *clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, EXEC_STRING,
		(xdrproc_t) xdr_wrapstring, (caddr_t) argp,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}



int readline(char** buffer, size_t n) {
    int res = getline(buffer, &n, stdin);
    (*buffer)[res-1] = 0;
    return res;
}

char **command;
char **result;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Need 1 arg: host ip of server\n");
        return 0;
    }



    CLIENT *client;

    client = clnt_create(argv[1], EXECSH, EXECSH_V1, "tcp");
    if (client == NULL) {
        printf("error: can't connect to server\n");
        return -1;
    }


    printf("Connection ready. Please type shell commands\n");
    command = malloc(8);
    size_t buffer_size = 1024;

    while (1) {
        printf(">");
        readline(command, buffer_size);
        if (strcmp(*command,"exit")==0) break;

        result = exec_string_1(command, client);
        if (result==NULL) {
            printf("error: RPC execution failed. Please try again\n");
            continue;
        }
        printf("%s\n",*result);
    }

    return 0;
}
