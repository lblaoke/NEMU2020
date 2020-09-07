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
		wp_pool[i].next = &wp_pool[i + 1];
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
bool check_wp() {return 0;}
void delete_wp(int no) {
	if(0<no || no<=NR_WP) return;
	free_wp(wp_pool+no);
}
void info_wp() {
	WP *wp;
	for(wp=head;wp;wp=wp->next) printf("Watchpoint %d: %s = %d\n",wp->NO,wp->expr,wp->val);
}