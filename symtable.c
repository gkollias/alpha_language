#include "symtable.h"

extern FILE* syntax_fp;
extern FILE* symbol_fp;

scope* scope_head=NULL;
hashRecord* print_head=NULL; 



int hash(char* key)
{

	int i;
	
	if(key != NULL)
	{
		if(key[0] == '$')
			i = key[1]-'a';
		else
			i=key[0]-'a';
	}
	else 
		return -1;
	if(i<0)
		i=key[0]-'A'+26;
	if(key[0]=='A')
		fprintf(syntax_fp,"key = %d",key[0]-'A');

	if(i<0)
		i = 51;

	return i%HASHSIZE;
}





varType* newVar(char *name)
{
	varType* tmp;

	tmp=(varType *)malloc(sizeof(varType));
	tmp->name=strdup(name);

	return tmp;
}



funType* newFun(char *name,arglist* args)
{

	funType* tmp;

	tmp=(funType *)malloc(sizeof(funType));
	tmp->name=strdup(name);
	tmp->list=args;
	
	return tmp;
}



arglist* newArg(varType* var)
{

	arglist* tmp;

	tmp=(arglist *)malloc(sizeof(arglist));
	tmp->var=var;
	tmp->next=NULL;

	return tmp;
}



hashRecord* newRec(void* kind,symbol_t type,int scope,unsigned line,unsigned offset,scopespace_t space)
{

	hashRecord* tmp;

	tmp=(hashRecord *)malloc(sizeof(hashRecord));
	tmp->isActive=1;

	if(type == VAR_S || TEMP_VAR_S)
		tmp->types.var=kind;
	else if(type == PROGRAM_FUNCTION_S || type == LIBRARY_FUNCTION_S)
		tmp->types.fun=kind;

	tmp->type=type;
	tmp->scope=scope;
	tmp->line=line;
	tmp->offset=offset;
	tmp->space=space;
	tmp->next=NULL;

	return tmp;
}



scope *newSymtable(int scp)
{

	scope* tmp;
	int i;

	tmp=(scope *)malloc(sizeof(scope));
	tmp->defscope=scp;

	for(i=0;i<HASHSIZE;i++)
	{
		tmp->sym[i]=NULL;//(hashRecord *)malloc(sizeof(hashRecord));
	}

	tmp->next=NULL;

	return tmp;
}



arglist* insertArgList(arglist* head,arglist* argument)
{

	arglist* tmp;
	tmp=head;

	if(tmp==NULL)
	{
		head=argument;
	}
	else
	{
		while(tmp->next!=NULL)
		{
			tmp=tmp->next;
		}
		
		tmp->next=argument;
	}

	return head;
}



arglist* lookArgList(arglist* head,char* key)
{

	arglist* tmp=head;
	

	while(tmp!=NULL)
	{
		if(!strcmp(tmp->var->name,key))
		{
			fprintf(syntax_fp,"ID %s found in arglist!\n",key);
			return tmp;
		}
		else
			tmp=tmp->next;
	}
	fprintf(syntax_fp,"ID %s does not exist in arglist!\n",key);
	return tmp;
}


scope* lookScopes(int Scope)
{

	scope* tmp=scope_head;

	while(tmp)
	{
		if(tmp->defscope==Scope)
			return tmp;

		else
			tmp=tmp->next;
	}

	return NULL;
}


hashRecord* insertHash(hashRecord* head,hashRecord* in)
{

	hashRecord* tmp;
	tmp=head;

	if(tmp==NULL)
	{
		head=in;
	}
	else
	{
		while(tmp->next!=NULL)
		{
			tmp=tmp->next;
		}
		tmp->next=in;
	}
	
	return head;
	

}


scope* insertNewScope(int scpe)
{
	scope* tmp=lookScopes(scpe);
	scope* temp=scope_head;
	if(tmp!=NULL)
		return NULL;

 	tmp=newSymtable(scpe);

	if(temp==NULL)
		scope_head=tmp;

	else
	{
		while(temp->next!=NULL)
			temp=temp->next;


		temp->next=tmp;
	}

	return tmp;
}

hashRecord* insertScopes(int scpe,hashRecord* in,char* key)
{

	scope* tmp=lookScopes(scpe);

	if(tmp==NULL)
		tmp=insertNewScope(scpe);	

	tmp->sym[hash(key)]=insertHash(tmp->sym[hash(key)],in);
	return in;

}


void hide(int scpe)
{

	int i;
	hashRecord* rec;

	scope* tmp=lookScopes(scpe);


	if(tmp!=NULL)
	{
		for(i=0;i<HASHSIZE;i++)
		{
			if(tmp->sym[i]!=NULL)
			{
				rec=tmp->sym[i];
				while(rec!=NULL)
				{
					rec->isActive=0;
					rec=rec->next;
				}
			}
		}
	}
}

