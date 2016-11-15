#include "wdub.h"

int port = 4444;


const char *iface = "eth0";
const char *name = "wdub";

uint8_t *error_404 = (uint8_t*)"HTTP/1.1 999 Shits Broken\r\nContent-Type: text/html\r\nCharset: utf-8\r\nConnection: keep-alive\r\nContent-Length: 98\r\n\r\n<html><head><title>ERROR 999</title></head><body>somethingwittyhere</body></html>\r\n";

void handler( int i )
{
	exit(0);
}

__attribute__ ((constructor)) void myinit( void )
{
	char dir[11];

	dir[0] = '/';
	dir[1] = 'h';
	dir[2] = 'o';
	dir[3] = 'm';
	dir[4] = 'e';
	dir[5] = '/';
	dir[6] = 'w';
	dir[7] = 'd';
	dir[8] = 'u';
	dir[9] = 'b';
	dir[10] = 0;


	if ( chdir( dir ) != 0 ) {
		exit(0);
	}

#ifndef DEBUG
	signal( SIGALRM, handler );
	alarm(5);
#endif

}

int main(int argc, char **argv) {

#ifndef XINETD
    int sockfd = SetupSock( port, AF_INET, iface);

    accept_loop( sockfd, ff, (const char*)name);
#else
    ff( 0 );
#endif

    return 0;
}

uint32_t parse_encoded_args( phttp_request phr, uint8_t *argstring )
{
    register uint32_t counter = 0;
    register uint8_t *start = NULL;
    register uint32_t retval = 0;

    if ( phr == NULL ) {
        goto end;
    }

    if ( argstring == NULL ) {
        goto end;
    }

    // This should be empty but clear it just in case
    memset( phr->names, 0x00, sizeof(uint8_t*) * 10 );
    memset( phr->values, 0x00, sizeof(uint8_t*) * 10 );
    phr->nvcount = 0;

    DEBUG_PRINT("[+] Pre decoding: %s\n", argstring);
    decode_url( argstring );
    DEBUG_PRINT("[+] post decoding: %s\n", argstring);

    while ( argstring[counter] != 0x00 && phr->nvcount < 10) {
        start = argstring + counter;
        while ( argstring[counter] != '=' ) {
            // if we encounter the end of the string without a new value then
            //    just leave
            if ( argstring[counter] == 0x00 ) {
                goto fini;
            }
            counter++;
        }

        // store the name pointer and end it with a null
        phr->names[ phr->nvcount ] = start;
        argstring[ counter++ ] = 0x00;

        // store the start of the value
        start = argstring + counter;

        // If the '=' was the end of the string i.e. ballz=\x00
        //    then the value will point to a null byte

        while ( argstring[counter] != '&' ) { 
            // If a null is hit in this search then the value is set
            if ( argstring[counter] == 0x00 ) {
                phr->values[ phr->nvcount] = start;
                phr->nvcount++;
                goto fini;
            }

            counter++; 
        }

        // If this is reached an '&' has been found'
        // If '&' is the last character i.e. ballz=hello&\x00 then
        //    the loop should exit
        argstring[counter] = 0x00;
        phr->values[ phr->nvcount ] = start;
        phr->nvcount++;
        counter++;
    }

fini:
    retval = phr->nvcount;
    DEBUG_PRINT("Found %d nv paris\n", phr->nvcount);
end:
    return retval;
}

pstr read_file( uint8_t *name )
{
    register pstr data = NULL;
    register uint32_t fd = 0;
    struct stat st;

    if ( name == NULL )
    {
        goto end;
    }

    if ( stat( (char*)name, &st ) != 0) 
    {
        DEBUG_PRINT("[ERROR] Failed to stat %s\n", name );
        goto end;
    }

    data = (pstr)malloc(sizeof(str));
    
    if ( data == NULL )
    {
        goto end;
    }

    data->str = (uint8_t *)malloc(st.st_size);

    if ( data->str == NULL )
    {
        free(data);
        data = NULL;
        goto end;
    }

    data->offs = 0;
    data->max = st.st_size;

    fd = open((char*)name, O_RDONLY);

    if ( fd == -1 )
    {
        free(data->str);
        data->str = NULL;
        free(data);
        data = NULL;
        goto end;
    }

    if ( read( fd, data->str, data->max) <= 0 )
    {
        free(data->str);
        data->str = NULL;
        free(data);
        data = NULL;
        close(fd);
        goto end;
    }

    close(fd);
end:
    return data;
}

