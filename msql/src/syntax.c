
#include "common.h"
#include "syntax.h"
#include "char.h"


static HTAB_INFO syntax_htab;


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


int
create_syntax_htab(int cnt)
{
  int rt;
  
  if (syntax_htab.table) return 0;
  
  rt = hcreate(&syntax_htab, cnt);
  
  return rt;
}


HTAB_INFO* 
get_syntax_htab(void)
{
  if (!syntax_htab.table) return NULL;
  
  return &syntax_htab;
}


int 
push_syntax_htab(char *key, s_tr_node *root)
{
  ENTRY item, *rti;
  
  if (!syntax_htab.table) return 0;
  
  item.key = key;
  item.data = root;
  rti = hsearch(&syntax_htab, item, ENTER); 
  
  return !!rti;
}


ENTRY*
pop_syntax_htab(char *key)
{
  ENTRY item, *rti;
  
  if (!syntax_htab.table) return NULL;
  
  memset(&item, 0, sizeof(ENTRY));
  item.key = key;
  rti = hsearch(&syntax_htab, item, FIND);
  
  return rti;
}


static s_tr_node*
search_end(s_tr_node *root)
{
  s_tr_node *rtn;
  
  if (!root) return NULL;
  
  if (strcmp(root->key, "@") == 0) {
  
#if 0
    debug("%s \n", root->key);
    if (root->right) debug("still has a right: %s. \n", root->right->key);
    if (root->left) debug("still has a left: %s. \n", root->left->key);
    if (root->left && root->left->left) debug("still has a left left: %s. \n", 
    root->left->left->key);
    if (root->sub) debug("still has a sub: %s. \n", root->sub->key);
#endif
    
    if (!root->right && !root->left && !root->sub) {
      //debug("@e. \n");
      return root;
    }
  }
  
  if (root->is_token) {
    //debug("token: %s \n", root->key);
    return NULL;
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
search_syntax_tree(s_tr_node *root, token_list *tk_lst)
{
  s_tr_node *rtn, *nd;
  ENTRY *rti;
  token_list *lst, *sl, *el;
  
  nd = NULL;
  
  if (!root) return NULL;
  
  if (root->is_token) {
    //debug("token node: %s \n", root->key);
    
    root->next = NULL;
    if (strcmp(root->key, tk_lst->tk.key) != 0) return NULL;
    
    //debug("found token: %s \n", root->key);
    
    /* the end token, check the path if it's at the end.
       */
    if (!tk_lst->next) {
    
      if (!root->sub && !root->left && !root->right) return root;
      
      rtn = search_end(root->sub);
      if (rtn) return root;
      
      rtn = search_end(root->left);
      if (rtn) return root;
      
      rtn = search_end(root->right);
      if (rtn) return root;
      
      //debug("no end path. \n");
      return NULL;
    }
    else {
      tk_lst = tk_lst->next;
      //debug("try find: %s \n", tk_lst->tk.key);
    }
  }
  
  
  if (root->loop) {
    //debug("loop node: %s \n", root->key);
    nd = root->loop;
  }
  
  
  if (root->is_in_syntax_tree && !nd) {
  
    /* get the root of sub tree from hash table.
       */
    rti = pop_syntax_htab(root->key);
    if (!rti) return NULL;
    if (!rti->data) {
      //debug("sub tree not found: %s \n", root->key);
      return NULL;
    }
    
    nd = rti->data;
    //debug("syntax sub tree found: %s \n", root->key);
  }
  
  if (nd) {
  
    /* make new token list from the trail of old list, 
      * and search the sub solution path again and again until done.
      */
    sl = tk_lst;
    el = NULL;
    while (1) {
    
      rtn = search_syntax_tree(nd, sl);
      
      /* reconnect the list
          */
      lst = sl;
      while (lst) {
        if (!lst->next) break;
        lst = lst->next;
      }
      if (el) lst->next = el;
      
      if (rtn) {
        //debug("en token: %s \n", lst->tk.key);
        //if (el) debug("sn token: %s \n\n", el->tk.key);
        if (!el) return rtn;
        tk_lst = el;
        
        break;
      }
      
      el = lst;
      if (el == sl) return NULL;
      
      /* cut the list from the trail.
          */
      lst = sl;
      while (lst) {
        if (lst->next == el) break;
        lst = lst->next;
      }
      lst->next = NULL;
    }	
  }
  
  
  if (root->back) {
    //debug("go back to node: %s \n", root->back->key);
    rtn = search_syntax_tree(root->back, tk_lst);
    if (rtn) return rtn;
  }
  
  if (root->sub) {
    //debug("sub: %s \n", root->sub->key);
    root->next = root->sub;
    rtn = search_syntax_tree(root->sub, tk_lst);
    if (rtn) return rtn;
  }
  
  if (root->left) {
    //debug("left: %s \n", root->left->key);
    root->next = root->left;
    rtn = search_syntax_tree(root->left, tk_lst);
    if (rtn) return rtn;
  }
  
  if (root->right) {
    //debug("right: %s \n", root->right->key);
    root->next = root->right;
    rtn = search_syntax_tree(root->right, tk_lst);
    if (rtn) return rtn;
  }
  
  return NULL;
}


static void
show_tokens(token_list *tk_lst)
{
  token_list *lst;
  
  lst = tk_lst->next;
  while (lst) {
    debug("%s ", lst->tk.key);
    lst = lst->next;
  }
  debug("\n\n");
}


int 
check_syntax(s_tr_node *root, token_list *tk_lst)
{
  int rt;
  s_tr_node *rtn;
  
  
  fs();
  
  if (!tk_lst || !tk_lst->next) return 0;
  
  debug("try find solution path in syntax tree..., for: \n");
  show_tokens(tk_lst);
  
  rtn = search_syntax_tree(root, tk_lst->next);
  if (!rtn) goto END;
  
  debug("\nsolution path FOUND for: \n");
  show_tokens(tk_lst);
  //show_path(root);
  
  fe();
  return 1;
  
END:
  debug("\nsolution path NOT found for: \n");
  show_tokens(tk_lst);
  
  return 0;
}


int 
dbg_syntax(void)
{
  int rt;
  char *sql_str;
  token_list tk_lst;
  ENTRY *rti;
  s_tr_node *root;
  
  fs();
  
  
  rti = pop_syntax_htab("<table definition>");
  if (!rti) return 0;
  root = rti->data;
  if (!root) return 0;
  
  debug("check sql syntax..., for: %s \n", rti->key);
  
  sql_str = "CREATE TABLE person ( name CHAR ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0; 	
  free_tokens(&tk_lst);
  
  sql_str = "CREATE TABLE person ( name CHAR , age INT , born DATE ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0; 	
  free_tokens(&tk_lst);
  
  sql_str = "CREATE TABLE person ( name CHAR ( 10 ) , sex CHAR ( 1 ) ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0; 	
  free_tokens(&tk_lst);
  
  sql_str = "CREATE TABLE person ( name CHAR ) "
    "ON COMMIT DELETE ROWS ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0; 	
  free_tokens(&tk_lst);
  
  
  rti = pop_syntax_htab("<drop table statement>");
  if (!rti) return 0;
  root = rti->data;
  if (!root) return 0;
  
  debug("check sql syntax..., for: %s \n", rti->key);
  
  sql_str = "DROP TABLE person RESTRICT ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "DROP TABLE person CASCADE ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  debug("check sql syntax for %s, done. \n", rti->key);
  
  
  rti = pop_syntax_htab("<alter table statement>");
  if (!rti) return 0;
  root = rti->data;
  if (!root) return 0;
  
  debug("check sql syntax..., for: %s \n", rti->key);
  
  sql_str = "ALTER TABLE person ADD COLUMN sex CHAR ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "ALTER TABLE person ALTER COLUMN sex DROP DEFAULT ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  debug("check sql syntax for %s, done. \n", rti->key);
  
  
  rti = pop_syntax_htab("<SQL data change statement>");
  if (!rti) return 0;
  root = rti->data;
  if (!root) return 0;
  
  debug("check sql syntax..., for: %s \n", rti->key);
  
  sql_str = "INSERT INTO person VALUES ( 'mars' , 30 ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "DELETE FROM person WHERE name = 'mars' ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "UPDATE person SET name = 'fumin' WHERE name = 'mars' ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);	
  
  debug("check sql syntax for %s, done. \n", rti->key);
  
  
  rti = pop_syntax_htab("<query expression>");
  if (!rti) return 0;
  root = rti->data;
  if (!root) return 0;	
  
  debug("check sql syntax..., for: %s \n", rti->key);
  
  sql_str = "SELECT * FROM person ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "SELECT * FROM person WHERE age = 30 ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  
  sql_str = "SELECT sex  FROM person "
    "WHERE name = 'mars' OR age = 30 ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst); 
  
  
  sql_str = "SELECT sex  FROM person "
    "WHERE name = 'mars' AND age = 30 ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  sql_str = "SELECT sex  FROM person "
    "WHERE ( name = 'mars' AND age = 30 ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst);
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst); 	
  
  sql_str = "SELECT sex  FROM person "
    "WHERE ( name = 'mars' OR age = 30 ) ";
  memset(&tk_lst, 0, sizeof(tk_lst));
  rt = check_lex(sql_str, &tk_lst);
  if (!rt) return 0;
  show_tokens(&tk_lst); 
  rt = check_syntax(root, &tk_lst);
  if (!rt) return 0;	
  free_tokens(&tk_lst);
  
  debug("check sql syntax for %s, done. \n", rti->key);	
  
  fe();
  return 1;
}


