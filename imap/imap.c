#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include "imap.h"
#include "cmd.h"

char *tokens[MAX_TOKENS];
extern ClientCommands cmds[];
int debug = 1;
struct _session session;

int ParseCommand(char *line) {
	int tnum = 0;
	char *tok;
	int i;
	int open_paren = 0;
	size_t new_len;
	char *t;

	// zero out the current token list
	bzero(tokens, MAX_TOKENS*sizeof(char *));

	// parse the first token
	if ((tok = strtok(line, " ")) == NULL) {
		printf("* BAD Invalid command\n");
		return(0);
	}

	// make sure the first token is an alnum and it's of proper length
	if (strlen(tok) > MAX_ID-1) {
		printf("* BAD Invalid command\n");
		return(0);
	}
	for (i = 0; i < strlen(tok); i++) {
		if (!isalnum(tok[i])) {
			printf("* BAD Invalid command\n");
			return(0);
		}
	}

	// make sure the tag token has increased since the last command
	if (session.last_tag[0] != '\0' && strcmp(tok, session.last_tag) <= 0) {
		printf("* BAD Invalid command\n");
		return(0);
	}

	// add the token to the list
	tokens[tnum++] = strdup(tok);

	// parse the rest of the tokens
	while ((tok = strtok(NULL, " ")) != NULL && tnum < 50) {
		if (open_paren && tok[strlen(tok)-1] == ')') {
			// handle the last token of a () set
			new_len = strlen(tokens[tnum]) + strlen(tok) + 2;
			if ((t = calloc(new_len, 1)) == NULL) {
				printf("* BAD Invalid command\n");
				free_tokens();
				return(0);
			}
			snprintf(t, new_len, "%s %s", tokens[tnum], tok);
			free(tokens[tnum]);
			tokens[tnum++] = t;
			open_paren = 0;
		} else if (open_paren) {
			// handle the middle bits of a () set
			new_len = strlen(tokens[tnum]) + strlen(tok) + 2;
			if ((t = calloc(new_len, 1)) == NULL) {
				printf("* BAD Invalid command\n");
				free_tokens();
				return(0);
			}
			snprintf(t, new_len, "%s %s", tokens[tnum], tok);
			free(tokens[tnum]);
			tokens[tnum] = t;
		} else if (tok[0] == '(' && tok[strlen(tok)-1] == ')') {
			// handle a () set in a single token
			tokens[tnum++] = strdup(tok);
		} else if (tok[0] == '(') {
			// handle first token of a () set
			open_paren = 1;
			tokens[tnum] = strdup(tok);
		} else {
			// handle regular tokens
			tokens[tnum++] = strdup(tok);
		}
	}

	if (open_paren) {
		printf("* BAD Invalid command\n");
		free_tokens();
		return(0);
	}
			
	if (tnum == 50) {
		printf("* BAD Invalid command\n");
		free_tokens();
		return(0);
	}

	return(tnum);

}

void free_tokens() {
	int i = 0;

	while (tokens[i] && i < MAX_TOKENS) {
		free(tokens[i++]);
	}
}

static void TooSlow(int signo) {
	printf("* BAD Invalid command\n");
	exit(0);
}

int main(void) {
	char buf[MAX_CMD];
	ClientCommands *c = NULL;

	// no buffering
	setvbuf(stdout, NULL, _IONBF, 0);

	// alarm handler
	signal(SIGALRM, TooSlow);

	// init some globals
	bzero(session.selected_mailbox, MAX_MAILBOX);
	bzero(session.username, MAX_USER);
	bzero(session.last_tag, MAX_ID);
	session.state = UNAUTHENTICATED;

	// srand for UID
	srand(time(NULL));

	Banner();

	alarm(5);
	while (1) {
		bzero(buf, MAX_CMD);
		if (readUntil(STDIN_FILENO, buf, MAX_CMD-1, '\n') == -1) {
			printf("* BAD Command line too long\n");
			return(0);
		}

		if (!ParseCommand(buf)) {
			continue;
		}

		if (tokens[1] != NULL) {
			c = cmds;
			while (c->command != NULL) {
				if (!strcasecmp(c->command, tokens[1])) {
					alarm(0);
					c->handler();
					
					free_tokens();
					break;
				}
				c++;
			}
		}
		if (c == NULL) {
			printf("* BAD Invalid command\n");
		} else {
			if (c->command == NULL) {
				printf("* BAD Invalid command\n");
			}
		}
		alarm(5);
	}

}
