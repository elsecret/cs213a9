#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

queue_t pending_read_queue;

void interrupt_service_routine() {
	void* x;
	void* y;
	void(*callback)(void*, void*);
	queue_dequeue(pending_read_queue, &x, &y, &callback);
	callback(x, y);
}

void handleOtherReads(void* resultv, void* countv) {
	/*
		If we're checking for the count to be 0, it must be decreased before we check it.
		Imagine the count was 1. Then it means the next disk read's interrupt, whatever
		value that is, that is the one that gets printed, and then we exit.
		So if you want to check for it to be equal to 0, then decrement it before checking,
		so that 1 becomes 0.
	*/
	
	(*(int*)countv)--;

	if (*(int*)countv == 0){
		printf("%d\n", *(int*)resultv);
		exit(EXIT_SUCCESS);
	}

	//printf("Result: %d .... Count: %d \n", *(int*)resultv, *(int*)countv);
	queue_enqueue(pending_read_queue, resultv, countv, handleOtherReads);
	disk_schedule_read(resultv, *(int*)resultv);
}

void handleFirstRead(void* resultv, void* countv) {
	if (*(int*)resultv == 0){
		printf("%d\n", *(int*)resultv);
		exit(EXIT_SUCCESS);
	}
	//printf("Got first one, it's: %d \n", *(int*)resultv);
	*(int*)countv = *(int*)resultv;

	queue_enqueue(pending_read_queue, resultv, countv, handleOtherReads);
	disk_schedule_read(resultv, *(int*) resultv);
}

int main(int argc, char** argv) {
	// Command Line Arguments
	static char* usage = "usage: treasureHunt starting_block_number";
	int starting_block_number;
	char *endptr;
	if (argc == 2)
		starting_block_number = strtol(argv[1], &endptr, 10);
	if (argc != 2 || *endptr != 0) {
		printf("argument error - %s \n", usage);
		return EXIT_FAILURE;
	}

	// Initialize
	uthread_init(1);
	disk_start(interrupt_service_routine);
	pending_read_queue = queue_create();
	// Start the Hunt
	int next_block;
	int count;
	queue_enqueue(pending_read_queue, &next_block, &count, handleFirstRead);
	disk_schedule_read(&next_block, starting_block_number);
	// TODO
	while (1); // inifinite loop so that main doesn't return before hunt completes
}
