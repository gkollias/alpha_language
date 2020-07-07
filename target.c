#include "target.h"
#define VM_MAGIC          1555
#define NULL_VMARG_VALUE  -1
extern FILE* syntax_fp;
extern FILE* instructions_fp;
extern FILE* instr_bin_fp;
extern quad* quads;
extern unsigned currQuad;

instruction*  instructions = (instruction *) 0;

unsigned i_total = 0;
unsigned int currInstruction = 0;
unsigned int instruction_ctr = 0;
incomplete_jump*    ij_head = (incomplete_jump*) 0;
unsigned            ij_total = 0;

/* Stacks */
symbol_list* func_stack = NULL;
/* Const Tables */
double*   numConsts = NULL;
unsigned  totalNumConsts = 0;
char**    stringConsts = NULL;
unsigned  totalStringConsts = 0;
char**    namedLibfuncs = NULL;
unsigned  totalNamedLibfuncs = 0;
userfunc* userFuncs = NULL;
unsigned  totalUserFuncs = 0;

unsigned  const_num_curr_size = 0;
unsigned  const_string_curr_size = 0;
unsigned  lib_func_curr_size = 0;
unsigned  user_func_curr_size = 0;

/* Variables for binary file */
int magic   = VM_MAGIC;
int globals = 0;

#define I_EXPAND_SIZE 1024
#define I_CURR_SIZE   (i_total*sizeof(instruction))
#define I_NEW_SIZE    (I_EXPAND_SIZE*sizeof(instruction)+I_CURR_SIZE)


void i_expand (void) {
	assert(i_total == currInstruction);
	instruction* i = (instruction *)malloc(I_NEW_SIZE);
	if(instructions){
		memcpy(i ,instructions , I_CURR_SIZE);
		free(instructions);
	}
	instructions = i;
	i_total += I_EXPAND_SIZE;
	fprintf(syntax_fp,"Successfully expanded\n");
}

#define CONST_NUM_EXPAND_SIZE 1024
#define CONST_NUM_CURRENT_SIZE   (const_num_curr_size*sizeof(double))
#define CONST_NUM_NEW_SIZE    (CONST_NUM_EXPAND_SIZE*sizeof(double)+CONST_NUM_CURRENT_SIZE)

void const_num_expand(void){
	assert(const_num_curr_size == totalNumConsts);
	double* d = (double *)malloc(CONST_NUM_NEW_SIZE);
	if(numConsts){
		memcpy(d ,numConsts , CONST_NUM_CURRENT_SIZE);
		free(numConsts);
	}
	numConsts = d;
	const_num_curr_size += CONST_NUM_EXPAND_SIZE;
	fprintf(syntax_fp,"Num Successfully expanded\n");
}


#define CONST_STRING_EXPAND_SIZE 1024
#define CONST_STRING_CURRENT_SIZE   (const_string_curr_size*sizeof(char *))
#define CONST_STRING_NEW_SIZE    (CONST_STRING_EXPAND_SIZE*sizeof(char *)+CONST_STRING_CURRENT_SIZE)

void const_string_expand(void){
	assert(const_string_curr_size == totalStringConsts);
	char** s = (char **)malloc(CONST_STRING_NEW_SIZE);
	if(stringConsts){
		memcpy(s ,stringConsts , CONST_STRING_CURRENT_SIZE);
		free(stringConsts);
	}
	stringConsts = s;
	const_string_curr_size += CONST_STRING_EXPAND_SIZE;
	fprintf(syntax_fp,"String Successfully expanded\n");
}


#define USER_FUNC_EXPAND_SIZE 128
#define USER_FUNC_CURRENT_SIZE   (user_func_curr_size*sizeof(userfunc *))
#define USER_FUNC_NEW_SIZE    (USER_FUNC_EXPAND_SIZE*sizeof(userfunc *)+USER_FUNC_CURRENT_SIZE)

