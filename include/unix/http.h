#ifndef HTTP_CUSTOM_IMPLEMENTATION
#define HTTP_CUSTOM_IMPLEMENTATION



#ifndef HEADER_KEY_MAX_LENGTH
#define HEADER_KEY_MAX_LENGTH 64
#endif

#ifndef HEADER_VALUE_MAX_LENGTH
#define HEADER_VALUE_MAX_LENGTH 64
#endif



/* Represents one of the following http request methods:
GET - Requests a representation of the specified resource. Requests using GET should only retrieve data.
HEAD - Asks for a response identical to a GET request, but without the response body.
POST - Submits an entity to the specified resource, often causing a change in state or side effects on the server.
PUT - Replaces all current representations of the target resource with the request payload.
DELETE - Deletes the specified resource.
CONNECT - Establishes a tunnel to the server identified by the target resource.
OPTIONS - Describes the communication options for the target resource.
TRACE - Performs a message loop-back test along the path to the target resource.
PATCH - Applies partial modifications to a resource.
*/
enum http_method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };



/* Represents a specific http protocol version. */
enum http_version { HTTP_1_0, HTTP_1_1, HTTP_2_0, HTTP_3_0 };



typedef struct http_header {
    char key[HEADER_KEY_MAX_LENGTH]; // Null-terminated string
    char value[HEADER_VALUE_MAX_LENGTH]; // Null-terminated string
} http_header;



typedef struct http_request {
    enum http_method request; // The request method, e.g. GET or POST.
    enum http_version protocol_version; // The http protocol version to be used, e.g. HTTP/1.1

    char* path; // A null-terminated string containing the requested path.
    
    http_header* headers; // An array of http headers to be included in the request.
    int header_count; // The array-size of 'headers', corresponding to the amount of headers in this request.

    char* data; // The data to be sent at the end of the request.
    unsigned long data_len; // The size (in chars) of 'data'.
} http_request;



typedef struct http_response {
    enum http_method request; // The request method, e.g. GET or POST.
    enum http_version protocol_version; // The http protocol version to be used, e.g. HTTP/1.1

    char* path; // A null-terminated string containing the requested path.
    
    http_header* headers; // An array of http headers to be included in the request.
    int header_count; // The array-size of 'headers', corresponding to the amount of headers in this request.

    char* data; // The data to be sent at the end of the request.
    unsigned long data_len; // The size (in chars) of 'data'.
} http_response;



/// @brief Deallocate the dynamically allocated parts of the submitted http_request.
/// @param request
void http_request_destroy(http_request request);

/// @brief Deallocate the dynamically allocated parts of the submitted http_response.
/// @param response
void http_response_destroy(http_response response);

/// @brief Convert a string into a http_request.
/// @param dst 
/// @param request 
/// @param str_len 
/// @return 
int http_str_to_req(http_request* dst, char* request, unsigned long str_len);

/// @brief Converts a http_response into a string.
/// @param dst 
/// @param response 
/// @param max_str_len 
/// @return 
int http_res_to_str(char* dst, http_response* response, unsigned long max_str_len);

/// @brief Calculate the memory size required to store a string representation of the submitted http_response.
/// @param  
/// @return 
unsigned long http_res_str_len(http_response);






// #### HEADER_SECTION ####



int http_header_set(http_header* dst, const char* key, const char* value);

/// @brief Deallocates a http_header.
/// @param header
// void http_header_destroy(http_header header);

/// @brief Converts a http_header into a string.
/// @param dst 
/// @param str A null-terminated string containing a header on the form: key: value
/// @param str_len 
/// @return 0 if success, or -1 if the string could no be parsed.
int http_str_to_header(http_header* dst, const char* str, unsigned long str_len);

/// @brief Converts a http_header into a string.
/// @param dst 
/// @param response 
/// @param max_str_len 
/// @return Number of characters written to 'dst', or -1 if the resulting string is longer than 'max_str_len'.
int http_header_to_str(char* dst, http_header response, unsigned long max_str_len);

int http_count_headers(const char* str, unsigned long str_len);

int http_str_to_headers(http_header* dst, int dst_size, const char* str, unsigned long str_len);



#endif // HTTP_CUSTOM_IMPLEMENTATION