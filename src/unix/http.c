#include "unix/http.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



void http_request_destroy(http_request request) {
    free(request.data);
    free(request.headers);
    free(request.path);
}

void http_response_destroy(http_response response) {
    free(response.data);
    free(response.headers);
    free(response.path);
}





// #### HEADER_SECTION ####


int http_header_set(http_header* dst, const char* key, const char* value) {
    const size_t key_len = strlen(key) * sizeof(char);
    const size_t value_len = strlen(key) * sizeof(char);

    strcpy(dst->key, key);
    strcpy(dst->value, value);

    return 0;
}

// void http_header_destroy(http_header header) {
//     free(header.key);
//     free(header.value);
// }


int http_str_to_header(http_header* dst, const char* str, const unsigned long str_len) {
    int colon = 0;
    while (str[colon] != ':' && colon < str_len) // Find the position of the colon
        ++colon;

    if (colon > str_len)
        return -1;

    int key_start = 0;
    int key_end = colon - 1;

    while (str[key_start] == ' ')
        ++key_start;
    while (str[key_end] == ' ')
        --key_end;
    

    
    int val_start = colon + 1;
    int val_end = str_len - 1;

    while (str[val_start] == ' ')
        ++val_start;
    while (str[val_end] == ' ')
        --val_end;
    


    const int key_len = key_end - key_start + 1;
    const int val_len = val_end - val_start + 1;

    if (key_len + 1 > HEADER_KEY_MAX_LENGTH ||
        val_len + 1 > HEADER_VALUE_MAX_LENGTH)
        return -1;

    memcpy(dst->key,   &str[key_start], key_len);
    memcpy(dst->value, &str[val_start], val_len);

    dst->key[key_len]   = '\0';
    dst->value[val_len] = '\0';

    return 0;
}

int http_header_to_str(char* dst, const http_header header, unsigned long max_str_len) {
    const int key_len = strlen(header.key);
    const int val_len = strlen(header.value);

    int total_length = key_len + val_len + 3;

    if (total_length > max_str_len)
        return -1;
    
    memcpy(dst, header.key, key_len);
    dst[key_len + 0] = ':';
    dst[key_len + 1] = ' ';
    memcpy(&dst[key_len + 2], header.value, val_len);
    dst[total_length - 1] = '\0';
}



int http_count_headers(const char* str, size_t str_len) {
    
}

int http_str_to_headers(http_header* dst, int dst_size, const char* str, size_t str_len) {
    int header_index = 0;

    int header_start = 0;
    int header_end = 0;
    while (str[header_end] != '\r' && header_end < str_len)
        ++header_end;
    

    while (header_start < str_len && header_index < dst_size) {
        int err = http_str_to_header(&dst[header_index], &str[header_start], header_end - header_start);

        header_index += err == 0;

        header_end += 2;
        header_start = header_end;
        while (str[header_end] != '\r' && header_end <= str_len)
            ++header_end;
    }

    return header_index;
}




