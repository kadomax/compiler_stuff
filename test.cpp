#include<cstdlib>
#include<cstdio>
#include<cstdint>
#include<cstring>
#include<cassert>



// stretchy buffer // init: NULL // free: sbfree() // push_back: sbpush() // size: sbcount() //
#define sbfree(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define sbpush(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define sbcount(a)        ((a) ? stb__sbn(a) : 0)
#define sbadd(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define sblast(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+n >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)  stb__sbgrowf((void **) &(a), (n), sizeof(*(a)))
#define sbprint(a) for(int i_ = 0 ; i_ < sbcount(a) ; i_++) printf("%d " , a[i_])
static void stb__sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*stb__sbm(*arr)+increment : increment+1;
   void *p = realloc(*arr ? stb__sbraw(*arr) : 0, itemsize * m + sizeof(int)*2);
   assert(p);
   if (p) {
      if (!*arr) ((int *) p)[1] = 0;
      *arr = (void *) ((int *) p + 2);
      stb__sbm(*arr) = m;
   }
}


#define max(x , y) x > y ? x : y
#define min(x , y) x < y ? x : y


enum TOKENTYPE{VAR , PLUS , MINUS , LPARAN , RPARAN , MUL , DIV , CONST , UNDEF};

struct token_t{
	TOKENTYPE t;
	float literal_const;
	char literal_var[100];
	char literal_op;
};

#define EMPTY_LITERAL_CONST 0
#define EMPTY_LITERAL_VAR "--empty--"
#define EMPTY_LITERAL_OP '\0'

token_t getEmptyToken(){
	token_t token = {UNDEF , EMPTY_LITERAL_CONST , EMPTY_LITERAL_VAR , '\0'};
	return token;
}

token_t createToken(TOKENTYPE t , float literal_const , char literal_var[] , char literal_op){
	token_t token;
	token.t = t;
	token.literal_const = literal_const;
	token.literal_op = literal_op; 
	strcpy(token.literal_var , literal_var);
	return token;
}

void printToken(token_t token){
	char token_type_string[10];
	switch(token.t){
		case VAR:
		{
			strcpy(token_type_string , "VAR");
			break;
		}
		case PLUS:
		{
			strcpy(token_type_string , "PLUS");
			break;
		}
		case MINUS:
		{
			strcpy(token_type_string , "MINUS");
			break;
		}
		case LPARAN:
		{
			strcpy(token_type_string , "LPARAN");
			break;
		}
		case RPARAN:
		{
			strcpy(token_type_string , "RPARAN");
			break;
		}
		case MUL:
		{
			strcpy(token_type_string , "MUL");
			break;	
		}
		case DIV:
		{
			strcpy(token_type_string , "DIV");
			break;
		}
		case CONST:
		{
			strcpy(token_type_string , "CONST");
			break;
		}
		default:
		{
			strcpy(token_type_string , "UNDEF");
			break;
		}
	}
	printf("token type = %s\n" , token_type_string);
	if(token.literal_const != EMPTY_LITERAL_CONST){
		printf("literal = %f \n" , token.literal_const);
	}
	else if(token.literal_op != EMPTY_LITERAL_OP){
		printf("literal = %c \n" , token.literal_op);
	}
	else if(strcmp(EMPTY_LITERAL_VAR , token.literal_var) != 0){
		printf("literal = %s \n" , token.literal_var);
	}
	printf("======================\n");
}


bool isDigit(char c){
	return (c >= '0' && c <= '9');
}

bool isLetter(char c){
	return (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_');
}

int getWord(char* source , uint32_t index , char word[]){
	// scans a word from the given index and puts it into the word[] buffer.
	// returns the index after the word
	uint32_t word_sp = 0;
	if(isLetter(source[index])){
		for(int i = index ; i < strlen(source) ; i++){
			if(isLetter(source[i]) || isDigit(source[i])){
				word[word_sp++] = source[i];
			}
			else{
				break;
			}
		}
	}
	word[word_sp] = '\0';
	return min(strlen(source) , index + word_sp);
}

int getNumber(char* source , uint32_t index , char number[]){
	uint32_t num_sp = 0;
	if(isDigit(source[index])){
		for(int i = index ; i < strlen(source) ; i++){
			if(isDigit(source[i]) || source[i] == '.'){
				number[num_sp++] = source[i];
			}
			else{
				break;
			}
		}
	}
	number[num_sp] = '\0';
	return min(strlen(source) , index + num_sp);
}

TOKENTYPE getOperatorTokenType(char c){
	switch(c){
		case '(':
			return LPARAN;
		case ')':
			return RPARAN;
		case '*':
			return MUL;
		case '/':
			return DIV;
		case '+':
			return PLUS;
		case '-':
			return MINUS;
		default:
			return UNDEF;
	}
}

bool isValidSym(char c){
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '*' || isLetter(c) || isDigit(c));
}

bool isOperator(char c){
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '*');
}

token_t* generateTokens(char* source){
	int index = 0;
	token_t* tokens = NULL;
	while(index < strlen(source)){
		if(isValidSym(source[index])){
			if(isOperator(source[index])){
				// printf("%c \n" , source[index]);
				token_t temp = createToken(getOperatorTokenType(source[index]) , EMPTY_LITERAL_CONST , EMPTY_LITERAL_VAR , source[index]);
				//printToken(temp);
				sbpush(tokens , temp);
				index++;
			}
			else if(isDigit(source[index])){
				char number[100];
				index = getNumber(source , index , number);
				token_t temp = createToken(CONST , atof(number) , EMPTY_LITERAL_VAR , EMPTY_LITERAL_OP);
				// printToken(temp);
				sbpush(tokens , temp);
				//printf("%s \n" , number);
			}
			else if(isLetter(source[index])){
				char word[100];
				index = getWord(source , index , word);
				token_t temp = createToken(VAR , EMPTY_LITERAL_CONST , word , EMPTY_LITERAL_OP);
				//printToken(temp);
				sbpush(tokens , temp);
				//printf("%s \n" , word);
			}
		}
		else{
			index++;
		}
	}
	return tokens;
}


int main(){

	printf("welcome to the tokenizer repl\n");
	char input[200];
	while(true){
		printf(">> ");
		gets_s(input);
		if(strcmp(input , "quit") == 0){
			break;
		}
		token_t* tokens = generateTokens(input);
		for(int i = 0 ; i < sbcount(tokens) ; i++){
			printToken(tokens[i]);
		}
		sbfree(tokens);
	}
}

#undef max
#undef min