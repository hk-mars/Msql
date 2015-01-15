
#include "executor.h"
#include "common.h"
#include "stack.h"
#include "se.h"
#include "bin_tree.h"
#include "gc.h"


static char*
eval_bool_value
(
  char  *r0, char  *r1, char *op, char *table_name
);

static s_value_term* search_condition(s_search_condition *c);


static int
add_table_name(char *name)
{
  FILE *f;
  int rt;
  
  fs();

  f = fopen("debug/tables/tables.txt", "a");
  if (!f) return 0;

  rt = fwrite(name, strlen(name), 1, f);
  if (!rt)  {
    debug("write [%s] failed. \n", name);
    return 0;
  }

  rt = fputc('\n', f);
  if (!rt) return 0;

  fclose(f);
  fe_return(1);
}

int
create_table(void *stack)
{
  s_table_definition *s = stack;
  char *name;
  char fname[255];
  FILE *f;
  s_table_element_list *elem;
  int rt;
  
  fs();

  name = s->tb_name.qlf_name.qlf_identifier;
  if (strlen(name) + strlen("debug/tables/.txt") + 1 > sizeof(fname)) {
    debug("err: table name too long, out of size %d bytes. \n", sizeof(fname));
    return 0;
  }

  if (is_table_exist(name)) {
    debug("table %s existed. \n", name);
    return 0;
  }

  /*add @table_name into tables.txt
   */
  rt = add_table_name(name);
  if (!rt) return 0;
  
  /*create property file.
   */
  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), name), ".txt");
  f = fopen(fname, "w+");
  if (!f) return 0;
  
  elem = &s->elm_lst;
  while (elem) {
  
    fprintf(f, "%s,", elem->elm.cl_def.cl_name);

    if (elem->elm.cl_def.dt_type.is_character_string_type) {
    
      elem->elm.cl_def.dt_type.ch_str_type.is_character_type ? 
        fprintf(f, "character") : 0;
      
      elem->elm.cl_def.dt_type.ch_str_type.is_character_varying_type ?
        fprintf(f, "character varying") : 0;
      
      elem->elm.cl_def.dt_type.ch_str_type.is_char_type ?
        fprintf(f, "char") : 0;
      
      elem->elm.cl_def.dt_type.ch_str_type.is_char_varying_type ?
        fprintf(f, "char") : 0;
      
      elem->elm.cl_def.dt_type.ch_str_type.is_varchar_type ?
        fprintf(f, "varchar") : 0;
      
      elem->elm.cl_def.dt_type.ch_str_type.has_length ?
        fprintf(f, ",%d", elem->elm.cl_def.dt_type.ch_str_type.length) : 0;
    }
    
    if (elem->elm.cl_def.dt_type.is_numeric_type) {

      (elem->elm.cl_def.dt_type.num_type.is_exact_numeric_type &&
        elem->elm.cl_def.dt_type.num_type.exact_num_type.is_dec_type) ?
          fprintf(f, "dec") : 0;
      
      (elem->elm.cl_def.dt_type.num_type.is_exact_numeric_type &&
        elem->elm.cl_def.dt_type.num_type.exact_num_type.is_int_type) ?
          fprintf(f, "int") : 0;
       
      (elem->elm.cl_def.dt_type.num_type.is_appro_numeric_type &&
        elem->elm.cl_def.dt_type.num_type.appr_num_type.is_float_type) ?
          fprintf(f, "float") : 0;
      
      (elem->elm.cl_def.dt_type.num_type.is_appro_numeric_type &&
        elem->elm.cl_def.dt_type.num_type.appr_num_type.is_real_type) ?
          fprintf(f, "real") : 0;
      
      (elem->elm.cl_def.dt_type.num_type.is_appro_numeric_type &&
        elem->elm.cl_def.dt_type.num_type.appr_num_type.is_double_type) ?
          fprintf(f, "double") : 0;
    }
    
    if (elem->elm.cl_def.dt_type.is_datetime_type) {
    
      elem->elm.cl_def.dt_type.date_type.is_date_type ?
        fprintf(f, "datetime") : 0;
      
      elem->elm.cl_def.dt_type.date_type.is_time_type ?
        fprintf(f, "time") : 0;
      
      elem->elm.cl_def.dt_type.date_type.is_timestamp_type ?
        fprintf(f, "timestamp") : 0;
    }

    fprintf(f, "\n");
    elem = elem->next;
  }

  fclose(f);
  debug("create table property file [%s], done. \n", fname);

  /*add new table to on table tree.
   */
  if (!create_table_node(name)) return 0;

  fe_return(1);
}

static int
delete_table_name(char *tb_name)
{
  FILE *f;
  char name[255];
  int len;
  int rt;
  
  fs();

  f = fopen("debug/tables/tables.txt", "r+");
  if (!f) return 0;

  while (!feof(f)) {
    memset(name, 0, sizeof(name));
    fgets(name, sizeof(name), f);

    name[strlen(name)-1] == '\n' ?
      name[strlen(name)-1] = '\0' : 0;

    debug("%s \n", name);
    if (!strcmpi(name, tb_name)) {

      len = strlen(name);

      debug("position: %d \n", ftell(f));
      fseek(f, ftell(f) - len - 2, SEEK_SET);
      debug("position back to: %d \n", ftell(f));
      
      memset(name, '-', len);
      rt = fwrite(name, 1, len, f);
      if (rt != len) {
        debug("write file failed. %d \n", rt);
        fclose(f);
        return 0;
      }
      
      fclose(f);
      fe_return(1);
    }
  }

  debug("table name [%s] was not found in tables.txt. \n", tb_name);
  fclose(f);
  return 0;
}

