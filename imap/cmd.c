#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glib.h>
#include "cmd.h"
#include "imap.h"
#ifdef VER2
#include "base64.h"
#endif

ClientCommands cmds[] = {
  // anytime commands
  {"CAPABILITY", CapabilityHandler},
  {"NOOP", NoopHandler},
  {"LOGOUT", LogoutHandler},
  // unauthenticated commands
  {"STARTTLS", NotImplementedHandler},
  {"AUTHENTICATE", NotImplementedHandler},
  {"LOGIN", LoginHandler},
  {"XRESUWEN", NewUserHandler},
  // authenticated commands
  {"SELECT", SelectHandler},
  {"EXAMINE", ExamineHandler},
  {"CREATE", CreateHandler},
  {"DELETE", DeleteHandler},
  {"RENAME", RenameHandler},
  {"SUBSCRIBE", NotImplementedHandler},
  {"UNSUBSCRIBE", NotImplementedHandler},
  {"LIST", ListHandler},
  {"LSUB", NotImplementedHandler},
  {"STATUS", StatusHandler},
  {"APPEND", AppendHandler},
  // selected state commands
#ifdef VER2
  {"CHECK", CheckHandler},
#endif
  {"CLOSE", CloseHandler},
  {"EXPUNGE", ExpungeHandler},
  {"FETCH", FetchHandler},
  {"STORE", StoreHandler},
  {"COPY", NotImplementedHandler},
  {"UID", NotImplementedHandler},
  {NULL, NULL},
};


extern char *tokens[];
extern struct _session session;

void Banner() {
#ifdef DEFCTF
	FILE *in;
	int i;
	if ((in = fopen("/dev/ctf", "r")) == NULL) {
		printf("oops\n");
		exit(0);
	}	
	fread(session.devctf, 16, 1, in);
	fclose(in);
#endif
	printf("* OK IMAP4rev1 server ready ");

#ifdef DEFCTF
	for (i = 0; i < 16; i++) {
		printf("%hhx",session.devctf[i]);
	}
#endif
	printf("\n");
}

int RandomNum() {
	char buf[10];

	return(rand());
}

unsigned int ValidTokens() {
	int i;
	unsigned int tc = 0;

	if (tokens == NULL) {
		return(0);
	}

	for (i = 0; i < MAX_TOKENS; i++) {
		if (tokens[i] == NULL) {
			break;
		}
		tc++;
	}

	return(tc);
}