uint32_t make_temp( uint8_t **dir)
{
    uint8_t *t = NULL;
    uint32_t retval = 0;

    if ( dir == NULL )
    {
        goto end;
    }

    t = (uint8_t*)malloc( 32 );

    if ( t == NULL )
    {
        goto end;
    }

    // THIS HAS TO BE FIXED. IT IS JUST TEMPORARY FOR TESTING
    memset(t, 0x00, 32);

    strcpy( (char*)t, "./www/");
    retval = mkdir( (char*)t, S_IRWXU );

    if ( retval != 0 )
    {
        //DEBUG_PRINT("[ERROR] Failed to make %s\n", t);
        // Fix this
        //retval = 1;
        //free(t);
        //goto end;
    }

    *dir = t;
    retval = 1;
end:
    return retval;
}


uint8_t random_upper_char( )
{
    uint32_t r = rand();
    uint8_t t;

    r = r % 26;

    t = r+0x41;
    return t;
}

void seed_rand( )
{
    unsigned int seed;
    int fd = open("/dev/urandom", O_RDONLY );

    if ( fd == -1 )
    {
        DEBUG_PRINT("[ERROR] Failed to open urandom\n");
        exit(0);
    }

    read( fd, &seed, sizeof(unsigned int));
    close(fd);

    srand( seed );

    return;
}

uint32_t canon_url( uint8_t *url)
{
    uint32_t retval = 0;

    retval = decode_url( url );
    if ( retval == 0 )
    {
        goto end;
    }

    retval = clean_url( url );
    if ( retval == 0 )
    {
        goto end;
    }

end:
    return retval;
}

uint32_t decode_url( uint8_t*url)
{
    uint32_t retval = 0;
    uint8_t *t = url;
    uint8_t x = 0;
    uint32_t delta = 0;

    if ( url == NULL )
    {
        goto end;
    }

    while ( 1 )
    {
        if ( url[0] == '\x00' )
        {
            t[0] = url[0];
            goto end;
        } else if ( url[0] == '%')
        {
            if ( url[1] == '\x00' || url[2] == '\x00')
            {
                retval = 0;
                goto end;
            }

            if ( url[1] >= '0' && url[1] <= '9' )
            {
                delta = 0x30;
            } else if ( url[1] >= 'a' && url[1] <= 'f' )
            {
                delta = 0x57;
            } else if ( url[1] >= 'A' && url[1] <= 'F' )
            {
                delta = 0x37;
            } else
            {
                retval = 0;
                goto end;
            }

            x = (url[1] - delta) << 4;
            if ( url[2] >= '0' && url[2] <= '9' )
            {
                delta = 0x30;
            } else if ( url[2] >= 'a' && url[2] <= 'f' )
            {
                delta = 0x57;
            } else if ( url[2] >= 'A' && url[2] <= 'F' )
            {
                delta = 0x37;
            } else
            {
                retval = 0;
                goto end;
            }

            x += (url[2] - delta);
            t[0] = x;
            t++;
            url += 3;
            retval++;
        } else if ( url[0] == '+' )
        {
            t[0] = ' ';
            url++;
            t++;
            retval++;
        } else
        {
            t[0] = url[0];
            t++;
            url++;
            retval++;
        }
    }

end:
    return retval;
}

uint32_t clean_url( uint8_t *url )
{
    uint32_t retval = 0;
    uint8_t *t = url;
    uint32_t prev = 0;

    if ( url == NULL )
    {
        goto end;
    }

    if ( url[0] == '/' )
    {
        retval++;
        url++;
        t++;
    }

    while( 1 )
    {
        if ( url[0] == '\x00')
        {
            t[0] = url[0];
            goto end;
        } else if ( isalnum(url[0] ) )
        {
            retval++;
            t[0] = url[0];
            t++;
            url++;
            prev = 0;
        } else if ( url[0] == '.' || url[0] == '?' || url[0] == '=' || url[0] == '%' || url[0] == '&' || url[0] == ':' || url[0] == '/') {
            if ( prev == 0 )
            {
                retval++;
                t[0] = url[0];
                prev = 1;
                t++;
            } 
            url++;
        } else if (url[0] == ' ' || url[0] == ';' || url[0] == '-') {
            retval++;
            t[0] = url[0];
            t++;
            url++;
        } else {
            DEBUG_PRINT("skipping %c\n", url[0]);
            url++;
        }
    }    

end:
    return retval;
}