void user_func_expand(void){
	assert(user_func_curr_size == totalUserFuncs);
	userfunc* f = (userfunc *)malloc(USER_FUNC_NEW_SIZE);
	if(userFuncs){
		memcpy(f ,userFuncs , USER_FUNC_CURRENT_SIZE);
		free(userFuncs);
	}
	userFuncs = f;
	user_func_curr_size += USER_FUNC_EXPAND_SIZE;
	fprintf(syntax_fp,"User Funcs Successfully expanded\n");
}

#define LIB_FUNC_EXPAND_SIZE 16
#define LIB_FUNC_CURRENT_SIZE   (lib_func_curr_size*sizeof(char *))
#define LIB_FUNC_NEW_SIZE    (LIB_FUNC_EXPAND_SIZE*sizeof(char *)+LIB_FUNC_CURRENT_SIZE)

void lib_func_expand(void){
	assert(lib_func_curr_size == totalNamedLibfuncs);
	char** f = (char **)malloc(LIB_FUNC_NEW_SIZE);
	if(namedLibfuncs){
		memcpy(f ,namedLibfuncs , LIB_FUNC_CURRENT_SIZE);
		free(namedLibfuncs);
	}
	namedLibfuncs = f;
	lib_func_curr_size += LIB_FUNC_EXPAND_SIZE;
	fprintf(syntax_fp,"Lib Funcs Successfully expanded\n");
}

/******************************   This array is used only for testing   ***********************************/
char**    namedVarNames = NULL;
unsigned  totalVars = 0;
unsigned  var_names_curr_size = 0;

#define VAR_EXPAND_SIZE 100
#define VAR_CURRENT_SIZE   (var_names_curr_size*sizeof(char *))
#define VAR_NEW_SIZE    (VAR_EXPAND_SIZE*sizeof(char *) + VAR_CURRENT_SIZE)

void var_names_expand(void){
	assert(var_names_curr_size == totalVars);
	char** v = (char **)malloc(VAR_NEW_SIZE);
	if(namedVarNames){
		memcpy(v ,namedVarNames , VAR_CURRENT_SIZE);
		free(namedVarNames);
	}
	namedVarNames = v;
	var_names_curr_size += VAR_EXPAND_SIZE;
	fprintf(syntax_fp,"Var Names Successfully expanded\n");
}
unsigned add_var_names(char* str,expression* e)
{
	unsigned tmp,i,flag = 0;
	if(var_names_curr_size == totalVars)
		var_names_expand();

	if(e->expr_value.symbol->space == PROGRAM_VAR)
	{
		for(i = 0;i < totalVars;i++)
			if(!strcmp(namedVarNames[i],e->expr_value.symbol->types.var->name))
			{flag = 1;break;}
		if(flag == 0 )//|| e->expr_value.symbol->types.var->name[0] == '$')
			globals++;
	}
	namedVarNames[totalVars] = strdup(str);
	tmp=totalVars;
	totalVars++;
	return tmp;
}
/**************************************************************************************************************/


void t_emit (instruction* t){
	
	instruction* i;
	if(currInstruction == i_total)
		i_expand();
	
	i          = instructions+currInstruction;
	i->opcode  = t->opcode;
	i->arg1    = t->arg1;
	i->arg2    = t->arg2;;
	i->result  = t->result;
	i->srcLine = t->srcLine;
	currInstruction++;

	fprintf(syntax_fp,"Successfully emited %d %d\n",i->opcode,i->srcLine);
}

vmarg* new_vmarg_void (void)
{
	vmarg* arg = (vmarg *) malloc(sizeof (vmarg));
	memset(arg, 0, sizeof (vmarg));
	return arg;
}


