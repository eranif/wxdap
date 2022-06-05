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

#ifndef JSON_HPP
#define JSON_HPP

#include "cJSON.hpp"
#include <atomic>
#include <cstring>
#include <memory>
#include <vector>
#include <wx/string.h>

struct JSON {
    cJSON* m_cjson = nullptr;
    std::atomic_int* m_refCount = nullptr;

private:
    JSON(cJSON* ptr);
    JSON AddItem(const wxString& name, cJSON* item);

    bool IsArray() const { return m_cjson && m_cjson->type == cJSON_Array; }
    bool IsObject() const { return m_cjson && m_cjson->type == cJSON_Object; }

    void DecRef();
    void IncRef();
    void Manage();
    void UnManage();
    void Delete();
    bool IsManaged() const { return m_refCount != nullptr; }

public:
    ~JSON();
    JSON() {}

    JSON& operator=(const JSON& other);
    JSON(const JSON& other);

    /**
     * @brief return the property name
     */
    wxString GetName() const
    {
        if(m_cjson == nullptr || !m_cjson->string) {
            return "";
        }
        return wxString(m_cjson->string);
    }

    /**
     * @brief return true if this object is not null
     */
    bool IsOK() const { return m_cjson != nullptr; }

    /**
     * @brief create new TOP level array
     * If you need to array to an existing JSON,
     * call AddArray()
     */
    static JSON CreateArray();

    /**
     * @brief create new TOP level object
     * If you need to object to an existing JSON,
     * call AddObject()
     */
    static JSON CreateObject();

    /**
     * @brief create JSON from wxString buffer
     */
    static JSON Parse(const wxString& source);

    /**
     * @brief object property access
     */
    JSON operator[](const wxString& index) const;

    /**
     * @brief index access
     */
    JSON operator[](size_t index) const;

    /**
     * @brief get number of children
     */
    size_t GetCount() const;

    /**
     * @brief add array to this JSON.
     * @param name the name of the array. If this JSON is of type array
     * the name is ignored
     * @return the newly added array. Check for IsOK()
     */
    JSON AddArray(const wxString& name = "") { return AddItem(name, cJSON_CreateArray()); }

    /**
     * @brief create and add object to this JSON.
     * @param name the name of the array. If this JSON is of type array
     * the name is ignored
     * @return the newly added object. Check for IsOK()
     */
    JSON AddObject(const wxString& name = "") { return AddItem(name, cJSON_CreateObject()); }

    /**
     * @brief add object to this JSON.
     * @return the newly added object
     */
    JSON AddObject(const wxString& name, const JSON& obj) { return AddObject(name.mb_str(wxConvUTF8).data(), obj); }
    JSON AddObject(const char* name, const JSON& obj);

    /**
     * @brief return value as wxString
     */
    wxString GetString(const wxString& defaultVaule = "") const;

    /**
     * @brief return value as number
     */
    double GetNumber(double defaultVaule = -1) const;

    /**
     * @brief return value as number
     */
    int GetInteger(int defaultVaule = -1) const;

    /**
     * @brief return value as boolean
     */
    bool GetBool(bool defaultVaule = false) const;

    /**
     * @brief return wxString array
     */
    std::vector<wxString> GetStringArray() const;

    /**
     * @brief return wxString representation for this object
     */
    wxString ToString(bool pretty = true) const;

    // Add properties to container (can be object or array)
    JSON Add(const wxString& name, const wxString& value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    JSON Add(const wxString& name, const std::vector<wxString>& value)
    {
        return Add(name.mb_str(wxConvUTF8).data(), value);
    }
    JSON Add(const wxString& name, const JSON& value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    JSON Add(const wxString& name, const char* value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    JSON Add(const wxString& name, double value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    JSON Add(const wxString& name, int value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    JSON Add(const wxString& name, long value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    JSON Add(const wxString& name, size_t value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    JSON Add(const wxString& name, bool value) { return Add(name.mb_str(wxConvUTF8).data(), value); }

    JSON Add(const char* name, const wxString& value);
    JSON Add(const char* name, const char* value);
    JSON Add(const char* name, bool value);
    JSON Add(const char* name, double value);
    JSON Add(const char* name, const std::vector<wxString>& value);
    JSON Add(const char* name, const JSON& value);
    JSON Add(const char* name, long value) { return Add(name, (double)value); }
    JSON Add(const char* name, size_t value) { return Add(name, (double)value); }
    JSON Add(const char* name, int value) { return Add(name, (double)value); }

    // Same as the above but without providing 'name'
    // useful for array
    JSON Add(const wxString& value) { return Add("", value); }
    JSON Add(const char* value) { return Add("", value); }
    JSON Add(double value) { return Add("", value); }
    JSON Add(long value) { return Add("", (double)value); }
    JSON Add(int value) { return Add("", (double)value); }
    JSON Add(size_t value) { return Add("", (double)value); }
    JSON Add(bool value) { return Add("", value); }
    JSON Add(const JSON& value) { return Add("", value); }
};
#endif // JSON_HPP
