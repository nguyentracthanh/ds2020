

#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

static void
execsh_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *exec_string_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case EXEC_STRING:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_wrapstring;
		local = (char *(*)(char *, struct svc_req *)) exec_string_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}

/*****************************/

const int buffer_size = 1024;
char** res;       
int res_size;     
int res_capacity; 

void init()
{
    res = malloc(8);     
    *res = malloc(buffer_size + 1); 
    res_size = 0;
    res_capacity = buffer_size;
}

char** exec_string_1_svc(char **command, struct svc_req *req)
{
    printf("Client ask: %s\n",*command);

    char buffer[buffer_size];
    res_size = 0;

    FILE* f = popen(*command, "r");
    while (1) {
        int bytesRead = fread(buffer, 1, buffer_size, f);
        if (bytesRead<=0) break;

        if (res_size + bytesRead > res_capacity) {
            *res = realloc(*res, res_capacity*2 + 1);
            res_capacity *= 2;
        }

        memcpy((*res)+res_size, buffer, bytesRead);
        res_size += bytesRead;
    }
    (*res)[res_size] = 0;

    fclose(f);
    return res;
}

int
main (int argc, char **argv)
{
    init();
	register SVCXPRT *transp;

	pmap_unset (EXECSH, EXECSH_V1);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, EXECSH, EXECSH_V1, execsh_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (EXECSH, EXECSH_V1, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, EXECSH, EXECSH_V1, execsh_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (EXECSH, EXECSH_V1, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}
