
#include "common.h"
#include "lex.h"
#include "char.h"
#include "gc.h"


static s_tr_node *lex_tr_root;
static HTAB_INFO *kw_htab;

void 
set_lex_tree(s_tr_node *root)
{
  lex_tr_root = root;
}


s_tr_node*
get_lex_tree(void)
{
  return lex_tr_root;
}

void 
set_kw_htab(HTAB_INFO *htab)
{
  kw_htab = htab;
}


static void
show_buf(char *buf, int size)
{
  debug("%d bytes:\n", size);
  buf += (size - 1);
  while(--size >= 0) debug("%c", *(buf - size));
  debug("\n");
}


static void
show_path(s_tr_node *s)
{
  fs();
  
  while(s) {
    debug("%s \n", s->key);
    s = s->next;
  }
  
  fe();
}


static int 
get_tk_key(char *s, char *e, char **ss, char **ee)
{
  while (s <= e)  {
  
    if (*s == ' ' || *s == '\r' || *s == '\n') {
      s++;
      continue;
    }
    if (s == e) return 0;
    
    *ee = memchr(s, ' ', e - s + 1);
    if (!*ee) {
      *ee = memchr(s, '\r', e - s + 1);
      if (!*ee) return 0;	
    }
    
    (*ee)--;
    *ss = s;
    return 1;
  }
  
  return 0;
}


static s_tr_node*
search_end(s_tr_node *root)
{
  s_tr_node *rtn;
  
  if (!root) return NULL;
  
  //debug("%s \n", root->key);
  
  if (strlen(root->key) == 1) {
  
    if (*root->key != '@') return NULL;
    if (root->right || root->left || root->sub) return NULL;
    
    //debug("@e. \n");
    return root;
  }
  
  if (root->sub) {
    //debug("sub %s \n", root->sub->key);
    rtn = search_end(root->sub);
    if (rtn) return rtn;
  }
  
  if (root->left) {
    //debug("left %s \n", root->left->key);
    rtn = search_end(root->left);
    if (rtn) return rtn;
  } 
  
  if (root->right) {
    //debug("right %s \n", root->right->key);
    rtn = search_end(root->right);
    if (rtn) return rtn;
  }
  
  return NULL;
}


static s_tr_node*
search_graph(s_tr_node *root, char *s, char *e)
{
  s_tr_node *rtn;
  
  if (!root) return NULL;

  if (strlen(root->key) == 1 && *root->key != '@') {
  
    if (*root->key != *s) return NULL;
    
    //debug("found %c \n", *s);
    
    s++;
    
    /* the end char, check is the end node.
       */
    if (s > e) {
    
      //debug("all chars found. \n");
      
      if (root->sub || root->left || root->right) {
      
        rtn = search_end(root->sub);
        if (rtn) return root;
        
        rtn = search_end(root->left);
        if (rtn) return root;
        
        rtn = search_end(root->right);
        if (rtn) return root;
        
        return NULL;
      }
      //debug("@e. \n");
      return root;
    }
    
    //debug("try find %c \n", *s);
    
    if (root->back) {
      //debug("go back to node: %s \n", root->back->key);
      rtn = search_graph(root->back, s, e);
      if (rtn) return rtn;
      
      //debug("not found. \n");
    }
  }

  if (root->sub) {
  
    //debug("sub %s \n", root->sub->key);
    root->next = root->sub;
    rtn = search_graph(root->sub, s, e);
    if (rtn) return rtn;
  }
  
  if (root->left) {
  
    //debug("left %s \n", root->left->key);
    root->next = root->left;
    rtn = search_graph(root->left, s, e);
    if (rtn) return rtn;
  }
  
  if (root->right) {
  
    //debug("right %s \n", root->right->key);
    root->next = root->right;
    rtn = search_graph(root->right, s, e);
    if (rtn) return rtn;
  }
  
  root->next = NULL;
  return NULL;
}


static s_tr_node*
get_token_node(s_tr_node *path)
{
  s_tr_node *sn, *en;
  int dep, max;
  
  dep = 0;
  max = 3;
  en = NULL;
  sn = path;
  while(sn) {
  
    if (sn->is_sub_bnf && !sn->left && !sn->right) {
      en = sn;
      
      if (!strcmp(sn->key, "<SQL special character>")) max = 4;
      if (!strcmp(sn->key, "<unsigned numeric literal>")) max = 4;
      
      if (++dep == max) break;
    }
    
    sn = sn->next;
  }
  
  return en;
}


int 
is_like_keyword(char *s)
{ 
  char *e;
  
  if (!s) return 0;
  
  e = s + strlen(s) - 1;
  if (s == e) return (*s == 'C');
  
  while (s <= e) {
    if (!is_latin_ch(*s) && *s != '_' && *s != '-') return 0;
    s++;
  }
  
  return 1;
}


static token_list* 
create_token(char *key, char *val)
{
  token_list *nd;
  
  if (!key) return NULL;
  
  nd = malloc(sizeof(token_list));
  if (!nd) return NULL;
  
  memset(nd, 0, sizeof(token_list));
  nd->tk.key = key;
  nd->tk.value = malloc(strlen(val) + 1);
  if (!nd->tk.value) return NULL;
  
  strcpy(nd->tk.value, val);
  
  return nd;
}


void
free_tokens(token_list *tk_lst)
{
  token_list *lst, *tmp;
  
  lst = tk_lst->next;
  while (lst) {
    tmp = lst->next;
    free(lst->tk.value);
    free(lst);
    lst = tmp;
  }
}


