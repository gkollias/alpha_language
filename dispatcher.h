#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "avm.h"

#define MAX_INSTRUCTIONS (unsigned) VM_NOP


#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

#define execute_jle execute_logical
#define execute_jge execute_logical
#define execute_jlt execute_logical
#define execute_jgt execute_logical

/*typedef char* (*tostring_func_t)(avm_memcell *);
typedef void (*execute_func_t)(instruction *);
typedef void (*memclear_func_t)(avm_memcell *);
typedef void (*library_func_t)(void);
typedef double (*arithmetic_func_t)(double x,double y);
typedef unsigned char (*logical_func_t)(double x,double y);
typedef unsigned char (*tobool_func_t)(avm_memcell *);*/

//unsigned char executionFinished = 0;
unsigned pc = 0;
unsigned currLine = 0;
unsigned codeSize = 0;
unsigned totalActuals = 0;
instruction* code = (instruction *)0;

char* typeStrings[] = {
	
	"number",
	"string",
	"bool",
	"table",
	"userFunc",
	"libFunc",
	"nil",
	"undef"

};

#define AVM_ENDING_PC codeSize

void execute_cycle(instruction* i);

void execute_assign(instruction* i);//todo

void execute_add(instruction* i);
void execute_sub(instruction* i);
void execute_mul(instruction* i);
void execute_div(instruction* i);
void execute_mod(instruction* i);

void execute_jeq(instruction* i);
void execute_jne(instruction* i);

void execute_jle(instruction* i);
void execute_jge(instruction* i);
void execute_jlt(instruction* i);
void execute_jgt(instruction* i);

void execute_jump(instruction* i);

void execute_call(instruction* i);
void execute_pusharg(instruction* i);
void execute_funcenter(instruction* i);
void execute_funcexit(instruction* i);

void execute_newtable(instruction* i);
void execute_tablegetelem(instruction* i);
void execute_tablesetelem(instruction* i);

void execute_nop(instruction* i);


void libfunc_print(void);
void libfunc_input(void);
void libfunc_objectmemberkeys(void);
void libfunc_objecttotalmembers(void);
void libfunc_objectcopy(void);
void libfunc_totalarguments(void);
void libfunc_argument(void);
void libfunc_typeof(void);
void libfunc_strtonum(void);
void libfunc_sqrt(void);
void libfunc_cos(void);
void libfunc_sin(void);



char* number_tostring(avm_memcell* mem);
char* string_tostring(avm_memcell* mem);
char* bool_tostring(avm_memcell* mem);
char* table_tostring(avm_memcell* mem);
char* userfunc_tostring(avm_memcell* mem);
char* libfunc_tostring(avm_memcell* mem);
char* nil_tostring(avm_memcell* mem);
char* undef_tostring(avm_memcell* mem);




double add_impl(double x,double y);
double sub_impl(double x,double y);
double mul_impl(double x,double y);
double div_impl(double x,double y);
double mod_impl(double x,double y);

unsigned char jle_impl(double x,double y);
unsigned char jge_impl(double x,double y);
unsigned char jlt_impl(double x,double y);
unsigned char jgt_impl(double x,double y);



unsigned char number_tobool(avm_memcell *mem);
unsigned char string_tobool(avm_memcell *mem);
unsigned char bool_tobool(avm_memcell *mem);
unsigned char table_tobool(avm_memcell *mem);
unsigned char userfunc_tobool(avm_memcell *mem);
unsigned char libfunc_tobool(avm_memcell *mem);
unsigned char nil_tobool(avm_memcell *mem);
unsigned char undef_tobool(avm_memcell *mem);


execute_func_t executeFuncs[]={

	execute_assign         /*0*/,
	execute_add            /*1*/,
	execute_sub            /*2*/,
	execute_mul            /*3*/,
	execute_div            /*4*/,
	execute_mod            /*5*/,
	execute_jeq            /*6*/,
	execute_jne            /*7*/,
	execute_jle            /*8*/,
	execute_jge            /*9*/,
	execute_jlt            /*10*/,
	execute_jgt            /*11*/,
	execute_call           /*12*/,
	execute_pusharg        /*13*/,
	execute_funcenter      /*14*/,
	execute_funcexit       /*15*/,
	execute_newtable       /*16*/,
	execute_tablegetelem   /*17*/,
	execute_tablesetelem   /*18*/,
	execute_jump           /*19*/,
	execute_nop            /*20*/
};

extern void avm_memclear_string(avm_memcell* m);
extern void avm_memclear_table(avm_memcell* m);

memclear_func_t memclearFuncs[]={

	0,                   /* number */
	avm_memclear_string, /* string */
	0,                   /* bool */
	avm_memclear_table,  /* table */
	0,                   /* userfunc */
	0,                   /* libfunc */
	0,                   /* nil */
	0,                   /* undef */
};

tostring_func_t tostringFuncs[]={

	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring
};

arithmetic_func_t arithmeticFuncs[]={

	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl,
};

logical_func_t logicalFuncs[]={

	jle_impl,
	jge_impl,
	jlt_impl,
	jgt_impl,
};

tobool_func_t toboolFuncs[]={

	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};

typedef struct table_list
{
	avm_table* t;
	struct table_list* next;

}table_list;

void insert_table_list(avm_table* in);
int check_preced(avm_table* in);
void reset_table_list(void);

/******************** Equality check ***********************/
typedef unsigned char (*check_equal_func_t)(avm_memcell *,avm_memcell *); 

unsigned char number_check_equal(avm_memcell *mem1,avm_memcell *mem2);
unsigned char string_check_equal(avm_memcell *mem1,avm_memcell *mem2);
unsigned char table_check_equal(avm_memcell *mem1,avm_memcell *mem2);
unsigned char user_func_check_equal(avm_memcell *mem1,avm_memcell *mem2);
unsigned char lib_func_check_equal(avm_memcell *mem1,avm_memcell *mem2);

check_equal_func_t checkEqual[]={

	number_check_equal,
	string_check_equal,
	0,
	table_check_equal,
	user_func_check_equal,
	lib_func_check_equal,
	0,
	0
};

#endif
