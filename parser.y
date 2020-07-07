/*
 * gkollias kotsiop
 * 
 */

%{
	#include <stdio.h>
	#include "definitions.h"
	#include "target.h"
	
	int yyerror (char* yaccProvidedMessage);
	int yylex (void);

	hashRecord* lookup_record = NULL;
	expression* tmp_expr = NULL;
	stack_num* f_scopes_stack = NULL;
	stack_num* if_label_stack = NULL;
	stack_num* else_label_stack = NULL;
	stack_num* has_else_stack = NULL;
	stack_num* while_start_label_stack=NULL;
	stack_num* while_end_label_stack=NULL;
	stack_num* for_expr_start_stack=NULL;
	stack_num* for_start_stack=NULL;
	stack_num* for_elist2_stack=NULL;
	stack_num* break_stack=NULL;
	stack_num* continue_stack=NULL;
	stack_num* num_of_breaks_stack=NULL;
	stack_num* num_of_continues_stack=NULL;
	stack_num* scope_offset_stack=NULL;

	stack_num* is_in_call_stack = NULL;

	int i,j,flag=0,flag2=0 , else_flag = 0 , else_Jlabel = 0 , tmp_quad = 0 ;
	int table_flag = -1 , object_flag = -1,call_flag = -1;
	int found_flag = -1;//0 for var 1 for function -1 for nothing
	char* string1;
	bool_t functor = B_FALSE;

	int id_empty = 0;
	int anonymous_ctr = 0;
	int in_function = 0;
	int function_scope = -1;
	int current_scope = 0;
	bool_t error_flag = B_FALSE;

	arglist* args = NULL; 
	expr_list* express=NULL;
	expr_list* indexed_elem=NULL;
	expr_list* indexed_value=NULL;
	expr_list* call_elist_stack = NULL;
	expr_list* function_name_stack = NULL;
	expr_list_list* elist_table_stack = NULL;
	expr_list_list* indexed_table_stack = NULL;
	expr_list_list* values_table_stack = NULL;
	expr_list_list* elist_param_stack = NULL;

	bc_stack* function_loop_stack = NULL;

	FILE* syntax_fp;
	FILE* symbol_fp;
	FILE* quads_fp;
	FILE* instructions_fp;
	FILE* instr_bin_fp;

	extern int          yylineno;
	extern char*        yytext;
	extern FILE*        yyin;
	extern scope*       scope_head;
	extern hashRecord*  print_head;
	//extern quad*        quads;
	extern unsigned int currQuad;

	
%}

%start program

%union {
	char*		stringValue;
	int		intValue;
	double		realValue;
	struct expression*	exprValue;
}

%token <stringValue> ID STRING FUNCTION CONTINUE BREAK NOT OR AND IF ELSE WHILE FOR LOCAL GLOBAL NIL RETURN QMARK TRUE FALSE DOT COMMA LHOOK RHOOK UDDOT RBRACKT LBRACKT LPAR RPAR SET DIFF LESS GREATER LEQUAL GEQUAL PLUS MINUS PP MM
%token <intValue> INTEGER
%token <realValue> FLOAT
%token <exprValue>  LINECOM COMMENT

%type <exprValue> expr stmt term lvalue primary call objectdef constant assignexpr retexpr funprefix func_id funcdef member objects
%type <exprValue> indexed elist
/* PRIORITY */

%right SET
%left  OR
%left AND
%nonassoc EQUAL DIFF
%nonassoc GREATER GEQUAL LESS LEQUAL
%left PLUS MINUS
%left MUL DIV MOD
%right NOT PP MM UMINUS
%left DOT
%left LBRACKT RBRACKT
%left LPAR RPAR

%expect 1
%destructor { free($$); } ID
%%

program:  stmts
       | {fprintf(syntax_fp,"empty\n");}/*empty*/
       ;


stmts:	stmt 
     | stmt stmts
     ;

stmt:  expr QMARK //{reset_temp();}
| ifstmt {fprintf(syntax_fp,"found if\n");}
| whilestmt {fprintf(syntax_fp,"found while\n");}
| forstmt {fprintf(syntax_fp,"found for\n");}
| returnstmt {fprintf(syntax_fp,"found return\n");}
| BREAK QMARK 
{
	fprintf(syntax_fp,"found break\n");
	if(for_start_stack != NULL || while_end_label_stack != NULL)
	{
		break_stack = push(break_stack,currQuad);
		num_of_breaks_stack->num++;
		currQuad++;
	}
	else
		fprintf(stderr,"Line %d:Error!Break is only allowed inside loops.\n",yylineno);
}
| CONTINUE QMARK 
{
	fprintf(syntax_fp,"found continue\n");
	if(for_start_stack != NULL || while_end_label_stack != NULL)
	{
		continue_stack = push(continue_stack,currQuad);
		num_of_continues_stack->num++;
		currQuad++;
	}
	else
		fprintf(stderr,"Line %d:Error!Continue is only allowed inside loops.\n",yylineno);
}
| block {fprintf(syntax_fp,"found block\n");}
| funcdef {$$ = $1;fprintf(syntax_fp,"found function definition\n");}
| QMARK {fprintf(syntax_fp,"found qmark\n");}
| LINECOM {fprintf(syntax_fp,"found single line comment\n");}
| COMMENT {fprintf(syntax_fp,"found multi line comment\n");}
| error {yyclearin;}
;

assignexpr: lvalue SET expr 
	  {
		expression* result;
		expression* tmp;
		
		
		result = new_expr_expr($1);
	
		if (result->type == TYPE_TABLE_ELEMENT)
		{
			emit( OP_TABLE_SET_ELEM,new_expr_expr($3),result,result->index,currQuad,yylineno);
			$$ = emit_if_table_item (result,currQuad,yylineno);
		}

		else
		{	
	  		result = new_expr_expr($1);
			tmp = new_expr_symbol(new_temp());
			emit(OP_ASSIGN , result , new_expr_expr($3) , NULL ,currQuad,yylineno);
			emit(OP_ASSIGN , tmp , result , NULL , currQuad,yylineno);
			$$ = tmp;
			
		}	
		fprintf(syntax_fp,"=");
	}
	  ;