static int
delete_property_file(char *tb_name)
{
  char fname[255];
  int rt;
  
  fs();

  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), tb_name), ".txt");
  debug("unlink [%s] \n", fname);
  rt = unlink(fname);
  if (rt) return 0;

  fe_return(1);
}

int
drop_table(void *stack)
{
  s_drop_table_statement *s = stack;
  char *name;
  int rt;
  s_bin_tree_node *nd;
  s_table_property *p;
  
  fs();

  name = s->tb_name.qlf_name.qlf_identifier;
  if (!is_table_exist(name)) {
    debug("table [%s] does not exist. \n", name);
    return 0;
  }
  debug("table [%s] was found on table tree. \n", name);
  

  /*delete table node from table tree, cocurrenly we would do:
    *-delete all the values info of each column.
    *-delete all the column info of this table.
    *-delete the table info.
    *where @info means the record file and the realtime tree.
    */
  rt = delete_table_node(name);
  if (!rt) return 0;
  
  /*delete @table_name from tables.txt.
   */
  rt = delete_table_name(name);
  if (!rt) return 0;

  /*delete @table_name[.txt] of table columns info.
   */
  rt = delete_property_file(name);
  if (!rt) return 0;
  
  fe_return(1);
}

static int
add_column(char *table_name, s_column_definition *df)
{
  char *column_name;
  char fname[255];
  FILE *f; 
  char column_info[1024];
  char *s;
  int rt;
  
  fs();

  column_name = df->cl_name;
  if (is_column_exist(table_name, column_name)) {
    debug("column [%s] existed in table [%s]. \n", column_name, table_name);
    return 0;
  }

  /*add column info into table property file.
   */
  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), table_name), ".txt");
  f = fopen(fname, "a");
  if (!f) return 0;

  memset(column_info, 0, sizeof(column_info));
  s = column_info;

  s += sprintf(s, "%s,", column_name);
  if (df->dt_type.is_character_string_type) {
  
    df->dt_type.ch_str_type.is_character_type ? 
      s += sprintf(s, "character") : 0;
    
    df->dt_type.ch_str_type.is_character_varying_type ?
      s += sprintf(s, "character varying") : 0;
    
    df->dt_type.ch_str_type.is_char_type ?
      s += sprintf(s, "char") : 0;
    
    df->dt_type.ch_str_type.is_char_varying_type ?
      s += sprintf(s, "char") : 0;
    
    df->dt_type.ch_str_type.is_varchar_type ?
      s += sprintf(s, "varchar") : 0;
    
    df->dt_type.ch_str_type.has_length ?
      s += sprintf(s, ",%d", df->dt_type.ch_str_type.length) : 0;
  }
  
  if (df->dt_type.is_numeric_type) {
  
    (df->dt_type.num_type.is_exact_numeric_type &&
      df->dt_type.num_type.exact_num_type.is_dec_type) ?
        s += sprintf(s, "dec") : 0;
    
    (df->dt_type.num_type.is_exact_numeric_type &&
      df->dt_type.num_type.exact_num_type.is_int_type) ?
        s += sprintf(s, "int") : 0;
     
    (df->dt_type.num_type.is_appro_numeric_type &&
      df->dt_type.num_type.appr_num_type.is_float_type) ?
        s += sprintf(s, "float") : 0;
    
    (df->dt_type.num_type.is_appro_numeric_type &&
      df->dt_type.num_type.appr_num_type.is_real_type) ?
        s += sprintf(s, "real") : 0;
    
    (df->dt_type.num_type.is_appro_numeric_type &&
      df->dt_type.num_type.appr_num_type.is_double_type) ?
        s += sprintf(s, "double") : 0;
  }
  
  if (df->dt_type.is_datetime_type) {
  
    df->dt_type.date_type.is_date_type ?
      s += sprintf(s, "datetime") : 0;
    
    df->dt_type.date_type.is_time_type ?
      s += sprintf(s, "time") : 0;
    
    df->dt_type.date_type.is_timestamp_type ?
      s += sprintf(s, "timestamp") : 0;
  }
  
  rt = fwrite(column_info, strlen(column_info), 1, f);
  if (!rt) {
    debug("write [%s] failed. \n", column_info);
    fclose(f);
    return 0;
  }

  rt = fputc('\n', f);
  if (!rt) return 0;  
  fclose(f);

  /*create a new column node on column tree.
   */
  rt = create_column_node(table_name, column_info);
  if (!rt) return 0;

  fe_return(1);
}

static int
delete_column_info(char *table_name, char *column_name)
{
  FILE *f;
  char fname[255];
  char name[255];
  int len;
  int rt;
  
  fs();

  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), table_name), ".txt");
  
  f = fopen(fname, "r+");
  if (!f) return 0;
  
  while (!feof(f)) {
    memset(name, 0, sizeof(name));
    fgets(name, sizeof(name), f);
    
    name[strlen(name)-1] == '\n' ?
      name[strlen(name)-1] = '\0' : 0;
    
    debug("%s \n", column_name);
    if (!memcmp(name, column_name, strlen(column_name))) {
    
      len = strlen(name);
      
      debug("position: %d \n", ftell(f));
      fseek(f, ftell(f) - len - 2, SEEK_SET);
      debug("position back to: %d \n", ftell(f));
      
      memset(name, '-', len);
      rt = fwrite(name, 1, len, f);
      if (rt != len) {
      debug("write file failed. %d \n", rt);
        fclose(f);
        return 0;
      }
      
      fclose(f);
      fe_return(1);
    }
  }
  
  debug("column [%s] was not found in [%s]. \n", column_name, fname);
  fclose(f);
  return 0;

}

