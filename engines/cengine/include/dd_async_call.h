#ifndef DD_ASYNC_CALL_H
#define DD_ASYNC_CALL_H

// handle async calls
struct dd_async_call {
	void (*callback)(void *context);
	void *context;
	int isComplete;
};

#endif
