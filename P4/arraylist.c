#include "kernel.h"

#define NULL 0

typedef struct arraylist {
    int *buffer;     // pointer to allocated memory
    int buffer_size; // the max number of integers the buffer can hold
    int length;      // the number of integers stored in the list
} arraylist;



////////////////////////////////////////////////////
//Functions that you need to implement:

void arraylist_add(arraylist *a, int x) {
    // Appending the value x to the end of the arraylist
    if (a->buffer == NULL) {
        a->buffer_size = 2;
        a->buffer = (int *)malloc(a->buffer_size*sizeof(int));
    }
    if (a->buffer_size == a->length) {
        a->buffer_size = a->buffer_size*2;
        int* temp = (int *)malloc(a->buffer_size*sizeof(int));
        for (int i = 0; i < a->length; ++i)
            temp[i] = a->buffer[i];
        a->buffer = temp;
    }
    a->buffer[a->length] = x;
    ++a->length;
}

void arraylist_insert(arraylist *a, int index, int x) {
    // Storing the value x at the specified index in the arraylist.
    // Previously stored values should be moved back rather than overwritten.
    
    // if index out of bounds, just return
    if (index > a->length)
        return;

    // take care of allocation/reallocation of memory
    if (a->buffer == NULL) {
        a->buffer_size = 2;
        a->buffer = (int *)malloc(a->buffer_size*sizeof(int));
    }
    if (a->buffer_size == a->length) {
        a->buffer_size = a->buffer_size*2;
        int* temp = (int *)malloc(a->buffer_size*sizeof(int));
        for (int i = 0; i < a->length; ++i)
            temp[i] = a->buffer[i];
        a->buffer = temp;
    }

    // move all elements to make space for insertion
    int i,temp,val;
    val = a->buffer[index];
    for(i = (index+1); i <= a->length; i++) {
        temp = a->buffer[i];
        a->buffer[i] = val;
        val = temp;
    }

    // insert element
    a->buffer[index] = x;
    ++a->length;    
}

void arraylist_free(arraylist *a) {
    // freeing any memory used by that arraylist
    if (a->buffer != NULL)
        free(a->buffer);
    free(a);
}

////////////////////////////////////////////////////



arraylist* arraylist_new() {
    arraylist *a = (arraylist *)malloc(sizeof(arraylist));
    a->buffer = NULL;
    a->buffer_size = 0;
    a->length = 0;

    return a;
}

void arraylist_remove(arraylist *a, int index) {
    int i;
    for(i = index; i < a->length-1; i++)
        a->buffer[i] = a->buffer[i+1];
    
    --a->length; 
}

int arraylist_get(arraylist *a, int index) {
    return(a->buffer[index]); 
}

void arraylist_print(arraylist *a) {
    printf("[");
    if (a->length > 0) {
        int i;
        for(i = 0; i < a->length-1; i++)
            printf("%d, ",arraylist_get(a,i));
        printf("%d", arraylist_get(a,a->length-1));
    }

    printf("]\n");
}

int main(int argc, char *argv[]) {
    // START OF TEST
    int i;
    arraylist *a = arraylist_new();

    arraylist_add(a, 0);
    arraylist_add(a, 1);
    arraylist_add(a, 2);
    arraylist_print(a);
    
    for (i = 0; i < a->length + 1; i++) {
        arraylist_insert(a, i, 100);
        printf("Insert position %d: ", i);
        arraylist_print(a);
        arraylist_remove(a, i);
    }
    printf("Clean: ");
    arraylist_print(a);

    arraylist_free(a);
    // END OF TEST

    return 0;
}
