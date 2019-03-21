//
// This is the solution to CPSC 213 Assignment 9.
// Do not distribute this code or any portion of it to anyone in any way.
// Do not remove this comment.
//

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
	// dequeue to get thread id, and unblock it
	uthread_t* t;
	queue_dequeue(pending_read_queue,(void*) &t, NULL, NULL);
	uthread_unblock(*t);
}

void* read_block(void* blocknov) {
	// TODO enqueue result, schedule read, and the update (correctly)
	int result = 0;
	disk_schedule_read(&result, *(int*)blocknov);
	uthread_t t = uthread_self();
	queue_enqueue(pending_read_queue, &t, NULL, NULL);
	uthread_block();
	sum += result;
	return NULL;
}

int main(int argc, char** argv) {

	// Command Line Arguments
	static char* usage = "usage: tRead num_blocks";
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

	int* indices = malloc(sizeof(int) * num_blocks);
	for (int i = 0; i < num_blocks; i++){
		indices[i] = i;
	}

	for (size_t i = 0; i < num_blocks - 1; i++)
	{
		uthread_create(read_block, &indices[i]);
	}
	uthread_join(uthread_create(read_block, &indices[num_blocks - 1]), NULL);

	// print the sum
	printf("%d\n", sum);
}