static int
drop_column(char *table_name, s_drop_column_definition *df)
{
  char *column_name;
  int rt;
  
  fs();

  column_name = df->cl_name;
  rt = delete_column_node(table_name, column_name);
  if (!rt) return 0;

  /*delete column info in table property file.
   */
  rt = delete_column_info(table_name, column_name);
  if (!rt) return 0;
  
  fe_return(1);
}

int
alter_table(void *stack)
{
  s_alter_table_statement *s = stack;
  char *table_name;
  int rt;
  
  fs();

  table_name = s->tb_name.qlf_name.qlf_identifier;
  if (!is_table_exist(table_name)) {
    debug("table [%s] does not exist. \n", table_name);
    return 0;
  }
  debug("table [%s] was found on table tree. \n", table_name);

  if (s->action.is_add_column_definition) {
    rt = add_column(table_name, &s->action.add_cl_def);
    if (!rt) return 0;
  }

  if (s->action.is_drop_column_definition) {
    rt = drop_column(table_name, &s->action.drop_cl_def);
    if (!rt) return 0;
  }
  
  fe_return(1);
}

static s_factor*
eval_factor(s_factor *f0, s_factor *f1, char op)
{
  s_factor *f2;
  s_value_expression_primary *p0, *p1, *p2;
  s_exact_numeric_literal *n0, *n1, *n2;
  
  fs();

  debug("%c \n", op);
  
  p0 = &f0->num_prim.val_expr_prim;
  n0 = &p0->uns_val_spec.uns_literal.us_num_lt.exact_num_lt;

  p1 = &f1->num_prim.val_expr_prim;
  n1 = &p1->uns_val_spec.uns_literal.us_num_lt.exact_num_lt;

  f2 = gc_malloc(sizeof(s_factor));
  if (!f2) return NULL;

  p2 = &f2->num_prim.val_expr_prim;
  n2 = &p2->uns_val_spec.uns_literal.us_num_lt.exact_num_lt;
  
  switch(op) {
  case '*':
    n2->dval = n0->dval * n1->dval;
    break;
  case '/':
    if (n1->dval < 1e-7) return NULL;
    n2->dval = n0->dval / n1->dval;
    break;
  case '+':
    n2->dval = n0->dval + n1->dval;
    break;
  case '-':
    n2->dval = n0->dval - n1->dval;
    break;    
  default:
    break;
  }

  debug("%f %c %f = %f \n", n0->dval, op, n1->dval, n2->dval);
  fe_return(f2);
}


static s_factor*
eval_term1(s_factor *f, s_term1 *t)
{
  s_factor *f1, *f2;
  
  fs();

  if (t->next) {
    f1 = eval_term1(&t->ft, t->next);
    if (!f1) return NULL;
  }
  else {
    f1 = &t->ft;
  }

  f2 = NULL;
  if (t->is_asterisk) f2 = eval_factor(f, f1, '*'); 
  if (t->is_solidus) f2 = eval_factor(f, f1, '/');
  if (!f2) f2 = f;

  fe_return(f2);
}

static s_factor*
eval_expr(s_term *t0, s_term *t1, char op)
{
  s_factor *f1, *f2, *f3;
  s_term *t3;

  fs();

  debug("%c \n", op);

  f1 = eval_term1(&t0->ft, &t0->tm1);
  if (!f1) return NULL;

  f2 = eval_term1(&t1->ft, &t1->tm1);
  if (!f2) return NULL;
  
  f3 = eval_factor(f1, f2, op);

  fe_return(f3);
}

static s_factor*
eval_term_factor(s_term *t, s_factor *f, char op)
{
  s_factor *f1, *f2;
  
  fs();

  debug("%c \n", op);
  
  f1 = eval_term1(&t->ft, &t->tm1);
  if (!f1) return NULL;

  f2 = eval_factor(f1, f, op);
 
  fe_return(f2);
}

static s_factor*
eval_numeric_expr1(s_term *t, s_numeric_value_expression1 *e, char op)
{
  s_term *t1;
  s_factor *f;
  
  fs();

  if (e->next) {
    f = eval_numeric_expr1(&e->tm, e->next, e->is_minus_sign ? '-' : '+');
    if (!f) return NULL;
    
    f = eval_term_factor(t, f, op);
    fe_return(f);
  }
  else {
    t1 = &e->tm;
  }

  f = NULL;
  
  if (e->is_minus_sign) {
    f= eval_expr(t, t1, '-');
    fe_return(f);
  }
  
  if (e->is_plus_sign) {
    f = eval_expr(t, t1, '+');
    fe_return(f);
  }
  
  if (!f) f= &t->ft;

  fe_return(f);
}


static s_factor*
eval_numeric_expr(s_numeric_value_expression *e)
{
  s_factor *f;
  char op;
  
  fs();

  op = (e->num_val_expr1.is_minus_sign ? '-' : '+');
  f = eval_numeric_expr1(&e->tm, &e->num_val_expr1, op);
  
  fe_return(f);
}

