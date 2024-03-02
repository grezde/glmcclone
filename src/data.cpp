#include "data.hpp"
#include <cmath>
#include <memory>
#include <utility>

// Proper tagged union in C++11

DataEntry::DataEntry(Type t) {
    type = t;
    integers.int64 = 0;
    switch(type) {
        case UNKOWN:
        case LEAF:
        case INT8:
        case INT16:
        case INT32:
        case INT64:
        case UINT8:
        case UINT16:
        case UINT32:
        case UINT64:
        case FLOAT32:
        case FLOAT64:
            break;
        case CHAR:
        case TYPE_COUNT:
            break;
        case STRING:
        case STR_SLICE:
            new(&str) string();
            break;
        case ERROR:
            error.location = 0;
            new(&error.message) string();
            break;
        case LIST:
        case VECTOR:
            new(&list) vector<DataEntry*>();
            break;
        case MAP:
            new(&dict) map<string, DataEntry*>;
            break;
        case BYTES:
            new(&bytes) vector<u8>();
            break;
        case ALLOC:
            allocation = {};
            break;
        case ZIPPED:
            zipped = {};
            break;
        }
}

DataEntry::~DataEntry() {
    switch(type) {
        case UNKOWN:
        case ERROR:
        case LEAF:
        case TYPE_COUNT:
        case INT8:
        case INT16:
        case INT32:
        case INT64:
        case UINT8:
        case UINT16:
        case UINT32:
        case UINT64:
        case FLOAT32:
        case FLOAT64:
            break;
        case CHAR:
            break;
        case STRING:
        case STR_SLICE:
            str.~string();
            break;
        case LIST:
        case VECTOR:
            list.~vector<DataEntry*>();
            break;
        case MAP:
            dict.~map<string, DataEntry*>();
            break;
        case BYTES:
            bytes.~vector<u8>();
            break;
        case ALLOC:
            delete allocation.inner;
            break;
        case ZIPPED:
            delete zipped.inner;
            break;
        }
}