term: LPAR expr RPAR {$$ = $2;}
| MINUS expr %prec UMINUS 
{
	expression* temp = new_expr_symbol(new_temp());
	if($2->type == TYPE_INT || $2->type == TYPE_FLOAT || $2->type == TYPE_VAR)
		emit(OP_UMINUS, temp , new_expr_expr($2) , NULL,currQuad,yylineno);
	else
		fprintf(stderr,"Line %d:Error.Uminus is used only in numeric types.\n",yylineno);
	$$ = temp;
}
| NOT expr
{
	expression* temp = new_expr_symbol(new_temp());
	emit(OP_NOT, temp , new_expr_expr($2) , NULL,currQuad,yylineno);

	$$ = temp;
}
| PP lvalue 
{
	if ($2->type == TYPE_TABLE_ELEMENT) 
	{
		$$ = emit_if_table_item($2 ,currQuad,yylineno);
		emit(OP_ADD,$$, $$,new_const_int_expr(1),currQuad,yylineno);
		emit( OP_TABLE_SET_ELEM, $$,$2, $2->index,currQuad,yylineno);
	}
	else
	{
		expression* temp = new_expr_symbol(new_temp());
		emit(OP_ADD, new_expr_expr($2), new_expr_expr($2), new_const_int_expr(1) ,currQuad,yylineno);
		emit(OP_ASSIGN,temp, new_expr_expr($2), NULL,currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
}
| lvalue PP
{
	expression* temp;
	if ($1->type == TYPE_TABLE_ELEMENT) 
	{
		$$ = emit_if_table_item($1 ,currQuad,yylineno);
		temp = new_expr_symbol(new_temp());
		emit(OP_ASSIGN, temp , $$, NULL,currQuad,yylineno);
		emit(OP_ADD,$$, $$,new_const_int_expr(1),currQuad,yylineno);
		emit( OP_TABLE_SET_ELEM, $$,$1, $1->index,currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
	else
	{	
		temp = new_expr_symbol(new_temp());
		emit(OP_ASSIGN,temp, new_expr_expr($1), NULL,currQuad,yylineno);
		emit(OP_ADD, new_expr_expr($1), new_expr_expr($1), new_const_int_expr(1),currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
}
| MM lvalue
{
	if ($2->type == TYPE_TABLE_ELEMENT) 
	{
		$$ = emit_if_table_item($2 ,currQuad,yylineno);
		emit(OP_SUB,$$, $$,new_const_int_expr(1),currQuad,yylineno);
		emit( OP_TABLE_SET_ELEM, $$,$2, $2->index,currQuad,yylineno);
	}
	else
	{
		expression* temp = new_expr_symbol(new_temp());
		emit(OP_SUB, new_expr_expr($2), new_expr_expr($2), new_const_int_expr(1) ,currQuad,yylineno);
		emit(OP_ASSIGN,temp, new_expr_expr($2), NULL,currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
}
| lvalue MM
{
	expression* temp;
	if ($1->type == TYPE_TABLE_ELEMENT) 
	{
		$$ = emit_if_table_item($1 ,currQuad,yylineno);
		temp = new_expr_symbol(new_temp());
		emit(OP_ASSIGN, temp , $$, NULL,currQuad,yylineno);
		emit(OP_SUB,$$, $$,new_const_int_expr(1),currQuad,yylineno);
		emit( OP_TABLE_SET_ELEM, $$,$1, $1->index,currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
	else
	{	
		temp = new_expr_symbol(new_temp());
		emit(OP_ASSIGN,temp, new_expr_expr($1), NULL,currQuad,yylineno);
		emit(OP_SUB, new_expr_expr($1), new_expr_expr($1), new_const_int_expr(1),currQuad,yylineno);
		$$ = new_expr_expr(temp);
	}
}
| primary { $$ = emit_if_table_item($1,currQuad,yylineno);}
;

expr: assignexpr {fprintf(syntax_fp,"found assign expression\n");}
    | expr PLUS expr 
    {
	if(check_types($1->type,$3->type,OP_ADD)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_ADD)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in arithmetic expression!\n",yylineno);
	}
    	expression* result = new_expr_symbol(new_temp());
	emit(OP_ADD, result, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);//change the label now is zero
	$$ = result;
	fprintf(syntax_fp,"found expr + expr \n");
    }
    | expr MINUS expr 
    {
	if(check_types($1->type,$3->type,OP_SUB)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_SUB)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in arithmetic expression!\n",yylineno);
	}
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_SUB, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);//change the label now is zero
	$$ = exp;
    	fprintf(syntax_fp,"found expr - expr \n");
    }
    | expr MUL expr 
    {
	if(check_types($1->type,$3->type,OP_MUL)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_MUL)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in arithmetic expression!\n",yylineno);
	}
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_MUL, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);
	$$ = exp;
    	fprintf(syntax_fp,"found expr * expr \n");
    }
    | expr DIV expr 
    {
	if(check_types($1->type,$3->type,OP_DIV)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_DIV)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in arithmetic expression!\n",yylineno);
	}
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_DIV, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);//change the label now is zero
	$$ = exp;
    	fprintf(syntax_fp,"found expr / expr \n");
    }
    | expr MOD expr 
    {
	if(check_types($1->type,$3->type,OP_MOD)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_MOD)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in arithmetic expression!\n",yylineno);
	}
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_MOD, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);
	$$ = exp;
    	fprintf(syntax_fp,"found expr mod expr \n");
    }
    | expr GREATER expr 
    { 
	if(check_types($1->type,$3->type,OP_IF_GREATER)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_IF_GREATER)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in logical expression!\n",yylineno);
	}
	$$ = emit_relop(OP_IF_GREATER,yylineno,$1,$3);
    	fprintf(syntax_fp,"found expr > expr \n");
    }
    | expr GEQUAL expr 
    {

	if(check_types($1->type,$3->type,OP_IF_GREATER_EQ)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_IF_GREATER_EQ)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in logical expression!\n",yylineno);
	}
	$$ = emit_relop(OP_IF_GREATER_EQ,yylineno,$1,$3);;
    	fprintf(syntax_fp,"found expr >= expr \n");
    }
    | expr LESS expr 
    {
	if(check_types($1->type,$3->type,OP_IF_LESS)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_IF_LESS)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in logical expression!\n",yylineno);
	}
    	$$ = emit_relop(OP_IF_LESS,yylineno,$1,$3);
    	fprintf(syntax_fp,"found expr < expr \n");
    }
    | expr LEQUAL expr 
    {
	if(check_types($1->type,$3->type,OP_IF_LESS_EQ)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
	if(check_types($1->type,$3->type,OP_IF_LESS_EQ)==TYPE_BOOL)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Boolean type in logical expression!\n",yylineno);
	}
    	$$ = emit_relop(OP_IF_LESS_EQ,yylineno,$1,$3);
        fprintf(syntax_fp,"found expr <= expr \n");
    }
    | expr EQUAL expr 
    {
	if(check_types($1->type,$3->type,OP_IF_EQ)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
    	$$ = emit_relop(OP_IF_EQ,yylineno,$1,$3);
        fprintf(syntax_fp,"found expr == expr \n");
    }
    | expr DIFF expr 
    {
	if(check_types($1->type,$3->type,OP_IF_NOT_EQ)==TYPE_ERROR)
	{
		/******************/
		error_flag = B_TRUE;
		/*****************/
		fprintf(stderr,"Line %d:Error.Type Mismatch!\n",yylineno);
	}
    	$$ = emit_relop(OP_IF_NOT_EQ,yylineno,$1,$3);
    	fprintf(syntax_fp,"found expr != expr \n");
    }
    | expr AND expr 
    {
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_AND, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);
	$$ = exp;
    	fprintf(syntax_fp,"found expr && expr \n");
    }
    | expr OR expr 
    {
    	expression* exp = new_expr_symbol(new_temp());
	emit(OP_OR, exp, new_expr_expr($1), new_expr_expr($3),currQuad,yylineno);
	$$ = exp;
    	fprintf(syntax_fp,"found expr || expr \n");
    }
    | term 
