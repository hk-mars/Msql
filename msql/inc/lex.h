
#ifndef __LEX_H__
#define __LEX_H__

#include "tree.h"
#include "hsearch.h"


typedef
struct S_TOKEN
{
  char *key;
  char *value;
} token;

typedef
struct S_TOKEN_LIST
{
  token tk;
  struct S_TOKEN_LIST *next;
} token_list;

int dbg_lex(void);
int is_like_keyword(char *s);
void set_lex_tree(s_tr_node *root);
s_tr_node* get_lex_tree(void);
void set_kw_htab(HTAB_INFO *htab);
int is_token(char *key);
int check_lex(char *sql_str, token_list *tk_lst) ;
void free_tokens(token_list *tk_lst);


#endif /* __LEX_H__ */


