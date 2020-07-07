#include "dispatcher.h"
#include<math.h>

int start=0;
avm_table* general = NULL;
table_list* table_list_head = NULL;

extern unsigned char executionFinished;


/* Implementation of the library func typeof
it returns the type of the given argument*/

void libfunc_typeof(void)
{

	unsigned n;
	char *s;

	n = avm_totalactuals();

	if(n!=1)
	{
		printf("one argument [not %d] expected in 'typeof'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		avm_memcellclear(&retval);
		retval.type = string_m;
		retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]); 
	}
}

/* Implementation of the library func print
it prints every argument in the screen */

void libfunc_print(void)
{

	unsigned n, i, j;
	int flag = 0;
	char *s;

	n = avm_totalactuals();
	for(i=0;i<n;++i)
	{
		s=strdup(avm_tostring(avm_getactual(i)));
		//printf("Actual arg:%s\n",s);
		for(j = 0;s[j] != '\0';j++)
		{
			if(s[j] == '\\')
				flag = 1;
			else
			{
				if(flag == 1)
				{
					if(s[j] == 'n')
						printf("\n");
					else if(s[j] == 't')
						printf("\t");
					else
						printf("%c%c",s[j-1],s[j]);
					flag = 0;
				}
				else
					printf("%c",s[j]);
			}
		}
		//printf("\n");
		//puts(s);
		free(s);
		start = 0;
	}
}


void libfunc_totalarguments(void)
{
	/*Get topsp of previous activation record*/
	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);

	if(!p_topsp)
	{
		printf("'totalarguments' called outside a function!\n");
		//executionFinished = 1;
		retval.type = nil_m;
	}
	else
	{
		/*Extract the number of actual arguments
		for the previous activation record*/

		retval.type = number_m;
		retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}

}


void libfunc_objectmemberkeys(void)
{
	avm_table* newtable = avm_tablenew();
	int i=0;
	unsigned n;
	avm_memcell* tmp;
	avm_table_bucket* chief;

	avm_memcell* cntr =(avm_memcell*)malloc(sizeof(avm_memcell));

	cntr->type = number_m;
	cntr->data.numVal = 0;

	n = avm_totalactuals();
	if(n!=1)
	{
		printf("one argument [not %d] expected in 'objectmemberkeys'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		tmp = avm_getactual(n-1);
	
		assert(tmp->type == table_m);

		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->numIndexed[i];

			while(chief)
			{
				avm_tablesetelem(newtable,cntr,&chief->key);
				cntr->data.numVal++;
				chief =chief->next;
			}
		}
		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->strIndexed[i];

			while(chief)
			{	
				avm_tablesetelem(newtable,cntr,&chief->key);
				cntr->data.numVal++;
				chief =chief->next;
			}
		}
		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->funcIndexed[i];

			while(chief)
			{
				avm_tablesetelem(newtable,cntr,&chief->key);
				cntr->data.numVal++;
				chief =chief->next;
			}
		}
		
		chief = tmp->data.tableVal->tableIndexedHead;

		while(chief)
		{
			avm_tablesetelem(newtable,cntr,&chief->key);
			cntr->data.numVal++;
			chief =chief->next;
		}
		for(i=0;i<2;i++)
		{
			chief = tmp->data.tableVal->boolIndexed[i];

			if(chief)
			{
				avm_tablesetelem(newtable,cntr,&chief->key);
				cntr->data.numVal++;
			}
		}
		
	}

	retval.type = table_m;
	retval.data.tableVal = newtable;


}

