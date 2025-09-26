// Shell.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"


#define HISTORY_SIZE 10

char history[HISTORY_SIZE][100]; // last 10 commands, max length 100
int history_count = 0;           // number of stored commands

int strncmp(const char *s1, const char *s2, int n);
char *strncpy(char *s, const char *t, int n);
/*int strncmp(const char *s1, const char *s2, int n) {
    for(int i = 0; i < n; i++) {
        if(s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0')
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}*/

// basic string functions for xv6 user programs

int strncmp(const char *p, const char *q, int n) {
    for(int i = 0; i < n; i++){
        if(p[i] != q[i] || p[i] == 0 || q[i] == 0)
            return (uchar)p[i] - (uchar)q[i];
    }
    return 0;
}

char *strncpy(char *dst, const char *src, int n) {
    for(int i = 0; i < n; i++){
        dst[i] = src[i];
        if(src[i] == 0) break;
    }
    return dst;
}

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);
void runcmd(struct cmd*) __attribute__((noreturn));

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit(1);

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(1);
    exec(ecmd->argv[0], ecmd->argv);
    fprintf(2, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      fprintf(2, "open %s failed\n", rcmd->file);
      exit(1);
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait(0);
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit(0);
}
// Tab completion helper
void complete_tab(char *buf) {
    int fd;
    struct dirent de;
    int len = strlen(buf);

    fd = open(".", 0);   // open current directory
    if(fd < 0){
        return;
    }

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;

        // check if the start of name matches buf
        if(strncmp(de.name, buf, len) == 0){
            // copy the full name into buf
            strncpy(buf, de.name, DIRSIZ);
            buf[DIRSIZ] = 0;  // null terminate
            break; // stop at first match
        }
    }
    close(fd);
}

// Read input from user with support for tab completion
/*int getcmd(char *buf, int nbuf) {
    int i = 0;
    char c;

    // print prompt
    write(2, "$ ", 2);

    while(i + 1 < nbuf && read(0, &c, 1) == 1){
        if(c == '\n'){ // Enter
            buf[i] = 0;
            write(1, "\n", 1);
            return i;
        } else if(c == 0x7f || c == '\b'){ // Backspace
            if(i > 0){
                i--;
                write(1, "\b \b", 3); // erase from screen
            }
        } else if(c == '\t'){ // Tab -> autocomplete
            complete_tab(buf);
            // redraw buffer after autocomplete
            write(2, "\r$ ", 3);
            write(2, buf, strlen(buf));
            i = strlen(buf); // update position
        } else {
            buf[i++] = c;
            write(1, &c, 1); // echo char
        }
    }

    buf[i] = 0;
    if(i == 0) return -1; // EOF
    return i;
}
*/
int getcmd(char *buf, int nbuf) {
    int i = 0;
    char c;

    // print prompt
    write(2, "$ ", 2);
    memset(buf, 0, nbuf);

    while (i + 1 < nbuf && read(0, &c, 1) == 1) {
        if (c == '\n' || c == '\r') { // Enter key
            buf[i] = 0;
            write(1, "\n", 1);

            // Save command in history if non-empty
            if (i > 0) {
                strncpy(history[history_count % HISTORY_SIZE], buf, 100);
                history[history_count % HISTORY_SIZE][99] = 0;
                history_count++;
            }

            return i;
        } else if (c == 0x7f || c == '\b') { // Backspace
            if (i > 0) {
                i--;
                buf[i] = 0;
                write(1, "\b \b", 3); // erase char from screen
            }
        } else if (c == '\t') { // Tab -> autocomplete
            complete_tab(buf);
            // redraw buffer after autocomplete
            write(2, "\r$ ", 3);
            write(2, buf, strlen(buf));
            i = strlen(buf); // update current position
        } else {
            buf[i++] = c;
            write(2, &c, 1); // echo char
        }
    }

    buf[i] = 0;
    if (i == 0) return -1; // EOF
    return i;
}

int
main(void)
{
    static char buf[100];
    int fd;
    int interactive = 0; // assume non-interactive

    // Check if stdin is console
    int fd_test = open("console", O_RDWR);
    if(fd_test >= 0){
        close(fd_test);
        interactive = 1;
    }

    // Ensure three file descriptors are open
    while ((fd = open("console", O_RDWR)) >= 0) {
        if (fd >= 3) {
            close(fd);
            break;
        }
    }

    // Read and run commands
    while(getcmd(buf, sizeof(buf)) >= 0){
        char *cmd = buf;
        while(*cmd == ' ' || *cmd == '\t')
            cmd++;

        if(*cmd == '\n' || *cmd == 0) // blank command
            continue;

        // Save command to history
        strncpy(history[history_count % HISTORY_SIZE], cmd, 100);
        history[history_count % HISTORY_SIZE][99] = 0;
        history_count++;

        // handle 'cd' command
        if(cmd[0]=='c' && cmd[1]=='d' && cmd[2]==' '){
            cmd[strlen(cmd)-1] = 0; // remove newline
            if(chdir(cmd+3) < 0)
                fprintf(2, "cannot cd %s\n", cmd+3);
        }
        // handle 'wait' command
        else if(strncmp(cmd, "wait", 4) == 0 && (cmd[4]=='\n' || cmd[4]==0)){
            wait(0);
        }
        // handle 'history' command
        else if(strncmp(cmd, "history", 7) == 0 && (cmd[7]=='\n' || cmd[7]==0)){
            int start = history_count > HISTORY_SIZE ? history_count - HISTORY_SIZE : 0;
            for(int i=start; i<history_count; i++){
                printf("%d: %s\n", i+1, history[i % HISTORY_SIZE]);
            }
        }
        // handle other commands
        else{
            if(fork1() == 0){
                runcmd(parsecmd(cmd));
            }
            wait(0); // parent waits for child
        }

        if(interactive)
            printf("$ ");  // print prompt for interactive mode
    }

    exit(0);
}


/* BEFORE HISTORY IMPLEMENTED
int
main(void)
{
    static char buf[100];
    int fd;
    int interactive = 0; // assume non-interactive

    // Check if stdin is console
    int fd_test = open("console", O_RDWR);
    if(fd_test >= 0){
        close(fd_test);
        interactive = 1;
    }

    // Ensure three file descriptors are open
    while ((fd = open("console", O_RDWR)) >= 0) {
        if (fd >= 3) {
            close(fd);
            break;
        }
    }

    // Read and run commands
    while(getcmd(buf, sizeof(buf)) >= 0){
        if(interactive)
            printf("$ ");  // prompt only for interactive mode

        char *cmd = buf;
        while(*cmd == ' ' || *cmd == '\t')
            cmd++;

        if(*cmd == '\n') // blank command
            continue;

        // handle 'cd' command
        if(cmd[0]=='c' && cmd[1]=='d' && cmd[2]==' '){
            cmd[strlen(cmd)-1] = 0; // remove newline
            if(chdir(cmd+3) < 0)
                fprintf(2, "cannot cd %s\n", cmd+3);
        }
        // handle 'wait' command
        else if(strncmp(cmd, "wait", 4) == 0 && (cmd[4]=='\n' || cmd[4]==0)){
            wait(0);
        }
        // handle other commands
        else{
            if(fork1() == 0){
                runcmd(parsecmd(cmd));
            }
            wait(0); // parent waits for child
        }
    }

    exit(0);
}
*/

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE|O_TRUNC, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
