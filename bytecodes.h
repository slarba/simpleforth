BYTECODE(LATEST, "latest", 0, 0, 0, { PUSH(&latest); })
BYTECODE(JUMP, "jump", 0, 0, FLAG_HASARG, {
    void *fn = ARG();
    ip = fn;
  })
BYTECODE(CALL, "call", 0, 0, FLAG_HASARG, { 
    void *fn = ARG();
    PUSHRS(ip);
    ip = fn;    
  })
BYTECODE(NOOP, "noop", 0, 0, 0, {
  })
BYTECODE(EXECUTE, "execute", 1, 0, 0, {
    PUSHRS(ip);
    ip = (void**)POP();    
  })
BYTECODE(BUITINEXEC, "exec-builtin", 1, 0, 0, {
    *--nestingstack = ip;
    builtin_immediatebuf[0] = (void*)POP();
    ip = builtin_immediatebuf;
  })
BYTECODE(IEXEC, "iexecute", 1, 0, 0, {
    dict_hdr_t *entry = (dict_hdr_t*)POP();
    void **code = cfa(entry);
    *--nestingstack = ip;
    if(entry->flags & FLAG_BUILTIN) {
      builtin_immediatebuf[0] = *code;
      ip = builtin_immediatebuf;
    } else {
      word_immediatebuf[1] = (void*)code;
      ip = word_immediatebuf;
    }
  })
BYTECODE(EOW, "eow", 0, 0, 0, { /* end of word marker, do nothing */ })
BYTECODE(HIDDEN, "hidden", 1, 0, 0, {
    dict_hdr_t *hdr = (dict_hdr_t*)POP();
    hdr->flags ^= FLAG_HIDDEN;    
  })
BYTECODE(TICK, "'", 0, 0, FLAG_HASARG|FLAG_IMMED, {
    read_word(inputstate, wordbuf);
    dict_hdr_t *de = find_word(wordbuf);
    cell token;
    if(de->flags & FLAG_BUILTIN) {
      token = (cell)(*(cfa(de)));
    } else {
      token = (cell)cfa(de);
    }
    if(state==STATE_IMMEDIATE) {
      PUSH(token);
    } else {
      comma((cell)&&l_LIT);
      comma(token);
    }    
  })
BYTECODE(RETURN, "exit", 0, 0, 0, { ip = POPRS(); })
BYTECODE(DIE, "die", 0, 0, 0, { return; })
BYTECODE(BRANCH, "branch", 0, 0, FLAG_HASARG, {
    tmp = INTARG();
    ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(FIELDGET, "field@", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    void *ptr = (void*)(POP() + tmp);
    PUSH(*((cell*)ptr));
  })
BYTECODE(FIELDSET, "field!", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    void *ptr = (void*)(POP() + tmp);
    *((cell*)ptr) = POP();
  })
BYTECODE(FFIELDGET, "ffield@", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    void *ptr = (void*)(POP() + tmp);
    FPUSH(*((float*)ptr));
  })
BYTECODE(FFIELDSET, "ffield!", 1, 1, FLAG_HASARG, {
    tmp = INTARG();
    void *ptr = (void*)(POP() + tmp);
    *((float*)ptr) = FPOP();
  })