void libfunc_objectcopy(void)
{

	avm_table* newtable = avm_tablenew();
	int i=0;
	unsigned n;
	avm_memcell* tmp;
	avm_table_bucket* chief;

	n = avm_totalactuals();
	if(n!=1)
	{
		printf("one argument [not %d] expected in 'objectcopy'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		tmp = avm_getactual(n-1);
	
		assert(tmp->type == table_m);

		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->numIndexed[i];

			while(chief)
			{
				avm_tablesetelem(newtable,&chief->key,&chief->value);
				chief =chief->next;
			}
		}
		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->strIndexed[i];

			while(chief)
			{	
				avm_tablesetelem(newtable,&chief->key,&chief->value);
				chief =chief->next;
			}
		}
		for(i=0;i<AVM_TABLE_HASHSIZE;i++)
		{
			chief = tmp->data.tableVal->funcIndexed[i];

			while(chief)
			{
				avm_tablesetelem(newtable,&chief->key,&chief->value);
				chief =chief->next;
			}
		}
		
		chief = tmp->data.tableVal->tableIndexedHead;

		while(chief)
		{
			avm_tablesetelem(newtable,&chief->key,&chief->value);
			chief =chief->next;
		}
		for(i=0;i<2;i++)
		{
			chief = tmp->data.tableVal->boolIndexed[i];

			if(chief)
				avm_tablesetelem(newtable,&chief->key,&chief->value);
		}
	}

	retval.type = table_m;
	retval.data.tableVal = newtable;

}


