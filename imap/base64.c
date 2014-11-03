#include <stdio.h>
#include <glib.h>
#include <string.h>

gchar *b64_encode(guchar *data, gint len) {
	gchar *out;

	out = g_base64_encode(data, len);

	return(out);
}

void b64_decode(gchar *data, gsize *len, unsigned char *raw_message) {
	guchar *out;

	out = g_base64_decode(data, len);
	memcpy(raw_message, out, *len);
	g_free(out);
}
