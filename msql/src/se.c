
/*
*search engine(se) subsystem.
*/


#include "se.h"
#include "common.h"


static s_bin_tree_node *table_tree;

static s_bin_tree_node*
get_value_tree(s_bin_tree_node **column_tree, char *column_name);


static void
delete_column_value_as_tree
(
  s_bin_tree_node *root, s_column_obj *obj, s_column_obj *obj_deleted
);


static void
show_column_tree(s_bin_tree_node *root)
{
  s_column_obj *obj;
  s_table_property *p;
  
  if (!root) return;

  obj = root->val;
  obj->name ? debug("%s", obj->name) : 0;
  obj->type ? debug(" %s", obj->type) : 0;
  obj->len ? debug(" %d", obj->len) : 0;
  debug("\n");
  
  show_column_tree(root->left);
  show_column_tree(root->right);  
}

void
show_column_list(s_column_obj_list *lst)
{
  s_column_obj *obj;
  
  fs();

  if (!lst) return;

  while (lst) {
  
    obj = lst->obj;
    obj->name ? debug("%s", obj->name) : 0;
    obj->type ? debug(" %s", obj->type) : 0;
    obj->len ? debug(" %d", obj->len) : 0;
    debug("\n");


    lst = lst->next;
  }

  fe();
}

static void
show_table_tree(s_bin_tree_node *root)
{
  s_table_property *p;
  
  if (!root) return;

  fs();
  
  debug("%s \n", root->key);

  p = root->val;
  if (p) show_column_list(p->cl_obj_lst);
  if (p) show_column_tree(p->cl_tree);
  
  show_table_tree(root->left);
  show_table_tree(root->right);

  fe();
}

static s_column_obj *
create_column_obj(char *column_info)
{
  s_column_obj *c;
  char *s;

  fs();
  
  c = calloc(1, sizeof(s_column_obj));
  if (!c) return 0;

  s = strtok(column_info, ",");
  if (!s) return NULL;
  
  c->name = strdup(s);
  debug("column name: %s \n", c->name);
  
  s = strtok(NULL, ",");
  if (s) {
    c->type = strdup(s);
    debug("column type: %s \n", c->type);
    
    s = strtok(NULL, ",");
    if (s) {
      c->len = atoi(s);
      debug("column len: %d \n", c->len);
    }
  }

  fe_return(c);
}

s_bin_tree_node *
make_column_tree(char *table_name)
{
  FILE *f;
  char fname[255];
  char column[255];
  char *key;
  s_bin_tree_node *column_tree;
  s_bin_tree_node *nd;
  s_column_obj *c;
  
  fs();

  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), table_name), ".txt");

  debug("open file %s \n", fname);
  
  f = fopen(fname, "r");
  if (!f) fe_return(1);

  column_tree = NULL;
  
  while (!feof(f)) {
    memset(column, 0, sizeof(column));
    fgets(column, sizeof(column), f);

    column[strlen(column)-1] == '\n' ?
      column[strlen(column)-1] = '\0' : 0;

    if (!strlen(column) || column[0] == '-') continue;
    
    debug("column info: %s\n", column);
    
    c = create_column_obj(column);
    if (!c) {
      fclose(f);
      return 0;
    }

    c->table_name = table_name;
    
    key = c->name;
    nd = binary_tree_isearch(INSERT, &column_tree, key, strlen(key)+1, strcmpi);
    if (!nd) {
      fclose(f);
      return 0;    
    }
    nd->val = c;
    
  }

  fclose(f);
  fe_return(column_tree);
}

s_column_obj_list*
make_column_obj_list(char *table_name)
{
  FILE *f;
  char fname[255];
  char column[255];
  s_column_obj_list *lst;
  s_column_obj_list *cur;
  s_column_obj_list *end;
  
  fs();
  
  memset(fname, 0, sizeof(fname));
  strcat(strcat(strcat(fname, "debug/tables/"), table_name), ".txt");
  
  debug("open file %s \n", fname);
  
  f = fopen(fname, "r");
  if (!f) fe_return(1);

  lst = malloc(sizeof(s_column_obj_list));
  if (!lst) return NULL;
  lst->next = NULL;

  cur = lst;
  while (!feof(f)) {
    memset(column, 0, sizeof(column));
    fgets(column, sizeof(column), f);
    
    column[strlen(column)-1] == '\n' ?
    column[strlen(column)-1] = '\0' : 0;
    
    if (!strlen(column) || column[0] == '-') continue;
    
    debug("column info: %s\n", column);
    
    cur->obj = create_column_obj(column);
    if (!cur->obj) {
      fclose(f);
      return 0;
    }

    end = cur;
    cur = cur->next = malloc(sizeof(s_column_obj_list));
    if (!cur) return NULL;
    cur->next = NULL;
  }

  if (cur != lst) {
    free(cur);
    end->next = NULL;
  }
  
  fclose(f);
  fe_return(lst);
}


static int
make_table_tree(void)
{
  FILE *f;
  char name[255];
  s_bin_tree_node *nd;
  s_table_property *p;
  int len;
  
  fs();

  f = fopen("debug/tables/tables.txt", "r");
  if (!f) return 1;
  
  while (!feof(f)) {
    memset(name, 0, sizeof(name));
    fgets(name, sizeof(name), f);
    
    name[strlen(name)-1] == '\n' ?
      name[strlen(name)-1] = '\0' : 0;

    if (!strlen(name) || name[0] == '-') continue;
    
    debug("table name: %s \n", name);

    len = strlen(name) + 1;
    nd = binary_tree_isearch(INSERT, &table_tree, name, len, strcmpi);
    if (!nd) {
      fclose(f);
      return 0;
    }

    p = malloc(sizeof(s_table_property));
    if (!p) return 0;

    p->name = nd->key;
    
    p->cl_tree = make_column_tree(name);

    p->cl_obj_lst = make_column_obj_list(name);
    if (!p->cl_obj_lst) return NULL;
    
    nd->val = p;
  }

  show_table_tree(table_tree);
  
  fclose(f);
  fe_return(1);
}

