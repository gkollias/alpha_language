#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include<math.h>
#include "avm.h"

#define VM_MAGIC          1555
#define NULL_VMARG_VALUE  -1


unsigned char executionFinished = 0;

/**************** Tables *********************/
double*   numConsts = NULL;
unsigned  totalNumConsts = 0;
char**    stringConsts = NULL;
unsigned  totalStringConsts = 0;
char**    namedLibfuncs = NULL;
unsigned  totalNamedLibfuncs = 0;
userfunc* userFuncs = NULL;
unsigned  totalUserFuncs = 0;

/* Variables for binary file */
int magic   =  0;
int globals = -1;
unsigned int curr_instruction   = 0;
FILE* instr_bin_fp;
instruction *code;


extern unsigned pc; 
extern unsigned totalActuals;
extern unsigned int codeSize;
extern tobool_func_t toboolFuncs[];
extern tostring_func_t tostringFuncs[];
extern memclear_func_t memclearFuncs[];

extern void libfunc_typeof(void);
extern void libfunc_print(void);
extern void execute_cycle(instruction* i);

/***********************************Lib function dispatcher***************************************/
char* lib_funcs_names[] = {
	"print",
	"input",
	"objectmemberkeys",
	"objecttotalmembers",
	"objectcopy",
	"totalarguments",
	"argument",
	"typeof",
	"strtonum",
	"sqrt",
	"cos",
	"sin"
};

int get_lib_name_index(char *name);

extern void libfunc_print(void);
extern void libfunc_input(void);
extern void libfunc_objectmemberkeys(void);
extern void libfunc_objecttotalmembers(void);
extern void libfunc_objectcopy(void);
extern void libfunc_totalarguments(void);
extern void libfunc_argument(void);
extern void libfunc_typeof(void);
extern void libfunc_strtonum(void);
extern void libfunc_sqrt(void);
extern void libfunc_cos(void);
extern void libfunc_sin(void);


library_func_t libFuncs[] = {
	libfunc_print,
	libfunc_input,
	libfunc_objectmemberkeys,
	libfunc_objecttotalmembers,
	libfunc_objectcopy,
	libfunc_totalarguments,
	libfunc_argument,
	libfunc_typeof,
	libfunc_strtonum,
	libfunc_sqrt,
	libfunc_cos,
	libfunc_sin
};
/**************************************************************************/

 
void avm_initstack(void) 
{ 
	unsigned i; 
 
	for(i=0; i<AVM_STACKSIZE; i++) 
	{ 
		AVM_WIPEOUT(stack[i]); 
		stack[i].type = undef_m; 
	}
	
	top = AVM_STACKSIZE-globals-1;
	//printf("to globals einai %d!\n",globals);
	//printf("to top einai %d!\n",top);

} 
 
void avm_assign(avm_memcell* lv,avm_memcell* res) 
{ 
	if(lv == res)
		return;

	if(lv->type == table_m && res->type == table_m && lv->data.tableVal == res->data.tableVal)
		return;

	if(res->type == undef_m)
		fprintf(stderr,"Warning!Trying to assign undef value!\n");


	avm_memcellclear(lv);
	memcpy(lv , res , sizeof(avm_memcell));
	assert(lv->type == res->type);
	//assert(lv->data.strVal == res->data.strVal);
	//assert(lv->data.numVal == res->data.numVal);
	
	/*if(lv->type == number_m)
	{	
		lv->data.numVal = res->data.numVal;
	}*/
	if(lv->type == string_m)
		lv->data.strVal = strdup(res->data.strVal);

	else if(lv->type == table_m)
		avm_tableincrefcounter(lv->data.tableVal);
	
}

void avm_memclear_string(avm_memcell* m) 
{ 
	assert(m->data.strVal); 
	free(m->data.strVal); 
 
} 
 
void avm_memclear_table(avm_memcell* m) 
{ 
	assert(m->data.tableVal); 
	avm_tabledecrefcounter(m->data.tableVal); 
	//printf("***********I decreased\n");
} 
 