;


primary: lvalue {$$ = $1;}
| call {$$ = $1;}
| objectdef{$$ = $1;}
| LPAR funcdef RPAR 
{
	$$ = new_expr_void();
	$$->type = TYPE_PROGRAM_FUNCTION;
	$$->expr_value.symbol = $2->expr_value.symbol;
}
| constant {$$ = $1;}
;
lvalue: ID 
      /*look up same scope && legal scopes*/
	{
		string1 = strdup($1);
		i = current_scope;
		$$ = new_expr_void();
		if(in_function == 1)
		{
			
			i = function_scope;
			/*look at current scope and previous(arguments)*/
			j = current_scope;
			while(f_scopes_stack != NULL && j > f_scopes_stack->num)
			{
			  lookup_record = lookScope(string1,j);
			  fprintf(syntax_fp,"********************I looked in %d ***********************\n",j);
			  if(lookup_record != NULL)
			  {
			       	if(lookup_record->type == VAR_S)
				{
					found_flag = 0;
					$$->expr_value.symbol = lookup_record;
					$$->type = TYPE_VAR;
				
					fprintf(syntax_fp,"Reference to meta:%s at scope %d\n",yytext,j);
					flag2 = 1;
				}
				else if(lookup_record->type == PROGRAM_FUNCTION_S || lookup_record->type == LIBRARY_FUNCTION_S)
				{
					found_flag = 1;
					if(lookup_record->type == PROGRAM_FUNCTION_S)
					{
						//$$->type = TYPE_ERROR;
						$$->type = TYPE_PROGRAM_FUNCTION;
						$$->expr_value.symbol = lookup_record;
					///	fprintf(stderr,"Redefinition Error. Program Function exists with id:%s at scope %d\n",yytext,current_scope);
					}
					if(lookup_record->type == LIBRARY_FUNCTION_S)
					{
						//$$->type = TYPE_ERROR;
						$$->type = TYPE_LIBRARY_FUNCTION;
						$$->expr_value.symbol = lookup_record;
					///fprintf(stderr,"Redefinition Error. Library Function exists with id:%s at scope %d\n",yytext,current_scope);
					}
					flag2 = 1;
				}
			  }
			  
			j--;
			}
			if(current_scope>0 && flag2 != 1)
			{	
				lookup_record=lookScope(string1,current_scope-1);
				if(lookup_record != NULL && flag2 != 1)
				{
				
					if(lookup_record->type == VAR_S && flag2 != 1)
					{
						found_flag = 0;
						$$->expr_value.symbol = lookup_record;
						$$->type = TYPE_VAR;
						fprintf(syntax_fp,"Reference to meta:%s at scope %d\n",yytext,current_scope);
						flag2 = 1;
					}
				}
			}
			for(j = current_scope-2;j>function_scope && flag2 != 1;j--)
			{	
				lookup_record = lookScope(string1,j);
				if(lookup_record != NULL)
				{
					if(lookup_record->type == PROGRAM_FUNCTION_S || lookup_record->type == LIBRARY_FUNCTION_S)
					{
						/*if(call_flag == 1)
						{
							printf("Function Call\n");
							if(lookup_record->type == PROGRAM_FUNCTION_S)
								$$->type = TYPE_PROGRAM_FUNCTION;
							else if(lookup_record->type == LIBRARY_FUNCTION_S)
								$$->type = TYPE_LIBRARY_FUNCTION;
							$$->expr_value.symbol = lookup_record;
							flag = 1;
							break;
						}*/
						found_flag = 1;
						//fprintf(stderr,"Illegal id:%s .A function with this name already exists\n",string1);
						if(lookup_record->type == PROGRAM_FUNCTION_S)
							$$->type = TYPE_PROGRAM_FUNCTION;
						if(lookup_record->type == LIBRARY_FUNCTION_S)
							$$->type = TYPE_LIBRARY_FUNCTION;
						//$$->type = TYPE_ERROR;
						$$->expr_value.symbol = lookup_record;
						flag = 1;
						break;
					}
				}
			}
		}/*end for in function*/
				
		
		
		while(i != -1 && flag2!= 1 ) 
		{
			lookup_record = lookScope(string1,i);
			if(lookup_record != NULL)
			{
				if(lookup_record->type == VAR_S)
				{
					found_flag = 0;
					$$->expr_value.symbol = lookup_record;
					$$->type = TYPE_VAR;
			
					fprintf(syntax_fp,"Reference to meta:%s\n",string1);
					flag = 1;
					break;
				}
				else if(lookup_record->type == PROGRAM_FUNCTION_S || lookup_record->type == LIBRARY_FUNCTION_S)
				{
					/*if(call_flag == 1)
					{
						printf("Function Call\n");
						if(lookup_record->type == PROGRAM_FUNCTION_S)
							$$->type = TYPE_PROGRAM_FUNCTION;
						else if(lookup_record->type == LIBRARY_FUNCTION_S)
							$$->type = TYPE_LIBRARY_FUNCTION;
						$$->expr_value.symbol = lookup_record;
						flag = 1;
						break;
					}*/
					//printf("Illegal id:%s .A function with this name already exists\n",string1);
					found_flag = 1;
					if(lookup_record->type == PROGRAM_FUNCTION_S)
						$$->type = TYPE_PROGRAM_FUNCTION;
					if(lookup_record->type == LIBRARY_FUNCTION_S)
						$$->type = TYPE_LIBRARY_FUNCTION;
					//$$->type = TYPE_ERROR;
					$$->expr_value.symbol = lookup_record;
					flag = 1;
					break;
				}
			}
			i--;
		}
		if(flag != 1 && flag2 != 1)
		{
			found_flag = -1;
			$$->type = TYPE_VAR;
			$$->expr_value.symbol = insertScopes(current_scope,
				             newRec(newVar(yytext),VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
				             ,string1);
					

			incCurrScopeOffset();
		}
		flag = 0;
		flag2 = 0;
	}
| LOCAL ID 
/*lookup same scope.Else insert hash table at this scope.*/
	{
		string1 = strdup((char*)$2);
		fprintf(syntax_fp,"found local id value:%s\n",string1);

		$$ = new_expr_void() ;
		if(current_scope == 0)
		{
			$$->type = TYPE_ERROR ; 
			$$->expr_value.bool_value = B_FALSE; 
			fprintf(stderr,"Line %d:Error! Local is used only for local range.\n",yylineno);
			/******************/
			error_flag = B_TRUE;
			/*****************/
		}
		else if(($$->expr_value.symbol = lookScope(yytext,current_scope)) != NULL)
		{
			$$->type = TYPE_VAR ;
		}
		else
		{
			$$->type = TYPE_VAR ;
			$$->expr_value.symbol = insertScopes(current_scope,
			             newRec(newVar(yytext),VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
			             ,yytext);
			
			incCurrScopeOffset();
		}
	}
		
| GLOBAL ID 
	{
	string1 = strdup((char*)$2);
	fprintf(syntax_fp,"found global id value:%s\n",string1);

	$$ = new_expr_void() ;
	$$->expr_value.symbol = lookScope(yytext,0);
	if($$->expr_value.symbol)
	{
		$$->type = TYPE_VAR ;
		
	}
	else
	{
		fprintf(stderr,"Line %d:Error! Global variable \"%s\" not found.\n",yylineno,yytext);
		$$->type = TYPE_ERROR ; 
		$$->expr_value.bool_value = B_FALSE; 
		/******************/
		error_flag = B_TRUE;
		/*****************/
	}
	}
| member {fprintf(syntax_fp,"found member\n");}
;




member: lvalue DOT ID 
{
	fprintf(syntax_fp,"lvalue.ID\n");
	$1 = emit_if_table_item($1 , currQuad , yylineno); // Emit code if r-value use of table item.
	$$ = new_expr_void (); // Make a new expression.
	$$->type=TYPE_TABLE_ELEMENT;
       	$$->expr_value.symbol = $1->expr_value.symbol;
       	$$->index = new_const_string_expr($3); // Const string index.
}
| lvalue LBRACKT expr RBRACKT
{
	if(!functor)
	{
		if($3->type == TYPE_STRING)
		{
			if(!strcmp($3->expr_value.str_value,"()"))
				functor = B_TRUE;
		}
	}
	fprintf(syntax_fp,"lvalue[expr]\n");
	$1 = emit_if_table_item($1 , currQuad , yylineno); // Emit code if r-value use of table item.
        $$ = new_expr_void (); // Make a new expression.
	$$->type=TYPE_TABLE_ELEMENT;
        $$->expr_value.symbol = $1->expr_value.symbol;
	$$->index = new_expr_expr($3);	
}
| call DOT ID 
{
	string1 = (char*)$3;
	fprintf(syntax_fp,"call.ID\n");
	$1 = emit_if_table_item($1 , currQuad , yylineno); // Emit code if r-value use of table item.
	$$ = new_expr_void (); // Make a new expression.
	$$->type=TYPE_TABLE_ELEMENT;
       	$$->expr_value.symbol = $1->expr_value.symbol;
       	$$->index = new_const_string_expr($3); // Const string index.
}
| call LBRACKT expr RBRACKT
{
	fprintf(syntax_fp,"call[expr]\n");
	$1 = emit_if_table_item($1 , currQuad , yylineno); // Emit code if r-value use of table item.
        $$ = new_expr_void (); // Make a new expression.
	$$->type=TYPE_TABLE_ELEMENT;
        $$->expr_value.symbol = $1->expr_value.symbol;
	$$->index = new_expr_expr($3);	
}
;


call: call  
    {
    //call_flag = 1;
    is_in_call_stack = push(is_in_call_stack,1);
    //if(elist_param_stack!=NULL)
	elist_param_stack=push_expr_list(elist_param_stack,NULL);
    }
    LPAR elist RPAR 
{
	expression* result;
	expression* func = $1;

	while(elist_param_stack->exp!=NULL)
	{
		emit( OP_PARAM,elist_param_stack->exp->exp,NULL ,NULL,currQuad,yylineno);
		elist_param_stack->exp = pop_expr(elist_param_stack->exp);
	}
	emit( OP_CALL,func,NULL ,NULL,currQuad,yylineno);
	result = new_expr_symbol(new_temp());
	emit( OP_GET_RET_VAL,result,NULL ,NULL,currQuad,yylineno);
	$$ = result;

	elist_param_stack = pop_expr_list(elist_param_stack);
	/*************************remove it on 24-6-2007**********************************/
	//if(elist_param_stack != NULL)
	//	elist_param_stack->exp = push_expr(elist_param_stack->exp,result);
	//$$->is_true = B_TRUE;
	fprintf(syntax_fp,"call(elist)\n");
	//call_flag = 0;
	is_in_call_stack = pop(is_in_call_stack);
	call_elist_stack = NULL;
}
| lvalue LPAR 
{
	//call_flag = 1;
	is_in_call_stack = push(is_in_call_stack,1);
	//if(elist_param_stack!=NULL)
		elist_param_stack=push_expr_list(elist_param_stack,NULL);
} 
elist RPAR 
{
	expression* result;
	expression* func = emit_if_table_item($1 , currQuad , yylineno);

	if(found_flag == 1)
	{
		fprintf(syntax_fp,"Function Call\n");
		if($1->expr_value.symbol->type == PROGRAM_FUNCTION_S)
			$1->type = TYPE_PROGRAM_FUNCTION;
		else if($1->expr_value.symbol->type == LIBRARY_FUNCTION_S)
			$1->type = TYPE_LIBRARY_FUNCTION;
	}
	else if(found_flag == 0 || found_flag == -1)
	{
		fprintf(syntax_fp,"Variable Call\n");
		if($1->expr_value.symbol->type == PROGRAM_FUNCTION_S)
			$1->type = TYPE_PROGRAM_FUNCTION;
		else if($1->expr_value.symbol->type == LIBRARY_FUNCTION_S)
			$1->type = TYPE_LIBRARY_FUNCTION;
	}
	while(elist_param_stack->exp!=NULL)
	{
		emit( OP_PARAM,elist_param_stack->exp->exp,NULL ,NULL,currQuad,yylineno);
		elist_param_stack->exp = pop_expr(elist_param_stack->exp);
	}
	emit( OP_CALL,func,NULL ,NULL,currQuad,yylineno);
	result = new_expr_symbol(new_temp());
	emit( OP_GET_RET_VAL,result,NULL ,NULL,currQuad,yylineno);
	$$ = result;
	elist_param_stack = pop_expr_list(elist_param_stack);
	/*************************remove it on 24-6-2007**********************************/
	//if(elist_param_stack != NULL)
	//	elist_param_stack->exp = push_expr(elist_param_stack->exp,result);
	fprintf(syntax_fp,"lvalue(elist)\n");
	//call_flag = 0;
	is_in_call_stack = pop(is_in_call_stack);
	call_elist_stack = NULL;
}
| LPAR funcdef RPAR LPAR 
{
	//call_flag = 1;
	is_in_call_stack = push(is_in_call_stack,1);
	//if(elist_param_stack!=NULL)
		elist_param_stack=push_expr_list(elist_param_stack,NULL);
}
elist RPAR 
{
	expression* result;
	expression* func = new_expr_void();
	func->type = TYPE_PROGRAM_FUNCTION;
	func->expr_value.symbol = $2->expr_value.symbol;
	while(elist_param_stack->exp!=NULL)
	{
		emit( OP_PARAM,elist_param_stack->exp->exp,NULL ,NULL,currQuad,yylineno);
		elist_param_stack->exp = pop_expr(elist_param_stack->exp);
	}
	emit( OP_CALL,func,NULL ,NULL,currQuad,yylineno);
	result = new_expr_symbol(new_temp());
	emit( OP_GET_RET_VAL,result,NULL ,NULL,currQuad,yylineno);
	$$ = result;
	elist_param_stack = pop_expr_list(elist_param_stack);
	/*************************remove it on 24-6-2007**********************************/
	//if(elist_param_stack != NULL)
	//	elist_param_stack->exp = push_expr(elist_param_stack->exp,result);
	fprintf(syntax_fp,"(funcdef)(elist)\n");
	//call_flag = 0;
	is_in_call_stack = pop(is_in_call_stack);
}
;

fun_param: COMMA expr
	 {
	 	if(elist_table_stack!=NULL)
			elist_table_stack->exp=insertExprList(elist_table_stack->exp,newItem($2));

		if(is_in_call_stack != NULL)//(call_flag == 1)
			elist_param_stack->exp = push_expr(elist_param_stack->exp , $2);

	 }
 	 fun_param/*{$$=$2;}*/ 
	 | /*empty*/
	 ;

elist: expr 
     {
     	if(elist_table_stack!=NULL)
		elist_table_stack->exp=insertExprList(elist_table_stack->exp,newItem($1));

	if(is_in_call_stack != NULL && elist_param_stack!=NULL)//call1
		elist_param_stack->exp = push_expr(elist_param_stack->exp , $1);
     }
     fun_param 
     | {fprintf(syntax_fp,"elist empty\n");}/*empty*/
     ;

objects: elist
       {
         object_flag=0;
	 $$ = $1;
	 fprintf(syntax_fp,"found elist\n");
       } 
       | indexed
       {
	 object_flag=1;
	 $$ = $1;
	 fprintf(syntax_fp,"found indexed\n");
       }
       ;

objectdef: LBRACKT 
	 {
		elist_table_stack=push_expr_list(elist_table_stack,express);
         	indexed_table_stack=push_expr_list(indexed_table_stack,indexed_elem);
         	values_table_stack=push_expr_list(values_table_stack,indexed_value);	
	 }
	 objects RBRACKT 
	 {
	   expression* exptmp;
	   int i;
	   expr_list* tmp=NULL;
	   expr_list* indeces=NULL;
	   expr_list* values=NULL;
	   
	   if(elist_table_stack!=NULL)
	   {
	   	tmp=elist_table_stack->exp;
	   	elist_table_stack=pop_expr_list(elist_table_stack);
	   }

           if(indexed_table_stack!=NULL)
	   {
	   	indeces=indexed_table_stack->exp;
	   	indexed_table_stack=pop_expr_list(indexed_table_stack);
	   }

           if(values_table_stack!=NULL)
	   {
	   	values=values_table_stack->exp;
	   	values_table_stack=pop_expr_list(values_table_stack);
	   }

	 
	   exptmp=new_expr_void();
	   exptmp->type=TYPE_NEWTABLE;
	   exptmp->expr_value.symbol=new_temp();
	   exptmp->is_true = B_TRUE;
	   emit(OP_TABLE_CREATE, exptmp, NULL , NULL , currQuad , yylineno);

	   if(object_flag==0)
	   {	
	   	for(i=0;tmp!=NULL;i++)
	   	{
	   		emit( OP_TABLE_SET_ELEM,tmp->exp,exptmp ,new_const_int_expr(i),currQuad,yylineno);
	   		tmp=tmp->next;
		}
	   }
	   else if(object_flag==1)
	   {
	   	for(i=0;values!=NULL||indeces!=NULL;i++)
	   	{
			if(values != NULL && indeces!=NULL)
			{
				emit( OP_TABLE_SET_ELEM,values->exp,exptmp ,indeces->exp,currQuad,yylineno);
	   			values=values->next;
				indeces=indeces->next;
			}
		}
	    }

	   object_flag = -1;
	   express = NULL;
	   values  = NULL;
	   indeces = NULL;

     	   $$=exptmp;
	   fprintf(syntax_fp,"found object definition\n");
	 }
	 ;

indexelements: COMMA indexedelem indexelements 
	     | /*empty*/
	     ;

indexed:  indexedelem indexelements {fprintf(syntax_fp,"found indexed elements\n");}
       ;

indexedelem: LHOOK expr
	   {
		if(indexed_table_stack!=NULL)
	   		indexed_table_stack->exp=insertExprList(indexed_table_stack->exp,newItem($2));

		if(!functor)
		{	
			if($2->type == TYPE_STRING)
			{
				if(!strcmp($2->expr_value.str_value,"()"))
					functor = B_TRUE;
			}
		}
	   }
	   UDDOT expr
	   {
	   	if(values_table_stack!=NULL)
	   		values_table_stack->exp=insertExprList(values_table_stack->exp,newItem($5));
	   }
	   RHOOK
	   ;

blockst:stmts 
       | /*empty*/
       ;


block: LHOOK 
     {
     	/*scope managment*/
     	current_scope++;
	insertNewScope(current_scope);
	fprintf(syntax_fp,"***scope = %d***\n",current_scope);
     }
     blockst 
     RHOOK
     { 
     	/*call of hide for scope*/
     	hide(current_scope);
	if(current_scope == function_scope)
		in_function = 0;
	current_scope--;
	if(current_scope == function_scope)
		in_function = 0;
	fprintf(syntax_fp,"***scope = %d***\n",current_scope);
	fprintf(syntax_fp,"found block\n");
     }
     ;

func_id: ID 
       {
		string1 = strdup($1);
		i = current_scope;
		$$ = new_expr_void();
		if(in_function == 1)
		{
			
			i = function_scope;
			/*look at current scope and previous(arguments)*/
			j = current_scope;
			while(f_scopes_stack != NULL && j > f_scopes_stack->num)
			{
			  lookup_record = lookScope(string1,j);
			  fprintf(syntax_fp,"********************I looked in %d ***********************\n",j);
			  if(lookup_record != NULL)
			  {
			       	if(lookup_record->type == VAR_S)
				{
					//found_flag = 0;
					//$$->expr_value.symbol = lookup_record;
					$$->type = TYPE_ERROR;
					/******************/
					error_flag = B_TRUE;
					/*****************/
					fprintf(stderr,"Line %d:Redefinition Error.Variable exists with id:%s at scope %d\n",yylineno,yytext,j);
					flag2 = 1;
				}
				else if(lookup_record->type == PROGRAM_FUNCTION_S || lookup_record->type == LIBRARY_FUNCTION_S)
				{
					//found_flag = 1;
					if(lookup_record->type == PROGRAM_FUNCTION_S)
					{
					   $$->type = TYPE_ERROR;
					   /******************/
					   error_flag = B_TRUE;
					   /*****************/
					   //$$->expr_value.symbol = lookup_record;
					   fprintf(stderr,"Line %d:Redefinition Error.Program Function exists with id:%s at scope %d\n",yylineno,yytext,j);
					}
					if(lookup_record->type == LIBRARY_FUNCTION_S)
					{	
					   $$->type = TYPE_ERROR;
					   /******************/
					   error_flag = B_TRUE;
					   /*****************/
					   //$$->expr_value.symbol = lookup_record;
					   fprintf(stderr,"Line %d:Redefinition Error. Library Function exists with id:%s at scope %d\n",yylineno,yytext,j);
					}
					flag2 = 1;
				}
			  }
			  
			j--;
			}
			if(current_scope>0 && flag2 != 1)
			{	
				lookup_record=lookScope(string1,current_scope-1);
				if(lookup_record != NULL && flag2 != 1)
				{
				
					if(lookup_record->type == VAR_S && flag2 != 1)
					{
					   //found_flag = 0;
					   //$$->expr_value.symbol = lookup_record;
					   $$->type = TYPE_ERROR;
					   /******************/
					   error_flag = B_TRUE;
			 		   /*****************/						
				       fprintf(stderr,"Line %d:Redefinition Error.Variable exists with id:%s at scope %d\n",yylineno,yytext,current_scope-1);
					   flag2 = 1;
					}
				}
			}
			
		}/*end for in function*/
				
		while(i != -1 && flag2!= 1 && in_function!=1) 
		{
			lookup_record = lookScope(string1,i);
			if(lookup_record != NULL)
			{
				if(lookup_record->type == VAR_S)
				{
					//found_flag = 0;
					//$$->expr_value.symbol = lookup_record;
					$$->type = TYPE_ERROR;
					/******************/
					error_flag = B_TRUE;
					/*****************/
					fprintf(stderr,"Line %d:Redefinition Error.Variable exists with id:%s at scope %d\n",yylineno,yytext,i);
					flag = 1;
					break;
				}
				else if(lookup_record->type == PROGRAM_FUNCTION_S || lookup_record->type == LIBRARY_FUNCTION_S)
				{
				
					fprintf(stderr,"Line %d:Redefinition Error. Function already exists with id:%s at scope %d\n",yylineno,yytext,i);
					//found_flag = 1;
					$$->type = TYPE_ERROR;
					/******************/
					error_flag = B_TRUE;
					/*****************/
					//$$->expr_value.symbol = lookup_record;
					flag = 1;
					break;
				}
			}
			i--;
		}
		if(flag != 1 && flag2 != 1)
		{
			//found_flag = -1;
			//$$ = new_expr_void();
             		$$->type = TYPE_STRING;
	     		$$->expr_value.str_value = strdup($1);
		}
		
		$$->type = TYPE_STRING;
	     	$$->expr_value.str_value = strdup($1);
		flag = 0;
		flag2 = 0;
       }
       | 
       {
        $$ = new_expr_void();
        $$->type = TYPE_STRING;
        char* tmp = (char*)malloc(400);
       	anonymous_ctr++;
	fprintf(syntax_fp,"function id empty\n"); 
	sprintf(tmp ,"Anonymous$%d",anonymous_ctr);
	$$->expr_value.str_value = strdup(tmp);
       }/*empty*/
       ;

funprefix: FUNCTION func_id
	 {
	 	$$ = new_expr_expr($2);
	 	f_scopes_stack = push(f_scopes_stack,current_scope);
        	if(in_function == 0)
        	{
	 		function_scope = current_scope;
			in_function = 1;
		}
		current_scope++;
		insertNewScope(current_scope);
		fprintf(syntax_fp,"***scope = %d ***\n",current_scope);

		scope_offset_stack = push(scope_offset_stack,currScopeOffset());
		enterScopeSpace();
		resetFormalArgsOffset();
	 }
	;

funcdef: funprefix 
       LPAR idlist RPAR 
       {
       		/******************Store the lists of continue and break when in function and restore them at the end*************************/
		function_loop_stack = push_bc_stack(function_loop_stack,
				break_stack,continue_stack,num_of_breaks_stack,num_of_continues_stack,
				for_start_stack,while_end_label_stack);
		break_stack            = NULL;
		continue_stack         = NULL;
		num_of_breaks_stack    = NULL;
		num_of_continues_stack = NULL;
		for_start_stack        = NULL;
		while_end_label_stack  = NULL;

       		tmp_expr = new_expr_void();
		tmp_expr->type = TYPE_PROGRAM_FUNCTION;
		tmp_expr->expr_value.symbol = insertScopes(current_scope - 1 ,
	             newRec(newFun($1->expr_value.str_value,args),PROGRAM_FUNCTION_S,current_scope - 1,yylineno,currScopeOffset(),currScopeSpace())
		     ,$1->expr_value.str_value);
		
		function_name_stack = push_expr(function_name_stack,tmp_expr);

		assert(tmp_expr->expr_value.symbol);
		assert(tmp_expr->expr_value.symbol->types.fun->name);

		tmp_expr->expr_value.symbol->types.fun->i_address = currQuad;
		emit(OP_FUNC_START, new_expr_expr(tmp_expr) , NULL , NULL,currQuad,yylineno);
		args = NULL;

		enterScopeSpace();
		resetFunctionLocalsOffset();
       }
       block
       {
       		$$ = new_expr_expr(function_name_stack->exp);
		assert($$);
		assert($$->expr_value.symbol->types.fun);
       		$$->expr_value.symbol->types.fun->total_locals = currScopeOffset();/*printf("@%d\n\n",currScopeOffset());*/
		$$->is_true = B_TRUE;
       		exitScopeSpace();
		exitScopeSpace();
		restoreCurrScopeOffset(scope_offset_stack->num);
		scope_offset_stack = pop(scope_offset_stack);	

		emit(OP_FUNC_END, function_name_stack->exp , NULL , NULL,currQuad,yylineno);
		function_name_stack = pop_expr(function_name_stack);
       		
		 /*call of hide for scope*/
       		hide(current_scope);
		current_scope--;
		if(current_scope == function_scope)
			in_function = 0;
		fprintf(syntax_fp,"***scope = %d***\n",current_scope);
		f_scopes_stack = pop(f_scopes_stack);

	       	fprintf(syntax_fp,"found function definition\n");
		/**********************  Restore the continue and break stacks**********************/
		break_stack = function_loop_stack->break_stack;
		continue_stack = function_loop_stack->continue_stack;
		num_of_breaks_stack = function_loop_stack->num_of_breaks_stack;
		num_of_continues_stack = function_loop_stack->num_of_continues_stack;
		for_start_stack        = function_loop_stack->for_start_stack;
		while_end_label_stack  = function_loop_stack->while_end_label_stack;
		function_loop_stack = pop_bc_stack(function_loop_stack);
       }
	;

constant: INTEGER 
	{	
		fprintf(syntax_fp,"found integer :%d\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_INT;
		$$->expr_value.i_value = $1;
		if($1 == 0)
			$$->is_true = B_FALSE;
		else
			$$->is_true = B_TRUE;
	}
	| FLOAT 
	{
		fprintf(syntax_fp,"found float :%lf\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_FLOAT;
		$$->expr_value.f_value = $1;
		if($1 == 0)
			$$->is_true = B_FALSE;
		else
			$$->is_true = B_TRUE;
	}
	| STRING 
	{	
		fprintf(syntax_fp,"found string:%s\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_STRING;
		$$->expr_value.str_value = strdup($1);
		if(!strcmp($1,""))
			$$->is_true = B_FALSE;
		else
			$$->is_true = B_TRUE;
	}
	| NIL 
	{
		fprintf(syntax_fp,"found nil:%s\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_NIL;
		$$->expr_value.bool_value = B_FALSE;
		$$->is_true = B_FALSE;
	}
	| TRUE 
	{
		fprintf(syntax_fp,"found true:%s\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_BOOL;
		$$->expr_value.bool_value = B_TRUE;
		$$->is_true = B_TRUE;
	
	}
	| FALSE 
	{
		fprintf(syntax_fp,"found false:%s\n",$1);
		$$ = new_expr_void();
		$$->type = TYPE_BOOL;
		$$->expr_value.bool_value = B_FALSE;
		$$->is_true = B_FALSE;
	
	}
     ;

ids: COMMA ID 
   {
   	args = insertArgList(args,newArg(newVar($2)));

   	if(lookScope(yytext,current_scope) != NULL)
   	{
		printf("Redefinition Error!\n");
		//exit(-1);
	}
   	else
   	{
   		insertScopes(current_scope,
			     newRec(newVar(yytext),VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
		     	     ,yytext);

		incCurrScopeOffset();
   	}
   } ids
   |/*empty*/
   ;

idlist: ID 
      {
	if(functor)
	{
		fprintf(syntax_fp,"param id :%s\n",$1);
      		insertScopes(current_scope,
	         	    newRec(newVar("this"),VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
	             	    ,"this");
		args = insertArgList(args,newArg(newVar("this")));
		incCurrScopeOffset();
		functor = B_FALSE;
	}
	if(lookScope(yytext,current_scope) != NULL)
   	{
		printf("Redefinition Error!\n");
		//exit(-1);
	}
	fprintf(syntax_fp,"param id :%s\n",$1);
      	insertScopes(current_scope,
	             newRec(newVar(yytext),VAR_S,current_scope,yylineno,currScopeOffset(),currScopeSpace())
	             ,yytext);
	args = insertArgList(args,newArg(newVar(yytext)));
	incCurrScopeOffset();
	} ids {fprintf(syntax_fp,"found id list\n");}
      | /*empty*/
      ;

elsestm: ELSE 
       {
       	has_else_stack = push(has_else_stack,1);
	else_label_stack = push(else_label_stack,currQuad++);//keep the label in case there is an else
	fprintf(syntax_fp,"found else\n");
       } stmt
       | /*empty*/
       {has_else_stack = push(has_else_stack,0);/*else_label_stack = push(else_label_stack,currQuad);*//*keep the label of else*/}
       ;

ifstmt: IF LPAR expr RPAR 
      {
	emit(OP_IF_EQ, new_jump_expr(currQuad + 2), new_bool_expr(B_TRUE), new_expr_expr($3),currQuad,yylineno);
	if_label_stack = push(if_label_stack,currQuad++);//keep the label of the after stmt jump
      }
      stmt 
      {
      		//else_label_stack = push(else_label_stack,currQuad);//keep the label in case there is an else
      		//emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL , if_label_stack->num , yylineno);
		//if_label_stack = pop(if_label_stack);
		//currQuad--;
      }
      elsestm
      {
      	if(has_else_stack->num == 0)
	{
      		emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL , if_label_stack->num , yylineno);
		currQuad--;//=2;
		if_label_stack = pop(if_label_stack);
		has_else_stack = pop(has_else_stack);
	}
	else if(has_else_stack->num == 1)
	{
		emit(OP_JUMP, new_jump_expr(else_label_stack->num + 1), NULL , NULL , if_label_stack->num , yylineno);
		currQuad--;
		emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL , else_label_stack->num , yylineno);
		currQuad--;
		if_label_stack = pop(if_label_stack);
		else_label_stack = pop(else_label_stack);
		has_else_stack = pop(has_else_stack);
	}
      }
      ;

whilestmt: WHILE{ while_start_label_stack = push(while_start_label_stack,currQuad); } LPAR expr RPAR 
	 {
	 	num_of_breaks_stack = push(num_of_breaks_stack,0);
		num_of_continues_stack = push(num_of_continues_stack,0);
	 	emit(OP_IF_EQ, new_jump_expr(currQuad + 2), new_bool_expr(B_TRUE), new_expr_expr($4),currQuad,yylineno);
		while_end_label_stack=push(while_end_label_stack,currQuad++);
	 }
	 stmt
	 {
	 	
		emit(OP_JUMP, new_jump_expr(while_start_label_stack->num), NULL , NULL ,currQuad , yylineno);
		
		printf("@start:%d end:%d curr:%d\n\n\n",while_start_label_stack->num,while_end_label_stack->num,currQuad);
		emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL ,while_end_label_stack->num , yylineno);
		currQuad--;
		while(break_stack != NULL && num_of_breaks_stack->num > 0)
	 	{
	  		emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL ,break_stack->num , yylineno);
			break_stack = pop(break_stack);
			num_of_breaks_stack->num--;
			currQuad--;
	  	}
	  	while(continue_stack != NULL && num_of_continues_stack->num > 0)
	  	{
	  		emit(OP_JUMP, new_jump_expr(while_start_label_stack->num), NULL ,NULL,continue_stack->num , yylineno);
			continue_stack = pop(continue_stack);
			num_of_continues_stack->num--;
			currQuad--;
	  	}
		while_start_label_stack=pop(while_start_label_stack);
		while_end_label_stack=pop(while_end_label_stack);
		num_of_breaks_stack = pop(num_of_breaks_stack);
		num_of_continues_stack = pop(num_of_continues_stack);
	 }
	 ;

forstmt: FOR LPAR elist QMARK
       {
        num_of_breaks_stack = push(num_of_breaks_stack,0);
	num_of_continues_stack = push(num_of_continues_stack,0);
	for_expr_start_stack=push(for_expr_start_stack,currQuad);
       }
       expr QMARK
       {
	   for_start_stack=push(for_start_stack,currQuad);
	   currQuad+=2;
       }elist
       {
	   emit(OP_JUMP, new_jump_expr(for_expr_start_stack->num), NULL , NULL ,currQuad , yylineno);
	   emit(OP_IF_EQ, new_jump_expr(currQuad),  new_bool_expr(B_TRUE) ,new_expr_expr( $6 ) ,for_start_stack->num , yylineno);
	   currQuad--;
       }
       RPAR stmt
       {
          emit(OP_JUMP, new_jump_expr(for_start_stack->num + 2), NULL , NULL ,currQuad , yylineno);
	  emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL ,for_start_stack->num + 1 , yylineno);
	  currQuad--;
	  while(break_stack != NULL && num_of_breaks_stack->num > 0)
	  {
	  	emit(OP_JUMP, new_jump_expr(currQuad), NULL , NULL,break_stack->num , yylineno);
		break_stack = pop(break_stack);
		num_of_breaks_stack->num--;
		currQuad--;
	  }
	  while(continue_stack != NULL && num_of_continues_stack->num > 0)
	  {
	  	emit(OP_JUMP, new_jump_expr(for_start_stack->num+2) , NULL ,NULL,continue_stack->num , yylineno);
		continue_stack = pop(continue_stack);
		num_of_continues_stack->num--;
		currQuad--;
	  }
	  for_start_stack=pop(for_start_stack);
	  for_expr_start_stack=pop(for_expr_start_stack);
	  num_of_breaks_stack = pop(num_of_breaks_stack);
	  num_of_continues_stack = pop(num_of_continues_stack);
       }
       ;

retexpr: expr QMARK 
       {
	  	expression* tmp =  new_expr_expr($1);
		emit(OP_RET, tmp, NULL , NULL ,currQuad,yylineno);
		$$ = tmp;
       }
       | QMARK
       {
		emit(OP_RET, NULL, NULL , NULL ,currQuad,yylineno);
       }
       ;

returnstmt: RETURN retexpr
	  ;

%%

int yyerror (char* yaccProvidedMessage)
{
	fprintf(stderr, "%s: at line %d, before token: --->%s<---\n", yaccProvidedMessage, yylineno, yytext);
	fprintf(stderr, "INPUT NOT VALID\n");
}

//**********************************************************************

int main(int argc, char** argv)
{
	fflush(stdin);
	char* bin_name;
	syntax_fp = fopen("syntax.txt","w");
	if (argc > 1) {
		if (!(yyin = fopen(argv[1], "r"))) {
				fprintf(stderr, "Cannot read file: %s\n", argv[1]);
				return 1;
		}
		bin_name = (char*)malloc(200);
		sprintf(bin_name,"%s.abc",argv[1]);
	}
	else
	{
		yyin = stdin;
		bin_name = strdup("user_input.abc");
	}
	libFunctions();
	yyparse();
	if(error_flag == B_TRUE)
		exit(-1);
	symbol_fp = fopen("symbol_table.txt","w");
	quads_fp  = fopen("quads.i","w");
	instructions_fp  = fopen("instructions.t","w");
	instr_bin_fp = fopen(bin_name,"w");
	if(!quads_fp || !symbol_fp)
		return 1;
	printSymbolTable();
	fprintf(quads_fp,"\n\t***************** List of the Quads ******************\n");
	printQuads();
	generate_t();

	printInstructions();
	print_const_nums();
	print_const_strings();
	print_user_funcs();
	print_lib_funcs();

	//printf("to functors einai:%d!\n",functor);

	
	return 0;
}