static void
make_column_value_tree(char *tb_name, s_bin_tree_node *root)
{
  char *s;
  FILE *f;
  char *buf;
  s_bin_tree_node *nd;
  s_column_obj *obj;
  s_bin_tree_node **t;
  
  fs();

  if (!root) return;

  s = gc_malloc(strlen("debug/tables/values/") + strlen(tb_name) + strlen(".") + 
    strlen(root->key) + strlen(".txt") + 1);
  if (!s) return;

  sprintf(s, "debug/tables/values/%s.%s.txt", tb_name, root->key);

  f = fopen(s, "r");
  if (!f) return;

  buf = gc_malloc(80);
  if (!buf) return;
  
  debug("read values from [%s]... \n", s);
  while (!feof(f)) {
    memset(buf, 0, 80);
    fgets(buf, 80, f);

    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf) || buf[0] == '-') continue;

    debug("value: [%s]. \n", buf);

    obj = root->val;
    t = &obj->val_tree;
    if (!strcmpi(obj->type, "char")) {

      nd = binary_tree_isearch(INSERT, t, buf, strlen(buf) + 1, strcmpi);
      if (!nd) return;
    }
    else if (!strcmpi(obj->type, "int")) {
    
      obj->int_val = atoi(buf);
      nd = binary_tree_isearch(INSERT, t, &obj->int_val, 4, int_cmp);
      if (!nd) return;
    }
    else {
    }
  }

  fclose(f);
  
  root->left ? make_column_value_tree(tb_name, root->left) : 0;
  root->right ? make_column_value_tree(tb_name, root->right) : 0;

  fe();
}


static void
make_table_value_tree(s_bin_tree_node *root)
{
  s_table_property *p;
  
  fs();

  if (!root) return;
  
  p = root->val;
  p->cl_tree ? make_column_value_tree(root->key, p->cl_tree) : 0;

  root->left ? make_table_value_tree(root->left) : 0;
  root->right ? make_table_value_tree(root->right) : 0;

  fe();
}

int
init_se(void)
{
  int rt;

  fs();

  rt = make_table_tree();
  if (!rt) return 0;

  make_table_value_tree(table_tree);
  
  fe_return(1);
}

int 
is_table_exist(char *table_name)
{
  int rt;
  
  fs();
  
  rt = !!binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);

  fe_return(rt);
}

s_bin_tree_node*
get_table_node(char *table_name)
{
  s_bin_tree_node *nd;

  fs();
  
  nd = binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);

  fe_return(nd);
}

s_table_property*
get_table_property(char *table_name)
{
  s_bin_tree_node *nd;

  fs();

  debug("table name [%s].\n", table_name);
  
  nd = get_table_node(table_name);

  if (!nd) return NULL;

  fe_return(nd->val);
}

