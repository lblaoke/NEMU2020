#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

typedef struct {
    swaddr_t prev_ebp;
    swaddr_t ret_addr;
    uint32_t args[4];
} PartOfStackFrame;

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_help(char *args);
static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}
static int cmd_q(char *args) {
	return -1;
}
static int cmd_si(char *args) {
	int num=1;
	if(args) sscanf(args,"%d",&num);
	cpu_exec(num);
	return 0;
}
static int cmd_info(char *args) {
	int i;
	if(!args) {
		printf("command required!\n");
		return 0;
	}
	switch(args[0]) {
		case('r'):
			for(i=0;i<8;i++) printf("%s 0x%08x\n",regsl[i],reg_l(i));
			printf("eip 0x%08x\n",cpu.eip);
			break;
		case('w'):
			info_wp();
			break;
		default:
			printf("illegel command!\n");
			assert(0);
	}
	return 0;
}
static int cmd_x(char *args) {
	int i,n;
	swaddr_t addr;
	bool suc;

	args=strtok(args," ");
	sscanf(args,"%d",&n);
	args=args+strlen(args)+1;
	addr=expr(args,&suc);
	if (!suc)assert (1);

	printf("0x%08x:\n",addr);
	for(i=0;i<n;i++,addr+=4) printf("0x%08x ",swaddr_read(addr,4,R_DS));
	printf("\n");
	return 0;
}
static int cmd_p(char *args) {
	bool success;
	uint32_t result=expr(args,&success);
	if(success) printf("0x%08x\n",result);
	else assert(0);
	return 0;
}
static int cmd_b(char *args) {return 0;}
static int cmd_w(char *args) {
	bool success;
	WP *wp=new_wp();
	strcpy(wp->expr,args);
	wp->val=expr(args,&success);
	if(!success) assert(0);
	printf("%s = 0x%08x\n",args,wp->val);
	return 0;
}
static int cmd_d(char *args) {
	int no;
	sscanf(args,"%d",&no);
	delete_wp(no);
	return 0;
}

void elf_func(swaddr_t,char *);

void read_ebp (swaddr_t addr , PartOfStackFrame *ebp) {
	ebp -> prev_ebp = swaddr_read (addr , 4, R_SS);
	ebp -> ret_addr = swaddr_read (addr + 4 , 4, R_SS);
	int i;
	for (i = 0;i < 4;i ++) ebp -> args [i] = swaddr_read (addr + 8 + 4 * i , 4, R_SS);
}
static int cmd_bt(char *args) {
	int j = 0;
	PartOfStackFrame now_ebp;
	char tmp [32];
	swaddr_t addr = reg_l (R_EBP);
	now_ebp.ret_addr = cpu.eip;
	while (addr > 0) {
		printf ("#%d  0x%08x in ",j++,now_ebp.ret_addr);
		elf_func(now_ebp.ret_addr,tmp);
		printf("%s\t",tmp);
		read_ebp (addr,&now_ebp);
		if (strcmp (tmp,"main") == 0)printf ("( )\n");
		else printf ("( %d , %d , %d , %d )\n", now_ebp.args[0],now_ebp.args[1],now_ebp.args[2],now_ebp.args[3]);
		addr = now_ebp.prev_ebp;
	}
	return 0;
}

static int cmd_cache(char *args) {return 0;}

static int cmd_page(char *args) {
	if(!args) return 0;

	lnaddr_t lnaddr;
	sscanf(args,"%x",&lnaddr);
	hwaddr_t hwaddr = page_translate(lnaddr);

	if(cpu.cr0.protect_enable && cpu.cr0.paging) printf("0x%x -> 0x%x\n",lnaddr,hwaddr);
	else printf("\033[1;33mPage address convertion is invalid.\n\033[0m");
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler)(char *);
} cmd_table[]={
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step into implementation of N instructions after the suspension of execution.When N is notgiven,the default is 1.", cmd_si},
	{ "info", "r for print register state\nw for print watchpoint information", cmd_info},
	{ "x", "Calculate the value of the expression and regard the result as the starting memory address.", cmd_x},
	{ "p", "Expression evaluation", cmd_p},
	{ "b", "Breakpoint + *ADDR.", cmd_b},
	{ "w", "Stop the execution of the program if the result of the expression has changed.", cmd_w},
	{ "d", "Delete the Nth watchpoint", cmd_d},
	{ "bt", "Print stack frame chain", cmd_bt},
	{ "cache", "Print cache block infomation", cmd_cache},
	{ "page", "Convert linear address to physical one", cmd_page}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
