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

#ifndef DAPJSON_HPP
#define DAPJSON_HPP

#include "cJSON.hpp"
#include "dap_exports.hpp"

#include <atomic>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>
#include <wx/string.h>

namespace dap
{
struct WXDLLIMPEXP_DAP Json {
    cJsonDap* m_cjson = nullptr;
    std::atomic_int* m_refCount = nullptr;

private:
    Json(cJsonDap* ptr);
    Json AddItem(const wxString& name, cJsonDap* item);

    void DecRef();
    void IncRef();
    void Manage();
    void UnManage();
    void Delete();
    bool IsManaged() const { return m_refCount != nullptr; }

public:
    ~Json();
    Json() {}

    bool IsArray() const { return m_cjson && m_cjson->type == cJsonDap_Array; }
    bool IsObject() const { return m_cjson && m_cjson->type == cJsonDap_Object; }

    Json& operator=(const Json& other);
    Json(const Json& other);

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
     * If you need to array to an existing Json,
     * call AddArray()
     */
    static Json CreateArray();

    /**
     * @brief create new TOP level object
     * If you need to object to an existing Json,
     * call AddObject()
     */
    static Json CreateObject();

    /**
     * @brief create Json from wxString buffer
     */
    static Json Parse(const wxString& source);

    /**
     * @brief object property access
     */
    Json operator[](const wxString& index) const;

    /**
     * @brief index access
     */
    Json operator[](size_t index) const;

    /**
     * @brief get number of children
     */
    size_t GetCount() const;

    /**
     * @brief add array to this Json.
     * @param name the name of the array. If this Json is of type array
     * the name is ignored
     * @return the newly added array. Check for IsOK()
     */
    Json AddArray(const wxString& name = "") { return AddItem(name, cJSON_CreateArray()); }

    /**
     * @brief create and add object to this Json.
     * @param name the name of the array. If this Json is of type array
     * the name is ignored
     * @return the newly added object. Check for IsOK()
     */
    Json AddObject(const wxString& name = "") { return AddItem(name, cJSON_CreateObject()); }

    /**
     * @brief add object to this Json.
     * @return the newly added object
     */
    Json AddObject(const wxString& name, const Json& obj) { return AddObject(name.mb_str(wxConvUTF8).data(), obj); }
    Json AddObject(const char* name, const Json& obj);

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
    Json Add(const wxString& name, const wxString& value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    Json Add(const wxString& name, const std::vector<wxString>& value)
    {
        return Add(name.mb_str(wxConvUTF8).data(), value);
    }
    Json Add(const wxString& name, const Json& value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    Json Add(const wxString& name, const char* value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    Json Add(const wxString& name, double value) { return Add(name.mb_str(wxConvUTF8).data(), value); }
    Json Add(const wxString& name, int value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    Json Add(const wxString& name, long value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    Json Add(const wxString& name, size_t value) { return Add(name.mb_str(wxConvUTF8).data(), (double)value); }
    Json Add(const wxString& name, bool value) { return Add(name.mb_str(wxConvUTF8).data(), value); }

    Json Add(const char* name, const wxString& value);
    Json Add(const char* name, const char* value);
    Json Add(const char* name, bool value);
    Json Add(const char* name, double value);
    Json Add(const char* name, const std::vector<wxString>& value);
    Json Add(const char* name, const Json& value);
    Json Add(const char* name, long value) { return Add(name, (double)value); }
    Json Add(const char* name, size_t value) { return Add(name, (double)value); }
    Json Add(const char* name, int value) { return Add(name, (double)value); }

    // Same as the above but without providing 'name'
    // useful for array
    Json Add(const wxString& value) { return Add("", value); }
    Json Add(const char* value) { return Add("", value); }
    Json Add(double value) { return Add("", value); }
    Json Add(long value) { return Add("", (double)value); }
    Json Add(int value) { return Add("", (double)value); }
    Json Add(size_t value) { return Add("", (double)value); }
    Json Add(bool value) { return Add("", value); }
    Json Add(const Json& value) { return Add("", value); }
};

} // namespace dap
#endif // JSON_HPP