BYTECODE(0BRANCH, "0branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    if(!POP()) ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(1BRANCH, "1branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    if(POP()) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(LTBRANCH, "<branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a<b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(GTBRANCH, ">branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a>b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(LTEBRANCH, "<=branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a<=b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(GTEBRANCH, ">=branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a>=b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(LTZBRANCH, "0<branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell a = POP();
    if(a<0) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(GTZBRANCH, "0>branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell a = POP();
    if(a>0) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(LTEZBRANCH, "0<=branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell a = POP();
    if(a<=0) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(GTEZBRANCH, "0>=branch", 1, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell a = POP();
    if(a>=0) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(NEQBRANCH, "<>branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a!=b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(EQBRANCH, "=branch", 2, 0, FLAG_HASARG, {
    tmp = INTARG();
    cell b = POP();
    cell a = POP();
    if(a==b) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(FROMR, "r>", 0, 0, 0, {
    tmp = (cell)POPRS();
    PUSH(tmp);    
  })
BYTECODE(TOR, ">r", 1, 0, 0, {
    tmp = POP();
    PUSHRS(tmp);    
  })
BYTECODE(2FROMR, "2r>", 0, 0, 0, {
    tmp = (cell)POPRS();
    PUSH(tmp);    
    tmp = (cell)POPRS();
    PUSH(tmp);    
  })
BYTECODE(2TOR, "2>r", 2, 0, 0, {
    tmp = POP();
    PUSHRS(tmp);    
    tmp = POP();
    PUSHRS(tmp);    
  })
BYTECODE(RDROP, "rdrop", 0, 0, 0, { rs++; })
BYTECODE(2RDROP, "2rdrop", 0, 0, 0, { rs+=2; })
BYTECODE(DSPFETCH, "dsp@", 0, 0, 0, {
    tmp = (cell)ds;
    PUSH(tmp);    
  })
BYTECODE(DSPSTORE, "dsp!", 1, 0, 0, {
    cell *newds = (cell*)POP();
    ds = newds;
  })
BYTECODE(RSPPUT, "rsp!", 1, 0, 0, { rs = (void***)POP(); })
BYTECODE(RSPGET, "rsp@", 0, 0, 0, { PUSH(rs); })
BYTECODE(LIT, "lit", 0, 0, FLAG_HASARG, { PUSH(INTARG()); })
BYTECODE(LITPLUS, "lit+", 1, 0, FLAG_HASARG, { AT(0) += INTARG(); })
BYTECODE(LITMINUS, "lit-", 1, 0, FLAG_HASARG, { AT(0) -= INTARG(); })
BYTECODE(FLIT, "flit", 0, 0, FLAG_HASARG, { FPUSH(FLOATARG()); ip++; })
BYTECODE(FLITPLUS, "flit+", 0, 1, FLAG_HASARG, { FAT(0) += FLOATARG(); ip++; })
BYTECODE(FLITMINUS, "flit-", 0, 1, FLAG_HASARG, { FAT(0) -= FLOATARG(); ip++; })
BYTECODE(DUP, "dup", 1, 0, 0, {
    tmp = TOP();
    PUSH(tmp);    
  })
BYTECODE(FDUP, "fdup", 0, 1, 0, {
    float val = FTOP();
    FPUSH(val);    
  })
BYTECODE(FDUP2, "fdup2", 0, 2, 0, {
    float val = FAT(1);
    FPUSH(val);    
    val = FAT(1);
    FPUSH(val);
  })
BYTECODE(FDUPVEC, "fdupvec", 0, 3, 0, {
    float val = FAT(2);
    FPUSH(val);    
    val = FAT(2);
    FPUSH(val);
    val = FAT(2);
    FPUSH(val);
  })
BYTECODE(DUPAT, "dup@", 1, 0, 0, {
    cell *addr = (cell*)TOP();
    PUSH(*addr);
  })
BYTECODE(NIP, "nip", 2, 0, 0, {
    AT(1) = AT(0); ds++;
  })
BYTECODE(FNIP, "fnip", 0, 2, 0, {
    FAT(1) = FAT(0); fs++;
  })
BYTECODE(2NIP, "2nip", 3, 0, 0, {
    AT(2) = AT(0); ds+=2;
  })
BYTECODE(2FNIP, "2fnip", 0, 3, 0, {
    FAT(2) = FAT(0); fs+=2;
  })
BYTECODE(2DUP, "2dup", 2, 0, 0, {
    tmp = AT(1);
    PUSH(tmp);
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(F2DUP, "f2dup", 0, 2, 0, {
    float tmp = FAT(1);
    FPUSH(tmp);
    tmp = FAT(1);
    FPUSH(tmp);    
  })
BYTECODE(CONDDUP, "?dup", 1, 0, 0, {
    tmp = TOP();
    if(tmp) PUSH(tmp);    
  })
BYTECODE(SWAP, "swap", 2, 0, 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;    
  })
BYTECODE(FSWAP, "fswap", 0, 2, 0, {
    float tmp = FAT(1);
    FAT(1) = FAT(0);
    FAT(0) = tmp;    
  })
BYTECODE(SWAPDUP, "swapdup", 2, 0, 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;
    PUSH(tmp);
  })
BYTECODE(OVER, "over", 2, 0, 0, {
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(TUCK, "tuck", 2, 0, 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;
    tmp = AT(1);
    PUSH(tmp);
  })
BYTECODE(DROP, "drop", 1, 0, 0, { ++ds; })
BYTECODE(FDROP, "fdrop", 0, 1, 0, { ++fs; })
BYTECODE(ROT, "rot", 3, 0, 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(ebx);
    PUSH(eax);
    PUSH(ecx);    
  })
BYTECODE(MROT, "-rot", 3, 0, 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(eax);
    PUSH(ecx);
    PUSH(ebx);    
  })
BYTECODE(FROT, "frot", 0, 3, 0, {
    float eax = FPOP();
    float ebx = FPOP();
    float ecx = FPOP();
    FPUSH(ebx);
    FPUSH(eax);
    FPUSH(ecx);    
  })
BYTECODE(FMROT, "-frot", 0, 3, 0, {
    float eax = FPOP();
    float ebx = FPOP();
    float ecx = FPOP();
    FPUSH(eax);
    FPUSH(ecx);
    FPUSH(ebx);    
  })
BYTECODE(2DROP, "2drop", 2, 0, 0, { ds += 2; })
BYTECODE(DIVMOD, "/mod", 2, 0, 0, {
    cell a = POP();
    cell b = POP();
    PUSH( b % a );
    PUSH( b / a );    
  })
BYTECODE(UDIVMOD, "u/mod", 2, 0, 0, {
    cell a = POP();
    cell b = POP();
    PUSH( (unsigned)b % (unsigned)a );
    PUSH( (unsigned)b / (unsigned)a );    
  })
BYTECODE(ADD, "+", 2, 0, 0, { 
    tmp = POP();
    AT(0) += tmp;     
  })
BYTECODE(BIADD, "bi+", 3, 0, 0, { 
    tmp = POP();
    AT(0) += tmp;     
    tmp = POP();
    AT(0) += tmp;
  })
BYTECODE(SUB, "-", 2, 0, 0, { 
    tmp = POP();
    AT(0) -= tmp;     
  })
BYTECODE(MUL, "*", 2, 0, 0, { 
    tmp = POP();
    AT(0) *= tmp;     
  })
BYTECODE(DIV, "/", 2, 0, 0, { 
    tmp = POP();
    AT(0) /= tmp;     
  })
BYTECODE(MOD, "mod", 2, 0, 0, { 
    tmp = POP();
    AT(0) = AT(0) % tmp;     
  })
BYTECODE(NEG, "invert", 1, 0, 0, {
    AT(0) = ~AT(0);     
  })
BYTECODE(ROL, "lshift", 2, 0, 0, { 
    tmp = POP();
    AT(0) <<= tmp;    
  })
BYTECODE(ROR, "rshift", 2, 0, 0, {
    tmp = POP();
    AT(0) >>= tmp;     
  })
BYTECODE(AND, "and", 2, 0, 0, {
    tmp = POP();
    AT(0) &= tmp;     
  })
BYTECODE(OR, "or", 2, 0, 0, {
    tmp = POP();
    AT(0) |= tmp;     
  })
BYTECODE(XOR, "xor", 2, 0, 0, {
    tmp = POP();
    AT(0) ^= tmp;     
  })
BYTECODE(IMMEDIATE, "immediate", 0, 0, FLAG_IMMED, { latest->flags ^= FLAG_IMMED; })
BYTECODE(GTEZ, "0>=", 1, 0, 0, {
    AT(0) = AT(0) >= 0;    
  })
BYTECODE(LTEZ, "0<=", 1, 0, 0, {
    AT(0) = AT(0) <= 0;    
  })
BYTECODE(GTZ, "0>", 1, 0, 0, {
    AT(0) = AT(0) > 0;    
  })
BYTECODE(LTZ, "0<", 1, 0, 0, {
    AT(0) = AT(0) < 0;    
  })
BYTECODE(LT, "<", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) < tmp;     
  })
BYTECODE(GT, ">", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) > tmp;     
  })
BYTECODE(LE, "<=", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) <= tmp;     
  })
BYTECODE(GE, ">=", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) >= tmp;     
  })
