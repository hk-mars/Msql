
#ifndef __TREE_H__
#define __TREE_H__

typedef 
struct tr_node 
{
  struct tr_node *father;
  struct tr_node *left;
  struct tr_node *right;
  struct tr_node *sub;
  struct tr_node *next;
  struct tr_node *back;
  struct tr_node *loop;
  
  char *key;
  unsigned char is_inside_loop_node;
  unsigned char is_outside_loop_node;
  unsigned char is_sub_bnf;
  unsigned char is_token;
  unsigned char is_in_syntax_tree;
}
s_tr_node;

s_tr_node* insert_tree_left(s_tr_node *root, char *key);
s_tr_node* insert_tree_right(s_tr_node *root, char *key);
s_tr_node* insert_tree_sub(s_tr_node *root, char *key);

#endif /* __TREE_H__ */