static void
delete_value_tree(s_bin_tree_node *root, s_column_obj *obj)
{
  char *s;
  
  fs();

  if (!root) return;

  root->left ? delete_value_tree(root->left, obj) : 0;
  root->right ? delete_value_tree(root->right, obj) : 0;

  s = gc_malloc(1024);
  if (!s) return;
  
  if (!strcmpi(obj->type, "int")) {
  
    debug("int value [%d].\n", *(int*)root->key);

    sprintf(s, "debug/tables/values/%s.%s E %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);

    sprintf(s, "debug/tables/values/%s.%s NE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);

    sprintf(s, "debug/tables/values/%s.%s GE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);

    sprintf(s, "debug/tables/values/%s.%s LE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);    

    sprintf(s, "debug/tables/values/%s.%s G %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);

    sprintf(s, "debug/tables/values/%s.%s L %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("delete [%s].\n", s);
    unlink(s);    
  }
  else if (!strcmpi(obj->type, "char")) {
  
    debug("char value [%s].\n", root->key);

    sprintf(s, "debug/tables/values/%s.%s E %s.txt",
      obj->table_name, obj->name, root->key);

    debug("delete [%s].\n", s);
    unlink(s);

    sprintf(s, "debug/tables/values/%s.%s NE %s.txt",
      obj->table_name, obj->name, root->key);

    debug("delete [%s].\n", s);
    unlink(s);    
  }
  else {
    debug("unkown value type.\n");
  }

  binary_tree_delete(root, root);
  
  fe();
}

void
delete_column_tree(s_bin_tree_node *root, char *table_name)
{
  s_bin_tree_node *nd;
  s_column_obj *o;
  char *s;
  
  fs();

  if (!root) return;
  
  root->left ? delete_column_tree(root->left, table_name) : 0;
  root->right ? delete_column_tree(root->right, table_name) : 0;

  nd = get_column_node(&root, root->key);
  if (nd) {
    o = nd->val;
    o->table_name = table_name;
    delete_value_tree(o->val_tree, o);

    s = gc_malloc(1024);
    if (!s) return;

    sprintf(s, "debug/tables/values/%s.%s.txt", o->table_name, o->name);

    debug("delete [%s].\n", s);
    unlink(s);
  }
  
  binary_tree_delete(root, root);

  fe();
}

static void
free_table_obj_list(s_column_obj_list *lst)
{
  fs();

  if (!lst) return;
  
  if (lst->next) free_table_obj_list(lst->next);
  free(lst);

  fe();
}

int
delete_table_node(char *table_name)
{
  s_bin_tree_node *nd;
  s_table_property *p;
  int rt;
  char *s;
  
  fs();

  nd = binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);
  if (!nd) return 0;

  p = nd->val;

  free_table_obj_list(p->cl_obj_lst);
  
  delete_column_tree(p->cl_tree, table_name);
  nd->val = NULL;

  s = gc_malloc(1024);
  if (!s) return;
  
  sprintf(s, "debug/tables/values/%s.txt", table_name);
  
  debug("delete [%s].\n", s);
  unlink(s);

  debug("node name: %s \n", nd->key);
  rt = binary_tree_delete(&table_tree, nd);
  if (!rt) return 0;

  fe_return(1);
}


s_bin_tree_node*
create_table_node(char *table_name)
{
  s_bin_tree_node *nd;
  char *key;
  s_bin_tree_node *table_node;
  s_bin_tree_node *column_tree;
  s_table_property *p;
  
  fs();

  debug("table name: %s \n", table_name);
  
  key = table_name;
  nd = binary_tree_isearch(INSERT, &table_tree, key, strlen(key) + 1, strcmpi);

  p = malloc(sizeof(s_table_property));
  if (!p) return NULL;
  
  p->cl_tree = make_column_tree(table_name);
  if (!p->cl_tree) return NULL;

  p->cl_obj_lst = make_column_obj_list( table_name);
  if (!p->cl_obj_lst) return NULL;

  nd->val = p;

  show_table_tree(table_tree);
  
  fe_return(nd);
}

int
is_column_exist(char *table_name, char *column_name)
{
  s_bin_tree_node *nd;
  s_table_property *p;
  
  nd = binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);
  if (!nd) return 0;

  p = nd->val;
  nd = binary_tree_isearch(SEARCH, p->cl_tree, column_name, 0, strcmpi);
  return !!nd;
}


static int
update_row_with_new_column(char *fname, s_column_obj *obj)
{
  char *s;
  int rt;
  const int DEFAULT_INT = 0;
  const char *DEFAULT_CHAR = "\'\'";
  
  fs();

  s = gc_malloc(1024);
  if (!s) return 0;
  
  if (!strcmpi(obj->type, "int")) {
    sprintf(s, ",%d", DEFAULT_INT); 
  }
  else if (!strcmpi(obj->type, "char")) {
    sprintf(s, ",%s", DEFAULT_CHAR); 
  }
  else {
  }

  debug("update value [%s].\n", s);
  
  rt = copy_file_and_update(fname, "debug/tables/values/tmp.txt", s);
  if (!rt) {
    unlink("debug/tables/values/tmp.txt");
    return 0;
  }

  unlink(fname);
  
  rt = copy_file("debug/tables/values/tmp.txt", fname);
  if (!rt) return 0;

  unlink("debug/tables/values/tmp.txt");

  fe_return(1);
}


static void
add_column_value_as_tree
(
  s_bin_tree_node *root, s_column_obj *obj, s_column_obj *obj_new
)
{
  char *s;
  
  fs();

  if (!root) return;

  root->left ? add_column_value_as_tree(root->left, obj, obj_new) : 0;
  root->right ? add_column_value_as_tree(root->right, obj, obj_new) : 0;

  s = gc_malloc(1024);
  if (!s) return;
  
  if (!strcmpi(obj->type, "int")) {
  
    debug("int value [%d].\n", *(int*)root->key);

    sprintf(s, "debug/tables/values/%s.%s E %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);

    sprintf(s, "debug/tables/values/%s.%s NE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);

    sprintf(s, "debug/tables/values/%s.%s GE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);

    sprintf(s, "debug/tables/values/%s.%s LE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);  

    sprintf(s, "debug/tables/values/%s.%s G %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);

    sprintf(s, "debug/tables/values/%s.%s L %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);
  }
  else if (!strcmpi(obj->type, "char")) {
  
    debug("char value [%s].\n", root->key);

    sprintf(s, "debug/tables/values/%s.%s E %s.txt",
      obj->table_name, obj->name, root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);

    sprintf(s, "debug/tables/values/%s.%s NE %s.txt",
      obj->table_name, obj->name, root->key);

    debug("update [%s].\n", s);
    update_row_with_new_column(s, obj_new);   
  }
  else {
    debug("unkown value type.\n");
  }
  
  fe();
}


static void
update_column_tree(s_bin_tree_node *root, s_column_obj *obj, char *tp)
{
  s_bin_tree_node *nd;
  s_column_obj *o;
  
  fs();

  if (!root) {
    debug("column tree is null.\n");
    return;
  }
  
  root->left ? update_column_tree(root->left, obj, tp) : 0;
  root->right ? update_column_tree(root->right, obj, tp) : 0;

  o = root->val;
  o->table_name = obj->table_name;

  nd = get_value_tree(&root, root->key);
  if (nd) {
  
     debug("type [%s].\n", tp);
     if (!strcmpi(tp, "add")) {
       add_column_value_as_tree(nd, o, obj);
       fe();
     }
     else if (!strcmpi(tp, "delete")) {
       delete_column_value_as_tree(nd, o, obj);
     }
     else {
     }
  }
}


static int
update_value_with_new_column(s_column_obj *obj)
{
  s_bin_tree_node *nd;
  s_table_property *p;
  char *s;
  int rt;
  
  fs();

  p = get_table_property(obj->table_name);
  if (!p) return 0;

  update_column_tree(p->cl_tree, obj, "add");

  s = gc_malloc(1024);
  if (!s) return 0;

  sprintf(s, "debug/tables/values/%s.txt", obj->table_name);
  
  rt = update_row_with_new_column(s, obj);
  if (!rt) return 0;

  fe_return(1);
}

int
create_column_node(char *table_name, char *column_info)
{
  s_bin_tree_node *nd;
  s_column_obj *c;
  char *key;
  s_table_property *p;
  int rt;

  fs();
  
  debug("column info: %s\n", column_info);

  nd = binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);
  if (!nd) return 0;
  
  c = create_column_obj(column_info);
  if (!c) return 0;

  c->table_name = table_name;
  p = nd->val;

  /*update values.
   */
  rt = update_value_with_new_column(c);
  if (!rt) return 0;

  key = c->name;
  nd = binary_tree_isearch(INSERT, &p->cl_tree, key, strlen(key)+1, strcmpi);
  if (!nd) return 0;   
  nd->val = c;

  fe_return(1);
}

static int
delete_value_of_column_of_row(char *row, int column_index)
{
  char *s, *e, *s1;
  int i;
  
  fs();

  debug("column index [%d].\n", column_index);

  i = 0;
  s1 = NULL;

  e = row + strlen(row) - 1;
  
  s = strtok(row, ",");
  if (!s) return NULL;
  i++;

  debug("s [%s].\n", s);
  
  if (i == column_index) {
  
    debug("found start.\n");

    s1 = s;
    while (s1 <= e) *s1++ = *(s1 + strlen(s) + 1);
  }
  else {
    while (s) {

      *(s + strlen(s)) = ',';
      
      s = strtok(NULL, ",");
      if (!s) break;

      debug("s [%s].\n", s);
      
      i++;
      if (i == column_index) {
      
        debug("found start.\n");

        s--;
        s1 = s;
        while (s1 <= e) *s1++ = *(s1 + strlen(s) + 1);
        
        break;
      }
    }
  }
  
  fe_return(1);
}


int
copy_file_and_delete(char *src, char *dst, int column_index)
{
  FILE *f, *f1;
  char *buf;
  
  fs();

  f = fopen(src, "r");
  if (!f) return 0;
  debug("[%s] was opened.\n", src);


  f1 = fopen(dst, "a");
  if (!f1) return 0;
  debug("[%s] was opened.\n", dst);

  
  buf = gc_malloc(80);
  if (!buf) return 0;
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
  
    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;
  
    debug("copy @row: [%s].\n", buf);

    delete_value_of_column_of_row(buf, column_index);

    debug("row updated to [%s].\n", buf);
  
    fprintf(f1, "%s\n", buf);
  }
  
  fclose(f);  
  fclose(f1);

  fe_return(1);
}


static int
get_column_index_of_row(s_column_obj *obj)
{
  int index;
  s_table_property *p;
  s_column_obj_list *lst;
  
  fs();

  index = 0;

  p = get_table_property(obj->table_name);
  if (!p) return 0;

  debug("column name [%s].\n", obj->name);
  
  lst = p->cl_obj_lst;
  while (lst) {

    debug("match [%s].\n", lst->obj->name);

    index++;
    if (!strcmpi(lst->obj->name, obj->name)) fe_return(index);

    lst = lst->next;
  }
  
  return 0;
}

static int
delete_column_value_as_row(char *fname, s_column_obj *obj)
{
  int index;
  int rt;
  
  fs();

  index = get_column_index_of_row(obj);
  if (!index) return 0;
  
  rt = copy_file_and_delete(fname, "debug/tables/values/tmp.txt", index);
  if (!rt) {
    unlink("debug/tables/values/tmp.txt");
    return 0;
  }

  unlink(fname);
  
  rt = copy_file("debug/tables/values/tmp.txt", fname);
  if (!rt) return 0;

  unlink("debug/tables/values/tmp.txt");

  fe_return(1);
}


static void
delete_column_value_as_tree
(
  s_bin_tree_node *root, s_column_obj *obj, s_column_obj *obj_deleted
)
{
  char *s;
  int flag;
  
  fs();

  if (!root) return;

  root->left ? delete_column_value_as_tree(root->left, obj, obj_deleted) : 0;
  root->right ? delete_column_value_as_tree(root->right, obj, obj_deleted) : 0;

  s = gc_malloc(1024);
  if (!s) return;

  flag = !strcmpi(obj->name, obj_deleted->name);

  if (flag) {
    sprintf(s, "debug/tables/values/%s.%s.txt",
      obj->table_name, obj->name);
    unlink(s);
  }
  
  if (!strcmpi(obj->type, "int")) {
  
    debug("int value [%d].\n", *(int*)root->key);

    sprintf(s, "debug/tables/values/%s.%s E %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }
    
    sprintf(s, "debug/tables/values/%s.%s NE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }

    sprintf(s, "debug/tables/values/%s.%s GE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }


    sprintf(s, "debug/tables/values/%s.%s LE %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }


    sprintf(s, "debug/tables/values/%s.%s G %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }


    sprintf(s, "debug/tables/values/%s.%s L %d.txt",
      obj->table_name, obj->name, *(int*)root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }

  }
  else if (!strcmpi(obj->type, "char")) {
  
    debug("char value [%s].\n", root->key);

    sprintf(s, "debug/tables/values/%s.%s E %s.txt",
      obj->table_name, obj->name, root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }


    sprintf(s, "debug/tables/values/%s.%s NE %s.txt",
      obj->table_name, obj->name, root->key);

    if (!flag) {
      debug("update [%s].\n", s);
      delete_column_value_as_row(s, obj_deleted);
    }
    else {
      debug("delete [%s].\n", s);
      unlink(s);
    }

  }
  else {
    debug("unkown value type.\n");
  }

  fe();
}


static int
delete_value_as_column(s_column_obj *obj)
{
  s_table_property *p;
  char *s;
  
  fs();

  p = get_table_property(obj->table_name);
  if (!p) return 0;

  update_column_tree(p->cl_tree, obj, "delete");

  s = gc_malloc(1024);
  if (!s) return;
  
  sprintf(s, "debug/tables/values/%s.txt",obj->table_name);
  
  debug("update [%s].\n", s);
  delete_column_value_as_row(s, obj);

  fe_return(1);
}


int
delete_column_node(char *table_name, char *column_name)
{
  s_bin_tree_node *nd;
  char *key;
  int rt;
  s_table_property *p;
  s_column_obj *o;
  
  fs();

  debug("column name: %s.\n", column_name);

  nd = binary_tree_isearch(SEARCH, &table_tree, table_name, 0, strcmpi);
  if (!nd) return 0;

  p = nd->val;

  key = column_name;
  nd = binary_tree_isearch(SEARCH, &p->cl_tree, key, strlen(key)+1, strcmpi);
  if (!nd) return 0;

  /*delete values.
   */
  o = nd->val;
  o->table_name = table_name;
  rt = delete_value_as_column(o);
  if (!rt) return 0;

  rt = binary_tree_delete(&p->cl_tree, nd);
  if (!rt) return 0;
    
  fe_return(1);
}

static s_bin_tree_node*
update_value_index_tree
(
  s_bin_tree_node *column_tree, s_column_obj *obj
)
{
  s_bin_tree_node *nd;
  char *key;
  s_column_obj *o;
  s_bin_tree_node **t;
  
  fs();

  key = obj->name;
  nd = binary_tree_isearch(SEARCH, &column_tree, key, 0, strcmpi);
  if (!nd) return NULL;

  o = nd->val;
  t = &o->val_tree;
  
  if (!strcmpi(obj->type, "char")) {

    debug("string value: [%s].\n", obj->str_val);
    key = obj->str_val;
    nd = binary_tree_isearch(INSERT, t, key, strlen(key) + 1, strcmpi);
    if (!nd) return NULL;
  }
  else if (!strcmpi(obj->type, "int")) {

    debug("int value: [%d].\n", obj->int_val);
    key = &obj->int_val;
    nd = binary_tree_isearch(INSERT, t, key, 4, int_cmp);
    if (!nd) return NULL;
  }
  else {

    debug("unkown value type. \n");
    return 0;
  }
  
  fe_return(nd);
}

static int
update_value_index_file
(
  char *tb_name, char *column_name, s_column_obj *obj)
{
  FILE *f;
  char *s;
  
  fs();

  s = gc_malloc(strlen("debug/tables/values/") + strlen(tb_name) + strlen(".") + 
    strlen(column_name) + strlen(".txt") + 1);
  if (!s) return;
  
  sprintf(s, "debug/tables/values/%s.%s.txt", tb_name, column_name);


  debug("open [%s]. \n", s);
  
  f = fopen(s, "a");
  if (!f) return 0;

  if (!strcmpi(obj->type, "char")) {
  
    fprintf(f, "%s\n", obj->str_val);
  }
  else if (!strcmpi(obj->type, "int")) {

    fprintf(f, "%d\n", obj->int_val);
  }
  else {
    fclose(f);
    return 0;
  }
  
  fclose(f);
  fe_return(1);
}

static int
update_str_value_file
(
  char *table_name, char *column_name, char *op, char *str, char *one_row
)
{
  char *fname;
  FILE *f;
  
  fs();

  fname = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".") + strlen(column_name) + strlen(op) + strlen(str) + 
      strlen(".txt") + 1);
      
  if (!fname) return 0;

  sprintf(fname,
    "debug/tables/values/%s.%s %s %s.txt", 
      table_name, column_name, op, str);

  debug("write one row [%s] into [%s]. \n", one_row, fname);

  f = fopen(fname, "a");
  if (!f) return 0;

  fprintf(f, "%s\n", one_row);

  fclose(f);
  fe_return(1);
}

static int
copy_str_value_file
(
  char *table_name, char *column_name, 
  char *op1, char *op2, char *val1, char *val2
)
{
  char *fname;
  FILE *f;
  char *buf;
  int rt;
  
  fs();
  
  fname = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".") + strlen(column_name) + strlen(op1) + strlen(val1) + 
      strlen(".txt") + 1);
  
  if (!fname) return 0;
  
  sprintf(fname,
    "debug/tables/values/%s.%s %s %s.txt", 
      table_name, column_name, op1, val1);
  
  f = fopen(fname, "r");
  if (!f) return 0;

  buf = gc_malloc(80);
  if (!buf) return 0;
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
    
    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;
    
    rt = update_str_value_file(table_name, column_name, op2, val2, buf);
    if (!rt) return 0;
  }
  
  fclose(f);
  fe_return(1);

}

static void
update_str_column_value_file
(
  char *table_name, s_bin_tree_node *root,
  s_column_obj *obj, char *one_row
)
{
  char *op;
  char *val;

  fs();
  
  if (!root) return;

  val = root->key;
  
  op = (strcmpi(val, obj->str_val) ? "NE" : "E");

  update_str_value_file(table_name, obj->name, op, val, one_row);

  if (strcmpi(val, obj->str_val)) {
    copy_str_value_file(table_name, obj->name, "E", "NE", val, obj->str_val);
  }

  root->left ?
    update_str_column_value_file(table_name, root->left, obj, one_row) : 0;

  root->right ?
    update_str_column_value_file(table_name, root->right, obj, one_row) : 0;

  fe();
}

static int
update_int_value_file
(
  char *table_name, char *column_name, char *op, int val, char *one_row
)
{
  char *fname;
  FILE *f;
  
  fs();
  
  fname = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".") + strlen(column_name) + strlen(op) + sizeof(val) + 
      strlen(".txt") + 1);
      
  if (!fname) return 0;
  
  sprintf(fname,
    "debug/tables/values/%s.%s %s %d.txt", 
      table_name, column_name, op, val);
  
  debug("write one row [%s] into [%s]. \n", one_row, fname);
  
  f = fopen(fname, "a");
  if (!f) return 0;
  
  fprintf(f, "%s\n", one_row);

  fclose(f);
  fe_return(1);
}

static int
copy_int_value_file
(
  char *table_name, char *column_name, 
  char *op1, char *op2, int val1, int val2
)
{
  char *fname;
  FILE *f;
  char *buf;
  int rt;
  
  fs();
  
  fname = gc_malloc(strlen("debug/tables/values/") + strlen(table_name) + 
    strlen(".") + strlen(column_name) + strlen(op1) + sizeof(val1) + 
      strlen(".txt") + 1);
  
  if (!fname) return 0;
  
  sprintf(fname,
    "debug/tables/values/%s.%s %s %d.txt", 
      table_name, column_name, op1, val1);
  
  f = fopen(fname, "r");
  if (!f) return 0;

  buf = gc_malloc(80);
  if (!buf) return 0;
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);

    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;

    rt = update_int_value_file(table_name, column_name, op2, val2, buf);
    if (!rt) return 0;
  }
  
  fclose(f);
  fe_return(1);

}

static void
update_int_column_value_file
(
  char *table_name, s_bin_tree_node *root,
  s_column_obj *obj, char *one_row
)
{
  char *op;
  int val;
  
  fs();
  
  if (!root) return;

  val = *(int*)root->key;
  debug("[%d]. \n", val);

  /* = */
  if (val == obj->int_val) {
    update_int_value_file(table_name, obj->name, "E", val, one_row);
    update_int_value_file(table_name, obj->name, "LE", val, one_row);
    update_int_value_file(table_name, obj->name, "GE", val, one_row);
  }

  /* != */
  if (val != obj->int_val) {
    update_int_value_file(table_name, obj->name, "NE", val, one_row);

    if (obj->is_new) {
      copy_int_value_file(table_name, obj->name, "E", "NE", val, obj->int_val);
    }
  }

  /* > */
  if (val > obj->int_val) {
    update_int_value_file(table_name, obj->name, "L", val, one_row);
    update_int_value_file(table_name, obj->name, "LE", val, one_row);
  }
  

  /* < */
  if (val < obj->int_val) {
    update_int_value_file(table_name, obj->name, "G", val, one_row);
    update_int_value_file(table_name, obj->name, "GE", val, one_row);
  }
  
  root->left ?
    update_int_column_value_file(table_name, root->left, obj, one_row) : 0;
  
  root->right ?
    update_int_column_value_file(table_name, root->right, obj, one_row) : 0;
  
  fe();
}


static s_bin_tree_node*
search_min_greater_node(s_bin_tree_node *root, int val)
{
  s_bin_tree_node *nd, *rt;
  
  fs();

  if (!root) return NULL;
  
  nd = binary_tree_isearch(SEARCH, &root, &val, 0, int_cmp);
  if (!nd) return NULL;

  debug("[%d].\n", *(int*)nd->key);
  
  nd->left ? 0: debug("left null. \n");
  nd->right ? 0: debug("right null. \n");

  if (!nd->right) {
    nd->father ? debug("father: [%d].\n", *(int*)nd->father->key) : 0;
   
    rt = NULL;
    if (nd->father) {
      rt = (nd->father->right == nd ? nd->father->father : NULL);
      rt = (nd->father->left == nd ? nd->father : NULL);
    }
  }
  else {

    rt = nd = nd->right;
    while (nd) {
      debug("[%d].\n", *(int*)nd->key);
      
      rt = nd;
      nd = nd->left;
    }
  }
  
  fe_return(rt);
}

static s_bin_tree_node*
search_max_lesser_node(s_bin_tree_node *root, int val)
{
  s_bin_tree_node *nd, *rt;
  
  fs();

  if (!root) return NULL;
  
  nd = binary_tree_isearch(SEARCH, &root, &val, 0, int_cmp);
  if (!nd) return NULL;

  debug("[%d].\n", *(int*)nd->key);
  
  nd->left ? 0: debug("left null. \n");
  nd->right ? 0: debug("right null. \n");

  if (!nd->left) {

    rt = NULL;
    if (nd->father) {
      rt = (nd->father->right == nd ? nd->father : NULL);
    }    
  }
  else {
  
    rt = nd = nd->left;
    while (nd) {
      debug("[%d].\n", *(int*)nd->key);
      
      rt = nd;
      nd = nd->right;
    }
  }
  
  fe_return(rt);
}


static int
update_value_file
(
  char *table_name, s_bin_tree_node *column_tree,
  s_column_obj *obj, char *one_row
)
{
  s_bin_tree_node *nd;
  char *key;
  s_column_obj *o;
  int rt;
  int val;
  
  fs();
  
  key = obj->name;
  nd = binary_tree_isearch(SEARCH, &column_tree, key, 0, strcmpi);
  if (!nd) return NULL;

  o = nd->val;
  
  if (!strcmpi(obj->type, "char")) {
    update_str_column_value_file(table_name, o->val_tree, obj, one_row);
  }
  else if (!strcmpi(obj->type, "int")) {
  
    update_int_column_value_file(table_name, o->val_tree, obj, one_row);

    if (!obj->is_new) goto END;
    
    nd = search_min_greater_node(o->val_tree, obj->int_val);
    if (nd) {
  
      val = *(int*)nd->key;
      debug("min-greater-val: [%d].", val);
  
      copy_int_value_file(table_name, obj->name, "GE", "GE", val, obj->int_val);
      copy_int_value_file(table_name, obj->name, "GE", "G", val, obj->int_val);
    }
  
    nd = search_max_lesser_node(o->val_tree, obj->int_val);
    if (nd) {
      val = *(int*)nd->key;
      debug("max-lesser-val: [%d].", val);
  
      copy_int_value_file(table_name, obj->name, "LE", "LE", val, obj->int_val);
      copy_int_value_file(table_name, obj->name, "LE", "L", val, obj->int_val);
    }     
    
  }
  else {
    debug("unkown value type. \n");
    return 0;
  }

END:  
  fe_return(1);
}

int
insert_column_value
(
  s_table_property *p, s_column_obj *obj, char *one_row
)
{
  s_bin_tree_node *column_tree;
  s_bin_tree_node *nd;
  int rt;
  
  fs();

  column_tree = p->cl_tree;

  nd = update_value_index_tree(column_tree, obj);
  if (nd) {

    rt = update_value_index_file(p->name, obj->name, obj);
    if (!rt) return 0;
  }

  obj->is_new = !!nd;

  obj->is_new ? debug("new value. \n") : 0;

  rt = update_value_file(p->name, column_tree, obj, one_row);
  if (!rt) return 0;
  
  fe_return(1);
}

static s_bin_tree_node*
search_min_greater(s_bin_tree_node *root, int val)
{
  s_bin_tree_node *nd, *nd1;
  int v;

  fs();
  
  if (!root) return NULL;

  nd = root;
  while (nd) {
    if (!nd->left) break;
    nd = nd->left;
  }

  if (!nd) return NULL;
  
  v = *(int*)nd->key;
  debug("left leaf [%d].\n", v);

  while (nd) {

    v = *(int*)nd->key;
    debug("[%d].\n", v);

    if (v > val) {
      debug("found.\n");
      break;
    }
    
    if (nd->right) {
      debug("try right.\n");
      
      nd1 = search_min_greater(nd->right, val);
      if (nd1) {
        nd = nd1;
        break;
      }
      
      debug("try right not found.\n");
    }
    
    if (nd == root) {
      debug("root.\n");
      return NULL;
    }
    
    nd = nd->father;
    debug("try father.\n");
  }

  if (!nd) return NULL;

  fe_return(nd);
}

static s_bin_tree_node*
search_max_lesser(s_bin_tree_node *root, int val)
{
  s_bin_tree_node *nd, *nd1;
  int v;

  fs();
  
  if (!root) return NULL;

  nd = root;
  while (nd) {
    if (!nd->right) break;
    nd = nd->right;
  }

  if (!nd) {
    nd = root;
  }
  else {
    v = *(int*)nd->key;
    debug("right leaf [%d].\n", v);
  }
  
  while (nd) {

    v = *(int*)nd->key;
    debug("[%d].\n", v);

    if (v < val) {
      debug("found.\n");
      break;
    }
    
    if (nd->left) {
      debug("try left.\n");
      
      nd1 = search_max_lesser(nd->left, val);
      if (nd1) {
        nd = nd1;
        break;
      }
      
      debug("try left not found.\n");
    }
    
    if (nd == root) {
      debug("root.\n");
      return NULL;
    }
    
    nd = nd->father;
    debug("try father.\n");
  }

  if (!nd) return NULL;

  fe_return(nd);
}


static s_bin_tree_node*
search_numeric_value(s_bin_tree_node *root, s_value_term *t)
{
  s_bin_tree_node *nd;
  
  fs();

  if (!root) return NULL;

  debug("op [%s], val [%d].\n", t->op, t->obj.int_val);

  if (!strcmpi(t->op, "L") || !strcmpi(t->op, "LE")) {
    nd = search_min_greater(root, t->obj.int_val);
    if (!nd) return NULL;
    t->op = "L";
    t->obj.int_val = *(int*)nd->key;
  }
  else if (!strcmpi(t->op, "G") || !strcmpi(t->op, "GE")) {
    nd = search_min_greater(root, t->obj.int_val);
    if (!nd) return NULL;
    t->op = "GE";
    t->obj.int_val = *(int*)nd->key;
  }
  else {
    return NULL;
  }

  nd ? debug("a nearest value was found [%d].\n", *(int*)nd->key) : 0;

  fe_return(nd);
}

s_bin_tree_node*
get_column_node(s_bin_tree_node **t, char *column_name)
{
  s_bin_tree_node *nd;
  
  fs();

  debug("column name: [%s].\n", column_name);
  
  nd = binary_tree_isearch(SEARCH, t, column_name, 0, strcmpi);

  fe_return(nd);
}

static s_bin_tree_node*
get_value_tree(s_bin_tree_node **column_tree, char *column_name)
{
  s_bin_tree_node *nd;
  s_column_obj *o;
  
  fs();
  
  nd = get_column_node(column_tree, column_name);
  if (!nd) return NULL;

  o = nd->val;
  if (!o) return NULL;

  fe_return(o->val_tree);
}

int
search_value(char *table_name, s_value_term *t)
{
  s_bin_tree_node *nd;
  s_table_property *p;
  s_column_obj *o;
  s_bin_tree_node *val_tree;
  char fname[255];
  char *s;
  FILE *f;
  int rt;
  
  fs();

  if (!table_name) return 0;
  debug("table name [%s].\n", table_name);
  
  memset(fname, 0, sizeof(fname));

  nd = get_table_node(table_name);
  if (!nd) return 0;

  p = nd->val;

  nd = get_column_node(&p->cl_tree, t->obj.name);
  if (!nd) return 0;

  o = nd->val;
  val_tree = o->val_tree;

  debug("op [%s].\n", t->op);

  s = fname;
  s += sprintf(s, "debug/tables/values/%s.%s %s ",
    table_name, t->obj.name, t->op);
  
  debug("value type [%s].\n", t->obj.type);
  
  if (!strcmpi(t->obj.type, "char")) {
    s += sprintf(s, "%s.txt", t->obj.str_val);
  }
  else if (!strcmpi(t->obj.type, "int")) {
    s += sprintf(s, "%d.txt", t->obj.int_val); 
  }
  else {
    return 0;
  }

  
  f = fopen(fname, "r");
  if (f) {
  
    debug("value file [%s] was found.\n", fname);
    fclose(f);

    t->r = fname;
    
    fe_return(1);
  }
  debug("value file [%s] was not found.\n", fname);

  if (strcmpi(t->obj.type, "int")) return 0;

  nd = search_numeric_value(val_tree, t);
  if (!nd) {

    if (strcmpi(t->op, "NE")) return 0;
    
    t->op = "E";
    
    s = fname;
    s += sprintf(s, "debug/tables/values/%s.txt",table_name);
  
    f = fopen(fname, "r");
    if (f) {
    
      debug("value file [%s] was found.\n", fname);
      fclose(f);
      
      t->r = fname;
      
      fe_return(1);
    }
    debug("value file [%s] was not found.\n", fname);
  
    return 0;
  }
  
  
  rt = search_value(table_name, t);
  
  fe_return(rt);
}

static s_query_info_list*
match_query_table(s_query_info_list *lst, char *table_name)
{
  s_query_info_list *q;
  
  fs();

  debug("table name [%s].\n", table_name);
  
  lst = lst->next;
  while (lst) {
    if (!strcmpi(lst->p->name, table_name)) fe_return(lst);
    lst = lst->next;
  }

  return NULL;
}

static char*
get_date_type(s_table_property *p, char *column_name)
{
  s_column_obj_list *lst;
  
  fs();

  lst = p->cl_obj_lst;
  while (lst) {

    if (!strcmpi(lst->obj->name, column_name)) {

      lst->obj->type ? debug("[%s].\n", lst->obj->type) : 0;
      
      fe_return(lst->obj->type);
    }
    
    lst = lst->next;
  }

  return NULL;
}

static int
match_data_type(char *lv, char *rv)
{
  fs();

  if (strcmpi(lv, rv)) return 0;

  fe_return(1);
}

int
copy_file(char *src, char *dst)
{
  FILE *f, *f1;
  char *buf;
  
  fs();

  f = fopen(src, "r");
  if (!f) return 0;
  debug("[%s] was opened.\n", src);


  f1 = fopen(dst, "a");
  if (!f1) return 0;
  debug("[%s] was opened.\n", dst);

  
  buf = gc_malloc(80);
  if (!buf) return 0;
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
  
    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;
  
    debug("copy @row: [%s].\n", buf);
  
    fprintf(f1, "%s\n", buf);
  }
  
  fclose(f);  
  fclose(f1);

  fe_return(1);
}

int
copy_file_and_update(char *src, char *dst, char *update_str)
{
  FILE *f, *f1;
  char *buf;
  
  fs();

  f = fopen(src, "r");
  if (!f) return 0;
  debug("[%s] was opened.\n", src);


  f1 = fopen(dst, "a");
  if (!f1) return 0;
  debug("[%s] was opened.\n", dst);

  
  buf = gc_malloc(80);
  if (!buf) return 0;
  
  while (!feof(f)) {
  
    memset(buf, 0, 80);
    fgets(buf, 80, f);
  
    buf[strlen(buf)-1] == '\n' ?
      buf[strlen(buf)-1] = '\0' : 0;
    
    if (!strlen(buf)) continue;
  
    debug("copy @row: [%s].\n", buf);
  
    fprintf(f1, "%s%s\n", buf, update_str);
  }
  
  fclose(f);  
  fclose(f1);

  fe_return(1);
}



static char*
make_file_path
(
  char *dir, char *table_name, char *column_name, char *op, char *val
)
{
  char *fpath;
  
  fs();

  if (!dir || !table_name || !column_name || !op || !val) return NULL;
  
  fpath = gc_malloc(strlen(dir) + strlen(table_name) + strlen(".") +
    strlen(column_name) + strlen(op) + strlen(val) + strlen(".txt"));

  if(!fpath) return NULL;

  sprintf(fpath, "%s%s.%s %s %s.txt", dir, table_name, column_name, op, val);

  debug("[%s].\n", fpath);
  
  fe_return(fpath);
}

static void
search_equal_value
(
  s_bin_tree_node *tr1, s_bin_tree_node *tr2, 
  s_value_term *t, char *r1, char *r2
)
{
  s_bin_tree_node *nd;
  char *r0;
  char *s;
  int val;
  
  if (!tr1 || !tr2) return;
  
  if (!strcmpi(t->obj.type, "int")) {

    nd = binary_tree_isearch(SEARCH, &tr2, tr1->key, 0, int_cmp);
    if (nd) {

      val = *(int*)tr1->key;
      
      debug("[%d] value matched.\n", val);
     
      r0 = gc_malloc(1024);
      if (!r0) return;

      sprintf(r0, "%s%s.%s %s %d.txt", "debug/tables/values/", 
        t->obj.table_name, t->obj.name, "E", val);
      
      copy_file(r0, r1);


      sprintf(r0, "%s%s.%s %s %d.txt", "debug/tables/values/", 
        t->obj1.table_name, t->obj1.name, "E", val);

      copy_file(r0, r2);
    }

  }
  else if (!strcmpi(t->obj.type, "char")) {

    nd = binary_tree_isearch(SEARCH, &tr2, tr1->key, 0, strcmpi);
    if (nd) {

      debug("[%s] value matched.\n", tr1->key);
      
      r0 = make_file_path("debug/tables/values/", t->obj.table_name, 
        t->obj.name, t->op, nd->key);

      if (!r0) return;

      copy_file(r0, r1);

      r0 = make_file_path("debug/tables/values/", t->obj1.table_name, 
        t->obj1.name, t->op, nd->key);

      if (!r0) return;
      
      copy_file(r0, r2);
    }
  }
  else {
    return;
  }

  search_equal_value(tr1->left, tr2, t, r1, r2);
  search_equal_value(tr1->right, tr2, t, r1, r2);

}

static void
search_not_equal_value
(
  s_bin_tree_node *tr1, s_bin_tree_node *tr2, 
  s_value_term *t, char *r1, char *r2
)
{
  s_bin_tree_node *nd;
  char *r0;
  char *s;
  int val;

  if (!tr1 || !tr2) return;
  
  if (!strcmpi(t->obj.type, "int")) {

    val = *(int*)tr1->key;
    
    nd = binary_tree_isearch(SEARCH, &tr2, tr1->key, 0, int_cmp);
    if (!nd) {
    
      debug("[%d] value matched.\n", val);
     
      r0 = gc_malloc(1024);
      if (!r0) return;

      sprintf(r0, "%s%s.%s %s %d.txt", "debug/tables/values/", 
        t->obj.table_name, t->obj.name, "E", val);
      
      copy_file(r0, r1);
    }
    else {

      debug("[%d] value was marked.\n", val);
      nd->mark = Y;
    }
  }
  else if (!strcmpi(t->obj.type, "char")) {

    nd = binary_tree_isearch(SEARCH, &tr2, tr1->key, 0, strcmpi);
    if (!nd) {

      debug("[%s] value matched.\n", tr1->key);
    
      r0 = make_file_path("debug/tables/values/", t->obj.table_name, 
        t->obj.name, "E", tr1->key);

      if (!r0) return;

      copy_file(r0, r1);   
    }
    else {

      debug("[%s] value was marked.\n", tr1->key);
      nd->mark = Y;
    }
  }
  else {
    return;
  }

  search_not_equal_value(tr1->left, tr2, t, r1, r2);
  search_not_equal_value(tr1->right, tr2, t, r1, r2);

}

static void
search_unmarked_value(s_bin_tree_node *root, s_value_term *t, char *r)
{
  char *r0;
  int val;
  
  if (!root || !t || !r) return;

  fs();
  
  search_unmarked_value(root->left, t, r);
  search_unmarked_value(root->right, t, r);
  
  if (!root->mark) {
    
    if (!strcmpi(t->obj1.type, "int")) {
  
      val = *(int*)root->key;
      
      debug("unmarked [%d] value matched.\n", val);
     
      r0 = gc_malloc(1024);
      if (!r0) return;
  
      sprintf(r0, "%s%s.%s %s %d.txt", "debug/tables/values/", 
        t->obj1.table_name, t->obj1.name, "E", val);
      
      copy_file(r0, r);
    }
    else if (!strcmpi(t->obj1.type, "char")) {
  
      debug("unmarked [%s] value matched.\n", root->key);
    
      r0 = make_file_path("debug/tables/values/", t->obj1.table_name, 
        t->obj1.name, "E", root->key);
  
      if (!r0) return;
  
      copy_file(r0, r);
    }
    else {
      return;
    }
  }
  else {
    root->mark = !Y;
  }
}

int
search_value_1(s_query_info_list *lst, s_value_term *t)
{
  s_query_info_list *q1, *q2;
  char *tp1, *tp2;
  s_bin_tree_node *tr1, *tr2;
  char *r1, *r2;
  
  fs();

  q1 = match_query_table(lst, t->obj.table_name);
  if (!q1) return 0;

  q2 = match_query_table(lst, t->obj1.table_name);
  if (!q2) return 0;

  tp1 = get_date_type(q1->p, t->obj.name);
  if (!tp1) return 0;

  tp2 = get_date_type(q1->p, t->obj1.name);
  if (!tp2) return 0;  

  if (!match_data_type(tp1, tp2)) return 0;

  tr1 = get_value_tree(&q1->p->cl_tree, t->obj.name);
  if (!tr1) return 0;

  tr2 = get_value_tree(&q2->p->cl_tree, t->obj1.name);
  if (!tr2) return 0;

  debug("op [%s].\n", t->op);

  t->obj.type = tp1;
  t->obj1.type = tp2;


  r1 = gc_malloc(1024);
  if (!r1) return 0;
  
  r2 = gc_malloc(1024);
  if (!r2) return 0;

  sprintf(r1, "(%s.%s %s %s.%s).%s.%s.txt", t->obj.table_name, t->obj.name,
    t->op, t->obj1.table_name, t->obj1.name,
    t->obj.table_name, t->obj.name);

  sprintf(r2, "(%s.%s %s %s.%s).%s.%s.txt", t->obj.table_name, t->obj.name,
    t->op, t->obj1.table_name, t->obj1.name,
    t->obj1.table_name, t->obj1.name);
  
  unlink(r1);
  unlink(r2);

  if (!strcmpi(t->op, "E")) {

    search_equal_value(tr1, tr2, t, r1, r2);
  }
  else if (!strcmpi(t->op, "NE")) {
  
    search_not_equal_value(tr1, tr2, t, r1, r2);

    search_unmarked_value(tr2, t, r2);
  }
  else if (!strcmpi(t->op, "G")) {
    
  }
  else if (!strcmpi(t->op, "GE")) {
    
  }
  else if (!strcmpi(t->op, "L")) {
    
  }
  else if (!strcmpi(t->op, "LE")) {
    
  }  
  else {
    return 0;
  }

  t->r = r1;
  t->r1 = r2;

  fe_return(1);
}