BYTECODE(ULT, "u<", 2, 0, 0, {
    tmp = POP();
    AT(0) = (unsigned)AT(0) < (unsigned)tmp;     
  })
BYTECODE(UGT, "u>", 2, 0, 0, {
    tmp = POP();
    AT(0) = (unsigned)AT(0) > (unsigned)tmp;     
  })
BYTECODE(ULE, "u<=", 2, 0, 0, {
    tmp = POP();
    AT(0) = (unsigned)AT(0) <= (unsigned)tmp;     
  })
BYTECODE(UGE, "u>=", 2, 0, 0, {
    tmp = POP();
    AT(0) = (unsigned)AT(0) >= (unsigned)tmp;     
  })
BYTECODE(EQ, "=", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) == tmp;    
  })
BYTECODE(NEQ, "<>", 2, 0, 0, {
    tmp = POP();
    AT(0) = AT(0) != tmp;    
  })
BYTECODE(EQZERO, "0=", 1, 0, 0, { AT(0) = AT(0)==0; })
BYTECODE(NOTEQZERO, "0<>", 1, 0, 0, { AT(0) = AT(0)!=0; })
BYTECODE(FIND, "find", 1, 0, 0, {
    char *wordname = (char*) POP();
    PUSH(find_word(wordname));    
  })
