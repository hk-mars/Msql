
#ifndef __STACK_H__
#define __STACK_H__

#include "lex.h"

/*
*define the  elements of MSQL stack, which comes from and MUST be 
*consistent with the MSQL BNF in 'sql2_bnf.txt' file.
*/

typedef unsigned char flag_t;

typedef 
enum
{
  GLOBAL = 1,
  LOCAL
}
e_table_domain;

typedef e_table_domain s_temporary_t;

typedef char* identifier_t;

typedef identifier_t catalog_name_t;
typedef identifier_t unqualified_schema_name_t;
typedef identifier_t qualified_identifier_t;

typedef
struct schema_name
{
  flag_t has_catalog_name_t;
  catalog_name_t ctl_name;
  unqualified_schema_name_t unqlf_scm_name;
}
s_schema_name;

typedef
struct qualified_name
{
  flag_t has_schema_name;
  s_schema_name scm_name;
  qualified_identifier_t qlf_identifier;
}
s_qualified_name;

typedef qualified_identifier_t local_table_name_t;

typedef
struct qualified_local_table_name
{
  local_table_name_t lc_tb_name;
}
s_qualified_local_table_name;

typedef
struct table_name
{
  flag_t is_qualified_name;
  s_qualified_name qlf_name;
  s_qualified_local_table_name q_lc_name;
}
s_table_name;

typedef identifier_t s_column_name;
typedef unsigned int unsigned_integer;
typedef unsigned_integer s_length ;

typedef
struct character_string_type
{
  flag_t is_character_type;
  flag_t is_char_type;
  flag_t is_character_varying_type;
  flag_t is_char_varying_type;
  flag_t is_varchar_type;
  flag_t has_length;
  s_length length;
}
s_character_string_type;

typedef
struct character_set_specification
{
}
s_character_set_specification;

typedef
struct national_character_string_type
{
  flag_t is_national_character_type;
  flag_t is_national_char_type;
  flag_t is_national_character_varying_type;
  flag_t is_national_char_varying_type;
  flag_t is_national_varchar_type;
  flag_t has_length;
  s_length length;
}
s_national_character_string_type;

typedef
enum
{
  BIT_T,
  BIT_VARYING_T
}
e_bit_string_type;

typedef
struct bit_string_type
{
  flag_t has_length;
  e_bit_string_type type;
  s_length length;
}
s_bit_string_type;

typedef unsigned_integer precision_t;
typedef unsigned_integer scale_t;

typedef
struct exact_numeric_type
{
  flag_t is_dec_type;
  flag_t is_int_type;
  
  flag_t has_precision;
  precision_t precision;
  flag_t has_scale;
  scale_t scale;
}
s_exact_numeric_type;

typedef
struct approximate_numeric_type
{
  flag_t is_float_type;
  flag_t is_real_type;
  flag_t is_double_type;
  
  flag_t has_precision;
  precision_t precision;  
}
s_approximate_numeric_type;

typedef
struct numeric_type
{
  flag_t is_exact_numeric_type;
  flag_t is_appro_numeric_type;
  s_exact_numeric_type exact_num_type;
  s_approximate_numeric_type appr_num_type;
}
s_numeric_type;

typedef unsigned_integer time_fractional_seconds_precision_t;
typedef time_fractional_seconds_precision_t time_precision_t;
typedef time_fractional_seconds_precision_t timestamp_precision_t;

typedef
struct datatime_type
{
  flag_t is_date_type;
  flag_t is_time_type;
  flag_t is_timestamp_type;
  
  flag_t has_time_precision;
  time_precision_t tm_precision;
  flag_t has_timestamp_precision;
  timestamp_precision_t tmst_precision;
  flag_t has_time_zone;
}
s_datetime_type;

typedef
enum
{
  INTERVAL_T
}
e_interval_type;

typedef
enum
{
  YEAR_T,
  MONTH_T,
  DAY_T,
  HOUR_T,
  MINUTE_T
}
e_non_second_datetime_field;

typedef e_non_second_datetime_field non_second_datetime_field_t;
typedef unsigned_integer interval_leading_field_precision_t;

