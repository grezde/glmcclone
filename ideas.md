
# Ideas

## Block data enum

Block data enums are u8 enumerations that are attached to blocks.
For example, block enums include: 
- `colors`: colored blocks (like wool, concrete, beds)
- `light_colors`: colored light-emitting blocks (like lamps) 
- `wood_variants`: what wood variants to use (like wood)
- `orientations`: which orientation does the block face (like pistons)
- `fullness`: used in tiny cubes to select the shape (like the chese block in april fools 2023)
- `pallete`: used in tiny cubes to select the texture
A block can have multiple enums: beds have `wood_variant` and `colors` enums, a storage drawer (like in the mod) may have 2 `wood_variant` enums. A tiny block may have a `fullness` and a `pallete` enum.

## Binary JSON-style format

```c++

enum Tag {
    UNKOWN,
    ERROR,
    NULL,

    U8,
    U16,
    U32,
    U64,
    U128,
    I8,
    I16,
    I32,
    I64,
    I128,
    F16,
    F32,
    F64,
    F128,

    NULL_STRING, // + null-terminated string
    BYTES,  // + size as u32
    STRING, // + size as u16 + string
    ARRAY,  // + length as u16 + nodes
    VECTOR, // + length as u16 + element tag + nodes without tag
    MAP, // + entry count as u16 + entries
        // for each entry: string without the tag + value node
    
    ALLOC, // + size as u32 + inner node, the rest is 0s
    ZIPPED, // + compression_type as u8 + size as u32 + compressed tag as bytes
    
};

```

## Text representation of that

```json
// comments are allowed
{
    // STANDARD JSON:
    "d": 1, // i32
    "fg": 1.0, // f32
    "a": "bdf", // strings
    "b": ["sff", "fjsfjs", "sjdfsk"], // arrays
    "aa": { "a": 1 }, // maps

    // MODIFICATIONS:
    "e": 0x0f0f0f12378abcff0294, // bytes
    dksjlf: 2, // names in maps can be quoted or unquoted, 
        // if they begin with a number or any illegal characters, then they must be quoted
    
    // EXTRA TYPES:
    "jf": <u16> 12, // write type before the literal for specific type
    "q": <nullstr> "adfsd", // null terminated string
    "c": <vec> ["adfs", "adfs", "sjffsjk"], // vectors
    "f": <alloc, 100> { "a": "b" }, // allocation of specific bytes
    "g": <zip, gzip> { "a": "qqqq" }, // compression
}
```