void make_operand(expression *e,vmarg *arg){
	assert(arg);
/*printf("***expression type: %d *** \n",e->type);*/
	switch(e->type){

		case TYPE_VAR:
		case TYPE_TEMP_VAR:
		case TYPE_NEWTABLE:
		case TYPE_TABLE_ELEMENT:
		case TYPE_CONST_BOOL: {
		  
		  assert(e->expr_value.symbol);
		  /******************4 testing*************************/
		  add_var_names(e->expr_value.symbol->types.var->name,e);
		  /***************************************************/
		  arg->val=e->expr_value.symbol->offset;	
		  switch(e->expr_value.symbol->space){

			  case PROGRAM_VAR:    arg->type=global_a;break;
		 	  case FUNCTION_LOCAL: arg->type=local_a;  break;
			  case FORMAL_ARG:     arg->type=formal_a; break;
			  default: assert(0);
		  }

	  	  break;				
	  	}
		
		case TYPE_BOOL: {
			arg->val = e->is_true;
			arg->type= bool_a;
			break;
		}

		case TYPE_STRING: {
			arg->val = consts_newstring(e->expr_value.str_value);
			arg->type= string_a;
			break;
		}

		case TYPE_INT: {
			arg->val = consts_newnumber(e->expr_value.i_value);
			arg->type= number_a;
			break;
		}

		case TYPE_FLOAT: {
			arg->val = consts_newnumber(e->expr_value.f_value);
			arg->type= number_a;
			break;
		}

		case TYPE_NIL:  {
			arg->type = nil_a;
			break;
		}

		/*Functions*/

		case TYPE_PROGRAM_FUNCTION:{
			
			//arg->val  = e->expr_value.symbol->t_address;
			assert(e->expr_value.symbol->types.fun);
			arg->val  = add_user_functions(e->expr_value.symbol->types.fun->name,
					               e->expr_value.symbol->t_address,
						       e->expr_value.symbol->types.fun->total_locals);

			arg->type = userfunc_a;
			break;
		}
		case TYPE_LIBRARY_FUNCTION:{
		
			//arg->val  = e->expr_value.symbol->t_address;
			arg->val = add_lib_functions( e->expr_value.symbol->types.fun->name );
			arg->type = libfunc_a;
			break;
		}
		case TYPE_LABEL:{
		
			arg->val  = e->expr_value.label;
			arg->type = label_a;
			break;
		}
		default: printf("type = %d\n",e->type);assert(0);

	}
}
void make_number_operand(vmarg* arg,double val)
{
	arg->val = consts_newnumber(val);
	arg->type = number_a;
}

void make_bool_operand(vmarg* arg,bool_t val)
{
	arg->val = val;
	arg->type = bool_a;
}

void make_retval_operand(vmarg* arg){ arg->type = retval_a;}

unsigned consts_newnumber(double val)
{
	unsigned tmp,i;
	for(i=0;i<totalNumConsts;i++)
		if(numConsts[i] == val)
			return i;
	if(const_num_curr_size == totalNumConsts)
		const_num_expand();
	
	numConsts[totalNumConsts] = val;
	tmp=totalNumConsts;
	totalNumConsts++;
	return tmp;
}

unsigned consts_newstring(char* str)
{
	unsigned tmp,i;
	for(i=0;i<totalStringConsts;i++)
		if(!strcmp(stringConsts[i],str))
			return i;
	if(const_string_curr_size == totalStringConsts)
		const_string_expand();

	stringConsts[totalStringConsts] = strdup(str);

	tmp=totalStringConsts;
	totalStringConsts++;
	return tmp;
}

unsigned add_lib_functions(char* str)
{
	unsigned tmp,i;
	for(i = 0;i<totalNamedLibfuncs;i++)
		if(!strcmp(str,namedLibfuncs[i]))
			return i;
	if(lib_func_curr_size == totalNamedLibfuncs)
		lib_func_expand();
	namedLibfuncs[totalNamedLibfuncs] = strdup(str);

	tmp=totalNamedLibfuncs;
	totalNamedLibfuncs++;
	return tmp;
}