int NotImplementedHandler() {
	if (!ValidTokens(1)) {
		printf("* BAD Command format error\n");
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	printf("%s BAD Command not implemented\n", tokens[0]);

	return(0);
}

int ParseFlags(char *str, Flags *f) {
	char buf[MAX_FLAG_SIZE];
	char *tok;

	if (str == NULL || f == NULL) {
		return(-1);
	}

	f->all = NOFLAG;
	bzero(buf, MAX_FLAG_SIZE);

	// make sure we have the proper format
	if (str[0] == '(' && str[strlen(str)-1] == ')') {
		str[strlen(str)-1] = '\0';
		if ((tok = strtok(str+1, " ")) == NULL) {
			return(0);
		}
	} else if (str[0] != '(' && str[strlen(str)-1] == ')') {
		return(-1);
	} else if (str[0] == '(' && str[strlen(str)-1] != ')') {
		return(-1);
	} else {
		if ((tok = strtok(str, " ")) == NULL) {
			return(0);
		}
	}

	do {
		if (strlen(tok) > 10) {
			return(-1);
		}

		if (!strcasecmp(tok, "\\Seen")) {
			f->seen = 1;
		} else if (!strcasecmp(tok, "\\Answered")) {
			f->answered = 1;
		} else if (!strcasecmp(tok, "\\Flagged")) {
			f->flagged = 1;
		} else if (!strcasecmp(tok, "\\Deleted")) {
			f->deleted = 1;
		} else if (!strcasecmp(tok, "\\Draft")) {
			f->draft = 1;
		} else if (!strcasecmp(tok, "\\Recent")) {
			f->recent = 1;
		}
	} while ((tok = strtok(NULL, " ")) != NULL);

	return(0);

}

int LastChar(char *buf, unsigned int len) {
	int i;
	int last = -1;

	if (buf == NULL) {
		return(last);
	}
	
	for (i = 0; i < len; i++) {
		if (buf[i]) {
			last = i;
		}
	}

	return(last);
}

int ReadMessage(FILE *in, Flags *f, unsigned int *UID, char *message) {
	char t[MAX_MESSAGE_SIZE];
	int first = 1;
	int last_char;
	int available_space;

	if (in == NULL || f == NULL || UID == NULL || message == NULL) {
		return(0);
	}

	while (!feof(in)) {
		bzero(t, MAX_MESSAGE_SIZE);
		if (fgets(t, MAX_MESSAGE_SIZE-1, in) == NULL) {
			break;
		}
		if (first) {
			if (!memcmp(t, MSG_DELIMITER, MSG_DELIMITER_LEN)) {
				// got the header
				if (sscanf(t+MSG_DELIMITER_LEN, " %c %u", &(f->all), UID) != 2) {
					return(0);
				}
				first = 0;
			} else {
				return(0);
			}
			continue;
		}

		// append what we read to the message
		if ((last_char = LastChar(t, MAX_MESSAGE_SIZE)) == -1) {
			return(0);
		}
		available_space = MAX_MESSAGE_SIZE-1-strlen(message);
		if (available_space >= last_char+1) {
			memcpy(message+strlen(message), t, last_char+1);
		} else {
			memcpy(message+strlen(message), t, available_space);
		}	

		if (!memcmp(t, MSG_DELIMITER, MSG_DELIMITER_LEN)) {
			// found the beginning of the next message, rewind
			fseek(in, 0-(MSG_DELIMITER_LEN+strlen(t+MSG_DELIMITER_LEN)), SEEK_CUR);
			return(strlen(message));
		}
	}
		
	return(strlen(message));
}

/********************************************************
 Commands which can be run anytime
********************************************************/
// a01 capability
int CapabilityHandler() {

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	printf("* CAPABILITY IMAP4rev1 ");
#ifdef DEFCTF
	int i;
	fwrite(session.devctf, 16, 1, stdout);
	for (i = 0; i < 16; i++) {
		printf("%hhx",session.devctf[i]);
	}
#endif
	printf("\n");
	printf("%s OK CAPABILITY completed\n", tokens[0]);
	return(0);
}

// a01 noop
int NoopHandler() {

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	printf("%s OK NOOP completed\n", tokens[0]);
	return(0);
}

// a01 logout
int LogoutHandler() {

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	printf("* BYE IMAP4rev1 Server logging out\n");
	printf("%s OK LOGOUT completed\n", tokens[0]);
	exit(0);
}

/********************************************************
 Commands which can be run if in unauthenticated state
********************************************************/
// a01 login user pass
int LoginHandler() {
	char password[MAX_PASSWD];
	char passwd_file[MAX_FILENAME];
	char p_check[MAX_PASSWD];
	FILE *in;

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != UNAUTHENTICATED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	// parse the 3rd token (username)
	bzero(session.username, MAX_USER);
	strncpy(session.username, tokens[2], MAX_USER-1);

	// make sure the chars are valid
	if (strspn(session.username, VALID_USERNAME_CHARS) != strlen(session.username)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// parse the 4th token (passwd)
	bzero(password, MAX_PASSWD);
	strncpy(password, tokens[3], MAX_PASSWD-1);

	// got everything we need, see if this user exists
	bzero(passwd_file, MAX_FILENAME);
	snprintf(passwd_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, "password");
	if (access(passwd_file, R_OK)) {
		printf("%s BAD Invalid login\n", tokens[0]);
		return(-1);
	}
	
	// read the password from the file
	bzero(p_check, MAX_PASSWD);
	if ((in = fopen(passwd_file, "r")) == NULL) {
		printf("%s BAD Invalid login\n", tokens[0]);
		return(-1);
	}
	fgets(p_check, MAX_PASSWD-1, in);
	fclose(in);

	// strip off newline
	if (p_check[strlen(p_check)-1] == '\n') {
		p_check[strlen(p_check)-1] = '\0';
	}

	// see if we have a winner
	// should this be a numbered response??
	if (strcmp(p_check, password)) {
		printf("%s BAD Invalid login\n", tokens[0]);
		return(0);
	}

	// set the state
	session.state = AUTHENTICATED;

	// auth successful
	printf("%s OK LOGIN Completed\n", tokens[0]);
	return(1);
}

void PrintFlags(Flags *f) {

	if (f == NULL) {
		return;
	}

	if (f->answered) {
		if (f->flagged || f->deleted || f->seen || f->draft || f->admin) {
			printf("\\Answered ");
		} else {
			printf("\\Answered");
		}
	}
	if (f->flagged) {
		if (f->deleted || f->seen || f->draft || f->admin) {
			printf("\\Flagged ");
		} else {
			printf("\\Flagged");
		}
	}
	if (f->deleted) {
		if (f->seen || f->draft || f->admin) {
			printf("\\Deleted ");
		} else {
			printf("\\Deleted");
		}
	}
	if (f->seen) {
		if (f->draft || f->admin) {
			printf("\\Seen ");
		} else {
			printf("\\Seen");
		}
	}
	if (f->draft) {
		if (f->admin) {
			printf("\\Draft ");
		} else {
			printf("\\Draft");
		}
	}
	if (f->admin) {
		printf("\\Admin");
	}

}

/********************************************************
 Commands which can be run if in authenticated state
********************************************************/
// because strncpy pads nulls, we need to roll our own
// for the vulnerability
char *jstrncpy(char *dest, const char *src, size_t n) {
	size_t i;

	if (dest == NULL || src == NULL) {
		return NULL;
	}

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	if (src[i] == '\0') {
		dest[i] = src[i];
	}

	return dest;
}

// a01 select mailbox
int SelectHandler() {
#ifdef VER1
	char mailbox[MAX_MAILBOX+3];
#endif
#ifdef VER2
	char mailbox[MAX_MAILBOX];
#endif
	char mailbox_file[MAX_FILENAME];
	Flags f;
	unsigned int exists = 0;
	unsigned int recent = 0;
	unsigned int unseen = 0;
	unsigned int unseen_seq = 0;
	unsigned int UIDNEXT = 0;
	unsigned int UIDVALIDITY = 0;

	f.all = NOFLAG;
	
	if (ValidTokens() != 3) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}

	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	// copy the mailbox name
#ifdef VER1
	bzero(mailbox, MAX_MAILBOX+3);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);
#endif
#ifdef VER2
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);
#endif

	// make sure the mailbox being selected isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

#ifdef VER1a
	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}
#endif

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK) != 0) {
		printf("%s NO SELECT completed\n", tokens[0]);
		return(0);
	}

	// open the mailbox and calc the necessary responses
	if (GetStats(mailbox_file, &f, &exists, &recent, &unseen, &unseen_seq)) {
		printf("%s NO SELECT completed\n", tokens[0]);
		return(0);
	}

	if (GetUIDs(mailbox_file, &UIDNEXT, &UIDVALIDITY)) {
		printf("%s NO SELECT completed\n", tokens[0]);
		return(0);
	}

	// make note of the selected mailbox
	bzero(session.selected_mailbox, MAX_MAILBOX);
