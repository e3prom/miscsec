#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8980       /* default TCP port to listen to */
#define SPACE   4       /* default output spacing */

/* functions prototypes */
void hexdump(const unsigned char *data_buffer, const unsigned int length,
	     const unsigned int lwidth);
void usage(char *prog_name);

/* output spacing */
const unsigned int space = SPACE;

int main(int argc, char *argv[])
{
    int sockfd, cl_sockfd;		        /* Sockets */

    int af = 0;
    char af_name[5] = "IPv4\0";                 /* default to IPv4 */
    char socktype[5] = "tcp\0";                 /* default to TCP */

    static const uint8_t listen_ip6[16] = { 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00,
					    0x00, 0x00, 0x00, 0x00
                                          };
    char clstr_addr[INET_ADDRSTRLEN];
    char clstr_addr6[INET6_ADDRSTRLEN];

    long int port = PORT;			/* default listen port */
    unsigned int short listen_port;

    struct sockaddr_in host_addr, client_addr;
    struct sockaddr_in6 host_addr6, client_addr6;

    socklen_t sin_size;
    int recv_length=1, yes=1, pid, c, s;
    unsigned char buffer[1024];

    unsigned int lwidth = 16;			/* default line width */


    /* verify argument vector length and if the help is asked. */
    if ((argc < 2) || (!strcmp(argv[argc-1], "--help")))
        usage(argv[0]);

    /* define pointer for input validation using strtol(). */
    char *eptr;

    /* if options are given */
    while ((++argv)[0]) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
                /* IPv4 */
                case '4':
                    strncpy(af_name, "IPv4", sizeof(af_name));
                    af_name[sizeof(af_name)-1] = '\0';
                    break;
                /* IPv6 */
                case '6':
                    if (af == 0)
                        af = 1;
                    strncpy(af_name, "IPv6", sizeof(af_name));
                    af_name[sizeof(af_name)-1] = '\0';
                    break;
                /* Listen port */
                case 'p':
                    if (argv[1] != NULL) {
                        port = strtol(argv[1], &eptr, 10);
                        /* input check */
                        if (*eptr != '\0' || port > USHRT_MAX) {
                            fprintf(stderr, "Error: invalid port number specified.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    break;
                /* Line width */
                case 'w':
                    if (argv[1] != NULL) {
                        lwidth = strtol(argv[1], &eptr, 10);
                        /* display warning if >= 32 char. width */
                        if (lwidth >= 32) {
                           printf("[*] Warning: the specified line width"
                                  " exceed 32 characters, the output may"
                                  " be too large for your terminal size.\n");
                        }
                        /* input check */
                        if (*eptr != '\0' || lwidth > INT_MAX) {
                            fprintf(stderr, "Error: invalid number of lines specified.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    break;
                /* Socket type */
                case 't':
                    if (argv[1] != NULL) {
                        strncpy(socktype, argv[1], sizeof(socktype));
                        socktype[sizeof(socktype)-1] = '\0';
                        if ((strncasecmp("sctp", socktype, 4) == 0) && af == 0) {
                            af = 2;
                        } else if ((strncasecmp("sctp", socktype, 4) == 0) && af == 1) {
                            af = 3;
                        } else if (strncasecmp("tcp", socktype, 3) == 0) {
                        } else {
                           fprintf(stderr, "Unknown specified socket type %s"
                                           " for address family %s.\n", socktype, af_name);
                           exit(EXIT_FAILURE);
                        }
                    }
                    break;
                default:
                    fprintf(stderr, "Unknown option %c\n", argv[0][1]);
                    fprintf(stderr, "Use --help to print help information.\n");
                    exit(EXIT_FAILURE);
                    break;
             }
        }
    }

    /* address-family's specific sockaddr_in structures */
    switch (af) {
        /* IPv4 (TCP) */
        default:
            /* Creating IPv4 TCP socket */
            if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Error while creating TCP socket.\n");
                exit(EXIT_FAILURE);
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
                fprintf(stderr, "error while setting up socket option SO_REUSEADDR.\n");
            /* Setting up IPv4 server's sockaddr_in structure */
            host_addr.sin_family = AF_INET;             /* IPv4 address family */
            host_addr.sin_port = htons(port);           /* Convert port from ASCII to
                                                         * network-byte order. */
            host_addr.sin_addr.s_addr = 0;              /* listen on 0.0.0.0 */
            memset(&(host_addr.sin_zero), '\0', 8);     /* Zero the unused part of the
                                                         * structure. */
            /* bind */
            if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) == -1) {
                fprintf(stderr, "Error while binding to socket.\n");
                exit(EXIT_FAILURE);
            }
            listen_port = ntohs(host_addr.sin_port);
            break;
        /* IPv6 (TCP) */
        case 1:
            /* Creating IPv6 TCP socket */
            if ((sockfd = socket(PF_INET6, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Error while creating TCP socket.\n");
                exit(EXIT_FAILURE);
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
                fprintf(stderr, "error while setting up socket option SO_REUSEADDR.\n");
            /* Setting up IPv6 server's sockaddr_in6 structure */
            host_addr6.sin6_family = AF_INET6;		/* IPv6 address family */
            host_addr6.sin6_port = htons(port);		/* Convert port from ASCII to
    						         * network-byte order. */
            memcpy(host_addr6.sin6_addr.s6_addr, listen_ip6, sizeof(listen_ip6));
            /* bind */
            if (bind(sockfd, (struct sockaddr *)&host_addr6, sizeof(struct sockaddr_in6)) == -1) {
                fprintf(stderr, "Error while binding to socket.\n");
                exit(EXIT_FAILURE);
            }
            listen_port = ntohs(host_addr6.sin6_port);
            break;
        /* IPv4 (SCTP 1:1) */
        case 2:
            /* Creating IPv4 SCTP one-to-one socket */
            if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1) {
                fprintf(stderr, "Error while creating SCTP socket.\n");
                exit(EXIT_FAILURE);
            }
            /* Setting up IPv4 server's sockaddr_in structure */
            host_addr.sin_family = AF_INET;             /* IPv4 address family */
            host_addr.sin_port = htons(port);           /* Convert port from ASCII to
                                                         * network-byte order. */
            host_addr.sin_addr.s_addr = 0;              /* listen on 0.0.0.0 */
            memset(&(host_addr.sin_zero), '\0', 8);     /* Zero the unused part of the
                                                         * structure. */
            /* bind */
            if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr_in)) == -1) {
                fprintf(stderr, "Error while binding to socket.\n");
                exit(EXIT_FAILURE);
            }
            listen_port = ntohs(host_addr.sin_port);
            break;
        /* IPv6 (SCTP 1:1) */
        case 3:
            /* Creating IPv6 SCTP one-to-one socket */
            if ((sockfd = socket(PF_INET6, SOCK_STREAM, IPPROTO_SCTP)) == -1) {
                fprintf(stderr, "Error while creating SCTP socket.\n");
                exit(EXIT_FAILURE);
            }
            /* Setting up IPv6 server's sockaddr_in6 structure */
            host_addr6.sin6_family = AF_INET6;          /* IPv6 address family */
            host_addr6.sin6_port = htons(port);         /* Convert port from ASCII to
                                                         * network-byte order. */
            memcpy(host_addr6.sin6_addr.s6_addr, listen_ip6, sizeof(listen_ip6));
            /* bind */
            if (bind(sockfd, (struct sockaddr *)&host_addr6, sizeof(struct sockaddr_in6)) == -1) {
                fprintf(stderr, "Error while binding to socket.\n");
                exit(EXIT_FAILURE);
            }
            listen_port = ntohs(host_addr6.sin6_port);
            break;
    }

    /* listen */
    if (listen(sockfd, 5) == -1) {
        fprintf(stderr, "Error while listening on socket.\n");
        exit(EXIT_FAILURE);
    }

    printf("[*] SERVER: listening and accepting %s connections on local port %s/%d\n",
           af_name, socktype, listen_port);

    while(1) {
        switch (af) {
            /* IPv4 (TCP) */
            default:
                sin_size = sizeof(struct sockaddr_in);
                /* accept connection from client and create new socket descriptor */
                cl_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
                if (cl_sockfd == -1) {
                    fprintf(stderr, "Error while accepting client connection.\n");
                    break;
                }
                printf("[+] TCP: new client connection from host %s (src port tcp/%d)\n",
                       inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, clstr_addr,
                       INET_ADDRSTRLEN), ntohs(client_addr.sin_port));
                break;
            /* IPv6 (TCP) */
            case 1:
                sin_size = sizeof(struct sockaddr_in6);
                /* accept connection from client and create new socket descriptor */
                cl_sockfd = accept(sockfd, (struct sockaddr *)&client_addr6, &sin_size);
                if (cl_sockfd == -1) {
                    fprintf(stderr, "Error while accepting client connection.\n");
                    break;
                }
                printf("[+] TCP: new client connection from host %s (src port tcp/%d)\n",
                       inet_ntop(AF_INET6, &client_addr6.sin6_addr.s6_addr, clstr_addr6,
                       INET6_ADDRSTRLEN), ntohs(client_addr6.sin6_port));
                break;
            /* IPv4 (SCTP 1:1) */
            case 2:
                sin_size = sizeof(struct sockaddr_in);
                /* accept connection from client and create new socket descriptor */
                cl_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
                if (cl_sockfd == -1) {
                    fprintf(stderr, "Error while accepting client connection.\n");
                    break;
                }
                printf("[+] SCTP: new client connection from host %s (src port sctp/%d)\n",
                       inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, clstr_addr,
                       INET_ADDRSTRLEN), ntohs(client_addr.sin_port));
                break;
            /* IPv6 (SCTP 1:1) */
            case 3:
                sin_size = sizeof(struct sockaddr_in6);
                /* accept connection from client and create new socket descriptor */
                cl_sockfd = accept(sockfd, (struct sockaddr *)&client_addr6, &sin_size);
                if (cl_sockfd == -1) {
                    fprintf(stderr, "Error while accepting client connection.\n");
                    break;
                }
                printf("[+] SCTP: new client connection from host %s (src port sctp/%d)\n",
                       inet_ntop(AF_INET6, &client_addr6.sin6_addr.s6_addr, clstr_addr6,
                       INET6_ADDRSTRLEN), ntohs(client_addr6.sin6_port));
                break;
        }
        /* forking process to handle multiple concurrent connections */
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "An error occured when forking the process.");
            exit(EXIT_FAILURE);
        }
        /* child process */
        if (pid == 0) {
            /* closing parent socket */
            close(sockfd);
            /* send hello and ensure it doesn't return -1 */
            if (send(cl_sockfd, "Hello\n", 6, 0) == -1) {
                fprintf(stderr, "Error while sending hello message.\n");
                return(EXIT_FAILURE);
            }
            /* read from client socket and check for errors. */
            if ((recv_length = recv(cl_sockfd, &buffer, 1024-1, 0)) == -1)
               fprintf(stderr, "Error while reading on client socket.\n");
            /* check for off-by-one error above and ensure the buffer is
             * null-terminated. */
            buffer[recv_length] = '\0';
            /* while there is data from the socket */
            while (recv_length > 0) {
                switch (af) {
                    case 0: printf("[+] RECV: received %d bytes from %s (tcp/%d)\n", recv_length,
				   clstr_addr, client_addr.sin_port); break;
                    case 1: printf("[+] RECV: received %d bytes from %s (tcp/%d)\n", recv_length,
				   clstr_addr6, client_addr6.sin6_port); break;
                    case 2: printf("[+] RECV: received %d bytes from %s (sctp/%d)\n", recv_length,
                                   clstr_addr, client_addr.sin_port); break;
                    case 3: printf("[+] RECV: received %d bytes from %s (sctp/%d)\n", recv_length,
                                   clstr_addr6, client_addr6.sin6_port); break;
                }
                /* print start horizontal bar */
                for (s = 0; s < space; s++) { putchar(32); }
                for (c = 0; c < (lwidth*4)+3; c++) { putchar('-'); }
                printf("\n");
                /* call hexdump() */
                hexdump(buffer, recv_length, lwidth);
                /* print end horizontal bar */
                for (s = 0; s < space; s++) { putchar(32); }
                for (c = 0; c < (lwidth*4)+3; c++) { putchar('-'); }
                printf("\n");
                /* read again from the socket */
                if ((recv_length = recv(cl_sockfd, &buffer, 1024-1, 0)) == -1)
                    fprintf(stderr, "Error while reading on client socket.\n");
                buffer[recv_length] = '\0';
            }
            /* closing client socket */
            close(cl_sockfd);
            switch (af) {
                case 0:
                    printf("[-] TCP: connection from host %s (src port tcp/%d) closed.\n",
                           inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, clstr_addr,
                           INET_ADDRSTRLEN), ntohs(client_addr.sin_port));
                    break;
                case 1:
                    printf("[-] TCP: connection from host %s (src port tcp/%d) closed.\n",
                           inet_ntop(AF_INET6, &client_addr6.sin6_addr.s6_addr, clstr_addr6,
                           INET6_ADDRSTRLEN), ntohs(client_addr6.sin6_port));
                    break;
            }
            /* exit child process */
            exit(EXIT_SUCCESS);
        } else {
            close(cl_sockfd);
        }
    }
    exit(EXIT_SUCCESS);
}