unsigned add_user_functions(char* id,unsigned address,unsigned localSize){
	unsigned tmp,i;
	userfunc* f;
	for(i = 0;i<totalUserFuncs;i++)
	{
		if(userFuncs[i].address == address)
			return i;
	}
	if(user_func_curr_size == totalUserFuncs)
		user_func_expand();
	f = (userfunc *)malloc(sizeof(userfunc));
	f->id        = strdup(id);
	f->address   = address;
	f->localSize = localSize;
	//(*userFuncs) = *f;
	//userFuncs++;
	userFuncs[totalUserFuncs] = *f;

	tmp=totalUserFuncs;
	totalUserFuncs++;
	return tmp;
}

void generate(vmopcode op,quad* q)
{
	instruction* i = instruction_malloc();
	i->result = new_vmarg_void();
	i->opcode = op;
	assert(q);
	assert(q->result);//result is always != null
	make_operand(q->result,i->result);
	if(q->arg1 != NULL)
	{
		i->arg1 = new_vmarg_void();
		make_operand(q->arg1,i->arg1);
	}
	if(q->arg2 != NULL)
	{
		i->arg2 = new_vmarg_void();
		make_operand(q->arg2,i->arg2);
	}
	//printf("opcode : %d\n",op);

	q->taddress=currInstruction;
	i->srcLine = q->line;
	t_emit(i);

}

void generate_ADD(quad *q)		{ generate(VM_ADD,q); }
void generate_SUB(quad *q)		{ generate(VM_SUB,q); }
void generate_MUL(quad *q)		{ generate(VM_MUL,q); }
void generate_DIV(quad *q)		{ generate(VM_DIV,q); }
void generate_MOD(quad *q)		{ generate(VM_MOD,q); }
void generate_NEWTABLE (quad *q) 	{ generate(VM_NEWTABLE, q); }
void generate_TABLEGETELEM (quad *q) 	{ generate(VM_TABLE_GET_ELEM,q); }
void generate_TABLESETELEM (quad *q) 	{ generate(VM_TABLE_SET_ELEM,q); }
void generate_ASSIGN (quad *q) 		{ generate(VM_ASSIGN, q); }

void generate_NOP (void){ 
	instruction* i = instruction_malloc();
	i->opcode=VM_NOP; 
	t_emit(i);
}


void generate_relational (vmopcode op, quad* q) {
	instruction* i = instruction_malloc();

	i->result = new_vmarg_void();
	i->opcode = op;
	
	if(q->arg1 != NULL)
	{
		i->arg1 = new_vmarg_void();
		make_operand(q->arg1, i->arg1);
	}
	if(q->arg2 != NULL)
	{
		i->arg2 = new_vmarg_void();
		make_operand(q->arg2, i->arg2);
	}

	i->result->type = label_a;
	assert(q->result->type == TYPE_LABEL);
	if (q->result->expr_value.label < q->label)
		i->result->val = quads[q->result->expr_value.label].taddress;
	else
		add_incomplete_jump(currInstruction, q->result->expr_value.label);

	q->taddress = currInstruction;
	i->srcLine = q->line;
	t_emit(i);
}

void generate_JUMP (quad* q)		{ generate_relational(VM_JUMP, q); }
void generate_IF_EQ (quad* q)		{ generate_relational(VM_JEQ, q); }
void generate_IF_NOTEQ(quad* q) 	{ generate_relational(VM_JNE, q); }
void generate_IF_GREATER (quad* q) 	{ generate_relational(VM_JGT, q); }
void generate_IF_GREATEREQ(quad* q) 	{ generate_relational(VM_JGE, q); }
void generate_IF_LESS (quad* q) 	{ generate_relational(VM_JLT, q); }
void generate_IF_LESSEQ (quad* q) 	{ generate_relational(VM_JLE, q); }


void generate_PARAM(quad* q) {
	instruction* i = instruction_malloc();

	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	i->srcLine = q->line;
	i->opcode = VM_PUSH_ARG;
	if(q->result != NULL)
		make_operand(q->result, i->result);
	t_emit(i);
}

void generate_CALL(quad* q) {
	instruction* i = instruction_malloc();

	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	i->srcLine = q->line;
	i->opcode = VM_CALL;
	assert(q->result);//there should always be present!
	make_operand(q->result, i->result);
	t_emit(i);
}