typedef
struct start_field
{
  non_second_datetime_field_t ns_dt_field;
  flag_t has_interval_leading_field_precision;
  interval_leading_field_precision_t inter_ld_field_precision;
}
s_start_field;

typedef unsigned_integer interval_fractional_seconds_precision_t;

typedef
struct end_field
{
  flag_t is_non_second_datetime_field;
  non_second_datetime_field_t ns_dt_field;
  flag_t has_interval_fractional_seconds_precision;
  interval_fractional_seconds_precision_t i_fr_sc_precision;
}
s_end_field;

typedef
struct single_datetime_field
{
  flag_t is_non_second_datetime_field;
  non_second_datetime_field_t ns_dt_field;
  flag_t has_interval_leading_field_precision;
  interval_leading_field_precision_t inter_ld_field_precision;
  flag_t has_interval_fractional_seconds_precision;
  interval_fractional_seconds_precision_t inter_fr_sc_precision;
}
s_single_datetime_field;

typedef
struct interval_qualifier
{
  s_start_field st_field;
  s_end_field en_field;
  flag_t has_single_datetime_field;
  s_single_datetime_field sg_dt_field;
}
s_interval_qualifier;

typedef
struct interval_type
{
  e_interval_type type;
  s_interval_qualifier qualifier;
}
s_interval_type;

typedef
struct data_type
{
  flag_t is_character_string_type;
  flag_t is_national_character_string_type;
  flag_t is_bit_string_type;
  flag_t is_numeric_type;
  flag_t is_datetime_type;
  flag_t is_interval_type;
  
  s_character_string_type ch_str_type;
  flag_t has_character_set_specification;
  s_character_set_specification ch_set_spec;

  s_national_character_string_type nt_ch_str_type;
  s_bit_string_type bit_str_type;
  s_numeric_type num_type;
  s_datetime_type date_type;
  s_interval_type inter_type;
}
s_data_type;

typedef s_qualified_name s_domain_name;

typedef
struct sign
{
  flag_t is_plus;
  flag_t is_minus;
}
s_sign;

typedef
struct exact_numeric_literal
{
  flag_t is_unsignd_integer;
  flag_t has_period;
  unsigned_integer uinteger;
  unsigned_integer mantissa;
  double dval;
}
s_exact_numeric_literal;

typedef s_exact_numeric_literal s_mantissa;
typedef signed int signed_int_t;
typedef signed_int_t s_exponent;

typedef
struct approximate_numeric_literal
{
  s_mantissa mantissa;
  s_exponent exponent;
}
s_approximate_numeric_literal;

typedef
struct unsigned_numeric_literal
{
  flag_t is_exact_numeric_literal;
  flag_t is_approximate_numeric_literal;
  s_exact_numeric_literal exact_num_lt;
  s_approximate_numeric_literal appr_num_lt;
}
s_unsigned_numeric_literal;

typedef
struct signed_numeric_literal
{
  flag_t has_sign;
  s_sign sign;
  s_unsigned_numeric_literal unsign_num_ltr;
}
s_signed_numeric_literal;

typedef
struct comment_introducer
{
  flag_t has_tow_minus_sign;
  flag_t has_three_plus_minus_sign;
}
s_comment_introducer;

typedef
struct comment
{
  s_comment_introducer cm_intr;
  char *content;
}
s_comment;

typedef
struct separator
{
  flag_t is_comment;
  flag_t is_space;
  flag_t is_newline;
  s_comment cm;
  struct separator *next;
}
s_separator;

typedef
struct character_string_literal
{
  flag_t has_character_set_specification;
  flag_t has_separator;
  s_character_set_specification ch_set_spec;
  s_separator sep;
  char *str;
}
s_character_string_literal;

typedef
struct national_character_string_literal
{
  flag_t has_separator;
  s_separator sep;
  char *str;
}
s_national_character_string_literal;

typedef
struct bit_string_literal
{
  char *str;
}
s_bit_string_literal;

typedef
struct hex_string_literal
{
  char *str;
}
s_hex_string_literal;

