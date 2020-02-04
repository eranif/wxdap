#include "GDBMI.hpp"
#include "dap/StringUtils.hpp"

vector<string> GDBMI::SplitToBlocksCurly(const string& buffer)
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

unordered_map<string, string> GDBMI::SplitToKeyValues(const string& buffer)
{
    string k, v;
    unordered_map<string, string> M;
    enum eState { kInString, kReadingKey, kReadingValue };
    eState state = kReadingKey;
    int depth = 0;
    for(char ch : buffer) {
        switch(state) {
        case kReadingKey:
            // Can only be depth 0
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
            case '{':
            case '[':
                depth++;
                break;
            case '}':
            case ']':
                depth--;
                break;
            case ',':
                if(depth == 0) {
                    // We are done collecting this value
                    state = kReadingKey;
                    if(!k.empty() && !v.empty()) {
                        M.insert({ k, v });
                    }
                    k.clear();
                    v.clear();
                } else {
                    v.append(1, ch);
                }
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
    return M;
}

dap::Breakpoint GDBMI::DoParseBreakpoint(const string& block)
{
    dap::Breakpoint bpt;
    auto map = SplitToKeyValues(block);

    // we now have key=value
    if(map.count("file")) {
        bpt.source.path = StringUtils::ToUnixPath(map["file"]);
    }

    if(map.count("fullname")) {
        bpt.source.path = StringUtils::ToUnixPath(map["fullname"]);
    }

    if(map.count("line")) {
        bpt.line = atoi(map["line"].c_str());
    }

    if(map.count("number")) {
        bpt.id = atoi(map["number"].c_str());
    }

    if(map.count("pending")) {
        bpt.source.path = "<PENDING>";
    }
    bpt.verified = (bpt.id != -1);
    return bpt;
}

dap::Breakpoint GDBMI::ParseBreakpoint(const string& gdbOutput)
{
    auto blocks = SplitToBlocksCurly(gdbOutput);
    if(blocks.size() != 1) {
        return {};
    }
    return DoParseBreakpoint(blocks[0]);
}

vector<dap::Breakpoint> GDBMI::ParseBreakpoints(const string& gdbOutput)
{
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

    vector<dap::Breakpoint> bpts;
    // Split by comma
    for(auto& block : blocks) {
        auto bp = DoParseBreakpoint(block);
        bpts.emplace_back(bp);
    }
    return bpts;
}

dap::StackFrame GDBMI::ParseStackFrame(const string& gdbOutput)
{
    auto blocks = SplitToBlocksCurly(gdbOutput);
    if(blocks.size() != 1) {
        return {};
    }
    return DoParseStackFrame(blocks[0]);
}

dap::StackFrame GDBMI::DoParseStackFrame(const string& block)
{
    // Frame example:
    //
    // {level="0",addr="0x0000000000401558",func="bar",file="C:/Users/Eran/Documents/AmitTest/AmitTest/main.cpp",
    // fullname="C:\\Users\\Eran\\Documents\\AmitTest\\AmitTest\\main.cpp",line="8"}

    dap::StackFrame frame;
    auto map = SplitToKeyValues(block);

    if(map.count("file")) {
        frame.source.path = StringUtils::ToUnixPath(map["file"]);
    }
    if(map.count("fullname")) {
        frame.source.path = StringUtils::ToUnixPath(map["fullname"]);
    }
    if(map.count("line")) {
        frame.line = atoi(map["line"].c_str());
    }
    if(map.count("level")) {
        frame.number = atoi(map["level"].c_str());
    }
    return frame;
}

GDBMI::eStoppedReason GDBMI::ParseStoppedReason(const string& gdbOutput)
{
    //*stopped,reason="breakpoint-hit",disp="keep",bkptno="3",frame={addr="0x0000000000401558",func="bar",
    // args=[],file="C:/Users/Eran/Documents/AmitTest/AmitTest/main.cpp",
    // fullname="C:\\Users\\Eran\\Documents\\AmitTest\\AmitTest\\main.cpp",line="8"},thread-id="1",stopped-threads="all"
    string buffer = StringUtils::AfterFirst(gdbOutput, ',');
    auto map = SplitToKeyValues(buffer);
    auto where = map.find("reason");
    if(where->second == "breakpoint-hit") {
        return kBreakpointHit;
    }
    return kUnknown;
}