void generate_GETRETVAL(quad* q) {
	instruction* i = instruction_malloc();

	i->arg1 = new_vmarg_void();
	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	i->srcLine = q->line;
	i->opcode = VM_ASSIGN;
	if(q->result != NULL)
		make_operand(q->result, i->result);
	
	make_retval_operand(i->arg1);
	t_emit(i);
}

void generate_FUNCSTART(quad* q){
	assert(q);
	instruction* i = instruction_malloc();
	hashRecord* f = q->result->expr_value.symbol;
	/******************* jump add **************************/
	f->returnList = append_instruction_list(f->returnList,currInstruction);

	i->result = new_vmarg_void();
	i->opcode = VM_JUMP;
	i->result->type = label_a;
	t_emit(i);
	/******************************************************/

	i = instruction_malloc();
	i->result = new_vmarg_void();
	f->t_address = currInstruction;
	q->taddress = currInstruction;
	
	//add_user_functions(f->types.fun->name,f->t_address,f->types.fun->total_locals);
	//printf("@func start:%d\n\n\n\n\n\n",q->result->expr_value.symbol->types.fun->total_locals);
	func_stack = push_symbol(func_stack,f);
	i->opcode = VM_FUNC_ENTER;
	i->srcLine = q->line;
	if(q->result != NULL)
		make_operand(q->result,i->result);
	t_emit(i);
}

void generate_FUNCEND(quad* q){
	hashRecord* f = func_stack->symbol;
	instruction* i = instruction_malloc();
	func_stack = pop_symbol(func_stack);
	i->result = new_vmarg_void();
	
	patch_instruction_label(f->returnList,currInstruction);
	
	q->taddress = currInstruction;
	i->opcode = VM_FUNC_EXIT;
	i->srcLine = q->line;
	if(q->result != NULL)
		make_operand(q->result,i->result);
	t_emit(i);

}


void generate_RETURN(quad* q){
	hashRecord* f;
	instruction* i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->result = new_vmarg_void();

	q->taddress = currInstruction;
	i->opcode = VM_ASSIGN;
	i->srcLine = q->line;
	make_retval_operand(i->result);//was result
	
	if(q->result)
		make_operand(q->result,i->arg1);

	t_emit(i);

	f = func_stack->symbol;
	f->returnList = append_instruction_list(f->returnList,currInstruction);


	i = instruction_malloc();
	i->result = new_vmarg_void();

	i->opcode = VM_JUMP;
	i->result->type = label_a;
	t_emit(i);
	return;
}
/*********************logical*********************/
void generate_NOT (quad* q) {
	instruction* i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	q->taddress = currInstruction;

	i->opcode = VM_JEQ;
	make_operand(q->arg1, i->arg1);
	make_bool_operand(i->arg2, B_FALSE);
	i->result->type = label_a;
	i->result->val  = currInstruction + 3;
	t_emit(i);

	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_FALSE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = NULL;
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_JUMP;
	//reset_operand (i->arg1);
	//reset_operand (i->arg2);
	i->result->type = label_a;
	i->result->val  = currInstruction + 2;
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_TRUE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
}

void generate_OR (quad* q) {
	instruction* i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	
	i->opcode = VM_JEQ;
	make_operand(q->arg1, i->arg1);
	make_bool_operand(i->arg2, B_TRUE);
	i->result->type = label_a;
	i->result->val  = currInstruction + 4;
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	i->opcode = VM_JEQ;
	make_operand(q->arg2, i->arg1);
	make_bool_operand(i->arg2, B_TRUE);
	i->result->type = label_a;
	i->result->val  = currInstruction + 3;
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_FALSE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = NULL;
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_JUMP;
	//reset_operand (i->arg1);
	//reset_operand (i->arg2);
	i->result->type = label_a;
	i->result->val  = currInstruction + 2;
	t_emit(i);

	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_TRUE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
}