static double
eval_numeric_column(s_numeric_value_expression *e)
{
  s_factor *f;
  double val;
  s_unsigned_numeric_literal *num;
  
  fs();
  
  f = eval_numeric_expr(e);
  if (!f) return 0;

  num = &f->num_prim.val_expr_prim.uns_val_spec.uns_literal.us_num_lt;
  val = num->exact_num_lt.dval;
  
  fe_return(val);
}

static char*
eval_string_column(s_string_value_expression *e)
{
  char *s;
  
  fs();

  s = e->ch_val_expr.ch_factor.ch_prim.str_val_expr_prim.char_str_ltr.str;

  fe_return(s);
}

static int
update_table_value_file(char *table_name, char *one_row)
{
  char *fname;
  FILE *f;
  
  fs();
  
  fname = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".txt") + 1);
  
  if (!fname) return 0;
  
  sprintf(fname, "debug/tables/values/%s.txt", table_name);
  
  debug("write one row [%s] into [%s]. \n", one_row, fname);
  
  f = fopen(fname, "a");
  if (!f) return 0;
  
  fprintf(f, "%s\n", one_row);
  
  fclose(f);
  fe_return(1);

}

static int
insert_values(char *table_name, s_table_value_constructor_list *lst)
{
  int rt;
  s_row_value_constructor_list *lst2;
  s_value_expression *e;
  double d;
  char *s;
  s_column_obj obj;
  s_bin_tree_node *nd;
  s_table_property *p;
  s_column_obj_list *cl_lst;
  char one_row[255];
  char *ss;
  
  fs();

  nd = get_table_node(table_name);
  if (!nd) return 0;

  p = nd->val;
  cl_lst = p->cl_obj_lst;

  show_column_list(cl_lst);

  memset(one_row, 0, sizeof(one_row));
  ss = one_row;
  
  while (lst) {
    
    if (lst->row_val_cstr.is_row_value_constructor_list) {
    
      debug("row value element list with (). \n");
      lst2 = &lst->row_val_cstr.row_val_list;
      while (lst2) {

        memset(&obj, 0, sizeof(s_column_obj));
        
        e = &lst2->row_val_elm.val_expr;
        
        if (e->is_numeric_value_expression) {
          d = eval_numeric_column(&e->num_val_expr);
          debug("numeric value: [%f]. \n", d);

          obj.type = "int";
          cl_lst->obj->int_val = (int)d;
          ss += sprintf(ss, "%d,", cl_lst->obj->int_val);
        }
        
        if (e->is_string_value_expression) {
          s = eval_string_column(&e->str_val_expr);
          if (!s) return 0;
          debug("string value: [%s]. \n", s);
          
          obj.type = "char";
          cl_lst->obj->str_val = s;
          ss += sprintf(ss, "%s,", cl_lst->obj->str_val);
        }

        debug("type compare: %s \n", cl_lst->obj->type);
        if (strcmpi(cl_lst->obj->type, obj.type)) {
          debug("unkown data type. \n");
          return 0;
        }

        cl_lst = cl_lst->next;
        lst2 = lst2->next;
      }
    }
    
    lst = lst->next;
  }

  *(ss - 1) = '\0';
  debug("one row val: [%s]. \n", one_row);
  
  cl_lst = p->cl_obj_lst;
  while (cl_lst) {
    rt = insert_column_value(p, cl_lst->obj, one_row);
    if (!rt) return 0;

    cl_lst = cl_lst->next;
  }

  rt = update_table_value_file(table_name, one_row);
  if (!rt) return 0;
  
  fe_return(1);
}


int
insert_table(void *stack)
{
  s_insert_statement *s = stack;
  char *table_name;
  int rt;
  s_non_join_query_primary *prim;
  
  fs();

  table_name = s->tb_name.qlf_name.qlf_identifier;
  if (!is_table_exist(table_name)) {
    debug("table [%s] does not exist. \n", table_name);
    return 0;
  }
  debug("table [%s] was found on table tree. \n", table_name);  

  prim = &s->insert_cl_src.qexpression.njoin_qexpr.njoin_qterm.njoin_qprimary;
  rt = insert_values(table_name, &prim->sptable.tb_val_cstr.val_lst);
  if (!rt) return 0;

  fe_return(1);
}

static char*
search_table(char *table_name)
{
  int rt;
  char *fpath;
  FILE *f;
  
  fs();

  debug("table name: [%s].\n", table_name);

  if (!is_table_exist(table_name)) return NULL;

  fpath = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".txt") + 1);
  if (!fpath) return NULL;

  sprintf(fpath, "debug/tables/values/%s.txt", table_name);

  f = fopen(fpath, "r");
  if (!f) {
    debug("[%s] is not found. \n", fpath);
    return 0;
  }

  debug("[%s] is found. \n", fpath);
  fclose(f);
  fe_return(fpath);
}

