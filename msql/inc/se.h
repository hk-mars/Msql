

#ifndef __SE_H__
#define __SE_H__

#include "bin_tree.h"

typedef
struct column_obj
{
  char *name;
  char *type;
  long len;
  
  int int_val;
  char *str_val;

  s_bin_tree_node *val_tree;
  int is_new;

  char *table_name;
}
s_column_obj;

typedef
struct column_obj_list
{
  s_column_obj *obj;
  struct column_obj_list *next;
}
s_column_obj_list;

typedef
struct table_property
{
  char *name;
  s_column_obj_list *cl_obj_lst;
  s_bin_tree_node *cl_tree;
}
s_table_property;

typedef
struct value_term
{
  s_column_obj obj;
  s_column_obj obj1;
  
  char *op;
  char *r;
  char *r1;
}
s_value_term;

typedef
struct query_info_list
{
  s_table_property *p;
  s_bin_tree_node *tr;

  struct query_info_list *next;
}
s_query_info_list;


int init_se(void);
int is_table_exist(char *table_name);
s_bin_tree_node* get_table_node(char *table_name);
s_table_property* get_table_property(char *table_name);
s_bin_tree_node* get_column_node(s_bin_tree_node **t, char *column_name);
int delete_table_node(char *table_name);
s_bin_tree_node* create_table_node(char *table_name);
int is_column_exist(char *table_name, char *column_name);
int  create_column_node(char *table_name, char *column_info);
int delete_column_node(char *table_name, char *column_name);
void show_column_list(s_column_obj_list *lst);

int
insert_column_value
(
  s_table_property *p, s_column_obj *obj, char *one_row
);

int search_value(char *table_name, s_value_term *t);
int search_value_1(s_query_info_list *lst, s_value_term *t);
int copy_file(char *src, char *dst);

#endif /* __SE_H__ */


