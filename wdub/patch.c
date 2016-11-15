#include "wdub.h"

uint32_t handle_PATCH( int fd, phttp_request phr )
{
    uint32_t retval = 0;
    uint32_t length = 0;
    uint8_t *file = NULL;
    uint32_t urllen = 0;
    pstr of = NULL;
    uint8_t *out = NULL;
    pstr response = NULL;
    pstr outdata = NULL;
    uint32_t start = 0;

    if ( phr == NULL ) {
        goto end;
    }

    if ( phr->root == NULL ) {
        DEBUG_PRINT("[ERROR] No root directory.\n");
        goto end;
    }

    if ( phr->url == NULL ) {
        goto end;
    }

    if ( phr->content_data == NULL ) {
        goto end;
    }

    if ( phr->content_length == NULL ) {
        goto end;
    }

    outdata = malloc( sizeof(str) );

    if ( outdata == NULL ) {
        goto end;
    }

    if ( phr->url[0] == '/' && ( phr->url[1] == ' ' || phr->url[1] == '\x00') ) {
        phr->url = (uint8_t*)"/index.html";
    }

    while ( phr->root[length] != 0 ) {
        length++;
    }

    while ( phr->url[urllen] != 0x00 ) {
        urllen++;
    }

    // The + two is for the possibility of adding a '/'
    //      plus the final null byte
    file = (uint8_t*)malloc( length + urllen + 2 );

    if ( file == NULL ) {
        goto end;
    }

    memset( file, 0x00, length + urllen + 2);

    if ( memcmp( phr->url, "/index.html", urllen ) != 0 
#ifndef VONE
            && memcmp( phr->url, "/index.ydg", urllen ) != 0 
#endif
       ) {
        send_error( fd, 415);
        goto end;
    }

    // Copy the root directory
    memcpy( file, phr->root, length );

    // check for starting '/'
    if (phr->url[0] != '/' ) {
        file[length] = '/';
        length++;
    }

    memcpy( file + length, phr->url, urllen );

    of = read_file( file );

    if ( of == NULL ) {
        send_error( fd, 404 );
        goto end;
    }

    start = 0;

    if ( phr->xoffset != NULL ) {
        start = catoi( phr->xoffset );
    }

    urllen = catoi( phr->content_length );
    if ( start + urllen > of->max ) {
        urllen += start;
    } else {
        urllen = of->max;
    }

    out = alloca( urllen );

    outdata = init_str( outdata, out, urllen );

    if ( outdata == NULL ) {
        goto end;
    }

    // Write the initial data
    if ( overwrite_data( outdata, of->str, 0, of->max ) == 0 ) {
        goto end;
    }

    if ( overwrite_data( outdata, phr->content_data, start, catoi(phr->content_length)) == 0 ) {
        send_error( fd, 400 );
        goto end;
    }

    if ( write_file( outdata, file ) == 0 ) {
        send_error( fd, 400 );
        goto end;
    }

    response = create_str( (uint8_t*)"HTTP/1.1 200 OK\r\n" );

    if ( response == NULL ) {
        DEBUG_PRINT("[ERROR] Failed to initialize response string\n");
        goto end;
    }

    AddConnection( response, 0 );
    AddServer( response );
    AddSetCookie( response );

    if ( append_str( response, (uint8_t*)"\r\n" ) == 0 ) {
        DEBUG_PRINT("[ERROR] Failed to append crlf crlf\n");
        goto end;
    }

    send( fd, response->str, response->max, 0 );

    retval = 1;
end:
    if ( outdata ) {
        free(outdata);
    }

    if ( file ) {
        free( file );
        file = NULL;
    }

    if ( of ) {
        free_str( of );
    }

    if ( response ) {
        free_str( response );
    }

    return retval;
}

