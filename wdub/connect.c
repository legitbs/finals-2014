#include "wdub.h"

uint8_t *connect_response = (uint8_t *)"HTTP/1.1 200 Connection established\r\nProxy-Agent: Yodogz\r\n\r\n";

// This function assumes that select() returned 0 and
//    the fd is in non-blocking mode.
// I guess there is the slight possibility that between the select
//    return and this call some data could have come over the socket
//    and you will lose a byte. Thankfully this isn't production software.
uint32_t is_closed( int fd )
{
    register uint32_t retval = 0;
    uint8_t ch;

    if ( recv( fd, &ch, 1, MSG_PEEK ) <= 0 ) {
        retval = 1;
    }

    return retval;
}

uint32_t handle_CONNECT( int fd, phttp_request phr )
{
    uint32_t retval = 0;
    uint32_t length = 0;
    uint32_t port = 80;
    uint8_t *pst = NULL;
    uint8_t *dest = NULL;
    struct hostent *hd = NULL;
    int tgtfd = 0;
    struct sockaddr_in sin;
    fd_set fdl;
    int outfd = 0;
    int nfds = 0;
    char buffer[1024];
    struct timeval tv;

    if ( phr == NULL ) {
        goto end;
    }

    if ( phr->url == NULL ) {
        goto end;
    }

    memset( &sin, 0x00, sizeof( struct sockaddr_in ));
    FD_ZERO( &fdl );

    while ( phr->url[length] != '\x00' && phr->url[length] != ' ' && phr->url[length] != ':' ) {
        length++;
    }

    if ( phr->url[length] == ':' ) {
        phr->url[length] = '\x00';
        length++;
        pst = phr->url + length;

        while ( phr->url[length] != ' ' && phr->url[length] != '\x00' ) {
            length++;
        }

        phr->url[length] = '\x00';

        port = atoi( (const char*)pst );    
        if ( port == 0 || port > 65535 ) {
            goto end;
        }
    } else {
        phr->url[length] = '\x00';
    }

    dest = phr->url;

    hd = gethostbyname( (const char*)dest );

    if ( hd == NULL ) {
        DEBUG_PRINT("[-] gethostbyname failed for %s\n", dest );
        goto end;
    }
    

    DEBUG_PRINT("[+] About to connect to %s port: %d\n", dest, port);

    tgtfd = socket( AF_INET, SOCK_STREAM, 0 );

    if ( tgtfd < 0 ) {
        DEBUG_PRINT("[-] Failed to initialize socket\n");
        goto end;
    }

    sin.sin_family = AF_INET;
    memcpy( &sin.sin_addr.s_addr, &hd->h_addr[0], hd->h_length);
    sin.sin_port = htons(port);

    if ( connect( tgtfd, (struct sockaddr *)&sin, sizeof(sin) ) < 0 ) {
        DEBUG_PRINT("[-] Failed to connect to %s\n", dest );
        goto end;
    }

    nfds = tgtfd;

    if ( nfds < fd ) {
        nfds = fd;
    }

    nfds++;

    write( fd, (char*)connect_response, strlen((const char*)connect_response));

    fcntl( tgtfd, F_SETFL, O_NONBLOCK);
    fcntl( fd, F_SETFL, O_NONBLOCK);

    DEBUG_PRINT("[+] Socket opened: %d\n", tgtfd);
        
    while ( 1 ) {
        FD_SET( fd, &fdl);
        FD_SET( tgtfd, &fdl);

        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        outfd = select( nfds, &fdl, NULL, NULL, &tv );
        DEBUG_PRINT("[+] outfd: %d\n", outfd);

        if ( outfd == 0 ) {
            if ( is_closed( fd ) ) {
                goto end;
            }
            if ( is_closed(tgtfd) ) {
                goto end;
            }
            continue;
        }

        if ( outfd < 0 ) {
            goto end;
        }

        memset( buffer, 0x00, 1023);
        if ( FD_ISSET( fd, &fdl ) ) {
            length = recv( fd, buffer, 1023, 0 );
            outfd = tgtfd;
        } else if ( FD_ISSET ( tgtfd, &fdl ) ) {
            length = recv( tgtfd, buffer, 1023, 0 );
            outfd = fd;
        } else {
            goto end;
        }

        if ( length <= 0 ) {
            goto end;
        }

        DEBUG_PRINT("[+] Writing %d\n", length);
        write( outfd, buffer, length );
                
    }

    retval = 1;
    
end:
    DEBUG_PRINT("[+] Socket closed: %d\n", tgtfd);
    close( tgtfd );
    return retval;
}

