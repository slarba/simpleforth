/* --------------------------------------------------
 * Portable Forth-like language interpreter
 * (c) 2011 Marko Lauronen, Sysart Oy
 * --------------------------------------------------
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <sys/mman.h>

#include <dyncall.h>
#include <dynload.h>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef USE_GC
 #include <gc.h>
 #define MALLOC(x) GC_MALLOC(x)
 #define MALLOC_ATOMIC(x) GC_MALLOC_ATOMIC(x)
 #define REALLOC(ptr,newlen) GC_REALLOC(ptr,newlen)
 #define RUNGC() GC_gcollect()
 #define FREE(x)
#else
 #define MALLOC(x) malloc(x)
 #define MALLOC_ATOMIC(x)
 #define REALLOC(ptr,newlen) realloc(ptr,newlen)
 #define RUNGC()
 #define FREE(x) free(x)
#endif

#define FORTH_VERSION 1

/* ---------- limits ---------- */
#define MAX_WORD_NAME_LEN 32
#define NESTINGSTACK_MAX_DEPTH 16

/* ---------- dictionary entry flags ---------- */
#define BIT(x) (1<<(x))
#define FLAG_HIDDEN      BIT(0)
#define FLAG_IMMED       BIT(1)
#define FLAG_BUILTIN     BIT(2)
#define FLAG_HASARG      BIT(3)
#define FLAG_INLINE      BIT(4)
#define FLAG_DEFERRED    BIT(5)

/* ---------- compiler state ---------- */
#define STATE_IMMEDIATE 0
#define STATE_COMPILE   1

/* the most important type, the cell. MUST be exactly of the pointer length! */
typedef long cell;

/* preprocessor trick to test sizeof(long)==sizeof(void*)? */

/* dictionary definition header. NEVER change the order of these fields, it's crucial! */
typedef struct dict_hdr_t {
  cell                 flags;
  struct dict_hdr_t   *next;
  char                 name[MAX_WORD_NAME_LEN];
} dict_hdr_t;

typedef struct thread_state_t {
  cell killed;
  struct thread_state_t *next;
  void **ip;
  cell *ds;
  void ***rs;
  cell *ts;
  float *fs;
  cell *t0;
  cell *s0;
  void ***r0;
  float *f0;
} thread_state_t;

/* utility structure for creating builtins */
typedef struct builtin_word_t {
  char *name;
  void *code;
  cell flags;
} builtin_word_t;

/* free memory pointers and latest defined word */
static void       *here;              /* working memory */
static void       *here0;             /* beginning of working memory */
static cell       here_size;          /* size of allocated pool */
static dict_hdr_t *latest = NULL;
static thread_state_t *current_thread = NULL;

/* utility functions */
static dict_hdr_t *find_word(const char *name) {
  if(!name) return NULL;
  dict_hdr_t *hdr = latest;
  while(hdr) {
    if(!(hdr->flags & FLAG_HIDDEN) &&
       !strncmp(hdr->name, name, MAX_WORD_NAME_LEN)) 
      return hdr;
    hdr = hdr->next;
  }
  return NULL;
}

static void **cfa(dict_hdr_t *word) {
  return (void**)(word+1);
}

static void comma(cell value) {
  *(cell*)here = value;
  here += sizeof(cell);
}

static dict_hdr_t *create_word(const char *name, cell flags) {
  if(!name) {
    name="\0";  /* for creating unnamed words */
  }

  dict_hdr_t *new = (dict_hdr_t*)here;
  here += sizeof(dict_hdr_t);
  strncpy(new->name, name, MAX_WORD_NAME_LEN);
  new->flags = flags;
  new->next = latest;
  latest = new;
  return new;
}

typedef struct reader_state_t {
  FILE *stream;
  char *linebuf;
  cell linebuf_size;
  char *next_char;
} reader_state_t;

static void init_reader_state(reader_state_t *state, char *linebuf, cell linebuf_size, FILE *fp) {
  state->stream = fp;
  state->linebuf = linebuf;
  state->linebuf[0] = '\0';
  state->linebuf_size = linebuf_size;
  state->next_char = linebuf;
}

