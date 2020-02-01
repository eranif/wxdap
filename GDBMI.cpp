#include "GDBMI.hpp"
#include "dap/StringUtils.hpp"

GDBMI::GDBMI() {}

GDBMI::~GDBMI() {}

GDBMI::Breakpoint::Vec_t GDBMI::ParseBreakpoints(const string& gdbOutput)
{
    // ^done,BreakpointTable={nr_rows="2",nr_cols="6",hdr=[{width="7",alignment="-1",col_name="number",colhdr="Num"},{width="14",alignment="-1",col_name="type",colhdr="Type"},{width="4",alignment="-1",col_name="disp",colhdr="Disp"},{width="3",alignment="-1",col_name="enabled",colhdr="Enb"},{width="18",alignment="-1",col_name="addr",colhdr="Address"},{width="40",alignment="2",col_name="what",colhdr="What"}],body=[bkpt={number="1",type="breakpoint",disp="keep",enabled="y",addr="0x0000000000401564",func="main(int,
    // char**)",file="C:/Users/Eran/Documents/AmitTest/AmitTest/main.cpp",fullname="C:\\Users\\Eran\\Documents\\AmitTest\\AmitTest\\main.cpp",line="10",thread-groups=["i1"],times="0",original-location="main.cpp:10"},bkpt={number="2",type="breakpoint",disp="keep",enabled="y",addr="0x0000000000401564",func="main(int,
    // char**)",file="C:/Users/Eran/Documents/AmitTest/AmitTest/main.cpp",fullname="C:\\Users\\Eran\\Documents\\AmitTest\\AmitTest\\main.cpp",line="12",thread-groups=["i1"],times="0",original-location="main.cpp:12"}]}

    size_t where = gdbOutput.find("body=");
    if(where == string::npos) {
        return {};
    }

    string buffer = gdbOutput.substr(where + 5); // 5 => body=

    // [bkpt={number="1" ...
    auto blocks = SplitToBlocksCurly(buffer);
    if(blocks.empty()) {
        return {};
    }

    GDBMI::Breakpoint::Vec_t bpts;
    // Split by comma
    for(auto& block : blocks) {
        auto keyValues = SplitToKeyValues(block);
        // we now have key=value
        Breakpoint bp;
        for(auto& pr : keyValues) {
            const string& key = pr.first;
            const string& value = pr.second;
            if(key.empty() || value.empty()) {
                continue;
            }
            if(key == "file") {
                bp.file = value;
            } else if(key == "fullname") {
                bp.fullname = value;
            } else if(key == "line") {
                bp.line = value;
            } else if(key == "number") {
                bp.number = value;
            } else if(key == "enabled") {
                bp.enabled = value == "y" ? true : false;
            } else if(key == "func") {
                bp.func = value;
            } else if(key == "type") {
                bp.type = value;
            }
        }
        bpts.emplace_back(bp);
    }
    return bpts;
}

vector<string> GDBMI::SplitToBlocksCurly(string& buffer)
{
    int depth = -1;
    enum eState { kOutside, kInside };
    eState state = kOutside;
    string block;
    vector<string> blocks;
    for(char ch : buffer) {
        switch(state) {
        case kOutside:
            switch(ch) {
            case '{':
                depth++;
                if(depth == 0) {
                    state = kInside;
                    block.clear();
                }
                break;
            case '}':
                depth--;
                break;
            default:
                break;
            }
            break;
        case kInside:
            switch(ch) {
            case '{':
                ++depth;
                block.append(1, ch);
                break;
            case '}':
                --depth;
                if(depth == -1) {
                    state = kOutside;
                    blocks.emplace_back(block);
                    block.clear();
                } else {
                    block.append(1, ch);
                }
                break;
            default:
                block.append(1, ch);
                break;
            }
            break;
        }
    }
    return blocks;
}

vector<pair<string, string>> GDBMI::SplitToKeyValues(string& buffer)
{
    string k, v;
    vector<pair<string, string>> V;
    enum eState { kInString, kReadingKey, kReadingValue };
    eState state = kReadingKey;
    for(char ch : buffer) {
        switch(state) {
        case kReadingKey:
            switch(ch) {
            case '=':
                state = kReadingValue;
                break;
            default:
                k.append(1, ch);
                break;
            }
            break;
        case kReadingValue:
            switch(ch) {
            case ',':
                // We are done collecting this value
                state = kReadingKey;
                V.push_back({ k, v });
                k.clear();
                v.clear();
                break;
            case '"':
                state = kInString;
                break;
            default:
                v.append(1, ch);
                break;
            }
            break;
        /// Handle string state
        case kInString:
            switch(ch) {
            case '"':
                state = kReadingValue;
                break;
            default:
                v.append(1, ch);
                break;
            }
            break;
        }
    }
    return V;
}