typedef
struct date_string
{
  char *str;
}
s_date_string;

typedef
struct date_literal
{
  s_date_string date_str;
}
s_date_literal;

typedef
struct time_string
{
  char *str;
}
s_time_string;

typedef
struct time_literal
{
  s_time_string time_str;
}
s_time_literal;

typedef
struct timestamp_string
{
  char *str;
}
s_timestamp_string;

typedef
struct timestamp_literal
{
  s_timestamp_string tms_str;
}
s_timestamp_literal;

typedef
struct datetime_literal
{
  flag_t is_date_literal;
  flag_t is_time_literal;
  flag_t is_timestamp_literal;
  s_date_literal dt_literal;
  s_time_literal tm_literal;
  s_timestamp_literal tms_literal;
}
s_datetime_literal;

typedef
struct interval_string
{
  char *str;
}
s_interval_string;

typedef
struct interval_literal
{
  flag_t has_sign;
  s_sign sign;
  s_interval_string inter_str;
  s_interval_qualifier inter_qlf;
}
s_interval_literal;

typedef
struct general_literal
{
  flag_t is_character_string_literal;
  flag_t is_national_string_literal;
  flag_t is_bit_string_literal;
  flag_t is_hex_string_literal;
  flag_t is_datetime_literal;
  flag_t is_interval_literal;
  s_character_string_literal ch_str_lt;
  s_national_character_string_literal nch_str_lt;
  s_bit_string_literal bit_str_lt;
  s_hex_string_literal hex_str_lt;
  s_datetime_literal date_lt;
  s_interval_literal inter_lt;
}
s_general_literal;

typedef
struct literal
{
  flag_t is_signed_numeric_literal;
  flag_t is_general_literal;
  s_signed_numeric_literal sign_num_ltr;
  s_general_literal gen_lt;
}
s_literal;

typedef
struct datetime_value_function
{
}
s_datetime_value_function;

typedef
struct default_option
{
  flag_t is_literal;
  flag_t is_datetime_value_function;
  flag_t is_user;
  flag_t is_current_user;
  flag_t is_session_user;
  flag_t is_system_user;
  flag_t is_null;
  s_literal ltr;
  s_datetime_value_function dt_vl_function;
}
s_default_option;

typedef
struct default_clause
{
	s_default_option def_option;
}
s_default_clause;

typedef s_qualified_name constraint_name_t;

typedef
struct constraint_name_definition
{
  constraint_name_t cst_name;
}
s_constraint_name_definition;

typedef
enum
{
  NOT_NULL_T,
  UNIQUE_SPECIFICATION_T,
  REFERENCE_SPECIFICATION_T,
  CHECK_CONSTRAINT_DEFINITION_T
}
e_column_constraint;

typedef
enum
{
  UNIQUE_T,
  PRIMARY_KEY_T
}
e_unique_specification;

typedef
struct reference_specification
{
}
s_reference_specification;

typedef
struct check_constraint_definition
{
}
s_check_constraint_definition;

typedef
struct column_constraint
{
  e_column_constraint type;
  e_unique_specification unique_spec;
  s_reference_specification ref_spec;
  s_check_constraint_definition check_cst_definition;
}
s_column_constraint;

typedef
struct constraint_attributes
{
}
s_constraint_attributes;

typedef
struct column_constraint_definition
{
  flag_t has_constraint_name_definition;
  s_constraint_name_definition cst_name_definition;
  s_column_constraint cl_constraint;
  flag_t has_constraint_attributes;
  s_constraint_attributes cst_attributes;
}
s_column_constraint_definition;

typedef s_qualified_name collate_name_t;

typedef
struct collate_clause
{
  collate_name_t collate_name;
}
s_collate_clause;

typedef
struct column_definition
{
  s_column_name cl_name;
  
  flag_t is_data_type;
  s_data_type dt_type;
  s_domain_name dm_name;
  
  flag_t has_default_clause;
  s_default_clause def_clause;
  
  flag_t has_column_contraint_definition;
  s_column_constraint_definition cl_cst_def;
  