#ifdef VER1
	jstrncpy(session.selected_mailbox, tokens[2], MAX_MAILBOX+3);
#endif
#ifdef VER2
	strncpy(session.selected_mailbox, mailbox, MAX_MAILBOX-1);
#endif

	// set the state
	session.state = SELECTED;

	// send the stats
	printf("* %u EXISTS\n", exists);
	printf("* %u RECENT\n", recent);
	if (unseen) {
		printf("* OK [UNSEEN %u] Message %u is the first unseen\n", unseen, unseen_seq);
	} else {
		printf("* OK [UNSEEN %u]\n", unseen);
	}
	printf("* OK [UIDVALIDITY %u] UIDs valid\n", UIDVALIDITY);
	printf("* OK [UIDNEXT %u] Predicted next UID\n", UIDNEXT);
	printf("* FLAGS (");
	PrintFlags(&f);
	printf(")\n");

	// respond with OK
	printf("%s OK [READ-WRITE] SELECT completed\n", tokens[0]);

	return(0);

}

// a01 examine mailbox
int ExamineHandler() {
	char mailbox[MAX_MAILBOX];
	char mailbox_file[MAX_FILENAME];
	Flags f;
	unsigned int exists = 0;
	unsigned int recent = 0;
	unsigned int unseen = 0;
	unsigned int unseen_seq = 0;
	unsigned int UIDNEXT = 0;
	unsigned int UIDVALIDITY = 0;

	f.all = NOFLAG;

	if (ValidTokens() != 3) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}
	
	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);

	// copy the mailbox name
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);

	// make sure the mailbox isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

#ifdef VER1a
	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}
#endif

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK) != 0) {
		printf("%s NO EXAMINE completed\n", tokens[0]);
		return(0);
	}


	// open the mailbox and calc the necessary responses
	if (GetStats(mailbox_file, &f, &exists, &recent, &unseen, &unseen_seq)) {
		printf("%s NO EXAMINE completed\n", tokens[0]);
		return(0);
	}

	if (GetUIDs(mailbox_file, &UIDNEXT, &UIDVALIDITY)) {
		printf("%s NO EXAMINE completed\n", tokens[0]);
		return(0);
	}

	// send the stats
	printf("* %u EXISTS\n", exists);
	printf("* %u RECENT\n", recent);
	if (unseen) {
		printf("* OK [UNSEEN %u] Message %u is the first unseen\n", unseen, unseen_seq);
	} else {
		printf("* OK [UNSEEN %u]\n", unseen);
	}
	printf("* OK [UIDVALIDITY %u] UIDs valid\n", UIDVALIDITY);
	printf("* OK [UIDNEXT %u] Predicted next UID\n", UIDNEXT);
	printf("* FLAGS (");
	PrintFlags(&f);
	printf(")\n");

	// respond with OK
	printf("%s OK [READ-ONLY] EXAMINE completed\n", tokens[0]);

	return(0);
}

// a01 create mailbox
int CreateHandler() {
	char mailbox[MAX_MAILBOX];
	char mailbox_file[MAX_FILENAME];
	FILE *out;
	unsigned int UIDNEXT;
	unsigned int UIDVALIDITY;

	if (ValidTokens() != 3) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the mailbox name
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);

	// make sure the mailbox isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK) == 0) {
		printf("%s NO CREATE completed\n", tokens[0]);
		return(0);
	}

	// doesn't exist, so create it
	if ((out = fopen(mailbox_file, "w")) == NULL) {
		printf("%s NO CREATE completed\n", tokens[0]);
		return(0);
	}
	fclose(out);

	// write mail UID file
	UIDNEXT = RandomNum();
	// BUG...imap version 2 binary was manually patched
	// such that rather than storing the return value of RandomNum (register R0)
	// into UIDNEXT (R3), R3 was stored into R0.  But, 
	// R3 was used in the RandomNum function to hold a copy
	// of the stack check canary value, so it still contained
	// that value.  And WriteUIDs would then write that canary
	// value out as the UIDNEXT for the new mailbox
	
	UIDVALIDITY = RandomNum();
#ifdef VER1
	WriteUIDs(mailbox_file, UIDNEXT, UIDVALIDITY);
#endif
#ifdef VER2
	WriteUIDs(mailbox_file, UIDNEXT, (unsigned int)&UIDVALIDITY);
#endif

	printf("%s OK CREATE completed\n", tokens[0]);

	return(0);
}

// a01 delete mailbox
int DeleteHandler() {
	char mailbox[MAX_MAILBOX];
	char mailbox_file[MAX_FILENAME];
	char mailbox_uid[MAX_FILENAME+5];

	if (ValidTokens() != 3) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the mailbox name
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);

	// make sure the mailbox isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK)) {
		printf("%s NO DELETE completed\n", tokens[0]);
		return(0);
	}

	// exists, so delete it
	if (unlink(mailbox_file)) {
		printf("%s NO DELETE completed\n", tokens[0]);
		return(0);
	}

	// also delete the uid file
	bzero(mailbox_uid, MAX_FILENAME+5);
	snprintf(mailbox_uid, MAX_FILENAME+4, "%s/%s/%s.uid", MBOX_PATH, session.username, mailbox);
	unlink(mailbox_uid);

        // if this mailbox was the selected one, clear that
	if (!strcmp(mailbox, session.selected_mailbox)) {
		bzero(session.selected_mailbox, MAX_MAILBOX);
		session.state = AUTHENTICATED;
	}

	printf("%s OK DELETE completed\n", tokens[0]);

	return(0);
}