string unescapeString(const string& raw);
void DataEntry::prettyPrint(std::ostream& out, u32 indent) const {
    //for(u32 i=0; i<indent; i++)
    //    out << "   ";
    switch(type) {
        case UNKOWN:
            out << "????";
            break;
        case ERROR:
            out << "ERROR:" << error.location << ": " << error.message; 
            break;
        case LEAF:
            out << "LEAF";
            break;
        case INT8:
            out << "<b8> " << integers.int8;
            break;
        case INT16:
            out << "<b16> " << integers.int16;
            break;
        case INT32: 
            out << "<b32> " << integers.int32;
            break;
        case INT64: 
            out << "<b64> " << integers.int64;
            break;
        case UINT8:
            out << "<b8> " << integers.uint8;
            break;
        case UINT16:
            out << "<b16> " << integers.uint16;
            break;
        case UINT32: 
            out << "<b32> " << integers.uint32;
            break;
        case UINT64: 
            out << "<b64> " << integers.uint64;
            break;
        case FLOAT32: 
            out << "<b32> " << integers.float32;
            break;
        case FLOAT64: 
            out << "<b64> " << integers.float64;
            break;
        case CHAR: 
            out << "'" << character << "'";
            break;
        case STRING:
            out << "\"" << unescapeString(str) << "\"";
            break;
        case STR_SLICE:
            out << "<slice> \"" << unescapeString(str) << "\"";
            break;
        case LIST:
        case VECTOR: 
            if(type == VECTOR)
                out << "<vec> ";
            out << "[\n";
            for(DataEntry* child : list) {
                for(u32 i=0; i<indent+1; i++)
                    out << "   ";
                child->prettyPrint(out, indent+1);
                out << ",\n";
            }
            for(u32 i=0; i<indent; i++)
                out << "   ";
            out << "]";
            break;
        case MAP: 
            out << "{\n";
            for(const std::pair<string, DataEntry*> p : dict) {
                for(u32 i=0; i<indent+1; i++)
                    out << "   ";
                out << unescapeString(p.first) << ": ";
                p.second->prettyPrint(out, indent+1);
                out << ",\n";
            }
            for(u32 i=0; i<indent; i++)
                out << "   ";
            out << "}";
            break;
        case BYTES:
            out << "BYTES";
            break;
        case ALLOC: 
            out << "ALLOC";
            break;
        case ZIPPED: 
            out << "ZIPPED";
            break;
        case TYPE_COUNT: 
            out << "?????";
            break;
    }
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool isAlpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool isNum(char c) {
    return '0' <= c && c <= '9';
}

bool isValidDictEntry(char c) {
    return isAlpha(c) || isNum(c) || c == '_'; 
}

string unescapeString(const string& raw) {
    string s = "";
    //s.reserve(raw.size());
    for(u32 i=0; i < raw.size(); i++) {
        switch(raw[i]) {
            case '\'':
            case '"':
            case '\\':
                s.push_back('\\');
                s.push_back(raw[i]);
                break;
            case '\0':
                s.append("\\0");
                break;
            case '\n':
                s.append("\\n");
                break;
            case '\t':
                s.append("\\t");
                break;
            case '\r':
                s.append("\\r");
                break;
            case '\x1b':
                s.append("\\e");
                break;
            default:
                s.push_back(raw[i]);
                break;
        }
    }
    return s;
}

char escapeChar(char c) {
    switch(c) {
        case '0': return '\0';
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'e': return '\x1b';
        default:  return c;
    }
}

string getStringLiteral(const string& text, u32& index) {
    index++;
    string s;
    while(index < text.size() && text[index] != '"') {
        if(text[index] == '\\') {
            index++;
            if(index == text.size())
                return s;
            s += escapeChar(text[index]);
        }
        else {
            s += text[index];
            index++;
        }
    }
    if(index != text.size())
        index++;
    return s;
}

string getAtom(const string& text, u32& index) {
    u32 initial = index;
    while(index < text.size() && isValidDictEntry(text[index]))
        index++;
    return text.substr(initial, index-initial);
}

void ignoreWS(const string& text, u32& index) {
    begining:
    while(index < text.size() && isWhitespace(text[index]))
        index++;
    if(index + 1 >= text.size())
        return;
    if(text[index] == '/' && text[index+1] == '/') {
        index += 2;
        while(index < text.size() && text[index] != '\n')
            index++;
        goto begining;
    }
    else if(text[index] == '/' && text[index+1] == '*') {
        index += 2;
        while(index < text.size() && !(text[index-1] == '*' && text[index] == '/'))
            index++;
        index++;
        goto begining;
    }
}

u64 getUINT(const string& text, u32& index) {
    u64 value = 0;
    while(index < text.size() && isNum(text[index])) {
        value = value*10 + text[index]-'0';
        index++;
    }
    return value;
}

i64 DataEntry::geti64() {
    switch(type) {
        case INT8: return integers.int8;
        case INT16: return integers.int16;
        case INT32: return integers.int32;
        case INT64: return integers.int64;
        case UINT8: return integers.uint8;
        case UINT16: return integers.uint16;
        case UINT32: return integers.uint32;
        case UINT64: return integers.uint64;
        default: return 0;
    }
}

void DataEntry::seti64(i64 value) {
    switch(type) {
        case INT8: integers.int8 = value; break;
        case INT16: integers.int16 = value; break;
        case INT32: integers.int32 = value; break;
        case INT64: integers.int64 = value; break;
        case UINT8: integers.uint8 = value; break;
        case UINT16: integers.uint16 = value; break;
        case UINT32: integers.uint32 = value; break;
        case UINT64: integers.uint64 = value; break;
        default: break;
    }
}

f64 DataEntry::getf64() {
    switch(type) {
        case FLOAT32: return integers.float32;
        case FLOAT64: return integers.float64;
        default: return 0;
    }
}

void DataEntry::setf64(f64 value) {
    switch(type) {
        case FLOAT32: integers.float32 = value; break;
        case FLOAT64: integers.float64 = value; break;
        default: break;
    }
}

DataEntry* DataEntry::readText(const string &text) {
    u32 i = 0;
    return DataEntry::readFromText(text, i);
}

DataEntry* DataEntry::readFromText(const string &text, u32& index) {
 
    #define ERR(errmsg, cleanup)  do{                                      \
        DataEntry* de_err = new DataEntry(DataEntry::ERROR);                       \
        de_err->error = { index, errmsg };                              \
        cleanup;                                                    \
        return de_err;                                                  \
    }while(0)
    
    ignoreWS(text, index);
    if(index == text.size()) ERR("Unexpected end of input", );
    if(text[index] == '{') {
        DataEntry* dict = new DataEntry(MAP);
        index++;
        while(true) {
            ignoreWS(text, index);
            if(index == text.size())
                ERR("Dictionary not closed", delete dict);
            if(text[index] == '}') {
                index++;
                return dict;
            }
            string key;
            if(text[index] == '"')
                key = getStringLiteral(text, index);
            else if(isValidDictEntry(text[index]))
                key = getAtom(text, index);
            else
                ERR("Unexpected character in dictionary", delete dict);
            if(index == text.size()) 
                ERR("Unexpected end of input in dictionary", delete dict);
            ignoreWS(text, index);
            if(index == text.size() || text[index] != ':') 
                ERR("Key must be followed by colon", delete dict);
            index++;
            DataEntry* value = readFromText(text, index);
            if(value->type == ERROR) {
                delete dict;
                return value;
            }
            else
                dict->dict[key] = value;
            ignoreWS(text, index);      
            if(index == text.size() || (text[index] != ',' && text[index] != '}')) 
                ERR("Key value pairs must be separated by commas", delete dict);
            if(text[index] == ',')
                index++;
        }
    }
    else if(text[index] == '[') {
        DataEntry* list = new DataEntry(LIST);
        index++;
        while(true) {
            ignoreWS(text, index);
            if(index == text.size())
                ERR("List not closed", delete list);
            if(text[index] == ']') {
                index++;
                return list;
            }
            DataEntry* element = readFromText(text, index);
            if(element->type == ERROR) {
                delete list;
                return element;
            }
            else
                list->list.push_back(element);
            ignoreWS(text, index);      
            if(index == text.size() || (text[index] != ',' && text[index] != ']')) 
                ERR("List elements must be sepparated by commas", delete list);
            if(text[index] == ',')
                index++;
        }
    }
    else if(text[index] == '<') {
        index++;
        string s = getAtom(text, index);
        if(index == text.size()) ERR("Cast not closed", );
        int sit = 0;
        if(s == "b8") sit = 1;
        else if(s == "b16") sit = 2;
        else if(s == "b32") sit = 3;
        else if(s == "b64") sit = 4;
        else if(s == "slice") sit = 5;
        else if(s == "vec") sit = 6;
        else if(s == "zip") sit = 7;
        else if(s == "alloc") sit = 8;
        else ERR("Wrong cast", );
        if(sit < 7) {
            if(text[index] != '>')
                ERR("Cast not closed", );
            index++;   
        } else if(sit == 7) {
            ignoreWS(text, index);
            string s = getAtom(text, index);
            if(index == text.size() || text[index] != '>')
                ERR("Wrong format of zip entry", );
            index++;
            CompressionType ct;
            if(s == "none") ct = UNCOMPRESSED;
            else if(s == "gzip") ct = GZIP;
            else if(s == "rle") ct = RLE;
            else ERR("Wrong format of zip entry", );
            DataEntry* inner = readFromText(text, index);
            if(inner->type == ERROR)
                return inner;
            DataEntry* outer = new DataEntry(ALLOC);
            outer->zipped = { ct, inner };
            return outer;
        } else if(sit == 8) {
            ignoreWS(text, index);
            u64 value = getUINT(text, index);
            if(value == 0)
                ERR("Cannot create alloc entry with that value",);
            if(index == text.size() || text[index] != '>')
                ERR("Incomplete cast", );
            index++;
            DataEntry* inner = readFromText(text, index);
            if(inner->type == ERROR)
                return inner;
            DataEntry* outer = new DataEntry(ALLOC);
            outer->allocation = { (u32)value, inner };
            return outer;
        }
        DataEntry* inner = readFromText(text, index);
        if(inner->type == ERROR)
            return inner;
        if(sit < 5 && INT8 <= inner->type && inner->type <= UINT64) {
            i64 inside = inner->geti64();
            inner->type = (Type)(INT8 + (inner->type - INT8)/4*4 + sit-1);
            inner->seti64(inside);
        }
        else if((sit == 3 || sit == 4) && (inner->type == FLOAT32 || inner->type == FLOAT64)) {
            f64 inside = inner->getf64();
            inner->type = (Type)(FLOAT32 + sit);
            inner->setf64(inside);
        }
        else if(sit == 5 && inner->type == STRING)
            inner->type = STR_SLICE;
        else if(sit == 6 && inner->type == LIST)
            inner->type = VECTOR;
        else ERR("Impossible cast", delete inner);
        return inner;
    }
    else if(text[index] == '"') {
        string s = getStringLiteral(text, index);
        if(index == text.size()) ERR("Unterminated string", );
        DataEntry* d = new DataEntry(STRING);
        d->str = s;
        return d;
    }
    else if(text[index] == '\'') {
        // TODO: add escapes
        if(index + 2 >= text.size() || text[index+2] != '\'')
            ERR("Unterminated character", );
        DataEntry* d = new DataEntry(CHAR);
        d->character = text[index+1];
        index += 3;
        return d;
    }
    else if(text[index] == '-' || isNum(text[index])) {
        u8 sign = 1;
        if(text[index] == '-') {
            sign = -1;
            index++;
        }
        // TODO: add hex, binary number literals
        DataEntry* de = new DataEntry(INT64);
        de->integers.int64 = sign * (i64)getUINT(text, index);
        if(index == text.size()) return de;
        if(text[index] == '.') {
            index++;
            de->type = FLOAT64;
            de->integers.float64 = de->integers.int64;
            f64 fp = 0.1;
            while(index < text.size() && isNum(text[index])) {
                de->integers.float64 += (f64)(text[index] - '0') * fp;
                fp *= 0.1;
                index++;
            }
        }
        if(index == text.size()) return de;
        if(text[index] == 'e') {
            index++;
            if(index == text.size()) return de;
            sign = 1;
            if(text[index] == '-') {
                sign = -1;
                index++;
            }
            else if(text[index] == '+')
                index++;
            i64 exp = getUINT(text, index);
            if(de->type != FLOAT64) {
                de->type = FLOAT64;
                de->integers.float64 = de->integers.int64;
            }
            de->integers.float64 *= std::pow(10, sign * exp);
        }
        return de;
    }
    else ERR(string("Unexpected character '") + text[index] + "'", );

    #undef ERR
}