  flag_t has_collate_clause;
  s_collate_clause clt_clause;
}
s_column_definition;

typedef
struct table_constraint_definition
{
}
s_table_constraint_definition;

typedef
struct table_element
{
  flag_t is_column_definition;
  s_column_definition cl_def;
  s_table_constraint_definition tb_cst_def;
}
s_table_element;

typedef
struct element_list
{
  s_table_element elm;
  struct element_list *next;
}
s_table_element_list;

typedef
struct table_definition
{
  flag_t is_temporary;
  s_temporary_t temporary;
  s_table_name tb_name;
  s_table_element_list elm_lst;
}
s_table_definition;

typedef
struct levels_clause
{
  flag_t is_cascaded;
  flag_t is_local;
}
s_levels_clause;

typedef
struct column_name_list
{
  s_column_name clname;
  struct column_name_list *next;
}
s_column_name_list;

typedef s_column_name_list s_view_column_list;

typedef
struct add_column_definition
{
  s_column_definition cl_def;
}
s_add_column_definition;

typedef
struct set_column_default_clause
{
  s_default_clause df_clause;
}
s_set_column_default_clause;

typedef
struct alter_column_action
{
  flag_t is_set_column_default_clause;
  flag_t is_drop_column_default_clause;
  s_set_column_default_clause set_default_clause;
}
s_alter_column_action;

typedef
struct alter_column_definition
{
  s_column_name cl_name;
  s_alter_column_action alter_cl_ac;
}
s_alter_column_definition;

typedef
struct drop_behavior
{
  flag_t is_cascade;
  flag_t is_restrict;
}
s_drop_behavior;

typedef
struct drop_column_definition
{

  s_column_name cl_name;
  s_drop_behavior drop_bh;
}
s_drop_column_definition;

typedef
struct add_table_constraint_definition
{
  s_table_constraint_definition cst_def;
}
s_add_table_constraint_definition;

typedef s_qualified_name s_constraint_name;

typedef
struct drop_table_constraint_definition
{
  s_constraint_name cst_name;
  s_drop_behavior drop_bh;
}
s_drop_table_constraint_definition;

typedef
struct alter_talbe_action
{
  flag_t is_add_column_definition;
  flag_t is_alter_column_definition;
  flag_t is_drop_column_definition;
  flag_t is_add_table_constraint_definition;
  flag_t is_drop_table_constraint_definition;
  
  s_add_column_definition add_cl_def;
  s_alter_column_definition alter_cl_def;
  s_drop_column_definition drop_cl_def;
  s_add_table_constraint_definition add_cst_def;
  s_drop_table_constraint_definition drop_cst_def;
}
s_alter_table_action;

typedef
struct alter_table_statement
{
  s_table_name tb_name;
  s_alter_table_action action;
}
s_alter_table_statement;

typedef
struct drop_table_statement
{
  s_table_name tb_name;
  s_drop_behavior drop_bhv;
}
s_drop_table_statement;

typedef
struct insert_column_list
{
  s_column_name_list cl_name_list;
}
s_insert_column_list;

typedef
struct unsigned_literal
{
  flag_t is_unsigned_numeric_literal;
  s_unsigned_numeric_literal us_num_lt;
}
s_unsigned_literal;

typedef
struct general_value_specification
{
}
s_general_value_specification;

typedef
struct unsigned_value_specification
{
  flag_t is_unsigned_literal;
  flag_t is_general_value_specification;
  s_unsigned_literal uns_literal;
  s_general_value_specification gen_val_spec;
}
s_unsigned_value_specification;

typedef identifier_t correlation_name_t;

typedef
struct qualifier
{
  flag_t is_table_name;
  flag_t is_correlation_name;
  s_table_name tb_name;
  correlation_name_t cr_name;
}
s_qualifier;

typedef
struct column_reference
{
  flag_t has_qualifier;
  s_qualifier qlf;
  s_column_name cname;
}
s_column_reference;

typedef
struct set_function_type
{
  flag_t is_avg;
  flag_t is_max;
  flag_t is_min;
  flag_t is_sum;
  flag_t is_count;
}
s_set_function_type;