void avm_memcellclear(avm_memcell* m) 
{ 
 
	memclear_func_t f; 
 
	if(m->type!=undef_m) 
	{ 
		
		//printf("avm memcellclear!!!Type:%d\n\n",m->type);
		f=memclearFuncs[m->type];//printf("avm memcellclear!!!\n\n");
		if(f) 
			(*f)(m); 
		if(m->type!=table_m)
			m->type=undef_m;
	} 
 
} 
 
 
avm_memcell* avm_translate_operand(vmarg* arg , avm_memcell* reg) 
{ 
	//printf("type: %d\n",arg->type);
 
	switch(arg->type) 
	{ 
 		case global_a : return &stack[AVM_STACKSIZE - 1 - arg->val];
		case local_a  : return &stack[topsp - arg->val]; 
		case formal_a : return &stack[topsp + AVM_STACKENV_SIZE + 1 + arg->val]; 
 
		case retval_a : return &retval; 
 
		case number_a :  
		{ 
			reg->type = number_m; 
			reg->data.numVal = consts_getnumber(arg->val); 
			return reg; 
		} 
		 
		case string_a :  
		{ 
			reg->type = string_m; 
			reg->data.strVal = consts_getstring(arg->val); 
			return reg; 
		} 
 
		case bool_a :  
		{ 
			reg->type = bool_m; 
			reg->data.boolVal = arg->val; 
			return reg; 
		} 
 
		case nil_a :  
		{ 
			reg->type = nil_m; 
			return reg; 
		} 
 
		case userfunc_a : 
		{ 
			reg->type = userfunc_m; 
			reg->data.funcVal = userFuncs[arg->val].address;//arg->val; /*Address already stored*/ 
			return reg; 
 
		} 
 
		case libfunc_a : 
		{ 
			reg->type = libfunc_m; 
			reg->data.libfuncVal = libfuncs_getused(arg->val);  
			return reg; 
 
		} 
 
		default : 
			printf("***type:%d-%d\n",arg->type,pc);assert(0); 
	} 
 
} 
 
void avm_initialize(void) 
{ 
	avm_initstack(); 
 
	avm_registerlibfunc("print", libfunc_print); 
	avm_registerlibfunc("typeof", libfunc_typeof); 
 
	/*Same for all the other library functions*/ 
} 



char *avm_tostring(avm_memcell* m) 
{ 
 
	assert(m->type >= 0 && m->type <= undef_m); 
	return (*tostringFuncs[m->type])(m); 
  
} 


void avm_dec_top(void) 
{ 
 
	if(!top) 
	{		/*Stack Overflow*/ 
		fprintf(stderr,"Stack Overflow!\n"); 
		executionFinished = 1; 
	} 
	else 
		--top; 
		 
} 
 
void avm_push_envvalue(unsigned val) 
{ 
	stack[top].type        = number_m; 
	stack[top].data.numVal = val; 
	avm_dec_top(); 
} 
 
unsigned avm_get_envvalue(unsigned i) 
{ 
	unsigned val; 
	//printf("######type:%d index:%d\n",stack[i].type,i);
	assert(stack[i].type == number_m);//teacher has it =  
	val = (unsigned)stack[i].data.numVal; 
	assert(stack[i].data.numVal == ((double)val)); 
 
	return val; 
} 
 
void avm_callsaveenviroment(void) 
{ 
	avm_push_envvalue(totalActuals);           //printf("save actuals in top:%d\n",top+1);
	avm_push_envvalue(pc + 1);                 //printf("save pc in top:%d\n",top+1);
	avm_push_envvalue(top + totalActuals + 2); //printf("save top in top:%d\n",top+1);
	avm_push_envvalue(topsp);                  //printf("save topsp in top:%d , %d\n",top+1,topsp);
} 
 
