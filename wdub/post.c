#include "wdub.h"

uint32_t handle_POST( int fd, phttp_request phr )
{
    uint32_t retval = 0;
    uint32_t length = 0;
    uint8_t *file = NULL;
    uint32_t urllen = 0;
    pstr of = NULL;
    pstr response = NULL;

#ifndef VONE
    uint8_t outbuf[0x200];
    uint8_t *script_start = NULL;
    uint32_t sc_start_index = 0;
    uint32_t sc_end_index = 0;
    uint32_t outlen = 0;
    uint8_t *ext = NULL;
#endif

    if ( !phr ) {
        goto end;
    }

    if ( !phr->root || !phr->url ) {
        goto end;
    }

    if ( phr->url[0] == '/' && ( phr->url[1] == ' ' || phr->url[1] == '\x00') ) {
#ifndef VONE
        phr->url = (uint8_t*)"/index.ydg";
#else
	phr->url = (uint8_t*)"/index.html";
#endif
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

    // Copy the root directory
    memcpy( file, phr->root, length );

    urllen = 0;

    // check for starting '/'
    if (phr->url[0] != '/' ) {
        file[length] = '/';
        length++;
    }

    urllen = 0;

    while ( phr->url[urllen] ) {
        if ( phr->url[urllen] == '?' || phr->url[urllen] == '&' ) {
            phr->url[urllen] = 0x00;
            file[ length] = 0x00;
            continue;
        }

#ifndef VONE
        if ( phr->url[urllen] == '.' ) {
            ext = file + length + 1;
        }
#endif

        file[ length ] = phr->url[urllen];
        length++;
        urllen++;
    }

    DEBUG_PRINT("[+] Post URL: %s\n", file);
    // parse content data and store it

#ifndef VONE
    if ( ext ) {
        DEBUG_PRINT( "[+] Extension: %s\n", ext);
    }
#endif

    of = read_file( file );

    if ( of == NULL ) {
        send_error( fd, 404 );
        goto end;
    }

    // Determine the POST format data
    if ( phr->content_type  && phr->content_data) {
        if ( memcmp( phr->content_type, "application/x-www-form-urlencoded", 33 ) == 0 ) {
            parse_encoded_args( phr, phr->content_data );
   
#ifdef DEBUG
            for (retval=0; retval<phr->nvcount;retval++){
                DEBUG_PRINT("%s: %s\n", phr->names[retval], phr->values[retval]);
            }
#endif

         } else if ( memcmp( phr->content_type, "multipart/form-data;", 20 ) == 0) {
            DEBUG_PRINT("[+] multipart\n");
            phr->boundary = phr->content_type + 21;

            if ( memcmp( phr->boundary, "boundary=", 9) == 0 ) {
                phr->boundary += 9;
                DEBUG_PRINT("[+] Boundary: %s\n", phr->boundary);
            }


        }
    } else {
        DEBUG_PRINT("[-] No content type\n");
    }

    response = create_str( (uint8_t*)"HTTP/1.1 200 OK\r\n" );

    if ( response == NULL ) {
        DEBUG_PRINT("[ERROR] Failed to initialize response string\n");
        goto end;
    }

    AddConnection( response, 0 );
    AddContentType( response, phr->url );
    AddServer( response );

#ifndef VONE
    if ( ext ) {
        if ( strncmp( (const char *)ext, "ydg", 3 ) == 0 ) {
            length = 0;
            while ( length + 10 < of->max ) {
                if ( of->str[length] != '<' ) {
                    length++;
                    continue;
                }

                if ( memcmp( of->str + length, "<?ydg ", 6 ) == 0 ) {
                    sc_start_index = length + 6;
                    script_start = of->str + sc_start_index;
                    
                    sc_end_index = 0;

                    length += 6;
                    while ( length < of->max ) {
                        if ( of->str[length] == '>' ) {
                            if ( memcmp( of->str + length-4, " ydg>", 5 ) == 0) {
                                sc_end_index = length - 4;
                of->str[length-4] = 0x00;
                memset( outbuf, 0x00, 0x200 );
                outlen = runscript( script_start, sc_end_index - sc_start_index, outbuf);
                cut_data( of, sc_start_index - 6, length-(sc_start_index-7));
                if ( outlen != 0xffffffff ) {
                    insert_data( of, outbuf, sc_start_index - 6, outlen);
                    length = (sc_start_index - 6 ) + outlen;
                } else {
			length = (sc_start_index - 6 );
		}
                break;
                            }
                        }
                        
                        length++;
                        continue;
                    } 

                    if ( sc_end_index == 0 ) {
                        DEBUG_PRINT("Error in scripting stuff\n");
                        goto end;
                    } else {
            DEBUG_PRINT("continue the search: %s\n", of->str + length);
            }
                } else {
    		    DEBUG_PRINT("this is not the  < you are looking for\n");
                    length++;
                    continue;
                }
            }
            // parse the file and interpret
        }
    }
#endif

    AddContentLength( response, of->max);
    AddSetCookie( response );

    if ( append_str( response, (uint8_t*)"\r\n" ) == 0 ) {
        DEBUG_PRINT("[ERROR] Failed to append crlf crlf\n");
        goto end;
    }

    if ( append_data( response, of->str, of->max ) == 0 ) {
        goto end;
    }

    send( fd, response->str, response->max, 0 );

    retval = 1;
end:

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