typedef
struct set_quantifier
{
  flag_t is_all;
  flag_t is_distinct;
}
s_set_quantifier;

typedef
struct general_set_function
{
  s_set_function_type func_type;
}
s_general_set_function;

typedef
struct set_function_specification
{
  flag_t is_count;
  flag_t is_general_set_function;
  s_general_set_function set_func;
}
s_set_function_specification;

typedef
struct value_expression_primary
{
  flag_t is_unsigned_value_specification;
  flag_t is_column_reference;
  flag_t is_set_function_secification;
  flag_t is_paren_value_expression;
  s_unsigned_value_specification uns_val_spec;
  s_column_reference cl_ref;
  s_set_function_specification set_fun_spec;
  struct value_expression *paren_expr;
}
s_value_expression_primary;

typedef
struct numeric_value_function
{
}
s_numeric_value_function;

typedef 
struct numeric_primary
{
  flag_t is_value_expression_primary;
  flag_t is_numeric_value_function;
  s_value_expression_primary val_expr_prim;
  s_numeric_value_function num_val_func;
}
s_numeric_primary;

typedef
struct factor
{
  flag_t has_sign;
  s_sign sign;
  s_numeric_primary num_prim;
}
s_factor;

typedef
struct term1
{
  flag_t is_asterisk;
  flag_t is_solidus;
  flag_t is_end;
  s_factor ft;
  struct term1 *next;
}
s_term1;

typedef
struct term
{
  s_factor ft;
  s_term1 tm1;
}
s_term;

typedef
struct numeric_value_expression1
{
  flag_t is_plus_sign;
  flag_t is_minus_sign;
  flag_t is_end;
  s_term tm;
  struct numeric_value_expression1 *next;
}
s_numeric_value_expression1;

typedef
struct numeric_value_expression
{
  s_term tm;
  s_numeric_value_expression1 num_val_expr1;
}
s_numeric_value_expression;

typedef
struct string_value_function
{
}
s_string_value_function;


typedef
struct string_value_expression_primary
{
  flag_t is_character_string_literal;
  s_character_string_literal char_str_ltr;
}
s_string_value_expression_primary;

typedef
struct character_primary
{
  flag_t is_string_value_expression_primary;
  s_string_value_expression_primary str_val_expr_prim;
}
s_character_primary;

typedef
struct character_factor
{
  flag_t has_collate_clause;
  s_character_primary ch_prim;
  s_collate_clause clt_clause;
}
s_character_factor;

typedef
struct concatenation
{
}
s_concatenation;

typedef
struct character_value_expression
{
  s_character_factor ch_factor;
  s_concatenation concatenate;
}
s_character_value_expression;

typedef
struct bit_primary
{
  flag_t is_value_expression_primary;
  s_value_expression_primary *val_expr_prim;
}
s_bit_primary;

typedef s_bit_primary s_bit_factor;

typedef
struct bit_concatenation
{
}
s_bit_concatenation;

typedef
struct bit_value_expression
{
  s_bit_factor ch_factor;
  s_bit_concatenation bit_concaten;  
}
s_bit_value_expression;

typedef
struct string_value_expression
{
  flag_t is_character_value_expression;
  flag_t is_bit_value_expression;
  s_character_value_expression ch_val_expr;
  s_bit_value_expression bit_val_expr;
}
s_string_value_expression;

typedef
struct datetime_value_expression
{
}
s_datetime_value_expression;

typedef
struct interval_value_expression
{
}
s_interval_value_expression;

typedef
struct value_expression
{
  flag_t is_numeric_value_expression;
  flag_t is_string_value_expression;
  flag_t is_datetime_value_expression;
  flag_t is_interval_value_expression;
  s_numeric_value_expression num_val_expr;
  s_string_value_expression str_val_expr;
  s_datetime_value_expression date_val_expr;
  s_interval_value_expression inter_val_expr;
}
s_value_expression;

typedef
struct as_clause
{
  flag_t has_as;
  s_column_name cl_name;
}
s_as_clause;

