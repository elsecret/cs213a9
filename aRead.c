#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

queue_t      pending_read_queue;
unsigned int sum = 0;

void interrupt_service_routine() {
	void* val;
	void(*callback)(void*, void*);
	queue_dequeue(pending_read_queue, &val, NULL, &callback);
	callback(val, NULL);
}

void handle_read(void* resultv, void* not_used) {
	sum += *(int*)resultv;
}

void handle_read_and_exit(void* resultv, void* not_used) {
	handle_read(resultv, NULL);
	printf("%d\n", sum);
	exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
	// Command Line Arguments
	static char* usage = "usage: aRead num_blocks";
	int num_blocks;
	char *endptr;
	if (argc == 2)
		num_blocks = strtol(argv[1], &endptr, 10);
	if (argc != 2 || *endptr != 0) {
		printf("argument error - %s \n", usage);
		return EXIT_FAILURE;
	}
	// Initialize
	uthread_init(1);
	disk_start(interrupt_service_routine);
	pending_read_queue = queue_create();
	// Sum Blocks
	//int * result = malloc(sizeof(int) * num_blocks);
	//for (int blockno = 0; blockno < num_blocks - 1; blockno++) {
	//	queue_enqueue(pending_read_queue, &result[blockno], NULL, handle_read);
	//	disk_schedule_read(&result[blockno], blockno);
	//}
	//queue_enqueue(pending_read_queue, &result[num_blocks - 1], NULL, handle_read_and_exit);
	//disk_schedule_read(&result[num_blocks - 1], num_blocks - 1);
	//// why even free it when program is gonna exit

	//this way is more dangerous i think?? but it works
	for (int i = 0; i < num_blocks; i++){
		int r = 0;
		if (i < num_blocks - 1){
			queue_enqueue(pending_read_queue, &r, NULL, handle_read);
		} else {
			queue_enqueue(pending_read_queue, &r, NULL, handle_read_and_exit);
		}
		disk_schedule_read(&r, i);
	}

	while (1);

}