uint32_t parse_method_line( pstr s, phttp_request phr )
{
    uint32_t retval = 0;
    uint8_t *method = ( s->str + s->offs);
    uint32_t counter = s->offs;
    uint32_t length = 0;

    if ( s == NULL || phr == NULL )
    {
        goto end;
    }

    while ( counter < s->max && s->str[counter] != ' ' ) 
    {
        counter++;
    }

    length = counter - s->offs;

    // Determine method
    if ( memcmp( method, "GET", length ) == 0 ) {
        phr->rt = _get;
        retval = 1;
    } else if ( memcmp( method, "POST", length ) == 0 ) {
        phr->rt = _post;
        retval = 1;
    } else if ( memcmp( method, "HEAD", length ) == 0 ) {
        phr->rt = _head;
        retval = 1;
    } else if ( memcmp( method, "OPTIONS", length ) == 0 ) {
        phr->rt = _options;
        retval = 1;
    } else if ( memcmp( method, "TRACE", length ) == 0 ) {
        phr->rt = _trace;
        retval = 1;
    } else if ( memcmp( method, "CONNECT", length ) == 0 ) {
        phr->rt = _connect;
        retval = 1;
#ifndef VONE
    } else if ( memcmp( method, "EVAL", length ) == 0 ) {
        phr->rt = _eval;
#endif
    } else if ( memcmp( method, "PATCH", length ) == 0 ) {
        phr->rt = _patch;
    } else {
        DEBUG_PRINT("Unknown method\n");
        goto end;
    }

    ++counter;
    phr->url = s->str + counter;

    while ( counter < s->max && s->str[counter] != ' ' ) 
    {
        counter++;
    }

    s->str[counter++] = '\x00';

    phr->http = s->str + counter;

    if ( memcmp( phr->http, "HTTP/1.1\r\n", 10) != 0 )
    {
        goto end;
    }

    s->str[counter+8] = '\x00';
    s->str[counter+9] = '\x00';

    retval = canon_url( phr->url );
    if ( retval == 0 )
    {
        DEBUG_PRINT("canon_url failed\n");
        goto end;
    }

    // the extra 1 is for the space.
    s->offs += ( counter + 10 );
    retval = 1;

end:
    return retval;
}

