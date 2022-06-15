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

namespace dap
{
#define CHECK_IS_CONTAINER()        \
    if(!m_cjson) {                  \
        return Json(nullptr);       \
    }                               \
    if(!IsArray() && !IsObject()) { \
        return Json(m_cjson);       \
    }

Json::Json(cJsonDap* ptr)
    : m_cjson(ptr)
{
}

void Json::DecRef()
{
    if(m_refCount) {
        (*m_refCount)--;
        if(m_refCount->load() == 0) {
            // Releas the underlying pointer
            Delete();
            delete m_refCount;
            m_refCount = nullptr;
        }
    }
}

void Json::IncRef()
{
    if(m_refCount) {
        (*m_refCount)++;
    }
}

void Json::Manage()
{
    if(!IsManaged()) {
        m_refCount = new std::atomic_int;
        m_refCount->store(1);
    }
}

void Json::UnManage()
{
    if(m_refCount) {
        delete m_refCount;
        m_refCount = nullptr;
    }
}

Json& Json::operator=(const Json& other)
{
    if(this == &other) {
        return *this;
    }
    DecRef();
    m_refCount = other.m_refCount;
    m_cjson = other.m_cjson;
    // Increase the ref count if needed
    IncRef();
    return *this;
}

Json::Json(const Json& other) { *this = other; }

Json::~Json()
{
    DecRef();
    m_cjson = nullptr;
}

Json Json::operator[](const wxString& index) const
{
    if(m_cjson == nullptr) {
        return Json(nullptr);
    }

    cJsonDap* child = m_cjson->child;
    while(child) {
        if(child->string && strcmp(child->string, index.c_str()) == 0) {
            return Json(child);
        }
        child = child->next;
    }
    return Json(nullptr);
}

Json Json::AddItem(const wxString& name, cJsonDap* item)
{
    if(m_cjson == nullptr) {
        cJSON_Delete(item);
        return Json(nullptr);
    }
    if(m_cjson->type != cJsonDap_Array && m_cjson->type != cJsonDap_Object) {
        cJSON_Delete(item);
        return Json(nullptr);
    }
    if(m_cjson->type == cJsonDap_Array) {
        cJSON_AddItemToArray(m_cjson, item);
    } else {
        cJSON_AddItemToObject(m_cjson, name.c_str(), item);
    }
    return Json(item);
}

wxString Json::ToString(bool pretty) const
{
    if(m_cjson == nullptr) {
        return "";
    }
    char* c = pretty ? cJSON_Print(m_cjson) : cJSON_PrintUnformatted(m_cjson);
    wxString str(c);
    free(c);
    return str;
}

Json Json::CreateArray()
{
    Json arr(cJSON_CreateArray());
    arr.Manage();
    return arr;
}

Json Json::CreateObject()
{
    Json obj(cJSON_CreateObject());
    obj.Manage();
    return obj;
}

void Json::Delete()
{
    // Delete only when owned
    if(m_cjson) {
        cJSON_Delete(m_cjson);
        m_cjson = nullptr;
    }
}

Json Json::Add(const char* name, const wxString& value) { return Add(name, value.mb_str(wxConvUTF8).data()); }

Json Json::Add(const char* name, const char* value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateString(value));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateString(value));
    }
    return Json(m_cjson);
}

Json Json::Add(const char* name, double value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateNumber(value));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateNumber(value));
    }
    return Json(m_cjson);
}

Json Json::Add(const char* name, bool value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        cJSON_AddItemToObject(m_cjson, name, cJSON_CreateBool(value ? 1 : 0));
    } else {
        // Array
        cJSON_AddItemToArray(m_cjson, cJSON_CreateBool(value ? 1 : 0));
    }
    return Json(m_cjson);
}

wxString Json::GetString(const wxString& defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJsonDap_String) {
        return defaultVaule;
    }
    return m_cjson->valuestring;
}

double Json::GetNumber(double defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJsonDap_Number) {
        return defaultVaule;
    }
    return m_cjson->valuedouble;
}

int Json::GetInteger(int defaultVaule) const
{
    if(!m_cjson || m_cjson->type != cJsonDap_Number) {
        return defaultVaule;
    }
    return m_cjson->valueint;
}

bool Json::GetBool(bool defaultVaule) const
{
    if(!m_cjson || (m_cjson->type != cJsonDap_True && m_cjson != cJsonDap_False)) {
        return defaultVaule;
    }
    return m_cjson->type == cJsonDap_True ? true : false;
}

Json Json::operator[](size_t index) const
{
    if(index >= GetCount()) {
        return Json(nullptr);
    }
    cJsonDap* child = m_cjson->child;
    size_t where = 0;
    while(where != index) {
        child = child->next;
        ++where;
    }
    return Json(child);
}

size_t Json::GetCount() const
{
    if(m_cjson == nullptr) {
        return 0;
    }
    size_t count(0);
    cJsonDap* child = m_cjson->child;
    while(child) {
        ++count;
        child = child->next;
    }
    return count;
}

Json Json::AddObject(const char* name, const Json& obj)
{
    if(!m_cjson) {
        return obj;
    }
    cJSON_AddItemToObject(m_cjson, name, obj.m_cjson);
    if(obj.IsManaged()) {
        Json& o = const_cast<Json&>(obj);
        o.UnManage(); // We take ownership
    }
    return obj;
}

Json Json::Add(const char* name, const std::vector<wxString>& value)
{
    auto a = AddArray(name);
    for(const auto& s : value) {
        a.Add(s);
    }
    return a;
}

std::vector<wxString> Json::GetStringArray() const
{
    if(!m_cjson || m_cjson->type != cJsonDap_Array) {
        return {};
    }
    std::vector<wxString> arr;
    size_t count = GetCount();
    arr.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        arr.push_back((*this)[i].GetString());
    }
    return arr;
}

Json Json::Add(const char* name, const Json& value)
{
    CHECK_IS_CONTAINER();
    if(IsObject()) {
        return AddObject(name, value);
    } else {
        if(value.IsManaged()) {
            Json& o = const_cast<Json&>(value);
            o.UnManage(); // We take ownership
        }
        cJSON_AddItemToArray(m_cjson, value.m_cjson);
        return value;
    }
}

Json Json::Parse(const wxString& source)
{
    Json json(cJSON_Parse(source.c_str()));
    json.Manage();
    return json;
}
} // namespace dap