void avm_calllibfunc(char *id) 
{ 
	library_func_t f; 
 
	f = avm_getlibraryfunc(id); 
	 
	if(!f) 
	{ 
		fprintf(stderr,"Unsupported Library Function %s called\n", id); 
		executionFinished = 1; 
	} 
 
	else 
	{ 
		/* Notice that enter func and exit func  
		are called manually! */ 
 
		topsp = top;//printf("@@@@%d\n",topsp); 
		totalActuals = 0; 
 
		(*f)();//Call lib func 
 
		if(!executionFinished) 
		{
			execute_funcexit((instruction*)0);//Return Sequence 
			/*unsigned oldTop;
			
			oldTop = top;
			top   = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET+1);//printf("\nload top from top:%d\n",topsp + AVM_SAVEDTOP_OFFSET);
			pc    = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);//printf("load pc from top:%d\n",topsp + AVM_SAVEDPC_OFFSET);
			topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);//printf("load topsp from top:%d\n",topsp + AVM_SAVEDTOPSP_OFFSET);
			while( oldTop++ <= top ) //Inentionally ignoring First
			{
				if(stack[oldTop].type == table_m)
					avm_tableincrefcounter(stack[oldTop].data.tableVal);
				avm_memcellclear(&stack[oldTop]);
			}*/
		}
	} 
 
}

void avm_calltable(avm_table* t)
{
	avm_memcell* functors = (avm_memcell *)malloc(sizeof(avm_memcell));
	avm_memcell* userf;
	userfunc* id;
		
	functors->type = string_m;

	functors->data.strVal = strdup("()");

	assert(t);

	userf = avm_tablegetelem(t,functors);

	if(!userf)
	{
		printf("Object cannot be called because it does not contain the index '()' !!\n");
		executionFinished = 1;
		return;
	}

	else
	{
		if(userf->type!=userfunc_m)
		{
			printf("Object cannot be called because indexed element with '()' is not a function!!\n");
			executionFinished = 1;
			return;
		}
		
		else
		{
			pc = userf->data.funcVal;//printf("1.pc = %d funcval=%d opcode:%d\n\n",pc,func->data.funcVal,code[pc].opcode);
			//assert(pc < AVM_ENDING_PC);
			assert(code[pc].opcode == VM_FUNC_ENTER);	
		}
	}
	
}
 
unsigned avm_totalactuals(void) 
{ 
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET); 
} 
 
avm_memcell* avm_getactual(unsigned i) 
{ 
	assert(i < avm_totalactuals()); 
	//printf("Stack offset:%d\n",topsp + AVM_STACKENV_SIZE + 1 + i);
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
} 
 
 
unsigned char avm_tobool(avm_memcell* mem) 
{ 
	assert(mem->type >= 0 && mem->type <undef_m);//was an = to undef 
	return (*toboolFuncs[mem->type])(mem); 
} 
 
void avm_tabledecrefcounter(avm_table* t) 
{ 
	assert(t->refCounter > 0); 
	if(!--t->refCounter) 
		avm_tabledestroy(t);
} 
 
void avm_tableincrefcounter(avm_table* t) 
{  
	++t->refCounter; 
//	printf("I increased!\n");
} 
 
void avm_tablebucketsinit(avm_table_bucket** p) 
{ 
	unsigned i; 
	for(i=0; i<AVM_TABLE_HASHSIZE; ++i)
		p[i] = (avm_table_bucket*)0;
} 
 
avm_table* avm_tablenew(void) 
{ 
	avm_table* t = (avm_table*)malloc(sizeof(avm_table)); 
	AVM_WIPEOUT(*t); 
 
	t->total = 0; 
	t->refCounter = t->total; 
 
	avm_tablebucketsinit( t->numIndexed ); 
	avm_tablebucketsinit( t->strIndexed );
	avm_tablebucketsinit( t->funcIndexed );

	t->tableIndexedHead = (avm_table_bucket*)0;
	t->boolIndexed[0] = (avm_table_bucket*)0;
	t->boolIndexed[1] = (avm_table_bucket*)0;
}
 
