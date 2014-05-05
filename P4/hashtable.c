#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include<limits.h>
#include<sys/time.h>
#include<sys/resource.h>


typedef struct kv_pair {
   int key;
   int value;
} * kv_pair;

kv_pair kv_create (int key, int value) {
  kv_pair kv = (kv_pair) malloc(sizeof(struct kv_pair));
  kv->key = key;
  kv->value = value;
  return kv;
}

typedef struct arraylist {
    kv_pair *buffer;     // pointer to allocated memory
    int buffer_size; // the max number of integers the buffer can hold
    int length;      // the number of integers stored in the list
} arraylist;

void resize_buffer(arraylist *a) {
   if (a->buffer==NULL) {
      a->buffer_size = 2;
      a->buffer = malloc(a->buffer_size*sizeof(kv_pair));
    }
    if (a->length > .75*a->buffer_size) {
      a->buffer_size = 2*a->buffer_size;
      a->buffer = realloc(a->buffer, a->buffer_size*sizeof(kv_pair));
    }
}

void arraylist_add(arraylist *a, kv_pair kv) {
    // Appending the value x to the end of the arraylist;
    resize_buffer(a);
    a->buffer[a->length] = kv;
    ++a->length;
}

void arraylist_insert(arraylist *a, int index, kv_pair kv) {
    // Storing the value x at the specified index in the arraylist.
    // Previously stored values should be moved back rather than overwritten.
    resize_buffer(a);
    int i;
    for(i=a->length-1; i >= index; i--) {
      a->buffer[i+1] = a->buffer[i];
    }
    a->buffer[index] = kv;
    ++a->length;
}

void arraylist_free(arraylist *a) {
    // freeing any memory used by that arraylist
    if (a->buffer) {
      int i;
      for(i=0; i < a->length; i++) {
         free(a->buffer[i]);
      }
      free(a->buffer);
    }
    free(a);
}

arraylist* arraylist_new() {
    arraylist *a = (arraylist *)malloc(sizeof(arraylist));
    a->buffer = NULL;
    a->buffer_size = 0;
    a->length = 0;

    return a;
}

void arraylist_remove(arraylist *a, int index) {
    free(a->buffer[index]);
    int i;
    for(i = index; i < a->length; i++) {
       a->buffer[i] = a->buffer[i+1];
    }
    --a->length;
}

kv_pair arraylist_get(arraylist *a, int index) {
    return(a->buffer[index]);
}

unsigned long hash (int* key) {
   unsigned long hash = 5381;
   char* str = (char*)key;
   int c;
   while ((c = *str++)) {
      hash = ((hash << 5) + hash) + c;
   }
   return hash;
}

typedef struct hashtable {
    arraylist **buffer;     // pointer to allocated memory
    int buffer_size; // the max number of integers the buffer can hold
    int length;      // the number of integers currently stored in the hash table
    int puts;		 // the number of integers put in the hash table
} hashtable;

void hashtable_free (hashtable *self) {
   int i;
   for(i=0; i < self->buffer_size; i++) {
      arraylist_free(self->buffer[i]); 
   }
   if (self->buffer) {
      free(self->buffer);
   }
}

void hashtable_remove (struct hashtable *self, int key) {
   int hashval = (int) (hash(&key) % self->buffer_size);
   arraylist *a = self->buffer[hashval];
   int index = arraylist_index(a, key);
   arraylist_remove(a, index);
   --self->length;
}

void hashtable_replace (struct hashtable *self, int key, int value, int old_size) {
   int old_hash = (int) (hash(&key) % old_size);
   int new_hash = (int) (hash(&key) % self->buffer_size);
   if (old_hash != new_hash) {
     arraylist *old = self->buffer[old_hash];
     arraylist *new = self->buffer[new_hash];
     int index = arraylist_index(old, key);
     arraylist_remove(old, index); 
     kv_pair kv = kv_create (key, value);
     arraylist_add(new, kv);
   }
   ++self->puts;
}   

void resize_hash_buffer(hashtable *self) {
   if (self->buffer==NULL) {
      self->buffer_size = 2;
      self->buffer = malloc(self->buffer_size*sizeof(arraylist*));
      int i;
      for(i = 0; i < self->buffer_size; i++) {
         self->buffer[i] = arraylist_new();
      }
    }
    else if (self->length > .75*self->buffer_size) {
      int old_size = self->buffer_size;
      self->buffer_size = 2*self->buffer_size;
      self->buffer = realloc(self->buffer, self->buffer_size*sizeof(arraylist*));
      int i;
      for(i = old_size; i < self->buffer_size; i++) {
         self->buffer[i] = arraylist_new();
      }
      for(i=0; i < self->length; i++) {
         int j;
         for(j=0; j < self->buffer[i]->length; j++) {
            kv_pair kv = self->buffer[i]->buffer[j];
            hashtable_replace(self, kv->key, kv->value, old_size);
         }
      }
    }
}