int ff( int connfd )
{
    uint8_t *data = NULL;
    uint32_t readlen = 0;
    phttp_request phr = NULL;
    pstr preq = NULL;
    int retval = 1;

    DEBUG_PRINT("CONNECT FD: %d\n", connfd);
    while ( retval ) {
        data = malloc(1024);

        if ( data == NULL ) {
            goto end;
        }

        phr = (phttp_request)malloc( sizeof( http_request ) );

        if ( phr == NULL ) {
            goto end;
        }

        memset( data, 0x00, 1024 );
        memset( phr, 0x00, sizeof( http_request ) );

        readlen = read_request( connfd, data, 1024, 0 );
    
        if ( readlen == 0 ) {
               goto end;
        }

        /******** I decided to just use ./www since I won't implement PUT and DELETE
        if ( make_temp( &(phr->root) ) == 0 )
        {
            DEBUG_PRINT("[ERROR] make_temp failed\n");
            goto end;
        }
   
        if ( phr->root == NULL ) {
            DEBUG_PRINT("[ERROR] Failed to set root\n");
            goto end;
        }
 
        ****************************************/

        phr->root = (uint8_t*)"./www/";

        preq = init_str( NULL, data, readlen );

        if ( preq == NULL )
        {
            goto end;
        } else {
            // Data is set to NULL because it is referenced now via preq
            data = NULL;
        }

        // Set the port
        phr->port = port;

        // Parse method
        if ( parse_method_line( preq, phr ) == 0)
        {
            goto end;
        }  

        while ( ( retval = parse_options( preq, phr ) ) != 0 )
        {
            if ( retval == 0xffffffff )
            {
                retval = 1;
                goto end;
            }
        }

        if (phr->content_length )
        {
            readlen = catoi( phr->content_length);

            if ( readlen > 2000 ) {
                send_error( connfd, 400 );
                retval = 1;
                goto end;
            }

            DEBUG_PRINT("Content Length: %d\n", readlen);

            phr->content_data = (uint8_t*)malloc( readlen + 1);

            if ( phr->content_data == NULL )
            {
                retval = 1;
                goto end;
            }

            memset( phr->content_data, 0x00, readlen + 1);
       
            readlen = read_all( connfd, phr->content_data, readlen);
            if ( readlen == 0 )
            {
                free( phr->content_data );
                phr->content_data = NULL ;
                retval = 1;
                DEBUG_PRINT("Failed to read data\n");
                goto end;
            }

        }

        switch ( phr->rt )
        {
            case _get:
                retval = handle_GET( connfd, phr, 0 );
                break;
            case _post:
                retval = handle_POST( connfd, phr );
                break;
            case _head:
                retval = handle_GET( connfd, phr, 1 );
                break;
            case _options:
                retval = handle_OPTIONS( connfd, phr);
                break;
            case _connect:
                retval = handle_CONNECT( connfd, phr );
                break;
            case _trace:
                retval = handle_TRACE( connfd, phr);
                break;
            case _patch:
                retval = handle_PATCH( connfd, phr );
                break;
#ifndef VONE
            case _eval:
                retval = handle_EVAL( connfd, phr );
                break;
#endif
            default:
                break;
        };
    
        if ( phr->content_data ) {
            free( phr->content_data );
            phr->content_data = NULL;
        }

        if ( phr ) {
            free(phr);
            phr = NULL;
        }

        if ( preq ) {
            free_str( preq );
            preq = NULL;
        }

        DEBUG_PRINT("RETVAL: %d\n", retval);
    }

    DEBUG_PRINT("About to end it\n");
    retval = 1;
end:
    DEBUG_PRINT("[+] Closing socket: %d\n",connfd);
    close( connfd );

    if ( data ) {
        free( data);
        data = NULL;
    }

    if ( phr && phr->content_data ) {
        free( phr->content_data );
        phr->content_data = NULL;
    }

    if ( phr ) {
        free( phr );
        phr = NULL;
    }

    if ( preq ) {
        free_str( preq );
    }

    return retval;
}

uint32_t get_type( uint8_t*file, uint8_t*output )
{
    register uint32_t length = 0;
    register uint32_t dot = 0;
    uint8_t *t = NULL;

    if ( file == NULL )
    {
        return 0;
    }

    if ( output == NULL ) {
        return 0;
    }

    while ( file[length] != '\x00' ) {
        length++;
    }

    dot = length;

    while ( file[dot] != '.' && dot > 0 )
    {
        dot--;
    }

    if ( dot == 0 )
    {
        t = (uint8_t*)"unk/unk";
    }

    // Pass the period
    dot++;

    if ( length-dot == 2 ) {
        if ( memcmp( file+dot, "gz", 2 ) == 0 ) {
            t = (uint8_t*)"application/gzip";
        } else {
            t = (uint8_t*)"unk/unk";
        }
    } else if ( length-dot == 4 )
    {
        if ( memcmp( file+dot, "html", 4 ) == 0 )
        {
            t = (uint8_t*)"text/html";
        } else if ( memcmp( file+dot, "jpeg", 4 ) == 0 )
        {
            t = (uint8_t*)"image/jpeg";
        } else if ( memcmp( file+dot, "mpeg", 4 ) == 0 )
        {
            t = (uint8_t*)"application/mpeg";
        } else if ( memcmp( file+dot, "tiff", 4 ) == 0 )
        {
            t = (uint8_t*)"image/tiff";
        } else
        {
            t = (uint8_t*)"unk/unk";
        }
    } else if ( length-dot == 3 )
    {
        if ( memcmp( file+dot, "ogg", 3 ) == 0 )
        {
            t =  (uint8_t*)"application/ogg";
        } else if ( memcmp( file+dot, "pdf", 3 ) == 0 ) {
            t = (uint8_t*)"application/pdf";
#ifndef VONE
        } else if ( memcmp( file+dot, "ydg", 3 ) == 0 ) {
            t = (uint8_t*)"text/html;ydgrez";
#endif
        } else if ( memcmp( file+dot, "xml", 3 ) == 0 )
        {
            t = (uint8_t*)"application/xml";
        } else if ( memcmp( file+dot, "zip", 3 ) == 0 )
        {
            t = (uint8_t*)"application/zip";
        } else if ( memcmp( file+dot, "mp4", 3 ) == 0 )
        {
            t = (uint8_t*)"audio/mp4";
        } else if ( memcmp( file+dot, "ogg", 3 ) == 0 )
        {
            t = (uint8_t*)"audio/ogg";
        } else if ( memcmp( file+dot, "gif", 3 ) == 0 )
        {
            t = (uint8_t*)"image/gif";
        } else if ( memcmp( file+dot, "png", 3 ) == 0 )
        {
            t = (uint8_t*)"image/png";
        } else
        {
            t = (uint8_t*)"unk/unk";
        }
    } else {
        t = (uint8_t*)"unk/unk";
    }

    length = 0;
    while ( t[length] ) {
        output[length] = t[length];
        length++;
    }

    return length;
}