void avm_tablebucketdestroy(avm_table_bucket** p) 
{ 
	unsigned i; 
	avm_table_bucket* b; 
	avm_table_bucket* del; 
	 
	for(i=0; i<AVM_TABLE_HASHSIZE; ++i) 
	{ 
		for(b = *p; b;) 
		{ 
			del = b; 
			b = b->next; 
			avm_memcellclear(&del->key); 
			avm_memcellclear(&del->value); 
			free(del); 
		} 
		p[i] = (avm_table_bucket*)0; 
	} 
}
 
void avm_tabledestroy(avm_table* t) 
{ 
	avm_table_bucket* b;
	avm_table_bucket* del; 

	assert(t);
	
	avm_tablebucketdestroy(t->strIndexed); 
	avm_tablebucketdestroy(t->numIndexed);
	avm_tablebucketdestroy(t->funcIndexed);

	if(t->boolIndexed[0])
	{
		avm_memcellclear(&t->boolIndexed[0]->key);
		avm_memcellclear(&t->boolIndexed[0]->value);
	}
	if(t->boolIndexed[1])
	{
		avm_memcellclear(&t->boolIndexed[1]->key);
		avm_memcellclear(&t->boolIndexed[1]->value);
	}

	for(b = t->tableIndexedHead; b;) 
	{ 
			del = b; 
			b = b->next; 
			avm_memcellclear(&del->key); 
			avm_memcellclear(&del->value); 
			free(del); 
	}

	t->tableIndexedHead = (avm_table_bucket*)0;
	//free(t); 
} 
 
int numHash(double num) 
{ 
	return (unsigned)num % AVM_TABLE_HASHSIZE; 
} 
 
int strHash(char* str) 
{ 
	unsigned i; 
 
	i = str[0]; 
	return i % AVM_TABLE_HASHSIZE; 
} 
 
avm_table_bucket* new_table_bucket(void) 
{ 
	avm_table_bucket* bucket; 
	bucket = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	bucket->key.type = undef_m;
	bucket->value.type = undef_m;
	bucket->next = NULL;
} 
 
