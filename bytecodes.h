BYTECODE(LATEST, "latest", 0, { PUSH(&latest); })
BYTECODE(CALL, "call", FLAG_HASARG, { 
    void *fn = ARG();
    PUSHRS(ip);
    ip = fn;    
  })
BYTECODE(EXECUTE, "execute", 0, {
    PUSHRS(ip);
    ip = (void**)POP();    
  })
BYTECODE(EOW, "eow", 0, { /* end of word marker, do nothing */ })
BYTECODE(HIDDEN, "hidden", 0, {
    dict_hdr_t *hdr = (dict_hdr_t*)POP();
    hdr->flags ^= FLAG_HIDDEN;    
  })
BYTECODE(TICK, "'", FLAG_HASARG|FLAG_IMMED, {
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
BYTECODE(RETURN, "exit", 0, { ip = POPRS(); })
BYTECODE(DIE, "die", 0, { return; })
BYTECODE(BRANCH, "branch", FLAG_HASARG, {
    tmp = INTARG();
    ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(0BRANCH, "0branch", FLAG_HASARG, {
    tmp = INTARG();
    if(!POP()) ip += (tmp/sizeof(void*))-1;    
  })
BYTECODE(FROMR, "r>", 0, {
    tmp = (cell)POPRS();
    PUSH(tmp);    
  })
BYTECODE(TOR, ">r", 0, {
    tmp = POP();
    PUSHRS(tmp);    
  })
BYTECODE(RDROP, "rdrop", 0, { (void)POPRS(); })
BYTECODE(DSPFETCH, "dsp@", 0, {
    tmp = (cell)ds;
    PUSH(tmp);    
  })
BYTECODE(RSPPUT, "rsp!", 0, { rs = (void***)POP(); })
BYTECODE(RSPGET, "rsp@", 0, { PUSH(rs); })
BYTECODE(LIT, "lit", FLAG_HASARG, { PUSH(INTARG()); })
BYTECODE(DUP, "dup", 0, {
    tmp = TOP();
    PUSH(tmp);    
  })
BYTECODE(2DUP, "2dup", 0, {
    tmp = AT(1);
    PUSH(tmp);
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(CONDDUP, "?dup", 0, {
    tmp = TOP();
    if(tmp) PUSH(tmp);    
  })
BYTECODE(SWAP, "swap", 0, {
    tmp = AT(1);
    AT(1) = AT(0);
    AT(0) = tmp;    
  })
BYTECODE(OVER, "over", 0, {
    tmp = AT(1);
    PUSH(tmp);    
  })
BYTECODE(DROP, "drop", 0, { ++ds; })
BYTECODE(ROT, "rot", 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(ebx);
    PUSH(eax);
    PUSH(ecx);    
  })
BYTECODE(MROT, "-rot", 0, {
    cell eax = POP();
    cell ebx = POP();
    cell ecx = POP();
    PUSH(eax);
    PUSH(ecx);
    PUSH(ebx);    
  })
BYTECODE(2DROP, "2drop", 0, { ds += 2; })
BYTECODE(DIVMOD, "/mod", 0, {
    cell a = POP();
    cell b = POP();
    PUSH( b % a );
    PUSH( b / a );    
  })
BYTECODE(ADD, "+", 0, { 
    tmp = POP();
    AT(0) += tmp;     
  })
BYTECODE(SUB, "-", 0, { 
    tmp = POP();
    AT(0) -= tmp;     
  })
BYTECODE(MUL, "*", 0, { 
    tmp = POP();
    AT(0) *= tmp;     
  })
BYTECODE(DIV, "/", 0, { 
    tmp = POP();
    AT(0) /= tmp;     
  })
BYTECODE(MOD, "mod", 0, { 
    tmp = POP();
    AT(0) = AT(0) % tmp;     
  })
BYTECODE(NEG, "invert", 0, {
    AT(0) = ~AT(0);     
  })
BYTECODE(ROL, "lshift", 0, { 
    tmp = POP();
    AT(0) <<= tmp;    
  })
BYTECODE(ROR, "rshift", 0, {
    tmp = POP();
    AT(0) >>= tmp;     
  })
BYTECODE(AND, "and", 0, {
    tmp = POP();
    AT(0) &= tmp;     
  })
BYTECODE(OR, "or", 0, {
    tmp = POP();
    AT(0) |= tmp;     
  })
BYTECODE(XOR, "xor", 0, {
    tmp = POP();
    AT(0) ^= tmp;     
  })
BYTECODE(IMMEDIATE, "immediate", FLAG_IMMED, { latest->flags ^= FLAG_IMMED; })
BYTECODE(LT, "<", 0, {
    tmp = POP();
    AT(0) = AT(0) < tmp;     
  })
BYTECODE(GTZ, "0>", 0, {
    AT(0) = AT(0) > 0;    
  })
BYTECODE(LTZ, "0<", 0, {
    AT(0) = AT(0) < 0;    
  })
BYTECODE(GT, ">", 0, {
    tmp = POP();
    AT(0) = AT(0) > tmp;     
  })
BYTECODE(LE, "<=", 0, {
    tmp = POP();
    AT(0) = AT(0) <= tmp;     
  })
BYTECODE(GE, ">=", 0, {
    tmp = POP();
    AT(0) = AT(0) >= tmp;     
  })
BYTECODE(EQ, "=", 0, {
    tmp = POP();
    AT(0) = AT(0) == tmp;    
  })
BYTECODE(NEQ, "<>", 0, {
    tmp = POP();
    AT(0) = AT(0) != tmp;    
  })
BYTECODE(EQZERO, "0=", 0, { AT(0) = AT(0)==0; })
BYTECODE(NOTEQZERO, "0<>", 0, { AT(0) = AT(0)!=0; })
BYTECODE(FIND, "find", 0, {
    char *wordname = (char*) POP();
    PUSH(find_word(wordname));    
  })
BYTECODE(CREATE, "create", 0, { create_word((char*)POP(), 0); })
BYTECODE(WORD, "word", 0, { PUSH(read_word(inputstate, wordbuf)); })
BYTECODE(KEY, "key", 0, { PUSH(read_key(inputstate)); })
BYTECODE(EMIT, "emit", 0, { emit_char(POP(), outp); })
BYTECODE(ADD1, "1+", 0, { AT(0) += 1; })
BYTECODE(SUB1, "1-", 0, { AT(0) -= 1; })
BYTECODE(MEMADD, "+!", 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr += tmp;    
  })
BYTECODE(MEMSUB, "-!", 0, {
    cell *addr = (cell*)POP();
    tmp = POP();
    *addr -= tmp;    
  })
BYTECODE(LBRAC, "[", FLAG_IMMED, { state = STATE_IMMEDIATE; })
BYTECODE(RBRAC, "]", 0, { state = STATE_COMPILE; })
BYTECODE(COMMA, ",", 0, {
    tmp = POP();
    *(cell*)here = tmp;
    here += sizeof(cell);    
  })
BYTECODE(STORE, "!", 0, {
    cell *ptr = (cell*)POP();
    tmp = POP();
    *ptr = tmp;    
  })
BYTECODE(FETCH, "@", 0, {
    cell *ptr = (cell*)POP();
    PUSH(*ptr);    
  })
BYTECODE(CSTORE, "c!", 0, {
    char *ptr = (char*)POP();
    tmp = POP();
    *ptr = (char)tmp;    
  })
BYTECODE(CFETCH, "c@", 0, {
    char *ptr = (char*)POP();
    PUSH(*ptr);    
  })
BYTECODE(TOCFA, ">cfa", 0, {
    dict_hdr_t *ptr = (dict_hdr_t*)POP();
    PUSH((ptr+1));    
  })
BYTECODE(TELL, "tell", 0, { fprintf(stdout, (char*)POP()); })
BYTECODE(MALLOC, "malloc", 0, {
    tmp = POP();
    PUSH(malloc(tmp));    
  })
BYTECODE(MFREE, "mfree", 0, { free((void*)POP()); })
BYTECODE(CMOVE, "cmove", 0, {
    tmp = POP();
    void *dst = (void*)POP();
    void *src = (void*)POP();
    memcpy(dst, src, tmp);
  })
BYTECODE(INTERPRET, "interpret", 0, {
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
BYTECODE(IRETURN, "interpretreturn", 0, {
    ip = *nestingstack++;    
  })
BYTECODE(OPENFILE, "open-file", 0, {
    char *mode = (char*)POP();
    char *fn = (char*)POP();
    PUSH(open_file(fn, mode));    
  })
BYTECODE(CLOSEFILE, "close-file", 0, {
    reader_state_t *state = (reader_state_t*)POP();
    close_file(state);    
  })
BYTECODE(ISEOF, "?eof", 0, {
    reader_state_t *state = (reader_state_t*)POP();
    PUSH(is_eof(state));
  })
