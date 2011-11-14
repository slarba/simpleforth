BYTECODE(LATEST, "latest", 0, 0, { PUSH(&latest); })
BYTECODE(JUMP, "jump", 0, FLAG_HASARG, {
    void *fn = ARG();
    ip = fn;
  })
BYTECODE(CALL, "call", 0, FLAG_HASARG, { 
    void *fn = ARG();
    PUSHRS(ip);
    ip = fn;    
  })
BYTECODE(NOOP, "noop", 0, 0, {
  })
BYTECODE(EXECUTE, "execute", 1, 0, {
    PUSHRS(ip);
    ip = (void**)POP();    
  })
BYTECODE(IEXEC, "iexecute", 1, 0, {
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
BYTECODE(EOW, "eow", 0, 0, { /* end of word marker, do nothing */ })
BYTECODE(HIDDEN, "hidden", 1, 0, {
    dict_hdr_t *hdr = (dict_hdr_t*)POP();
    hdr->flags ^= FLAG_HIDDEN;    
  })
BYTECODE(TICK, "'", 0, FLAG_HASARG|FLAG_IMMED, {
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
BYTECODE(RETURN, "exit", 0, 0, { ip = POPRS(); })
BYTECODE(DIE, "die", 0, 0, { return; })
BYTECODE(BRANCH, "branch", 0, FLAG_HASARG, {
    tmp = INTARG();
    ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(0BRANCH, "0branch", 0, FLAG_HASARG, {
    tmp = INTARG();
    if(!POP()) ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(1BRANCH, "1branch", 0, FLAG_HASARG, {
    tmp = INTARG();
    if(POP()) ip += (tmp/sizeof(void*))-1;
  })
BYTECODE(FROMR, "r>", 0, 0, {
    tmp = (cell)POPRS();
    PUSH(tmp);    
  })
BYTECODE(TOR, ">r", 1, 0, {
    tmp = POP();
    PUSHRS(tmp);    
  })
BYTECODE(2FROMR, "2r>", 0, 0, {
    tmp = (cell)POPRS();
    PUSH(tmp);    
    tmp = (cell)POPRS();
    PUSH(tmp);    
  })
BYTECODE(2TOR, "2>r", 2, 0, {
    tmp = POP();
    PUSHRS(tmp);    
    tmp = POP();
    PUSHRS(tmp);    
  })
BYTECODE(RDROP, "rdrop", 0, 0, { (void)POPRS(); })
BYTECODE(2RDROP, "2rdrop", 0, 0, { (void)POPRS(); (void)POPRS(); })
BYTECODE(DSPFETCH, "dsp@", 0, 0, {
    tmp = (cell)ds;
    PUSH(tmp);    
  })
BYTECODE(DSPSTORE, "dsp!", 1, 0, {
    cell *newds = (cell*)POP();
    ds = newds;
  })
BYTECODE(RSPPUT, "rsp!", 1, 0, { rs = (void***)POP(); })
BYTECODE(RSPGET, "rsp@", 0, 0, { PUSH(rs); })
BYTECODE(LIT, "lit", 0, FLAG_HASARG, { PUSH(INTARG()); })
BYTECODE(LITPLUS, "lit+", 0, FLAG_HASARG, { AT(0) += INTARG(); })
BYTECODE(LITMINUS, "lit-", 0, FLAG_HASARG, { AT(0) -= INTARG(); })
BYTECODE(DUP, "dup", 1, 0, {
    tmp = TOP();
    PUSH(tmp);    
  })
BYTECODE(DUPAT, "dup@", 1, 0, {
    cell *addr = (cell*)TOP();
    PUSH(*addr);
  })
BYTECODE(NIP, "nip", 2, 0, {
    AT(1) = AT(0); ds++;
  })
BYTECODE(2NIP, "2nip", 3, 0, {
    AT(2) = AT(0); ds+=2;
  })
BYTECODE(2DUP, "2dup", 2, 0, {
    tmp = AT(1);
    PUSH(tmp);
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(CONDDUP, "?dup", 1, 0, {
    tmp = TOP();
    if(tmp) PUSH(tmp);    
  })
BYTECODE(SWAP, "swap", 2, 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;    
  })
BYTECODE(OVER, "over", 2, 0, {
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(TUCK, "tuck", 2, 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;
    tmp = AT(1);
    PUSH(tmp);
  })
BYTECODE(DROP, "drop", 1, 0, { ++ds; })
BYTECODE(ROT, "rot", 3, 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(ebx);
    PUSH(eax);
    PUSH(ecx);    
  })
BYTECODE(MROT, "-rot", 3, 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(eax);
    PUSH(ecx);
    PUSH(ebx);    
  })
BYTECODE(2DROP, "2drop", 2, 0, { ds += 2; })
BYTECODE(DIVMOD, "/mod", 2, 0, {
    cell a = POP();
    cell b = POP();
    PUSH( b % a );
    PUSH( b / a );    
  })
BYTECODE(ADD, "+", 2, 0, { 
    tmp = POP();
    AT(0) += tmp;     
  })
BYTECODE(SUB, "-", 2, 0, { 
    tmp = POP();
    AT(0) -= tmp;     
  })
BYTECODE(MUL, "*", 2, 0, { 
    tmp = POP();
    AT(0) *= tmp;     
  })
BYTECODE(DIV, "/", 2, 0, { 
    tmp = POP();
    AT(0) /= tmp;     
  })
BYTECODE(MOD, "mod", 2, 0, { 
    tmp = POP();
    AT(0) = AT(0) % tmp;     
  })
BYTECODE(NEG, "invert", 1, 0, {
    AT(0) = ~AT(0);     
  })
BYTECODE(ROL, "lshift", 2, 0, { 
    tmp = POP();
    AT(0) <<= tmp;    
  })
BYTECODE(ROR, "rshift", 2, 0, {
    tmp = POP();
    AT(0) >>= tmp;     
  })
BYTECODE(AND, "and", 2, 0, {
    tmp = POP();
    AT(0) &= tmp;     
  })
BYTECODE(OR, "or", 2, 0, {
    tmp = POP();
    AT(0) |= tmp;     
  })
BYTECODE(XOR, "xor", 2, 0, {
    tmp = POP();
    AT(0) ^= tmp;     
  })
BYTECODE(IMMEDIATE, "immediate", 0, FLAG_IMMED, { latest->flags ^= FLAG_IMMED; })
BYTECODE(LT, "<", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) < tmp;     
  })
BYTECODE(GTZ, "0>", 1, 0, {
    AT(0) = AT(0) > 0;    
  })
BYTECODE(LTZ, "0<", 1, 0, {
    AT(0) = AT(0) < 0;    
  })
BYTECODE(GT, ">", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) > tmp;     
  })
BYTECODE(LE, "<=", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) <= tmp;     
  })
BYTECODE(GE, ">=", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) >= tmp;     
  })
