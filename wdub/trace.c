#include "wdub.h"

#define MLENMAX 28
#define TRACELEN 6
#define HTTPLEN 11
#define DATAMAX 0x200
#define UIEP (uint8_t *)

uint32_t handle_TRACE( int fd, phttp_request phr )
{
	struct {
		uint8_t method_line[MLENMAX];
		str response;
		uint32_t mlen;
		uint32_t retval;
		uint8_t data[DATAMAX];
	} a;

	a.mlen = 0;
	a.retval = 0;

	if ( !phr ) {
		goto end;
	}

	if ( phr->root == NULL ) {
		goto end;
	}

	if ( phr->url == NULL ) {
		goto end;
	}

	memset( a.data, 0x00, DATAMAX);

	a.mlen = strlen( (const char *)phr->root);

	strncpy( (char *)a.data, (const char *)phr->root, a.mlen);
	strncpy( (char*)a.data + a.mlen, (const char *)phr->url, 0xff - a.mlen);

	if ( access( (const char *)a.data, F_OK) ) {
		send_error( fd, 404);
		DEBUG_PRINT("%s does not exist\n", a.data);
		goto end;
	}

	close(a.mlen);

	memset( a.data, 0x00, DATAMAX);

	init_str( &a.response, a.data, DATAMAX );

	memset( a.method_line, 0x00, MLENMAX );

	memcpy( a.method_line, "TRACE ", 6 );
	a.mlen = 6;

	while ( a.mlen <= (MLENMAX - (HTTPLEN) ) ) {
		a.method_line[ a.mlen ] = phr->url[ a.mlen-6];

		if ( a.method_line[ a.mlen ] == ' ' || a.method_line[ a.mlen] == 0x00 ) {
			break;
		}

		a.mlen++;
	}

	memcpy( a.method_line + a.mlen, " HTTP/1.1\r\n", HTTPLEN );
	a.mlen += HTTPLEN;

	overwrite_data( &a.response, a.method_line, a.response.offs, a.mlen );
	a.response.offs += a.mlen;

	if ( phr->user_agent ) {
		if ( overwrite_data( &a.response, UIEP"User-Agent: ", a.response.offs, 12) == 0 ) {
			goto end;
		}
		a.response.offs += 12;

		a.mlen = strlen( (const char *)phr->user_agent);
		if ( overwrite_data( &a.response, phr->user_agent, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	

	if ( phr->accept ) {
		if ( overwrite_data( &a.response, UIEP"Accept: ", a.response.offs, 8) == 0 ) {
			goto end;
		}
		a.response.offs += 8;

		a.mlen = strlen( (const char *)phr->accept);
		if ( overwrite_data( &a.response, phr->accept, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->host ) {
		if ( overwrite_data( &a.response, UIEP"Host: ", a.response.offs, 6) == 0 ) {
			goto end;
		}
		a.response.offs += 6;

		a.mlen = strlen( (const char *)phr->host);
		if ( overwrite_data( &a.response, phr->host, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}


	if ( phr->accept_language ) {
		if ( overwrite_data( &a.response, UIEP"Accept-Language: ", a.response.offs, 17) == 0 ) {
			goto end;
		}
		a.response.offs += 17;

		a.mlen = strlen( (const char *)phr->accept_language);
		if ( overwrite_data( &a.response, phr->accept_language, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}

	if ( phr->accept_encoding ) {
		if ( overwrite_data( &a.response, UIEP"Accept-Encoding: ", a.response.offs, 17) == 0 ) {
			goto end;
		}
		a.response.offs += 17;

		a.mlen = strlen( (const char *)phr->accept_encoding);
		if ( overwrite_data( &a.response, phr->accept_encoding, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}

	if ( phr->content_type ) {
		if ( overwrite_data( &a.response, UIEP"Content-Type: ", a.response.offs, 14) == 0 ) {
			goto end;
		}
		a.response.offs += 14;

		a.mlen = strlen( (const char *)phr->content_type);
		if ( overwrite_data( &a.response, phr->content_type, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}

	if ( phr->proxy ) {
		if ( overwrite_data( &a.response, UIEP"Proxy: ", a.response.offs, 7) == 0 ) {
			goto end;
		}
		a.response.offs += 7;

		a.mlen = strlen( (const char *)phr->proxy);
		if ( overwrite_data( &a.response, phr->proxy, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->boundary ) {
		if ( overwrite_data( &a.response, UIEP"Boundary: ", a.response.offs, 10) == 0 ) {
			goto end;
		}
		a.response.offs += 10;

		a.mlen = strlen( (const char *)phr->boundary);
		if ( overwrite_data( &a.response, phr->boundary, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->connection ) {
		if ( overwrite_data( &a.response, UIEP"Connection: ", a.response.offs, 12) == 0 ) {
			goto end;
		}
		a.response.offs += 12;

		a.mlen = strlen( (const char *)phr->connection);
		if ( overwrite_data( &a.response, phr->connection, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}

	if ( phr->cache_control ) {
		if ( overwrite_data( &a.response, UIEP"Cache-Control: ", a.response.offs, 15) == 0 ) {
			goto end;
		}
		a.response.offs += 15;

		a.mlen = strlen( (const char *)phr->cache_control);
		if ( overwrite_data( &a.response, phr->cache_control, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	
	if ( phr->content_length ) {
		if ( overwrite_data( &a.response, UIEP"Content-Length: ", a.response.offs, 16) == 0 ) {
			goto end;
		}
		a.response.offs += 16;

		a.mlen = strlen( (const char *)phr->content_length);
		if ( overwrite_data( &a.response, phr->content_length, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->content_encoding ) {
		if ( overwrite_data( &a.response, UIEP"Content-Encoding: ", a.response.offs, 18) == 0 ) {
			goto end;
		}
		a.response.offs += 18;

		a.mlen = strlen( (const char *)phr->content_encoding);
		if ( overwrite_data( &a.response, phr->content_encoding, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->xoffset ) {
		if ( overwrite_data( &a.response, UIEP"X-Offset: ", a.response.offs, 10) == 0 ) {
			goto end;
		}
		a.response.offs += 10;

		a.mlen = strlen( (const char *)phr->xoffset);
		if ( overwrite_data( &a.response, phr->xoffset, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->referer ) {
		if ( overwrite_data( &a.response, UIEP"Referer: ", a.response.offs, 9) == 0 ) {
			goto end;
		}
		a.response.offs += 9;

		a.mlen = strlen( (const char *)phr->referer);
		if ( overwrite_data( &a.response, phr->referer, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( phr->dnt ) {
		if ( overwrite_data( &a.response, UIEP"DNT: ", a.response.offs, 5) == 0 ) {
			goto end;
		}
		a.response.offs += 5;

		a.mlen = strlen( (const char *)phr->dnt);
		if ( overwrite_data( &a.response, phr->dnt, a.response.offs, a.mlen ) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;

		if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
			goto end;
		}

		a.response.offs += 2;
	}
	
	if ( overwrite_data( &a.response, UIEP"\r\n", a.response.offs, 2 ) == 0 ) {
		goto end;
	}

	if ( phr->content_data ) {
		a.mlen = catoi( phr->content_length );
		if ( overwrite_data( &a.response, phr->content_data, a.response.offs, a.mlen) == 0 ) {
			goto end;
		}
		a.response.offs += a.mlen;
	}

	if ( send( fd, a.response.str, a.response.max, 0 ) <= 0 ) {
		goto end;
	}

	a.retval = 1;
end:
	return a.retval;
}