typedef
struct derived_column
{
  flag_t is_as_clause;
  s_value_expression val_expr;
  s_as_clause as_cls;
}
s_derived_column;

typedef
struct select_sublist
{
  flag_t is_derived_column;
  flag_t is_qualifier;
  s_derived_column derived_cl;
  s_qualifier qlf;
}
s_select_sublist;

typedef
struct select_list
{
  flag_t is_asterisk;
  flag_t is_select_sublist;
  s_select_sublist slst;
  struct select_list *next; 
}
s_select_list;

typedef
struct table_reference
{
  s_table_name tb_name;
}
s_table_reference;

typedef
struct table_reference_list
{
  s_table_reference tb_ref;
  struct table_reference_list *next;
}
s_table_reference_list;

typedef
struct row_value_constructor_element
{
  flag_t is_value_expression;
  flag_t is_null_specification;
  flag_t is_default_specification;
  s_value_expression val_expr;
}
s_row_value_constructor_element;

typedef
struct row_value_constructor_list
{
  s_row_value_constructor_element row_val_elm;
  struct row_value_constructor_list *next;
}
s_row_value_constructor_list;

typedef
struct row_subquery
{
}
s_row_subquery;

typedef
struct row_value_constructor
{
  flag_t is_row_value_constructor_element;
  flag_t is_row_value_constructor_list;
  flag_t is_row_subquery;
  s_row_value_constructor_element row_val_cstr_elm;
  s_row_value_constructor_list row_val_list;
  s_row_subquery a_row_subquery;
}
s_row_value_constructor;

typedef
struct table_value_constructor_list
{
  s_row_value_constructor row_val_cstr;
  struct table_value_constructor_list *next;
}
s_table_value_constructor_list;

typedef
struct table_value_constructor
{
  s_table_value_constructor_list val_lst;
}
s_table_value_constructor;

typedef
struct explicit_table
{
  s_table_name tb_name;
}
s_explicit_table;

typedef
struct comp_op
{
  flag_t is_equal_op;
  flag_t is_not_equal_op;
  flag_t is_greater_than_op;
  flag_t is_less_than_op;
  flag_t is_less_than_or_equal_op;
  flag_t is_greater_than_or_equal_op;
}
s_comp_op;

typedef
struct comparison_predicate
{
  s_row_value_constructor lrow_val_cstr;
  s_comp_op op;
  s_row_value_constructor rrow_val_cstr;
}
s_comparison_predicate;

typedef
struct between_predicate
{
  flag_t has_not;
  s_row_value_constructor row_val_cstr1;
  s_row_value_constructor row_val_cstr2;
  s_row_value_constructor row_val_cstr3;
}
s_between_predicate;

typedef void* s_table_subquery;

typedef
struct in_value_list
{
  s_value_expression val_expr;
  struct in_value_list *next;
}
s_in_value_list;

typedef
struct in_predicate_value
{
  flag_t is_talbe_subquery;
  s_table_subquery tb_subquery;
  s_in_value_list in_val_list;
}
s_in_predicate_value;

typedef
struct in_predicate
{
  flag_t has_not;
  s_row_value_constructor row_val_cstr1;
  s_in_predicate_value in_pred_value;
}
s_in_predicate;

typedef s_character_value_expression s_match_value;
typedef s_character_value_expression s_pattern;
typedef s_character_value_expression s_escape_character;

typedef
struct like_predicate
{
  flag_t has_not;
  flag_t has_escape_character;
  s_match_value match_val;
  s_pattern pt;
  s_escape_character esc_character;
}
s_like_predicate;

typedef
struct null_predicate
{
  flag_t has_not;
  s_row_value_constructor row_val_cstr;
}
s_null_predicate;

typedef
struct quantifier
{
  flag_t is_all;
  flag_t is_some;
}
s_quantifier;

typedef
struct quantified_comparison_predicate
{
  s_row_value_constructor row_val_cstr;
  s_comp_op cmp_op;
  s_quantifier qtf;
  s_table_subquery tb_subquery;
}
s_quantified_comparison_predicate;

typedef
struct exists_predicate
{
  s_table_subquery tb_subquery;
}
s_exists_predicate;