BYTECODE(CREATE, "create", 1, 0, 0, { create_word((char*)POP(), 0); })
BYTECODE(WORD, "word", 0, 0, 0, { PUSH(read_word(inputstate, wordbuf)); })
BYTECODE(IWORD, "iword", 0, 0, 0, { PUSH(read_word(inputstate, linebuf)); })
BYTECODE(KEY, "key", 0, 0, 0, { PUSH(read_key(inputstate)); })
BYTECODE(EMIT, "emit", 1, 0, 0, { emit_char(POP(), outp); })
BYTECODE(ADD1, "1+", 1, 0, 0, { AT(0) += 1; })
BYTECODE(SUB1, "1-", 1, 0, 0, { AT(0) -= 1; })
BYTECODE(MEMADD, "+!", 2, 0, 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr += tmp;    
  })
BYTECODE(MEMSUB, "-!", 2, 0, 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr -= tmp;    
  })
BYTECODE(LBRAC, "[", 0, 0, FLAG_IMMED, { state = STATE_IMMEDIATE; })
BYTECODE(RBRAC, "]", 0, 0, 0, { state = STATE_COMPILE; })
BYTECODE(COMMA, ",", 1, 0, 0, {
    tmp = POP();
    *(cell*)here = tmp;
    here += sizeof(cell);    
  })
BYTECODE(FCOMMA, "f,", 0, 1, 0, {
    float val = FPOP();
    *(float*)here = val;
    here += sizeof(cell);    
  })
BYTECODE(STORE, "!", 2, 0, 0, {
    cell *ptr = (cell*)POP();
    tmp = POP();
    *ptr = tmp;    
  })
BYTECODE(FETCH, "@", 1, 0, 0, {
    cell *ptr = (cell*)POP();
    PUSH(*ptr);    
  })
BYTECODE(FSTORE, "f!", 1, 1, 0, {
    float *ptr = (float*)POP();
    float val = FPOP();
    *ptr = val;
  })
BYTECODE(FFETCH, "f@", 1, 0, 0, {
    float *ptr = (float*)POP();
    FPUSH(*ptr);    
  })
BYTECODE(VARAT, "var@", 0, 0, FLAG_HASARG, {
    cell *ptr = (cell*)ARG();
    PUSH(*ptr);
  })
BYTECODE(VARTO, "var!", 1, 0, FLAG_HASARG, {
    cell *ptr = (cell*)ARG();
    *ptr = POP();
  })
BYTECODE(FVARAT, "fvar@", 0, 0, FLAG_HASARG, {
    float *ptr = (float*)ARG();
    FPUSH(*ptr);
  })
BYTECODE(FVARTO, "fvar!", 0, 1, FLAG_HASARG, {
    float *ptr = (float*)ARG();
    *ptr = FPOP();
  })
BYTECODE(CSTORE, "c!", 2, 0, 0, {
    char *ptr = (char*)POP();
    tmp = POP();
    *ptr = (char)tmp;    
  })
BYTECODE(CFETCH, "c@", 1, 0, 0, {
    char *ptr = (char*)POP();
    PUSH(*ptr);    
  })
BYTECODE(SSTORE, "s!", 2, 0, 0, {
    short *ptr = (short*)POP();
    tmp = POP();
    *ptr = (short)tmp;    
  })
BYTECODE(SFETCH, "s@", 1, 0, 0, {
    short *ptr = (short*)POP();
    PUSH(*ptr);    
  })
BYTECODE(BYTECOPY, "c@c!", 2, 0, 0, {
    char *src = (char*)AT(1);
    char *dst = (char*)AT(0);
    *dst++ = *src++;
    AT(0) = (cell)dst;
    AT(1) = (cell)src;
  })
BYTECODE(TOCFA, ">cfa", 1, 0, 0, {
    dict_hdr_t *ptr = (dict_hdr_t*)POP();
    PUSH((ptr+1));    
  })