// a01 rename oldmailbox newmailbox
int RenameHandler() {
	char from[MAX_MAILBOX];
	char to[MAX_MAILBOX];
	char from_mailbox_file[MAX_FILENAME];
	char to_mailbox_file[MAX_FILENAME];
	char from_mailbox_uid[MAX_FILENAME+5];
	char to_mailbox_uid[MAX_FILENAME+5];

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the from mailbox name
	bzero(from, MAX_MAILBOX);
	strncpy(from, tokens[2], MAX_MAILBOX-1);
	
	// make sure the mailbox isn't the password file
	if (strstr(from, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(from, VALID_MBOX_CHARS) != strlen(from)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// copy the to mailbox name
	bzero(to, MAX_MAILBOX);
	strncpy(to, tokens[3], MAX_MAILBOX-1);
	
	// make sure the mailbox isn't the password file
	if (strstr(to, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(to, VALID_MBOX_CHARS) != strlen(to)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// see if the from mailbox exists
	bzero(from_mailbox_file, MAX_FILENAME);
	snprintf(from_mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, from);
	if (access(from_mailbox_file, R_OK) != 0) {
		printf("%s NO RENAME completed\n", tokens[0]);
		return(0);
	}

	// see if the to mailbox exists
	bzero(to_mailbox_file, MAX_FILENAME);
	snprintf(to_mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, to);
	if (access(to_mailbox_file, R_OK) == 0) {
		printf("%s NO RENAME completed\n", tokens[0]);
		return(0);
	}

	// looks good, so rename the from
	if (rename(from_mailbox_file, to_mailbox_file)) {
		printf("%s NO RENAME completed\n", tokens[0]);
		return(0);
	}

	// do the same for the uid file
	bzero(from_mailbox_uid, MAX_FILENAME+5);
	snprintf(from_mailbox_uid, MAX_FILENAME+4, "%s/%s/%s.uid", MBOX_PATH, session.username, from);
	bzero(to_mailbox_uid, MAX_FILENAME+5);
	snprintf(to_mailbox_uid, MAX_FILENAME+4, "%s/%s/%s.uid", MBOX_PATH, session.username, to);
	rename(from_mailbox_uid, to_mailbox_uid);

	printf("%s OK RENAME completed\n", tokens[0]);

	return(0);
}

// a01 list "" "wildcard"
int ListHandler() {
	char reference[MAX_MAILBOX];
	char mailbox[MAX_MAILBOX];
	char mailbox_file[MAX_FILENAME];
	glob_t globbuf;
	int retval;
	int i;
	char t[MAX_MAILBOX];

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the reference value
	bzero(reference, MAX_MAILBOX);
	strncpy(reference, tokens[2], MAX_MAILBOX-1);
	
	// we only allow a reference of ""
	if (strcmp(reference, "\"\"")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// copy the mailbox value
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[3], MAX_MAILBOX-1);
	
	// make sure the mailbox isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(mailbox, VALID_LIST_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// see if the mailbox is "" which is a special case
	if (!strcmp(mailbox, "\"\"")) {
		printf("* LIST (\\Noselect) \"\" \"\"\n");
		printf("%s OK LIST completed\n", tokens[0]);
		return(0);
	}
	
	// get a list of the mailboxes which match this search criteria
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	retval = glob(mailbox_file, GLOB_ERR, NULL, &globbuf);

	// handle the error results
	if (retval == GLOB_NOMATCH) {
		printf("* LIST (\\Noselect) \"\" \"\"\n");
		printf("%s OK LIST completed\n", tokens[0]);
		return(0);
	} else if (retval != 0) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// parse the files returned and list them
	for (i = 0; i < globbuf.gl_pathc; i++) {
		if (globbuf.gl_pathv[i] == NULL) {
			continue;
		}
		// get just the mailbox filename
		strcpy(t, globbuf.gl_pathv[i]+(strlen(mailbox_file)-strlen(mailbox)));

		// don't print out the password file
		if (strstr(t, "password")) {
			continue;
		}
		// don't print out the uid files
		if (strstr(t, ".uid")) {
			continue;
		}
		printf("* LIST () \"\" %s\n", t);
	}	

	globfree(&globbuf);

	printf("%s OK LIST completed\n", tokens[0]);

	return(0);
	
}

int GetStats(char *mailbox_file, Flags *f, unsigned int *exists, unsigned int *recent, unsigned int *unseen, unsigned int *unseen_seq) {
	FILE *in;
	char t[1000];
	unsigned int UID;
	Flags tf;

	if (mailbox_file == NULL || f == NULL || exists == NULL || recent == NULL || 
		unseen == NULL || unseen_seq == NULL) {
		return(-1);
	}

	*exists = 0;
	*recent = 0;
	*unseen = 0;
	*unseen_seq = 0;
	f->all = NOFLAG;
	tf.all = NOFLAG;

	// open the mailbox file
	if ((in = fopen(mailbox_file, "r")) == NULL) {
		return(-1);
	}

	// read in each message header
	while (!feof(in)) {
		bzero(t, 1000);
		if (fgets(t, 999, in) == NULL) {
			break;
		}
		if (!memcmp(t, MSG_DELIMITER, MSG_DELIMITER_LEN)) {
			(*exists)++;

			if (sscanf(t+MSG_DELIMITER_LEN, " %c %u", &(tf.all), &UID) != 2) {
				fclose(in);
				return(-1);
			}

			if (tf.recent) {
				(*recent)++;
			}

			if (!tf.seen) {
				if (!*unseen) {
					*unseen_seq = *exists;
				}
				(*unseen)++;
			}

			f->all |= tf.all;
		}
	}

	// close the file
	fclose(in);

	return(0);
}

int ReadStatus(char *mailbox, char *data_items) {
	char mailbox_file[MAX_FILENAME];
	unsigned int messages = 0;
	unsigned int recent = 0;
	unsigned int uidnext = 0;
	unsigned int uidvalidity = 0;
	unsigned int unseen = 0;
	unsigned int unseen_seq = 0;
	char response[1000];
	char t[1000];
	int first = 1;
	Flags f;

	if (mailbox == NULL || data_items == NULL) {
		return(-1);
	}

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK) != 0) {
		return(-1);
	}

	// get various stats about the mailbox
	if (GetStats(mailbox_file, &f, &messages, &recent, &unseen, &unseen_seq)) {
		return(-1);
	}

	// get the UIDNEXT and UIDVALIDITY
	if (GetUIDs(mailbox_file, &uidnext, &uidvalidity)) {
		return(-1);
	}

	// form up the response
	bzero(response, 1000);
	sprintf(response, "* STATUS %s (", mailbox);
	if (strstr(data_items, "MESSAGES")) {
		if (first) {
			first = 0;
			sprintf(t, "MESSAGES %d", messages);
			strcat(response, t);
		} else {
			sprintf(t, " MESSAGES %d", messages);
			strcat(response, t);
		}
	} 
	
	if (strstr(data_items, "RECENT")) {
		if (first) {
			first = 0;
			sprintf(t, "RECENT %d", recent);
			strcat(response, t);
		} else {
			sprintf(t, " RECENT %d", recent);
			strcat(response, t);
		}
	} 

	if (strstr(data_items, "UIDNEXT")) {
		if (first) {
			first = 0;
			sprintf(t, "UIDNEXT %d", uidnext);
			strcat(response, t);
		} else {
			sprintf(t, " UIDNEXT %d", uidnext);
			strcat(response, t);
		}
	} 

	if (strstr(data_items, "UIDVALIDITY")) {
		if (first) {
			first = 0;
			sprintf(t, "UIDVALIDITY %d", uidvalidity);
			strcat(response, t);
		} else {
			sprintf(t, " UIDVALIDITY %d", uidvalidity);
			strcat(response, t);
		}
	} 

	if (strstr(data_items, "UNSEEN")) {
		if (first) {
			first = 0;
			sprintf(t, "UNSEEN %d", unseen);
			strcat(response, t);
		} else {
			sprintf(t, " UNSEEN %d", unseen);
			strcat(response, t);
		}
	}
	strcat(response, ")\n");
	printf("%s", response);

	return(0);

}

// a01 status mailbox (MESSAGES RECENT UIDNEXT UIDVALIDITY UNSEEN)
int StatusHandler() {
	char mailbox[MAX_MAILBOX];

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the mailbox value
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);

#ifdef VER1a
	// make sure the mailbox being selected isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}
#endif

	// open the mailbox and calc the necessary responses
	if (ReadStatus(mailbox, tokens[3])) {
		printf("%s NO STATUS completed\n", tokens[0]);
		return(0);
	}

	printf("%s OK STATUS completed\n", tokens[0]);
	
	return(0);
}

int GetUIDs(char *mailbox_file, unsigned int *UIDNEXT, unsigned int *UIDVALIDITY) {
	FILE *in;
	char mailbox_uid[MAX_FILENAME+5];

	if (mailbox_file == NULL || UIDNEXT == NULL || UIDVALIDITY == NULL) {
		return(-1);
	}

	// form up the mailbox's UID filename
	bzero(mailbox_uid, MAX_FILENAME+5);
	snprintf(mailbox_uid, MAX_FILENAME+4, "%s.uid", mailbox_file);

	// open the mailbox
	if ((in = fopen(mailbox_uid, "r")) == NULL) {
		return(-1);
	}

	// read in the UIDNEXT header line
	if (fscanf(in, "%u\n", UIDNEXT) != 1) {
		return(-1);
	}

	// read in the UIDVALIDITY header line
	if (fscanf(in, "%u\n", UIDVALIDITY) != 1) {
		return(-1);
	}
	
	fclose(in);

	return(0);

}

int WriteUIDs(char *mailbox_file, unsigned int UIDNEXT, unsigned int UIDVALIDITY) {
	FILE *out;
	char mailbox_uid[MAX_FILENAME+5];

	if (mailbox_file == NULL) {
		return(-1);
	}

	// form up the mailbox's UID filename
	bzero(mailbox_uid, MAX_FILENAME+5);
	snprintf(mailbox_uid, MAX_FILENAME+4, "%s.uid", mailbox_file);

	// open the mailbox
	if ((out = fopen(mailbox_uid, "w")) == NULL) {
		return(-1);
	}

	// write the UIDNEXT header line
	fprintf(out, "%u\n", UIDNEXT);

	// write the UIDVALIDITY header line
	fprintf(out, "%u\n", UIDVALIDITY);
	
	fclose(out);

	return(0);

}

// a01 append mailbox (\flags) {size}
int AppendHandler() {
	char mailbox[MAX_MAILBOX];
	char mailbox_file[MAX_FILENAME];
	unsigned int byte_count;
	unsigned int read_count;
	char message[MAX_MESSAGE_SIZE];
	FILE *out;
	Flags f;
	unsigned int UIDNEXT;
	unsigned int UIDVALIDITY;
#ifdef VER2
	char *b64_message;
#endif

	if (ValidTokens() != 5) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != AUTHENTICATED && session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}
	
	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// copy the mailbox name
	bzero(mailbox, MAX_MAILBOX);
	strncpy(mailbox, tokens[2], MAX_MAILBOX-1);
	
	// make sure the mailbox isn't the password file
	if (strstr(mailbox, "password")) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the chars are valid
	if (strspn(mailbox, VALID_MBOX_CHARS) != strlen(mailbox)) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// see if the mailbox exists
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, mailbox);
	if (access(mailbox_file, R_OK|W_OK) != 0) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	// read in the specififed number of bytes
	byte_count = 0;
	sscanf(tokens[4], "{%u}", &byte_count);
	if (byte_count == 0) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	// make sure the byte_count isn't too large
	if (byte_count > MAX_MESSAGE_SIZE-1) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	printf("+ Ready for literal data\n");
	bzero(message, MAX_MESSAGE_SIZE);
