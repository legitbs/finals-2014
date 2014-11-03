typedef struct _ClientCommands {
	char *command;
	int (*handler)(void);
} ClientCommands;

#define NOFLAG 0x80
typedef union _flags {
	struct {
		unsigned char admin:1;
		unsigned char seen:1;
		unsigned char answered:1;
		unsigned char flagged:1;
		unsigned char deleted:1;
		unsigned char draft:1;
		unsigned char recent:1;
		unsigned char res2:1;
	};
	unsigned char all;

} Flags;

int NotImplementedHandler();
int CapabilityHandler();
int NoopHandler();
int LogoutHandler();
int LoginHandler();
int SelectHandler();
int ExamineHandler();
int CreateHandler();
int DeleteHandler();
int RenameHandler();
int ListHandler();
int LsubHandler();
int StatusHandler();
int AppendHandler();
int CheckHandler();
int CloseHandler();
int ExpungeHandler();
int FetchHandler();
int StoreHandler();
int NewUserHandler();
void PrintFlags();
void free_tokens();
void Banner();
int readUntil(int, char *, int, char);
int GetStats(char *, Flags *, unsigned int *, unsigned int *, unsigned int *, unsigned int *);
int GetUIDs(char *, unsigned int *, unsigned int *);
int WriteUIDs(char *, unsigned int, unsigned int);