BYTECODE(EQ, "=", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) == tmp;    
  })
BYTECODE(NEQ, "<>", 2, 0, {
    tmp = POP();
    AT(0) = AT(0) != tmp;    
  })
BYTECODE(EQZERO, "0=", 1, 0, { AT(0) = AT(0)==0; })
BYTECODE(NOTEQZERO, "0<>", 1, 0, { AT(0) = AT(0)!=0; })
BYTECODE(FIND, "find", 1, 0, {
    char *wordname = (char*) POP();
    PUSH(find_word(wordname));    
  })
BYTECODE(CREATE, "create", 1, 0, { create_word((char*)POP(), 0); })
BYTECODE(WORD, "word", 0, 0, { PUSH(read_word(inputstate, wordbuf)); })
BYTECODE(IWORD, "iword", 0, 0, { PUSH(read_word(inputstate, linebuf)); })
BYTECODE(KEY, "key", 0, 0, { PUSH(read_key(inputstate)); })
BYTECODE(EMIT, "emit", 1, 0, { emit_char(POP(), outp); })
BYTECODE(ADD1, "1+", 1, 0, { AT(0) += 1; })
BYTECODE(SUB1, "1-", 1, 0, { AT(0) -= 1; })
BYTECODE(MEMADD, "+!", 2, 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr += tmp;    
  })
BYTECODE(MEMSUB, "-!", 2, 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr -= tmp;    
  })
