/**
 * @file rb_queue.c
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Queue implemented with a ring buffer and type-wrappers for generic datatypes.
 * @version 0.1
 * @date 2021-02-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <rb_queue.h>


/**
 * @brief Simple memcpy implementation. I dont want to include stdlib
 * 
 * @param dest Memory copy destination
 * @param src Memoy copy source
 * @param n Number of btes to copy
 */
void queue_memcpy(void *dest, void *src, size_t n)
{
    for(int i = 0; i < n; i++)
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
}

/**
 * @brief Initalize an empty rb_queue
 * 
 * @param q Handle to queue struct to be initalized
 * @param elements Handle to pre-allocated array of elements to provide a queue interface on top of. Can be of any type
 * @param capacity Number of elements in the allocated array
 * @param element_size Size (bytes) of a single element
 */
void initRBQueue(rb_queue_t* q, uint8_t* elements, uint32_t capacity, uint32_t element_size)
{
    q->elements = elements;
    q->element_size = element_size;
    q->capacity = capacity;
    q->_size = 0;
    q->_head = 1;
    q->_tail = 0;
}

/**
 * @brief Check if queue is empty
 * 
 * @param q Queue to check
 * @return true queue has no elements in it
 * @return false queue has at least one element in it
 */
bool isRBQueueEmpty(rb_queue_t* q)
{
    return q->_size == 0;
}

/**
 * @brief Check if queue is at capacity
 * 
 * @param q queue to check
 * @return true queue is at capacity
 * @return false queue has space for another item
 */
bool isRBQueueFull(rb_queue_t* q)
{
    return q->_size == q->capacity;
}

/**
 * @brief Add a element to the end of the queue if space is available WARNING -> Ensure that *element is the same type as q->elements[0]
 * 
 * @param q queue to add element to
 * @param element Handle to the element to be added
 * @return true Element sucessfully added
 * @return false Queue does not exist or is full
 */
bool rbEnqueue(rb_queue_t* q, void* element)
{
    if(!q || isRBQueueFull(q))
        return false;

    q->_tail = (q->_tail + 1) % q->capacity;
    queue_memcpy( &(q->elements[q->_tail * q->element_size]), element, q->element_size);
    q->_size++;
    
    return true;
}

/**
 * @brief Remove an element from the queue. WARNING -> Ensure that *dest is the same type as q->elements[0]
 * 
 * @param q queue to remove element from
 * @param dest Where to copy element to
 * @return true Element copied sucessfully
 * @return false queue was already empty
 */
bool rbDequeue(rb_queue_t* q, void* dest)
{
    if (!q || isRBQueueEmpty(q))
        return false;

    queue_memcpy(dest, &(q->elements[q->_head * q->element_size]) , q->element_size);
    q->_head = (q->_head + 1) % q->capacity;
    q->_size --;

    return true;
}

/**
 * @brief Look at first element in queue without removing it. WARNING -> Ensure that *dest is the same type as q->elements[0]
 * 
 * @param q queue to check
 * @param dest Where to copy element to
 * @return true Element copied sucessfully
 * @return false queue is empty
 */
bool rbPeek(rb_queue_t* q, void* dest)
{
    if (!q || isRBQueueEmpty(q))
        return false;

    queue_memcpy(dest, &(q->elements[q->_head * q->element_size]) , q->element_size);
    return true;
}