BYTECODE(TELL, "tell", 1, 0, 0, { fputs((char*)POP(), outp); })
BYTECODE(MALLOC, "malloc", 1, 0, 0, {
    tmp = POP();
    PUSH(MALLOC(tmp));    
  })
BYTECODE(MALLOCATOM, "malloc-atomic", 1, 0, 0, {
    tmp = POP();
    PUSH(MALLOC_ATOMIC(tmp));
  })
BYTECODE(REALLOC, "mrealloc", 2, 0, 0, {
    void *ptr = (void*)POP();
    tmp = POP();
    PUSH(REALLOC(ptr,tmp));
  })
BYTECODE(RUNGC, "rungc", 0, 0, 0, {
    RUNGC();
  })
BYTECODE(MFREE, "mfree", 1, 0, 0, { FREE((void*)POP()); })
BYTECODE(CCOPY, "ccopy", 3, 0, 0, {
    tmp = POP();
    void *dst = (void*)POP();
    void *src = (void*)POP();
    memcpy(dst, src, tmp);
  })
BYTECODE(CMOVE, "cmove", 3, 0, 0, {
    tmp = POP();
    void *dst = (void*)POP();
    void *src = (void*)POP();
    memmove(dst, src, tmp);
  })
BYTECODE(STRCAT, "strcat", 2, 0, 0, {
    char *dst = (char*)POP();
    char *src = (char*)POP();
    PUSH(strcat(dst, src));
  })
BYTECODE(STRLENGTH, "strlen", 1, 0, 0, {
    char *str = (char*)POP();
    PUSH(strlen(str));
  })
BYTECODE(STRNCOPY, "strncpy", 3, 0, 0, {
    tmp = POP();
    char *dst = (char*)POP();
    char *src = (char*)POP();
    PUSH(strncpy(dst, src, tmp));    
  })
BYTECODE(STRCOPY, "strcpy", 2, 0, 0, {
    char *dst = (char*)POP();
    char *src = (char*)POP();
    PUSH(strcpy(dst, src));
  })
BYTECODE(STRCOMP, "strcmp", 2, 0, 0, {
    char *b = (char*)POP();
    char *a = (char*)POP();
    PUSH(strcmp(a,b));
  })
BYTECODE(PARSENUM, "number", 1, 0, 0, {
    char *endptr = NULL;
    char *str = (char*)POP();
    cell val = (cell)strtol(str, &endptr, base);
    if(*endptr!='\0') {
      PUSH(0);
    } else {
      PUSH(val);
      PUSH(1);
    }
  })
BYTECODE(PARSEFNUM, "fnumber", 1, 0, 0, {
    char *endptr = NULL;
    char *str = (char*)POP();
    float val = strtof(str, &endptr);
    if(*endptr!='\0') {
      PUSH(0);
    } else {
      FPUSH(val);
      PUSH(1);
    }
  })
BYTECODE(INTERPRET, "interpret", 0, 0, 0, {
    char *word = read_word(inputstate,linebuf);
    if(!word) NEXT();
    dict_hdr_t *entry = find_word(word);
    if(!entry) {
      char *endptr = NULL;
      cell val = (cell)strtol(word, &endptr, base);
      if(*endptr!='\0') {
	printf("ERROR: no such word: %s\n", word);
      } else {
	if(state==STATE_COMPILE) {
	  comma((cell) &&l_LIT);
	  comma(val);
	} else {
	  PUSH(val);
	}
      }
      NEXT();
    }
    if(state==STATE_COMPILE && !(entry->flags & FLAG_IMMED)) {
      if(entry->flags & FLAG_BUILTIN) {
	comma((cell)(*cfa(entry)));
      } else {
	  comma((cell) &&l_CALL);
	  comma((cell) cfa(entry));
      }
    } else {
      void **code = cfa(entry);
      *--nestingstack = ip;
      if(entry->flags & FLAG_BUILTIN) {
	builtin_immediatebuf[0] = *code;
	ip = builtin_immediatebuf;
      } else {
	word_immediatebuf[1] = (void*)code;
	ip = word_immediatebuf;
      }
    }
  })
BYTECODE(IRETURN, "ireturn", 0, 0, 0, {
    ip = *nestingstack++;    
  })
BYTECODE(OPENFILE, "open-file", 2, 0, 0, {
    char *mode = (char*)POP();
    char *fn = (char*)POP();
    PUSH(open_file(fn, mode));    
  })
BYTECODE(CLOSEFILE, "close-file", 1, 0, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    close_file(state);    
  })
BYTECODE(ISEOF, "?eof", 1, 0, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    PUSH(is_eof(state));
  })
