#ifndef CUSTOM_BOOLSET_STRUCT_AND_FUNCTIONS
#define CUSTOM_BOOLSET_STRUCT_AND_FUNCTIONS

/*
TODO: Split the boolean data into sections that have a byte-size
equal to the cache-line-size of the system. This may have a small performance impact on
smaller collections, but can increase performance for larger collections, which is the
purpose of this class.
*/

#ifndef BOOLSET_TYPE
#define BOOLSET_TYPE unsigned long
#endif

typedef struct boolset_t {
    // The size 'data' in whole primitives.
    BOOLSET_TYPE size;
    BOOLSET_TYPE* data;
} boolset_t;


/// @brief Creates a new instance of boolset_t
/// @return A new initialized instance of boolset_t
boolset_t boolset_create();



void boolset_destroy(boolset_t* b);



/*
Sets the boolean at the given 'index' in the submitted 'set'
to the value of 'val'. The boolean is set to 0 if val is equal to 0.
Otherwise, the boolean is set to 1.
*/
void boolset_set(boolset_t* set, BOOLSET_TYPE index, int val);



/*
Returns the value of the boolean at the given 'index' in the given 'set'.
*/
int boolset_get(const boolset_t* set, BOOLSET_TYPE index);



/* Calls the parameter function pointer 'callback' on each bit in the boolset 'set' that is true (1).
The index of the bit is given to the function as an argument.
*/
void boolset_foreach(const boolset_t* set, void(*callback)(BOOLSET_TYPE index, void* args), void* args);



#endif // CUSTOM_BOOLSET_STRUCT_AND_FUNCTIONS