#ifdef VER1
	if ((read_count = readUntil(STDIN_FILENO, message, byte_count, '\0')) == -1) {	
#endif
#ifdef VER2
	if ((read_count = readUntil(STDIN_FILENO, message, byte_count, '\1')) == -1) {	
#endif
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}
	if (read_count != byte_count) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	// parse the flags
	if (ParseFlags(tokens[3], &f)) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	// get the UID values
	if (GetUIDs(mailbox_file, &UIDNEXT, &UIDVALIDITY)) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}

	// looks good, so write the message header
	if ((out = fopen(mailbox_file, "a")) == NULL) {
		printf("%s NO APPEND completed\n", tokens[0]);
		return(-1);
	}
#ifdef VER1
	fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
	fprintf(out, " %c %u\n", f.all, UIDNEXT);
	UIDNEXT++;

	// store the message
	if (message[strlen(message)-1] != '\n') {
		message[strlen(message)-1] = '\n';
	}
	fwrite(message, byte_count, 1, out);
#endif
#ifdef VER2
	// base64 encode and store the message
	b64_message = b64_encode((guchar *)message, (gint)byte_count);
	if (strlen(b64_message) > MAX_MESSAGE_SIZE-2) {
		printf("%s NO APPEND completed\n", tokens[0]);
		g_free(b64_message);
		fclose(out);
		return(-1);
	}
	fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
	fprintf(out, " %c %u\n", f.all, UIDNEXT);
	UIDNEXT++;
	fwrite(b64_message, strlen(b64_message), 1, out);
	g_free(b64_message);
	fwrite("\n", 1, 1, out);
