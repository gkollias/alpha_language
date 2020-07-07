#include "definitions.h"
#include "symtable.h"

extern current_scope;
extern yylineno;
extern FILE* syntax_fp;
extern FILE* quads_fp;

unsigned programVarOffset    = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset     = 0;
unsigned scopeSpaceCounter   = 1;

int tempcounter = 0;

quad*        quads = (quad*) 0;
unsigned     total = 0;
unsigned int currQuad = 0;

#define EXPAND_SIZE 1024
#define CURR_SIZE   (total*sizeof(quad))
#define NEW_SIZE    (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)


void expand (void) {
	//assert(total == currQuad);
	quad* p = (quad*)malloc(NEW_SIZE);
	if(quads){
		memcpy(p ,quads , CURR_SIZE);
		free(quads);quads =NULL;printf("lol:%d,total:%d",currQuad,total);
	}
	quads = p;
	total += EXPAND_SIZE;
	fprintf(syntax_fp,"Successfully expanded\n");
}

void emit ( iopcode op,
	    expression* result,	
            expression* arg1,
            expression* arg2,
            unsigned label,
            unsigned line ){
	quad* p;
	if(label >= total)//changed it from currQuad
		expand();
	
	p         = quads+label;
	p->op     = op;
	p->arg1   = arg1;
	p->arg2   = arg2;
	
	p->result = result;
	p->label  = label;
	p->line   = line;
	currQuad++;
	fprintf(syntax_fp,"Successfully emited\n");
	//printf("label = %d\n",op);
}


scopespace_t currScopeSpace(void) {
	if(scopeSpaceCounter == 1)
		return PROGRAM_VAR;
	else
		if(scopeSpaceCounter%2 == 0)
			return FORMAL_ARG;
		else
			return FUNCTION_LOCAL; 


}


unsigned currScopeOffset (void) {
	switch (currScopeSpace()) {
		case PROGRAM_VAR    : return programVarOffset;
		case FUNCTION_LOCAL : return functionLocalOffset;
		case FORMAL_ARG     : return formalArgOffset;
		default: assert(0);
	}
}

void incCurrScopeOffset (void) {
	switch (currScopeSpace()) {
		case PROGRAM_VAR    : ++programVarOffset; 	break;
		case FUNCTION_LOCAL : ++functionLocalOffset; 	break;
		case FORMAL_ARG     : ++formalArgOffset; 	break;
		default: assert(0);
	}
}

void enterScopeSpace (void)
	{ scopeSpaceCounter++; }

void exitScopeSpace (void)
	{ assert(scopeSpaceCounter > 1); --scopeSpaceCounter; }


void resetFormalArgsOffset(void)
{	formalArgOffset = 0;	}

void resetFunctionLocalsOffset(void)
{	functionLocalOffset = 0;	}

void restoreCurrScopeOffset( unsigned n) {
	switch (currScopeSpace()) {
		case PROGRAM_VAR    : programVarOffset = n;	break;
		case FUNCTION_LOCAL : functionLocalOffset = n;	break;
		case FORMAL_ARG     : formalArgOffset = n;	break;
		default: assert(0);
	}
}



char* new_temp_name (void) {
	char* tmp = (char *)malloc(40);
	sprintf(tmp,"$temp_%d",tempcounter);
	tempcounter++;
 	return tmp;
}
void reset_temp (void) { 
	tempcounter = 0; 
}