BYTECODE(ISEOL, "?eol", 1, 0, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    PUSH(is_eol(state));
  })
BYTECODE(PROMPT, "prompt", 2, 0, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    char *prompt = (char*)POP();
    prompt_line(prompt, state);
  })
BYTECODE(GETT0, "tsp@", 0, 0, 0, { PUSH(ts); })
BYTECODE(SETT0, "tsp!", 1, 0, 0, { ts = (cell*)POP(); })
BYTECODE(TOTMP, ">t", 1, 0, 0, { *--ts = POP(); })
BYTECODE(FROMTMP, "t>", 0, 0, 0, { PUSH(*ts++); })
BYTECODE(RESETCALLVM, "dcreset", 0, 0, 0, { dcReset(callvm); })
BYTECODE(DCARGBOOL, "dcbool", 1, 0, 0, { dcArgBool(callvm, (DCbool)POP()); })
BYTECODE(DCARGCHAR, "dcchar", 1, 0, 0, { dcArgChar(callvm, (DCchar)POP()); })
BYTECODE(DCARGSHORT, "dcshort", 1, 0, 0, { dcArgShort(callvm, (DCshort)POP()); })
BYTECODE(DCARGFLOAT, "dcfloat", 0, 1, 0, { dcArgFloat(callvm, (DCfloat)FPOP()); })
BYTECODE(DCARGDOUBLE, "dcdouble", 0, 1, 0, { dcArgDouble(callvm, (DCdouble)FPOP()); })
BYTECODE(DCARGINT, "dcint", 1, 0, 0, { dcArgInt(callvm, (DCint)POP()); })
BYTECODE(DCARGLONG, "dclong", 1, 0, 0, { dcArgLong(callvm, (DClong)POP()); })
BYTECODE(DCARGPTR, "dcptr", 1, 0, 0, { dcArgPointer(callvm, (DCpointer)POP()); })
BYTECODE(DCCALLVOID, "dccallvoid", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    dcCallVoid(callvm, funcptr);
  })
BYTECODE(DCCALLBOOL, "dccallbool", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallBool(callvm, funcptr));
  })
BYTECODE(DCCALLFLOAT, "dccallfloat", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    FPUSH(dcCallFloat(callvm, funcptr));
  })
BYTECODE(DCCALLDOUBLE, "dccalldouble", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    FPUSH(dcCallDouble(callvm, funcptr));
  })
BYTECODE(DCCALLCHAR, "dccallchar", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallChar(callvm, funcptr));
  })
BYTECODE(DCCALLSHORT, "dccallshort", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallShort(callvm, funcptr));
  })
BYTECODE(DCCALLINT, "dccallint", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallInt(callvm, funcptr));
  })
BYTECODE(DCCALLLONG, "dccalllong", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallLong(callvm, funcptr));
  })
BYTECODE(DCCALLPTR, "dccallptr", 1, 0, 0, {
    DCpointer funcptr = (DCpointer)POP();
    PUSH(dcCallPointer(callvm, funcptr));
  })
BYTECODE(DCLOAD, "dcloadlib", 1, 0, 0, {
    char *libname = (char*)POP();
    PUSH(dlLoadLibrary(libname));
  })
BYTECODE(DCFREE, "dcfreelib", 1, 0, 0, {
    dlFreeLibrary((void*)POP());
  })
BYTECODE(DCSYM, "dcsymbol", 2, 0, 0, {
    void *lib = (void*)POP();
    char *symbol = (char*)POP();
    PUSH(dlFindSymbol(lib, symbol));
  })
BYTECODE(NEWTHREAD, "new-thread", 4, 0, 0, {
    int ds_size = (int)POP();
    int rs_size = (int)POP();
    int ts_size = (int)POP();
    void **entry = (void**)POP();
    PUSH(create_thread(ds_size, rs_size, ts_size, entry));
  })
BYTECODE(KILLTHREAD, "kill-thread", 0, 0, 0, {
    kill_thread();

    ip = current_thread->ip;
    ds = current_thread->ds;
    rs = current_thread->rs;
    ts = current_thread->ts;
    fs = current_thread->fs;
    t0 = current_thread->t0;
    s0 = current_thread->s0;
    r0 = current_thread->r0;
    f0 = current_thread->f0;
  })