avm_memcell* avm_tablegetelem(avm_table* table,avm_memcell* index) 
{ 
	int id,i; 
	avm_table_bucket* tmp; 
	 
	if(index->type == number_m) 
	{ 
		id = numHash( index->data.numVal );
		tmp = table->numIndexed[id]; 
		 
		while(tmp) 
		{ 
			if(tmp->key.data.numVal == index->data.numVal) 
				return &tmp->value; 
			else 
				tmp = tmp->next; 
		} 
 
		//printf("There is no element in the table with the index %.4lf\n",index->data.numVal); 
		return NULL; 
	} 
	else if(index->type == string_m || index->type == libfunc_m) 
	{ 
		if(index->type == string_m)
			id = strHash( index->data.strVal );
		else if(index->type == libfunc_m)
			id = strHash( index->data.libfuncVal );
			
		tmp = table->strIndexed[id]; 
		 
		while(tmp) 
		{ 
			if(!strcmp(tmp->key.data.strVal , index->data.strVal)) 
				return &tmp->value; 
			else 
				tmp = tmp->next; 
		} 
 
		//printf("There is no element in the table with the index %s\n",index->data.strVal); 
		return NULL; 
	}
	else if(index->type == bool_m)
	{
		tmp = table->boolIndexed[index->data.boolVal]; 
		 
		if(tmp)
			return &tmp->value; 
		
		else
		{
			//printf("There is no element in the table with the index %d\n",index->data.boolVal); 
			return NULL;
		}
	}
	else if(index->type == userfunc_m)
	{
		id = numHash( index->data.funcVal );
		tmp = table->funcIndexed[id]; 
		 
		while(tmp) 
		{ 
			if(tmp->key.data.funcVal == index->data.funcVal) 
				return &tmp->value; 
			else 
				tmp = tmp->next; 
		} 
 
		//printf("There is no element in the table with the index %.d\n",index->data.funcVal); 
		return NULL; 
	}

	else if(index->type ==table_m)
	{
		tmp = table->tableIndexedHead;
		
		while(tmp) 
		{ 
			if(tmp->key.data.tableVal == index->data.tableVal) 
				return &tmp->value;
			else
				tmp = tmp->next; 
		} 
 
		//printf("There is no element in the table with the index %.d\n",index->data.funcVal); 
		return NULL;
		
	}
	
	else
	{
		printf("types undef,nil cannot be used as an index!\n");
		assert(0);
	}	
} 
void avm_tablesetelem(avm_table* table , avm_memcell* index , avm_memcell* content) 
{ 
	int id,i; 
	avm_table_bucket* tmp; 
	avm_table_bucket* temp; 
	 
	if(index->type == number_m) 
	{ 
		id = numHash( index->data.numVal );

		tmp = table->numIndexed[id]; 
		if(!tmp) 
		{ 
			table->numIndexed[id] = new_table_bucket(); 
		        avm_assign( &table->numIndexed[id]->key , index); 
			avm_assign( &table->numIndexed[id]->value , content);
			table->total++;
			return; 
		} 
		 
		while(tmp) 
		{ 
			if(tmp->key.data.numVal == index->data.numVal) 
			{ 
				avm_assign( &tmp->value , content); 
				return; 
			} 
			else 
			{	 
				temp = tmp; 
				tmp = tmp->next; 
			} 
		} 
 
		temp->next = new_table_bucket(); 
		avm_assign( &temp->next->key , index); 
		avm_assign( &temp->next->value , content);
		table->total++;
 
	} 
	else if(index->type == string_m || index->type == libfunc_m) 
	{ 
		if(index->type == string_m)
			id = strHash( index->data.strVal );
		else if(index->type == libfunc_m);
			id = strHash( index->data.libfuncVal );
			
		tmp = table->strIndexed[id];
		if(!tmp) 
		{ 
			table->strIndexed[id] = new_table_bucket(); 
		    	avm_assign( &table->strIndexed[id]->key , index); 
			avm_assign( &table->strIndexed[id]->value , content); 
			table->total++;
			return; 
		} 
		 
		while(tmp) 
		{ 
			if(!strcmp(tmp->key.data.strVal, index->data.strVal)) 
			{ 
				avm_assign( &tmp->value , content); 
				return; 
			} 
			else 
			{	 
				temp = tmp; 
				tmp = tmp->next; 
			} 
		} 
 
		temp->next = new_table_bucket(); 
		avm_assign( &temp->next->key , index); 
		avm_assign( &temp->next->value , content);
		table->total++;
 
	}

	else if(index->type == bool_m)
	{
		id = index->data.boolVal;
		tmp = table->boolIndexed[id]; 
		 
		if(tmp)
			avm_assign( &tmp->value , content);
		
		else
		{
			table->boolIndexed[id] = new_table_bucket();
			avm_assign( &table->boolIndexed[id]->key , index); 
			avm_assign( &table->boolIndexed[id]->value , content);
			table->total++;
		}
	}

	else if(index->type == userfunc_m)
	{
		id = numHash( index->data.funcVal );
		tmp = table->funcIndexed[id]; 
		
		if(!tmp) 
		{ 
			table->funcIndexed[id] = new_table_bucket(); 
		        avm_assign( &table->funcIndexed[id]->key , index); 
			avm_assign( &table->funcIndexed[id]->value , content);
			table->total++;
			return; 
		} 
		 
		while(tmp) 
		{ 
			if(tmp->key.data.funcVal == index->data.funcVal) 
			{ 
				avm_assign( &tmp->value , content); 
				return; 
			} 
			else 
			{	 
				temp = tmp; 
				tmp = tmp->next; 
			} 
		} 
 
		temp->next = new_table_bucket(); 
		avm_assign( &temp->next->key , index); 
		avm_assign( &temp->next->value , content);
		table->total++;
	}
	
	else if(index->type == table_m)
	{
		tmp = table->tableIndexedHead;

		if(!tmp)
		{
			table->tableIndexedHead = new_table_bucket();
			avm_assign( &table->tableIndexedHead->key , index); 
			avm_assign( &table->tableIndexedHead->value , content);
			table->total++;
			return;
		}
		while(tmp)
		{
			if(tmp->key.data.tableVal == index->data.tableVal) 
			{ 
				avm_assign( &tmp->value , content); 
				return; 
			} 
			else 
			{	 
				temp = tmp; 
				tmp = tmp->next; 
			} 
		}	
 
		temp->next = new_table_bucket();
		avm_assign( &temp->next->key , index);
		avm_assign( &temp->next->value , content);
		table->total++;
		
	}
	
	else 
	{
		printf("types undef,nil cannot be used as an index!\n");
		assert(0);
	}
 
}