void hashtable_create (struct hashtable *self) {
   self->buffer = NULL;
   self->buffer_size = 0;
   self->length = 0;
   self->puts = 0;
}

bool hashtable_exists (struct hashtable *self, int key) {
   int hashval = (int) (hash(&key) % self->buffer_size);
   arraylist *a = self->buffer[hashval];
   int i;
   for(i = 0; i < a->length; i++) {
      if (a->buffer[i]->key == key) {
	 return true;
      }
   }
   return false;
}

void hashtable_put (struct hashtable *self, int key, int value) {
   resize_hash_buffer(self);
   int hashval = (int) (hash(&key) % self->buffer_size);
   arraylist *a = self->buffer[hashval]; 
   if (hashtable_exists (self, key)) {
      int i;
      for(i=0; i < a->length; i++) {
         if (a->buffer[i]->key == key) {
           a->buffer[i]->value = value;
         }
      }
   }
   else { 
      kv_pair kv = kv_create (key, value);
      arraylist_add(a, kv);
   } 
   ++self->length;
   ++self->puts;
}

int hashtable_get (struct hashtable *self, int key) {
   int hashval = (int) (hash(&key) % self->buffer_size);
   arraylist *a = self->buffer[hashval];
   int i;
   for(i=0; i < a->length; i++) {
      if (a->buffer[i]->key == key) {
         return a->buffer[i]->value;
      }
   }
   char str[80];
   sprintf(str, "Error: key %d not found", key);
   puts(str);
   exit(1);
}

int arraylist_index (struct arraylist *a, int key) {
   int i;
   for(i=0; i < a->length; i++) {
     if (a->buffer[i]->key == key) {
        return i;
     }
   }
   return -1;
}


void hashtable_stats (struct hashtable *self) {
   char str[80];
   sprintf(str, "length = %d, N = %d, puts = %d", self->length, self->buffer_size, self->puts);
   puts(str);
}

// Basic test
void test1(void) {
    struct hashtable a;

    hashtable_create(&a);
    hashtable_put(&a,13,37);
    if(hashtable_get(&a,13)!=37) {
        printf("test1 failed.\n");
        return;
    }
    printf("test1 passed.\n");
}

// Test insertions
void test2(void) {
    struct hashtable a;
    int i;

    hashtable_create(&a);

    for (i=0; i<10; i++) {
        hashtable_put(&a, i, i*2);
        if (hashtable_get(&a, i) != i*2) {
            printf("test2 failed.\n");
            return;
        }
    }

    for (i=0; i<10; i++) {
        if (hashtable_get(&a, i) != i*2) {
            printf("test2 failed.\n");
            return;
        }
    }
    printf("test2 passed.\n");
}

// Test insertions w/ deletions
void test3(void) {
    struct hashtable a;
    int i;

    hashtable_create(&a);

    for (i=0; i<100; i++) {
        hashtable_put(&a, i, i*2);
        if (hashtable_get(&a, i) != i*2) {
            printf("test3 failed.\n");
            return;
        }
        if (i%2) {
            hashtable_remove(&a, i);
        }
    }
    for (i=0; i<100; i++) {
        if ((i%2)==0) {
            if (hashtable_get(&a, i) != i*2) {
                printf("test3 failed.\n");
                return;
            }
        }
    }

    printf("test3 passed.\n");
}

// Collision test
void test4(void) {
    struct hashtable a;
    int i;

    hashtable_create(&a);
    hashtable_put(&a, 2001, 37);
    for (i=0; i < 1000; i++) {
        hashtable_put(&a, i, i*2);
        if (hashtable_get(&a, i) != i*2) {
            printf("test4 failed.\n");
            return;
        }
        hashtable_remove(&a, i);
    }
    if (hashtable_get(&a, 2001) != 37) {
        printf("test4 failed.\n");
        return;
    }
    printf("test4 passed.\n");
}

int main(int argc, char *argv[]) {
    //If no arguments, do all tests
    if(argc==1) {
        test1();
        test2();
        test3();
        test4();
    } else {
        switch(argv[1][0]) {
        case '0':
            test1();
            break;
        case '1':
            test2();
            break;
        case '2':
            test3();
            break;
        case '3':
            test4();
            break;
        default:
            printf("Invalid argument\n");
            return(1);
        }
    }
    return 0;
}

