/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef DAP_cJSON__h
#define DAP_cJSON__h
#include "dap_exports.hpp"

#include <memory>

/* cJsonDap Types: */
#define cJsonDap_False 0
#define cJsonDap_True 1
#define cJsonDap_Null 2
#define cJsonDap_Number 3
#define cJsonDap_String 4
#define cJsonDap_Array 5
#define cJsonDap_Object 6

#define cJsonDap_IsReference 256

namespace dap
{
/* The cJsonDap structure: */
typedef struct cJsonDap {
    struct cJsonDap *next, *prev; /* next/prev allow you to walk array/object chains. Alternatively, use
                                  GetArraySize/GetArrayItem/GetObjectItem */
    struct cJsonDap* child; /* An array or object item will have a child pointer pointing to a chain of the items in the
                            array/object. */

    int type; /* The type of the item, as above. */

    char* valuestring;  /* The item's string, if type==cJsonDap_String */
    int valueint;       /* The item's number, if type==cJsonDap_Number */
    double valuedouble; /* The item's number, if type==cJsonDap_Number */

    char*
        string; /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJsonDap;

typedef struct cJSONDap_Hooks {
    void* (*malloc_fn)(size_t sz);
    void (*free_fn)(void* ptr);
} cJSONDap_Hooks;

/* Supply malloc, realloc and free functions to cJsonDap */
WXDLLIMPEXP_DAP void cJSON_InitHooks(cJSONDap_Hooks* hooks);

/* Supply a block of Json, and this returns a cJsonDap object you can interrogate. Call cJSON_Delete when finished. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_Parse(const char* value);
/* Render a cJsonDap entity to text for transfer/storage. Free the char* when finished. */
WXDLLIMPEXP_DAP char* cJSON_Print(cJsonDap* item);
/* Render a cJsonDap entity to text for transfer/storage without any formatting. Free the char* when finished. */
WXDLLIMPEXP_DAP char* cJSON_PrintUnformatted(cJsonDap* item);
/* Delete a cJsonDap entity and all subentities. */
WXDLLIMPEXP_DAP void cJSON_Delete(cJsonDap* c);

/* Returns the number of items in an array (or object). */
WXDLLIMPEXP_DAP int cJSON_GetArraySize(cJsonDap* array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_GetArrayItem(cJsonDap* array, int item);
/* Get item "string" from object. Case insensitive. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_GetObjectItem(cJsonDap* object, const char* string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back
 * to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
WXDLLIMPEXP_DAP const char* cJSON_GetErrorPtr();

/* These calls create a cJsonDap item of the appropriate type. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateNull();
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateTrue();
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateFalse();
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateBool(int b);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateNumber(double num);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateString(const char* string);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateArray();
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateObject();

/* These utilities create an Array of count items. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateIntArray(int* numbers, int count);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateFloatArray(float* numbers, int count);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateDoubleArray(double* numbers, int count);
WXDLLIMPEXP_DAP cJsonDap* cJSON_CreateStringArray(const char** strings, int count);

/* Append item to the specified array/object. */
WXDLLIMPEXP_DAP void cJSON_AddItemToArray(cJsonDap* array, cJsonDap* item);
WXDLLIMPEXP_DAP void cJSON_AddItemToObject(cJsonDap* object, const char* string, cJsonDap* item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJsonDap to a new
 * cJsonDap, but don't want to corrupt your existing cJsonDap. */
WXDLLIMPEXP_DAP void cJSON_AddItemReferenceToArray(cJsonDap* array, cJsonDap* item);
WXDLLIMPEXP_DAP void cJSON_AddItemReferenceToObject(cJsonDap* object, const char* string, cJsonDap* item);

/* Remove/Detatch items from Arrays/Objects. */
WXDLLIMPEXP_DAP cJsonDap* cJSON_DetachItemFromArray(cJsonDap* array, int which);
WXDLLIMPEXP_DAP void cJSON_DeleteItemFromArray(cJsonDap* array, int which);
WXDLLIMPEXP_DAP cJsonDap* cJSON_DetachItemFromObject(cJsonDap* object, const char* string);
WXDLLIMPEXP_DAP void cJSON_DeleteItemFromObject(cJsonDap* object, const char* string);

/* Update array items. */
WXDLLIMPEXP_DAP void cJSON_ReplaceItemInArray(cJsonDap* array, int which, cJsonDap* newitem);
WXDLLIMPEXP_DAP void cJSON_ReplaceItemInObject(cJsonDap* object, const char* string, cJsonDap* newitem);

#define cJSON_AddNullToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddNumberToObject(object, name, n) cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object, name, s) cJSON_AddItemToObject(object, name, cJSON_CreateString(s))
}; // namespace dap
#endif
