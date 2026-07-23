#include <stdio.h>
#include "coroutine.h"

static void worker(void *opaque)
{
    printf("worker step1\n");

    co_yield();

    printf("worker step2\n");

    co_yield();

    printf("worker step3\n");
}

int main(void)
{
    Coroutine *co = co_create(worker, NULL);

    while (!co_is_finished(co))
    {
        printf("enter coroutine\n");

        co_enter(co);

        printf("back to main\n");
    }

    co_destroy(co);

    return 0;
}