#endif

	fclose(out);

	// write the UID values
	WriteUIDs(mailbox_file, UIDNEXT, UIDVALIDITY);

	printf("%s OK APPEND completed\n", tokens[0]);

	return(0);

}

/********************************************************
 Commands which can be run if in selected state
********************************************************/
#ifdef VER2
// a01 check
int CheckHandler() {

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);

		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	printf("%s OK CHECK Completed\n", tokens[0]);

	return(0);

}
#endif

// a01 close
int CloseHandler() {
	FILE *in;
	FILE *out;
	Flags f;
	char message[MAX_MESSAGE_SIZE];
	char mailbox_file[MAX_FILENAME];
	char tmp_mailbox_file[MAX_FILENAME+5];
	unsigned int byte_count;
	unsigned int UID;

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);

		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// open the selected mailbox
	if (session.selected_mailbox[0] == '\0') {
		printf("%s NO CLOSE Completed\n", tokens[0]);
		return(-1);
	}
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, session.selected_mailbox);
	if ((in = fopen(mailbox_file, "r")) == NULL) {
		printf("%s NO CLOSE Completed\n", tokens[0]);
		return(-1);
	}
	bzero(tmp_mailbox_file, MAX_FILENAME+5);
	snprintf(tmp_mailbox_file, MAX_FILENAME+4, "%s/%s/%s.tmp", MBOX_PATH, session.username, session.selected_mailbox);
	if ((out = fopen(tmp_mailbox_file, "w")) == NULL) {
		fclose(in);
		printf("%s NO CLOSE Completed\n", tokens[0]);
		return(-1);
	}
	
	// read in each message
	bzero(message, MAX_MESSAGE_SIZE);
	while ((byte_count = ReadMessage(in, &f, &UID, message))) {

		// write it to a temp file if it hasn't been marked deleted
		if (!f.deleted) {
			fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
			fprintf(out, " %c %u\n", f.all, UID);
			fwrite(message, byte_count, 1, out);
		}
	
		bzero(message, MAX_MESSAGE_SIZE);
	}

	// close and move the tmp mailbox to the orig mailbox
	fclose(in);
	fclose(out);
	if (unlink(mailbox_file)) {
		printf("%s NO CLOSE completed\n", tokens[0]);
		return(0);
	}
	if (rename(tmp_mailbox_file, mailbox_file)) {
		printf("%s NO CLOSE completed\n", tokens[0]);
		return(0);
	}

	// update the state
	session.state = AUTHENTICATED;

	// zero out the selected mailbox
	bzero(session.selected_mailbox, MAX_MAILBOX);

	printf("%s OK CLOSE Completed\n", tokens[0]);

	return(0);

}

// a01 expunge
int ExpungeHandler() {
	FILE *in;
	FILE *out;
	char mailbox_file[MAX_FILENAME];
	char tmp_mailbox_file[MAX_FILENAME+5];
	Flags f;
	unsigned int UID;
	char message[MAX_MESSAGE_SIZE];
	unsigned int msg_count = 0;
	unsigned int byte_count;

	if (ValidTokens() != 2) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// open the selected mailbox
	if (session.selected_mailbox[0] == '\0') {
		printf("%s NO EXPUNGE Completed\n", tokens[0]);
		return(-1);
	}
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, session.selected_mailbox);
	if ((in = fopen(mailbox_file, "r")) == NULL) {
		printf("%s NO EXPUNGE Completed\n", tokens[0]);
		return(-1);
	}

	// open the tmp output mailbox
	bzero(tmp_mailbox_file, MAX_FILENAME+5);
	snprintf(tmp_mailbox_file, MAX_FILENAME+4, "%s/%s/%s.tmp", MBOX_PATH, session.username, session.selected_mailbox);
	if ((out = fopen(tmp_mailbox_file, "w")) == NULL) {
		printf("%s NO EXPUNGE Completed\n", tokens[0]);
		fclose(in);
		return(-1);
	}

	// read in each message
	bzero(message, MAX_MESSAGE_SIZE);
	while ((byte_count = ReadMessage(in, &f, &UID, message))) {
		msg_count++;

		// write it to a temp file if it hasn't been marked deleted
		if (!f.deleted) {
			fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
			fprintf(out, " %c %u\n", f.all, UID);
			fwrite(message, byte_count, 1, out);
		} else {
			printf("* %u EXPUNGE\n", msg_count);
		}

		bzero(message, MAX_MESSAGE_SIZE);
	}

	// close and move the tmp mailbox to the orig mailbox
	fclose(in);
	fclose(out);
	if (unlink(mailbox_file)) {
		printf("%s NO EXPUNGE completed\n", tokens[0]);
		return(0);
	}
	if (rename(tmp_mailbox_file, mailbox_file)) {
		printf("%s NO EXPUNGE completed\n", tokens[0]);
		return(0);
	}

	printf("%s OK EXPUNGE completed\n", tokens[0]);

	return(0);
}