typedef
struct unique_predicate
{
  s_table_subquery tb_subquery;
}
s_unique_predicate;

typedef
struct match_predicate
{
  flag_t has_unique;
  flag_t has_partial;
  flag_t has_full;
  s_row_value_constructor row_val_cstr;
  s_table_subquery tb_subquery;
}
s_match_predicate;

typedef
struct overlaps_predicate
{
  s_row_value_constructor row_val_cstr_1;
  s_row_value_constructor row_val_cstr_2;
}
s_overlaps_predicate;

typedef
struct predicate
{
  flag_t is_comparison_predicate;
  flag_t is_between_predicate;
  flag_t is_in_predicate;
  flag_t is_like_predicate;
  flag_t is_null_predicate;
  flag_t is_quantified_comparison_predicate;
  flag_t is_exist_predicate;
  flag_t is_unique_predicate;
  flag_t is_match_predicate;
  flag_t is_overlap_predicate;
  s_comparison_predicate cmp_predicate;
  s_between_predicate btw_predicate;
  s_in_predicate a_in_predicate;
  s_like_predicate lk_predicate;
  s_null_predicate a_null_predicate;
  s_quantified_comparison_predicate qcmp_predicate;
  s_exists_predicate a_exist_predicate;
  s_unique_predicate a_unique_predicate;
  s_match_predicate a_match_predicate;
  s_overlaps_predicate a_overlap_predicate;
}
s_predicate;

typedef void* p_search_condition;

typedef
struct boolean_primary
{
  flag_t is_predicate;
  flag_t is_search_condition;
  s_predicate pred;
  struct search_condition *se_condition;
}
s_boolean_primary;

typedef
struct truth_value
{
  flag_t is_true;
  flag_t is_false;
  flag_t is_unknown;
}
s_truth_value;

typedef
struct boolean_test
{
  flag_t has_truth_value;
  flag_t has_not;
  s_boolean_primary bool_prim;
  s_truth_value truth_val;
}
s_boolean_test;

typedef
struct boolean_factor
{
  flag_t has_not;
  s_boolean_test bool_test;
}
s_boolean_factor;

typedef
struct boolean_term1
{
  flag_t is_end;
  s_boolean_factor bool_factor;
  struct boolean_term1 *next;
}
s_boolean_term1;

typedef
struct boolean_term
{
  s_boolean_factor  bool_factor;
  s_boolean_term1 bool_term1;
  flag_t has_and;
}
s_boolean_term;

typedef
struct search_condition1
{
  flag_t is_end;
  s_boolean_term bool_term;
  struct search_condition1 *next;
}
s_search_condition1;

typedef
struct search_condition
{
  s_boolean_term bool_term;
  s_search_condition1 se_condition_1;
  flag_t has_or;
}
s_search_condition;

typedef
struct from_clause
{
  s_table_reference_list tb_ref_lst;
}
s_from_clause;

typedef
struct where_clause
{
  s_search_condition se_cond;
}
s_where_clause;

typedef
struct group_column_reference
{
  s_column_reference cl_ref;
}
s_group_column_reference;

typedef
struct group_column_reference_list
{
  s_group_column_reference grp_cl_ref;
  struct group_column_reference_list *next;
}
s_group_column_reference_list;

typedef
struct group_by_clause
{
  s_group_column_reference_list grp_cl_ref_lst;
}
s_group_by_clause;

typedef
struct having_clause
{
  s_search_condition se_cond;
}
s_having_clause;

typedef
struct table_expression
{
  flag_t has_where_clause;
  flag_t has_group_by_clause;
  flag_t has_having_clause;
  s_from_clause from_cls;
  s_where_clause where_cls;
  s_group_by_clause group_by_cls;
  s_having_clause having_cls;
}
s_table_expression;

typedef
struct query_specification
{
  flag_t has_set_quantifier;
  s_set_quantifier set_qtf;
  s_select_list select_lst;
  s_table_expression table_expr; 
}
s_query_specification;