hashRecord* lookScope(char *key,int scpe)
{

	hashRecord* rec;
	scope* tmp=lookScopes(scpe);

	if(tmp!=NULL)
	{
		rec=tmp->sym[hash(key)];
	
		while(rec!=NULL)
		{	
			if(rec->type == VAR_S || rec->type == TEMP_VAR_S && rec->isActive)
			{

				if(!strcmp(rec->types.var->name,key))
				{

					fprintf(syntax_fp,"ID %s Exists!\n",key);
					return rec;

				}
				else
					rec=rec->next;
			}
			else if((rec->type == PROGRAM_FUNCTION_S || rec->type == LIBRARY_FUNCTION_S) && rec->isActive)
			{
				if(!strcmp(rec->types.fun->name,key))
				{
					fprintf(syntax_fp,"Function %s Exists!\n",key);
					return rec;
				}
				else
					rec=rec->next;
			}
			else
				rec = rec->next;
		}
	}
	fprintf(syntax_fp,"ID %s Does Not Exist! at scope:%d\n",key,scpe);
	return NULL;
}




void printArgList(arglist* head)
{
	arglist* tmp=head;
	if(tmp==NULL)
	{
		fprintf(symbol_fp,"Function Has No Arguments!\n");
		return;
	}
	
	fprintf(symbol_fp,"The Arguments are:");
	while(tmp!=NULL)
	{
		if(tmp->next==NULL)
		{
			fprintf(symbol_fp,"%s\n",tmp->var->name);
			break;
		}
			
		fprintf(symbol_fp,"%s,",tmp->var->name);
		tmp=tmp->next;
		
	}
}
	
void printList(hashRecord* head)
{

	hashRecord* tmp=head;

	fprintf(symbol_fp,"\n");
	
	if(tmp==NULL)
		fprintf(symbol_fp,"list is empty\n");	
	else
	{
		while(tmp != NULL)
		{
			fprintf(symbol_fp,"Status:");
			
			if(tmp->isActive)
				fprintf(symbol_fp,"Active\n");	
			else
				fprintf(symbol_fp,"InActive\n");
			
			if(tmp->type==TEMP_VAR_S)
			{
				fprintf(symbol_fp,"VARIABLE\n");
				fprintf(symbol_fp,"Name:%s\n",tmp->types.var->name);
			}
			if(tmp->type==VAR_S)
			{
				fprintf(symbol_fp,"VARIABLE\n");
				fprintf(symbol_fp,"Name:%s\n",tmp->types.var->name);
			}
			else if(tmp->type==PROGRAM_FUNCTION_S)
			{
				fprintf(symbol_fp,"PROGRAM FUNCTION\n");
				fprintf(symbol_fp,"Name:%s\n",tmp->types.fun->name);
				printArgList(tmp->types.fun->list);
			}
			else if(tmp->type==LIBRARY_FUNCTION_S)
			{
				fprintf(symbol_fp,"LIBRARY FUNCTION\n");
				fprintf(symbol_fp,"Name:%s\n",tmp->types.fun->name);
					
			}

			fprintf(symbol_fp,"Scope:%d\n",tmp->scope);
			fprintf(symbol_fp,"Line:%d\n",tmp->line);
			fprintf(symbol_fp,"Offset:%d\n",tmp->offset);

			if(tmp->space==PROGRAM_VAR)
				fprintf(symbol_fp,"PROGRAM VARIABLE\n");
			else if(tmp->space==FUNCTION_LOCAL)
				fprintf(symbol_fp,"LOCAL VARIABLE IN FUNCTION\n");
			else if(tmp->space==FORMAL_ARG)
				fprintf(symbol_fp,"ARGUMENT IN FUNCTION\n");
			
			fprintf(symbol_fp,"\n");
			tmp=tmp->next;
		}
	}
}

void printScope(int scpe)
{
	int i;
	scope* tmp=lookScopes(scpe);

	if(tmp!=NULL)
	{
		for(i=0;i<HASHSIZE;i++)
		{
			if(tmp->sym[i]!=NULL)
				printList(tmp->sym[i]);
		}
	}
}

void printSymbolTable()
{
	scope* tmp=scope_head;
	int i=0;

	while(tmp!=NULL)
	{
		printScope(i);
		i++;
		tmp=tmp->next;
	}
}

void libFunctions()
{

	funType* fun;
	hashRecord* tmp;
	unsigned offsetCntr=1;


	fun=newFun("print",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"print");

	fun=newFun("input",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"input");

	fun=newFun("objectmemberkeys",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"objectmemberkeys");

	fun=newFun("objecttotalmembers",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"objecttotalmembers");

	fun=newFun("objectcopy",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"objectcopy");

	fun=newFun("totalarguments",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"totalarguments");

	fun=newFun("argument",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"argument");

	fun=newFun("typeof",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"typeof");

	fun=newFun("strtonum",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"strtonum");

	fun=newFun("sqrt",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"sqrt");

	fun=newFun("cos",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"cos");

	fun=newFun("sin",NULL);
	tmp=newRec(fun,LIBRARY_FUNCTION_S,0,0,offsetCntr++,PROGRAM_VAR);
	insertScopes(0,tmp,"sin");

	
}


stack_num* push( stack_num* head, int funct_scope){
	stack_num* new_node;
	new_node = (stack_num*)malloc(sizeof(stack_num));
	new_node->num = funct_scope;
	new_node->next = head;
	return new_node;
}

stack_num* pop(	stack_num* head ){
	stack_num* tmp = head;
	assert(head);
	head = head->next;
	free(tmp);
	return head;
}


