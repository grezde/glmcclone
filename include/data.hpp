#pragma once
#include "base.hpp"

struct DataEntry {
    enum CompressionType: u8 {
        UNCOMPRESSED = 0,
        RLE = 1,
        GZIP = 2
    };

    enum Type: u8 {
        UNKOWN = 0,
        ERROR = 1,
        LEAF = 2, // empty

        // integer and float types
        INT8,
        INT16,
        INT32,
        INT64,
        UINT8,
        UINT16,
        UINT32,
        UINT64,
        FLOAT32,
        FLOAT64,

        CHAR,
        STRING, // null terminated string
        STR_SLICE, // count + string (not null terminated)

        LIST, // list of values (count as u32 + values...)
        VECTOR, // list of values of the same type (count as u32 + type + values...) 
        MAP, // a map/dictionary/object (count + (key as null terminated string + value)...)

        BYTES, // list of bytes (count + bytes...)
        ALLOC, // a portion of the file which might be empty (size as u32 + inner value)
        ZIPPED, // a compressed tag (compression type + size as u32 + uncompressed size + inner value zipped)

        TYPE_COUNT
    };
    Type type;

    DataEntry(Type t);
    ~DataEntry();

    static DataEntry* readFile(const string& filename);

    static DataEntry* readFromText(const string& text, u32& index);
    static DataEntry* readText(const string& text);
    void prettyPrint(std::ostream& out, u32 indent = 0) const;

    static DataEntry* readBinary(FILE* in);
    static DataEntry* readWithoutTag(FILE* in, Type t);
    void writeWithoutTag(FILE* out);
    void writeBinary(FILE* out);


    i64 geti64();
    void seti64(i64 value);
    f64 getf64();
    void setf64(f64 value);

    inline bool isMap() { return type == MAP; }
    inline bool isListable() { return type == LIST || type == VECTOR; }
    inline bool isStringable() { return type == STRING || type == STR_SLICE; }
    
    inline bool has(const string& key) { return dict.find(key) != dict.end(); }
    inline DataEntry* child(const string& key) { return dict[key]; }
    inline DataEntry* schild(const string& key) { return has(key) ? child(key) : nullptr; }
    inline bool operator==(const string& strValue) { return str == strValue; }

    union {
        union {
            u8 uint8;
            i8 int8;
            u16 uint16;
            i16 int16;
            u32 uint32;
            i32 int32;
            u64 uint64;
            i64 int64;
            f32 float32;
            f64 float64;
        } integers;
        char character;
        string str;
        vector<DataEntry*> list;
        map<string, DataEntry*> dict;
        struct {
            u32 size;
            DataEntry* inner;
        } allocation;
        struct {
            CompressionType type;
            DataEntry* inner;
        } zipped;
        struct {
            u32 location;
            string message;
        } error;
        vector<u8> bytes;
    };

};