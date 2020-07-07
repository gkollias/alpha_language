#ifndef _TARGET_H
#define _TARGET_H

#include "definitions.h"

typedef enum vmopcode {
        VM_ASSIGN/*0*/,    VM_ADD/*1*/,           VM_SUB/*2*/,             VM_MUL/*3*/,            VM_DIV/*4*/,        VM_MOD/*5*/,
	VM_JEQ/*6*/,       VM_JNE/*7*/,           VM_JLE/*8*/,             VM_JGE/*9*/,            VM_JLT/*10*/,       VM_JGT/*11*/,
	VM_CALL/*12*/,     VM_PUSH_ARG/*13*/,     VM_FUNC_ENTER/*14*/,     VM_FUNC_EXIT/*15*/,     VM_NEWTABLE/*16*/,
	VM_TABLE_GET_ELEM/*17*/,                  VM_TABLE_SET_ELEM/*18*/, VM_JUMP/*19*/,          VM_NOP/*20*/
} vmopcode;


typedef enum  vmarg_t {
	label_a ,  global_a , formal_a , local_a    , number_a  ,
	string_a , bool_a   , nil_a    , userfunc_a , libfunc_a ,
	retval_a , undef_a
}vmarg_t;


typedef struct vmarg {
	vmarg_t   type;
	unsigned  val;
}vmarg;


typedef struct instruction{
	vmopcode   opcode;
	vmarg      *result;
	vmarg      *arg1;
	vmarg      *arg2;
	unsigned   srcLine;
}instruction;


typedef struct userfunc {
	unsigned   address;
	unsigned   localSize;
	char*      id;
}userfunc;


typedef struct incomplete_jump {
	unsigned                 instrNo;//jump instr num
	unsigned                 iaddress;//the i code j_target address
	struct incomplete_jump*  next;//A trivial linked list
}incomplete_jump;

typedef struct symbol_list{
	hashRecord* symbol;
	struct symbol_list* next;
}symbol_list;

/***** Expanding the tables ******/
void i_expand (void);
void const_num_expand(void);
void const_string_expand(void);
void user_func_expand(void);
void lib_func_expand(void);

unsigned  consts_newstring(char *s); 
unsigned  consts_newnumber(double n);
//unsigned  libfuncs_newused(char *s);
//unsigned  userfuncs_newfunc(hashRecord *sym);
void make_operand(expression *e,vmarg *arg);
void make_number_operand(vmarg* arg,double val);
void make_bool_operand(vmarg* arg,bool_t val);
void make_retvaloperand(vmarg* arg);
void t_emit ( instruction* t );
vmarg* new_vmarg_void (void);

unsigned consts_newnumber(double val);
unsigned consts_newstring(char* str);
unsigned add_lib_functions(char* str);
unsigned add_user_functions(char* id,unsigned address,unsigned localSize);

/*************** Generate Functions *****************/
void generate(vmopcode op,quad* q);
void generate_relational (vmopcode op, quad* q);

void generate_ADD(quad *q);
void generate_SUB(quad *q);
void generate_MUL(quad *q);
void generate_DIV(quad *q);
void generate_MOD(quad *q);
void generate_NEWTABLE (quad *q);
void generate_TABLEGETELEM (quad *q);
void generate_TABLESETELEM (quad *q);
void generate_ASSIGN (quad *q);
void generate_NOP (void);

void generate_JUMP (quad* q);
void generate_IF_EQ (quad* q);
void generate_IF_NOTEQ(quad* q);
void generate_IF_GREATER (quad* q);
void generate_IF_GREATEREQ(quad* q);
void generate_IF_LESS (quad* q);
void generate_IF_LESSEQ (quad* q);

void generate_PARAM(quad* q);
void generate_CALL(quad* q);
void generate_GETRETVAL(quad* q);
void generate_FUNCSTART(quad* q);
void generate_FUNCEND(quad* q);
void generate_RETURN(quad* q);

void generate_NOT (quad* q);
void generate_OR (quad* q);
void generate_AND (quad* q);
void generate_UMINUS (quad* q);

/************ Jump Utility Functions ************/
void patch_incomplete_jumps();
void add_incomplete_jump (unsigned instrNo, unsigned iaddress);
num_list* patch_instruction_label(num_list* list , unsigned label);
symbol_list* pop_symbol(symbol_list* head);
symbol_list* push_symbol(symbol_list* head,hashRecord* new_symbol);
num_list* append_instruction_list(num_list* head,unsigned instrNo);
void reset_operand(vmarg* arg);

/******** Print Functions ************/
void print_instruction (instruction* i);
void printInstructions(void);
void fwrite_vmarg(vmarg* arg,FILE* fp);
void print_const_nums(void);
void print_const_strings(void);
void print_user_funcs(void);
void print_lib_funcs(void);

instruction* instruction_malloc( void );
#endif
