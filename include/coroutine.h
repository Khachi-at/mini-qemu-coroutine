#ifndef COROUTINE_H
#define COROUTINE_H

#include <stdbool.h>

typedef struct _Coroutine Coroutine;

typedef void CoroutineEntry(void *opaque);

Coroutine *co_create(CoroutineEntry *entry, void *opaque);

void co_enter(Coroutine *co);

void co_yield (void);

Coroutine *co_self(void);

bool co_is_finished(Coroutine *co);

void co_destroy(Coroutine *co);

#endif