static char*
get_operator(s_comp_op *cmp_op, int has_not)
{
  char *op;
  
  fs();

  cmp_op->is_equal_op ? 
    debug("is_equal_op.\n") : 0;
  cmp_op->is_greater_than_op ? 
    debug("is_greater_than_op.\n") : 0;
  cmp_op->is_greater_than_or_equal_op ? 
    debug("is_greater_than_or_equal_op.\n") : 0;
  cmp_op->is_less_than_op ? 
    debug("is_less_than_op.\n") : 0;
  cmp_op->is_less_than_or_equal_op ? 
    debug("is_less_than_or_equal_op.\n") : 0;
  cmp_op->is_not_equal_op ? 
    debug("is_not_equal_op.\n") : 0;

  op = NULL;

  if (has_not) {
    debug("has not.\n");
    
    if (cmp_op->is_equal_op) op = "NE";
    if (cmp_op->is_greater_than_op) op = "LE";
    if (cmp_op->is_greater_than_or_equal_op) op = "L";
    if (cmp_op->is_less_than_op) op = "GE";
    if (cmp_op->is_less_than_or_equal_op) op = "G";
    if (cmp_op->is_not_equal_op) op = "E";    
  }
  else {
    if (cmp_op->is_equal_op) op = "E";
    if (cmp_op->is_greater_than_op) op = "G";
    if (cmp_op->is_greater_than_or_equal_op) op = "GE";
    if (cmp_op->is_less_than_op) op = "L";
    if (cmp_op->is_less_than_or_equal_op) op = "LE";
    if (cmp_op->is_not_equal_op) op = "NE";  
  }

  fe_return(op);
}


static s_query_info_list qlst;

static void
show_rtree(s_bin_tree_node *root)
{
  if (!root) return;

  debug("@row: [%s].\n", root->key);

  show_rtree(root->left);
  show_rtree(root->right);
}


static char*
sync_rtree_to_file(s_bin_tree_node *root, FILE *f)
{
  if (!root) return;
  
  debug("sync to file with @row [%s].\n", root->key);
  
  fprintf(f, "%s\n", root->key);
  
  sync_rtree_to_file(root->left, f);
  sync_rtree_to_file(root->right, f);
}


static s_bin_tree_node**
get_result_tree(char *table_name)
{
  s_query_info_list *q;
  
  fs();

  debug("table name [%s].\n", table_name);

  q = qlst.next;

  while (q) {
    if (!strcmp(q->p->name, table_name)) fe_return(&q->tr);

    q = q->next;
  }

  return NULL;
}

static char*
make_result_file_name(char *table_name)
{
  char *r;
  
  fs();

  r = gc_malloc(1024);
  if (!r) return NULL;

  sprintf(r, "%s.r.txt", table_name);
  
  fe_return(r);
}

static int
sync_results(s_query_info_list *q, s_value_term *r0)
{
  char *r;
  FILE *f;

  fs();

  if (!q) {
    debug("s_query_info_list is null.\n");
    
    return 0;
  }
  
  r = gc_malloc(1024);
  if (!r) return 0;
  
  while (q) {

    q->tr ? 0 : debug("the tree of table [%s] is null.\n", q->p->name);
    
    sprintf(r, "%s.r.txt", q->p->name);

    if (!q->tr) {
      if (r0->r) {
        if (!strcmpi(r0->obj.table_name, q->p->name)) {
          eval_bool_value(r0->r, "r.txt", "or", q->p->name);
        }
      }

      if (r0->r1) {
        if (!strcmpi(r0->obj1.table_name, q->p->name)) {
          eval_bool_value(r0->r1, "r.txt", "or", q->p->name);
        }
      }      
    }

    if (!q->tr) {
      debug("not @R was found for table [%s].\n", q->p->name);
      q = q->next;
      continue;
    }
    
    show_rtree(q->tr);

    f = fopen(r, "a");
    if (!f) return 0;

    sync_rtree_to_file(q->tr, f);

    fclose(f);

    q = q->next;
  }

  fe_return(1);
}



static void
delete_unmarked_node(s_bin_tree_node **root, s_bin_tree_node *nd)
{
  int rt;
  
  fs();

  if (!nd || !root) return;

  delete_unmarked_node(root, nd->left);
  delete_unmarked_node(root, nd->right);

  debug("@row [%s].\n", nd->key);
  
  if (!nd->mark) {
    debug("unmardk, so delete this node from tree.\n");
    rt = binary_tree_delete(root, nd);
  }
  else {
    debug("marked, so clear the mark to zero.\n");
    nd->mark = !Y;
  }
  
  fe();
}


static int
update_rtree_on_and(char *r0, char *r1, s_bin_tree_node **root)
{
  FILE *f;
  char *buf;
  s_bin_tree_node *nd;
  char *r2;
  
  int rt;
  
  fs();

  if (!r0 || !r1) return 0;

  buf = gc_malloc(80);
  if (!buf) return 0;

  if (strcmpi(r0, "r.txt") && strcmpi(r1, "r.txt")) {
    f = fopen(r0, "r");
    if (!f) return 0;

    debug("[%s] was opened.\n", r0);

    while (!feof(f)) {
    
      memset(buf, 0, 80);
      fgets(buf, 80, f);
    
      buf[strlen(buf)-1] == '\n' ?
        buf[strlen(buf)-1] = '\0' : 0;
      
      if (!strlen(buf)) continue;

      debug("@row read: [%s].\n", buf);

      nd = binary_tree_isearch(INSERT, root, buf, strlen(buf) + 1, strcmpi);
    }

    fclose(f);

    r2 = r1;
  }
  else if (!strcmpi(r0, "r.txt") && strcmpi(r1, "r.txt")) {
    r2 = r1;
  }
  else if (strcmpi(r0, "r.txt") && !strcmpi(r1, "r.txt")) {
    r2 = r0;
  }
  else {
    return 0;
  }

  f = fopen(r2, "r");
  if (!f) return 0;
  
  debug("[%s] was opened.\n", r2);
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
  
    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;

    debug("@row read: [%s].\n", buf);
    
    nd = binary_tree_isearch(SEARCH, root, buf, strlen(buf) + 1, strcmpi);

    if (nd) {
      nd->mark = Y;
      debug("@R marked: [%s].\n", buf);
    }
  }     

  fclose(f);

  /* scan the tree and delete the node which was not marked.
    * remember to clear the mark flag of node to "0" which was marked.
    */
  delete_unmarked_node(root, *root);
  
  fe_return(1);
}