BYTECODE(SWITCHTHREAD, "pause", 0, 0, 0, {
    current_thread->ip = ip;
    current_thread->ds = ds;
    current_thread->rs = rs;
    current_thread->ts = ts;
    current_thread->fs = fs;
    current_thread->t0 = t0;
    current_thread->s0 = s0;
    current_thread->r0 = r0;
    current_thread->f0 = f0;

    current_thread = current_thread->next;

    ip = current_thread->ip;
    ds = current_thread->ds;
    rs = current_thread->rs;
    ts = current_thread->ts;
    fs = current_thread->fs;
    t0 = current_thread->t0;
    s0 = current_thread->s0;
    r0 = current_thread->r0;
    f0 = current_thread->f0;
  })
BYTECODE(SETFS, "fsp!", 1, 0, 0, {
    fs = (float*)POP();
  })
BYTECODE(GETFS, "fsp@", 0, 0, 0, {
    PUSH(fs);
  })
BYTECODE(FADD, "f+", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(a+b);
  })
BYTECODE(FSUB, "f-", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(a-b);
  })
BYTECODE(FMUL, "f*", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(a*b);
  })
BYTECODE(FDIV, "f/", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(a/b);
  })
BYTECODE(POWF, "powf", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(powf(a,b));
  })
BYTECODE(FLT, "f<", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    PUSH(a<b);
  })
BYTECODE(FGT, "f>", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    PUSH(a>b);
  })
BYTECODE(FLE, "f<=", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    PUSH(a<=b);
  })
BYTECODE(FGE, "f>=", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    PUSH(a>=b);
  })
BYTECODE(FABS, "fabs", 0, 1, 0, {
    float a = FPOP();
    FPUSH(fabs(a));
  })
BYTECODE(FFLOOR, "ffloor", 0, 1, 0, {
    float a = FPOP();
    FPUSH(floorf(a));
  })
BYTECODE(FSQRT, "fsqrt", 0, 1, 0, {
    float a = FPOP();
    FPUSH(sqrtf(a));
  })
BYTECODE(FSIN, "fsin", 0, 1, 0, {
    float a = FPOP();
    FPUSH(sinf(a));
  })
BYTECODE(FCOS, "fcos", 0, 1, 0, {
    float a = FPOP();
    FPUSH(cosf(a));
  })
BYTECODE(FTAN, "ftan", 0, 1, 0, {
    float a = FPOP();
    FPUSH(tanf(a));
  })
BYTECODE(FASIN, "fasin", 0, 1, 0, {
    float a = FPOP();
    FPUSH(asinf(a));
  })
BYTECODE(FACOS, "facos", 0, 1, 0, {
    float a = FPOP();
    FPUSH(acosf(a));
  })
BYTECODE(FATAN, "fatan", 0, 1, 0, {
    float a = FPOP();
    FPUSH(atanf(a));
  })
BYTECODE(FATAN2, "fatan2", 0, 2, 0, {
    float b = FPOP();
    float a = FPOP();
    FPUSH(atan2f(a,b));
  })
BYTECODE(FCEIL, "fceil", 0, 1, 0, {
    float a = FPOP();
    FPUSH(ceilf(a));
  })
BYTECODE(FTOI, "f>i", 0, 1, 0, {
    float a = FPOP();
    PUSH((cell)a);
  })
BYTECODE(ITOF, "i>f", 1, 0, 0, {
    cell a = POP();
    FPUSH(a);
  })
BYTECODE(V3ADD, "v3+", 0, 6, 0, {
    FAT(3) += FAT(0);
    FAT(4) += FAT(1);
    FAT(5) += FAT(2);
    fs+=3;
  })
BYTECODE(V3SUB, "v3-", 0, 6, 0, {
    FAT(3) -= FAT(0);
    FAT(4) -= FAT(1);
    FAT(5) -= FAT(2);
    fs+=3;
  })
BYTECODE(V3SCALARMULT, "v3s*", 0, 4, 0, {
    FAT(1) *= FAT(0);
    FAT(2) *= FAT(0);
    FAT(3) *= FAT(0);
    fs+=1;
  })
BYTECODE(V3SCALARDIV, "v3s/", 0, 4, 0, {
    FAT(1) /= FAT(0);
    FAT(2) /= FAT(0);
    FAT(3) /= FAT(0);
    fs+=1;
  })
BYTECODE(V3DOT, "v3dot", 0, 6, 0, {
    float result = FAT(0)*FAT(3) + FAT(1)*FAT(4) + FAT(2)*FAT(5);
    fs+=6;
    FPUSH(result);
  })
