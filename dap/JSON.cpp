/*
  Copyright (c) 2020 Eran Ifrah

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

#include "JSON.hpp"
#define CHECK_VALID_THIS_RETURN_NULL() \
    if(!m_cjson) {                     \
        return JSON(nullptr);          \
    }

#define CHECK_IS_CONTAINER()        \
    CHECK_VALID_THIS_RETURN_NULL()  \
    if(!IsArray() && !IsObject()) { \
        return JSON(m_cjson);       \
    }

JSON::JSON(cJSON* ptr)
    : m_cjson(ptr)
{
}

JSON::~JSON() { m_cjson = nullptr; }

JSON JSON::operator[](const string& index) const
{
    if(m_cjson == nullptr) {
        return JSON(nullptr);
    }

    cJSON* child = m_cjson->child;
    while(child) {
        if(child->string && strcmp(child->string, index.c_str()) == 0) {
            return JSON(child);
        }
        child = child->next;
    }
    return JSON(nullptr);
}

JSON JSON::AddItem(const string& name, cJSON* item)
{
    if(m_cjson == nullptr) {
        cJSON_Delete(item);
        return JSON(nullptr);
    }
    if(m_cjson->type != cJSON_Array && m_cjson->type != cJSON_Object) {
        cJSON_Delete(item);
        return JSON(nullptr);
    }
    if(m_cjson->type == cJSON_Array) {
        cJSON_AddItemToArray(m_cjson, item);
    } else {
        cJSON_AddItemToObject(m_cjson, name.c_str(), item);
    }
    return JSON(item);
}

string JSON::ToString() const
{
    if(m_cjson == nullptr) {
        return "";
    }
    char* c = cJSON_Print(m_cjson);
    string str(c);
    free(c);
    return str;
}

JSON JSON::CreateArray()
{
    JSON arr(cJSON_CreateArray());
    return arr;
}

JSON JSON::CreateObject()
{
    JSON obj(cJSON_CreateObject());
    return obj;
}

void JSON::Delete()
{
    // Delete only when owned
    if(m_cjson) {
        cJSON_Delete(m_cjson);
        m_cjson = nullptr;
    }
}

JSON JSON::Add(const char* name, const string& value) { return Add(name, value.c_str()); }

JSON JSON::Add(const char* name, const char* value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateString(value));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateString(value));
    }
    return JSON(m_cjson);
}

JSON JSON::Add(const char* name, double value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateNumber(value));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateNumber(value));
    }
    return JSON(m_cjson);
}

JSON JSON::Add(const char* name, bool value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateBool(value ? 1 : 0));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateBool(value ? 1 : 0));
    }
    return JSON(m_cjson);
}

string JSON::GetString(const string& defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJSON_String) {
        return defaultVaule;
    }
    return m_cjson->valuestring;
}

double JSON::GetNumber(double defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJSON_Number) {
        return defaultVaule;
    }
    return m_cjson->valuedouble;
}

int JSON::GetInteger(int defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJSON_Number) {
        return defaultVaule;
    }
    return m_cjson->valueint;
}

bool JSON::GetBool(bool defaultVaule) const
{
    if(!m_cjson || (m_cjson->type != cJSON_True && m_cjson != cJSON_False)) {
        return defaultVaule;
    }
    return m_cjson->type == cJSON_True ? true : false;
}

JSON JSON::operator[](size_t index) const
{
    if(index >= GetCount()) {
        return JSON(nullptr);
    }
    cJSON* child = m_cjson->child;
    size_t where = 0;
    while(where != index) {
        child = child->next;
        ++where;
    }
    return JSON(child);
}

size_t JSON::GetCount() const
{
    if(m_cjson == nullptr) {
        return 0;
    }
    size_t count(0);
    cJSON* child = m_cjson->child;
    while(child) {
        ++count;
        child = child->next;
    }
    return count;
}

JSON JSON::AddObject(JSON obj, const char* name)
{
    if(!m_cjson) {
        return obj;
    }
    cJSON_AddItemToObject(m_cjson, name, obj.m_cjson);
    return obj;
}

JSON JSON::Add(const char* name, const vector<string>& value)
{
    auto a = AddArray(name);
    for(const auto& s : value) {
        a.Add(s);
    }
    return a;
}

vector<string> JSON::GetStringArray() const
{
    if(!m_cjson || m_cjson->type != cJSON_Array) {
        return {};
    }
    vector<string> arr;
    size_t count = GetCount();
    arr.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        arr.push_back((*this)[i].GetString());
    }
    return arr;
}

JSON JSON::Add(const char* name, JSON value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        return AddObject(value, name);
    } else {
        cJSON_AddItemToArray(m_cjson, value.m_cjson);
        return value;
    }
}

JSON JSON::Parse(const string& source) { return JSON(cJSON_Parse(source.c_str())); }
