#ifndef __DEFINITIONS_H_
#define __DEFINITIONS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"
#define EXPR_MALLOC (expression *)malloc(sizeof(expression))

typedef enum expression_t { 
	/*0*/TYPE_CONST, /*1*/TYPE_INT , /*2*/TYPE_FLOAT , /*3*/TYPE_STRING , /*4*/TYPE_BOOL , /*5*/TYPE_CONST_BOOL , 
	/*6*/TYPE_NIL , /*7*/TYPE_VAR , /*8*/TYPE_PROGRAM_FUNCTION , /*9*/TYPE_LIBRARY_FUNCTION ,
	/*10*/TYPE_TABLE_ELEMENT , /*11*/TYPE_NEWTABLE , /*12*/TYPE_TEMP_VAR , /*13*/TYPE_ERROR , /*14*/TYPE_LABEL } expression_t;
/*typedef struct { char* id; } Assignment;*/

typedef enum bool_t { B_FALSE , B_TRUE } bool_t;


typedef struct expression {
	expression_t         type;
	bool_t               is_true;
	struct expression*   index;
	num_list* 	     true_list;
	num_list* 	     false_list;

	union {
		hashRecord*  symbol;
		int          i_value;
		double       f_value;
		char*        str_value;
		bool_t       bool_value;
		unsigned     label;
	}expr_value;
} expression;

typedef enum iopcode {
        OP_ASSIGN/*0*/,         OP_ADD/*1*/,             OP_SUB/*2*/,             OP_MUL/*3*/,            OP_DIV/*4*/,        OP_MOD/*5*/,
	OP_UMINUS/*6*/,         OP_AND/*7*/,             OP_OR/*8*/,              OP_NOT/*9*/,            OP_IF_EQ/*10*/,     OP_IF_NOT_EQ/*11*/,
	OP_IF_LESS_EQ/*12*/,    OP_IF_GREATER_EQ/*13*/,  OP_IF_LESS/*14*/,        OP_IF_GREATER/*15*/,    OP_CALL/*16*/,      OP_PARAM/*17*/,
	OP_RET/*18*/,           OP_GET_RET_VAL/*19*/,    OP_FUNC_START/*20*/,     OP_FUNC_END/*21*/,      OP_TABLE_CREATE/*22*/,
	OP_TABLE_GET_ELEM/*23*/,OP_TABLE_SET_ELEM/*24*/, OP_JUMP/*25*/
} iopcode;

typedef struct quad {
	iopcode  	   op;
	expression*	   result;
	expression*	   arg1;
	expression*	   arg2;
	unsigned           label;
	unsigned           line;
	unsigned 	   taddress;
} quad;

typedef struct expr_list {

	expression* exp;
	struct expr_list* next;
}expr_list;

typedef struct expr_list_list {

	expr_list* exp;
	struct expr_list_list* next;
}expr_list_list;

typedef struct bc_stack{
	stack_num*  break_stack;
	stack_num*  continue_stack;
	stack_num*  num_of_breaks_stack;
	stack_num*  num_of_continues_stack;
	stack_num* for_start_stack;
	stack_num*  while_end_label_stack;
	struct bc_stack* next;
}bc_stack;

void expand (void);
void emit ( iopcode op,
	    expression* result,
            expression* arg1,
            expression* arg2,
            unsigned label,
            unsigned line );
scopespace_t currScopeSpace(void);
unsigned currScopeOffset (void);
void incCurrScopeOffset (void);
void enterScopeSpace (void);
void exitScopeSpace (void);
char* new_temp_name (void);
void reset_temp (void);
void resetFormalArgsOffset(void);
void resetFunctionLocalsOffset(void);
void restoreCurrScopeOffset( unsigned n);
hashRecord* new_temp (void);
expression* new_expr_void (void);
expression* new_expr_expr (const expression* expr);
expression* new_jump_expr ( unsigned label);
expression* new_bool_expr ( bool_t value);
expression* new_const_int_expr ( int value);
expression* new_const_string_expr(char* str);
expression* emit_relop(iopcode op , int line , expression* expr1,expression* expr2);
expression* emit_if_table_item(expression* expr,unsigned label,int line); 
int check_types(int type1,int type2,int op);
expression* new_expr_symbol (hashRecord* a);
expr_list* newItem(expression* expr);
expr_list* insertExprList(expr_list* head,expr_list* item);
expr_list* push_expr( expr_list* head, expression* new_node);
expr_list* pop_expr(expr_list* head );
expr_list_list* push_expr_list( expr_list_list* head, expr_list* new_node);
expr_list_list* pop_expr_list(expr_list_list* head );
bc_stack* push_bc_stack(bc_stack* head,
		stack_num* break_stack,stack_num* continue_stack,stack_num* num_of_breaks_stack,stack_num* num_of_continues_stack,
		stack_num* for_start_stack,stack_num*  while_end_label_stack);
bc_stack* pop_bc_stack(bc_stack* head);
num_list* make_list( unsigned QuadNo);
num_list* merge_list( num_list* list1,num_list* list2);
num_list* patch_list_label(num_list* list , unsigned label);
void print_list(num_list* list);
void print_quad(quad* q);
void printQuads(void);

void generate_t (void);
#endif