BYTECODE(V3LENSQUARED, "v3len^2", 0, 3, 0, {
    float result = FAT(0)*FAT(0) + FAT(1)*FAT(1) + FAT(2)*FAT(2);
    fs+=3;
    FPUSH(result);
  })
BYTECODE(V3CROSS, "v3cross", 0, 6, 0, {
    float a1 = FAT(5);
    float a2 = FAT(4);
    float a3 = FAT(3);
    float b1 = FAT(2);
    float b2 = FAT(1);
    float b3 = FAT(0);
    fs += 6;
    FPUSH(a2*b3 - a3*b2);
    FPUSH(a3*b1 - a1*b3);
    FPUSH(a1*b2 - a2*b1);
  })
BYTECODE(M33VMUL, "matvec3*", 0, 12, 0, {
    float a1 = FAT(11)*FAT(2) + FAT(10)*FAT(1) + FAT(9)*FAT(0);
    float a2 = FAT(8)*FAT(2) + FAT(7)*FAT(1) + FAT(6)*FAT(0);
    float a3 = FAT(5)*FAT(2) + FAT(4)*FAT(1) + FAT(3)*FAT(0);
    fs += 12;
    FPUSH(a1);
    FPUSH(a2);
    FPUSH(a3);
  })
BYTECODE(M33M33MUL, "matmat*", 0, 18, 0, {
    // m11 m12 m13 m21 m22 m23 m31 m32 m33
    // 8   7   6   5   4   3   2   1   0   
    // m11 m12 m13 m21 m22 m23 m31 m32 m33
    // 17  16  15  14  13  12  11  10  9
    float m11 = FAT(17)*FAT(8) + FAT(16)*FAT(5) + FAT(15)*FAT(2);
    float m12 = FAT(17)*FAT(7) + FAT(16)*FAT(4) + FAT(15)*FAT(1);
    float m13 = FAT(17)*FAT(6) + FAT(16)*FAT(3) + FAT(15)*FAT(0);

    float m21 = FAT(14)*FAT(8) + FAT(13)*FAT(5) + FAT(12)*FAT(2);
    float m22 = FAT(14)*FAT(7) + FAT(13)*FAT(4) + FAT(12)*FAT(1);
    float m23 = FAT(14)*FAT(6) + FAT(13)*FAT(3) + FAT(12)*FAT(0);

    float m31 = FAT(11)*FAT(8) + FAT(10)*FAT(5) + FAT(9)*FAT(2);
    float m32 = FAT(11)*FAT(7) + FAT(10)*FAT(4) + FAT(9)*FAT(1);
    float m33 = FAT(11)*FAT(6) + FAT(10)*FAT(3) + FAT(9)*FAT(0);

    fs += 9;

    FAT(8) = m11; FAT(7) = m12; FAT(6) = m13;
    FAT(5) = m21; FAT(4) = m22; FAT(3) = m23;
    FAT(2) = m31; FAT(1) = m32; FAT(0) = m33;
  })
BYTECODE(STOREV3, "storev3", 1, 3, 0, {
    float *dst = (float*)POP();
    *dst++ = FAT(2);
    *dst++ = FAT(1);
    *dst = FAT(0);
    fs += 3;
  })
BYTECODE(LOADV3, "loadv3", 1, 0, 0, {
    float *src = (float*)POP();
    fs -= 3;
    FAT(2) = *src++;
    FAT(1) = *src++;
    FAT(0) = *src;
  })
BYTECODE(DCMODE, "dcmode", 1, 0, 0, {
    dcMode(callvm, POP());
  })
BYTECODE(FORMAT, "format", 1, 0, 0, {
    char *output = NULL;
    char *fmt = (char*)POP();
    dcReset(callvm);
    dcMode(callvm, DC_CALL_C_ELLIPSIS);
    dcArgPointer(callvm, &output);
    dcArgPointer(callvm, fmt);
    while(*fmt!='\0') {
      if(*fmt++=='%') {
	switch(*fmt++) {
	case '%':
	  continue;
	case 'd':
	  dcArgInt(callvm, (int)POP());
	  break;
	case 'e':
	case 'f':
	  dcArgDouble(callvm, (double)FPOP());
	  break;
	case 's':
	  dcArgPointer(callvm, (void*)POP());
	  break;
	}
      }
    }
    dcCallInt(callvm, &asprintf);
    dcMode(callvm, DC_CALL_C_DEFAULT);
    char *tmp = (char*)MALLOC_ATOMIC(strlen(output)+1);
    strcpy(tmp, output);
    free(output);
    PUSH(tmp);
  })