void hexdump(const unsigned char *data_buffer, const unsigned int length, 
             const unsigned int lwidth)
{
    unsigned char byte;
    unsigned int i, j, s;
    unsigned int l = lwidth;

    /* print first line spaces */
    for (s=0; s<space; s++) { putchar(32); }

    for (i=0; i < length; i++) {
        byte = data_buffer[i];
        printf("%02x ", data_buffer[i]);
        /* if at l's value characters or at the end of the buffer */
        if (((i%l)==(l-1)) || (i==length-1)) {
            /* insert space character(s) according to the line width */
            for (j=0; j < (l-1)-(i%l); j++) {
                printf("   ");
            }
            printf(" | ");
            /* re-iterate through every line's characters and output its ASCII
             * equivalent if within range. */
            for (j=(i-(i%l)); j <= i; j++) {
                byte = data_buffer[j];
                if ((byte > 31) && (byte < 127)) {
                    printf("%c", byte);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            if (i!=length-1) {
                for (s=0; s<space; s++) { putchar(32); }
            }
        }
    }
}

void usage(char *prog_name)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t %s -4 [options]\n", prog_name);
    fprintf(stderr, "\t %s -6 [options]\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "\t -p PORT\t TCP port to listen on (default is tcp/8980).\n");
    fprintf(stderr, "\t -w NUM\t\t Hex dump NUM characters at every lines.\n");
    fprintf(stderr, "\t -t PROT\t Use transport-layer protocol PROT"
                    " (see below for supported protocols).\n");
    fprintf(stderr, "\t --help\t\t Print this help information.\n");
    fprintf(stderr, "\nSupported Protocols:\n");
    fprintf(stderr, "\t TCP\t Transmission Control Protocol (default)\n");
    fprintf(stderr, "\t SCTP\t Stream Control Transmission Protocol\n");
    exit(EXIT_SUCCESS);
}