void generate_AND (quad* q) {
	instruction* i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	
	i->opcode = VM_JEQ;
	make_operand(q->arg1, i->arg1);
	make_bool_operand(i->arg2, B_FALSE);
	i->result->type = label_a;
	i->result->val  = currInstruction + 4;
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	i->opcode = VM_JEQ;
	make_operand(q->arg2, i->arg1);
	make_bool_operand(i->arg2, B_FALSE);
	i->result->type = label_a;
	i->result->val  = currInstruction + 3;
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_TRUE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
	
	i = instruction_malloc();
	i->arg1 = NULL;
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_JUMP;
	//reset_operand (i->arg1);
	//reset_operand (i->arg2);
	i->result->type = label_a;
	i->result->val  = currInstruction + 2;
	t_emit(i);

	i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = NULL;
	i->result = new_vmarg_void();
	i->opcode = VM_ASSIGN;
	make_bool_operand(i->arg1, B_FALSE);
	//reset_operand(i->arg2);
	make_operand(q->result, i->result);
	t_emit(i);
}



void generate_UMINUS (quad* q){
	instruction* i = instruction_malloc();
	i->arg1 = new_vmarg_void();
	i->arg2 = new_vmarg_void();
	i->result = new_vmarg_void();
	q->taddress = currInstruction;
	
	i->opcode = VM_MUL;
	make_operand(q->arg1, i->arg1);
	make_number_operand(i->arg2, -1);
	//i->result->type = i->arg1->type;
	make_operand(q->result,i->result);
	t_emit(i);
}

void patch_incomplete_jumps(){
	if(ij_head == NULL)
		return;
	while(ij_head != NULL){
		if(ij_head->iaddress == currQuad )
			instructions[ij_head->instrNo].result->val = currInstruction;
		else
			instructions[ij_head->instrNo].result->val = quads[ij_head->iaddress].taddress;
		ij_head = ij_head->next;
	}
	return;
}


void add_incomplete_jump(unsigned instrNo, unsigned iaddress){
	incomplete_jump* tmp;
	incomplete_jump* new_node;
	new_node = (incomplete_jump *)malloc(sizeof(incomplete_jump));
	new_node->instrNo = instrNo;
	new_node->iaddress = iaddress;
	new_node->next = NULL;
	if(ij_head == NULL)
		ij_head = new_node;
	else
	{
		tmp = ij_head;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_node;
	}
	return;
}


symbol_list* push_symbol(symbol_list* head,hashRecord* new_symbol){
	symbol_list* tmp;
	if(head == NULL)
	{
		head = (symbol_list*)malloc(sizeof(symbol_list));
		head->symbol = new_symbol;
		head->next   = NULL;
		return head;
	}
	tmp = (symbol_list*)malloc(sizeof(symbol_list));
	tmp->symbol = new_symbol;
	tmp->next = head;
	head = tmp;
	return head;
}

symbol_list* pop_symbol(symbol_list* head){
	//hashRecord* sym = head->symbol;
	assert(head);	
	head = head->next;
	return head;
}


num_list* patch_instruction_label(num_list* list , unsigned label){
	num_list* tmp = list;
	int patch_flag = 0;
	
	if(list == NULL)
	{/*fprintf(syntax_fp,"NULL list at patch@\n");*/return NULL;}
	while(tmp!=NULL)
	{
		//printf(syntax_fp,"I patched @ %d    label = %d\n",tmp->num,label);
		if(tmp->num < currInstruction -1)
		{
			if(instructions[tmp->num+1].opcode == VM_FUNC_ENTER)
			{
				instructions[tmp->num].result->val = label+1;
				patch_flag = 1;
			}
		}
		if(patch_flag == 0)
		{
			instructions[tmp->num].result->val = label;
		}
		tmp=tmp->next;
		patch_flag = 0;
	}
	return NULL;
}

num_list* append_instruction_list(num_list* head,unsigned instrNo){
	num_list* tmp = head;
	num_list* new_node = (num_list*)malloc(sizeof(num_list));
	new_node->num = instrNo;
	new_node->next = NULL;
	if(head == NULL)
		head = new_node;
	else
	{
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_node;
	}
	return head;
}


