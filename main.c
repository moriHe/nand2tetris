#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int main() {

    typedef struct {
        const char *key;
        int value;
        bool used;
    } Entry;

    typedef struct {
        Entry *slots;
        size_t capacity;
        size_t size;
    } HashTable;

    static Entry slots[1024] = {0};
    static HashTable ht = {slots, 1024, 0};

    const char *test_str = "TEST";
    uint32_t  h32 = UINT32_C(2166136261);
    uint32_t prime = UINT32_C(16777619);
    size_t str_len = strlen(test_str);
    for (size_t i = 0; i < str_len; i++) {
        unsigned char uch = (unsigned char) test_str[i];
        h32 ^= uch;
        h32 = h32 * prime;
    }

    size_t index = h32 % ht.capacity;
    size_t counter = 0;
    while (true) {
    if (counter == ht.capacity) {
        fprintf(stderr, "Error: Hash Table is full. Can not add any more Labels.");
        return 1;
    }
    if (ht.slots[index].used == true) {
        if (strcmp(test_str, ht.slots[index].key) == 0) {
            fprintf(stderr, "Error: Duplicate Key detected.");
            return 1;
        }
        index = (index + 1) % ht.capacity;
        counter++;
        continue;
    }

    ht.slots[index].key = test_str;
    ht.slots[index].value = 10;
    ht.slots[index].used = true;
    ht.size = ht.size + 1;
    break;
    }


    return 0;
}