int
check_lex(char *sql_str, token_list *tk_lst) 
{
  char *s, *e, *ss, *ee, ch;
  ENTRY item, *rti;
  s_tr_node *rtn;
  int rt, possible;
  token_list *lst;
  
  fs();
  
  if (!lex_tr_root) return 0;
  if (!kw_htab) return 0;
  if (!tk_lst) return 0;
  
  lst = tk_lst;
  lst->next = NULL;
  
  s = (char*)malloc(strlen(sql_str) + 1);
  if (!s) return 0;
  memcpy(s, sql_str, strlen(sql_str) + 1);
  
  e = s + strlen(s) - 1;
  
  debug("sql string: %s \n", s);
  
  while (get_tk_key(s, e, &ss, &ee)) {
    if (ee == e) break;
    
    s = ee + 1;
    ch = *(ee+1);
    *s = '\0';
    
    debug("[search token]..., key: %s \n", ss);
    
    possible = is_like_keyword(ss);
    if (possible) {
    
      /* find token in hash table directly if possible.
         */
      memset(&item, 0, sizeof(item));
      item.key = ss;
      rti = hsearch(kw_htab, item, FIND);
      if (rti) {
        debug("%s token was found in hash table.\n", ss);
        debug("token name: <key word> \n\n");
        
        lst = lst->next  = create_token(rti->key, ss);
        if (!lst) goto ERR;
        
        *s= ch;
        continue;
      }
    }
    
    /* find token in token tree.
       */
    rtn = search_graph(lex_tr_root, ss, ee);
    if (!rtn) goto ERR;
    
    debug("%s token was found in tree. \n", ss);
    
    rtn = get_token_node(lex_tr_root);
    if (!rtn) goto ERR; 
    debug("token name: %s \n\n", rtn->key);
    
    lst = lst->next  = create_token(rtn->key, ss);
    if (!lst) goto ERR;
    
    *s= ch;
  }
  
  debug("sql tokens found: %s \n\n", sql_str);
  
  if (s) {
    free(s);
    s = NULL;
  }
  
  fe();
  return 1;
  
  ERR:
  
  debug("%s is unknow token. \n", ss);
  
  if (s) {
    free(s);
    s = NULL;
  }
  
  free_tokens(tk_lst);
  
  return 0;
}


int
dbg_lex(void)
{
  char *sql_str;
  token_list tk_lst;
  int rt;
  
  fs();
  
  memset(&tk_lst, 0, sizeof(tk_lst));
  
  /* check <regular identifier> tokens.
   */
  sql_str = "i id id123 id_123 id_123_456_abc ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  

  /* check <key word> tokens
   */
  sql_str = "ABSOLUTE  C CREATE ON ZONE ADA NAME UNNAMED ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <unsigned numeric literal> tokens.
   */
  sql_str = "1234.56 0.789 123456789 1234. .56789 12E3 12.4E5 ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <nationnal character string literal> tokens.
   */
  sql_str = "N'hello' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <bit string literal> tokens.
   */
  sql_str = "B'11001010' B'' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <hex string literal> tokens.
   */
  sql_str = "X'0123456789ABCDEFabcdef' X'' X'FF' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <character string literal> tokens.
   */
  sql_str = "'hello' 'world2014' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <date string> tokens.
   */
  sql_str = "'1987-11-11' '1984-4-1' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <time string> tokens.
   */
  sql_str = "'20:49:30' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <timestamp string> tokens.
   *  TO BE DONE.
   */
   
  /* check <interval string> tokens.
   */
  sql_str = "'2014-01' '201401' ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <delimiter identifier> tokens.
   */
  sql_str = "\"\"\"\" \"delimiterID_123\" ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check <SQL special character> tokens.
   */
  sql_str = "\" % & ' ( ) * + , - . / : ; < = > ? _ | ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);
  
  
  /* check remainning special tokens.
   */
  sql_str = "<> >= <= || .. [ ] ";
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  free_tokens(&tk_lst);	
  
  fe();
  return 1;
}


static s_tr_node*
search_key(s_tr_node *root, char *key)
{
  s_tr_node *rtn;
  
  if (!root) return NULL;
  if (root->is_outside_loop_node) return NULL;
  if (strlen(root->key) == 1) return NULL;
  
  if (!root->left && !root->right && root->sub) {
  
    if (strcmp(root->key, key) == 0) {
      //debug("in token tree, key found: %s \n", key);
      return root;
    }
  }
  
  if (root->sub) {
  
    //debug("sub %s \n", root->sub->key);
    rtn = search_key(root->sub, key);
    if (rtn) return rtn;
  }
  
  if (root->left) {
  
    //debug("left %s \n", root->left->key);
    rtn = search_key(root->left, key);
    if (rtn) return rtn;
  } 
  
  if (root->right) {
  
    //debug("right %s \n", root->right->key);
    rtn = search_key(root->right, key);
    if (rtn) return rtn;
  }
  
  return NULL;
}


int
is_token(char *key)
{
  s_tr_node *root;
  ENTRY item, *rti;
  
  if (!lex_tr_root) return 0;
  if (!kw_htab) return 0;
  
  if (is_like_keyword(key)) {; 
    memset(&item, 0, sizeof(item));
    item.key = key;
    rti = hsearch(kw_htab, item, FIND);
    if (rti) return 1;
  }
  
  root = search_key(lex_tr_root, key);
  if (!root) return 0;
  
  return 1;
}


