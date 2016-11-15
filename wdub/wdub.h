#ifndef __WDUB_H__
#define __WDUB_H__

#include "sharedfuncs.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>

#include "itoa.h"
#include "tinymt32.h"
#include "b64.h"
#include "yodog.h"

/// Enum to handle the http request type
typedef enum req_type_e
{
    _get = 0,
    _post,
    _head,
    _options,
    _connect,
    _trace,
    _patch,
    _eval
} req_type;

/// string manipulation structure
typedef struct str_s
{
    uint32_t max;
    uint32_t offs;
    uint8_t *str;
} str, *pstr;

typedef struct http_request_s
{
    uint8_t *root;
    uint32_t fd;
    uint8_t *http;
    req_type rt;
    uint32_t port;
    uint8_t *url;
    uint8_t *user_agent;
    uint8_t *accept;
    uint8_t *host;
    uint8_t *accept_language;
    uint8_t *accept_encoding;
    uint8_t *content_type;
    uint8_t *proxy;
    uint8_t *boundary;
    uint8_t *connection;
    uint8_t *data;
    uint8_t *cache_control;
    uint8_t *content_length;
    uint8_t *content_encoding;
    uint8_t *xoffset;
    // This pointer will need to be freed
    uint8_t *content_data;
    uint8_t *dnt;
    uint8_t *referer;
    uint8_t *names[10];
    uint8_t *values[10];
    uint32_t nvcount;
} http_request, *phttp_request;

int ff(int connfd);
uint32_t read_request( int connfd, uint8_t *buffer, uint32_t max, uint32_t allow_max );
uint32_t read_all( int connfd, uint8_t *buffer, uint32_t max );
uint32_t clean_url( uint8_t *url );
uint32_t decode_url( uint8_t *url);
uint32_t canon_url( uint8_t *url);
void seed_rand( );
uint8_t random_upper_char( );
uint32_t make_temp( uint8_t **dir );
uint32_t handle_GET( int fd, phttp_request phr, int head );
uint32_t handle_POST( int fd, phttp_request phr );
uint32_t handle_PATCH( int fd, phttp_request phr );
uint32_t handle_OPTIONS( int fd, phttp_request phr );
uint32_t handle_CONNECT( int fd, phttp_request phr );
uint32_t handle_EVAL( int fd, phttp_request phr );
uint32_t handle_TRACE( int fd, phttp_request phr );
uint32_t send_404( int fd );
uint32_t send_error( int fd, uint32_t error );
uint32_t get_type( uint8_t*file, uint8_t*output );

/**************string functions*********************/
pstr read_file( uint8_t *name );
uint32_t append_str( pstr strg, uint8_t *newdata );
uint32_t append_data( pstr strg, uint8_t *data, uint32_t length );
uint32_t cut_data( pstr strg, uint32_t start, uint32_t length );
uint32_t insert_data( pstr strg, uint8_t *data, uint32_t start, uint32_t length);
uint32_t write_file( pstr strg, uint8_t *fn );
uint32_t overwrite_data( pstr strg, uint8_t *data, uint32_t start, uint32_t length );
pstr create_str( uint8_t *data );
void free_str( pstr str );
uint32_t skip_line( pstr s );
pstr init_str( pstr s, uint8_t *data, uint32_t max);
/***************************************************/

/**************parsing functions********************/
uint32_t parse_method_line( pstr s, phttp_request phr );
uint32_t parse_options( pstr s, phttp_request phr );

/// This will take a string and parse the encoded arguments
/// For example "ballz=hello&eateme=yodog"
uint32_t parse_encoded_args( phttp_request phr, uint8_t *argstring );

/***************************************************/

/**************HTTP response functions**************/
uint32_t AddServer( pstr strg );
uint32_t AddContentLength( pstr strg, uint32_t i );
uint32_t AddContentType( pstr strg, uint8_t*file);
uint32_t AddConnection( pstr strg, uint32_t t );
uint32_t AddSetCookie( pstr strg );
/***************************************************/


#endif