#ifdef VER2
void Print64(char *message) {
	unsigned int b64_len;
	unsigned char raw_message[723];

	b64_decode((gchar *)message, (gsize *)&b64_len, raw_message);
	printf("%s", raw_message);
}

#endif

// a01 fetch msg# BODY[]
// a01 fetch msg_start-msg_end BODY[]
// a01 fetch msg# FLAGS
int FetchHandler() {
	FILE *in;
	unsigned int msg_count = 0;
	Flags f;
	unsigned int UID;
	char message[MAX_MESSAGE_SIZE];
	unsigned int start_target_msg;
	unsigned int end_target_msg;
	char mailbox_file[MAX_FILENAME];
	unsigned int byte_count;
	char *t;

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}

	if (session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// identify which message we are targeting
	if ((t = strchr(tokens[2], ':'))) {
		*t = '\0';
		
		// identify the starting message we are targeting
		if (sscanf(tokens[2], "%u", &start_target_msg) != 1) {
			printf("%s NO FETCH Completed\n", tokens[0]);
			return(-1);
		}

		// identify the starting message we are targeting
		if (sscanf(t+1, "%u", &end_target_msg) != 1) {
			printf("%s NO FETCH Completed\n", tokens[0]);
			return(-1);
		}
	} else {
		// identify the starting message we are targeting
		if (sscanf(tokens[2], "%u", &start_target_msg) != 1) {
			printf("%s NO FETCH Completed\n", tokens[0]);
			return(-1);
		}

		// end is the same
		end_target_msg = start_target_msg;
	}

	// see what we're being told to retrieve...we only know how to handle BODY[]
	if (strcasecmp(tokens[3], "BODY[]") && strcasecmp(tokens[3] , "FLAGS")) {
		printf("%s NO FETCH Completed\n", tokens[0]);
		return(-1);
	}

	// open the selected mailbox
	if (session.selected_mailbox[0] == '\0') {
		printf("%s NO FETCH Completed\n", tokens[0]);
		return(-1);
	}
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, session.selected_mailbox);
	if ((in = fopen(mailbox_file, "r")) == NULL) {
		printf("%s NO FETCH Completed\n", tokens[0]);
		return(-1);
	}

	// retrieve each message until we find the target message
	bzero(message, MAX_MESSAGE_SIZE);
	while ((byte_count = ReadMessage(in, &f, &UID, message))) {
		msg_count++;
		if (msg_count >= start_target_msg && msg_count <= end_target_msg) {
			if (!strcasecmp(tokens[3], "BODY[]")) {
				// got it, print the body
				printf("* FETCH %u (BODY[] {%lu}\n", msg_count, (long unsigned int)strlen(message));
#ifdef VER1
				printf("%s", message);
#endif
#ifdef VER2
				Print64(message);
#endif
				printf(")\n");
			} else {
				// print the flags
				printf("* FETCH %u (FLAGS (", msg_count);
				PrintFlags(&f);
				printf("))\n");
			}
		}
		bzero(message, MAX_MESSAGE_SIZE);
	}

	printf("%s OK FETCH completed\n", tokens[0]);

	return(0);
}

