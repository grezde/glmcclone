#pragma once
#include "base.hpp"
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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

        PAIR, // a pair of values
        TUPLE3, // a tuple of size 3

        BYTES, // list of bytes (count + bytes...)
        ALLOC, // a portion of the file which might be empty (size as u32 + inner value)
        ZIPPED, // a compressed tag (compression type + size as u32 + uncompressed size + inner value zipped)

        TYPE_COUNT
    };
    Type type;

    DataEntry(Type t);
    ~DataEntry();
    DataEntry* copy() const;
    static map<string, u8> enums;

    static DataEntry* readFile(const string& filename);

    static DataEntry* readFromText(const string& text, u32& index);
    static DataEntry* readText(const string& text);
    void prettyPrint(std::ostream& out, u32 indent = 0) const;

    static DataEntry* readBinary(FILE* in);
    static DataEntry* readWithoutTag(FILE* in, Type t);
    void writeWithoutTag(FILE* out);
    void writeBinary(FILE* out);

    i64 geti64() const;
    void seti64(i64 value);
    f64 getf64() const;
    void setf64(f64 value);

    inline bool isMap() const { return type == MAP; }
    inline bool isListable() const { return type == LIST || type == VECTOR; }
    inline bool isStringable() const { return type == STRING || type == STR_SLICE; }
    inline bool isInteger() const { return INT8 <= type && type <= UINT64; }
    inline bool isFloating() const { return INT8 <= type && type <= FLOAT64; }
    
    inline bool isIVEC2() const { return type == PAIR && tuple.first->isInteger() && tuple.second->isInteger(); }
    inline bool isIVEC3() const { return type == TUPLE3 && tuple.first->isInteger() && tuple.second->isInteger() && tuple.third->isInteger(); }
    inline ivec2 getIVEC2() const { return { (i32)tuple.first->geti64(), (i32)tuple.second->geti64() }; }
    inline ivec3 getIVEC3() const { return { (i32)tuple.first->geti64(), (i32)tuple.second->geti64(), (i32)tuple.third->geti64() }; }

    inline bool isVEC2() const { return type == PAIR && tuple.first->isFloating() && tuple.second->isFloating(); }
    inline bool isVEC3() const { return type == TUPLE3 && tuple.first->isFloating() && tuple.second->isFloating() && tuple.third->isFloating(); }
    inline vec2 getVEC2() const { return { tuple.first->getf64(), tuple.second->getf64() }; }
    inline vec3 getVEC3() const { return { tuple.first->getf64(), tuple.second->getf64(), tuple.third->getf64() }; }

    inline bool has(const string& key) const { return dict.find(key) != dict.end(); }
    inline DataEntry* child(const string& key) { return dict[key]; }
    inline DataEntry* schild(const string& key) { return has(key) ? child(key) : nullptr; }
    //inline bool operator==(const string& strValue) { return str == strValue; }
    void mergeStructure(const DataEntry* de);

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
            u32 row;
            u32 col;
            string message;
        } error;
        struct {
            DataEntry* first;
            DataEntry* second;
            DataEntry* third;
        } tuple;
        vector<u8> bytes;
    };

};