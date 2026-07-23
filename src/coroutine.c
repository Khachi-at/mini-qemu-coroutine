#include <stdlib.h>
#include <ucontext.h>
#include "coroutine.h"

#define CO_STACK_SIZE (6 * 1024)

typedef enum
{
    CO_NEW,
    CO_RUNNING,
    CO_SUSPEND,
    CO_DEAD,
} CoroutineState;

struct _Coroutine
{
    ucontext_t ctx;

    void *stack;
    size_t stack_size;

    CoroutineEntry *entry;
    void *opaque;

    struct _Coroutine *caller;

    CoroutineState state;
};

static Coroutine *current = NULL;

/*
 * main coroutine
 */
static Coroutine main_co;

static void coroutine_trampoline(void)
{
    Coroutine *self = current;

    self->entry(self->opaque);

    self->state = CO_DEAD;

    /*
     *return to caller
     */
    Coroutine *caller = self->caller;

    current = caller;
    setcontext(&caller->ctx);

    abort();
}

static void runtime_init(void)
{
    static int initialized;

    if (initialized)
    {
        return;
    }

    initialized = 1;

    getcontext(&main_co.ctx);

    main_co.stack = NULL;
    main_co.stack_size = 0;
    main_co.entry = NULL;
    main_co.opaque = NULL;
    main_co.caller = NULL;
    main_co.state = CO_RUNNING;

    current = &main_co;
}

Coroutine *co_create(CoroutineEntry *entry, void *opaque)
{
    runtime_init();

    Coroutine *co = calloc(1, sizeof(*co));

    if (!co)
    {
        return NULL;
    }

    co->stack_size = CO_STACK_SIZE;
    co->stack = malloc(co->stack_size);

    if (!co->stack)
    {
        free(co);
        return NULL;
    }

    co->entry = entry;
    co->opaque = opaque;

    co->state = CO_NEW;

    getcontext(&co->ctx);

    co->ctx.uc_stack.ss_sp = co->stack;
    co->ctx.uc_stack.ss_size = co->stack_size;
    co->ctx.uc_link = NULL;

    makecontext(&co->ctx, coroutine_trampoline, 0);

    return co;
}

void co_enter(Coroutine *co)
{
    runtime_init();

    if (!co)
    {
        return;
    }

    if (co->state == CO_DEAD)
    {
        return;
    }

    Coroutine *prev = current;

    co->caller = prev;

    current = co;

    co->state = CO_RUNNING;
    swapcontext(&prev->ctx, &co->ctx);
}

void co_yield(void)
{
    Coroutine *self = current;

    if (!self)
    {
        return;
    }

    Coroutine *caller = self->caller;

    if (!caller)
    {
        return;
    }

    self->state = CO_SUSPEND;

    current = caller;
    swapcontext(&self->ctx, &caller->ctx);
}

Coroutine *co_self(void)
{
    runtime_init();

    return current;
}

bool co_is_finished(Coroutine *co)
{
    return co && co->state == CO_DEAD;
}

void co_destroy(Coroutine *co)
{
    if (!co)
    {
        return;
    }

    if (co == &main_co)
    {
        return;
    }

    free(co->stack);
    free(co);
}