void libfunc_argument(void){
	
	unsigned address;
	unsigned n = avm_totalactuals();
	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcell* tmp;
	
	avm_memcellclear(&retval);
	
	if(!p_topsp)
	{
		printf("'argument' called outside a function!\n");
		//executionFinished = 1;
		retval.type = nil_m;
		return;
	}
	if(n!=1)
	{
		printf("one argument [not %d] expected in 'argument'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		/*Extract the number of actual arguments
		for the previous activation record*/

		tmp = avm_getactual(0);
		assert(tmp->type ==number_m);
		n = tmp->data.numVal;
		avm_assign(&retval,&stack[p_topsp + AVM_STACKENV_SIZE + 1 + n]);
	}

}


/* Implementation of the library func input
it returns the value given as input by user*/

void libfunc_input(void)
{

	unsigned n;
	char* in;
	int i,digit=1;
	int uminus = 1;
	int dotcntr = 0;

	avm_memcell* tmp;

	n = avm_totalactuals();

	if(n!=0)
	{
		printf("'input' takes no arguments!\n",n);
	}
	else
	{
		in = (char *)malloc(140);
		scanf("%s",in);

		if(!strcmp(in,"true"))
		{
			retval.type = bool_m;
			retval.data.boolVal = 1;
		}
		else if(!strcmp(in,"false"))
		{
			retval.type = bool_m;
			retval.data.boolVal = 0;
		}
		else if(!strcmp(in,"nil"))
		{
			retval.type = nil_m;
		}
		else
		{

			for(i=0;i<strlen(in);i++)
			{
			
				if(uminus)
				{
					if(in[0]=='-')
					{
						uminus = 0;
						if(strlen(in)==1)
						{
							digit = 0;
							break;
						}
						continue;
					}
				}
				if(in[i]=='.' && i!=strlen(in)-1)
				{
					++dotcntr;
					continue;
				}
			
				if(!isdigit(in[i]))
				{
					digit = 0;
					break;
				}
			}

			if(dotcntr > 1)
				digit = 0; 
			
			if(digit)
			{
				retval.type = number_m;
				retval.data.numVal = atof(in);
			}
			else
			{
				retval.type = string_m;
				retval.data.strVal = strdup(in);
			}
	
		}
	}

}


/* Implementation of the library func objectmemberkeys
it returns the total number of members of a table */

void libfunc_objecttotalmembers(void)
{
	unsigned n;
	avm_memcell* tmp;

	n = avm_totalactuals();
	if(n!=1)
	{
		printf("one argument [not %d] expected in 'objecttotalmembers'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		tmp = avm_getactual(n-1);
		if(tmp)
		{
			assert(tmp->type == table_m);
			avm_memcellclear(&retval);
			retval.type = number_m;
			retval.data.numVal = tmp->data.tableVal->total;
		}
	}


}



/* Implementation of the library func strtonum
it returns the numeric value of a string*/

void libfunc_strtonum(void)
{
	unsigned n;
	double res = 0;
	avm_memcell* tmp;

	n = avm_totalactuals();

	if(n!=1)
	{
		printf("one argument [not %d] expected in 'strtonum'!\n",n);
		retval.type = nil_m;
	}
	else
	{
		tmp = avm_getactual(n-1);
		if(tmp)
		{
			assert(tmp->type == string_m);
			avm_memcellclear(&retval);
			
			res = atof(tmp->data.strVal);
			if(!res)
				retval.type = nil_m;
			else
			{
				retval.type = number_m;
				retval.data.numVal = res;
			}
		}
	}

}

/* Implementation of the library func sqrt
it returns the sqrt(x)*/

void libfunc_sqrt(void)
{
	unsigned n;
	avm_memcell* tmp;

	n = avm_totalactuals();

	if(n!=1)
	{
		printf("one argument [not %d] expected in 'sqrt'!\n",n);
	}
	else
	{
		tmp = avm_getactual(n-1);
		if(tmp)
		{
			assert(tmp->type == number_m);
			avm_memcellclear(&retval);

			if(tmp->data.numVal<0)
				retval.type = nil_m;
			else
			{
				retval.type = number_m;
				retval.data.numVal = sqrt(tmp->data.numVal); 
			}
		}
	}

}

/* Implementation of the library func cos
it returns the cos(x)*/

void libfunc_cos(void)
{
	unsigned n;
	const double pi = 3.14159265;
	double radians;
	avm_memcell* tmp;

	n = avm_totalactuals();

	if(n!=1)
	{
		printf("one argument [not %d] expected in 'cos'!\n",n);
	}
	else
	{
		tmp = avm_getactual(n-1);
		if(tmp)
		{
			assert(tmp->type == number_m);
			avm_memcellclear(&retval);
			retval.type = number_m;
			radians = (tmp->data.numVal*pi)/180;

			retval.data.numVal = cos(radians); 
		}
	}
}

/* Implementation of the library func sin
it returns the sin(x)*/

void libfunc_sin(void)
{

	unsigned n;
	const double pi = 3.14159265;
	double radians;
	avm_memcell* tmp;

	n = avm_totalactuals();

	if(n!=1)
	{
		printf("one argument [not %d] expected in 'sin'!\n",n);
	}
	else
	{
		tmp = avm_getactual(n-1);
		if(tmp)
		{
			assert(tmp->type == number_m);
			avm_memcellclear(&retval);
			retval.type = number_m;
			radians = (tmp->data.numVal*pi)/180;

			retval.data.numVal = sin(radians); 
		}
	}
}




/*arithmetic*/
double add_impl(double x,double y){ return x+y;}
double sub_impl(double x,double y){ return x-y;}
double mul_impl(double x,double y){ return x*y;}
double div_impl(double x,double y){
	if(x == 0)
	{
		executionFinished = 1;
		fprintf(stderr,"Error : Divide by zero.");
	}
	return x/y;
}//error check}
double mod_impl(double x,double y){ return (unsigned)x%(unsigned)y;}

/*logical*/
unsigned char jle_impl(double x,double y){ return x<=y;}
unsigned char jge_impl(double x,double y){ return x>=y;}
unsigned char jlt_impl(double x,double y){ return x<y;}
unsigned char jgt_impl(double x,double y){ return x>y;}



/*Boolean*/
unsigned char number_tobool(avm_memcell *mem){ return mem->data.numVal!=0; }
unsigned char string_tobool(avm_memcell *mem){ return mem->data.strVal[0]!=0; }
unsigned char bool_tobool(avm_memcell *mem){ return mem->data.boolVal; }//was !=0
unsigned char table_tobool(avm_memcell *mem){ return 1; }
unsigned char userfunc_tobool(avm_memcell *mem){ return 1; }
unsigned char libfunc_tobool(avm_memcell *mem){ return 1; }
unsigned char nil_tobool(avm_memcell *mem){ return 0; }
unsigned char undef_tobool(avm_memcell *mem){ assert(0); return 0; }



void execute_cycle(instruction* i)
{
	instruction* instr;
	unsigned oldPc;

	if(executionFinished)
		return;

	else if(pc == AVM_ENDING_PC)
	{
		executionFinished = 1;
		return;
	}

	else
	{
		assert(pc < AVM_ENDING_PC);
		instr = code + pc;

		assert(instr->opcode >=0 && instr->opcode<=MAX_INSTRUCTIONS);

		if(instr->srcLine)
		{
			currLine=instr->srcLine;
		}

		oldPc = pc;
		//printf("before");
		//printf("@execute opcode:%d\n",instr->opcode);
		(*executeFuncs[instr->opcode])(instr);
		//printf("executing %d\n",instr->opcode);
		if(pc == oldPc)
			++pc;
	}
	

}
void execute_assign(instruction* i)
{
	avm_memcell* lv;
	avm_memcell* rv;

	lv = avm_translate_operand(i->result , (avm_memcell*)0);
	rv = avm_translate_operand(i->arg1 , &ax);
	
	assert(lv);
	//assert( &stack[0] <= lv );
	//assert( &stack[top] > lv || lv == &retval);
	//assert(lv && (&stack[0] <= lv && &stack[top] > lv || lv == &retval));

	assert(rv);

	avm_assign(lv,rv);
}

void execute_call(instruction* i)
{

	avm_memcell* func;
	char* s;
	func=avm_translate_operand(i->result,&ax);
	assert(func);

	if(func->type!=table_m)
		avm_callsaveenviroment();

	switch(func->type)
	{
		case userfunc_m : 
		{
			pc = func->data.funcVal;//printf("1.pc = %d funcval=%d opcode:%d\n\n",pc,func->data.funcVal,code[pc].opcode);
			assert(pc < AVM_ENDING_PC);
			assert(code[pc].opcode == VM_FUNC_ENTER);

			break;
		}

		case libfunc_m : avm_calllibfunc(func->data.libfuncVal); break;
		case string_m  : avm_calllibfunc(func->data.strVal); break;
		case table_m   :
		{
			execute_pusharg(i);
			avm_callsaveenviroment();
			//printf("Ekana push ton table:%s-%d\n",avm_tostring(func),totalActuals);
			avm_calltable(func->data.tableVal);
			break;
		}


		default :
		{
			s=strdup(avm_tostring(func));
			printf("Cannot bind %s to function!\n",s);
			free(s);
			executionFinished=1;

		}

	}
}

void execute_funcenter(instruction *i)
{

	avm_memcell* func;
	userfunc*	 funcInfo;

	func = avm_translate_operand(i->result , &ax);
	assert(func);//printf("2.pc = %d funcval=%d\n\n",pc,func->data.funcVal);
	assert(pc == func->data.funcVal);  //Func Address should match pc
	
	/*Callee actions here*/
	totalActuals = 0;
	funcInfo = avm_getfuncinfo(pc);
	topsp = top;//printf("func enter topsp:%d\n\n",topsp);
	top = top - funcInfo->localSize;

}

void execute_funcexit(instruction *i)
{

	unsigned oldTop;

	oldTop = top;
	//printf("func exit topsp:%d\n\n",topsp);
	top   = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);//printf("\nload top from top:%d\n",topsp + AVM_SAVEDTOP_OFFSET);
	pc    = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);//printf("load pc from top:%d\n",topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);//printf("load topsp from top:%d\n",topsp + AVM_SAVEDTOPSP_OFFSET);
	while( oldTop++ <= top ) //Inentionally ignoring First
	{
		if(stack[oldTop].type == table_m)
			avm_tableincrefcounter(stack[oldTop].data.tableVal);
		avm_memcellclear(&stack[oldTop]);
	}
}

