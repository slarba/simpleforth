/* --------------------------------------------------
 * Portable Forth-like language interpreter
 * (c) 2011 Marko Lauronen, Sysart Oy
 * --------------------------------------------------
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/mman.h>

#include <dyncall.h>
#include <dynload.h>
#include <readline/readline.h>

#ifdef USE_GC
 #include <gc.h>
 #define MALLOC(x) GC_MALLOC(x)
 #define REALLOC(ptr,newlen) GC_REALLOC(ptr,newlen)
 #define RUNGC() GC_gcollect()
 #define FREE(x)
#else
 #define MALLOC(x) malloc(x)
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

/*
static int alloc_here_area(unsigned int size) {
  here0 = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if(here0==MAP_FAILED) return 0;
  here = here0;
  here_size = size;
  return 1;
}

static int resize_area(unsigned int amount) {
  here0 = mremap(here0, here_size, here_size+amount, 0);
  if(here0==MAP_FAILED) return 0;
  here_size += amount;
  return 1;
}
*/

static void check_here() {
  /*
  if(here>=(here0+here_size)) {
    puts("resize");
    if(!resize_area(10*1024)) {
      fprintf(stderr, "cannot resize here area!\n");
      exit(1);
    }
  }
  */
}

static void **cfa(dict_hdr_t *word) {
  return (void**)(word+1);
}

static void comma(cell value) {
  *(cell*)here = value;
  here += sizeof(cell);
  check_here();
}

static dict_hdr_t *create_word(const char *name, cell flags) {
  if(!name) {
    name="\0";  /* for creating unnamed words */
  }

  dict_hdr_t *new = (dict_hdr_t*)here;
  here += sizeof(dict_hdr_t);
  check_here();
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
  char *lbuf = MALLOC(1024);
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
  fclose(fp->stream);
  FREE(fp->linebuf);
  FREE(fp);
}

static void skip_whitespaces(reader_state_t *state) {
  while(isspace(*state->next_char)) state->next_char++;
}

static cell is_eol(reader_state_t *state) {
  skip_whitespaces(state);
  return *state->next_char==0;
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

static void create_builtin(builtin_word_t *b) {
  create_word(b->name, b->flags | FLAG_BUILTIN);
  comma((cell)b->code);
}

#define PUSH(x)     *--ds = (cell)(x)
#define POP()       (*ds++)
#define INTARG()    ((cell)(*ip++))
#define ARG()       (*ip++)
#define PUSHRS(x)   *--rs = (void**)(x)
#define POPRS()     (*rs++)
#define NEXT()      goto **ip++
#define TOP()       (*ds)
#define AT(x)       (*(ds+(x)))

/* utilies for calculating branch offsets in inline bytecode and referencing bytecodes */
#define OFFSET(x) (void*)((x)*sizeof(cell))
#define WORD(name) &&l_##name

/* the interpreter! */
static void interpret(void **ip, cell *ds, void ***rs, reader_state_t *inputstate, FILE *outp)
{
  cell tmp;
  cell state = STATE_IMMEDIATE;
  cell base = 10;
  cell *s0 = ds;
  cell *t0 = NULL;
  cell *ts = NULL;
  void ***r0 = rs;

  char wordbuf[MAX_WORD_NAME_LEN];
  char linebuf[MAX_WORD_NAME_LEN];

  void **nestingstack_space[NESTINGSTACK_MAX_DEPTH];
  void ***nestingstack = nestingstack_space + NESTINGSTACK_MAX_DEPTH;

  void *builtin_immediatebuf[2] = { NULL, WORD(IRETURN) };
  void *word_immediatebuf[3]    = { WORD(CALL), NULL, WORD(IRETURN) };

  char stdinbuf[1024];
  reader_state_t stdin_state;

  DCCallVM* callvm = dcNewCallVM((DCsize)4096);
  
  setvbuf(stdin, NULL, _IONBF, 0);  // disable input buffering, we have our own
  init_reader_state(&stdin_state, stdinbuf, 1024, stdin);

  /* trick: include bytecodes.h with a macro for BYTECODE that produces builtin
   * list elements */
#define BYTECODE(label, name, nargs, flags, code) { name, &&l_##label, flags },
  static builtin_word_t builtins[] = {
#include "bytecodes.h"
    { NULL, NULL, 0 }
  };
#undef BYTECODE

  /* special case: if ip is NULL, fill the builtins into dictionary */
  if(!ip) {
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
    create_constant("state", (cell) &state);
    create_constant("cellsize", (cell)sizeof(cell));
    create_constant("base", (cell) &base);
    create_constant("here", (cell) &here);
    create_constant("hdrsize", (cell) sizeof(dict_hdr_t));
    create_constant("<stdin>", (cell) &stdin_state);
    create_constant("<stdout>", (cell) stdout);
    create_constant("input-stream", (cell) &inputstate);
    create_constant("output-stream", (cell) &outp);

    // QUIT is the topmost interpreter loop: interpret forever. better version implemented in
    // forth later that supports eof etc
    void *quitcode[] = { WORD(INTERPRET), 
			 WORD(BRANCH), OFFSET(-2),
			 WORD(EOW)
    };
    assemble_word("quit", 0, quitcode, sizeof(quitcode));
    return;
  }

  NEXT();

#if SAFE_INTERPRETER
 #define CHECKSTACK(name, amt) if(( ((cell)s0-(cell)ds) / sizeof(cell) ) < (amt)) { printf("%s: stack underflow\n", (name)); NEXT(); }
#else
 #define CHECKSTACK(name, amt)
#endif
#define BYTECODE(label, name, nargs, flags, code) l_##label: CHECKSTACK(name, nargs) code NEXT();
#include "bytecodes.h"
#undef BYTECODE
}

void init_interpreter(int argc, char **argv, unsigned int howmuchmemory) {
  /* if(!alloc_here_area(howmuchmemory)) {
    fprintf(stderr, "cannot allocate here area of size %d\n", howmuchmemory);
    exit(1);
  }
  */
  here0 = MALLOC(howmuchmemory);
  here = here0;
  here_size = howmuchmemory;
  interpret(NULL, NULL, NULL, NULL, NULL);
  create_constant("argc", (cell)argc);
  create_constant("argv", (cell)argv);
  create_constant("here0", (cell)here0);
}

int main(int argc, char **argv) {
  cell datastack[1024];
  void **returnstack[512];

#ifdef USE_GC
  GC_INIT();
#endif

  init_interpreter(argc, argv, 10*1024*1024);

  reader_state_t *fp = open_file("forth.f", "r");
  if(!fp) {
    fprintf(stderr, "Cannot open bootstrap file forth.f!\n");
    return 1;
  }

  void **initprog = cfa(find_word("quit"));
  interpret(initprog, datastack+1024, returnstack+512, fp, stdout);

  close_file(fp);
  return 0;
}
