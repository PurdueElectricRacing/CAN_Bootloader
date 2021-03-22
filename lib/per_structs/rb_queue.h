/**
 * @file RB_queue.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Queue implemented with a ring buffer and type-wrappers for generic datatypes.
 * @version 0.1
 * @date 2021-02-18
 * 
 */

#ifndef RB_QUEUE_H
#define RB_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief  Define the underlying data storage element. 32 bit is chosen to match MCU archetecture
 * 
 */
#define RB_QUEUE_DTYPE uint32_t 

#define GET_ELEMENT_INDEX  

/**
 * @brief Basic structure for holding a ring buffer based queue.
 * Only via a macro-defined init_queue_<type> as defined with the @ref create_queue_type macro
 */
typedef struct {
    uint8_t* elements;          ///< List for buffer storage. Must be provided
    uint32_t capacity;          ///< Size of the provided list     
    uint32_t element_size;      ///< Size of individual element in list

    uint32_t _size;             ///< Current number of elementes stored in queue
    uint32_t _head;             ///< Index of first item in queue    
    uint32_t _tail;             ///< Index of last item in queue
} rb_queue_t;

void queue_memcpy(void *dest, void *src, size_t n);

void initRBQueue(rb_queue_t* q, uint8_t* elements, uint32_t capacity, uint32_t element_size);
bool isRBQueueEmpty(rb_queue_t* q);
bool isRBQueueFull(rb_queue_t* q);
bool rbEnqueue(rb_queue_t* q, void* element);
bool rbDequeue(rb_queue_t* q, void* dest);
bool rbPeek(rb_queue_t* q, void* dest);

#endif