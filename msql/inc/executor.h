
#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

int create_table(void *stack);
int drop_table(void *stack);
int alter_table(void *stack);
int insert_table(void *stack);
int select_table(void *stack);

#endif /* __EXECUTOR_H__ */