void reset_operand(vmarg* arg){
	assert(arg);
	arg->type = undef_a;
	arg->val  = 0;
	//free(arg);
	arg = NULL;
	return;
}



/**********************************  PRINT FUNCTION  ************************************/
int var_ctr = 0;

#define PRINT_VM_ARG(VM_ARG)											\
	if (VM_ARG == NULL) fprintf(instructions_fp, "(empty)");						\
	else if (VM_ARG->type == label_a)						  			\
		fprintf(instructions_fp," %d ",VM_ARG->val);							\
	else if (VM_ARG->type == retval_a )									\
		fprintf(instructions_fp," retval %d ",VM_ARG->val);						\
	else if (VM_ARG->type == nil_a)										\
		fprintf(instructions_fp," NIL ");								\
	else if (VM_ARG->type == bool_a)									\
	{													\
		if(VM_ARG->val == 0)										\
			fprintf(instructions_fp," FALSE ");							\
		else if(VM_ARG->val == 1)									\
			fprintf(instructions_fp," TRUE ");							\
	}													\
	else if(VM_ARG->type == global_a || VM_ARG->type == formal_a || VM_ARG->type ==local_a)			\
	{fprintf(instructions_fp," %s ",namedVarNames[var_ctr]);var_ctr++;}					\
	else if(VM_ARG->type == userfunc_a)									\
		fprintf(instructions_fp," %s ",userFuncs[VM_ARG->val].id);					\
	else if(VM_ARG->type == libfunc_a)									\
		fprintf(instructions_fp," %s ",namedLibfuncs[VM_ARG->val]);					\
	else if(VM_ARG->type == number_a)									\
		fprintf(instructions_fp," %f ",numConsts[VM_ARG->val]);						\
	else if(VM_ARG->type == string_a)									\
		fprintf(instructions_fp," \"%s\" ",stringConsts[VM_ARG->val]);					\
	else if(VM_ARG->type == undef_a)									\
		fprintf(instructions_fp," UNDEF ");								\
	else {fprintf(stderr,"expr type: %d ",VM_ARG->type);assert(!"Unknown type in expression. Programmer's error");}


void print_instruction (instruction* i)
{
	const char* operator;
	if(i == NULL)
	{fprintf(syntax_fp,"i == NULL\n");return;}
	fprintf(instructions_fp,"%d:",instruction_ctr++);
	switch (i->opcode) {
		case VM_ASSIGN: operator = "ASSIGN"; break;
		case VM_ADD: operator = "ADD"; break;
		case VM_SUB: operator = "SUB"; break;
		case VM_MUL: operator = "MUL"; break;
		case VM_DIV: operator = "DIV"; break;
		//case VM_UMINUS: operator = "UMINUS"; break;
		case VM_MOD: operator = "MOD"; break;
		//case VM_AND: operator = "AND"; break;
		//case VM_OR: operator = "OR"; break;
		//case VM_NOT: operator = "NOT"; break;
		case VM_JEQ: operator = "JEQ"; break;
		case VM_JNE: operator = "JNE"; break;
		case VM_JLE: operator = "JLE"; break;
		case VM_JGE: operator = "JGE"; break;
		case VM_JLT: operator = "JLT"; break;
		case VM_JGT: operator = "JGT"; break;
		case VM_CALL: operator = "CALL"; break;
		case VM_PUSH_ARG: operator = "PUSH_ARG"; break;
		case VM_FUNC_ENTER: operator = "FUNC_ENTER"; break;
		case VM_FUNC_EXIT: operator = "FUNC_EXIT"; break;
		case VM_NEWTABLE: operator = "NEWTABLE"; break;
		case VM_TABLE_GET_ELEM: operator = "TABLE_GET_ELEM"; break;
		case VM_TABLE_SET_ELEM: operator = "TABLE_SET_ELEM"; break;
		case VM_JUMP: operator = "JUMP"; break;

		default: operator = "UNKNOWN"; break;
	}
	/*******  write to binary file  *******/
	fwrite(&i->opcode,1,1,instr_bin_fp);
	fwrite_vmarg(i->result,instr_bin_fp);
	fwrite_vmarg(i->arg1,instr_bin_fp);
	fwrite_vmarg(i->arg2,instr_bin_fp);
	/*************************************/
	fprintf(instructions_fp, "%s ", operator);
	PRINT_VM_ARG(i->result);
	fprintf(instructions_fp, ", ");
	PRINT_VM_ARG(i->arg1);
	fprintf(instructions_fp, ", ");
	PRINT_VM_ARG(i->arg2);
	fprintf(instructions_fp, "\n");
	fflush(stdout);
}
#undef PRINT_VM_ARG


