#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    void *data;
    size_t size, capacity, element_size;
} Vector;

void vector_init(Vector *vector, size_t initial_capacity, size_t element_size) {
    vector->capacity = initial_capacity ? initial_capacity : 1;
    vector->element_size = element_size;
    vector->size = 0;
    vector->data = malloc(vector->capacity * element_size);
    if (!vector->data) exit(EXIT_FAILURE);
}

void vector_resize(Vector *vector, size_t new_capacity) {
    vector->capacity = new_capacity ? new_capacity : 1;
    vector->data = realloc(vector->data, vector->capacity * vector->element_size);
    if (!vector->data) exit(EXIT_FAILURE);
}

void vector_push(Vector *vector, const void *value) {
    if (vector->size == vector->capacity) vector_resize(vector, vector->capacity * 2);
    memcpy((char *)vector->data + vector->size++ * vector->element_size, value, vector->element_size);
}

void *vector_get(Vector *vector, size_t index) {
    if (index >= vector->size) exit(EXIT_FAILURE);
    return (char *)vector->data + index * vector->element_size;
}

void vector_set(Vector *vector, size_t index, const void *value) {
    if (index >= vector->size) exit(EXIT_FAILURE);
    memcpy((char *)vector->data + index * vector->element_size, value, vector->element_size);
}

bool vector_contains(Vector *vector, const void *value) {
    for (size_t i = 0; i < vector->size; i++) {
        if (memcmp((char *)vector->data + i * vector->element_size, value, vector->element_size) == 0) return true;
    }
    return false;
}

void vector_remove(Vector *vector, size_t index) {
    if (index >= vector->size) exit(EXIT_FAILURE);
    memmove((char *)vector->data + index * vector->element_size,
            (char *)vector->data + (index + 1) * vector->element_size,
            (--vector->size - index) * vector->element_size);
}

ssize_t vector_find(Vector *vector, const void *value) {
    for (size_t i = 0; i < vector->size; i++) {
        if (memcmp((char *)vector->data + i * vector->element_size, value, vector->element_size) == 0) return i;
    }
    return -1;
}

void vector_compress(Vector *vector) {
    if (vector->capacity > vector->size) vector_resize(vector, vector->size);
}

void vector_copy(Vector *dest, const Vector *src) {
    vector_init(dest, src->capacity, src->element_size);
    memcpy(dest->data, src->data, src->size * src->element_size);
    dest->size = src->size;
}

void vector_clear(Vector *vector) {
    vector->size = 0;
}

void vector_shrink_to_fit(Vector *vector) {
    vector_resize(vector, vector->size);
}

void vector_swap(Vector *vector, size_t index1, size_t index2) {
    if (index1 >= vector->size || index2 >= vector->size) exit(EXIT_FAILURE);
    char *temp = malloc(vector->element_size);
    if (!temp) exit(EXIT_FAILURE);
    char *ptr1 = (char *)vector->data + index1 * vector->element_size;
    char *ptr2 = (char *)vector->data + index2 * vector->element_size;
    memcpy(temp, ptr1, vector->element_size);
    memcpy(ptr1, ptr2, vector->element_size);
    memcpy(ptr2, temp, vector->element_size);
    free(temp);
}

void vector_extend(Vector *dest, const Vector *src) {
    if (dest->element_size != src->element_size) exit(EXIT_FAILURE);
    for (size_t i = 0; i < src->size; i++) {
        vector_push(dest, (char *)src->data + i * src->element_size);
    }
}

Vector vector_filter(const Vector *vector, bool (*predicate)(const void *)) {
    Vector result;
    vector_init(&result, vector->capacity, vector->element_size);
    for (size_t i = 0; i < vector->size; i++) {
        void *element = (char *)vector->data + i * vector->element_size;
        if (predicate(element)) vector_push(&result, element);
    }
    return result;
}

void vector_map(Vector *vector, void (*transform)(void *)) {
    for (size_t i = 0; i < vector->size; i++) {
        transform((char *)vector->data + i * vector->element_size);
    }
}

void *vector_reduce(const Vector *vector, void *initial, void *(*reduce)(void *, const void *)) {
    void *result = initial;
    for (size_t i = 0; i < vector->size; i++) {
        result = reduce(result, (char *)vector->data + i * vector->element_size);
    }
    return result;
}

Vector vector_clone(const Vector *src) {
    Vector clone;
    vector_copy(&clone, src);
    return clone;
}

void vector_free(Vector *vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = vector->capacity = vector->element_size = 0;
}

#define VECTOR_FOR_EACH(type, element, vector) \
    for (type *element = (type *)(vector)->data; \
         (char *)element < (char *)(vector)->data + (vector)->size * (vector)->element_size; \
         element++)