double consts_getnumber(unsigned index){return numConsts[index];} 
char*  consts_getstring(unsigned index){assert(stringConsts[index]); return stringConsts[index];} 
char*  libfuncs_getused(unsigned index){assert(namedLibfuncs[index]); return namedLibfuncs[index];} 
/* With the following every library function is 
manually added in the VM library function resolution map */ 
void avm_registerlibfunc(char* id,void* address){}

userfunc* avm_getfuncinfo(unsigned address)
{
	int i;
	for(i=0;i<totalUserFuncs;i++)
	{
		if(userFuncs[i].address == address)//stack[address].data.funcVal
			return &(userFuncs[i]);
	}
	return NULL;
}  
library_func_t avm_getlibraryfunc(char* id)
{
	return libFuncs[get_lib_name_index(id)];
}  


/**********************************  Loading the binary code  *********************************/

void avm_load_instructions(instruction* i){
	i->result = (vmarg *)malloc(sizeof(vmarg));
	i->arg1 = (vmarg *)malloc(sizeof(vmarg));
	i->arg2 = (vmarg *)malloc(sizeof(vmarg));
	fread(&i->opcode,1,1,instr_bin_fp);
	avm_read_vmarg(i->result,instr_bin_fp);
	avm_read_vmarg(i->arg1,instr_bin_fp);
	avm_read_vmarg(i->arg2,instr_bin_fp);
}

void avm_read_vmarg(vmarg* arg,FILE* fp){

	fread(&arg->type,1,1,fp);//type
	fread(&arg->val,4,1,fp);//value
	if(arg->type == NULL_VMARG_VALUE)
		arg = NULL;

}

void avm_load_const_nums(void){
	int i;
	fread(&totalNumConsts,4,1,instr_bin_fp);

	numConsts = (double *)malloc(totalNumConsts*sizeof(double));

	/*4 testing*/printf("\t******** Const Numbers **********\n");
	for(i = 0;i < totalNumConsts; i++)
	{
		fread(&numConsts[i],8,1,instr_bin_fp);
		/*4 testing*/printf("%d: %f \n",i,numConsts[i]);
	}
}

void avm_load_const_strings(void){
	int i,str_length;
	fread(&totalStringConsts,4,1,instr_bin_fp);

	stringConsts = (char **)malloc(totalStringConsts*sizeof(char *));

	/*4 testing*/printf("\t******** Const Strings **********\n");
	for(i = 0;i < totalStringConsts; i++)
	{
		fread(&str_length,4,1,instr_bin_fp);
		stringConsts[i] = (char *)malloc(str_length+1);
		fread(stringConsts[i],str_length,1,instr_bin_fp);//-1
		/*4 testing*/printf("%d: \"%s\" \n",i,stringConsts[i]);
	}
}

void avm_load_user_funcs(void){
	int i,str_length;
	fread(&totalUserFuncs,4,1,instr_bin_fp);

	userFuncs = (userfunc *)malloc(totalUserFuncs*sizeof(userfunc));

	/*4 testing*/printf("\t******** User Functions **********\n");
	for(i = 0;i < totalUserFuncs; i++)
	{
		fread(&str_length,4,1,instr_bin_fp);
		userFuncs[i].id = (char *)malloc(str_length+1);
		fread(userFuncs[i].id,str_length,1,instr_bin_fp);//-1
		fread(&userFuncs[i].address,4,1,instr_bin_fp);
		fread(&userFuncs[i].localSize,4,1,instr_bin_fp);
		/*4 testing*/printf("%d:  id:%s address:%d locals:%d\n",i,userFuncs[i].id,userFuncs[i].address,userFuncs[i].localSize);
	}
}


