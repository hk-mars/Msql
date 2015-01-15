
#include "common.h"
#include "tree.h"


static s_tr_node*  
get_father(s_tr_node *nd, char *key)
{
  s_tr_node *tmp;
  
  if (!nd) return 0;
  
  tmp = nd->father;
  if (tmp) tmp->next = nd;
  nd = tmp;
  while (nd) {
  
    if (strcmp(nd->key, key) == 0) {
    
      if (nd->sub) {
        if (nd->sub == nd->next) {
        
#ifdef TREE_DBG
          debug("loop node: %s \n", nd->key);
#endif
          return nd;
        }
      }
    }
    
    tmp = nd->father;
    if (tmp) tmp->next = nd;
    nd = tmp;
  }
  
  return NULL;
}


s_tr_node* 
insert_tree_left(s_tr_node *root, char *key)
{
  s_tr_node *nd;
  
#ifdef TREE_DBG
  
  fs();
  
  if (root) {
    if (strlen(root->key) < 100) {
      debug("father:\n%s\nleft:\n%s\n", root->key, key);
    }
    else {
      debug("father:\n%s\nleft:\n%s\n", "......", "......");
    }
  }
  else {
    debug("father:\n%s\nleft:%s\n", "NULL", key);
  }
#endif
  
  if (!key) return NULL;
  
  nd = (s_tr_node*)malloc(sizeof(s_tr_node));
  if (!nd) return NULL;
  memset(nd, 0, sizeof(s_tr_node));
  nd->father = root;
  nd->key = key;	
  nd->loop = get_father(root, key);
  nd->is_inside_loop_node = !!nd->loop;
  
  if (root) root->left = nd;
  
  return nd;
}


s_tr_node* 
insert_tree_right(s_tr_node *root, char *key)
{
  s_tr_node *nd;
  
#ifdef TREE_DBG
  
  fs();
  
  if (root) {
    if (strlen(root->key) < 100) {
      debug("father:\n%s\nright:\n%s\n", root->key, key);
    }
    else {
      debug("father:\n%s\nright:\n%s\n", "......", "......");
    }
  }
  else {
    debug("father:\n%s\nright:%s\n", "NULL", key);
  }
#endif
  
  if (!key) return NULL;
  
  nd = (s_tr_node*)malloc(sizeof(s_tr_node));
  if (!nd) return NULL;
  memset(nd, 0, sizeof(s_tr_node));
  nd->father = root;
  nd->key = key;	
  nd->loop = get_father(root, key);
  nd->is_inside_loop_node = !!nd->loop;
  
  if (root) root->right = nd;
  
  return nd;
}


s_tr_node* 
insert_tree_sub(s_tr_node *root, char *key)
{
  s_tr_node *nd;
  
#ifdef TREE_DBG
  
  fs();
  
  if (root) {
    if (strlen(root->key) < 100) {
      debug("father:\n%s\nsub:\n%s\n", root->key, key);
    }
    else {
      debug("father:\n%s\nsub:\n%s\n", "......", "......");
    }
  }
  else {
    debug("father:\n%s\nsub:%s\n", "NULL", key);
  }
#endif
  
  nd = (s_tr_node*)malloc(sizeof(s_tr_node));
  if (!nd) return NULL;
  memset(nd, 0, sizeof(s_tr_node));
  nd->father = root;
  nd->key = key;	
  nd->loop = get_father(root, key);
  nd->is_inside_loop_node = !!nd->loop;
  
  if (root) root->sub = nd;
  
  return nd;
}


