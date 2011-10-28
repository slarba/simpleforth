/* --------------------------------------------------
 * Portable Forth-like language interpreter
 * (c) 2011 Marko Lauronen, Sysart Oy
 * --------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

/* ---------- limits ---------- */
#define MAX_WORD_NAME_LEN 32

/* ---------- dictionary entry flags ---------- */
#define FLAG_HIDDEN      (1<<0)
#define FLAG_IMMED       (1<<1)
#define FLAG_BUILTIN     (1<<2)
#define FLAG_HASARG      (1<<3)

/* ---------- compiler state ---------- */
#define STATE_IMMEDIATE 0
#define STATE_COMPILE   1

/* the most important type, the cell. MUST be exactly of the pointer length! */
typedef long cell;

/* preprocessor trick to test sizeof(long)==sizeof(void*)? */

/* dictionary definition header */
typedef struct dict_hdr_t {
  cell                 flags;
  struct dict_hdr_t   *next;
  char                 name[MAX_WORD_NAME_LEN];
} dict_hdr_t;

/* free memory pointers and latest defined word */
static void       *const_here;        /* constant pool  */
static void       *here;              /* working memory */
static dict_hdr_t *latest = NULL;

/* utility functions */
static dict_hdr_t *find_word(const char *name) {
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

static void ccomma(char value) {
  *(char*)here = value;
  here += sizeof(char);
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

static int read_key(reader_state_t *state) {
  if(*state->next_char=='\0') {
    if(!fgets(state->linebuf, state->linebuf_size, state->stream))
      return 0;
    state->next_char = state->linebuf;
  }
  return *state->next_char++;
}

static char *read_word(reader_state_t *state, char *tobuf) {
  char *buf = tobuf;

  // skip whitespaces first
 skipws:
  while(isspace(*state->next_char)) {
    state->next_char++;
  }

  // buffer exhausted? fill and reskip whitespaces
  if(*state->next_char == '\0') {
    if(!fgets(state->linebuf, state->linebuf_size, state->stream))
      return NULL;  // file exhausted
    state->next_char = state->linebuf;
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

typedef struct builtin_word_t {
  char *name;
  void *code;
  cell flags;
} builtin_word_t;

static void interpret(void **ip, cell *ds, void ***rs)
{
  register long tmp;
  cell state = STATE_IMMEDIATE;
  cell base = 10;
  cell *s0 = ds;
  char wordbuf[MAX_WORD_NAME_LEN];

  char lbuf[1024];
  reader_state_t inputstate;
  init_reader_state(&inputstate, lbuf, 1024, stdin);
  
#define PUSH(x)     *--ds = (cell)(x)
#define POP()       (*ds++)
#define INTARG()    ((cell)(*ip++))
#define ARG()       (*ip++)
#define PUSHRS(x)   *--rs = (void**)(x)
#define POPRS()     (*rs++)
#define NEXT()      goto **ip++
#define TOP()       (*ds)
#define AT(x)       (*(ds+(x)))

  static builtin_word_t builtins[] = {
    { "call", &&l_CALL, FLAG_HASARG },
    { "die", &&l_DIE, 0 },
    { "exit", &&l_RETURN, 0 },
    { "branch", &&l_BRANCH, FLAG_HASARG },
    { "0branch", &&l_0BRANCH, FLAG_HASARG },
    { "lit", &&l_LIT, FLAG_HASARG },
    { "dup", &&l_DUP, 0 },
    { "2dup", &&l_2DUP, 0 },
    { "?dup", &&l_CONDDUP, 0 },
    { "swap", &&l_SWAP, 0 },
    { "drop", &&l_DROP, 0 },
    { "2drop", &&l_2DROP, 0 },
    { ">r", &&l_TOR },
    { "r>", &&l_FROMR },
    { "rsp@", &&l_RSPGET },
    { "rsp!", &&l_RSPPUT },
    { "over", &&l_OVER, 0 },
    { "rot", &&l_ROT, 0 },
    { "-rot", &&l_MROT, 0 },
    { "here", &&l_HERE, 0 },
    { "latest", &&l_LATEST, 0 },
    { "base", &&l_BASE, 0 },
    { "s0", &&l_S0, 0 },
    { "find", &&l_FIND, 0 },
    { "create", &&l_CREATE, 0 },
    { "word", &&l_WORD, 0 },
    { "key", &&l_KEY, 0 },
    { "emit", &&l_EMIT, 0 },
    { "tell", &&l_TELL, 0 },
    { "state", &&l_STATE, 0 },
    { "[", &&l_LBRAC, FLAG_IMMED },
    { "]", &&l_RBRAC, 0 },
    { "1+", &&l_ADD1, 0 },
    { "1-", &&l_SUB1, 0 },
    { "+!", &&l_MEMADD, 0 },
    { "-!", &&l_MEMSUB, 0 },
    { "cell+", &&l_CELLPLUS, 0 },
    { "cell-", &&l_CELLMINUS, 0 },
    { "+", &&l_ADD, 0 },
    { "-", &&l_SUB, 0 },
    { "*", &&l_MUL, 0 },
    { "/", &&l_DIV, 0 },
    { "<", &&l_LT, 0 },
    { ">", &&l_GT, 0 },
    { "=", &&l_EQ, 0 },
    { "<>", &&l_NEQ, 0 },
    { "<=", &&l_LE, 0 },
    { ">=", &&l_GE, 0 },
    { "0=", &&l_EQZERO, 0 },
    { "0<>", &&l_NOTEQZERO, 0 },
    { "0>", &&l_GTZ, 0 },
    { "0<", &&l_LTZ, 0 },
    { "mod", &&l_MOD, 0 },
    { "invert", &&l_NEG, 0 },
    { "and", &&l_AND, 0 },
    { "or", &&l_OR, 0 },
    { "xor", &&l_XOR, 0 },
    { "lshift", &&l_ROL, 0 },
    { "rshift", &&l_ROR, 0 },
    { ">cfa", &&l_TOCFA, 0 },
    { ",", &&l_COMMA, 0 },
    { "dsp@", &&l_DSPFETCH, 0 },
    { "c,", &&l_CCOMMA, 0 },
    { "@", &&l_FETCH, 0 },
    { "c@", &&l_CFETCH, 0 },
    { "!", &&l_STORE, 0 },
    { "[compile]", &&l_COMPILE, FLAG_IMMED },
    { "c!", &&l_CSTORE, 0 },
    { "interpret", &&l_INTERPRET, 0 },
    { ".", &&l_DOT, 0 },
    { "hidden", &&l_HIDDEN, 0 },
    { "execute", &&l_EXECUTE, 0 },
    { "'", &&l_TICK, FLAG_HASARG|FLAG_IMMED },
    { "immediate", &&l_IMMEDIATE, FLAG_IMMED },
    { "disasm", &&l_DISASM, 0 },
    { "cellsize", &&l_CELLSIZE, 0 },
    { "consthere", &&l_CONSTPOOL, 0 },
    { "const,", &&l_CONSTCOMMA, 0 },
    { "constc,", &&l_CONSTCCOMMA, 0 },
    { "malloc", &&l_MALLOC, 0 },
    { "mfree", &&l_MFREE, 0 },
    { "pushxt", &&l_PUSHXT, 0 },

    { NULL, NULL, 0 }
  };

  /* special case: if ip is NULL, fill the builtins into dictionary */
  if(!ip) {
    builtin_word_t *b = builtins;
    while(b->name) {
      create_word(b->name, b->flags | FLAG_BUILTIN);
      comma((cell)b->code);
      ++b;
    }

    create_word(":", 0);
    void *colondef[] = { &&l_WORD, &&l_CREATE, &&l_RBRAC, &&l_RETURN, 0 };
    int i=0;
    while(colondef[i]) { comma((cell)colondef[i++]); }
    create_word(";", FLAG_IMMED);
    void *semicolondef[] = { &&l_LIT, &&l_RETURN, &&l_COMMA, &&l_LBRAC, &&l_RETURN, 0 };
    i = 0;
    while(semicolondef[i]) { comma((cell)semicolondef[i++]); }
    return;
  }

  NEXT();
 l_DISASM: {
    void **code = (void**)POP();
    while(*code!=&&l_RETURN) {
      builtin_word_t *b = builtins;
      while(b->name) {
	if(b->code==*code) {
	  if(b->flags & FLAG_HASARG) {
	    printf("0x%lx   %s %d\n", code, b->name, *(code+1));
	    code += 2;
	  } else {
	    printf("0x%lx   %s\n", code, b->name);
	    code++;
	  }
	  break;
	}
	b++;
      }
      if(!b->name) { puts("error disassembling"); NEXT(); }
    }
    printf("0x%lx   exit\n", code);
    NEXT();
  }
 l_CALL: { 
    void *fn = ARG();
    PUSHRS(ip);
    ip = fn;
    NEXT();
  }
 l_EXECUTE: {
    PUSHRS(ip);
    ip = (void**)POP();
    NEXT();
  }
 l_HIDDEN: {
    dict_hdr_t *hdr = (dict_hdr_t*)POP();
    hdr->flags ^= FLAG_HIDDEN;
    NEXT();
  }
 l_COMPILE: {
    char *word = read_word(&inputstate, wordbuf); // , MAX_WORD_NAME_LEN, stdin);
    dict_hdr_t *hdr = find_word(word);
    if(hdr->flags & FLAG_BUILTIN) {
      comma((cell)(*(cfa(hdr))));
    } else {
      comma((cell)&&l_CALL);
      comma((cell)cfa(hdr));
    }
    NEXT();
  }
 l_CELLSIZE: {
    PUSH(sizeof(cell));
    NEXT();
  }
 l_TICK: {
    if(state==STATE_IMMEDIATE) {
      char *word = read_word(&inputstate, wordbuf); // , MAX_WORD_NAME_LEN, stdin);
      dict_hdr_t *de = find_word(wordbuf);
      cell token;
      if(de->flags & FLAG_BUILTIN) {
	token = (cell)(*(cfa(de)));
      } else {
	token = (cell)cfa(de);
      }
      PUSH(token);
    } else {
      comma((cell)&&l_PUSHXT);
    }
    NEXT();
  }
 l_PUSHXT: {
    void *xt = ARG();
    if(xt==&&l_CALL) {
      PUSH(ARG());
    } else {
      PUSH(xt);
    }
    NEXT();
  }
 l_RETURN: {
    ip = POPRS();
    NEXT();
  }
 l_DIE: {
    return;
  }
 l_BRANCH: {
    tmp = INTARG();
    ip += (tmp/sizeof(void*))-1;
    NEXT();
  }
 l_0BRANCH: {
    tmp = INTARG();
    if(!POP())
      ip += (tmp/sizeof(void*))-1;
    NEXT();
  }
 l_FROMR: {
    tmp = (cell)POPRS();
    PUSH(tmp);
    NEXT();
  }
 l_TOR: {
    tmp = POP();
    PUSHRS(tmp);
    NEXT();
  }
 l_DSPFETCH: {
    tmp = (cell)ds;
    PUSH(tmp);
    NEXT();
  }
 l_RSPPUT: {
    rs = (void***)POP();
    NEXT();
  }
 l_RSPGET: {
    PUSH(rs);
    NEXT();
  }
 l_LIT: {
    PUSH(INTARG());
    NEXT();
  }
 l_DUP: {
    tmp = TOP();
    PUSH(tmp);
    NEXT();
  }
 l_2DUP: {
    tmp = AT(1);
    PUSH(tmp);
    tmp = AT(1);
    PUSH(tmp);
    NEXT();
  }
 l_CONDDUP: {
    tmp = TOP();
    if(tmp) PUSH(tmp);
    NEXT();
  }
 l_SWAP: {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;
    NEXT();
  }
 l_OVER: {
    tmp = AT(1);
    PUSH(tmp);
    NEXT();
  }
 l_DROP: {
    ++ds;
    NEXT();
  }
 l_ROT: {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(ebx);
    PUSH(eax);
    PUSH(ecx);
    NEXT();
  }
 l_MROT: {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(eax);
    PUSH(ecx);
    PUSH(ebx);
    NEXT();
  }
 l_2DROP: {
    ds += 2;
    NEXT();
  }
 l_ADD: { 
    tmp = POP();
    AT(0) += tmp;
    NEXT(); 
  }
 l_SUB: { 
    tmp = POP();
    AT(0) -= tmp;
    NEXT(); 
  }
 l_MUL: { 
    tmp = POP();
    AT(0) *= tmp;
    NEXT(); 
  }
 l_DIV: { 
    tmp = POP();
    AT(0) /= tmp;
    NEXT(); 
  }
 l_MOD: { 
    tmp = POP();
    AT(0) = AT(0) % tmp;
    NEXT(); 
  }
 l_NEG: {
    AT(0) = ~AT(0);
    NEXT(); 
  }
 l_ROL: { 
    tmp = POP();
    AT(0) <<= tmp;
    NEXT();
  }
 l_ROR: {
    tmp = POP();
    AT(0) >>= tmp;
    NEXT(); 
  }
 l_AND: {
    tmp = POP();
    AT(0) &= tmp;
    NEXT(); 
  }
 l_OR: {
    tmp = POP();
    AT(0) |= tmp;
    NEXT(); 
  }
 l_XOR: {
    tmp = POP();
    AT(0) ^= tmp;
    NEXT(); 
  }

 l_HERE: {
    PUSH(&here);
    NEXT();
  }
 l_CONSTPOOL: {
    PUSH(&const_here);
    NEXT();
  }

 l_LATEST: {
    PUSH(&latest);
    NEXT();
  }
 l_IMMEDIATE: {
    latest->flags ^= FLAG_IMMED;
    NEXT();
  }
 l_S0: {
    PUSH(&s0);
    NEXT();
  }

 l_LT: {
    tmp = POP();
    AT(0) = AT(0) < tmp;
    NEXT(); 
  }
 l_GTZ: {
    AT(0) = AT(0) > 0;
    NEXT();
  }
 l_LTZ: {
    AT(0) = AT(0) < 0;
    NEXT();
  }

 l_GT: {
    tmp = POP();
    AT(0) = AT(0) > tmp;
    NEXT(); 
  }

 l_LE: {
    tmp = POP();
    AT(0) = AT(0) <= tmp;
    NEXT(); 
  }

 l_GE: {
    tmp = POP();
    AT(0) = AT(0) >= tmp;
    NEXT(); 
  }
 l_EQ: {
    tmp = POP();
    AT(0) = AT(0) == tmp;
    NEXT();
  }
 l_NEQ: {
    tmp = POP();
    AT(0) = AT(0) != tmp;
    NEXT();
  }
 l_EQZERO: {
    AT(0) = AT(0)==0;
    NEXT(); 
  }

 l_NOTEQZERO: {
    AT(0) = AT(0)!=0;
    NEXT(); 
  }

 l_FIND: {
    char *wordname = (char*) POP();
    PUSH(find_word(wordname));
    NEXT();
  }
 l_CREATE: {
    create_word((char*)POP(), 0);
    NEXT();
  }
 l_WORD: {
    PUSH(read_word(&inputstate, wordbuf));  // , MAX_WORD_NAME_LEN, stdin
    NEXT();
  }
 l_KEY: {
    PUSH(read_key(&inputstate));
    NEXT();
  }
 l_EMIT: {
    emit_char(POP(), stdout);
    NEXT();
  }
 l_CELLPLUS: {
    AT(0) += sizeof(cell);
    NEXT();
  }
 l_CELLMINUS: {
    AT(0) -= sizeof(cell);
    NEXT();
  }
 l_ADD1: {
    AT(0) += 1;
    NEXT();
  }
 l_SUB1: {
    AT(0) -= 1;
    NEXT();
  }
 l_MEMADD: {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr += tmp;
    NEXT();
  }
 l_MEMSUB: {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr -= tmp;
    NEXT();
  }
 l_STATE: {
    PUSH(&state);
    NEXT();
  }
 l_BASE: {
    PUSH(&base);
    NEXT();
  }
 l_LBRAC: {
    state = STATE_IMMEDIATE;
    NEXT();
  }
 l_RBRAC: {
    state = STATE_COMPILE;
    NEXT();
  }
 l_COMMA: {
    tmp = POP();
    *(cell*)here = tmp;
    here += sizeof(cell);
    NEXT();
  }
 l_CCOMMA: {
    tmp = POP();
    *(char*)here = (char)tmp;
    here += sizeof(char);
    NEXT();
  }
 l_CONSTCOMMA: {
    tmp = POP();
    *(cell*)const_here = tmp;
    const_here += sizeof(cell);
    NEXT();
  }
 l_CONSTCCOMMA: {
    tmp = POP();
    *(char*)const_here = (char)tmp;
    const_here += sizeof(char);
    NEXT();
  }
 l_STORE: {
    cell *ptr = (cell*)POP();
    tmp = POP();
    *ptr = tmp;
    NEXT();
  }
 l_FETCH: {
    cell *ptr = (cell*)POP();
    PUSH(*ptr);
    NEXT();
  }
 l_CSTORE: {
    char *ptr = (char*)POP();
    tmp = POP();
    *ptr = (char)tmp;
    NEXT();
  }
 l_CFETCH: {
    char *ptr = (char*)POP();
    PUSH(*ptr);
    NEXT();
  }
 l_TOCFA: {
    dict_hdr_t *ptr = (dict_hdr_t*)POP();
    PUSH((ptr+1));
    NEXT();
  }
 l_DOT: {
    fprintf(stdout, "%lx\n", POP());
    NEXT();
  }
 l_TELL:
  {
    printf((char*)POP());
    NEXT();
  }
 l_MALLOC: {
    tmp = POP();
    PUSH(malloc(tmp));
    NEXT();
  }
 l_MFREE: {
    free((void*)POP());
    NEXT();
  }
 l_INTERPRET: {
    char linebuf[MAX_WORD_NAME_LEN];
    void *builtin_immediatebuf[2] = { NULL, &&l_INTERPRET };
    void *word_immediatebuf[3]    = { &&l_CALL, NULL, &&l_INTERPRET };

    /* read a word from input skipping whitespaces, if EOF then return */
    char *word = read_word(&inputstate,linebuf); // , MAX_WORD_NAME_LEN, stdin);
    if(!word) return;

    /* try to find the word from dictionary */
    dict_hdr_t *entry = find_word(word);
    if(!entry) {
      /* was not a word, try to parse a number */
      char *endptr = NULL;
      cell val = (cell)strtol(word, &endptr, base);
      if(*endptr!='\0') {
	/* not a number either, this is an error */
	printf("ERROR: no such word: %s\n", word);
      } else {
	/* yes, it's a number. if compiling, emit LIT instruction with the number */
	if(state==STATE_COMPILE) {
	  comma((cell) &&l_LIT);
	  comma(val);
	} else {
	  /* in immediate mode push the number to stack right away*/
	  PUSH(val);
	}
      }
      /* and then continue interpreting */
      goto l_INTERPRET;
    }

    /* was a real word and was found in dictionary. if compiling and word is not an immediate one,
     * emit CALL instruction or copy the builtin from the definition
     */
    if(state==STATE_COMPILE && !(entry->flags & FLAG_IMMED)) {
      if(entry->flags & FLAG_BUILTIN) {
	/* emit the builtin pointer */
	comma((cell)(*cfa(entry)));
      } else {
	/* emit the CALL instruction */
	comma((cell) &&l_CALL);
	comma((cell) cfa(entry));
      }
      /* and continue interpreting */
      goto l_INTERPRET;
    } else {
      /* we're in immediate mode OR the word was an immediate one...
       * emit the instruction into a temp buffer and execute from there, returning to
       * INTERPRET again
       */
      void **code = cfa(entry);
      if(entry->flags & FLAG_BUILTIN) {
	builtin_immediatebuf[0] = *code;
	ip = builtin_immediatebuf;
      } else {
	word_immediatebuf[1] = (void*)code;
	ip = word_immediatebuf;
      }
      NEXT();
    }
  }

  return;
}

static char constbuf[102400];
static char testbuf[102400];
static cell datastack[1024];
static void **returnstack[512];

int main() {
  here = testbuf;
  const_here = constbuf;

  setvbuf(stdin, NULL, _IONBF, 0);

  interpret(NULL, NULL, NULL);
  void **initprog = cfa(find_word("interpret"));
  interpret(initprog, datastack+1024, returnstack+512);
  return 0;
}