void avm_load_lib_funcs(void){
	int i,str_length;
	fread(&totalNamedLibfuncs,4,1,instr_bin_fp);

	namedLibfuncs = (char **)malloc(totalNamedLibfuncs*sizeof(char *));

	/*4 testing*/printf("\t******** Lib Functions **********\n");
	for(i = 0;i < totalNamedLibfuncs; i++)
	{
		fread(&str_length,4,1,instr_bin_fp);
		namedLibfuncs[i] = (char *)malloc(str_length);
		fread(namedLibfuncs[i],str_length,1,instr_bin_fp);//-1
		/*4 testing*/printf("%d: %s \n",i,namedLibfuncs[i]);
	}
}


void avm_read_binary(const char* binary_file_name){
	
	int i;
	assert(binary_file_name);
	instr_bin_fp = fopen(binary_file_name,"r");
	if(instr_bin_fp == NULL)
	{
		printf("Can't open file %s",binary_file_name);
		exit(1);
	}
	
	fread(&magic,4,1,instr_bin_fp);
	fread(&globals,4,1,instr_bin_fp);
	fread(&codeSize,4,1,instr_bin_fp);

	if(magic != VM_MAGIC)
	{
		fprintf(stderr,"Error: The input file %s is propably not an .abc file\n",binary_file_name);
		exit(2);
	}
	else if (globals < 0 || codeSize < 0) {
		fprintf(stderr, "Error: corrupted file %s\n", binary_file_name);
		exit(3);
	}
	else if (globals >= AVM_STACKSIZE) {
		fprintf(stderr, "Error: ENOMEM for file %s\n", binary_file_name);
		exit(4);
	}
	else
	{
		if (code)
			free(code);
		code = (instruction *) malloc(codeSize*sizeof(instruction));

		if (code == NULL) {
			fprintf(stderr, "Internal Error: cannot allocate mem for file %s\n", binary_file_name);
			exit(1000);
		}
		else
			for (i = 0; i < codeSize; i++)
				avm_load_instructions(&code[i]);
		
		avm_load_const_nums();
		avm_load_const_strings();
		avm_load_user_funcs();
		avm_load_lib_funcs();
	}
}

int get_lib_name_index(char *name)
{
	int i;
	for(i = 0; i<12; i++)
	{
		if(!strcmp(name,lib_funcs_names[i]))
			return i;
	}
}
extern char *typeStrings[]; 
void print_stack(){
	int i;
	printf("\n\t\tStack\n");
	for(i = AVM_STACKSIZE-1;i>top;i--){
		printf("%d.type:%s ",i,typeStrings[stack[i].type]);
		if(stack[i].type == number_m)
			printf("value:%f\n",stack[i].data.numVal);
		else if(stack[i].type == string_m)
			printf("value:%s\n",stack[i].data.strVal);
		else if(stack[i].type == libfunc_m)
			printf("value:%s\n",stack[i].data.libfuncVal);
		else
			printf("  un\n");
	}
}


int main (int argc, char** argv)
{
	const char* file_name = "user_input.abc";
	int i;
	instruction *current_instruction;
	/*if (argc == 1)
	{
		fprintf(stderr,"File Missing!\n");
		return 1;
	}*/
	if (argc == 2)
		file_name = argv[1];

	avm_read_binary(file_name);
	printf("\n\n\n\n\t**** Program Output ****\n\n");
	avm_initialize();

	while(!executionFinished)
	{
		execute_cycle(current_instruction);
		//print_stack();
		//scanf("%d",&i);
	}
	return 0;
}