static reader_state_t *open_file(const char *filename, const char *mode) {
  FILE *fp = fopen(filename, mode);
  if(!fp) return NULL;
  char *lbuf = MALLOC_ATOMIC(1024);
  if(!lbuf) goto err_exit;

  setvbuf(fp, NULL, _IONBF, 0);  // disable input buffering, we have our own

  reader_state_t *state = (reader_state_t*)MALLOC(sizeof(reader_state_t));
  if(!state) goto err_exit;
  init_reader_state(state, lbuf, 1024, fp);
  return state;

 err_exit:
  FREE(lbuf);
  fclose(fp);
  return NULL;
}

static void close_file(reader_state_t *fp) {
  if(fp->stream) fclose(fp->stream);
  FREE(fp->linebuf);
  FREE(fp);
}

static void skip_whitespaces(reader_state_t *state) {
  while(isspace(*state->next_char)) state->next_char++;
}

static cell is_eol(reader_state_t *state) {
  skip_whitespaces(state);
  return *state->next_char=='\0';
}

static cell is_eof(reader_state_t *fp) {
  return *fp->next_char=='\0' && feof(fp->stream);
}

static char *read_next_line(reader_state_t *state) {
  char *tmp = fgets(state->linebuf, state->linebuf_size, state->stream);
  if(!tmp) return NULL;
  state->next_char = tmp;
  return tmp;
}

static char *prompt_line(const char *prompt, reader_state_t *state) {
  char *tmp = readline(prompt);
  if(!tmp) {
    return NULL;
  }
  add_history(tmp);
  strncpy(state->linebuf, tmp, state->linebuf_size);
  free(tmp);
  state->next_char = state->linebuf;
  return state->next_char;
}

static int read_key(reader_state_t *state) {
  if(*state->next_char=='\0') {
    if(!read_next_line(state)) return -1;
  }
  return *state->next_char++;
}

static char *read_word(reader_state_t *state, char *tobuf) {
  char *buf = tobuf;

  // skip whitespaces first
 skipws:
  skip_whitespaces(state);

  // buffer exhausted? fill and reskip whitespaces
  if(*state->next_char == '\0') {
    if(!read_next_line(state)) return NULL;
    goto skipws;
  }

  // copy until next whitespace
  while(*state->next_char!='\0' && !isspace(*state->next_char)) {
    *buf++ = *state->next_char++;
  }
  state->next_char++;
  *buf = '\0';

  return tobuf;
}

static void emit_char(int c, FILE *fp) {
  fputc(c, fp);
}

static void *get_builtin(const char *name) {
  dict_hdr_t *hdr = find_word(name);
  return *(cfa(hdr));
}

static void assemble_word(const char *name, cell flags, void **code, cell codesize) {
  int i;
  create_word(name, flags);
  for(i=0; i<codesize/sizeof(void*); i++) {
    comma((cell)code[i]);
  }
  comma((cell)get_builtin("eow"));
}

static void create_constant(const char *name, cell value) {
  void *flagdef[] = { get_builtin("lit"), 0, get_builtin("exit") };
  flagdef[1] = (void*)value;
  assemble_word(name, FLAG_INLINE, flagdef, sizeof(flagdef));
}

typedef float aliasingfloat __attribute__((__may_alias__));

static void create_fconstant(const char *name, float value) {
  void *flagdef[] = { get_builtin("flit"), 0, get_builtin("exit") };
  *(aliasingfloat*)(&flagdef[1]) = value;
  assemble_word(name, FLAG_INLINE, flagdef, sizeof(flagdef));
}

static void create_builtin(builtin_word_t *b) {
  create_word(b->name, b->flags | FLAG_BUILTIN);
  comma((cell)b->code);
}

static thread_state_t *init_thread(cell *s0, void ***r0, cell *t0, void **entrypoint)
{
  thread_state_t *new = MALLOC(sizeof(thread_state_t));
  new->killed = 0;
  new->ip = entrypoint;
  new->s0 = s0;
  new->r0 = r0;
  new->t0 = t0;
  new->ds = new->s0;
  new->rs = new->r0;
  new->ts = new->t0;

  if(!current_thread) {
    current_thread = new;
    new->next = new;
  } else {
    new->next = current_thread->next;
    current_thread->next = new;
  }

  return new;
}

static thread_state_t *create_thread(int ds_size, int rs_size, int ts_size, void **entrypoint)
{
  return init_thread((cell*)MALLOC(ds_size*sizeof(cell)),
		     (void***)MALLOC(rs_size*sizeof(cell)),
		     (cell*)MALLOC(ts_size*sizeof(cell)),
		     entrypoint);
}

