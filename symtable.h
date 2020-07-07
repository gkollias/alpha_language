#ifndef __SYMTABLE_H_
#define __SYMTABLE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <assert.h>

#define HASHSIZE 52
#define META  0
#define FUNCT 1

#define LEGAL 1
#define ILLEGAL 0


#define FOUND 1
#define META_FOUND 2
#define FUNCT_FOUND 3
#define NOT_FOUND 0


typedef struct num_list{
	unsigned num;	
	struct num_list* next;
}num_list;

typedef enum symbol_t {
	VAR_S,
	PROGRAM_FUNCTION_S,
	LIBRARY_FUNCTION_S,
	TEMP_VAR_S

} symbol_t;

typedef enum scopespace_t {
	PROGRAM_VAR,
	FUNCTION_LOCAL,
	FORMAL_ARG
} scopespace_t;

typedef struct varType
{
	char* name;
}varType;

typedef struct arglist
{
	varType* var;
	struct arglist* next;
}arglist;

typedef struct funType
{
	char*     name;
	unsigned  i_address;
	int	  total_locals;
	arglist*  list;
}funType;

typedef struct hashRecord
{
	int isActive;
	
	union
	{
		varType* var;
		funType* fun;
	}types;

	int scope;
	unsigned line;
	scopespace_t  space;  //Originating scope space
	unsigned      offset; //Offset in scope space
	symbol_t type;
	unsigned  t_address;
	num_list* returnList;

	struct hashRecord* next;
}hashRecord;


typedef struct scope
{
	int defscope;
	hashRecord* sym[HASHSIZE];
	struct scope* next;

}scope;


typedef struct stack_num{
	int num;	
	struct stack_num* next;
}stack_num;



int hash(char* a);
varType* newVar(char *name);
funType* newFun(char *name,arglist* args);
arglist* newArg(varType* var);
hashRecord* newRec(void* kind,symbol_t type,int scope,unsigned line,unsigned offset,scopespace_t space);
scope *newSymtable(int scpe);
arglist* insertArgList(arglist* head,arglist* argument);
arglist* lookArgList(arglist* head,char* key);
hashRecord* insertHash(hashRecord* head,hashRecord* in);
scope *lookScopes(int Scope);
scope* insertNewScope(int scpe);
hashRecord* insertScopes(int scpe,hashRecord* in,char* key);
hashRecord* lookScope(char *key,int scpe);
void hide(int scpe);
void printList(hashRecord* head);
void printScope(int scpe);
void printArgList(arglist* head);
void printSymbolTable();
void libFunctions();
stack_num* push( stack_num* head, int funct_scope);
stack_num* pop(	stack_num* head );

#endif
