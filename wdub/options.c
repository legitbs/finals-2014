#include "wdub.h"

uint32_t handle_OPTIONS( int fd, phttp_request phr )
{
    uint32_t retval = 0;
    pstr response = NULL;

    if ( phr == NULL )
    {
        goto end;
    }

    if ( phr->root == NULL )
    {
        DEBUG_PRINT("[ERROR] No root directory.\n");
        goto end;
    }

    response = create_str( (uint8_t*)"HTTP/1.1 200 OK\r\n" );

    if ( response == NULL ) {
        DEBUG_PRINT("[ERROR] Failed to initialize response string\n");
        goto end;
    }

#ifndef VONE
    append_str( response, (uint8_t*)"Allow: OPTIONS,POST,EVAL,GET,HEAD,TRACE,PATCH\r\n");
#else
    append_str( response, (uint8_t*)"Allow: OPTIONS,POST,GET,HEAD,TRACE,PATCH\r\n");
#endif

    AddConnection( response, 0 );
    AddServer( response );
    AddContentLength( response, 0);
    AddSetCookie( response );

#ifndef VONE
    append_str( response, (uint8_t*)"Public: OPTIONS,POST,EVAL,GET,HEAD,TRACE,PATCH\r\n");
#else
    append_str( response, (uint8_t*)"Public: OPTIONS,POST,GET,HEAD,TRACE,PATCH\r\n");
#endif

    if ( append_str( response, (uint8_t*)"\r\n" ) == 0 ) {
        DEBUG_PRINT("[ERROR] Failed to append crlf crlf\n");
        goto end;
    }

    send( fd, response->str, response->max, 0 );

    retval = 1;
end:

    if ( response ) {
        free_str( response );
    }

    return retval;
}

