#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = wp_pool+i+1;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
WP* new_wp () {
	WP *wp=free_;
	if(!free_) return NULL;
	free_=free_->next;
	wp->next=head;
	return head=wp;
}
void free_wp(WP *wp) {
	WP *pre;
	for(pre=head;pre && pre->next!=wp;pre=pre->next);
	if(!pre) return;
	pre->next=wp->next;
	wp->next=free_;
	free_=wp;
}
bool check_wp() {
	WP *wp;
	uint32_t value;
	bool success;
	for(wp=head;wp;wp=wp->next) {
		value=expr(wp->expr,&success);
		if(!success) assert(0);
		if(value!=wp->val) {
			if(wp->b) {
				printf("Hit breakpoint %d at 0x%08x\n",wp->b,cpu.eip);
				wp=wp->next;
				continue;
			}
			printf("Watchpoint %d: %s\n",wp->NO,wp->expr);
			printf("Old value = %d\n",wp->val);
			printf("New value = %d\n",value);
			wp->val=value;
			return false;
		}
	}
	return true;
}
void delete_wp(int no) {
	if(0<no || no<=NR_WP) return;
	free_wp(wp_pool+no);
}
void info_wp() {
	WP *wp;
	for(wp=head;wp;wp=wp->next) printf("Watchpoint %d: %s = %d\n",wp->NO,wp->expr,wp->val);
}