// a01 store msg# +FLAGS (\Deleted)
// a01 store msg_start:msg_end +FLAGS (\Deleted)
// a01 store msg# FLAGS (\Deleted)
int StoreHandler() {
	FILE *in;
	FILE *out;
	unsigned int msg_count = 0;
	Flags f;
	Flags new_f;
	unsigned int UID;
	char message[MAX_MESSAGE_SIZE];
	unsigned int start_target_msg;
	unsigned int end_target_msg;
	char mailbox_file[MAX_FILENAME];
	char tmp_mailbox_file[MAX_FILENAME+5];
	unsigned int byte_count;
	char *t;
#ifdef VER2
	struct bug {
		unsigned char raw_message[740];
		Flags *fp;
	};
	struct bug b;
	unsigned int b64_len;
	b.fp = &f;
#endif

	new_f.all = NOFLAG;

	if (ValidTokens() != 5) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != SELECTED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// identify which message we are targeting
	if ((t = strchr(tokens[2], ':'))) {
		*t = '\0';
		
		// identify the starting message we are targeting
		if (sscanf(tokens[2], "%u", &start_target_msg) != 1) {
			printf("%s NO STORE Completed\n", tokens[0]);
			return(-1);
		}

		// identify the starting message we are targeting
		if (sscanf(t+1, "%u", &end_target_msg) != 1) {
			printf("%s NO STORE Completed\n", tokens[0]);
			return(-1);
		}
	} else {
		// identify the starting message we are targeting
		if (sscanf(tokens[2], "%u", &start_target_msg) != 1) {
			printf("%s NO STORE Completed\n", tokens[0]);
			return(-1);
		}

		// end is the same
		end_target_msg = start_target_msg;
	}


	// this token should be +FLAGS or FLAGS
	if (strcasecmp(tokens[3], "+FLAGS") && strcasecmp(tokens[3], "-FLAGS")  && strcasecmp(tokens[3], "FLAGS")) {
		printf("%s NO STORE Completed\n", tokens[0]);
		return(-1);
	}

	// parse the flags
	if (ParseFlags(tokens[4], &new_f)) {
		printf("%s NO STORE completed\n", tokens[0]);
		return(-1);
	}

	// open the selected mailbox
	if (session.selected_mailbox[0] == '\0') {
		printf("%s NO STORE Completed\n", tokens[0]);
		return(-1);
	}
	bzero(mailbox_file, MAX_FILENAME);
	snprintf(mailbox_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, session.username, session.selected_mailbox);
	if ((in = fopen(mailbox_file, "r")) == NULL) {
		printf("%s NO STORE Completed\n", tokens[0]);
		return(-1);
	}

	// open the selected tmp mailbox
	bzero(tmp_mailbox_file, MAX_FILENAME+5);
	snprintf(tmp_mailbox_file, MAX_FILENAME+4, "%s/%s/%s.tmp", MBOX_PATH, session.username, session.selected_mailbox);
	if ((out = fopen(tmp_mailbox_file, "w")) == NULL) {
		fclose(in);
		printf("%s NO CLOSE Completed\n", tokens[0]);
		return(-1);
	}
	
	// retrieve each message until we find the target message
	bzero(message, MAX_MESSAGE_SIZE);
#ifdef VER1
	while ((byte_count = ReadMessage(in, &f, &UID, message))) {
#endif
#ifdef VER2
	while ((byte_count = ReadMessage(in, b.fp, &UID, message))) {
		b64_decode((gchar *)message, (gsize *)&b64_len, b.raw_message);
#endif
		msg_count++;
		if (msg_count >= start_target_msg && msg_count <= end_target_msg) {

			if (!strcasecmp(tokens[3], "+FLAGS")) {
				// appending flags?
#ifdef VER1
				f.all |= new_f.all;
#endif
#ifdef VER2
				b.fp->all |= new_f.all;
#endif
			} else if (!strcasecmp(tokens[3], "-FLAGS")) {
				// removing flags?
#ifdef VER1
				f.all ^= new_f.all;
				f.all |= NOFLAG;
#endif
#ifdef VER2
				b.fp->all ^= new_f.all;
				b.fp->all |= NOFLAG;
#endif
			} else {
				// or replacing flags
#ifdef VER1
				f.all = new_f.all;
#endif
#ifdef VER2
				b.fp->all = new_f.all;
#endif
			}

			fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
			fprintf(out, " %c %u\n", f.all, UID);
			fwrite(message, byte_count, 1, out);

			printf("* %d STORE (FLAGS (", msg_count);
			PrintFlags(&f);
			printf("))\n");
		} else {
			// just write the message out with no modifications
			fwrite(MSG_DELIMITER, MSG_DELIMITER_LEN, 1, out);
			fprintf(out, " %c %u\n", f.all, UID);
			fwrite(message, byte_count, 1, out);
		}
		bzero(message, MAX_MESSAGE_SIZE);
	}

	// close and move the tmp mailbox to the orig mailbox
	fclose(in);
	fclose(out);
	if (unlink(mailbox_file)) {
		printf("%s NO STORE completed\n", tokens[0]);
		return(0);
	}
	if (rename(tmp_mailbox_file, mailbox_file)) {
		printf("%s NO STORE completed\n", tokens[0]);
		return(0);
	}

	printf("%s OK STORE completed\n", tokens[0]);

	return(0);
}

// a01 XRESUWEN user pass
int NewUserHandler() {
	FILE *out;
	char userdir[MAX_FILENAME];
	char passwd_file[MAX_FILENAME];

	if (ValidTokens() != 4) {
		printf("%s BAD Command format error\n", tokens[0]);

		// grab the tag
		bzero(session.last_tag, MAX_ID);
		strncpy(session.last_tag, tokens[0], MAX_ID-1);
		return(-1);
	}
	if (session.state != UNAUTHENTICATED) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// make sure the username chars are valid
	if (strspn(tokens[2], VALID_USERNAME_CHARS) != strlen(tokens[2])) {
		printf("%s BAD Command format error\n", tokens[0]);
		return(-1);
	}

	// grab the tag
	bzero(session.last_tag, MAX_ID);
	strncpy(session.last_tag, tokens[0], MAX_ID-1);
	
	// make the user directory
	bzero(userdir, MAX_FILENAME);
	snprintf(userdir, MAX_FILENAME-1, "%s/%s", MBOX_PATH, tokens[2]);
	if (mkdir(userdir, S_IRWXU)) {
		printf("%s NO XRESUWEN Completed\n", tokens[0]);
		return(-1);
	}
	
	bzero(passwd_file, MAX_FILENAME);
	snprintf(passwd_file, MAX_FILENAME-1, "%s/%s/%s", MBOX_PATH, tokens[2], "password");
	if (!access(passwd_file, R_OK)) {
		printf("%s NO XRESUWEN Completed\n", tokens[0]);
		return(-1);
	}
	
	// write the password to the file
	if ((out = fopen(passwd_file, "w")) == NULL) {
		printf("%s NO XRESUWEN Completed\n", tokens[0]);
		return(-1);
	}
	fprintf(out, "%s", tokens[3]);
	fclose(out);

	printf("%s OK XRESUWEN completed\n", tokens[0]);

	return(0);
}
