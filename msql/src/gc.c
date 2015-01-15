

/*
*memory garbage collector.
*/
 

#include "gc.h"
#include "common.h"
#include "bin_tree.h"


/*
*@gc_id => @gc_node(@id, @mm_tree) => @gc_tree,
*@mm_tree => @mm_node(@mm_id, @mark).
*/

static int gc_id;
static s_bin_tree_node *gc_tree;
static int cur_gc_id;

int
gc(void)
{
  s_bin_tree_node *nd;
  
  fs();

  gc_id++;

  nd = binary_tree_isearch(INSERT, &gc_tree, &gc_id, sizeof(int), int_cmp);
  if (!nd) return -1;
  
  debug("gc_id [%d].\n", gc_id);
  
  fe_return(gc_id);
}

int
set_gc(int id)
{
  fs();

  if (id > gc_id) return 0;

  cur_gc_id = id;

  debug("cur_gc_id [%d].\n", cur_gc_id);

  fe_return(1);
}

void*
gc_malloc(long size)
{
  void *p;
  s_bin_tree_node *nd, *root;
  int mm_id;

  fs();

  debug("cur_gc_id [%d].\n", cur_gc_id);
  nd = binary_tree_isearch(SEARCH, &gc_tree, &cur_gc_id, 0, int_cmp);
  if (!nd) return NULL;
  
  p = malloc(size);
  if (!p) return NULL;
  memset(p, 0, size);

  mm_id = (int)p;
  nd = binary_tree_isearch(INSERT, &nd->val, &mm_id, sizeof(int), int_cmp);
  if (!nd) return NULL;

  debug("malloc [%x].\n", mm_id);
  
  fe_return(p);
}

static void
free_mm_tree(s_bin_tree_node *root)
{
  int mm_id;
  
  if (!root) return;

  root->left ? free_mm_tree(root->left) : 0;
  root->right ? free_mm_tree(root->right) : 0;

  mm_id = *(int*)root->key;
  
  debug("free [%x].\n", mm_id);
  free(mm_id);
  
  binary_tree_delete(&root, root);
}

void
gc_free(void)
{
  s_bin_tree_node *nd;
  
  fs();

  nd = binary_tree_isearch(SEARCH, &gc_tree, &cur_gc_id, 0, int_cmp);
  if (!nd) return NULL;

  free_mm_tree(nd->val);

  binary_tree_delete(&gc_tree, nd);

  cur_gc_id = gc_id;
  
  fe();
}

int
dbg_gc(void)
{
  fs();

  fe_return(1);
}


