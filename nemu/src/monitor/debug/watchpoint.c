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
WP* new_wp() {
	WP *wp=free_;
	if(!free_) return NULL;
	free_=free_->next;
	wp->next=head;
	head=wp;
	return wp;
}
void free_wp(WP *wp) {
	WP *pre;
	if(!wp) return;
	if(wp==head) head=head->next;
	else {
		for(pre=head;pre && pre->next!=wp;pre=pre->next);
		if(!pre) return;
		pre->next=wp->next;
	}
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
			printf("Watchpoint %d: %s\n",wp->NO,wp->expr);
			printf("Old value = 0x%08x\n",wp->val);
			printf("New value = 0x%08x\n",value);
			wp->val=value;
			return false;
		}
	}
	return true;
}
void delete_wp(int no) {
	WP *wp=wp_pool+no;
	if(no<0 || NR_WP<=no) return;
	free_wp(wp);
}
void info_wp() {
	WP *wp;
	for(wp=head;wp;wp=wp->next) printf("Watchpoint %d: %s = %d\n",wp->NO,wp->expr,wp->val);
}