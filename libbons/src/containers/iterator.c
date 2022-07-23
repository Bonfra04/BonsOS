#include <containers/iterator.h>

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

iterator_t iterator(void (*next)(iterator_t* it), void (*prev)(iterator_t* it), void* position, void* structure_data)
{
    iterator_t it;
    it.position = position;
    it.next = next;
    it.prev = prev;
    it.structure_data = structure_data;
    return it;
}

void* __iterator_get(const iterator_t* it)
{
    return it->position;
}

void iterator_advance(iterator_t* it, int64_t n)
{
    if (n > 0)
        while (n--)
            it->next(it);
    else if (n < 0)
        while (n++)
            it->prev(it);
}

uint64_t iterator_distance(const iterator_t* first, const iterator_t* last)
{
    assert(first->structure_data == last->structure_data);

    uint64_t n = 0;
    iterator_t it = *first;
    while (it.position != last->position)
    {
        it.next(&it);
        n++;
    }
    return n;
}

iterator_t iterator_prev(const iterator_t* it, uint64_t n)
{
    iterator_t it_new = *it;
    while (n--)
        it_new.prev(&it_new);
    return it_new;
}

iterator_t iterator_next(const iterator_t* it, uint64_t n)
{
    iterator_t it_new = *it;
    while (n--)
        it_new.next(&it_new);
    return it_new;
}