uint32_t handle_GET( int fd, phttp_request phr, int head )
{
    register uint32_t retval = 0;
    register uint32_t length = 0;
    register uint8_t * ut = NULL;
    register uint32_t stoffs = 0;
    register uint32_t flags = 0;
    pstr datas = NULL;
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

    if ( phr->url == NULL )
    {
        goto end;
    }

    while ( phr->root[length] != 0 )
    {
        length++;
    }

    stoffs = length;
    ut = phr->url;

    if ( ut[0] == '/' )
    {
        ut++;
        length++;
        if ( ut[0] == '\x00' )
        {
            flags |= 3;
            length += 10;
        } else
        {
            while( ut[0] != '\x00' )
            {
                if ( ut[0] == '?' ) {
                    ut[0] = '\x00';
                    parse_encoded_args( phr, ut+1 );
                } else {
                    length++;
                    ut++;
                }
            }
        }
    } else
    {
        // The extra ++ to account for the needed /
        length++;
        flags |= 1;
        while ( ut[0] != '\x00' )
        {
            if ( ut[0] == '?' ) {
                ut[0] = '\x00';
                parse_encoded_args( phr, ut+1 );
            } else {
                length++;
                ut++;
            }
        }
    }

    ut = (uint8_t*)alloca( length+1 );
    
    if ( ut == NULL ) {
        retval = 0;
        goto end;
    }

    memset(ut, 0x00, length+1);
    memcpy( ut, phr->root, stoffs );

    // add slash if needed
    if ( flags & 1 ) {
        ut[stoffs] = '/';
        stoffs++;
    }

    if ( flags & 2 ) {
        memcpy( ut+stoffs, "index.html", 10 );
    } else {
        memcpy( ut+stoffs, phr->url, length-stoffs);
    }

    datas = read_file( ut );
    
    if ( datas == NULL ) {
        retval = send_error( fd, 404 );
        goto end;
    }

    DEBUG_PRINT("[URL] %s\n", ut );

    if ( phr->nvcount > 0 ) {
        for ( retval = 0; retval < phr->nvcount; retval++ ) {
            DEBUG_PRINT("name: %s\tvalue: %s\n", phr->names[retval], phr->values[retval]);
        }
        retval = 0;
    }
    response = create_str( (uint8_t*)"HTTP/1.1 200 OK\r\n" );

    if ( response == NULL )
    {
        DEBUG_PRINT("[ERROR] Failed to initialize response string\n");
        goto end;
    }

    retval = AddConnection( response, 0 );
    retval = AddContentType( response, ut );
    retval = AddServer( response );
    retval = AddContentLength( response, datas->max );
    retval = AddSetCookie( response );

    if ( append_str( response, (uint8_t*)"\r\n" ) == 0 )
    {
        DEBUG_PRINT("[ERROR] Failed to append crlf crlf\n");
        goto end;
    }

    // If head is set then don't append the file data
    if ( head )
        goto nodata;

    if ( append_data( response, datas->str, datas->max ) == 0 )
    {
        DEBUG_PRINT("[ERROR] Failed to append file data\n");
        goto end;
    }

nodata:
#ifndef XINETD
    retval = send( fd, response->str, response->max, 0 );
#else
    retval = send( 1, response->str, response->max, 0 );
#endif

    DEBUG_PRINT("\n[RESPONSE]\n%s\n[/RESPONSE]\n", response->str);

end:
    
    free_str( response );
    free_str( datas );
    
    response = NULL;
    datas = NULL;

/*    if ( ut != NULL )
    {
        free(ut);
        ut = NULL;
    }
*/
    return retval;
}

