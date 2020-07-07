

%{
#include "parser.h" /* <- will be generated from parser.y */

#define LBRACKET 0
#define RBRACKET 1
#define LSLASH   2
#define RSLASH   3

#define VFALSE	 0
#define VTRUE	 1

#define FATAL_ERROR 47
#define ERROR 	    48


typedef struct alpha_token_t
{
	char *key;
	int count;
	int token_count;
	int line;
	char* value;
	struct alpha_token_t* next;
}alpha_token_t;

int key_ctr=0;
int op_ctr=0;
int punct_ctr=0;
int int_ctr=0;
int float_ctr=0;
int id_ctr=0;
int string_ctr=0;
int line_com_ctr = 0;
int comment_ctr = 0;
int token_count = 0;


alpha_token_t* head=NULL;

alpha_token_t* newToken(int id,int index);
void printTokenList(char* file_name);

int com_array[100];

char buffer[500];

int alpha_yylex(void* yyval);



%}
/* Flex options */
%option noyywrap
%option yylineno

/* Flex macros */


/* keywords */
keyword_if		"if"
keyword_else		"else"
keyword_while		"while"
keyword_for		"for"
keyword_function	"function"
keyword_return 		"return"
keyword_break		"break"
keyword_continue	"continue"
keyword_and		"and"
keyword_not		"not"|"!"
keyword_or		"or"
keyword_local		"$"|"local"
keyword_global		"::"|"global"
keyword_true		"true"
keyword_false		"false"
keyword_nil		"nil"

/* operators */
operator_set		"="
operator_plus		"+"
operator_minus		"-"
operator_multi		"*"
operator_div		"/"
operator_mod		"%"
operator_equal		"=="
operator_diff		"!="
operator_pp		"++"
operator_mm		"--"
operator_greater	">"
operator_less		"<"
operator_gequal		">="
operator_lequal		"<="

/* punctuation */

punct_lhook		"{"
punct_rhook		"}"
punct_lbracket		"["
punct_rbracket		"]"
punct_dot		"."
punct_lpar		"("
punct_rpar		")"
punct_questionmark	";"
punct_comma		","
punct_updowndot		":"

integer 		[0-9]+
float			[0-9]+\.[0-9]+
id			[a-zA-Z][a-zA-Z_0-9]*
linecomment		"//".*|"#".*



%%

{keyword_if}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return IF; }
{keyword_else}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return ELSE; }
{keyword_while}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return WHILE; }
{keyword_for}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return FOR; }
{keyword_function}	{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return FUNCTION; }
{keyword_return}	{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return RETURN; }
{keyword_break}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return BREAK; }
{keyword_continue}	{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return CONTINUE; }
{keyword_and}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return AND; }
{keyword_not}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return NOT; }
{keyword_or}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return OR; }
{keyword_local}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return LOCAL; }
{keyword_global}	{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return GLOBAL; }
{keyword_true}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return TRUE; }
{keyword_false}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return FALSE; }
{keyword_nil}		{  token_count++; key_ctr++; yylval.stringValue = strdup(yytext); return NIL; }

{operator_set}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return SET; }
{operator_plus}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return PLUS; }
{operator_minus}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return MINUS; }
{operator_multi}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return MUL; }
{operator_div}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return DIV; }
{operator_mod}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return MOD; }
{operator_equal}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return EQUAL; }
{operator_diff}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return DIFF; }
{operator_pp}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return PP; }
{operator_mm}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return MM; }
{operator_greater}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return GREATER; }
{operator_less}		{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return LESS; }
{operator_gequal}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return GEQUAL; }
{operator_lequal}	{  token_count++; op_ctr++; yylval.stringValue = strdup(yytext); return LEQUAL; }

{punct_lhook}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return LHOOK; }
{punct_rhook}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return RHOOK; }
{punct_lbracket}	{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return LBRACKT; }
{punct_rbracket}	{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return RBRACKT; }
{punct_dot}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return DOT; }
{punct_lpar}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return LPAR; }
{punct_rpar}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return RPAR; }
{punct_questionmark}	{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return QMARK; }
{punct_comma}		{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return COMMA; }
{punct_updowndot}	{  token_count++; punct_ctr++; yylval.stringValue = strdup(yytext); return UDDOT; }

{integer}		{  token_count++; int_ctr++; yylval.intValue = atoi(yytext); return INTEGER; }

{float}			{  token_count++; float_ctr++; yylval.realValue = atof(yytext); return FLOAT; }

{id}			{  token_count++; id_ctr++; yylval.stringValue = strdup(yytext); return ID; }

{linecomment}		{  token_count++; line_com_ctr++; yylval.stringValue = strdup(yytext); return LINECOM; }