void execute_pusharg(instruction* i)
{

	avm_memcell* arg;

	arg = avm_translate_operand(i->result,&ax);
	assert(arg);
	//printf("total = %d-%s\n",arg->data.tableVal->total,avm_tostring(arg));

	/*This is actually stack[top] = arg but we have to use assign*/
	avm_assign(&stack[top],arg);
	++totalActuals;
	avm_dec_top();

}

void execute_arithmetic(instruction* i)
{

	avm_memcell* lv;
	avm_memcell* rv1;
	avm_memcell* rv2;

	arithmetic_func_t op;

	lv  = avm_translate_operand(i->result , (avm_memcell*)0);
	rv1 = avm_translate_operand(i->arg1 , &ax);
	rv2 = avm_translate_operand(i->arg2 , &bx);

	assert(lv);
	assert( &stack[0] <= lv );
	//assert( &stack[top] >= lv || lv == &retval);
	//assert( lv == &retval);
	assert(rv1 && rv2);

	if(rv1->type!=number_m || rv2->type!=number_m)
	{
		printf("Not a number in Arithmetic!\n");
		executionFinished = 1;
	}

	else
	{
		op = arithmeticFuncs[i->opcode - VM_ADD];
		avm_memcellclear(lv);
		lv->type = number_m;
		lv->data.numVal = (*op)(rv1->data.numVal , rv2->data.numVal);
	}

}

