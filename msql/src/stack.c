
#include "stack.h"
#include "common.h"
#include "syntax.h"
#include "gc.h"
#include "executor.h"


static token_list*
push_stack_of_search_condition
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_query_expression
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_non_join_query_expression
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_value_expression
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_set_quantifier
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_column_reference
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_qualifier
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_column_name_list
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_qualified_name
(
  token_list *lst, void *st
);

static token_list*
push_stack_of_unsigned_integer
(
  token_list *lst, void *st
);


#define next_token(lst) \
{ \
  if (!lst->next) return NULL; \
  lst = lst->next; \
} \

static int
match(token_list *lst, char *key)
{
  fs();
  
  if  (!lst) return NULL;

  debug("%s \n", key);
  if (strcmpi(lst->tk.key, key)) return 0;

  fe();
  return 1;
}

static int
match2(token_list *lst, char *key1, char *key2)
{
  if  (!lst || !lst->next) return NULL;

  return (match(lst, key1) && match(lst->next, key2));
}

static int
match3(token_list *lst, char *key1, char *key2, char *key3)
{
  if  (!lst || !lst->next || !lst->next->next) return NULL;

  return (match(lst, key1) && match2(lst->next, key2, key3));
}

static token_list*
push_stack_of_column_name
(
  token_list *lst, void *st
)
{
  s_column_name *cname = (s_column_name *)st;
  
  fs();

  if (!match(lst, "<regular identifier>")) return NULL;
  
  *cname = lst->tk.value;
  debug("column name: %s \n", *cname);

  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_length
(
  token_list *lst, void *st
)
{
  s_length *len = (s_length*)st;
  
  fs();

  if (!match(lst, "<unsigned integer>")) return NULL;
  
  *len = atoi(lst->tk.value);
  debug("length: %d \n", *len);

  fe_return(lst);
}

static token_list*
push_stack_of_character_string_type
(
  token_list *lst, void *st
)
{
  s_character_string_type *t = (s_character_string_type *)st;
  token_list *s;

  fs();
  
  if (match(lst, "CHARACTER")) {
    t->is_character_type = Y;

    if (!match(lst->next, "<left paren>")) fe_return(lst);
    next_token(lst); next_token(lst);
  }
  else if (match(lst, "CHAR")) {
    t->is_char_type = Y;
    
    if (!match(lst->next, "<left paren>")) fe_return(lst);
    next_token(lst); next_token(lst);
  }
  else if (match2(lst, "CHARACTER", "VARYING")) {
    t->is_character_varying_type = Y;
    next_token(lst); next_token(lst);

    if (!match(lst, "<left paren>")) return NULL;
    next_token(lst);		    
  }
  else if (match2(lst, "CHAR", "VARYING")) {
    t->is_char_varying_type = Y;
    next_token(lst); next_token(lst);

    if (!match(lst, "<left paren>")) return NULL;
    next_token(lst);		
  }
  else if (match(lst, "VARCHAR")) {
    t->is_varchar_type = Y;
    next_token(lst);

    if (!match(lst, "<left paren>")) return NULL;
    next_token(lst);    
  }
  else {
    return NULL;
  }

  s = push_stack_of_length(lst, &t->length);
  if (!s) return NULL;
  t->has_length = Y;
  
  if (!match(s->next, "<right paren>")) return NULL;
  next_token(s);
  
  fe_return(s);
}

static token_list*
push_stack_of_character_set_specification
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_national_character_string_type
(
  token_list *lst, void *st
)
{
  s_national_character_string_type *type = st;
  token_list *s;
  
  if (match2(lst, "NATIONAL", "CHARACTER")) {
    type->is_national_character_type = Y;
    if (match(lst->next->next, "<left paren>"))lst = lst->next->next->next;
  }
  else if (match2(lst, "NATIONAL", "CHAR_T")) {
    type->is_national_char_type = Y;
    if (match(lst->next->next, "<left paren>")) lst = lst->next->next->next;
  }
  else if (match3(lst, "NATIONAL", "CHARACTER", "VARYING")) {
    type->is_national_character_varying_type = Y;
    lst = lst->next->next->next->next;
  }
  else if (match3(lst, "NATIONAL", "CHAR", "VARYING")) {
    type->is_national_char_varying_type = Y;
    lst = lst->next->next->next->next;
  }
  else if (match2(lst, "NATIONAL", "VARCHAR")) {
    type->is_national_varchar_type = Y;
    lst = lst->next->next->next;
  }
  else {
    return NULL;
  }

  s = push_stack_of_length(lst, &type->length);
  if (s) {
    type->has_length = Y;
    lst = lst->next;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_bit_string_type
(
  token_list *lst, void *st
)
{
  s_bit_string_type *btype = (s_bit_string_type *)st;
  token_list *s;
  
  if (match(lst, "BIT")) {
    btype->type = BIT_T;
    if (match(lst->next, "<left paren>")) lst = lst->next->next;
  }
  else if (match2(lst, "BIT", "VARYING")) {
    btype->type = BIT_VARYING_T;
    lst = lst->next->next;
  }
  else {
    return NULL;
  }

  s = push_stack_of_length(lst, &btype->length);
  if (s) {
    btype->has_length = Y;
    lst = lst->next;
    fe_return(s);
  }  

  return NULL;
}

static token_list*
push_stack_of_precision
(
  token_list *lst, void *st
)
{
  precision_t *pr = (precision_t *)st;
  
  *pr = atoi(lst->tk.value);
  fe_return(lst);
}

static token_list*
push_stack_of_scale
(
  token_list *lst, void *st
)
{
  scale_t *sc = (scale_t *)st;
  
  *sc = atoi(lst->tk.value);
  fe_return(lst);
}

static token_list*
push_stack_of_exact_numeric_type
(
  token_list *lst, void *st
)
{
  s_exact_numeric_type *type = (s_exact_numeric_type *)st;
  token_list *s;

  if (match(lst, "DEC")) {
    type->is_dec_type = Y;
    debug("dec_type.\n");
  }
  else if (match(lst, "INT")) {
    type->is_int_type = Y;
    debug("int_type.\n");
    fe_return(lst);
  }
  else {
    return NULL;
  }

  if (match(lst->next, "<left paren>")) {
  
    s = push_stack_of_precision(lst->next->next, &type->precision);
    if (!s) return NULL;
    type->has_precision = Y;
    
    lst = s;
    if (match(lst->next, "<comma>")) {
    
      s = push_stack_of_scale(lst->next, &type->scale);
      if (!s) return NULL;
      type->has_scale = Y;
      lst = s;
    }

    if (!match(lst->next, "<right paren>")) return NULL;
    next_token(lst);
  }

  fe_return(lst);
}

static token_list*
push_stack_of_approximate_numeric_type
(
  token_list *lst, void *st
)
{
  s_approximate_numeric_type *type = (s_approximate_numeric_type *)st;
  token_list *s;

  if (match(lst, "FLOAT")) {
    type->is_float_type = Y;

    if (match(lst->next, "<left paren>")) {
    
    	s = push_stack_of_precision(lst->next->next, &type->precision);
    	if (!s) return NULL;
    	type->has_precision = Y;
    	return s->next;
    }
  }
  else if (match(lst, "REAL")) {
    type->is_real_type = Y;
  }
  else if (match(lst, "DOUBLE")) {
    type->is_double_type = Y;
  }
  else {
    return NULL;
  }

  fe_return(lst);
}

static token_list*
push_stack_of_numeric_type
(
  token_list *lst, void *st
)
{
  s_numeric_type *ntype = (s_numeric_type *)st;
  token_list *s;

  s = push_stack_of_exact_numeric_type(lst, &ntype->exact_num_type);
  if (s) {
    ntype->is_exact_numeric_type = Y;
    fe_return(s);
  }

  s = push_stack_of_approximate_numeric_type(lst, &ntype->appr_num_type);
  if (s) {
    ntype->is_appro_numeric_type = Y;
    fe_return(s);
  }
  
  return NULL;
}

static token_list*
push_stack_of_time_fractional_second_precision
(
  token_list *lst, void *st
)
{
  time_fractional_seconds_precision_t *tfsp;

  tfsp = (time_fractional_seconds_precision_t *)st;
  
  *tfsp = atoi(lst->tk.value);
  fe_return(lst);
}

static token_list*
push_stack_of_time_precision
(
  token_list *lst, void *st
)
{
  time_precision_t *tpr = (time_precision_t *)st;
  
  lst = push_stack_of_time_fractional_second_precision(lst, tpr);
  fe_return(lst);
}

static token_list*
push_stack_of_timestamp_precision
(
  token_list *lst, void *st
)
{
  timestamp_precision_t *tspr = (  timestamp_precision_t *)st;
    
  lst = push_stack_of_time_fractional_second_precision(lst, tspr);
  fe_return(lst);
}


static token_list*
push_stack_of_datetime_type
(
  token_list *lst, void *st
)
{
  s_datetime_type *type = (s_datetime_type *)st;
  token_list *s;

  if (match(lst, "DATE")) {
  
    type->is_date_type = Y;
  }
  else if (lst, "TIME") {
  
    type->is_time_type= Y;
    
    if (match(lst->next, "left paren")) {
      s = push_stack_of_time_precision(lst->next->next, 
      &type->tm_precision);
      if (!s) return NULL;
      type->has_time_precision = Y;

      lst = s->next;
      if (match3(lst->next, "WITH", "TIME", "ZONE")) {
        type->has_time_zone = Y;
        lst = lst->next->next->next->next;
      }
    }
  }
  else if (match(lst, "TIMESTAMP")) {

    type->is_timestamp_type = Y;
    
    if (match(lst->next, "left paren")) {
      s = push_stack_of_timestamp_precision(lst->next->next, 
      &type->tmst_precision);
      if (!s) return NULL;
      type->has_timestamp_precision = Y;

      lst = s->next;
      if (match3(lst->next, "WITH", "TIME", "ZONE")) {
        type->has_time_zone = Y;
        lst = lst->next->next->next->next;
      }
    }
  }
  else {
    return NULL;
  }

  fe_return(lst);
}

static token_list*
push_stack_of_non_second_datetime_field
(
  token_list *lst, void *st
)
{
  non_second_datetime_field_t *nsdf = (non_second_datetime_field_t *)st;
  
  if (match(lst, "YEAR")) {
    *nsdf = YEAR_T;
  }
  else if (match(lst, "MONTH")) {
    *nsdf = MONTH_T;
  }
  else if (match(lst, "DAY")) {
    *nsdf = DAY_T;
  }
  else if (match(lst, "HOUR")) {
    *nsdf = HOUR_T;
  }
  else if (match(lst, "MINUTE")) {
    *nsdf = MINUTE_T;
  }
  else {
    return NULL;
  }

  fe_return(lst);
}

static token_list*
push_stack_of_interval_leading_field_precision
(
  token_list *lst, void *st
)
{
  interval_leading_field_precision_t *ilfp = st;
  
  *ilfp = atoi(lst->tk.value);
  fe_return(lst);
}

static token_list*
push_stack_of_start_field
(
  token_list *lst, void *st
)
{
  s_start_field *sf = (s_start_field *)st;
  
  token_list *s;

  s = push_stack_of_non_second_datetime_field(lst, &sf->ns_dt_field);
  if (!s) return NULL;

  if (match(s, "<left paren>")) {
    lst = s->next;

    s = push_stack_of_interval_leading_field_precision(lst, 
    &sf->inter_ld_field_precision);
    if (!s) return NULL;

    return s->next;
  }

  fe_return(s);
}

static token_list*
push_stack_of_interval_fs_field_precision
(
  token_list *lst, void *st
)
{
  interval_fractional_seconds_precision_t *ifsp;

  ifsp = (interval_fractional_seconds_precision_t *)st;
  
  *ifsp = atoi(lst->tk.value);
  fe_return(lst);
}


static token_list*
push_stack_of_end_field
(
  token_list *lst, void *st
)
{
  s_end_field *ef = (s_end_field *)st;
  token_list *s;

  s = push_stack_of_non_second_datetime_field(lst,  &ef->ns_dt_field);
  if (s) {
    ef->is_non_second_datetime_field = Y;
    fe_return(lst);
  }

  if (match(lst, "SECOND")) {

    if (match(lst->next, "<left paren>")) {

      lst = lst->next;
      
      s = push_stack_of_interval_fs_field_precision(lst, &ef->i_fr_sc_precision);
      if (!s) return NULL;
      return s->next;
    }
  }

  return NULL;
}

static token_list*
push_stack_of_single_datetime_field
(
  token_list *lst, void *st
)
{
  s_single_datetime_field *sdf = (s_single_datetime_field *)st;
  token_list *s;

  s = push_stack_of_non_second_datetime_field(lst, &sdf->ns_dt_field);
  if (s) {
    if (match(s, "<left paren>")) {
      lst = s->next;
      
      s = push_stack_of_interval_leading_field_precision(lst, 
      &sdf->inter_ld_field_precision);
      if (!s) return NULL;
      
      return s->next;
    }
    fe_return(s);
  }

  if (match(lst, "SECOND")) {
  
  	if (match(lst->next, "<left paren>")) {
  
      lst = lst->next;

      s = push_stack_of_interval_leading_field_precision(lst, 
      &sdf->inter_ld_field_precision);
      if (!s) return NULL;

      sdf->has_interval_leading_field_precision = Y;

      if (!match(s->next, "<comma>")) {
        return s->next;
      }
      lst = s->next;
      
      s = push_stack_of_interval_fs_field_precision(lst->next, 
      &sdf->inter_fr_sc_precision);
      if (!s) return NULL;
      
      return s->next;
  	}
  	fe_return(lst);
  }

  return NULL;
}

static token_list*
push_stack_of_interval_qualifier
(
  token_list *lst, void *st
)
{
  s_interval_qualifier *iq = (s_interval_qualifier *)st;
  token_list *s;

  s = push_stack_of_start_field(lst, &iq->st_field);
  if (s) {
  
    lst = s->next;
    if (!match(lst, "TO")) return NULL;
    
    s = push_stack_of_end_field(lst->next, &iq->en_field);
    fe_return(s);
  }

  s = push_stack_of_single_datetime_field(lst, &iq->sg_dt_field);
  if (!s) return NULL;

  fe_return(s);
}

static token_list*
push_stack_of_interval_type
(
  token_list *lst, void *st
)
{
  s_interval_type *itype = (s_interval_type *)st;
  token_list *s;
  
  if (!match(lst, "INTERVAL")) return NULL;
  
  itype->type = INTERVAL_T;

  s = push_stack_of_interval_qualifier(lst->next, &itype->qualifier);
  if (!s) return NULL;

  fe_return(s);
}

static token_list*
push_stack_of_data_type
(
  token_list *lst, void *st
)
{
  s_data_type *dt = (s_data_type *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_character_string_type(lst, &dt->ch_str_type);
  if (s) {
    dt->is_character_string_type = Y;
    debug("character_string_type.\n");
    
    lst = s;
    s = push_stack_of_character_set_specification(lst->next, &dt->ch_set_spec);
    if (s) {
      dt->has_character_set_specification = Y;
      lst = s;
    }

    fe_return(lst);
  }

  s = push_stack_of_national_character_string_type(lst, &dt->nt_ch_str_type);
  if (s) {
    dt->is_national_character_string_type = Y;
    debug("national_character_string_type.\n");
    fe_return(s);
  }

  s = push_stack_of_bit_string_type(lst, &dt->bit_str_type);
  if (s) {
    dt->is_bit_string_type = Y;
    debug("bit_string_type.\n");
    fe_return(s);
  }

  s = push_stack_of_numeric_type(lst, &dt->num_type);
  if (s) {
    dt->is_numeric_type = Y;
    debug("numeric_type.\n");
    fe_return(s);
  }

  s = push_stack_of_datetime_type(lst, &dt->date_type);
  if (s) {
    dt->is_datetime_type = Y;
    debug("datetime_type.\n");
    fe_return(s);
  }

  s = push_stack_of_interval_type(lst, &dt->inter_type);
  if (s) {
    dt->is_interval_type = Y;
    debug("interval_type.\n");
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_domain_name
(
  token_list *lst, void *st
)
{
  return push_stack_of_qualified_name(lst, st);
}

static token_list*
push_stack_of_column_definition
(
  token_list *lst, void *st
)
{
  s_column_definition *cl_df = (s_column_definition *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_column_name(lst, &cl_df->cl_name);
  if (!s) return NULL;

  next_token(s);

  s = push_stack_of_data_type(s, &cl_df->dt_type);
  if (s) {
    cl_df->is_data_type = Y;
  }
  else {
    s = push_stack_of_domain_name(s, &cl_df->dm_name);
  }

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_table_element
(
  token_list *lst, void *st
)
{
  s_table_element *elm = (s_table_element *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_column_definition(lst, &elm->cl_def);
  if (s) {
    elm->is_column_definition = Y;
    fe();
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_table_element_list
(
  token_list *lst, void *st
)
{
  s_table_element_list *elm_lst = (s_table_element_list *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "<left paren>")) return NULL;
  next_token(lst);
  
  s = push_stack_of_table_element(lst, &elm_lst->elm);
  if (!s) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);

    elm_lst = elm_lst->next = gc_malloc(sizeof(s_table_element_list));
    if (!elm_lst) return NULL;
    elm_lst->next = NULL;
    
    s = push_stack_of_table_element(s->next, &elm_lst->elm);
    if (!s) return NULL;
  }

  if (!match(s->next, "<right paren>")) return NULL;
  next_token(s);

  fe_return(s);
}

static token_list*
push_stack_of_local_table_name
(
  token_list *lst, void *st
)
{
  local_table_name_t *name = (local_table_name_t *)st;
  fs();

  if (!match(lst, "<regular identifier>")) return NULL;
  
  *name = lst->tk.value;
  debug("local table name: %s \n", lst->tk.value);

  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_qualified_local_table_name
(
  token_list *lst, void *st
)
{
  s_qualified_local_table_name *sobj = (s_qualified_local_table_name *)st;
  token_list *s;

  fs();
  
  if (!match2(lst, "MODULE", "<period>")) return NULL;
  next_token(lst); next_token(lst);

  s = push_stack_of_local_table_name(lst, &sobj->lc_tb_name);

  s ? fe() : 0;
  
  fe_return(s);
}

static token_list*
push_stack_of_qualified_identifier
(
  token_list *lst, void *st
)
{
  qualified_identifier_t *qid = (qualified_identifier_t *)st;
  
  fs();

  if (!match(lst, "<regular identifier>")) return NULL;
  
  *qid = lst->tk.value;
  debug("qualified identifier: %s \n", *qid);
  
  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_catalog_name
(
  token_list *lst, void *st
)
{
  catalog_name_t *cname = (catalog_name_t *)st;
  
  fs();

  if (!match(lst, "<regular identifier>")) return NULL;
  
  *cname = lst->tk.value;
  debug("catalog name: %s \n", lst->tk.value);
  
  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_unqualified_schema_name
(
  token_list *lst, void *st
)
{
  unqualified_schema_name_t *o = (unqualified_schema_name_t *)st;
  
  fs();

  if (!match(lst, "<regular identifier>")) return NULL;
  
  *o = lst->tk.value;

  debug("unqulified schema name: %s \n", lst->tk.value);
  
  fe();
  fe_return(lst);
}


static token_list*
push_stack_of_schema_name
(
  token_list *lst, void *st
)
{
  s_schema_name *scname = (s_schema_name *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_catalog_name(lst, &scname->ctl_name);
  if (s) {
    scname->has_catalog_name_t = Y;
    if (match(s->next, "<period>")) {
      lst = s->next->next;
    }
  }

  s = push_stack_of_unqualified_schema_name(lst, &scname->unqlf_scm_name);

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_qualified_name
(
  token_list *lst, void *st
)
{
  s_qualified_name *qname = (s_qualified_name *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_schema_name(lst, &qname->scm_name);
  if (s) {
    if (match(s->next, "<period>")) {
      qname->has_schema_name = Y;
      lst = s->next->next;
    }
  }
  
  s = push_stack_of_qualified_identifier(lst, &qname->qlf_identifier);
  
  fe_return(s);
}


static token_list*
push_stack_of_table_name
(
  token_list *lst, void *st
)
{
  s_table_name *tb_name = (s_table_name *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_qualified_name(lst, &tb_name->qlf_name);
  if (s) fe_return(s);

  s = push_stack_of_qualified_local_table_name(lst, &tb_name->q_lc_name);
  
  fe_return(s);
}

static token_list*
push_stack_of_domain_temporary
(
  token_list *lst, void *st
)
{
  s_temporary_t *temp = (s_temporary_t *)st;
  
  fs();
  
  if (match(lst, "GLOBAL")) {
    *temp = GLOBAL;
    fe_return(lst);
  }
  
  if (match(lst, "LOCAL")) {
    *temp = LOCAL;
    fe_return(lst);
  }

  return NULL;
}

static token_list*
push_stack_of_table_definition
(
  token_list *lst, void *st
)
{
  s_table_definition *df = (s_table_definition *)st;
  token_list *s;
  
  fs();

  if (!match2(lst, "create", "table"))  return NULL;
  next_token(lst); next_token(lst);
  
  s = push_stack_of_domain_temporary(lst, &df->temporary);
  if (s) {
    df->is_temporary = Y;
    lst = s->next;
  }
  
  s = push_stack_of_table_name(lst, &df->tb_name);
  if (!s) return NULL;
  
  s = push_stack_of_table_element_list(s->next, &df->elm_lst);
  if (!s) return NULL;
  
  fe();
  fe_return(s);
}

static token_list*
push_stack_of_view_column_list
(
  token_list *lst, void *st
)
{
  return push_stack_of_column_name_list(lst, st);
}

static token_list*
push_stack_of_view_definition
(
  token_list *lst, void *st
)
{
  s_view_definition *def = (s_view_definition *)st;
  token_list *s;

  if (!match2(lst, "CREATE", "VIEW")) return NULL;
  next_token(lst); next_token(lst);

  s = push_stack_of_table_name(lst, &def->tb_name);
  if (!s) return NULL;

  if (match(s->next, "<left paren>")) {
    next_token(s);

    s = push_stack_of_view_column_list(s->next, &def->view_column_lst);
    if (!s) return NULL;
    next_token(s);
  }

  if (!match(s->next, "AS")) return NULL;
  next_token(s);

  s = push_stack_of_query_expression(s->next, &def->expr);
  fe_return(s);
}

static token_list*
push_stack_of_drop_behavior
(
  token_list *lst, void *st
)
{
  s_drop_behavior *bhv = (s_drop_behavior *)st;

  fs();
  
  if (match(lst, "CASCADE")) {
    bhv->is_cascade = Y;
  }
  else if (match(lst, "RESTRICT")) {
    bhv->is_restrict= Y;
  }
  else {
    return NULL;
  }

  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_drop_table_statement
(
  token_list *lst, void *st
)
{
  s_drop_table_statement *stm = (s_drop_table_statement *)st;
  token_list *s;
  
  fs();

  if (!match2(lst, "DROP", "TABLE")) return NULL;
  next_token(lst);
  next_token(lst);

  s = push_stack_of_table_name(lst, &stm->tb_name);
  if (!s) return NULL;

  s = push_stack_of_drop_behavior(s->next, &stm->drop_bhv);
  if (!s) return NULL;

  fe();
  fe_return(s);
}

static token_list*
push_stack_of_add_column_definition
(
  token_list *lst, void *st
)
{
  s_add_column_definition *df = (s_add_column_definition *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "ADD")) return NULL;
  next_token(lst);

  if (match(lst, "COLUMN")) next_token(lst);

  s = push_stack_of_column_definition(lst, &df->cl_def);
  
  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_sign
(
  token_list *lst, void *st
)
{
  s_sign *sign = (s_sign *)st;

  fs();
  
  if (match(lst, "<plus sign>")) {
    sign->is_plus = Y;
    fe_return(lst);
  }

  if (match(lst, "<minus sign>")) {
    sign->is_minus = Y;
    fe_return(lst);
  }

  return NULL;
}

static token_list*
push_stack_of_exact_numeric_literal
(
  token_list *lst, void *st
)
{
  s_exact_numeric_literal *ltr = (s_exact_numeric_literal *)st;
  token_list *s;

  fs();

  s = push_stack_of_unsigned_integer(lst, &ltr->uinteger);
  if (s) {
    ltr->is_unsignd_integer = Y;
    ltr->dval = ltr->uinteger;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_approximate_numeric_literal
(
  token_list *lst, void *st
)
{
  s_approximate_numeric_literal *ltr = (s_approximate_numeric_literal *)st;

  fs();
  
  if (!match(lst, "<approximate numeric literal>")) return NULL;

  fe_return(lst);
}

static token_list*
push_stack_of_unsigned_numeric_literal
(
  token_list *lst, void *st
)
{
  s_unsigned_numeric_literal *ltr = (s_unsigned_numeric_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_exact_numeric_literal(lst, &ltr->exact_num_lt);
  if (s) {
    ltr->is_exact_numeric_literal = Y;
    fe_return(s);
  }
  
  s = push_stack_of_approximate_numeric_literal(lst, &ltr->appr_num_lt);
  if (s) {
    ltr->is_approximate_numeric_literal = Y;
    fe_return(s);
  }	

  return NULL;
}

static token_list*
push_stack_of_character_string_literal
(
  token_list *lst, void *st
)
{
  s_character_string_literal *ltr = (s_character_string_literal *)st;

  fs();
  
  if (!match(lst, "<character string literal>")) return NULL;

  ltr->str = lst->tk.value;

  fe_return(lst);
}

static token_list*
push_stack_of_national_character_string_literal
(
  token_list *lst, void *st
)
{
  s_national_character_string_literal *ltr = st;

  fs();
  
  if (!match(lst, "<national character string literal>")) return NULL;

  fe_return(lst);
}

static token_list*
push_stack_of_bit_string_literal
(
  token_list *lst, void *st
)
{
  s_bit_string_literal *ltr = (s_bit_string_literal *)st;

  fs();
  
  if (!match(lst, "<bit string literal>")) return NULL;

  fe_return(lst);
}

static token_list*
push_stack_of_hex_string_literal
(
  token_list *lst, void *st
)
{
  s_hex_string_literal *ltr = (s_hex_string_literal *)st;

  fs();
  
  if (!match(lst, "<hex string literal>")) return NULL;

  fe_return(lst);
}

static token_list*
push_stack_of_date_string
(
  token_list *lst, void *st
)
{
  s_date_string *str = (s_date_string *)st;
  
  str->str = lst->tk.value;
  fe_return(lst);
}

static token_list*
push_stack_of_date_literal
(
  token_list *lst, void *st
)
{
  s_date_literal *ltr = (s_date_literal *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "DATE")) return NULL;
  next_token(lst);

  s = push_stack_of_date_string(lst, &ltr->date_str);

  fe_return(s);
}

static token_list*
push_stack_of_time_string
(
  token_list *lst, void *st
)
{
  s_time_string *str = (s_time_string *)st;
  
  str->str = lst->tk.value;
  fe_return(lst);
}

static token_list*
push_stack_of_time_literal
(
  token_list *lst, void *st
)
{
  s_time_literal *ltr = (s_time_literal *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "TIME")) return NULL;
  next_token(lst);
  
  s = push_stack_of_time_string(lst, &ltr->time_str);

  fe_return(s);
}

static token_list*
push_stack_of_timestamp_string
(
  token_list *lst, void *st
)
{
  s_timestamp_string *str = (s_timestamp_string *)st;
  
  str->str = lst->tk.value;
  fe_return(lst);
}

static token_list*
push_stack_of_timestamp_literal
(
  token_list *lst, void *st
)
{
  s_timestamp_literal *ltr = (s_timestamp_literal *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "TIMESTAMP")) return NULL;
  next_token(lst);
  
  s = push_stack_of_timestamp_string(lst, &ltr->tms_str);

  fe_return(s);
}

static token_list*
push_stack_of_datetime_literal
(
  token_list *lst, void *st
)
{
  s_datetime_literal *ltr = (s_datetime_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_date_literal(lst, &ltr->dt_literal);
  if (s) {
    ltr->is_date_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_time_literal(lst, &ltr->tm_literal);
  if (s) {
    ltr->is_time_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_timestamp_literal(lst, &ltr->tms_literal);
  if (s) {
    ltr->is_timestamp_literal = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_interval_string
(
  token_list *lst, void *st
)
{
  s_interval_string *str = (s_interval_string *)st;
  
  str->str = lst->tk.value;
  fe_return(lst);
}

static token_list*
push_stack_of_interval_literal
(
  token_list *lst, void *st
)
{
  s_interval_literal *ltr = (s_interval_literal *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "INTERVAL")) return NULL;

  s = push_stack_of_sign(lst, &ltr->sign);
  if (s) {
    ltr->has_sign = Y;
    lst = s->next;
  }

  s = push_stack_of_interval_string(lst, &ltr->inter_str);
  if (!s) return NULL;

  s= push_stack_of_interval_qualifier(s->next, &ltr->inter_qlf);
  if (!s) return NULL;

  fe_return(s);
}

static token_list*
push_stack_of_general_literal
(
  token_list *lst, void *st
)
{
  s_general_literal *ltr = (s_general_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_character_string_literal(lst, &ltr->ch_str_lt);
  if (s) {
    ltr->is_character_string_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_national_character_string_literal(lst, &ltr->nch_str_lt);
  if (s) {
    ltr->is_national_string_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_bit_string_literal(lst, &ltr->bit_str_lt);
  if (s) {
    ltr->is_bit_string_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_hex_string_literal(lst, &ltr->hex_str_lt);
  if (s) {
    ltr->is_hex_string_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_datetime_literal(lst, &ltr->date_lt);
  if (s) {
    ltr->is_datetime_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_interval_literal(lst, &ltr->inter_lt);
  if (s) {
    ltr->is_interval_literal = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_signed_numeric_literal
(
  token_list *lst, void *st
)
{
  s_signed_numeric_literal *ltr = (s_signed_numeric_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_sign(lst, &ltr->sign);
  if (s) {
    next_token(s);
    lst = s;
  }

  s = push_stack_of_general_literal(lst, &ltr->unsign_num_ltr);

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_literal
(
  token_list *lst, void *st
)
{
  s_literal *ltr = (s_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_signed_numeric_literal(lst, &ltr->sign_num_ltr);
  if (s) {
    ltr->is_signed_numeric_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_general_literal(lst, &ltr->gen_lt);
  if (s) {
    ltr->is_general_literal = Y;
    fe_return(s);
  }  

  return NULL;
}

static token_list*
push_stack_of_datetime_value_function
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_default_option
(
  token_list *lst, void *st
)
{
  s_default_option *option = (s_default_option *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_literal(lst, &option->ltr);
  if (s) {
    option->is_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_datetime_value_function(lst, &option->dt_vl_function);
  if (s) {
    option->is_datetime_value_function = Y;
    fe_return(s);
  }

  if (match(lst, "USER")) {
    option->is_user = Y;
    fe_return(lst);
  }
  
  if (match(lst, "CURRENT_USER")) {
    option->is_current_user = Y;
    fe_return(lst);
  }
  
  if (match(lst, "SESSION_USER")) {
    option->is_session_user = Y;
    fe_return(lst);
  }
  
  if (match(lst, "SYSTEM_USER")) {
    option->is_system_user = Y;
    fe_return(lst);
  }
  
  if (match(lst, "NULL")) {
    option->is_null = Y;
    fe_return(lst);
  }  

  return NULL;
}

static token_list*
push_stack_of_default_clause
(
  token_list *lst, void *st
)
{
  s_default_clause *clause = (s_default_clause *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "DEFAULT")) return NULL;

  s = push_stack_of_default_option(lst->next, &clause->def_option);
  
  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_set_column_default_clause
(
  token_list *lst, void *st
)
{
  s_set_column_default_clause *clause = (s_set_column_default_clause *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "SET")) return NULL;

  s = push_stack_of_default_clause(lst->next, &clause->df_clause);
  if (!s) return NULL;

  fe();
  fe_return(s);
}

static token_list*
push_stack_of_drop_column_default_clause
(
  token_list *lst, void *st
)
{
  fs();

  if (!match2(lst, "DROP", "DEFAULT")) return NULL;

  fe();
  fe_return(lst);
}

static token_list*
push_stack_of_column_action
(
  token_list *lst, void *st
)
{
  s_alter_column_action *ac = (s_alter_column_action *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_set_column_default_clause(lst, &ac->set_default_clause);
  if (s) {
    ac->is_set_column_default_clause = Y;
    fe_return(s);
  }

  s = push_stack_of_drop_column_default_clause(lst, NULL);
  if (s) {
    ac->is_drop_column_default_clause = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_alter_column_definition
(
  token_list *lst, void *st
)
{
  s_alter_column_definition *df = (s_alter_column_definition *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "ALTER")) return NULL;
  next_token(lst);

  if (match(lst, "COLUMN")) next_token(lst);

  s = push_stack_of_column_name(lst, &df->cl_name);
  if (!s) return NULL;

  s = push_stack_of_column_action(s->next, &df->alter_cl_ac);
  if (!s) return NULL;

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_drop_column_definition
(
  token_list *lst, void *st
)
{
  s_drop_column_definition *df = (s_drop_column_definition *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "DROP")) return NULL;
  next_token(lst);
  
  if (match(lst, "COLUMN")) next_token(lst);
  
  s = push_stack_of_column_name(lst, &df->cl_name);
  if (!s) return NULL;
  
  s = push_stack_of_drop_behavior(s->next, &df->drop_bh);

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_table_constraint_definition
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_add_table_constraint_definition
(
  token_list *lst, void *st
)
{
  s_add_table_constraint_definition *df = st;
  token_list *s;

  if (!match(lst, "ADD")) return NULL;
  next_token(lst);

  s = push_stack_of_table_constraint_definition(lst, &df->cst_def);
  fe_return(s);
}

static token_list*
push_stack_of_constraint_name
(
  token_list *lst, void *st
)
{
  return push_stack_of_qualified_name(lst, st);
}

static token_list*
push_stack_of_drop_table_constraint_definition
(
  token_list *lst, void *st
)
{
  s_drop_table_constraint_definition *df = st;
  token_list *s;
  
  if (!match2(lst, "DROP", "CONSTRAINT")) return NULL;
  next_token(lst);
  next_token(lst);
  
  s = push_stack_of_constraint_name(lst, &df->cst_name);
  if (!s) return NULL;
  
  s = push_stack_of_drop_behavior(s, &df->drop_bh);
  fe_return(s);
}

static token_list*
push_stack_of_alter_table_action
(
  token_list *lst, void *st
)
{
  s_alter_table_action *act = (s_alter_table_action *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_add_column_definition(lst, &act->add_cl_def);
  if (s) {
    act->is_add_column_definition = Y;
    goto END;
  }
  
  s = push_stack_of_alter_column_definition(lst, &act->alter_cl_def);
  if (s) {
    act->is_alter_column_definition = Y;
    goto END;
  }
  
  s = push_stack_of_drop_column_definition(lst, &act->drop_cl_def);
  if (s) {
    act->is_drop_column_definition = Y;
    goto END;
  }
  
  s = push_stack_of_add_table_constraint_definition(lst, &act->add_cst_def);
  if (s) {
    act->is_add_table_constraint_definition = Y;
    goto END;
  }

  s = push_stack_of_drop_table_constraint_definition(lst, &act->drop_cst_def);
  if (s) {
    act->is_drop_table_constraint_definition = Y;
    goto END;
  }

  return NULL;

END:
  fs();
  fe_return(s);
}

static token_list*
push_stack_of_alter_table_statement
(
  token_list *lst, void *st
)
{
  s_alter_table_statement *stm = (s_alter_table_statement *)st;
  token_list *s;

  fs();

  if (!match2(lst, "ALTER", "TABLE")) return NULL;
  next_token(lst);
  next_token(lst);
  
  s = push_stack_of_table_name(lst, &stm->tb_name);
  if (!s) return NULL;
  
  s = push_stack_of_alter_table_action(s->next, &stm->action);
  if (!s) return NULL;  

  fe();
  fe_return(s);
}

static token_list*
push_stack_of_column_name_list
(
  token_list *lst, void *st
)
{
  s_column_name_list *name_lst = (s_column_name_list *)st;
  token_list *s;
  s_column_name_list *next;

  fs();
  
  s = push_stack_of_column_name(lst, &name_lst->clname);
  if (!s) return NULL;

  name_lst->next = gc_malloc(sizeof(s_column_name_list));
  if (!name_lst->next) return NULL;
  
  next = name_lst->next;
  while (match(s->next, "<comma>")) {
    next_token(s);

    s = push_stack_of_column_name(s, &next->clname);
    if (!s) return NULL;
    
    next->next = gc_malloc(sizeof(s_column_name_list));
    if (!next->next) return NULL;
    next = next->next;
  }

  fe_return(s);
}

static token_list*
push_stack_of_insert_column_list
(
  token_list *lst, void *st
)
{
  s_insert_column_list *cl_lst = (s_insert_column_list *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_column_name_list(lst, &cl_lst->cl_name_list);

  fe_return(s);
}

static token_list*
push_stack_of_as_clause
(
  token_list *lst, void *st
)
{
  s_as_clause *cls = (s_as_clause *)st;
  token_list *s;

  if (match(lst, "AS")) {
    cls->has_as = Y;
    next_token(lst);
  }

  s = push_stack_of_column_name(lst, &cls->cl_name);
  fe_return(s);
}

static token_list*
push_stack_of_derived_column
(
  token_list *lst, void *st
)
{
  s_derived_column *cl = (s_derived_column *)st;
  token_list *s;

  s = push_stack_of_value_expression(lst, &cl->val_expr);
  if (!s) return NULL;

  lst = push_stack_of_as_clause(s->next, &cl->as_cls);
  if (lst) {
    cl->is_as_clause = Y;
    fe_return(lst);
  }

  fe_return(s);
}

static token_list*
push_stack_of_select_sublist
(
  token_list *lst, void *st
)
{
  s_select_sublist *slst = (s_select_sublist *)st;
  token_list *s;

  s = push_stack_of_derived_column(lst, &slst->derived_cl);
  if (s) {
    slst->is_derived_column = Y;
    fe_return(s);
  }

  s = push_stack_of_qualifier(lst, &slst->qlf);
  if (s) {
    slst->is_qualifier = Y;
    next_token(s); next_token(s);
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_select_list
(
  token_list *lst, void *st
)
{
  s_select_list *slst = (s_select_list *)st;
  token_list *s;
  s_select_list *next;
  
  if (match(lst, "<asterisk>")) {
    slst->is_asterisk = Y;
    fe_return(lst);
  }

  s = push_stack_of_select_sublist(lst, &slst->slst);
  if (!s) return NULL;

  next = slst->next = gc_malloc(sizeof(s_select_list));
  if (!next) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);
    
    s = push_stack_of_select_sublist(s->next, &next->slst);
    if (!s) return NULL;

    next = next->next = gc_malloc(sizeof(s_select_list));
    if (!next) return NULL;
  }

  fe_return(s);
}

static token_list*
push_stack_of_table_reference
(
  token_list *lst, void *st
)
{
  s_table_reference *ref = (s_table_reference *)st;
  
  return push_stack_of_table_name(lst, &ref->tb_name);
}

static token_list*
push_stack_of_table_reference_list
(
  token_list *lst, void *st 
)
{
  s_table_reference_list *rlst = (s_table_reference_list *)st;
  token_list *s;
  
  s = push_stack_of_table_reference(lst, &rlst->tb_ref);
  if (!s) return NULL;
  
  rlst->next = gc_malloc(sizeof(s_table_reference_list));
  if (!rlst->next) return NULL;

  if (match(s->next, "<comma>")) {
    next_token(s);
    
    s = push_stack_of_table_reference_list(s->next, rlst->next);
  }
  else {
    rlst->next = NULL;
  }
  
  fe_return(s);
}

static token_list*
push_stack_of_from_clause
(
  token_list *lst, void *st 
)
{
  s_from_clause *cls = (s_from_clause *)st;
  token_list *s;

  if (!match(lst, "FROM")) return NULL;
  next_token(lst);
  
  s = push_stack_of_table_reference_list(lst, &cls->tb_ref_lst);
  fe_return(s);
}

static token_list*
push_stack_of_where_clause
(
  token_list *lst, void *st 
)
{
  s_where_clause *cls = (s_where_clause *)st;
  if (!match(lst, "WHERE")) return NULL;
  next_token(lst);

  return push_stack_of_search_condition(lst, &cls->se_cond);
}

static token_list*
push_stack_of_group_column_reference
(
  token_list *lst, void *st 
)
{
  s_group_column_reference *ref = (s_group_column_reference *)st;
  
  return push_stack_of_column_reference(lst, &ref->cl_ref);
}

static token_list*
push_stack_of_group_column_reference_list
(
  token_list *lst, void *st 
)
{
  s_group_column_reference_list *rlst = (s_group_column_reference_list *)st;
  token_list *s;
  
  s = push_stack_of_group_column_reference(lst, &rlst->grp_cl_ref);
  if (!s) return NULL;
  
  rlst->next = gc_malloc(sizeof(s_group_column_reference_list));
  if (!rlst->next) return NULL;
  
  if (match(s->next, "<comma>")) {
    next_token(s);
    
    s = push_stack_of_group_column_reference_list(s->next, rlst->next);
  }
  
  fe_return(s);
}

static token_list*
push_stack_of_group_by_clause
(
  token_list *lst, void *st 
)
{
  s_group_by_clause *cls = (s_group_by_clause *)st;
  token_list *s;
  
  if (!match2(lst, "GROUP", "BY")) return NULL;
  next_token(lst); next_token(lst);
  
  s = push_stack_of_group_column_reference_list(lst, &cls->grp_cl_ref_lst);
  fe_return(s);
}

static token_list*
push_stack_of_having_clause
(
  token_list *lst, void *st 
)
{
  s_having_clause *cls = (s_having_clause *)st;
  
  if (!match(lst, "HAVING")) return NULL;
  next_token(lst);
  
  return push_stack_of_search_condition(lst, &cls->se_cond);
}

static token_list*
push_stack_of_table_expression
(
  token_list *lst, void *st 
)
{
  s_table_expression *expr = (s_table_expression *)st;
  token_list *s;

  s = push_stack_of_from_clause(lst, &expr->from_cls);
  if (!s) return NULL;
  lst = s;
  
  s = push_stack_of_where_clause(lst->next, &expr->where_cls);
  if (s) {
    expr->has_where_clause = Y;
    lst = s;
  }

  s = push_stack_of_group_by_clause(lst->next, &expr->group_by_cls);
  if (s) {
    expr->has_group_by_clause = Y;
    lst =s ;
  }

  s = push_stack_of_having_clause(lst->next, &expr->having_cls);
  if (s) {
    expr->has_having_clause = Y;
    lst = s;
  }

  fe_return(lst);
}

static token_list*
push_stack_of_query_specification
(
  token_list *lst, void *st 
)
{
  s_query_specification *spec = (s_query_specification *)st;
  token_list *s;

  if (!match(lst, "SELECT")) return NULL;
  next_token(lst);

  s = push_stack_of_set_quantifier(lst, &spec->set_qtf);
  if (s) {
    spec->has_set_quantifier = Y;
    lst = s;
  }

  s = push_stack_of_select_list(lst, &spec->select_lst);
  if (!s) return NULL;

  s = push_stack_of_table_expression(s->next, &spec->table_expr);
  fe_return(s);
}

static token_list*
push_stack_of_unsigned_literal
(
  token_list *lst, void *st 
)
{
  s_unsigned_literal *ltr = (s_unsigned_literal *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_unsigned_numeric_literal(lst, &ltr->us_num_lt);
  if (s) {
    ltr->is_unsigned_numeric_literal = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_general_value_specification
(
  token_list *lst, void *st 
)
{
  return NULL;
}

static token_list*
push_stack_of_unsigned_value_specification
(
  token_list *lst, void *st 
)
{
  s_unsigned_value_specification *spec = (s_unsigned_value_specification *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_unsigned_literal(lst, &spec->uns_literal);
  if (s) {
    spec->is_unsigned_literal = Y;
    fe_return(s);
  }

  s = push_stack_of_general_value_specification(lst, &spec->gen_val_spec);
  if (s) {
    spec->is_general_value_specification = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_correlation_name
(
  token_list *lst, void *st 
)
{
  correlation_name_t *name = (correlation_name_t *)st;
  
  *name = lst->tk.value;
  fe_return(lst);
}

static token_list*
push_stack_of_qualifier
(
  token_list *lst, void *st 
)
{
  s_qualifier *qlf = (s_qualifier *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_table_name(lst, &qlf->tb_name);
  if (s) {
    qlf->is_table_name = Y;
    fe_return(s);
  }

  s = push_stack_of_correlation_name(lst, &qlf->cr_name);
  if (s) {
    qlf->is_correlation_name = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_column_reference
(
  token_list *lst, void *st 
)
{
  s_column_reference *ref = (s_column_reference *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_qualifier(lst, &ref->qlf);
  if (s) {
    if (match(s->next, "<period>")) {
      lst = s->next->next;
      ref->has_qualifier = Y;
    }
  }

  s = push_stack_of_column_name(lst, &ref->cname);
  fe_return(s);
}

static token_list*
push_stack_of_set_function_type
(
  token_list *lst, void *st 
)
{
  s_set_function_type *set_type = (s_set_function_type *)st;
  
  if (match(lst, "AVG")) {
    set_type->is_avg = Y;
    fe_return(lst);
  }

  if (match(lst, "MAX")) {
    set_type->is_max = Y;
    fe_return(lst);
  }
  
  if (match(lst, "MIN")) {
    set_type->is_min = Y;
    fe_return(lst);
  }
  
  if (match(lst, "SUM")) {
    set_type->is_sum = Y;
    fe_return(lst);
  }
  
  if (match(lst, "COUNT")) {
    set_type->is_count = Y;
    fe_return(lst);
  }

  return NULL;  
}

static token_list*
push_stack_of_set_quantifier
(
  token_list *lst, void *st 
)
{
  s_set_quantifier *qtf = (s_set_quantifier *)st;
  
  if (match(lst, "DISTINCT")) {
    qtf->is_distinct = Y;
    fe_return(lst);
  }
  
  if (match(lst, "ALL")) {
    qtf->is_all = Y;
    fe_return(lst);
  }

  return NULL;
}

static token_list*
push_stack_of_general_set_function
(
  token_list *lst, void *st 
)
{
  s_general_set_function *set_func = (s_general_set_function *)st;
  token_list *s;

  s = push_stack_of_set_function_type(lst, &set_func->func_type);
  fe_return(s);
}

static token_list*
push_stack_of_set_function_specification
(
  token_list *lst, void *st 
)
{
  s_set_function_specification *spec = (s_set_function_specification *)st;
  token_list *s;

  if (match(lst, "COUNT")) {
    spec->is_count = Y;
    next_token(lst);
    
    if (!match3(lst, "<left paren>", "<asterisk>", "<right paren>"))
      return NULL;

    next_token(lst);
    next_token(lst);
    
    fe_return(lst);
  }

  s = push_stack_of_general_set_function(lst, &spec->set_func);
  if (s) {
    spec->is_general_set_function = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_value_expression_primary
(
  token_list *lst, void *st 
)
{
  s_value_expression_primary *prim = (s_value_expression_primary *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_unsigned_value_specification(lst, &prim->uns_val_spec);
  if (s) {
    prim->is_unsigned_value_specification = Y;
    fe_return(s);
  }

  s = push_stack_of_column_reference(lst, &prim->cl_ref);
  if (s) {
    prim->is_column_reference = Y;
    fe_return(s);
  }

  s = push_stack_of_set_function_specification(lst, &prim->set_fun_spec);
  if (s) {
    prim->is_set_function_secification = Y;
    fe_return(s);
  }

  if (match(lst, "<left paren>")) {
    next_token(lst);
  
    prim->paren_expr = gc_malloc(sizeof(s_value_expression));
    if (!prim->paren_expr) return NULL;
    
    s = push_stack_of_value_expression(lst, prim->paren_expr);
    if (s) prim->is_paren_value_expression = Y;
    
    if (!match(s->next, "<right paren>")) return NULL;
    fe_return(s->next);
  }    

  return NULL;
}

static token_list*
push_stack_of_numeric_value_function
(
  token_list *lst, void *st 
)
{
  return NULL;
}

static token_list*
push_stack_of_numeric_primary
(
  token_list *lst, void *st 
)
{
  s_numeric_primary *prim = (s_numeric_primary *)st;
  token_list *s;

  s = push_stack_of_value_expression_primary(lst, &prim->val_expr_prim);
  if (s) {
    prim->is_value_expression_primary = Y;
    fe_return(s);
  }

  s = push_stack_of_numeric_value_function(lst, &prim->num_val_func);
  if (s) {
    prim->is_numeric_value_function = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_factor
(
  token_list *lst, void *st 
)
{
  s_factor *factor = (s_factor *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_sign(lst, &factor->sign);
  if (s) {
    factor->has_sign = Y;
  }
  else {
    s = lst;
  }
  
  s = push_stack_of_numeric_primary(s, &factor->num_prim);
  fe_return(s);
}

static token_list*
push_stack_of_term1
(
  token_list *lst, void *st 
)
{
  s_term1 *term = (s_term1 *)st;
  token_list *s;

  fs();
  
  if (match(lst, "<asterisk>")) {
    term->is_asterisk = Y;  
  }
  else if (match(lst, "<solidus>")) {
    term->is_solidus = Y;
  }
  else if (match(lst, "@")) {
    term->is_end = Y;
    fe_return(lst);
  }
  else {
    return NULL;
  }

  next_token(lst);
  
  s = push_stack_of_factor(lst, &term->ft);
  if (!s) return NULL;
  lst = s;
  
  term->next = gc_malloc(sizeof(s_term1));
  if (!term->next) return NULL;
  
  s = push_stack_of_term1(s, term->next);
  s ? (lst = s) : (term->next = NULL);
  
  fe_return(lst);
}

static token_list*
push_stack_of_term
(
  token_list *lst, void *st
)
{
  s_term *term = (s_term *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_factor(lst, &term->ft);
  if (!s) return NULL;
  lst = s;

  s = push_stack_of_term1(s->next, &term->tm1);
  if (s) lst = s;

  fe_return(lst);
}

static token_list*
push_stack_of_numeric_value_expression1
(
  token_list *lst, void *st
)
{
  s_numeric_value_expression1 *expr = (s_numeric_value_expression1 *)st;
  token_list *s;

  fs();
  
  if (match(lst, "<plus sign>")) {
    expr->is_plus_sign = Y;
    next_token(lst);
  }
  else if (match(lst, "<minus sign>")) {
    expr->is_minus_sign = Y;
    next_token(lst);
  }
  else if (match(lst, "@")) {
    expr->is_end = Y;
    fe_return(lst);
  }
  else {
    return NULL;
  }
  
  s = push_stack_of_term(lst, &expr->tm);
  if (!s) return NULL;
  lst = s;
  
  expr->next = gc_malloc(sizeof(s_numeric_value_expression1));
  if (!expr->next) return NULL;
  
  s = push_stack_of_numeric_value_expression1(s->next, expr->next);
  if (!s) {
    expr->next = NULL;
  }
  else {
    lst = s;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_numeric_value_expression
(
  token_list *lst, void *st
)
{
  s_numeric_value_expression *expr = (s_numeric_value_expression *)st;
  token_list *s;

  fs();

  s = push_stack_of_term(lst, &expr->tm);
  if (!s) return NULL;
  lst = s;

  s = push_stack_of_numeric_value_expression1(s->next, &expr->num_val_expr1);
  if (s) lst = s;

  fe_return(lst);
}

static token_list*
push_stack_of_str_val_expression_primary
(
  token_list *lst, void *st
)
{
  s_string_value_expression_primary *p = st;
  token_list*s;

  fs();

  s = push_stack_of_character_string_literal(lst, &p->char_str_ltr);
  if (s) {
    p->is_character_string_literal = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_character_primary
(
  token_list *lst, void *st
)
{
  s_character_primary *prim = (s_character_primary *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_str_val_expression_primary(lst, &prim->str_val_expr_prim);
  if (s) {
    prim->is_string_value_expression_primary = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_collate_clause
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_character_factor
(
  token_list *lst, void *st
)
{
  s_character_factor *ft = (s_character_factor *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_character_primary(lst, &ft->ch_prim);

  fe_return(s);
}

static token_list*
push_stack_of_concatenation
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_character_value_expression
(
  token_list *lst, void *st
)
{
  s_character_value_expression *expr = (s_character_value_expression *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_character_factor(lst, &expr->ch_factor);
  
  fe_return(s);
}

static token_list*
push_stack_of_bit_value_expression
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_string_value_expression
(
  token_list *lst, void *st
)
{
  s_string_value_expression *expr = (s_string_value_expression *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_character_value_expression(lst, &expr->ch_val_expr);
  if (s) {
    expr->is_character_value_expression = Y;
    fe_return(s);
  }

  s = push_stack_of_bit_value_expression(lst, &expr->bit_val_expr);
  if (s) {
    expr->is_bit_value_expression = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_datetime_value_expression
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_interval_value_expression
(
  token_list *lst, void *st
)
{
  return NULL;
}


static token_list*
push_stack_of_value_expression
(
  token_list *lst, void *st
)
{
  s_value_expression *expr = (s_value_expression *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_numeric_value_expression(lst, &expr->num_val_expr);
  if (s) {
    expr->is_numeric_value_expression = Y;
    fe_return(s);
  }

  s = push_stack_of_string_value_expression(lst, &expr->str_val_expr);
  if (s) {
    expr->is_string_value_expression = Y;
    fe_return(s);
  }

  s = push_stack_of_datetime_value_expression(lst, &expr->date_val_expr);
  if (s) {
    expr->is_datetime_value_expression = Y;
    fe_return(s);
  }

  s = push_stack_of_interval_value_expression(lst, &expr->inter_val_expr);
  if (s) {
    expr->is_interval_value_expression = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_row_value_constructor_element
(
  token_list *lst, void *st
)
{
  s_row_value_constructor_element *elm = (s_row_value_constructor_element *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_value_expression(lst, &elm->val_expr);
  if (s) {
    elm->is_value_expression = Y;
    fe_return(s);
  }

  if (match(lst, "NULL")) {
    elm->is_null_specification = Y;
    fe_return(lst);
  }

  if (match(lst, "DEFAULT")) {
    elm->is_default_specification = Y;
    fe_return(lst);
  }

  return NULL;
}

static token_list*
push_stack_of_row_value_constructor_list
(
  token_list *lst, void *st
)
{
  s_row_value_constructor_list *elm = (s_row_value_constructor_list *)st;
  token_list *s;
  s_row_value_constructor_list *next;

  fs();
  
  s = push_stack_of_row_value_constructor_element(lst, &elm->row_val_elm);
  if (!s) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);

    elm = elm->next = gc_malloc(sizeof(s_row_value_constructor_list));
    if (!elm) return NULL;
    elm->next = NULL;

    s = push_stack_of_row_value_constructor_element(s->next, &elm->row_val_elm);
    if (!s) return NULL;
  }

  fe_return(s);
}

static token_list*
push_stack_of_row_subquery
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_row_value_constructor
(
  token_list *lst, void *st
)
{
  s_row_value_constructor *cstr = (s_row_value_constructor *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_row_value_constructor_element(lst, &cstr->row_val_cstr_elm);
  if (s) {
    cstr->is_row_value_constructor_element = Y;
    fe_return(s);
  }

  if (match(lst, "<left paren>")) {
    next_token(lst);

    s = push_stack_of_row_value_constructor_list(lst, &cstr->row_val_list);
    if (!s) return NULL;
    cstr->is_row_value_constructor_list = Y;
    
    if (!match(s->next, "<right paren>")) return NULL;
    fe_return(s->next);
  }

  s = push_stack_of_row_subquery(lst, &cstr->a_row_subquery);
  if (s) {
    cstr->is_row_subquery = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_table_value_constructor_list
(
  token_list *lst, void *st
)
{
  s_table_value_constructor_list *val_lst = (s_table_value_constructor_list *)st;
  token_list *s;
  s_table_value_constructor_list *next;

  fs();
  
  s = push_stack_of_row_value_constructor(lst, &val_lst->row_val_cstr);
  if (!s) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);

    next->next = gc_malloc(sizeof(s_table_value_constructor_list));
    if (!next->next) return NULL;
    next = next->next;
    next->next = NULL;

    s = push_stack_of_row_value_constructor(s, &next->row_val_cstr);
    if (!s) return NULL;
  }

  fe_return(s);
}

static token_list*
push_stack_of_table_value_constructor
(
  token_list *lst, void *st
)
{
  s_table_value_constructor *cstr = (s_table_value_constructor *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "VALUES")) return NULL;

  s = push_stack_of_table_value_constructor_list(lst->next,  &cstr->val_lst);

  fe_return(s);
}

static token_list*
push_stack_of_explicit_table
(
  token_list *lst, void *st
)
{
  s_explicit_table *tb = (s_explicit_table *)st;
  token_list *s;

  fs();
  
  if (!match(lst, "TABLE")) return NULL;

  s = push_stack_of_table_name(lst->next, &tb->tb_name);
  fe_return(s);
}

static token_list*
push_stack_of_simple_table
(
  token_list *lst, void *st
)
{
  s_simple_table *tb = (s_simple_table *)st;
  token_list *s;

  fs();

  s = push_stack_of_query_specification(lst, &tb->query_spec);
  if (s) {
    tb->is_query_specification = Y;
    fe_return(s);
  }

  s = push_stack_of_table_value_constructor(lst, &tb->tb_val_cstr);
  if (s) {
    tb->is_table_value_constructor = Y;
    fe_return(s);
  }

  s = push_stack_of_explicit_table(lst, &tb->explicit_tb);
  if (s) {
    tb->is_explicit_table = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_non_join_query_primary
(
  token_list *lst, void *st
)
{
  s_non_join_query_primary *prim = (s_non_join_query_primary *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_simple_table(lst, &prim->sptable);
  if (s) {
    prim->is_simple_table = Y;
    fe_return(s);
  }

  if (!match(lst, "<left parent>")) return NULL;

  s = push_stack_of_non_join_query_expression(lst->next, prim);
  if (!s) return NULL;
  
  if (!match(s->next, "<right parent>")) return NULL;

  fe_return(s->next);
}

static token_list*
push_stack_of_non_join_query_term
(
  token_list *lst, void *st
)
{
  s_non_join_query_term *term = (s_non_join_query_term *)st;
  token_list *s;

  fs();

  s = push_stack_of_non_join_query_primary(lst, &term->njoin_qprimary);
  if (s) term->is_non_join_query_primary = Y;

  fe_return(s);
}

static token_list*
push_stack_of_non_join_query_expression
(
  token_list *lst, void *st
)
{
  s_non_join_query_expression *expr = (s_non_join_query_expression *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_non_join_query_term(lst, &expr->njoin_qterm);
  if (s) expr->is_non_join_query_term = Y;

  fe_return(s);
}

static token_list*
push_stack_of_joined_table
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_query_expression
(
  token_list *lst, void *st
)
{
  s_query_expression *expr = (s_query_expression *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_non_join_query_expression(lst, &expr->njoin_qexpr);
  if (s) {
    expr->is_non_join_query_expression = Y;
  }
  else {
    s = push_stack_of_joined_table(lst, &expr->join_table);
    if (s) expr->is_joined_table = Y;
  }
  
  fe_return(s);
}

static token_list*
push_stack_of_insert_columns_and_source
(
  token_list *lst, void *st
)
{
  s_insert_columns_and_source *ins = (s_insert_columns_and_source *)st;
  token_list *s;

  fs();
  
  if (match2(lst, "DEFAULT", "VALUES")) {
    ins->is_default_values = Y;
    fe_return(lst->next);
  }

  if (match(lst, "<left paren>")) {
    next_token(lst);

    s = push_stack_of_insert_column_list(lst, &ins->insert_cl_lst);
    if (!s) return NULL;

    ins->has_insert_column_list = Y;
    next_token(s);
    next_token(s);
    lst = s;
  }

  s = push_stack_of_query_expression(lst, &ins->qexpression);
  if (s) ins->has_query_expression = Y;
  
  fe_return(s);
}

static token_list*
push_stack_of_insert_statement
(
  token_list *lst, void *st
)
{
  s_insert_statement *stm = (s_insert_statement *)st;
  token_list *s;
  
  fs();
  
  if (!match2(lst, "INSERT", "INTO")) return NULL;
  next_token(lst);
  next_token(lst);
  
  s = push_stack_of_table_name(lst, &stm->tb_name);
  if (!s) return NULL;
  
  s = push_stack_of_insert_columns_and_source(s->next, &stm->insert_cl_src);

  s ? fe() : 0;
  fe_return(s);
}

static token_list*
push_stack_of_comp_op
(
  token_list *lst, void *st
)
{
  s_comp_op *op = (s_comp_op *)st;

  fs();
  
  if (match(lst, "<equals operator>")) {
    op->is_equal_op = Y;
  }
  else if (match(lst, "<not equals operator>")) {
    op->is_not_equal_op = Y;
  }
  else if (match(lst, "<less than operator>")) {
    op->is_less_than_op = Y;
  }
  else if (match(lst, "<greater than operator>")) {
    op->is_greater_than_op = Y;
  }
  else if (match(lst, "<less than or equals operator>")) {
    op->is_less_than_or_equal_op = Y;
  }
  else if (match(lst, "<greater than or equals operator>")) {
    op->is_greater_than_or_equal_op = Y;
  }
  else {
    return NULL;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_comparison_predicate
(
  token_list *lst, void *st
)
{
  s_comparison_predicate *pred = (s_comparison_predicate *)st;
  token_list *s;

  fs();

  s = push_stack_of_row_value_constructor(lst, &pred->lrow_val_cstr);
  if (!s) return NULL;

  s = push_stack_of_comp_op(s->next, &pred->op);
  if (!s) return NULL;

  s = push_stack_of_row_value_constructor(s->next, &pred->rrow_val_cstr);
  
  fe_return(s);
}

static token_list*
push_stack_of_between_predicate
(
  token_list *lst, void *st
)
{
  s_between_predicate *pred = (s_between_predicate *)st;
  token_list *s;

  s = push_stack_of_row_value_constructor(lst, &pred->row_val_cstr1);
  if (!s) return NULL;

  if (match(s->next, "NOT")) {
    pred->has_not = Y;
    next_token(s);
  }

  if (!match(s->next, "BETWEEN")) return NULL;
  next_token(s);

  s = push_stack_of_row_value_constructor(s->next, &pred->row_val_cstr2);
  if (!s) return NULL;
  
  next_token(s);

  s = push_stack_of_row_value_constructor(s->next, &pred->row_val_cstr3);
  fe_return(s);
}

static token_list*
push_stack_of_table_subquery
(
  token_list *lst, void *st
)
{
  return push_stack_of_query_expression(lst, st);
}

static token_list*
push_stack_of_in_value_list
(
  token_list *lst, void *st
)
{
  s_in_value_list *val_lst = (s_in_value_list *)st;
  token_list *s;
  s_in_value_list *next;
  
  s = push_stack_of_value_expression(lst, &val_lst->val_expr);
  if (!s) return NULL;

  next = val_lst->next = gc_malloc(sizeof(s_in_value_list));
  if (!next) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);

    s = push_stack_of_value_expression(s->next, &next->val_expr);
    if (!s) return NULL;

    next = next->next = gc_malloc(sizeof(s_in_value_list));
    if (!next) return NULL;
  }

  fe_return(s);
}

static token_list*
push_stack_of_in_predicate_value
(
  token_list *lst, void *st
)
{
  s_in_predicate_value *val = (s_in_predicate_value *)st;
  token_list *s;

  s = push_stack_of_table_subquery(lst, &val->tb_subquery);
  if (s) {
    val->is_talbe_subquery = Y;
    fe_return(s);
  }

  if (match(s->next, "<left paren>")) {
    next_token(s);

    s = push_stack_of_in_value_list(s->next, &val->in_val_list);
    if (!s) return NULL;
    return s->next;
  }

  return NULL;
}

static token_list*
push_stack_of_in_predicate
(
  token_list *lst, void *st
)
{
  s_in_predicate *pred = (s_in_predicate *)st;
  token_list *s;
  
  s = push_stack_of_row_value_constructor(lst, &pred->row_val_cstr1);
  if (!s) return NULL;
  
  if (match(s->next, "NOT")) {
    pred->has_not = Y;
    next_token(s);
  }

  if (!match(s->next, "IN")) return NULL;
  next_token(s);

  s = push_stack_of_in_predicate_value(s->next, &pred->in_pred_value);
  fe_return(s);
}

static token_list*
push_stack_of_match_value
(
  token_list *lst, void *st  
)
{
  return push_stack_of_character_value_expression(lst, st);
}

static token_list*
push_stack_of_pattern
(
  token_list *lst, void *st
)
{
  return push_stack_of_character_value_expression(lst, st);
}

static token_list*
push_stack_of_escape_character
(
  token_list *lst, void *st
)
{
  return push_stack_of_character_value_expression(lst, st);
}


static token_list*
push_stack_of_like_predicate
(
  token_list *lst, void *st
)
{
  s_like_predicate *pred = (s_like_predicate *)st;
  token_list *s;

  s = push_stack_of_match_value(lst, &pred->match_val);
  if (!s) return NULL;

  if (match(s->next, "NOT")) {
    pred->has_not = Y;
    next_token(s);
  }

  if (!match(s->next, "LIKE")) return NULL;
  next_token(s);

  s = push_stack_of_pattern(s->next, &pred->pt);
  if (!s) return NULL;

  if (match(s->next, "ESCAPE")) {
    pred->has_escape_character = Y;
    next_token(s);

    s = push_stack_of_escape_character(s->next, &pred->esc_character);
  }

  fe_return(s);
}

static token_list*
push_stack_of_null_predicate
(
  token_list *lst, void *st
)
{
  s_null_predicate *pred = (s_null_predicate *)st;
  token_list *s;

  s = push_stack_of_row_value_constructor(lst, &pred->row_val_cstr);
  if (!s) return NULL;

  if (!match(s->next, "IS")) return NULL;
  next_token(s);

  if (match(s->next, "NOT")) {
    pred->has_not = Y;
    next_token(s);
  }

  next_token(s);

  fe_return(s);
}

static token_list*
push_stack_of_quantified_comparison_predicate
(
  token_list *lst, void *st
)
{
  return NULL;
}

static token_list*
push_stack_of_exist_predicate
(
  token_list *lst, void *st
)
{
  s_exists_predicate *pred = (s_exists_predicate *)st;
  token_list *s;

  if (!match(lst, "EXISTS")) return NULL;
  next_token(lst);

  s = push_stack_of_table_subquery(lst, &pred->tb_subquery);
  fe_return(s);
}

static token_list*
push_stack_of_unique_predicate
(
  token_list *lst, void *st
)
{
  s_unique_predicate *pred = (s_unique_predicate *)st;
  token_list *s;
  
  if (!match(lst, "UNIQUE")) return NULL;
  next_token(lst);
  
  s = push_stack_of_table_subquery(lst, &pred->tb_subquery);
  fe_return(s);
}

static token_list*
push_stack_of_match_predicate
(
  token_list *lst, void *st
)
{
  s_match_predicate *pred = (s_match_predicate *)st;
  token_list *s;

  s = push_stack_of_row_value_constructor(lst, &pred->row_val_cstr);
  if (!s) return NULL;

  if (!match(s->next, "MATCH")) return NULL;
  next_token(s);

  if (match(s->next, "UNIQUE")) {
    pred->has_unique = Y;
    next_token(s);
  }

  if (match(s->next, "PARTIAL")) {
    pred->has_partial = Y;
    next_token(s);
  }
  else if (match(s->next, "FULL")) {
    pred->has_full = Y;
    next_token(s);
  }
  else {
  }

  s = push_stack_of_table_subquery(s->next, &pred->tb_subquery);
  fe_return(s);
}

static token_list*
push_stack_of_overlap_predicate
(
  token_list *lst, void *st
)
{
  s_overlaps_predicate *pred = (s_overlaps_predicate *)st;
  token_list *s;

  s = push_stack_of_row_value_constructor(lst, &pred->row_val_cstr_1);
  if (!s) return NULL;

  if (!match(s->next, "OVERLAPS")) return NULL;
  next_token(s);

  s = push_stack_of_row_value_constructor(s->next, &pred->row_val_cstr_2);
  fe_return(s);
}

static token_list*
push_stack_of_predicate
(
  token_list *lst, void *st
)
{
  s_predicate *pred = (s_predicate *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_comparison_predicate(lst, &pred->cmp_predicate);
  if (s) {
    pred->is_comparison_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_between_predicate(lst, &pred->btw_predicate);
  if (s) {
    pred->is_between_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_in_predicate(lst, &pred->a_in_predicate);
  if (s) {
    pred->is_in_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_like_predicate(lst, &pred->lk_predicate);
  if (s) {
    pred->is_like_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_null_predicate(lst, &pred->a_null_predicate);
  if (s) {
    pred->is_null_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_quantified_comparison_predicate(lst, &pred->qcmp_predicate);
  if (s) {
    pred->is_quantified_comparison_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_exist_predicate(lst, &pred->a_exist_predicate);
  if (s) {
    pred->is_exist_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_unique_predicate(lst, &pred->a_unique_predicate);
  if (s) {
    pred->is_unique_predicate = Y;
    fe_return(s);
  }

  s = push_stack_of_match_predicate(lst, &pred->a_match_predicate);
  if (s) {
    pred->is_match_predicate = Y;
    fe_return(s);
  }
  
  s = push_stack_of_overlap_predicate(lst, &pred->a_overlap_predicate);
  if (s) {
    pred->is_overlap_predicate = Y;
    fe_return(s);
  }  

  return NULL;
}

static token_list*
push_stack_of_boolean_primary
(
  token_list *lst, void *st
)
{
  s_boolean_primary *prim = (s_boolean_primary *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_predicate(lst, &prim->pred);
  if (s) {
    prim->is_predicate = Y;
    fe_return(s);
  }

  if (!match(lst, "<left paren>")) return NULL;
  next_token(lst);

  prim->se_condition = gc_malloc(sizeof(s_search_condition));
  if (!prim->se_condition) return NULL;
  
  s = push_stack_of_search_condition(lst, prim->se_condition);
  if (!s) return NULL;
  
  if (!match(s->next, "<right paren>")) return NULL;
  next_token(s);

  prim->is_search_condition = Y;
  
  fe_return(s);
}

static token_list*
push_stack_of_truth_value
(
  token_list *lst, void *st
)
{
  s_truth_value *val = (s_truth_value *)st;
  
  if (match(lst, "TRUE")) {
    val->is_true = Y;
  }
  else if (match(lst, "FALSE")) {
    val->is_false = Y;
  }
  else if (match(lst, "UNKNOWN")) {
    val->is_unknown = Y;
  }
  else {
    return NULL;
  }

  fe_return(lst);
}

static token_list*
push_stack_of_boolean_test
(
  token_list *lst, void *st
)
{
  s_boolean_test *test = (s_boolean_test *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_boolean_primary(lst, &test->bool_prim);
  if (!s) return NULL;

  if (match(s->next, "IS")) {
    next_token(s);

    if (match(s->next, "NOT")) {
      test->has_not = Y;
      next_token(s);
    }

    s = push_stack_of_truth_value(s->next, &test->bool_prim);
  }

  fe_return(s);
}

static token_list*
push_stack_of_boolean_factor
(
  token_list *lst, void *st
)
{
  s_boolean_factor *ft = (s_boolean_factor *)st;
  token_list *s;

  fs();

  if (match(lst, "NOT")) {
    ft->has_not = Y;
    next_token(lst);
  }

  s = push_stack_of_boolean_test(lst, &ft->bool_test);
  
  fe_return(s);
}

static token_list*
push_stack_of_boolean_term1
(
  token_list *lst, void *st
)
{
  s_boolean_term1 *term = (s_boolean_term1 *)st;
  token_list *s;

  fs();
  
  if (match(lst, "@")) fe_return(lst);

  if (!match(lst, "AND")) return NULL;
  next_token(lst);

  s = push_stack_of_boolean_factor(lst, &term->bool_factor);
  if (!s) return NULL;
  lst = s;

  term->next = gc_malloc(sizeof(s_boolean_term1));
  if (!term->next) return NULL;
  term->next->next = NULL;

  s = push_stack_of_boolean_term1(s->next, term->next);
  if (s) {
    lst = s;
  }
  else {
    term->next = NULL;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_boolean_term
(
  token_list *lst, void *st
)
{
  s_boolean_term *term = (s_boolean_term *)st;
  token_list *s;

  fs();
  
  s = push_stack_of_boolean_factor(lst, &term->bool_factor);
  if (!s) return NULL;
  lst = s;

  s = push_stack_of_boolean_term1(s->next, &term->bool_term1);
  if (s) {
    term->has_and = Y;
    lst = s;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_search_condition1
(
  token_list *lst, void *st
)
{
  s_search_condition1 *condition = (s_search_condition1 *)st;
  token_list *s;

  fs();

  if (match(lst, "@")) fe_return(lst);

  if (!match(lst, "OR")) return NULL;
  next_token(lst);

  s = push_stack_of_boolean_term(lst, &condition->bool_term);
  if (!s) return NULL;
  lst = s;

  condition->next = gc_malloc(sizeof(s_search_condition1));
  if (!condition->next) return NULL;

  s = push_stack_of_search_condition1(s->next, condition->next);
  if (s) {
    lst = s;
  }
  else {
    condition->next = NULL;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_search_condition
(
  token_list *lst, void *st
)
{
  s_search_condition *cond = (s_search_condition *)st;
  token_list *s;

  fs();

  s = push_stack_of_boolean_term(lst, &cond->bool_term);
  if (!s) return NULL;
  lst = s;

  s = push_stack_of_search_condition1(s->next, &cond->se_condition_1);
  if (s) {
    cond->has_or = Y;
    debug("has_or.\n");
    
    lst = s;
  }
  else {
    cond->has_or = !Y;
  }
  
  fe_return(lst);
}

static token_list*
push_stack_of_delete_statement_searched
(
  token_list *lst, void *st
)
{
  s_delete_statement_searched *stm = (s_delete_statement_searched *)st;
  token_list *s;
  
  fs();
  
  if (!match2(lst, "DELETE", "FROM")) return NULL;
  next_token(lst);
  next_token(lst);
  
  s = push_stack_of_table_name(lst, &stm->tb_name);
  if (!s) return NULL;

  if (match(s->next, "WHERE")) {
    next_token(s);
    
    s = push_stack_of_search_condition(s->next, &stm->se_condition);
    if (!s) return NULL;
    stm->has_search_condition = Y;
  }

  fe_return(s);
}

static token_list*
push_stack_of_object_column
(
  token_list *lst, void *st
)
{
  return push_stack_of_column_name(lst, st);
}

static token_list*
push_stack_of_update_source
(
  token_list *lst, void *st
)
{
  s_update_source *src = (s_update_source *)st;
  token_list *s;

  s = push_stack_of_value_expression(lst, &src->val_expr);
  if (s) {
    src->is_value_expression = Y;
    fe_return(s);
  }

  if (match(lst, "NULL")) {
    src->is_null_specification = Y;
    fe_return(s);
  }

  if (match(lst, "DEFAULT")) {
    src->is_default = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_set_clause
(
  token_list *lst, void *st
)
{
  s_set_clause *clause= (s_set_clause *)st;
  token_list *s;

  s = push_stack_of_object_column(lst, &clause->object_cl);
  if (!s) return NULL;

  if (!match(s->next, "<equals operator>")) return NULL;
  next_token(s);

  s = push_stack_of_update_source(s->next, &clause->upd_src);
  fe_return(s);
}


static token_list*
push_stack_of_set_clause_list
(
  token_list *lst, void *st
)
{
  s_set_clause_list *clst = (s_set_clause_list *)st;
  token_list *s;
  s_set_clause_list *next;

  s = push_stack_of_set_clause(lst, &clst->set_cls);
  if (!s) return NULL;

  next = clst->next = gc_malloc(sizeof(s_set_clause_list));
  if (!next) return NULL;

  while (match(s->next, "<comma>")) {
    next_token(s);

    s = push_stack_of_set_clause(s->next, &next->set_cls);
    if (!s) return NULL;

    next = next->next = gc_malloc(sizeof(s_set_clause_list));
    if (!next) return NULL;
  }

  fe_return(s);
}

static token_list*
push_stack_of_update_statement_searched
(
  token_list *lst, void *st
)
{
  s_update_statement_searched *stm = (s_update_statement_searched *)st;
  token_list *s;

  fs();

  if (!match(lst, "UPDATE")) return NULL;
  next_token(lst);

  s = push_stack_of_table_name(lst, &stm->tb_name);
  if (!s) return NULL;

  if (!match(s->next, "SET")) return NULL;
  next_token(s);

  s = push_stack_of_set_clause_list(s->next, &stm->set_cls_list);
  if (!s) return NULL;

  if (match(s->next, "WHERE")) {
    next_token(s);

    s = push_stack_of_search_condition(s->next, &stm->se_condition);
    if (s) stm->has_search_condition = Y;
  }

  fe_return(s);
}

static token_list*
push_stack_of_unsigned_integer
(
  token_list *lst, void *st
)
{
  unsigned_integer *uint = (unsigned_integer *)st;

  fs();
  
  if (!match(lst, "<unsigned integer>")) return NULL;
  
  *uint = atoi(lst->tk.value);
  debug("%d \n", *uint);

  fe_return(lst);
}

static token_list*
push_stack_of_sort_key
(
  token_list *lst, void *st
)
{
  s_sort_key *key = (s_sort_key *)st;
  token_list *s;

  s = push_stack_of_column_name(lst, &key->cl_name);
  if (s) {
    key->is_column_name = Y;
    fe_return(s);
  }

  s = push_stack_of_unsigned_integer(lst, &key->uint);
  if (s) {
    key->is_unsigned_integer = Y;
    fe_return(s);
  }

  return NULL;
}

static token_list*
push_stack_of_ordering_specification
(
  token_list *lst, void *st
)
{
  s_ordering_specification *spec = (s_ordering_specification *)st;
  
  if (match(lst, "ASC")) {
    spec->is_asc = Y;
    fe_return(lst);
  }

  if (match(lst, "DESC")) {
    spec->is_desc= Y;
    fe_return(lst);
  }  

  return NULL;
}

static token_list*
push_stack_of_sort_specification
(
  token_list *lst, void *st
)
{
  s_sort_specification *spec = (s_sort_specification *)st;
  token_list *s;

  s = push_stack_of_sort_key(lst, &spec->skey);
  if (!s) return NULL;

  lst = s;
  s = push_stack_of_collate_clause(lst->next, &spec->collate_cls);
  if (s) {
    spec->has_collate_clause = Y;
  }
  else {
    s = lst;
  }

  lst = s;
  s = push_stack_of_ordering_specification(lst->next, &spec->order_spec);
  if (s) {
    spec->has_ordering_specification = Y;
  }
  else {
    s = lst;
  }

  fe_return(s);
}

static token_list*
push_stack_of_sort_specification_list
(
  token_list *lst, void *st
)
{
  s_sort_specification_list *slst = (s_sort_specification_list *)st;
  token_list *s;
  s_sort_specification_list *next;

  s = push_stack_of_sort_specification(lst, &slst->sort_spec);
  if (!s) return NULL;

  next = slst->next = gc_malloc(sizeof(s_sort_specification_list));
  if (!next) return NULL;
  
  while (match(s->next, "<comma>")) {
    next_token(s);
    
    s = push_stack_of_sort_specification(s->next, &next->sort_spec);
    if (!s) return NULL;
    
    next = next->next = gc_malloc(sizeof(s_sort_specification_list));
    if (!next) return NULL;
  }
  
  fe_return(s);
}

static token_list* 
push_stack_of_order_by_clause
(
  token_list *lst, void *st
)
{
  s_order_by_clause *clause = (s_order_by_clause *)st;
  token_list *s;

  fs();

  if (!match2(lst, "ORDER", "BY")) return NULL;
  next_token(lst); next_token(lst);

  s = push_stack_of_sort_specification_list(lst, &clause->sort_spec_list);

  fe_return(s);
}

static token_list*
push_stack_of_direct_select_statement_rows
(
  token_list *lst, void *st
)
{
  s_direct_select_statement_multiple_rows *stm = st;
  token_list *s;

  fs();

  s = push_stack_of_query_expression(lst, &stm->query_expr);
  if (!s) return NULL;
  lst = s;

  s = push_stack_of_order_by_clause(s->next, &stm->order_by_cls);
  if (s) {
    stm->has_order_by_clause = Y;
    lst = s;
  }

  fe_return(lst);
}


static HTAB_INFO obj_htab;

static int
push_obj_htab(char *key, s_object *obj)
{
  ENTRY item, *rti;

  fs();
  
  item.key = key;
  rti = hsearch(&obj_htab, item, FIND);
  if (rti) return 0;

  item.data = obj;
  rti = hsearch(&obj_htab, item, ENTER);

  rti ? fe() : NULL;
  
  return !!rti;
}

s_object*
pop_obj_htab(char *key)
{
  ENTRY item, *rti;

  fs();
  
  item.key = key;
  rti = hsearch(&obj_htab, item, FIND);
  if (!rti) return NULL;

  fe();
  return rti->data;
}

int
init_stack(void)
{
  int rt;
  char *key;
  s_object *obj;
  
  fs();

  rt = hcreate(&obj_htab, 32);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;
  
  key = "CREATE";
  obj->name = "<table definition>";
  obj->push = push_stack_of_table_definition;
  obj->stack_size = sizeof(s_table_definition);
  obj->execute = create_table;
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;

  key = "DROP";
  obj->name = "<drop table statement>";
  obj->push = push_stack_of_drop_table_statement;
  obj->stack_size = sizeof(s_drop_table_statement);
  obj->execute = drop_table;
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;

  key = "ALTER";
  obj->name = "<alter table statement>";
  obj->push = push_stack_of_alter_table_statement;
  obj->stack_size = sizeof(s_alter_table_statement);
  obj->execute = alter_table;
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;
  
  key = "INSERT";
  obj->name = "<SQL data change statement>";
  obj->push = push_stack_of_insert_statement;
  obj->stack_size = sizeof(s_insert_statement);
  obj->execute = insert_table;
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;
  
  key = "DELETE";
  obj->name = "<SQL data change statement>";
  obj->push = push_stack_of_delete_statement_searched;
  obj->stack_size = sizeof(s_delete_statement_searched);
  rt = push_obj_htab(key, obj);
  if (!rt) return 0; 
  
  obj = malloc(sizeof(s_object));
  if (!obj) return 0;
  
  key = "UPDATE";
  obj->name = "<SQL data change statement>";
  obj->push = push_stack_of_update_statement_searched;
  obj->stack_size = sizeof(s_update_statement_searched);
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  obj = malloc(sizeof(s_object));
  if (!obj) return 0;
  
  key = "SELECT";
  obj->name = "<direct select statement: multiple rows>";
  obj->push = push_stack_of_direct_select_statement_rows;
  obj->stack_size = sizeof(s_direct_select_statement_multiple_rows);
  obj->execute = select_table;
  rt = push_obj_htab(key, obj);
  if (!rt) return 0;

  debug("stack hash table %d entries, %d entries used, %d entries free. \n\n", 
               obj_htab.size, obj_htab.filled, obj_htab.size - obj_htab.filled); 

  fe();
  return 1;
}

int
push_stack(token_list *lst, s_object *obj)
{
  token_list *s;

  fs();

  obj->stack = gc_malloc(obj->stack_size);
  if (!obj->stack) return 0;

  s = obj->push(lst->next, obj->stack); 
  if (!s) return 0;

  fe();
  return 1;
}