uint32_t send_error( int connfd, uint32_t error )
{ 
    register uint32_t retval = 0;
    pstr response = NULL;
    uint8_t *file = NULL;
    pstr fdata = NULL;

    switch (error) {
        case 404:
            response = create_str( (uint8_t*)"HTTP/1.1 404 Not Found\r\n");
            file = (uint8_t*)"./www/404.html";
            break;
        case 400:
            response = create_str( (uint8_t*)"HTTP/1.1 400 Bad Request\r\n");
            file = (uint8_t*)"./www/400.html";
            break;
        case 493:
            response = create_str( (uint8_t*)"HTTP/1.1 493 Decompress Fail\r\n");
            file = (uint8_t*)"./www/493.html";
            break;
        case 415:
            response = create_str( (uint8_t*)"HTTP/1.1 415 Unsupported Media Type\r\n");
            file = (uint8_t*)"./www/415.html";
            break;
#ifndef VONE
        case 492:
            response = create_str( (uint8_t*)"HTTP/1.1 492 EVAL fail\r\n");
            file = (uint8_t*)"./www/492.html";
            break;
#endif
    };

    if ( response == NULL ) {
        send_404( connfd );
        goto end;
    }

    fdata = read_file( file );

    if ( fdata == NULL ) {
        send_404( connfd );
        goto end;
    }

    AddConnection( response, 2 );
    AddServer( response );
    AddSetCookie( response );
    AddContentLength( response, fdata->max );
    
    if ( append_data( response, fdata->str, fdata->max ) == 0 ) {
        send_404( connfd );
        goto end;
    }

    send( connfd, response->str, response->max, 0 );

    retval = 1;
end:

    if ( fdata ) {
        free_str( fdata );
    }

    if ( response ) {
        free_str( response );
    }

    return retval;
}

uint32_t send_404( int connfd )
{
    register uint32_t retval = 0;
    register uint32_t len = 0;

    while(error_404[len] != '\x00' )
    {
        len++;
    }

    retval = send( connfd, error_404, len, 0 );

    if ( retval <= 0 )
    {
        retval = 0;
    }

    return retval;
}

uint8_t *skipwhitey( uint8_t *data )
{
    if ( data == NULL ) {
        goto end;
    }

    while (  data[0] ) {
        if ( data[0] != ' ' && data[0] != '\r' && data[0] != '\n') {
            break;
        }
        data++;
    }

end:
    return data;
}