BYTECODE(LBRAC, "[", 0, FLAG_IMMED, { state = STATE_IMMEDIATE; })
BYTECODE(RBRAC, "]", 0, 0, { state = STATE_COMPILE; })
BYTECODE(COMMA, ",", 1, 0, {
    tmp = POP();
    *(cell*)here = tmp;
    here += sizeof(cell);    
  })
BYTECODE(STORE, "!", 2, 0, {
    cell *ptr = (cell*)POP();
    tmp = POP();
    *ptr = tmp;    
  })
BYTECODE(FETCH, "@", 1, 0, {
    cell *ptr = (cell*)POP();
    PUSH(*ptr);    
  })
BYTECODE(VARAT, "var@", 0, FLAG_HASARG, {
    cell *ptr = (cell*)ARG();
    PUSH(*ptr);
  })
BYTECODE(VARTO, "var!", 0, FLAG_HASARG, {
    cell *ptr = (cell*)ARG();
    *ptr = POP();
  })
BYTECODE(CSTORE, "c!", 2, 0, {
    char *ptr = (char*)POP();
    tmp = POP();
    *ptr = (char)tmp;    
  })
BYTECODE(CFETCH, "c@", 1, 0, {
    char *ptr = (char*)POP();
    PUSH(*ptr);    
  })
BYTECODE(BYTECOPY, "c@c!", 2, 0, {
    char *src = (char*)AT(1);
    char *dst = (char*)AT(0);
    *dst++ = *src++;
    AT(0) = (cell)dst;
    AT(1) = (cell)src;
  })
BYTECODE(TOCFA, ">cfa", 1, 0, {
    dict_hdr_t *ptr = (dict_hdr_t*)POP();
    PUSH((ptr+1));    
  })
BYTECODE(TELL, "tell", 1, 0, { fprintf(stdout, (char*)POP()); })
BYTECODE(MALLOC, "malloc", 1, 0, {
    tmp = POP();
    PUSH(MALLOC(tmp));    
  })
BYTECODE(REALLOC, "mrealloc", 2, 0, {
    void *ptr = (void*)POP();
    tmp = POP();
    PUSH(REALLOC(ptr,tmp));
  })
BYTECODE(RUNGC, "rungc", 0, 0, {
    RUNGC();
  })
BYTECODE(MFREE, "mfree", 1, 0, { FREE((void*)POP()); })
BYTECODE(CCOPY, "ccopy", 3, 0, {
    tmp = POP();
    void *dst = (void*)POP();
    void *src = (void*)POP();
    memcpy(dst, src, tmp);
  })
BYTECODE(CMOVE, "cmove", 3, 0, {
    tmp = POP();
    void *dst = (void*)POP();
    void *src = (void*)POP();
    memmove(dst, src, tmp);
  })
BYTECODE(STRLENGTH, "strlen", 1, 0, {
    char *str = (char*)POP();
    PUSH(strlen(str));
  })
BYTECODE(STRCOPY, "strcpy", 2, 0, {
    char *dst = (char*)POP();
    char *src = (char*)POP();
    strcpy(dst, src);
  })
BYTECODE(STRCOMP, "strcmp", 2, 0, {
    char *b = (char*)POP();
    char *a = (char*)POP();
    PUSH(strcmp(a,b));
  })
BYTECODE(PARSENUM, "number", 1, 0, {
    char *endptr = NULL;
    char *str = (char*)POP();
    cell val = (cell)strtol(str, &endptr, base);
    if(*endptr!='\0') {
      printf("ERROR: not a valid number: %s\n", str);
    }
    else PUSH(val);
  })
BYTECODE(INTERPRET, "interpret", 0, 0, {
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
BYTECODE(IRETURN, "ireturn", 0, 0, {
    ip = *nestingstack++;    
  })
BYTECODE(OPENFILE, "open-file", 2, 0, {
    char *mode = (char*)POP();
    char *fn = (char*)POP();
    PUSH(open_file(fn, mode));    
  })
BYTECODE(CLOSEFILE, "close-file", 1, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    close_file(state);    
  })
BYTECODE(ISEOF, "?eof", 1, 0, {
    reader_state_t *state = (reader_state_t*)POP();
    PUSH(is_eof(state));
  })
