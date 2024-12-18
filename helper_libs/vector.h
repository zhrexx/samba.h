// ========================================================================================>
// ZHRXXgroup Project 🚀 - ZLib
// File: vector.h
// Author(s): ZHRXXgroup
// Version: 1
// ========================================================================================>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    void *data;
    size_t size;
    size_t capacity;
    size_t element_size;
} Vector;

void vector_init(Vector *vector, size_t initial_capacity, size_t element_size);
void vector_resize(Vector *vector, size_t new_capacity);
void vector_push(Vector *vector, const void *value);
void *vector_get(Vector *vector, size_t index);
void vector_set(Vector *vector, size_t index, const void *value);
bool vector_contains(Vector *vector, const void *value);
void vector_remove(Vector *vector, size_t index);
ssize_t vector_find(Vector *vector, const void *value);
void vector_compress(Vector *vector);
void vector_copy(Vector *dest, const Vector *src);
void vector_free(Vector *vector);

//----------------------------------------------//
//----------------------------------------------//

void vector_init(Vector *vector, size_t initial_capacity, size_t element_size) {
    if (initial_capacity == 0) initial_capacity = 1;
    vector->data = malloc(initial_capacity * element_size);
    if (!vector->data) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    vector->size = 0;
    vector->capacity = initial_capacity;
    vector->element_size = element_size;
}

void vector_resize(Vector *vector, size_t new_capacity) {
    if (new_capacity == 0) new_capacity = 1;
    void *new_data = realloc(vector->data, new_capacity * vector->element_size);
    if (!new_data) {
        fprintf(stderr, "Failed to reallocate memory\n");
        exit(EXIT_FAILURE);
    }
    vector->data = new_data;
    vector->capacity = new_capacity;
}

void vector_push(Vector *vector, const void *value) {
    if (vector->size == vector->capacity) {
        vector_resize(vector, vector->capacity == 0 ? 1 : vector->capacity * 2);
    }
    void *target = (char *)vector->data + (vector->size * vector->element_size);
    memcpy(target, value, vector->element_size);
    vector->size++;
}

void *vector_get(Vector *vector, size_t index) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return (char *)vector->data + (index * vector->element_size);
}

void vector_set(Vector *vector, size_t index, const void *value) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    void *target = (char *)vector->data + (index * vector->element_size);
    memcpy(target, value, vector->element_size);
}

bool vector_contains(Vector *vector, const void *value) {
    for (size_t i = 0; i < vector->size; i++) {
        void *current_element = (char *)vector->data + (i * vector->element_size);

        if (vector->element_size == sizeof(char *)) {
            const char *str1 = *(const char **)current_element;
            const char *str2 = (const char *)value;
            if (strcmp(str1, str2) == 0) {
                return true;
            }
        } else {
            if (memcmp(current_element, value, vector->element_size) == 0) {
                return true;
            }
        }
    }
    return false;
}

void vector_remove(Vector *vector, size_t index) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    void *target = (char *)vector->data + (index * vector->element_size);
    void *next_element = (char *)vector->data + ((index + 1) * vector->element_size);
    memmove(target, next_element, (vector->size - index - 1) * vector->element_size);
    vector->size--;
}

#define VECTOR_FOR_EACH(type, element, vector) \
    for (type *element = (type *)(vector)->data; \
         (char *)element < (char *)(vector)->data + (vector)->size * (vector)->element_size; \
         element++)

ssize_t vector_find(Vector *vector, const void *value) {
    for (size_t i = 0; i < vector->size; i++) {
        void *current_element = (char *)vector->data + (i * vector->element_size);
        if (memcmp(current_element, value, vector->element_size) == 0) {
            return i;
        }
    }
    return -1;  // Not found
}

void vector_compress(Vector *vector) {
	if (vector->capacity > vector->size) {
        vector_resize(vector, vector->size);
    }
}


void vector_copy(Vector *dest, const Vector *src) {
    vector_init(dest, src->capacity, src->element_size);
    memcpy(dest->data, src->data, src->size * src->element_size);
    dest->size = src->size;
}




void vector_free(Vector *vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    vector->element_size = 0;
}