static int
update_rtree_on_or(char *r0, char *r1, s_bin_tree_node **root)
{
  FILE *f;
  char *buf;
  s_bin_tree_node *nd;
  char *r2;
  
  int rt;
  
  fs();
  
  if (!r0 || !r1) return 0;
  
  buf = gc_malloc(80);
  if (!buf) return 0;
    
  if (strcmpi(r0, "r.txt") && strcmpi(r1, "r.txt")) {
    f = fopen(r0, "r");
    if (!f) return 0;
  
    debug("[%s] was opened.\n", r0);
    
    while (!feof(f)) {
    
      memset(buf, 0, 80);
      fgets(buf, 80, f);
      
      buf[strlen(buf)-1] == '\n' ?
        buf[strlen(buf)-1] = '\0' : 0;
      
      if (!strlen(buf)) continue;
      
      debug("@row read: [%s].\n", buf);
      
      nd = binary_tree_isearch(INSERT, root, buf, strlen(buf) + 1, strcmpi);
    }
  
    fclose(f);
  
    r2 = r1;
  }
  else if (!strcmpi(r0, "r.txt") && strcmpi(r1, "r.txt")) {
    r2 = r1;
  }
  else if (strcmpi(r0, "r.txt") && !strcmpi(r1, "r.txt")) {
    r2 = r0;
  }
  else {
    return 0;
  }
  
  f = fopen(r2, "r");
  if (!f) return 0;
  
  debug("[%s] was opened.\n", r2);
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
  
    buf[strlen(buf)-1] == '\n' ?
    buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;
    
    debug("@row read: [%s].\n", buf);
    
    nd = binary_tree_isearch(INSERT, root, buf, strlen(buf) + 1, strcmpi);
  }
  
  fclose(f);
  
  fe_return(1);
}


static char*
eval_bool_value(char  *r0, char  *r1, char *op, char *table_name)
{
  int rt;
  char  *r2;
  s_bin_tree_node **tr;
  
  fs();

  r0 ? debug("r0 [%s].\n", r0) : 0;
  op ? debug("op [%s].\n", op) : 0;
  r1 ? debug("r1 [%s].\n", r1) : 0;


  tr = get_result_tree(table_name);
  if (!tr) return 0;

  *tr ? debug("result tree is not null.\n") : debug("result tree is null.\n");

  if (!strcmpi(op, "and")) {
    
    rt = update_rtree_on_and(r0, r1, tr);
    if (!rt) return 0;
  }
  else if (!strcmpi(op, "or")) {
    
    rt = update_rtree_on_or(r0, r1, tr);
    if (!rt) return 0;
  }
  else {
    return NULL;
  }

  r2 = "r.txt";

  fe_return(r2);
}


static s_value_term*
eval_bool_term(s_value_term *t0, s_value_term *t1, char *op)
{
  int rt;
  s_value_term *t2;
  char *r0, *r1, *r2;
  char *table_name;
  s_bin_tree_node **tr;
  
  fs();

  if (!t0 || !t1) return NULL;


  t0->r ? debug("r0[%s]\n", t0->r) : 0;
  t0->r1 ? debug("r1[%s]\n", t0->r1) : 0;
  t0->op ? debug("op[%s]\n", t0->op) : 0;
  debug("\n");

  debug("[%s]\n", op);

  t1->r ? debug("[r0%s]\n", t1->r) : 0;
  t1->r1 ? debug("r1[%s]\n", t1->r1) : 0;
  t1->op ? debug("op[%s]\n", t1->op) : 0;
  debug("\n");


  t2 = gc_malloc(sizeof(s_value_term));
  if (!t2) return NULL;
  
  if (t0->r && t1->r &&
    !strcmpi(t0->obj.table_name, t1->obj.table_name)) {

    r0 = t0->r;
    r1 = t1->r;

    r2 = eval_bool_value(r0, r1, op, t0->obj.table_name);
    if (!r2) return NULL;

    t2->r = r2;
    t2->obj.table_name = t0->obj.table_name;
  }
  else if (t0->r && t1->r1 && 
    !strcmpi(t0->obj.table_name, t1->obj1.table_name)) {
    
    r0 = t0->r;
    r1 = t1->r1;

    r2 = eval_bool_value(r0, r1, op, t0->obj.table_name);
    if (!r2) return NULL;

    t2->r = r2;
    t2->obj.table_name = t0->obj.table_name;
  }
  else {
    debug("@~@\n");

    r0 = t0->r;
    table_name = t0->obj.table_name;
    r1 = "r.txt";

    tr = get_result_tree(table_name);
    if (!tr) return 0;
    
    if (!(*tr)) {
      r2 = eval_bool_value(r0, r1, "or", table_name);
    }
    else {
      r2 = eval_bool_value(r0, r1, op, table_name);
    }

    t2->r = r2;
    t2->obj.table_name = table_name;
  }

  if (!t0->r1) {

    if (t1->r1) {

      r0 = (r1 == t1->r ? t1->r1 : t1->r);
      table_name = (r1 == t1->r ? t1->obj1.table_name : t1->obj.table_name);
      
      r1 = "r.txt";

      r2 = eval_bool_value(r0, r1, op, table_name);
      if (!r2) return NULL;

      t2->r1 = r2;
      t2->obj1.table_name = table_name;
    }

    debug("@^@\n");
    
    fe_return(t2);
  }
  
  if (t0->r1 && t1->r &&
    !strcmpi(t0->obj1.table_name, t1->obj.table_name)) {

    r0 = t0->r1;
    r1 = t1->r;
    table_name = t0->obj1.table_name;

    r2 = eval_bool_value(r0, r1, op, table_name);
    if (!r2) return NULL;
  }
  else if (t0->r1 && t1->r1 &&
    !strcmpi(t0->obj1.table_name, t1->obj1.table_name)) {
    
    r0 = t0->r1;
    r1 = t1->r1;
    table_name = t0->obj1.table_name;

    r2 = eval_bool_value(r0, r1, op, table_name);
    if (!r2) return NULL;  
  }
  else {

    debug("@!@\n");
    
    r0 = t0->r1;
    table_name = t0->obj1.table_name;
    
    tr = get_result_tree(table_name);
    if (!tr) return 0;

    r1 = "r.txt";
    
    if (!(*tr)) {
      r2 = eval_bool_value(r0, r1, "or", table_name);
    }
    else {
      r2 = eval_bool_value(r0, r1, op, table_name);
    }
    
    if (!r2) return NULL;
  }

  t2->r1 = r2;
  t2->obj1.table_name = table_name;

  fe_return(t2);
}


