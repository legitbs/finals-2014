//#define MBOX_PATH "/home/imap/mboxes"
#define MBOX_PATH "/home/joe/0x28thieves/lbs/imap/mboxes"
#define MAX_CMD 1000
#define MAX_ID 20
#define MAX_USER 20
#define MAX_PASSWD 32
#define MAX_FILENAME 190
#define MAX_MESSAGE_SIZE 1000
#define MAX_FLAG_SIZE 70
#define MAX_MAILBOX 100

#define UNAUTHENTICATED 0
#define AUTHENTICATED 1
#define SELECTED 2

#define VALID_USERNAME_CHARS "abcdefghijlkmnopqrstuvwxyz01234567890"
#define VALID_MBOX_CHARS "abcdefghijlkmnopqrstuvwxyz01234567890/"
#define VALID_LIST_CHARS "abcdefghijlkmnopqrstuvwxyz01234567890*%\"/"
#define MSG_DELIMITER "\0\0\0\0\0\0\0\0\0\0"
#define MSG_DELIMITER_LEN 10
#define MAX_TOKENS 50

struct _session {
        char selected_mailbox[MAX_MAILBOX];
        char username[MAX_USER];
        int state;
        char last_tag[MAX_ID];
	unsigned char devctf[16];
};

