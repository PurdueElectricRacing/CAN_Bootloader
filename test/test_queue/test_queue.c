
#include <unity.h>
#include <rb_queue.h>
#include <stdio.h>

/**
 * @brief Simple u32 queue to enqueue & dequeue 10 u32.
 * 
 */
void testQueue_u32(void)
{
    uint32_t items [10] = {0};
    uint32_t temp = 99;
    rb_queue_t q;

    initRBQueue(&q, (uint8_t*) items, 10, sizeof(uint32_t));

    // Enqueue 10 items
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT(rbEnqueue(&q, &i) == true);
    }

    // Attempt to overflow queue
    TEST_ASSERT_MESSAGE(rbEnqueue(&q, &temp) == false, "Can't overflow queue");

    // Dequeue and check that FIFO order is preserved
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT_MESSAGE(rbDequeue(&q, &temp) == true, "Dequeue items");
        TEST_ASSERT_MESSAGE(temp == i, "FIFO Order");
    }

    // Check that you can remove all items
    TEST_ASSERT_MESSAGE(rbDequeue(&q, &temp) == false, "Completley dequeue all items");

}

/**
 * @brief Test creating a queue with a struct as the queue element. Assertions will check that each element was properly copied
 *  out of the queue.
 *  
 */
void testQueue_struct(void)
{   
    // Define struct with different datatypes to test memory copy
    typedef struct  {
        uint16_t a;
        uint32_t b;
        uint8_t  c: 4;
        uint8_t  d: 4;
    } queue_item_t;

    queue_item_t items [10] = {0};
    
    // Model item that will be enqueued to all spots in queue
    queue_item_t model_item = {
        .a = 0xa,
        .b = 0xb,
        .c = 0xc,
        .d = 0xd
    };
    // Dummy item used to compare to model
    queue_item_t test_item;

    // Initalize RB queue to hold 10 queue_item_t
    rb_queue_t q;
    initRBQueue(&q, (uint8_t*) items, 10, sizeof(queue_item_t));

    // Enqueue 10 items
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT(rbEnqueue(&q, &model_item) == true);
    }

    // Attempt to overflow queue
    TEST_ASSERT_MESSAGE(rbEnqueue(&q, &model_item) == false, "Can't overflow queue");

    // Dequeue and check that FIFO order is preserved & struct elements are properly copied 
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT_MESSAGE(rbDequeue(&q, &test_item) == true,        "Dequeue items");
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(model_item.a, test_item.a,  "Struct copy u32");
        TEST_ASSERT_EQUAL_HEX16_MESSAGE(model_item.b, test_item.b,  "Struct copy u16");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(model_item.c, test_item.c,   "Struct copy u8 bitfield");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(model_item.d, test_item.d,   "Struct copy u8 bitfield");
    }

    // Check that you can remove all items
    TEST_ASSERT_MESSAGE(rbDequeue(&q, &test_item) == false, "Completley dequeue all items");

}


int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(testQueue_u32);
    RUN_TEST(testQueue_struct);

    return UNITY_END();
}
