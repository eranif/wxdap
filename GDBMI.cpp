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

vector<pair<string, string>> GDBMI::SplitToKeyValues(const string& buffer)
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

dap::Breakpoint GDBMI::DoParseBreakpoint(const string& block)
{
    dap::Breakpoint bpt;
    auto keyValues = SplitToKeyValues(block);

    // we now have key=value
    for(auto& pr : keyValues) {
        const string& key = pr.first;
        const string& value = pr.second;
        if(key.empty() || value.empty()) {
            continue;
        }
        if(key == "file") {
            bpt.source.path = StringUtils::ToUnixPath(value);
        } else if(key == "fullname") {
            bpt.source.path = StringUtils::ToUnixPath(value);
        } else if(key == "line") {
            bpt.line = atoi(value.c_str());
        } else if(key == "number") {
            bpt.id = atoi(value.c_str());
        } /* else if(key == "func") {
             bpt.func = value;
         } else if(key == "type") {
             bpt.type = value;
         }*/
    }
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