static s_value_term*
search_primary(s_boolean_primary *p, int has_not)
{
  s_predicate *pr;
  s_row_value_constructor *cst;
  s_comparison_predicate *cmp;
  s_row_value_constructor_list *lst;
  s_value_expression *e;
  s_column_reference *ref;
  int rt;
  s_value_term t, *t1;
  
  fs();

  if (p->is_search_condition) {
    t1 = search_condition(p->se_condition);
    fe_return(t1);
  }

  pr = &p->pred;
  
  cmp = &pr->cmp_predicate;

  memset(&t, 0, sizeof(s_value_term));
  
  t.op = get_operator(&cmp->op, has_not);
  if (!t.op) return NULL;
  
  debug("op: [%s].\n", t.op);

  cst = &cmp->lrow_val_cstr;
  if (!cst->is_row_value_constructor_element) return NULL;
  debug("left row_value_constructor_elemenf.\n");

  e = &cst->row_val_cstr_elm.val_expr;
  if (e->is_numeric_value_expression) {
    debug("numeric_value_expression.\n");

    t.obj.name = 
      e->num_val_expr.tm.ft.num_prim.val_expr_prim.is_column_reference ?
        e->num_val_expr.tm.ft.num_prim.val_expr_prim.cl_ref.cname : NULL;
      
    t.obj.name ? debug("column ref [%s].\n", t.obj.name) : 0;

    if (!t.obj.name) {
      t.obj.int_val = (int)eval_numeric_column(&e->num_val_expr);
      debug("int value [%d].\n", t.obj.int_val);
      t.obj.type = "int";
    }
    else {
      ref = &e->num_val_expr.tm.ft.num_prim.val_expr_prim.cl_ref;

      if (ref->has_qualifier) {
        debug("has_qualifier.\n");
        t.obj.table_name = ref->qlf.tb_name.qlf_name.qlf_identifier;
      }
    }
    
  }
  else if (e->is_string_value_expression) {
    debug("string_value_expression.\n");

    t.obj.str_val = eval_string_column(&e->str_val_expr);
    debug("string value [%s].\n", t.obj.str_val);
    t.obj.type = "char";
  }
  else {
    return NULL;
  }
  
  cst = &cmp->rrow_val_cstr;
  if (!cst->is_row_value_constructor_element) return NULL;
  debug("right row_value_constructor_element.\n");

  e = &cst->row_val_cstr_elm.val_expr;
  if (e->is_numeric_value_expression) {
    debug("numeric_value_expression.\n");

    if (!t.obj.name) {
      t.obj.name = 
        e->num_val_expr.tm.ft.num_prim.val_expr_prim.is_column_reference ?
          e->num_val_expr.tm.ft.num_prim.val_expr_prim.cl_ref.cname : NULL;
      
      t.obj.name ? debug("column ref [%s].\n", t.obj.name) : 0;
    }
    else {

      t.obj1.name = 
        e->num_val_expr.tm.ft.num_prim.val_expr_prim.is_column_reference ?
          e->num_val_expr.tm.ft.num_prim.val_expr_prim.cl_ref.cname : NULL;
      
      t.obj1.name ? debug("column ref1 [%s].\n", t.obj1.name) : 0;

      if (!t.obj1.name) {
        t.obj.int_val = (int)eval_numeric_column(&e->num_val_expr);
        debug("int value [%d].\n", t.obj.int_val);
        t.obj.type = "int";
      }
      else {
        ref = &e->num_val_expr.tm.ft.num_prim.val_expr_prim.cl_ref;

        if (ref->has_qualifier) {
          debug("has_qualifier.\n");
          t.obj1.table_name = ref->qlf.tb_name.qlf_name.qlf_identifier;
        }
        
        if (!t.obj1.table_name) return NULL;
      }
    }
  }
  else if (e->is_string_value_expression) {
    debug("string_value_expression.\n");

    t.obj.str_val = eval_string_column(&e->str_val_expr);
    debug("string value [%s].\n", t.obj.str_val);
    t.obj.type = "char";
  }
  else {
    return NULL;
  }

  if (t.obj1.name) {
    
    rt = search_value_1(&qlst, &t);
    if (!rt) return NULL;
    
  }
  else {

    if (!t.obj.table_name) {

      if (!qlst.next) return NULL;

      t.obj.table_name = qlst.next->p->name;
      debug("qlst.next->p->name [%s].\n", qlst.next->p->name);
      
    }
    else {
      debug("t.obj.table_name [%s].\n", t.obj.table_name);
    }

    rt = search_value(t.obj.table_name, &t);
    if (!rt) return NULL;
    debug("@R was found at [%s].\n", t.r);
  }

  t1 = gc_malloc(sizeof(s_value_term));
  if (!t1) return NULL;

  memcpy(t1, &t, sizeof(s_value_term));
  
  fe_return(t1);
}

