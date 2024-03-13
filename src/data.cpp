#include "data.hpp"
#include "base.hpp"
#include <cmath>
#include <cstring>
#include <memory>
#include <utility>

map<string, u8> DataEntry::enums;

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
            error.row = 0;
            error.col = 0;
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
        case PAIR:
        case TUPLE3:
            tuple = {};
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
        case PAIR:
            if(tuple.first != nullptr) delete tuple.first;
            if(tuple.second != nullptr) delete tuple.second;
            break;
        case TUPLE3:
            if(tuple.first != nullptr) delete tuple.first;
            if(tuple.second != nullptr) delete tuple.second;
            if(tuple.third != nullptr) delete tuple.third;
            break;
        }
}

DataEntry* DataEntry::copy() const {
    DataEntry* result = new DataEntry(type);
    switch (type) {
        case UNKOWN:
        case ERROR:
        case LEAF:
        case TYPE_COUNT:
            break;
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
        case CHAR:
            result->integers = integers;
            break;
        case STRING:
        case STR_SLICE:
            result->str = str;
            break;
        case LIST:
        case VECTOR:
            result->list.reserve(list.size());
            for(DataEntry* de : list)
                result->list.push_back(de->copy());
            break;
        case MAP:
            for(const std::pair<string, DataEntry*> p : dict)
                result->dict[p.first] = p.second->copy();
            break;
        case PAIR:
            result->tuple.first = tuple.first->copy();
            result->tuple.second = tuple.second->copy();
            break;
        case TUPLE3:
            result->tuple.first = tuple.first->copy();
            result->tuple.second = tuple.second->copy();
            result->tuple.third = tuple.third->copy();
            break;
        case BYTES:
            result->bytes.resize(bytes.size());
            memcpy(&result->bytes[0], &bytes[0], bytes.size());
            break;
        case ALLOC:
            result->allocation.size = allocation.size;
            result->allocation.inner = allocation.inner->copy();
            break;
        case ZIPPED:
            result->zipped.type = zipped.type;
            result->zipped.inner = zipped.inner->copy();
            break;
    }
    return result;
}