"(*"			{ 
				int i = 0 , num = 0 ;
				int is_finished = -1;
				char c,next_c;
				com_array[0] = LBRACKET;
				
				
				while(is_finished != VTRUE)
				{
					c = input();
					sprintf(yytext,"%s%c",yytext,c);
					if(c == '(' )
					{
						next_c = input();
						if(next_c == '*')
						{
							sprintf(yytext,"%s%c",yytext,next_c);
							num++;
							com_array[num] = LBRACKET;
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
						
					if(c == '/')
					{
						next_c = input();
						if(next_c == '*')
						{
							sprintf(yytext,"%s%c",yytext,next_c);
							num++;
							com_array[num] = LSLASH;
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
					
					if(c == '*')
					{
						next_c = input();
						if(next_c == '/')
						{
							if(com_array[num] == LSLASH)
							{
								sprintf(yytext,"%s%c",yytext,next_c);
								num--;
							}
							else
							{
								fprintf(stderr, "Error in nested comments at line %d.\n",yylineno);
								return ERROR;
							}
						}
						else if(next_c == ')')
						{
							if(com_array[num] == LBRACKET)
							{
								sprintf(yytext,"%s%c",yytext,next_c);
								num--;
							}
							else
							{
								fprintf(stderr, "Error in nested comments at line %d.\n",yylineno);
								return ERROR;
							}
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
					if(c == EOF)
					{
						fprintf(stderr, "Error!Found EOF before the end of the comments\n");
						return FATAL_ERROR;
					}
					if(num == -1)
						is_finished = VTRUE;
					
					
				}
				
					
				if(is_finished == VTRUE)
				{
					comment_ctr++;
					token_count++; 
					yylval.stringValue = strdup(yytext); 
					return COMMENT;
				} 
			}

"/*"			{ 
				int i = 0 , num = 0 ;
				int is_finished = -1;
				char c,next_c;
				com_array[0] = LSLASH;
				
				
				while(is_finished != VTRUE)
				{
					c = input();
					sprintf(yytext,"%s%c",yytext,c);
					if(c == '(' )
					{
						next_c = input();
						if(next_c == '*')
						{
							sprintf(yytext,"%s%c",yytext,next_c);
							num++;
							com_array[num] = LBRACKET;
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
						
					if(c == '/')
					{
						next_c = input();
						if(next_c == '*')
						{
							sprintf(yytext,"%s%c",yytext,next_c);
							num++;
							com_array[num] = LSLASH;
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
					
					if(c == '*')
					{
						next_c = input();
						if(next_c == '/')
						{
							if(com_array[num] == LSLASH)
							{
								sprintf(yytext,"%s%c",yytext,next_c);
								num--;
							}
							else
							{
								fprintf(stderr, "Error in nested comments at line %d.\n",yylineno);
								return ERROR;
							}
						}
						else if(next_c == ')')
						{
							if(com_array[num] == LBRACKET)
							{
								sprintf(yytext,"%s%c",yytext,next_c);
								num--;
							}
							else
							{
								fprintf(stderr, "Error in nested comments at line %d.\n",yylineno);
								return ERROR;
							}
						}
						else if(next_c == EOF)
						{
							fprintf(stderr, "Error!Found EOF before the end of the comments\n");
							return FATAL_ERROR;
						}
						else
							unput(next_c);
					}
					if(c == EOF)
					{
						fprintf(stderr, "Error!Found EOF before the end of the comments\n");
						return FATAL_ERROR;
					}
					if(num == -1)
						is_finished = VTRUE;
					
					
				}
				if(is_finished == VTRUE)
				{
					comment_ctr++;
					token_count++; 
					yylval.stringValue = strdup(yytext); 
					return COMMENT;
				}
			}



\"			{
				char c;
				int error=0;
				int slash=0;
				int i=0;

				while((c=input())!='"')
				{
					if(c==EOF)
					{
						error=1;
						fprintf(stderr, "Found EOF before the end of the string.\n"); 
						return FATAL_ERROR;
					}
					else if(c=='\n'&&slash==0)
					{
						error=1;
						buffer[i]='\0';
						fprintf(stderr, "String error at line %d    value: '%s'\n",yylineno,buffer); 
						return ERROR;
					}
					else if(c == '\\')
					{
						slash=1;
						buffer[i++]=c;
						c=input();
						if(c==' ')
						{
							c=input();
							if(c=='\n')
							{
								buffer[i]='\0';
								fprintf(stderr, "String error at line %d    value: '%s'\n",yylineno,buffer); 
								return ERROR;
							}
							else 
								unput(c);
						}
						else if(c=='"')
						{
							c=input();
							if(c=='\n')
							{
								error=1;
								buffer[i]='\0';
								fprintf(stderr, "String error at line %d    value: '%s'\n",yylineno,buffer);
								return ERROR;
							}
							else 
								unput(c);
						}
						else
						{
							if(c!='\n')
								buffer[i++]=c;
						}
					}
					else
					{
						buffer[i++]=c;
						slash=0;
						error=0;
					}
				}
				if(error == 0)
				{
					buffer[i]='\0';
					string_ctr++;
					token_count++; 
					yytext = strcpy(yytext,buffer);
					yylval.stringValue = strdup(yytext); 
					return STRING;
				}
				else
				{
					buffer[i]='\0';
					fprintf(stderr, "String error at line %d    value: '%s'\n",yylineno,buffer); 
					return ERROR;
				}
			}

"\t"		{}
" "		{}
"\n"		{}

.		{ /*fprintf(stderr, "Cannot match character '%s' with any rule\n", yytext); */}
%%

/*int main (int argc, char** argv)
{
	if (argc > 1) {
		if (!(yyin = fopen(argv[1], "r"))) {
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}
	}
	else
		yyin = stdin;

	
	while(alpha_yylex(newToken(yylex(),yylineno)) != 1)
	{}
	
	if(argc > 2)
		printTokenList(argv[2]);
	else
		printTokenList(NULL);
	return 0;
}*/

alpha_token_t* newToken(int id,int index)
{

	alpha_token_t *newnode;

	newnode=(alpha_token_t *)malloc(sizeof(alpha_token_t));

	if(id==IF)
	{
		newnode->key=strdup("KEYWORD_IF");
		newnode->count = key_ctr;
	}
	else if(id==ELSE)
	{
		newnode->key=strdup("KEYWORD_ELSE");
		newnode->count = key_ctr;
	}
	else if(id==WHILE)
	{
		newnode->key=strdup("KEYWORD_WHILE");	
		newnode->count = key_ctr;
	}
	else if(id==FOR)
	{
		newnode->key=strdup("KEYWORD_FOR");
		newnode->count = key_ctr;
	}
	else if(id==FUNCTION)
	{
		newnode->key=strdup("KEYWORD_FUNCTION");
		newnode->count = key_ctr;
	}
	else if(id==RETURN)
	{
		newnode->key=strdup("KEYWORD_RETURN");
		newnode->count = key_ctr;
	}
	else if(id==BREAK)
	{
		newnode->key=strdup("KEYWORD_BREAK");
		newnode->count = key_ctr;
	}
	else if(id==CONTINUE)
	{
		newnode->key=strdup("KEYWORD_CONTINUE");
		newnode->count = key_ctr;
	}
	else if(id==AND)
	{
		newnode->key=strdup("KEYWORD_AND");
		newnode->count = key_ctr;
	}
	else if(id==NOT)
	{
		newnode->key=strdup("KEYWORD_NOT");
		newnode->count = key_ctr;
	}
	else if(id==OR)
	{
		newnode->key=strdup("KEYWORD_OR");
		newnode->count = key_ctr;
	}
	else if(id==LOCAL)
	{
		newnode->key=strdup("KEYWORD_LOCAL");
		newnode->count = key_ctr;
	}
	else if(id==GLOBAL)
	{
		newnode->key=strdup("KEYWORD_GLOBAL");
		newnode->count = key_ctr;
	}
	else if(id==TRUE)
	{
		newnode->key=strdup("KEYWORD_VTRUE");
		newnode->count = key_ctr;
	}
	else if(id==FALSE)
	{
		newnode->key=strdup("KEYWORD_VFALSE");
		newnode->count = key_ctr;
	}
	else if(id==NIL)
	{
		newnode->key=strdup("KEYWORD_NIL");
		newnode->count = key_ctr;
	}
	else if(id==SET)
	{
		newnode->key=strdup("OPERATOR_SET");
		newnode->count = op_ctr;
	}
	else if(id==PLUS)
	{
		newnode->key=strdup("OPERATOR_PLUS");
		newnode->count = op_ctr;
	}
	else if(id==MINUS)
	{
		newnode->key=strdup("OPERATOR_MINUS");
		newnode->count = op_ctr;
	}
	else if(id==MUL)
	{
		newnode->key=strdup("OPERATOR_MUL");
		newnode->count = op_ctr;
	}
	else if(id==DIV)
	{
		newnode->key=strdup("OPERATOR_DIV");
		newnode->count = op_ctr;
	}
	else if(id==MOD)
	{
		newnode->key=strdup("OPERATOR_MOD");
		newnode->count = op_ctr;
	}
	else if(id==EQUAL)
	{
		newnode->key=strdup("OPERATOR_EQUAL");
		newnode->count = op_ctr;
	}
	else if(id==DIFF)
	{
		newnode->key=strdup("OPERATOR_DIFF");
		newnode->count = op_ctr;
	}
	else if(id==PP)
	{
		newnode->key=strdup("OPERATOR_PP");
		newnode->count = op_ctr;
	}
	else if(id==MM)
	{
		newnode->key=strdup("OPERATOR_MM");
		newnode->count = op_ctr;
	}
	else if(id==GREATER)
	{
		newnode->key=strdup("OPERATOR_GREATER");
		newnode->count = op_ctr;
	}
	else if(id==LESS)
	{
		newnode->key=strdup("OPERATOR_LESS");
		newnode->count = op_ctr;
	}
	else if(id==GEQUAL)
	{
		newnode->key=strdup("OPERATOR_GEQUAL");
		newnode->count = op_ctr;
	}
	else if(id==LEQUAL)
	{
		newnode->key=strdup("OPERATOR_LEQUAL");
		newnode->count = op_ctr;
	}
	else if(id==INTEGER)
	{
		newnode->key=strdup("INTEGER");
		newnode->count = int_ctr;
	}
	else if(id==FLOAT)
	{
		newnode->key=strdup("FLOAT");
		newnode->count = float_ctr;
	}
	else if(id==STRING)
	{
		newnode->key=strdup("STRING");
		newnode->count = string_ctr;
	}
	else if(id==LHOOK)
	{
		newnode->key=strdup("PUNCT_LHOOK");
		newnode->count = punct_ctr;
	}
	else if(id==RHOOK)
	{
		newnode->key=strdup("PUNCT_RHOOK");
		newnode->count = punct_ctr;
	}
	else if(id==LBRACKT)
	{
		newnode->key=strdup("PUNCT_LBRACKET");
		newnode->count = punct_ctr;
	}
	else if(id==RBRACKT)
	{
		newnode->key=strdup("PUNCT_RBRACKET");
		newnode->count = punct_ctr;
	}
	else if(id==DOT)
	{
		newnode->key=strdup("PUNCT_DOT");
		newnode->count = punct_ctr;
	}
	else if(id==LPAR)
	{
		newnode->key=strdup("PUNCT_LPAR");
		newnode->count = punct_ctr;
	}
	else if(id==RPAR)
	{
		newnode->key=strdup("PUNCT_RPAR");
		newnode->count = punct_ctr;
	}
	else if(id==QMARK)
	{
		newnode->key=strdup("PUNCT_QMARK");
		newnode->count = punct_ctr;
	}
	else if(id==COMMA)
	{
		newnode->key=strdup("PUNCT_COMMA");
		newnode->count = punct_ctr;
	}
	else if(id==UDDOT)
	{
		newnode->key=strdup("PUNCT_UDDOT");
		newnode->count = punct_ctr;
	}
	else if(id==ID)
	{
		newnode->key=strdup("ID");
		newnode->count = id_ctr;
	}
	else if(id==LINECOM)
	{
		newnode->key=strdup("LINECOM");
		newnode->count = line_com_ctr;
	}
	else if(id==COMMENT)
	{
		newnode->key=strdup("COMMENT");
		newnode->count = comment_ctr;
	}
	else if(id==ERROR)
		newnode->key=strdup("ERROR");
	else if(id==FATAL_ERROR)
		newnode->key=strdup("FATAL_ERROR");
	
	newnode->token_count = token_count;
	newnode->value = strdup(yytext);
	newnode->line=index;
	newnode->next=NULL;

	return newnode;
	
}

int alpha_yylex(void* newtoken)
{
	alpha_token_t *tmp = newtoken;	
	if(!strcmp(tmp->key,"ERROR"))
		return 0;
	if(!strcmp(tmp->key,"FATAL_ERROR"))
		return 1;
	if(head==NULL)
	{
		head=newtoken;
		return 0;
	}
	else
	{
		tmp=head;
		while(tmp->next!=NULL)
		{
			tmp=tmp->next;
		}

		tmp->next=newtoken;
		return 0;

	}	
	return 1;

}

void printTokenList(char* file_name)
{
	alpha_token_t *tmp = head;
	FILE* fp;
	if(file_name != NULL)
		fp = fopen(file_name,"w");
	if(fp == NULL)
		return;
	while(tmp!=NULL)
	{
		if(file_name == NULL)
			fprintf(stdout, "%d,%d,\"%s\" %s\n", tmp->token_count,tmp->line,tmp->value,tmp->key);
		else
			fprintf(fp, "%d,%d,\"%s\" %s\n", tmp->token_count,tmp->line,tmp->value,tmp->key);

		tmp=tmp->next;
	}

}