void printInstructions(void)
{
	int i;
	fwrite(&magic,4,1,instr_bin_fp);
	fwrite(&globals,4,1,instr_bin_fp);
	fwrite(&currInstruction,4,1,instr_bin_fp);
	for(i=0;i<currInstruction;i++)
		print_instruction (&instructions[i]);
}

void fwrite_vmarg(vmarg* arg,FILE* fp){
	if(arg == NULL)
	{
		int null_value = NULL_VMARG_VALUE;
		fwrite(&null_value,1,1,fp);//type
		fwrite(&null_value,4,1,fp);//value
		return;
	}
	else
	{
		fwrite(&arg->type,1,1,fp);//type
		fwrite(&arg->val,4,1,fp);//value
		return;
	}

}

void print_const_nums(void){
	int i;
	fwrite(&totalNumConsts,4,1,instr_bin_fp);
	printf("\t******** Const Numbers **********\n");
	for(i = 0;i < totalNumConsts; i++)
	{
		printf("%d: %f \n",i,numConsts[i]);
		fwrite(&numConsts[i],8,1,instr_bin_fp);
	}
}

void print_const_strings(void){
	int i,str_length;
	fwrite(&totalStringConsts,4,1,instr_bin_fp);
	printf("\t******** Const Strings **********\n");
	for(i = 0;i < totalStringConsts; i++)
	{
		str_length = strlen(stringConsts[i]);
		printf("%d: \"%s\" \n",i,stringConsts[i]);
		fwrite(&str_length,4,1,instr_bin_fp);
		fwrite(stringConsts[i],str_length,1,instr_bin_fp);//-1
	}
}

void print_user_funcs(void){
	int i,str_length;
	fwrite(&totalUserFuncs,4,1,instr_bin_fp);
	printf("\t******** User Functions **********\n");
	for(i = 0;i < totalUserFuncs; i++)
	{
		str_length = strlen(userFuncs[i].id);
		printf("%d:  id:%s address:%d locals:%d\n",i,userFuncs[i].id,userFuncs[i].address,userFuncs[i].localSize);
		fwrite(&str_length,4,1,instr_bin_fp);
		fwrite(userFuncs[i].id,str_length,1,instr_bin_fp);//-1
		fwrite(&userFuncs[i].address,4,1,instr_bin_fp);
		fwrite(&userFuncs[i].localSize,4,1,instr_bin_fp);
	}
}


void print_lib_funcs(void){
	int i,str_length;
	fwrite(&totalNamedLibfuncs,4,1,instr_bin_fp);
	printf("\t******** Lib Functions **********\n");
	for(i = 0;i < totalNamedLibfuncs; i++)
	{
		str_length = strlen(namedLibfuncs[i]);
		printf("%d: %s \n",i,namedLibfuncs[i]);
		fwrite(&str_length,4,1,instr_bin_fp);
		fwrite(namedLibfuncs[i],str_length,1,instr_bin_fp);//-1
	}
}



instruction* instruction_malloc( void ){
	instruction *i = (instruction *)malloc(sizeof(instruction));
	i->arg1   = NULL;
	i->arg2   = NULL;
	i->result = NULL;
	return i;
}