void execute_logical(instruction* i)
{

	//avm_memcell* lv;
	avm_memcell* rv1;
	avm_memcell* rv2;

	logical_func_t op;
	/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
	//lv->type = undef_m;
	assert(i->result->type == label_a);
	//lv  = avm_translate_operand(i->result , (avm_memcell*)0);
	rv1 = avm_translate_operand(i->arg1 , &ax);
	rv2 = avm_translate_operand(i->arg2 , &bx);

	//assert(lv && &stack[0] <= lv && &stack[top] > lv || lv == &retval);
	assert(rv1 && rv2);

	if(rv1->type!=number_m || rv2->type!=number_m)
	{
		fprintf(stderr,"Not a number in Comparison!\n");
		executionFinished = 1;
	}

	else
	{
		op = logicalFuncs[i->opcode - VM_JLE];//printf("lv->type:%d\n\n",lv->type);
		//avm_memcellclear(lv);printf("here!!!!!!!\n\n");
		//lv->type = bool_m;
		//lv->data.boolVal = ;
		if((*op)(rv1->data.numVal , rv2->data.numVal))
			execute_jump(i);
	}

}

void execute_jeq(instruction* i)
{
	avm_memcell* rv1;
	avm_memcell* rv2;

	unsigned char result = 0;

	assert(i->result->type == label_a);//was i->opcode

	rv1 = avm_translate_operand(i->arg1 , &ax);
	rv2 = avm_translate_operand(i->arg2 , &bx);

	if(rv1->type == undef_m || rv2->type == undef_m)
	{
		fprintf(stderr,"'undef' involved in equality!\n");
		executionFinished = 1;
	}
	else if(rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;

	else if(rv1->type == bool_m || rv2->type == bool_m)
		result = ( avm_tobool(rv1) == avm_tobool(rv2) );

	else if(rv1->type != rv2->type)
	{
		fprintf(stderr," %s == %s is illegal!\n",typeStrings[rv1->type] , typeStrings[rv2->type]);
		executionFinished = 1;
	}
	else
	{
		/*Equality checked with dispatching*/
		check_equal_func_t equal_f= checkEqual[rv1->type];
		result = (*equal_f)(rv1,rv2);
	}
	if(!executionFinished && result)
		pc = i->result->val;

}

void execute_jne(instruction* i)
{
	avm_memcell* rv1;
	avm_memcell* rv2;

	unsigned char result = 0;

	assert(i->result->type == label_a);//

	rv1 = avm_translate_operand(i->arg1 , &ax);
	rv2 = avm_translate_operand(i->arg2 , &bx);

	if(rv1->type == undef_m || rv2->type == undef_m)
	{
		printf("'undef' involved in comparison!\n");
		executionFinished = 1;
	}
	else if(rv1->type == nil_m || rv2->type == nil_m)
		result = rv1->type == nil_m && rv2->type == nil_m;

	else if(rv1->type == bool_m || rv2->type == bool_m)
		result = ( avm_tobool(rv1) == avm_tobool(rv2) );

	else if(rv1->type != rv2->type)
	{
		fprintf(stderr," %s != %s is illegal!\n",typeStrings[rv1->type] , typeStrings[rv2->type]);
		executionFinished = 1;
	}
	else
	{
		/*Equality checked with dispatching*/
		check_equal_func_t equal_f= checkEqual[rv1->type];
		result = (*equal_f)(rv1,rv2);
	}

	if(!executionFinished && !result)/**********************************put ! in result********************/
		pc = i->result->val;

}

void execute_jump(instruction* i)
{
	/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
	//avm_memcell* lv; 
        //lv = avm_translate_operand(i->result , (avm_memcell*)0);
	assert(i->result->type == label_a);
	pc = i->result->val;
}


void execute_newtable(instruction* i)
{
	avm_memcell* lv; 
	lv = avm_translate_operand(i->result , (avm_memcell*)0);

	assert(lv );//&& (&stack[0] <= lv && &stack[top] > lv || lv == &retval));

	avm_memcellclear(lv);

	lv->type = table_m;
	lv->data.tableVal = avm_tablenew();
	avm_tableincrefcounter(lv->data.tableVal);
}

void execute_tablegetelem(instruction* i)
{
	avm_memcell* lv;
	avm_memcell* t;
	avm_memcell* index;
	avm_memcell* content;

	char *s1,*s2;
/*****************Maybe it is different*****************/
	lv = avm_translate_operand(i->result , (avm_memcell*)0);
	t  = avm_translate_operand(i->arg1 , (avm_memcell*)0);
	index  = avm_translate_operand(i->arg2 , &ax);

	assert(lv );//&& (&stack[0] <= lv && &stack[top] > lv || lv == &retval));
	assert(t );//&& &stack[0] <= t && &stack[top] > t);
	assert(index);

	avm_memcellclear(lv);
	lv->type = nil_m;/*Default value*/

	if(t->type != table_m)
	{
		fprintf(stderr,"%d-Illegal use of type %s as table!\n",pc,typeStrings[t->type]);
		executionFinished = 1;
	}
	else
	{
		content = avm_tablegetelem(t->data.tableVal , index);

		if(content)
			avm_assign(lv , content);
		else
		{
			s1 = strdup(avm_tostring(t));
			s2 = strdup(avm_tostring(index));
			/*Warning*/
			fprintf(stderr," %s[%s] not found!\n", s1 , s2);

			free(s1);
			free(s2);
		}


	}

}

void execute_tablesetelem(instruction* i)
{
	avm_memcell* t;
	avm_memcell* index;
	avm_memcell* c;
/*****************Maybe it is different*****************/
	t  = avm_translate_operand(i->arg1 , &cx);//(avm_memcell*)0);
	index  = avm_translate_operand(i->arg2 , &ax);
	c  = avm_translate_operand(i->result , &bx);

	assert(t);// && &stack[0] <= t && &stack[top] > t);
	assert(index && c);

	assert(c->type>=number_m && c->type<=undef_m);
	assert(index->type>=number_m && index->type<=undef_m);
	
	if(t->type != table_m)
	{
		fprintf(stderr,"%d-set-Illegal use of type %s as table!\n",pc,typeStrings[t->type]);
		executionFinished = 1;
	}

	else
		avm_tablesetelem(t->data.tableVal , index , c);
}







void execute_nop(instruction* i){}/********************************  TODO  ************************************/
char* number_tostring(avm_memcell* mem)
{
	char* tmp = (char *)malloc(100);
	assert(mem->type == number_m);
	if(mem->data.numVal == (int)mem->data.numVal)
		sprintf(tmp , "%0.f" , mem->data.numVal);
	else
		sprintf(tmp , "%.4f" , mem->data.numVal);
	return tmp;
}

char* string_tostring(avm_memcell* mem)
{
	return mem->data.strVal;
}

char* bool_tostring(avm_memcell* mem)
{
	switch(mem->data.boolVal)
	{
		case 0 : return "false";
		case 1 : return "true";

		default : assert(0);
	}	
}
char* table_tostring(avm_memcell* mem)
{
	int i;
	int flag =0;
	char* lbracket = "[ ";
	char* rbracket = " ]";
	char* lhook = "{ ";
	char* rhook = " }";
	char* comma = ",";
	char* uddot = ":";
	char* self = "<index>";
	char* tmp;
	char* final = (char*)malloc(512);
	avm_table_bucket* chief;


	

	assert(mem->type == table_m);
	
	if(!start)
	{
		general = mem->data.tableVal;
		start =1;
	}

	insert_table_list(mem->data.tableVal);
	
	//printf("to g->size einai:%d\n\n",general->total);
	sprintf(final,"%s",lbracket);

	/*numIndexed*/
	for(i=0;i<AVM_TABLE_HASHSIZE;i++)
	{
		chief = mem->data.tableVal->numIndexed[i];

		while(chief)
		{
			if(chief->value.type == table_m)
			{
				if(chief->value.data.tableVal==general)
				{	
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
				}
				else if(chief->value.data.tableVal == mem->data.tableVal)
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
					
				}
				else if(check_preced(chief->value.data.tableVal))
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;	
				}
				else
				{
				     if(!flag)
				        sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				     else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);						
					flag = 1;
					
				}
			}
			else if(chief->value.type!=table_m)
			{
			   if(!flag)
				sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
			   else
			 	sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				
				
			   flag = 1;
			}

			chief =chief->next;
		}
	}
	/*strIndexed--libfuncIndexed*/
	for(i=0;i<AVM_TABLE_HASHSIZE;i++)
	{
		chief = mem->data.tableVal->strIndexed[i];

		while(chief)
		{	
			if(chief->value.type == table_m)	
			{
				if(chief->value.data.tableVal==general)
				{	
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);						
					flag = 1;
				}
				else if(chief->value.data.tableVal == mem->data.tableVal)
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
					
				}
				else if( check_preced(chief->value.data.tableVal))
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;	
				}
				else
				{
				     if(!flag)
				        sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				     else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);						
					flag = 1;
					
				}
			}
	
			else if(chief->value.type != table_m )
			{

			   if(!flag)
				sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);		
			   else
				sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
			
			   flag = 1;
			}

			chief =chief->next;
		}
	}
	/*tableIndexed*/
	chief = mem->data.tableVal->tableIndexedHead;

	while(chief)
	{
		if(chief->value.type == table_m)
		{
			if((chief->value.data.tableVal==general && chief->key.data.tableVal!=general )||(chief->value.data.tableVal == mem->data.tableVal && 
						chief->key.data.tableVal!=mem->data.tableVal)||( check_preced(chief->value.data.tableVal) && 
						!check_preced(chief->key.data.tableVal)))
			{	
				if(!flag)
					sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
				else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					
				flag = 1;
			}
			else if((chief->key.data.tableVal==general && chief->value.data.tableVal!=general)||(chief->key.data.tableVal==mem->data.tableVal && 
					chief->value.data.tableVal!=mem->data.tableVal)||( check_preced(chief->key.data.tableVal) && 
					!check_preced(chief->value.data.tableVal)))
			{	
				if(!flag)
					sprintf(final ,"%s%s%s%s%s%s",final,lhook,self,uddot,avm_tostring(&chief->value),rhook);
				else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,self,uddot,avm_tostring(&chief->value),rhook);
			
				flag = 1;
			}
			
			else
			{	
				if(!flag)
					sprintf(final ,"%s%s%s%s%s%s",final,lhook,self,uddot,self,rhook);
				else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,self,uddot,self,rhook);
			
				flag = 1;
			}
				
		}
		
		
		else if(chief->value.type!=table_m)// && chief->key.data.tableVal!=mem->data.tableVal)
		{
			if(chief->key.data.tableVal==general)
			{
			    if(!flag)
		              sprintf(final ,"%s%s%s%s%s%s",final,lhook,self,uddot,avm_tostring(&chief->value),rhook);
		            else
			      sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,self,uddot,avm_tostring(&chief->value),rhook);
				
			    flag = 1;
				
			}
			else if(chief->key.data.tableVal==mem->data.tableVal)
			{
			    if(!flag)
		              sprintf(final ,"%s%s%s%s%s%s",final,lhook,self,uddot,avm_tostring(&chief->value),rhook);
		            else
			      sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,self,uddot,avm_tostring(&chief->value),rhook);
				
			    flag = 1;
				
			}
			else
			{
	               	    if(!flag)
		              sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
		            else
			      sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				
		       
			    flag = 1;
			}
		}
		chief = chief->next;
	
	}

	/*userfuncIndexed*/
	for(i=0;i<AVM_TABLE_HASHSIZE;i++)
	{
		chief = mem->data.tableVal->funcIndexed[i];

		while(chief)
		{
			if(chief->value.type == table_m)
			{
				if(chief->value.data.tableVal==general)
				{	
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
				}
				else if(chief->value.data.tableVal == mem->data.tableVal)
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
					
				}
				else if( check_preced(chief->value.data.tableVal))
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;	
				}
				else
				{
				     if(!flag)
				        sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				     else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);						
					flag = 1;
					
				}

			}
			else if(chief->value.type!=table_m)
			{
			   if(!flag)
				sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
			   else
				sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				
			   flag = 1;
			}

			chief =chief->next;
		}
	}
	/*boolIndexed*/
	for(i=0;i<2;i++)
	{
		chief = mem->data.tableVal->boolIndexed[i];

		if(chief)
		{
			if(chief->value.type == table_m)
			{
				if(chief->value.data.tableVal==general)
				{	
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
				}
				else if(chief->value.data.tableVal == mem->data.tableVal)
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;
					
				}
				else if( check_preced(chief->value.data.tableVal))
				{
					if(!flag)
						sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,self,rhook);
					else
						sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,self,rhook);

					flag = 1;	
				}
				else
				{
				     if(!flag)
				        sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				     else
					sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);						
					flag = 1;
					
				}
			}
			else if(chief->value.type!=table_m)
			{
			   if(!flag)
				sprintf(final ,"%s%s%s%s%s%s",final,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
			   else
				sprintf(final ,"%s%s%s%s%s%s%s",final,comma,lhook,avm_tostring(&chief->key),uddot,avm_tostring(&chief->value),rhook);
				
			   flag = 1;
			}

		}

	}

	sprintf(final,"%s%s",final,rbracket);
	reset_table_list();

	return final;


}
char* userfunc_tostring(avm_memcell* mem)
{
	char* tmp;
	userfunc *f;
	assert(mem->data.funcVal);
	tmp = (char*)malloc(20);

	f=avm_getfuncinfo(mem->data.funcVal);

	sprintf(tmp,"user function %d",f->address);
	
	return tmp;
}
char* libfunc_tostring(avm_memcell* mem)
{
	char* tmp;
	assert(mem->data.libfuncVal);
	tmp = (char*)malloc(20 + strlen(mem->data.libfuncVal));

	sprintf(tmp,"library function %s",mem->data.libfuncVal);
	
	return tmp;
}