typedef
struct simple_table
{
  flag_t is_query_specification;
  flag_t is_table_value_constructor;
  flag_t is_explicit_table;
  
  s_query_specification query_spec;
  s_table_value_constructor tb_val_cstr;
  s_explicit_table explicit_tb;
}
s_simple_table;

typedef
struct non_join_query_primary
{
  flag_t is_simple_table;
  s_simple_table sptable;
}
s_non_join_query_primary;

typedef
struct non_join_query_term
{
  flag_t is_non_join_query_primary;
  s_non_join_query_primary njoin_qprimary;
}
s_non_join_query_term;

typedef
struct non_join_query_expression
{
  flag_t is_non_join_query_term;
  s_non_join_query_term njoin_qterm;
}
s_non_join_query_expression;

typedef
struct joined_talbe
{
}
s_joined_talbe;

typedef
struct query_expression
{
  flag_t is_non_join_query_expression;
  flag_t is_joined_table;
  s_non_join_query_expression njoin_qexpr;
  s_joined_talbe join_table;
}
s_query_expression;

typedef
struct view_definition
{
  flag_t has_view_column_list;
  flag_t has_check_option;
  flag_t has_levels_clause;
  s_table_name tb_name;
  s_view_column_list view_column_lst;
  s_query_expression expr;
  s_levels_clause level_cls;
}
s_view_definition;

typedef
struct insert_columns_and_source
{
  flag_t has_query_expression;
  flag_t has_insert_column_list;
  s_insert_column_list insert_cl_lst;
  s_query_expression qexpression;
  flag_t is_default_values;
}
s_insert_columns_and_source;

typedef
struct insert_statement
{
  s_table_name tb_name;
  s_insert_columns_and_source insert_cl_src;
}
s_insert_statement;

typedef
struct delete_statement_searched
{
  flag_t has_search_condition;
  s_table_name tb_name;
  s_search_condition se_condition;
}
s_delete_statement_searched;

typedef s_column_name s_object_column;

typedef
struct update_source
{
  flag_t is_value_expression;
  flag_t is_null_specification;
  flag_t is_default;
  s_value_expression val_expr;
}
s_update_source;

typedef
struct set_clause
{
  s_object_column object_cl;
  s_update_source upd_src;
}
s_set_clause;

typedef
struct set_clause_list
{
  s_set_clause set_cls;
  struct set_clause_list *next;
}
s_set_clause_list;

typedef
struct update_statement_searched
{
  flag_t has_search_condition;
  s_table_name tb_name;
  s_set_clause_list set_cls_list;
  s_search_condition se_condition;
}
s_update_statement_searched;

typedef
struct sort_key
{
  flag_t is_column_name;
  flag_t is_unsigned_integer;
  s_column_name cl_name;
  unsigned_integer uint;
}
s_sort_key;

typedef
struct ordering_specification
{
  flag_t is_asc;
  flag_t is_desc;
}
s_ordering_specification;

typedef
struct sort_specification
{
  flag_t has_collate_clause;
  flag_t has_ordering_specification;
  s_sort_key skey;
  s_collate_clause collate_cls;
  s_ordering_specification order_spec;
}
s_sort_specification;

typedef
struct sort_specification_list
{
  s_sort_specification sort_spec;
  struct sort_specification_list *next;
}
s_sort_specification_list;

typedef
struct order_by_clause
{
  s_sort_specification_list sort_spec_list;
}
s_order_by_clause;

typedef
struct direct_select_statement_multiple_rows
{
  flag_t has_order_by_clause;
  s_query_expression query_expr;
  s_order_by_clause order_by_cls;
}
s_direct_select_statement_multiple_rows;


/*
*stack object.
*/
typedef token_list* (*PUSH_FUNC) (token_list*, void*);
typedef int (*EXE_FUNC) (void*);

typedef
struct object
{
  char *name;
  PUSH_FUNC push;
  void *stack;
  int stack_size;
  EXE_FUNC execute;
}
s_object;


int init_stack(void);
s_object* pop_obj_htab(char *key);
int push_stack(token_list *lst, s_object *obj);


#endif /* __STACK_H__ */