static s_value_term*
search_bool_factor(s_boolean_factor *f)
{
  s_boolean_primary *p;
  s_value_term *r;
  
  
  fs();
  
  p = &f->bool_test.bool_prim;
  r = search_primary(p, f->has_not);
  if (!r) return NULL;

  fe_return(r);
}

static s_value_term*
search_bool_term1(s_boolean_term1 *t1)
{
  s_value_term *r0, *r1, *r2;
  int rt;
  
  fs();

  r0 = search_bool_factor(&t1->bool_factor);
  if (!r0) return NULL;
  
  if (t1->next) {
    debug("AND.\n");
    
    r1 = search_bool_term1(t1->next);

    r2 = eval_bool_term(r0, r1, "and");
  }
  else {
    r2 = r0;
  }

  fe_return(r2);
}

static s_value_term*
search_bool_term(s_boolean_term *t)
{
  s_value_term *r1, *r2, *r3;
  
  fs();

  r1 = search_bool_factor(&t->bool_factor);
  if (!r1) return NULL;

  if (!t->has_and) {    
    fe_return(r1);
  }
  
  debug("AND.\n");
  
  r2 = search_bool_term1(&t->bool_term1);

  r3 = eval_bool_term(r1, r2, "and");

  fe_return(r3);
}

static s_value_term*
search_condition1(s_search_condition1 *c1)
{
  s_value_term *r0, *r1, *r2;
  
  fs();

  r0 = search_bool_term(&c1->bool_term);
  if (!r0) return NULL;

  if (c1->next) {

    debug("OR.\n");

    r1 = search_condition1(c1->next);

    r2 = eval_bool_term(r0, r1, "or");
  }
  else {
    r2 = r0;
  }

  fe_return(r2);
}

static s_value_term*
search_condition(s_search_condition *c)
{
  s_value_term *r0, *r1, *r2;

  fs();

  /*there are tow categories of term: AND and OR,
     *scan the condition stack and evalute all terms,
     *the results would be recorded in @R, where @R is a file path.
     *
     *uses a binary tree to store and filter the results when evaluting the terms,
     *@R was stored on the fitered tree,  so can be read with different seqence to show.
     */
  
  r0 = search_bool_term(&c->bool_term);
  if (!r0) return NULL;

  c->has_or ? debug("has_or.\n") : debug("has_not_or.\n");
  if (!c->has_or) fe_return(r0);
  
  debug("OR.\n");

  r1 = search_condition1(&c->se_condition_1);
  if (!r1) return NULL;

  r2 = eval_bool_term(r0, r1, "or");

  fe_return(r2);
}

static int
query_all(s_table_expression *e)
{
  s_from_clause *f;
  s_table_reference_list *lst;
  char *table_name;
  int rt;
  char *fpath;
  s_where_clause *w;
  s_value_term *r;
  s_query_info_list *q;
  char *rr;
  
  fs();

  e->has_where_clause ? debug("has_where_clause.\n") : 0;
  e->has_having_clause ? debug("has_having_clause.\n") : 0;
  e->has_group_by_clause ? debug("has_group_by_clause.\n") : 0;

  f = &e->from_cls;
  lst = &f->tb_ref_lst;

  memset(&qlst, 0, sizeof(s_query_info_list));
  q = &qlst;
  
  while(lst) {
  
    table_name = lst->tb_ref.tb_name.qlf_name.qlf_identifier;
    if (!table_name) return 0;

    q = q->next = gc_malloc(sizeof(s_query_info_list));
    if (!q) return 0;
    
    q->p = get_table_property(table_name);
    if (!q->p) return 0;
   
    lst = lst->next;
  }
    
  if (e->has_where_clause) {
  
    w = &e->where_cls;
    r = search_condition(&w->se_cond);
    
    rt = (r ? sync_results(qlst.next, r) : 0);
  }
  else {

    rt = 0;
    
    q = &qlst;
    q = q->next;
    while (q) {
    
      rr = search_table(q->p->name);
      if (rr) rt += copy_file(rr, make_result_file_name(q->p->name));

      q = q->next;
    }
  }

  if (!rt) debug("nothing was found.\n");
  fe_return(rt);
}

int
select_table(void *stack)
{
  s_direct_select_statement_multiple_rows *s = stack;
  s_query_specification *q;
  s_table_expression *e;
  int rt;
  
  fs();

  q = &s->query_expr.njoin_qexpr.njoin_qterm.njoin_qprimary.sptable.query_spec;
  e = &q->table_expr;
  
  if (q->select_lst.is_asterisk) {
    rt = query_all(e);
    if (!rt) return 0;
  }

  fe_return(1);
}

