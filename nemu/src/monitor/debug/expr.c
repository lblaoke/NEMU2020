#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {NOTYPE = 256, EQ , NEQ , AND , OR , MINUS , POINTOR , NUMBER , HNUMBER , REGISTER , MARK};

static struct rule {
	char *regex;
	int token_type,priority;
} rules[]={
	{"\\b[0-9]+\\b",NUMBER,0},				// number
	{"\\b0[xX][0-9a-fA-F]+\\b",HNUMBER,0},		// 16 number
	{"\\$[a-zA-Z]+",REGISTER,0},				// register
	{"\\b[a-zA-Z_0-9]+" , MARK , 0},		// mark
	{"!=",NEQ,3},						// not equal	
	{"!",'!',6},						// not
	{"\\*",'*',5},						// mul
	{"/",'/',5},						// div
	{"	+",NOTYPE,0},					// tabs
	{" +",NOTYPE,0},					// spaces
	{"\\+",'+',4},						// plus
	{"-",'-',4},						// sub
	{"==", EQ,3},						// equal
	{"&&",AND,2},						// and
	{"\\|\\|",OR,1},						// or
	{"\\(",'(',7},                        // left bracket   
	{"\\)",')',7},                        // right bracket 
};


#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type,priority;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				
				position += substr_len;
				
				switch(rules[i].token_type) {
					case(NOTYPE):break;
					case(REGISTER):
						tokens[nr_token].type=rules[i].token_type;
						tokens[nr_token].priority=rules[i].priority;
						strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
						tokens[nr_token].str[substr_len-1]='\0';
						nr_token++;
						break;
					default:
						tokens[nr_token].type=rules[i].token_type;
						tokens[nr_token].priority=rules[i].priority;
						strncpy(tokens[nr_token].str,substr_start,substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

uint32_t expr(char *e,bool *success) {
	if(!make_token(e)) {
		*success=false;
		return 0;
	}
	*success=true;
	return 0;
}