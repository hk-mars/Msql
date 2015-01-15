

#ifndef __SYNTAX_H__
#define __SYNTAX_H__


#include "tree.h"
#include "hsearch.h"
#include "lex.h"


int create_syntax_htab(int cnt);
HTAB_INFO* get_syntax_htab(void);
int push_syntax_htab(char *key, s_tr_node *root);
ENTRY* pop_syntax_htab(char *key);
int check_syntax(s_tr_node *root, token_list *tk_lst);
int dbg_syntax(void);


#endif /* __SYNTAX_H__ */


