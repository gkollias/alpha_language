#ifndef __AVM_H__ 
#define __AVM_H__ 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define AVM_STACKSIZE 4096 
#define AVM_WIPEOUT(m) memset( &(m) , 0 , sizeof(m) )  
#define AVM_STACKENV_SIZE 4 
#define AVM_NUMACTUALS_OFFSET 4 
#define AVM_SAVEDPC_OFFSET    3 
#define AVM_SAVEDTOP_OFFSET   2 
#define AVM_SAVEDTOPSP_OFFSET 1 
#define AVM_TABLE_HASHSIZE    211 


/********************************************************/
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

/*******************************************************/


typedef enum avm_memcell_t 
{ 
	number_m /*0*/, string_m /*1*/, bool_m /*2*/, table_m /*3*/, userfunc_m /*4*/, libfunc_m /*5*/, nil_m /*6*/, undef_m/*7*/ 
}avm_memcell_t; 
 
 
struct avm_table; 
 
typedef struct avm_memcell 
{ 
	avm_memcell_t type; 
 
	union 
	{ 
		double        numVal; 
		char*         strVal; 
		unsigned char boolVal;//bool_t 
		struct avm_table*    tableVal; 
		unsigned      funcVal; 
		char*         libfuncVal; 
	}data; 
 
}avm_memcell; 
 
 
typedef struct avm_table_bucket 
{ 
	avm_memcell key; 
	avm_memcell value; 
	struct avm_table_bucket* next; 
}avm_table_bucket; 
 
 
typedef struct avm_table 
{ 
	unsigned       refCounter; 
	
	avm_table_bucket*  strIndexed[AVM_TABLE_HASHSIZE]; 
	avm_table_bucket*  numIndexed[AVM_TABLE_HASHSIZE]; 
	avm_table_bucket*  funcIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket*  boolIndexed[2];
	avm_table_bucket*  tableIndexedHead;
	
	unsigned       total; 
}avm_table; 
 
typedef char* (*tostring_func_t)(avm_memcell *); 
typedef void (*execute_func_t)(instruction *); 
typedef void (*memclear_func_t)(avm_memcell *); 
typedef void (*library_func_t)(void); 
typedef double (*arithmetic_func_t)(double x,double y); 
typedef unsigned char (*logical_func_t)(double x,double y); 
typedef unsigned char (*tobool_func_t)(avm_memcell *);

 
/*Memory*/
avm_memcell stack[AVM_STACKSIZE]; 
/*Registers*/
avm_memcell ax,bx,cx; 
avm_memcell retval; 
unsigned    top,topsp; 
 
void avm_initstack(void); 
avm_memcell* avm_translate_operand(vmarg* arg , avm_memcell* reg); 
void avm_assign(avm_memcell* lv,avm_memcell* res); 
 
void avm_tabledecrefcounter(avm_table* t); 
void avm_tableincrefcounter(avm_table* t); 
void avm_tablebucketsinit(avm_table_bucket** p); 
avm_table* avm_tablenew(void); 
void avm_tablebucketdestroy(avm_table_bucket** p); 
void avm_tabledestroy(avm_table* t); 
int numHash(double num); 
int strHash(char* str); 
avm_table_bucket* new_table_bucket(void); 
 
 
avm_memcell* avm_tablegetelem(avm_table* table , avm_memcell* index); 
void avm_tablesetelem(avm_table* table , avm_memcell* index , avm_memcell* content); 
 
 
void avm_initialize(void); 
void avm_error(); 
char *avm_tostring(avm_memcell* m); 
unsigned avm_totalactuals(void); 
avm_memcell* avm_getactual(unsigned i); 
unsigned char avm_tobool(avm_memcell* mem); 
 
/*funcs*/ 
void avm_calllibfunc(char* funcName); 
void avm_callsaveenviroment(void); 
void avm_dec_top(void); 
void avm_push_envvalue(unsigned val); 
unsigned avm_get_envvalue(unsigned i); 
userfunc* avm_getfuncinfo(unsigned address);//todo 
 
 
library_func_t avm_getlibraryfunc(char* id); 
void avm_registerlibfunc(char* id,void* address);//todo  
void avm_calllibfunc(char *id);
void avm_calltable(avm_table* t);

void avm_memclear_string(avm_memcell* m); 
void avm_memclear_table(avm_memcell* m); 
void avm_memcellclear(avm_memcell* m); 
 
 
/*TODO*/ 
double consts_getnumber(unsigned index); 
char* consts_getstring(unsigned index); 
char* libfuncs_getused(unsigned index);

/*Read Binary File Function*/
void avm_load_instructions(instruction* i);
void avm_read_vmarg(vmarg* arg,FILE* fp);
void avm_load_const_nums(void);
void avm_load_const_strings(void);
void avm_load_user_funcs(void);
void avm_load_lib_funcs(void);
void avm_read_binary(const char* binary_file_name);


#endif 