static void kill_thread() {
  if(!current_thread) return;
  if(current_thread->next == current_thread) {
    return;
  }
  current_thread->killed = 1;
  thread_state_t *i = current_thread;
  while(i->next!=current_thread) i = i->next;
  i->next = current_thread->next;
  current_thread = i;
}

#define PUSH(x)     *--ds = (cell)(x)
#define POP()       (*ds++)
#define FPUSH(x)     *--fs = (float)(x)
#define FPOP()       (*fs++)
#define INTARG()    ((cell)(*ip++))
#define FLOATARG()  (*(float*)ip)
#define ARG()       (*ip++)
#define PUSHRS(x)   *--rs = (void**)(x)
#define POPRS()     (*rs++)
#define NEXT()      goto **ip++
#define TOP()       (*ds)
#define FTOP()      (*fs)
#define AT(x)       (*(ds+(x)))
#define FAT(x)      (*(fs+(x)))

/* utilies for calculating branch offsets in inline bytecode and referencing bytecodes */
#define OFFSET(x) (void*)((x)*sizeof(cell))
#define WORD(name) &&l_##name

/* the interpreter! */
static void interpret(void **ip, cell *ds, void ***rs, reader_state_t *inputstate, FILE *outp, int argc, char **argv)
{
  static int initialized = 0;

  register cell tmp;

  cell state = STATE_IMMEDIATE;

  cell base = 10;

  cell *s0 = ds;
  cell *t0 = NULL;
  float *f0 = NULL;
  cell *ts = NULL;
  float *fs = NULL;
  void ***r0 = rs;

  void **nestingstack_space[NESTINGSTACK_MAX_DEPTH];
  void ***nestingstack = nestingstack_space + NESTINGSTACK_MAX_DEPTH;

  void **debugger_vector = NULL;

  void *builtin_immediatebuf[2] = { NULL, WORD(IRETURN) };
  void *word_immediatebuf[3]    = { WORD(CALL), NULL, WORD(IRETURN) };

  char wordbuf[MAX_WORD_NAME_LEN];
  char linebuf[MAX_WORD_NAME_LEN];

  char stdinbuf[1024];
  reader_state_t stdin_state;

  DCCallVM* callvm = dcNewCallVM((DCsize)4096);
  
  /* trick: include bytecodes.h with a macro for BYTECODE that produces builtin
   * list elements */
  static builtin_word_t builtins[] = {
    #define BYTECODE(label, name, nargs, nfargs, flags, code) { name, &&l_##label, flags },
    #include "bytecodes.h"
    #undef BYTECODE
    { NULL, NULL, 0 }
  };

  /* one time init: install the builtins into dictionary and define some essential variables */
  if(!initialized) {
    initialized = 1;

    setvbuf(stdin, NULL, _IONBF, 0);  // disable input buffering, we have our own
    init_reader_state(&stdin_state, stdinbuf, 1024, stdin);

    builtin_word_t *b = builtins;
    while(b->name) create_builtin(b++);
    
    /* some constants */
    create_constant("version", FORTH_VERSION);
    create_constant("f_builtin", FLAG_BUILTIN);
    create_constant("f_hasarg", FLAG_HASARG);
    create_constant("f_immediate", FLAG_IMMED);
    create_constant("f_hidden", FLAG_HIDDEN);
    create_constant("f_inline", FLAG_INLINE);
    create_constant("f_deferred", FLAG_DEFERRED);
    create_constant("s0", (cell) &s0);
    create_constant("r0", (cell) &r0);
    create_constant("t0", (cell) &t0);
    create_constant("f0", (cell) &f0);
    create_constant("state", (cell) &state);
    create_constant("cellsize", (cell)sizeof(cell));
    create_constant("floatsize", (cell)sizeof(float));
    create_constant("base", (cell) &base);
    create_constant("here", (cell) &here);
    create_constant("here0", (cell)here0);
    create_constant("hdrsize", (cell) sizeof(dict_hdr_t));
    create_constant("<stdin>", (cell) &stdin_state);
    create_constant("<stdout>", (cell) stdout);
    create_constant("input-stream", (cell) &inputstate);
    create_constant("output-stream", (cell) &outp);
    create_constant("argc", (cell)argc);
    create_constant("argv", (cell)argv);
    create_constant("current-thread", (cell)&current_thread);
    create_constant("debugger-vector", (cell)&debugger_vector);

    create_constant("syscall-fn", (cell)&syscall);

    create_constant("SYS_ioctl", (cell)SYS_ioctl);
    create_constant("SYS_open", (cell)SYS_open);
    create_constant("SYS_read", (cell)SYS_read);
    create_constant("SYS_write", (cell)SYS_write);
    create_constant("SYS_close", (cell)SYS_close);
    create_constant("SYS_fcntl", (cell)SYS_fcntl);
    create_constant("SYS_nanosleep", (cell)SYS_nanosleep);
    create_constant("SYS_gettimeofday", (cell)SYS_gettimeofday);
    create_constant("SYS_mkdir", (cell)SYS_mkdir);
    create_constant("SYS_rmdir", (cell)SYS_rmdir);
    create_constant("SYS_fork", (cell)SYS_fork);
    create_constant("SYS_dup", (cell)SYS_dup);
    create_constant("SYS_lseek", (cell)SYS_lseek);
    create_constant("SYS_chdir", (cell)SYS_chdir);
    create_constant("SYS_creat", (cell)SYS_creat);
    create_constant("SYS_unlink", (cell)SYS_unlink);
    create_constant("SYS_rename", (cell)SYS_rename);
    create_constant("SYS_getcwd", (cell)SYS_getcwd);

    create_constant("DC_CALL_C_ELLIPSIS", (cell)DC_CALL_C_ELLIPSIS);
    create_constant("DC_CALL_C_DEFAULT", (cell)DC_CALL_C_DEFAULT);

    create_fconstant("FLT_MAX", FLT_MAX);
    create_fconstant("FLT_MIN", FLT_MIN);
    create_fconstant("FLT_EPSILON", FLT_EPSILON);
    create_fconstant("PI", M_PI);

    init_thread(s0, r0, t0, ip);

    // QUIT is the topmost interpreter loop: interpret forever. better version implemented in
    // forth later that supports eof etc
    void *quitcode[] = { WORD(INTERPRET), 
			 WORD(BRANCH), OFFSET(-2),
			 WORD(EOW)
    };
    ip = quitcode;
  }

  NEXT();

#if SAFE_INTERPRETER
 #define CHECKSTACK(name, amt) if((((cell)s0-(cell)ds)/sizeof(cell)) < amt)  \
    {                                                                        \
      printf("%s: stack underflow\n", (name));                               \
      PUSHRS(ip);							\
      ip = debugger_vector;						\
      NEXT();                                                                \
    }
 #define CHECKFSTACK(name, amt) if((((cell)f0-(cell)fs)/sizeof(float)) < amt)  \
    {                                                                        \
      printf("%s: float stack underflow\n", (name));                               \
      PUSHRS(ip);							\
      ip = debugger_vector;						\
      NEXT();                                                                \
    }
#else
 #define CHECKSTACK(name, amt)
 #define CHECKFSTACK(name, amt)
#endif

#define BYTECODE(label, name, nargs, nfargs, flags, code) l_##label: CHECKSTACK(name, nargs) CHECKFSTACK(name, nfargs) code NEXT();
#include "bytecodes.h"
}

