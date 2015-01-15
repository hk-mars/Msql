

#ifndef __HSEARCH_H__
#define __HSEARCH_H__

/* Compare function */
typedef int (*__compar_fn_t) (const void *, const void *);

/* Action which shall be performed in the call the hsearch.  */
typedef 
enum 
{
  FIND,
  ENTER
}
ACTION;

typedef 
struct entry 
{
  char *key;
  void *data;
  int dt_sz;
}
ENTRY;

/* Opaque type for internal use.  */
struct _ENTRY;

/* Data type for reentrant functions.  */
typedef 
struct hsearch_data 
{
  struct _ENTRY *table;
  unsigned int size;
  unsigned int filled;
}
HTAB_INFO;

/* Family of hash table handling functions.  These non-reentrant
   functions all work on a signle internal hashing table.  */

/* Search for entry matching ITEM.key in internal hash table.  If
   ACTION is `FIND' return found entry or signal error by returning
   NULL.  If ACTION is `ENTER' replace existing data (if any) with
   ITEM.data.  */
ENTRY *hsearch (struct hsearch_data *htab, ENTRY item, ACTION action);

/* Create a new hashing table which will at most contain NEL elements.  */
int hcreate (struct hsearch_data *htab, long nel);

/* Destroy current internal hashing table.  */
void hdestroy (struct hsearch_data *htab);

#endif /* __HSEARCH_H__ */


