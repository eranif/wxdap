//
// This file was automatically generated by wxrc, do not edit by hand.
//

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/filesys.h>
#include <wx/fs_mem.h>
#include <wx/xrc/xh_all.h>
#include <wx/xrc/xmlres.h>

#if wxCHECK_VERSION(2, 8, 5) && wxABI_VERSION >= 20805
#define XRC_ADD_FILE(name, data, size, mime) wxMemoryFSHandler::AddFileWithMimeType(name, data, size, mime)
#else
#define XRC_ADD_FILE(name, data, size, mime) wxMemoryFSHandler::AddFile(name, data, size)
#endif

static size_t xml_res_size_0 = 0;
static unsigned char xml_res_file_0[] = {};

static size_t xml_res_size_1 = 282;
static unsigned char xml_res_file_1[] = {
    60,  63,  120, 109, 108, 32,  118, 101, 114, 115, 105, 111, 110, 61,  34,  49,  46,  48,  34,  32,  101, 110,
    99,  111, 100, 105, 110, 103, 61,  34,  85,  84,  70,  45,  56,  34,  63,  62,  10,  60,  114, 101, 115, 111,
    117, 114, 99,  101, 32,  120, 109, 108, 110, 115, 61,  34,  104, 116, 116, 112, 58,  47,  47,  119, 119, 119,
    46,  119, 120, 119, 105, 100, 103, 101, 116, 115, 46,  111, 114, 103, 47,  119, 120, 120, 114, 99,  34,  62,
    10,  32,  32,  60,  33,  45,  45,  32,  72,  97,  110, 100, 108, 101, 114, 32,  71,  101, 110, 101, 114, 97,
    116, 105, 111, 110, 32,  105, 115, 32,  79,  78,  32,  45,  45,  62,  10,  32,  32,  60,  111, 98,  106, 101,
    99,  116, 32,  99,  108, 97,  115, 115, 61,  34,  119, 120, 66,  105, 116, 109, 97,  112, 34,  32,  110, 97,
    109, 101, 61,  34,  112, 108, 97,  99,  101, 104, 111, 108, 100, 101, 114, 49,  54,  34,  62,  85,  73,  95,
    100, 98,  103, 99,  108, 105, 95,  98,  105, 116, 109, 97,  112, 115, 46,  99,  112, 112, 36,  46,  46,  95,
    46,  46,  95,  46,  46,  95,  46,  46,  95,  46,  46,  95,  46,  46,  95,  85,  115, 101, 114, 115, 95,  101,
    114, 97,  110, 95,  65,  112, 112, 68,  97,  116, 97,  95,  76,  111, 99,  97,  108, 95,  84,  101, 109, 112,
    95,  112, 108, 97,  99,  101, 104, 111, 108, 100, 101, 114, 49,  54,  46,  112, 110, 103, 60,  47,  111, 98,
    106, 101, 99,  116, 62,  10,  60,  47,  114, 101, 115, 111, 117, 114, 99,  101, 62,  10
};

void wxC10A1InitBitmapResources()
{

    // Check for memory FS. If not present, load the handler:
    {
        wxMemoryFSHandler::AddFile(wxT("XRC_resource/dummy_file"), wxT("dummy one"));
        wxFileSystem fsys;
        wxFSFile* f = fsys.OpenFile(wxT("memory:XRC_resource/dummy_file"));
        wxMemoryFSHandler::RemoveFile(wxT("XRC_resource/dummy_file"));
        if(f)
            delete f;
        else
            wxFileSystem::AddHandler(new wxMemoryFSHandlerBase);
    }

    XRC_ADD_FILE(
        wxT("XRC_resource/UI_dbgcli_bitmaps.cpp$.._.._.._.._.._.._Users_eran_AppData_Local_Temp_placeholder16.png"),
        xml_res_file_0, xml_res_size_0, wxT("image/png"));
    XRC_ADD_FILE(wxT("XRC_resource/UI_dbgcli_bitmaps.cpp$D__Temp_dbgcli_UI_dbgcli_bitmaps.xrc"), xml_res_file_1,
                 xml_res_size_1, wxT("text/xml"));
    wxXmlResource::Get()->Load(wxT("memory:XRC_resource/UI_dbgcli_bitmaps.cpp$D__Temp_dbgcli_UI_dbgcli_bitmaps.xrc"));
}