static char *word_completion_generator(const char *text, int state)
{
  static dict_hdr_t *index = NULL;
  static int len=0;

  if(!state) {
    len = strlen(text);
    index = latest;
  }

  while(index!=NULL) {
    if(!(index->flags & FLAG_HIDDEN) && strncmp(index->name, text, len)==0) {
      char *n = strdup(index->name);
      if(!n) { fprintf(stderr, "Error: out of memory, exiting\n"); exit(1); }
      index = index->next;
      return n;
    }
    index = index->next;
  }

  return NULL;
}

static char **word_completion(const char *text, int start, int end)
{
  return rl_completion_matches(text, &word_completion_generator);
}

static void init_readline()
{
  rl_readline_name = "sforth";
  rl_attempted_completion_function = &word_completion;
}

int main(int argc, char **argv) {
  cell datastack[1024];
  void **returnstack[512];

#ifdef USE_GC
  GC_INIT();
#endif

  init_readline();

  reader_state_t *fp = open_file("forth.f", "r");
  if(!fp) {
    fprintf(stderr, "Cannot open bootstrap file forth.f!\n");
    return 1;
  }

  here_size = 10*1024*1024;
  here0 = MALLOC(here_size);
  here = here0;
  interpret(NULL, datastack+1024, returnstack+512, fp, stdout, argc, argv);
  close_file(fp);

  return 0;
}