hashRecord* new_temp (void) {
	hashRecord* tmp_node = NULL;
	char* name =  strdup(new_temp_name());

	tmp_node = lookScope(name, current_scope);//mporei na xreiastei allagi gia look se ola ta scopes
	if(tmp_node == NULL)
	{	tmp_node =  insertScopes(current_scope,
			             newRec(newVar(name),TEMP_VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
			             ,name);
		incCurrScopeOffset();
		return tmp_node;
	}
	else
		return tmp_node;
}


expression* new_expr_void (void)
{
	expression* expr = (expression*) malloc(sizeof (expression));
	expr->true_list = NULL;
	expr->false_list = NULL;
	memset(expr, 0, sizeof (expression));
	return expr;
}


expression* new_expr_expr (const expression* expr)
{
	expression* result = new_expr_void();
	memcpy(result, expr, sizeof (expression));
	return result;
}


expression* new_expr_symbol (hashRecord* a)
{
	expression* expr = new_expr_void();
	if(a->type == VAR_S)
		expr->type = TYPE_VAR;
	if(a->type == TEMP_VAR_S)
		expr->type = TYPE_TEMP_VAR;
	expr->expr_value.symbol = a;
	return expr;
}


expression* new_jump_expr ( unsigned label)
{
	expression* expr = new_expr_void();
	expr->type = TYPE_LABEL;
	expr->expr_value.label = label;
	return expr;
}

expression* new_bool_expr ( bool_t value)
{
	expression* expr = new_expr_void();
	expr->type = TYPE_BOOL;
	expr->is_true = value;
	expr->expr_value.bool_value = value;
	return expr;
}

expression* new_const_int_expr ( int value)
{
	expression* expr = new_expr_void();
	expr->type = TYPE_INT;
	expr->expr_value.i_value = value;
	return expr;
}

expression* new_const_string_expr(char* str)
{

	expression* expr = new_expr_void();
	expr->type = TYPE_STRING;//TYPE_TABLE_ELEMENT;
	expr->expr_value.str_value=strdup(str);
	return expr;
}


expression* emit_relop(iopcode op , int line , expression* expr1,expression* expr2)
{
	expression* tmp = new_expr_void() ;
	emit(op, new_jump_expr(currQuad + 3), new_expr_expr(expr1), new_expr_expr(expr2),currQuad,line);
	tmp = new_expr_symbol(new_temp());
	emit(OP_ASSIGN , tmp , new_bool_expr(B_FALSE), NULL , currQuad , line);
	emit(OP_JUMP , new_jump_expr(currQuad + 2) , NULL , NULL , currQuad , line);
	emit(OP_ASSIGN , tmp , new_bool_expr(B_TRUE), NULL ,currQuad , line);
	return tmp;
}

expression* emit_if_table_item(expression* expr,unsigned label,int line)
{
	if(expr->type!=TYPE_TABLE_ELEMENT)
		return expr;

	else
	{
		expression* result=new_expr_void();
		result->type=TYPE_TEMP_VAR;
		result->expr_value.symbol=new_temp();
		emit(OP_TABLE_GET_ELEM,result,expr,expr->index,label,line );
		return result;

	}
		
}

int check_types(int type1,int type2,int op)
{


	if(type1!=type2)
	{
		if(type1==TYPE_BOOL)
			return TYPE_BOOL;

		if((type1==TYPE_INT && type2==TYPE_FLOAT)||(type1==TYPE_FLOAT && type2==TYPE_INT))
			return TYPE_FLOAT;
		
		if((type1==TYPE_INT || type1==TYPE_FLOAT || type1==TYPE_BOOL || type1==TYPE_STRING)&&(type2==TYPE_INT || type2==TYPE_FLOAT || type2==TYPE_BOOL || type2==TYPE_STRING))
			return TYPE_ERROR;

		return type1;
	}
	else
	{
		if(type1==TYPE_BOOL && op!=OP_IF_EQ && op!=OP_IF_NOT_EQ) 
			return TYPE_BOOL;

		if(type1==TYPE_STRING && op!=OP_ADD)
			return TYPE_ERROR;

		return type1;
	}
	
}


expr_list* newItem(expression* expr)
{

	expr_list* tmp;

	tmp=(expr_list *)malloc(sizeof(expr_list));
	tmp->exp=new_expr_expr(expr);
	tmp->next=NULL;

	return tmp;
}

expr_list* insertExprList(expr_list* head,expr_list* item)
{

	expr_list* tmp;
	tmp=head;

	if(tmp==NULL)
		head=item;
	else
	{
		while(tmp->next!=NULL)
			tmp=tmp->next;

		tmp->next=item;
	}

	return head;
}


expr_list* push_expr( expr_list* head, expression* new_node){
	expr_list* tmp = (expr_list*)malloc(sizeof(expr_list));
	tmp->exp = new_node;
	tmp->next = head;
	return tmp;
}

expr_list* pop_expr(expr_list* head ){
	assert(head);
	head = head->next;
	return head;
}

expr_list_list* push_expr_list( expr_list_list* head, expr_list* new_node){
	expr_list_list* tmp = (expr_list_list*)malloc(sizeof(expr_list_list));
	tmp->exp = new_node;
	tmp->next = head;
	return tmp;
}

bc_stack* push_bc_stack(bc_stack* head,
		stack_num* break_stack,stack_num* continue_stack,stack_num* num_of_breaks_stack,stack_num* num_of_continues_stack,
		stack_num* for_start_stack,stack_num*  while_end_label_stack){
	bc_stack* tmp = (bc_stack*)malloc(sizeof(bc_stack));
	tmp->break_stack = break_stack;
	tmp->continue_stack = continue_stack;
	tmp->num_of_breaks_stack = num_of_breaks_stack;
	tmp->num_of_continues_stack = num_of_continues_stack;
	tmp->for_start_stack = for_start_stack;
	tmp->while_end_label_stack = while_end_label_stack;
	tmp->next = head;
	return tmp;
}

bc_stack* pop_bc_stack(bc_stack* head){
	assert(head);
	return head->next;

}


expr_list_list* pop_expr_list(expr_list_list* head ){
	assert(head);
	head = head->next;
	return head;
}


num_list* make_list( unsigned QuadNo){
	num_list* new_node;
	new_node = (num_list*)malloc(sizeof(num_list));
	new_node->num = QuadNo;
	new_node->next = NULL;
	return new_node;
}

num_list* merge_list( num_list* list1,num_list* list2){
	num_list* tmp = list1;

	if(list1 == NULL && list2 == NULL)
		return NULL;
	if(list2 == NULL)
		return list1;
	if(list1 == NULL)
		return list2;

	while(tmp->next!= NULL)
		tmp = tmp->next;

	tmp->next = list2;
	return list1;
}


num_list* patch_list_label(num_list* list , unsigned label){
	num_list* tmp = list;
	
	if(list == NULL)
		return NULL;
	while(tmp!=NULL)
	{
		quads[tmp->num].result = new_jump_expr(label);
		tmp=tmp->next;
	}
	return NULL;
}

void print_list(num_list* list){
	num_list* tmp = list;
	while(tmp!=NULL)
	{
		printf("%d ",tmp->num);
		tmp = tmp->next;
	}
	printf("\n");
}



/**********************************  PRINT FUNCTION  ************************************/

#define PRINT_EXPR(EXPR)												\
	if (EXPR == NULL) fprintf(quads_fp, "(empty)");									\
	else if (EXPR->type == TYPE_ERROR) fprintf(quads_fp, "Error");							\
	else if (EXPR->type == TYPE_VAR || EXPR->type == TYPE_PROGRAM_FUNCTION || EXPR->type == TYPE_LIBRARY_FUNCTION   \
			|| EXPR->type == TYPE_TEMP_VAR									\
			|| EXPR->type == TYPE_TABLE_ELEMENT || EXPR->type == TYPE_NEWTABLE ) 				\
	{														\
		if(EXPR->expr_value.symbol != NULL)									\
		{													\
			if(EXPR->type!=TYPE_TABLE_ELEMENT)								\
			{												\
			if(EXPR->expr_value.symbol->types.var != NULL)							\
				fprintf(quads_fp, "%s", EXPR->expr_value.symbol->types.var->name);			\
			else if(EXPR->expr_value.symbol->types.fun != NULL)						\
				fprintf(quads_fp, "%s", EXPR->expr_value.symbol->types.fun->name);			\
			}												\
			else if(EXPR->type==TYPE_TABLE_ELEMENT)								\
			{												\
				if(EXPR->index!=NULL)								        \
					fprintf(quads_fp, "%s", EXPR->expr_value.symbol->types.var->name);		\
				   else											\
					fprintf(quads_fp, "%s", EXPR->expr_value.str_value);				\
			}												\
			else												\
				fprintf(syntax_fp, "Symbol == NULL");							\
		}													\
	}														\
	else if (EXPR->type == TYPE_INT) fprintf(quads_fp, "%d", EXPR->expr_value.i_value);				\
	else if (EXPR->type == TYPE_FLOAT) fprintf(quads_fp, "%f", EXPR->expr_value.f_value);				\
	else if (EXPR->type == TYPE_STRING) fprintf(quads_fp, "\"%s\"", EXPR->expr_value.str_value);			\
	else if (EXPR->type == TYPE_LABEL) fprintf(quads_fp, "%d", EXPR->expr_value.label);				\
	else if (EXPR->type == TYPE_BOOL) 										\
	{														\
		if(EXPR->expr_value.bool_value == B_TRUE)								\
			fprintf(quads_fp, "TRUE");									\
		else if(EXPR->expr_value.bool_value == B_FALSE)								\
			fprintf(quads_fp, "FALSE");									\
	}														\
	else if (EXPR->type == TYPE_NIL) fprintf(quads_fp, "NIL");							\
	else {fprintf(stderr,"expr type: %d",EXPR->type);assert(!"Unknown type in expression. Programmer's error");}

void print_quad (quad* q)
{
	const char* operator;
	if(q == NULL)
	{fprintf(syntax_fp,"q == NULL\n");return;}
	fprintf(quads_fp,"%d:",q->label);
	switch (q->op) {
		case OP_ASSIGN: operator = "ASSIGN"; break;
		case OP_ADD: operator = "ADD"; break;
		case OP_SUB: operator = "SUB"; break;
		case OP_MUL: operator = "MUL"; break;
		case OP_DIV: operator = "DIV"; break;
		case OP_UMINUS: operator = "UMINUS"; break;
		case OP_MOD: operator = "MOD"; break;
		case OP_AND: operator = "AND"; break;
		case OP_OR: operator = "OR"; break;
		case OP_NOT: operator = "NOT"; break;
		case OP_IF_EQ: operator = "IF_EQ"; break;
		case OP_IF_NOT_EQ: operator = "IF_NOT_EQ"; break;
		case OP_IF_LESS_EQ: operator = "IF_LESS_EQ"; break;
		case OP_IF_GREATER_EQ: operator = "IF_GREATER_EQ"; break;
		case OP_IF_LESS: operator = "IF_LESS"; break;
		case OP_IF_GREATER: operator = "IF_GREATER"; break;
		case OP_CALL: operator = "CALL"; break;
		case OP_PARAM: operator = "PARAM"; break;
		case OP_RET: operator = "RET"; break;
		case OP_GET_RET_VAL: operator = "GET_RET_VAL"; break;
		case OP_FUNC_START: operator = "FUNC_START"; break;
		case OP_FUNC_END: operator = "FUNC_END"; break;
		case OP_TABLE_CREATE: operator = "TABLE_CREATE"; break;
		case OP_TABLE_GET_ELEM: operator = "TABLE_GET_ELEM"; break;
		case OP_TABLE_SET_ELEM: operator = "TABLE_SET_ELEM"; break;
		case OP_JUMP: operator = "JUMP"; break;

		default: operator = "UNKNOWN"; break;
	}

	if(q == NULL)
		return;


	fprintf(quads_fp, "%s ", operator);
	PRINT_EXPR(q->result);
	fprintf(quads_fp, ", ");
	PRINT_EXPR(q->arg1);
	fprintf(quads_fp, ", ");
	PRINT_EXPR(q->arg2);
	fprintf(quads_fp, "\n");
	fflush(stdout);
}
#undef PRINT_EXPR


void printQuads(void)
{
	int i;

	for(i=0;i<currQuad;i++)
		print_quad (&quads[i]);
}

/*************************** GENERATORS ********************************/

extern void generate_ADD(quad *p);
extern void generate_SUB(quad *p);
extern void generate_MUL(quad *p);
extern void generate_DIV(quad *p);
extern void generate_MOD(quad *p);
extern void generate_NEWTABLE (quad *p);
extern void generate_TABLEGETELEM (quad *p);
extern void generate_TABLESETELEM (quad *p);
extern void generate_ASSIGN (quad *p);
extern void generate_NOP ();

extern void generate_JUMP (quad* q);
extern void generate_IF_EQ (quad* q);
extern void generate_IF_NOTEQ(quad* q);
extern void generate_IF_GREATER (quad* q);
extern void generate_IF_GREATEREQ(quad* q);
extern void generate_IF_LESS (quad* q);
extern void generate_IF_LESSEQ (quad* q);

extern void generate_PARAM(quad* q);
extern void generate_CALL(quad* q);
extern void generate_GETRETVAL(quad* q);
extern void generate_FUNCSTART(quad* q);
extern void generate_FUNCEND(quad* q);
extern void generate_RETURN(quad* q);

extern void generate_NOT(quad* q);
extern void generate_OR(quad* q);
extern void generate_AND (quad* q);
extern void generate_UMINUS (quad* q);

extern void patch_incomplete_jumps();

typedef void (*generator_func_t)(quad*);

generator_func_t generators[] = {
	generate_ASSIGN      /*0*/,
	generate_ADD         /*1*/,
	generate_SUB         /*2*/,
	generate_MUL         /*3*/,
	generate_DIV         /*4*/,
	generate_MOD         /*5*/,
	generate_UMINUS      /*6*/,
	generate_AND         /*7*/,
	generate_OR          /*8*/,
	generate_NOT         /*9*/,
	generate_IF_EQ       /*10*/,
	generate_IF_NOTEQ    /*11*/,
	generate_IF_LESSEQ   /*12*/,
	generate_IF_GREATEREQ/*13*/,
	generate_IF_LESS     /*14*/,
	generate_IF_GREATER  /*15*/,
	generate_CALL        /*16*/,
	generate_PARAM       /*17*/,
	generate_RETURN      /*18*/,
	generate_GETRETVAL   /*19*/,
	generate_FUNCSTART   /*20*/,
	generate_FUNCEND     /*21*/,
	generate_NEWTABLE    /*22*/,
	generate_TABLEGETELEM/*23*/,
	generate_TABLESETELEM/*24*/,
	generate_JUMP        /*25*/,
};


void generate_t (void){
	unsigned i;
	for ( i = 0;i < currQuad;++i)//printf("%d:label:%d line:%d taddress:%d\n",i,quads[i].label,quads[i].line,quads[i].taddress);
		(*generators[quads[i].op])(quads+i);
	patch_incomplete_jumps();
}