char* nil_tostring(avm_memcell* mem){return "nil";}
char* undef_tostring(avm_memcell* mem){return "undef";}

void insert_table_list(avm_table* in)
{
	table_list* tmp = table_list_head;

	if(tmp == NULL)
	{
		table_list_head = (table_list*)malloc(sizeof(table_list));
		table_list_head->t = in;
		table_list_head->next = NULL;
	}

	else
	{
		while(tmp->next)
			tmp=tmp->next;

		tmp->next = (table_list*)malloc(sizeof(table_list));
		tmp->next->t = in;
		tmp->next->next = NULL;
	}
}
		
	
int check_preced(avm_table* in)
{
	table_list* tmp = table_list_head;
	table_list* temp = NULL;

	if(tmp)
	{
		if(!tmp->next)
			return 0;
	}
	if(!tmp)
		return 0;

	while(tmp->next)
	{
		temp = tmp;
		tmp = tmp->next;
		if(temp->t == in)
			return 1;
	}

	return 0;
}

void reset_table_list(void)
{
	table_list* tmp;

	if(table_list_head == NULL)
		return;

	else
	{
		table_list_head = NULL;
	}
	
}


/************************************* Equality functions *******************************/


unsigned char number_check_equal(avm_memcell *mem1,avm_memcell *mem2){
	assert(mem1->type == number_m && mem2->type == number_m);
	if(mem1->data.numVal == mem2->data.numVal)
		return 1;
	else
		return 0;
}

unsigned char string_check_equal(avm_memcell *mem1,avm_memcell *mem2){
	assert(mem1->type == string_m && mem2->type == string_m);
	if(!strcmp(mem1->data.strVal,mem2->data.strVal))
		return 1;
	else
		return 0;
}

unsigned char table_check_equal(avm_memcell *mem1,avm_memcell *mem2){
	assert(mem1->type == table_m && mem2->type == table_m);
	if(mem1->data.tableVal == mem2->data.tableVal)
		return 1;
	else
		return 0;
}

unsigned char user_func_check_equal(avm_memcell *mem1,avm_memcell *mem2){
	assert(mem1->type == userfunc_m && mem2->type == userfunc_m);
	if(mem1->data.funcVal == mem2->data.funcVal)
		return 1;
	else
		return 0;
}

unsigned char lib_func_check_equal(avm_memcell *mem1,avm_memcell *mem2){
	assert(mem1->type == libfunc_m && mem2->type == libfunc_m);
	if(mem1->data.libfuncVal == mem2->data.libfuncVal)
		return 1;
	else
		return 0;
}
