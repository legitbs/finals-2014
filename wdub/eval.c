#include "wdub.h"
#include <zlib.h>

#define CHUNK 0x4000
#define GZIP 16

uint32_t handle_EVAL( int fd, phttp_request phr )
{
    uint32_t retval = 0;
    uint8_t outbuff[0x200];
    uint32_t counter = 0;
    uint32_t cl = 0;
    pstr response = NULL;
    uLongf urllen = 0;
    uint8_t url[0x40];
    uint8_t uncomp[0x1000];
    uint8_t *evalme = NULL;
    int32_t err = 0;

    if ( phr == NULL ) {
        goto end;
    }

    if ( phr->root == NULL ) {
        goto end;
    }

    if ( phr->url == NULL ) {
        goto end;
    }

    urllen = strlen( ( const char * )phr->url ) % 0x40;

    if ( phr->content_data == NULL ) {
        goto end;
    }

    memset( outbuff, 0x00, 0x200);
    memcpy( url, outbuff, 0x40);
    memcpy( url, phr->url, urllen );
    memset( uncomp, 0, 0x1000 );

    cl = catoi( phr->content_length);

    evalme = phr->content_data;

    if ( phr->content_encoding ) {
        urllen = 0x1000;
        if ( memcmp( phr->content_encoding, "compress", 8 ) == 0 ) {
            err = uncompress( uncomp, &urllen, phr->content_data, cl);
            if ( err != Z_OK ) {
                send_error( fd, 493 );
                goto end;
            }
            evalme = uncomp;   
        }
    } else {
        urllen = cl;
    }

    DEBUG_PRINT("[+] About to eval:\n%s\n", evalme );

    /// Perhaps ensure that the entire script is ascii
    urllen = runscript( evalme, urllen, outbuff );

    if (  urllen != 0xffffffff ) {
        DEBUG_PRINT("[+] ");
        while ( counter < urllen ) {
            DEBUG_PRINT("%x ", outbuff[counter]);
            counter++;
        }
        DEBUG_PRINT("\n\n");        
    } else {
        send_error( fd, 492 );
        goto end;
    }

    response = create_str( (uint8_t*)"HTTP/1.1 200 OK\r\n" );

    if ( response == NULL ) {
        DEBUG_PRINT("[ERROR] Failed to initialize response string\n");
        goto end;
    }

    AddConnection( response, 0 );
    AddContentType( response, url );
    AddServer( response );
    AddContentLength( response, urllen );
    AddSetCookie( response );

    if ( append_str( response, (uint8_t*)"\r\n" ) == 0 ) {
        DEBUG_PRINT("[ERROR] Failed to append crlf crlf\n");
        goto end;
    }

    if ( append_data( response, outbuff, urllen ) == 0 ) {
        DEBUG_PRINT("[ERROR] Failed to append yodog output\n");
        goto end;
    }

    DEBUG_PRINT("About to send %d bytes\n", response->max);
    send( fd, response->str, response->max, 0 );

    retval = 1;

end:
    if ( response ) {
        free_str( response );
    }

    return retval;
}