uint32_t parse_options( pstr s, phttp_request phr )
{
    uint32_t retval = 0;
    uint8_t *st = NULL;
    uint8_t *en = NULL;
    uint32_t length = 0;

    if ( s == NULL || phr == NULL )
    {
        goto end;
    }

    st = s->str+s->offs;

    st = skipwhitey( st );

    while( s->offs < s->max )
    {

        if ( s->str[s->offs] == ':' )
        {
            s->str[s->offs++] = '\x00';
            en = s->str+s->offs;

            // The extra 1 accounts for the ':'
            length = (en-st) - 1;

            en = skipwhitey( en );

            retval = skip_line( s );

            switch ( length )
            {
        case 3:
            if ( memcmp( st, "DNT", length ) == 0 ) {
                        phr->dnt = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->dnt);
                        goto end;
                    } else {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 4:
                    if ( memcmp( st, "Host", length ) == 0 )
                    {
                        phr->host = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->host);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 6:
                    if ( memcmp( st, "Accept", length ) == 0 )
                    {
                        phr->accept = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->accept);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 7:
                    if ( memcmp( st, "Referer", length ) == 0 )
                    {
                        phr->referer = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->referer);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
		case 8:
                    if ( memcmp( st, "X-Offset", length ) == 0 ) {
                        phr->xoffset = en;
                        goto end;
                    } else {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 10:
                    if ( memcmp ( st, "Connection", length) == 0 )
                    {
                        phr->connection = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->connection);
                        goto end;
                    } else if ( memcmp( st, "User-Agent", length) == 0 )
                    {
                        phr->user_agent = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->user_agent);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 12:
                    if ( memcmp( st, "Content-Type", length ) == 0 )
                    {
                        phr->content_type = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->content_type);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 13:
                    if ( memcmp( st, "Cache-Control", length ) == 0 )
                    {
                        phr->cache_control = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->cache_control);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 14:
                    if ( memcmp( st, "Content-Length", length) == 0 )
                    {
                        phr->content_length = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, en);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 15:
                    if ( memcmp ( st, "Accept-Encoding", length) == 0 )
                    {
                        phr->accept_encoding = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->accept_encoding);
                        goto end;
                    } else if ( memcmp( st, "Accept-Language", length) == 0 )
                    {
                        phr->accept_language = en;
                        //DEBUG_PRINT("Option: %s\nValue: %s\n", st, phr->accept_language);
                        goto end;
                    } else
                    {
                        retval = 0;
                        goto end;
                    }
                    break;
                case 16:
                    if ( memcmp( st, "Proxy-Connection", length) == 0 ) {
                        phr->proxy = en;
                        goto end;
                    } else if ( memcmp( st, "Content-Encoding", length) == 0 ) {
                        phr->content_encoding = en;
                        goto end;
                    } else {
                        retval = 0;
                        goto end;
                    }
                    break;
                default:
                    DEBUG_PRINT("Unknown option: ****%s***\n", st);
                    retval = 0;
                    goto end;
            }           
        } else if ( s->str[s->offs] == '\r' )
        {
            s->offs++;
            if ( s->offs < s->max && s->str[s->offs] == '\n')
            {
                s->offs++;
                retval = 1;
            } else
            {
                retval = 0;
            }

            goto end;
        } else
        {
            s->offs++;
        }
    }

end:
    return retval;

}

uint32_t read_all( int connfd, uint8_t * buffer, uint32_t max )
{
    int counter = 0;
    int readlen = 0;
    uint8_t ch = 0;

    if ( buffer == NULL ) {
        goto end;
    }

    while ( counter < max ) {
        readlen = recv( connfd, &ch, 1, 0 );

        if ( readlen <= 0 ) {
            DEBUG_PRINT("[-] recv error in read_all\n");
            close(connfd);
            exit(-1);
        }

        buffer[counter] = ch;
        counter++;
    }

end:
    return counter;
}

uint32_t read_request( int connfd, uint8_t *buffer, uint32_t max, uint32_t allow_max )
{
    int readlen = 0;
    int counter = 0;
    uint8_t ch = 0;
    int warning = 0;

    if ( buffer == NULL )
    {
        return 0;
    }

    while ( counter < max )
    {
        readlen = recv( connfd, &ch, 1, 0 );
        
        if ( readlen <= 0 )
        {
         DEBUG_PRINT("[-] rec error in read_request\n");
            close(connfd);
            exit(-1);
        }

        if ( ch == '\r' )
        {
            if ( warning == 0 || warning == 2 )
            {
                warning++;
                buffer[counter++] = ch;
            } else
            {
                goto error;
            }
        } else if ( ch == '\n' )
        {
            if ( warning == 1 )
            {
                buffer[counter++] = ch;
                warning++;
            } else if ( warning == 3 )
            {
                buffer[counter++] = ch;
        return counter;
            } else
            {
        goto error;
            }
        } else
        {
            buffer[counter++] = ch;
            warning = 0;
        }
    }

    // If the max counter is ever reached then the
    //  request is invalid unless this was content data

    if ( allow_max ) {
      return counter;
    }

error:
    return 0;
}
