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
			if(!(regexec(re+i,e+position,1,&pmatch,0) || pmatch.rm_so)) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				
				position += substr_len;
				
				switch(rules[i].token_type) {
					case(NOTYPE): break;
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
uint32_t eval(int l,int r) {
	uint32_t result=0,l_operand,r_operand;
	int i,min_index=l,min_priority=10,bracket_count=0;

	if(l>r) return result;
	if(tokens[l].type==')' || tokens[r].type=='(') assert(0);
	while(tokens[l].type=='(' && tokens[r].type==')') {
		l++;
		r--;
		if(l>r) assert(0);
	}

	if(l==r) {
		if(tokens[l].type==HNUMBER) sscanf(tokens[l].str,"%x",&result);
		else if(tokens[l].type==REGISTER) {
			if(!strcmp(tokens[l].str,"eip")) result=cpu.eip;
			else {
				for(i=0;i<8;i++) if(!strcmp(tokens[l].str,regsl[i])) break;
				if(i<8) result=reg_l(i);
				else assert(0);
			}
		}
		else if(tokens[l].type==NUMBER) sscanf(tokens[l].str,"%d",&result);
		else assert(0);
		return result;
	}

	for(i=l;i<=r;i++) switch(tokens[i].type) {
		case('('):
			bracket_count++;
			break;
		case(')'):
			bracket_count--;
			break;
		default: if(tokens[i].priority && !bracket_count && tokens[i].priority<min_priority) {
				min_index=i;
				min_priority=tokens[i].priority;
			}
	}

	printf("%s ",tokens[min_index].str);

	l_operand=eval(l,min_index-1);
	r_operand=eval(min_index+1,r);
	if(min_index==l) switch(tokens[min_index].type) {
		case('-'): return (~r_operand)+1;
		case('!'): return !r_operand;
		case('*'): return swaddr_read(r_operand,4);
	} else switch(tokens[min_index].type) {
		case('+'): return l_operand+r_operand;
		case('-'): return l_operand-r_operand;
		case('*'): return (uint32_t)((int)l_operand*(int)r_operand);
		case('/'): return (uint32_t)((int)l_operand/(int)r_operand);
		case(AND): return l_operand && r_operand;
		case(OR): return l_operand+r || r_operand;
		case(EQ): return l_operand+r==r_operand;
		case(NEQ): return l_operand+r!=r_operand;
	}

	assert(0);
	return result;
}
int check_op(int p) {
	if(p<0 || p>=nr_token) return 0;
	if(tokens[p].type=='-' || tokens[p].type=='!' || tokens[p].type=='*') return 3;
	if(1<=tokens[p].priority && tokens[p].priority<=6) return 2;
	return 1;
}
uint32_t expr(char *e,bool *success) {
	int i,op;
	*success=make_token(e);
	if(!*success) return 0;

	op=check_op(0);
	if(op==2) assert(0);
	if(op==3) tokens[0].priority=10;
	for(i=1;i<nr_token;i++) if(check_op(i-1)>1) {
		op=check_op(i);
		if(op==2) assert(0);
		if(op==3) tokens[i].priority=10;
	}

	return eval(0,nr_token-1);
}