void DataEntry::mergeStructure(const DataEntry* other) {
    if(!(this->isMap() && other->isMap()))
        return;
    for(std::pair<string, DataEntry*> p : other->dict) {
        DataEntry*& newchild = dict[p.first];
        if(this->has(p.first)) {
            if(newchild->isMap()) {
                newchild->mergeStructure(p.second);
                continue;
            }
            delete newchild;
        }
        newchild = p.second->copy();
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
            out << "ERROR: " << error.row << ":" << error.col << ": " << error.message; 
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
                out << (child != list.back() ? ",\n" : "\n");
            }
            for(u32 i=0; i<indent; i++)
                out << "   ";
            out << "]";
            break;
        case MAP: {
            u32 index = 0;
            out << "{\n";
            for(const std::pair<string, DataEntry*> p : dict) {
                for(u32 i=0; i<indent+1; i++)
                    out << "   ";
                out << unescapeString(p.first) << ": ";
                p.second->prettyPrint(out, indent+1);
                out << (index != dict.size()-1 ? ",\n" : "\n");
                index++;
            }
            for(u32 i=0; i<indent; i++)
                out << "   ";
            out << "}";
        }
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
        case PAIR:
        case TUPLE3:
            out << "(";
            tuple.first->prettyPrint(out, indent);
            out << ", ";
            tuple.second->prettyPrint(out, indent);
            if(type == TUPLE3) {
                out << ", ";
                tuple.third->prettyPrint(out, indent);
            }
            out << ")";
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

i64 DataEntry::geti64() const {
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

f64 DataEntry::getf64() const {
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
        u32 _linen = 0, _lastl = 0; for(u32 i=0; i<index; i++) if(text[i] == '\n') { _linen++; _lastl=i; } \
        de_err->error = { _linen+1, index - _lastl, errmsg };                              \
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
    else if(text[index] == '(') {
        index++;
        DataEntry* first = DataEntry::readFromText(text, index);
        if(first->type == ERROR) return first;
        ignoreWS(text, index);
        if(index == text.size() || text[index] != ',')
            ERR("Pair or tuple must have elements separated by commas", delete first);
        index++;
        DataEntry* second = DataEntry::readFromText(text, index);
        if(second->type == ERROR) { delete first; return second; }
        ignoreWS(text, index);
        if(!(text[index] == ',' || text[index] == ')')) {
            cout << index << " '" << text[index] << "'\n";
            ERR("Pair or tuple must have elements separated by commas 2", delete first; delete second);
        }
        if(text[index] == ')') {
            index++;
            DataEntry* pair = new DataEntry(PAIR);
            pair->tuple = { first, second, nullptr };
            return pair;
        }
        index++;
        DataEntry* third = DataEntry::readFromText(text, index);
        if(third->type == ERROR) { delete first; delete second; return third; }
        ignoreWS(text, index);
        if(index == text.size() || text[index] != ')')
            ERR("Tuple must have 3 elements separated by commas", delete first; delete second; delete third);
        index++;
        DataEntry* tuple = new DataEntry(TUPLE3);
        tuple->tuple = { first, second, third };
        return tuple;
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
    else if(isValidDictEntry(text[index])) {
        string atom = getAtom(text, index);
        if(enums.find(atom) != enums.end()) {
            DataEntry* de = new DataEntry(UINT8);
            de->integers.uint8 = enums[atom];
            return de;
        }
        ERR("Unexpected atom \"" + atom + "\"", );
    }
    else ERR(string("Unexpected character '") + text[index] + "'", );

    #undef ERR
}

void DataEntry::writeBinary(FILE *out) {
    fwrite(&type, 1, 1, out);
    writeWithoutTag(out);
}

void DataEntry::writeWithoutTag(FILE* out) {
    #define sfwrite(ptr, size) fwrite(ptr, size, 1, out)

    switch (type) {
        case UNKOWN:
        case ERROR:
        case LEAF:
        case TYPE_COUNT:
            break;
        case INT8:
        case UINT8:
        case CHAR:
            sfwrite(&integers.int8, 1);
            break;
        case INT16:
        case UINT16:
            sfwrite(&integers.int16, 2);
            break;
        case INT32:
        case UINT32:
        case FLOAT32:
            sfwrite(&integers.int32, 4);
            break;
        case INT64:
        case UINT64:
        case FLOAT64:
            sfwrite(&integers.int64, 8);
            break;
        case STRING:
        case STR_SLICE: {
            u16 size = str.size();
            sfwrite(&size, 2);
            sfwrite(&str[0], size);
        }
            break;
        case LIST: {
            u16 count = list.size();
            sfwrite(&count, 2);
            for(DataEntry* de : list)
                de->writeBinary(out);
        }
            break;
        case VECTOR: {
            u16 count = list.size();
            Type t2 = list.size() == 0 ? UNKOWN : list[0]->type;
            sfwrite(&t2, 1);
            sfwrite(&count, 2);
            for(DataEntry* de : list)
                de->writeWithoutTag(out);
        }
            break;
        case MAP: {
            u16 size = dict.size();
            sfwrite(&size, 2);
            for(const std::pair<string, DataEntry*> p : dict) {
                size = p.first.size();
                sfwrite(&size, 2);
                sfwrite(&p.first[0], size);
                p.second->writeBinary(out);
            }
        }
            break;
        case BYTES:
            ERR_EXIT("WIP");
            break;
        case ALLOC:
            ERR_EXIT("WIP");
            break;
        case ZIPPED:
            ERR_EXIT("WIP"); 
            break;
        case PAIR:
            tuple.first->writeBinary(out);
            tuple.second->writeBinary(out);
            break;
        case TUPLE3:
            tuple.first->writeBinary(out);
            tuple.second->writeBinary(out);
            tuple.third->writeBinary(out);
            break;
        }

#undef sfwrite
}

DataEntry* DataEntry::readWithoutTag(FILE *in, Type t) {
    #define sfread(ptr, size) do{\
        bytesRead = fread(ptr, size, 1, in);\
        if(bytesRead != 1) {\
            delete de;\
            DataEntry* de_err = new DataEntry(ERROR);\
            de_err->error = { (u32)-1, (u32)-1, "Unexpected end of input" };\
            return de_err;\
        }\
    }while(0)

    i32 bytesRead;
    DataEntry* de = new DataEntry(t);
    switch(t) {
        case UNKOWN:
        case ERROR:
        case LEAF:
        case TYPE_COUNT:
            return de;
            break;
        case INT8:
        case UINT8:
        case CHAR:
            sfread(&de->integers.int8, 1);
            break;
        case INT16:
        case UINT16:
            sfread(&de->integers.int16, 2);
            break;
        case INT32:
        case UINT32:
        case FLOAT32:
            sfread(&de->integers.int32, 4);
            break;
        case INT64:
        case UINT64:
        case FLOAT64:
            sfread(&de->integers.int64, 8);
            break;
        case STRING:
        case STR_SLICE: {
            u16 size;
            sfread(&size, 2);
            de->str.resize(size);
            sfread(&de->str[0], size);
        }
            break;
        case LIST: {
            u16 count;
            sfread(&count, 2);
            de->list.reserve(count);
            for(u16 i=0; i<count; i++) {
                DataEntry* child = readBinary(in);
                if(child->type == ERROR) {
                    delete de;
                    return child;
                }
                de->list.push_back(child);
            }
        }
            break;
        case VECTOR: {
            u16 count;
            Type t2;
            sfread(&t2, 1);
            sfread(&count, 2);
            de->list.reserve(count);
            for(u16 i=0; i<count; i++) {
                DataEntry* child = readWithoutTag(in, t2);
                if(child->type == ERROR) {
                    delete de;
                    return child;
                }
                de->list.push_back(child);
            }
        }
            break;
        case MAP: {
            u16 entries;
            sfread(&entries, 2);
            string key;
            for(u16 i=0; i<entries; i++) {
                u16 keysize;
                sfread(&keysize, 2);
                key.resize(keysize);
                sfread(&key[0], keysize);
                DataEntry* value = readBinary(in);
                if(value->type == ERROR) {
                    delete de;
                    return value;
                }
                de->dict[key] = value;
            }
        }
            break;
        case BYTES:
            ERR_EXIT("WIP");
            break;
        case ALLOC:
            ERR_EXIT("WIP");
            break;
        case ZIPPED:
            ERR_EXIT("WIP"); 
            break;
        case PAIR:
            de->tuple.first = readBinary(in);
            de->tuple.second = readBinary(in);
            break;
        case TUPLE3:
            de->tuple.first = readBinary(in);
            de->tuple.second = readBinary(in);
            de->tuple.third = readBinary(in);
            break;
        }
    return de;
    #undef sfread
}

DataEntry* DataEntry::readBinary(FILE *in) {
    Type t;
    fread(&t, 1, 1, in);
    return readWithoutTag(in, t);
}

DataEntry* DataEntry::readFile(const string &filename) {
    string file1 = filename + ".td";
    if(fileExists(file1.c_str()))
        return readText(readFileString(filename.c_str()));
    file1[file1.size()-2] = 'b';
    FILE* fin = fopen(file1.c_str(), "rb");
    if(fin == nullptr) return nullptr;
    DataEntry* d = readBinary(fin);
    fclose(fin);
    return d;
}