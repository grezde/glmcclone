#include "base.hpp"
#include "data.hpp"
#include <fstream>
#include <sstream>

void help() {
    cerr << "Usage: datatool command [...args]\n";
    cerr << "  merge [output file] [...input files] - merge 1 or more input files\n";
    cerr << "  get [file] [path] - get a map value out of a file\n";
    cerr << "  dejem [.jem file] [.td file] - converts a Blockbench .jem file into an entity model file\n";
    cerr << "  t2b [input file] [output file] - converts between text data and binary data representations\n";
    cerr << "  b2t [input file] [output file] - converts between text data and binary data representations\n";
    cerr << "  bundle [input folder] [output file] - bundle a folder into a binary data file\n";
}

#define CHECK_ARGSIZE(x) if(args.size() != x) { cerr << "ERROR: Command " << __func__ << " requires " << x << " arguments.\n"; help(); return 1; }

i32 b2t(vector<string>& args) {
    CHECK_ARGSIZE(2);
    FILE* fin = fopen(args[0].c_str(), "rb");
    DataEntry* de = DataEntry::readBinary(fin);
    if(de->type == DataEntry::ERROR)
        ERR_EXIT("PARSE ERROR: " << de->error.message);
    fclose(fin);
    std::ofstream fout(args[1].c_str());
    if(!fout.is_open()) ERR_EXIT("Cannot write into file " << args[1]);
    de->prettyPrint(fout);
    fout.close();
    return 0;
}

i32 t2b(vector<string>& args) {
    CHECK_ARGSIZE(2);
    DataEntry* de = DataEntry::readText(readFileString(args[0].c_str()));
    if(de->type == DataEntry::ERROR)
        ERR_EXIT("PARSE ERROR: " << de->error.message);
    FILE* fout = fopen(args[1].c_str(), "wb");
    if(!fout) ERR_EXIT("Cannot write into file " << args[1]);
    de->writeBinary(fout);
    fclose(fout);
    return 0;
}

i32 bundle(vector<string>& args) {
    (void) args;
    ERR_EXIT("Not yet implemented");
    return 0;
}

glm::ivec3 getFromListI3(DataEntry* de, u32 offset=0) {
    return { (i32)de->list[offset+0]->geti64(),(i32)de->list[offset+1]->geti64(), (i32)de->list[offset+2]->geti64() };
}

glm::ivec2 getFromListI2(DataEntry* de, u32 offset=0) {
    return { (i32)de->list[offset+0]->geti64(),(i32)de->list[offset+1]->geti64() };
}

DataEntry* makeTupleI2(glm::ivec2 i) {
    DataEntry* dex = new DataEntry(DataEntry::INT64); dex->seti64(i.x);
    DataEntry* dey = new DataEntry(DataEntry::INT64); dey->seti64(i.y);
    DataEntry* fin = new DataEntry(DataEntry::PAIR);
    fin->tuple.first = dex;
    fin->tuple.second = dey;
    return fin;
}

DataEntry* makeTupleI3(glm::ivec3 i) {
    DataEntry* dex = new DataEntry(DataEntry::INT64); dex->seti64(i.x);
    DataEntry* dey = new DataEntry(DataEntry::INT64); dey->seti64(i.y);
    DataEntry* dez = new DataEntry(DataEntry::INT64); dez->seti64(i.z);
    DataEntry* fin = new DataEntry(DataEntry::TUPLE3);
    fin->tuple.first = dex;
    fin->tuple.second = dey;
    fin->tuple.third = dez;
    return fin;
}

DataEntry* makeString(const string& str) {
    DataEntry* de = new DataEntry(DataEntry::STRING);
    de->str = str;
    return de;
}

i32 dejem(vector<string>& args) {
    CHECK_ARGSIZE(2);
    DataEntry* in = DataEntry::readText(readFileString(args[0].c_str()));
    if(in->type == DataEntry::ERROR)
        ERR_EXIT("PARSE ERROR: " << in->error.message);
    in = in->schild("models");
    if(!in) return 1;

    DataEntry* out = new DataEntry(DataEntry::MAP);
    DataEntry* cuboids = new DataEntry(DataEntry::MAP);
    DataEntry* objects = new DataEntry(DataEntry::MAP);
    out->dict["cuboids"] = cuboids;
    out->dict["objects"] = objects;

    for(DataEntry* part : in->list) {
        string name = part->schild("part")->str;
        i32 i=0;
        glm::ivec3 translate = getFromListI3(part->schild("translate"));
        DataEntry* object = new DataEntry(DataEntry::MAP);
        objects->dict[name] = object;
        object->dict["pivot"] = makeTupleI3(translate);
        DataEntry* objectCuboids = new DataEntry(DataEntry::LIST);
        object->dict["cuboids"] = objectCuboids;

        for(DataEntry* box : part->schild("boxes")->list) {
            string boxname = part->schild("boxes")->list.size() == 1 ? name : (std::ostringstream() << name << "_" << (i++)).str();
            glm::ivec2 uv = getFromListI2(box->schild("textureOffset"));
            glm::ivec3 pos = getFromListI3(box->schild("coordinates"), 0);
            glm::ivec3 dims = getFromListI3(box->schild("coordinates"), 3);

            DataEntry* cuboid = new DataEntry(DataEntry::MAP);
            cuboid->dict["dims"] = makeTupleI3(dims);
            cuboid->dict["uv"] = makeTupleI2({ uv.x + dims.x, uv.y + dims.x });
            cuboids->dict[boxname] = cuboid;

            DataEntry* appcuboid = new DataEntry(DataEntry::MAP);
            appcuboid->dict["id"] = makeString(boxname);
            appcuboid->dict["pos"] = makeTupleI3(pos);
            objectCuboids->list.push_back(appcuboid);
            
        }
    }

    std::ofstream fout(args[1].c_str());
    if(!fout.is_open()) ERR_EXIT("Cannot write into file " << args[1]);
    out->prettyPrint(fout);
    fout.close();
    return 0;
}

int main(int argc, const char** argv) {
    vector<string> args;
    for(i32 i=1; i<argc; i++)
        args.push_back(argv[i]);
    if(args.size() == 0) {
        cerr << "ERROR: No arguments provided.\n";
        help();
        return 1;
    }

    #define CHECK(name) else if(args[0] == #name) { args.erase(args.begin()); return name(args); } 
    if(false) {}
    CHECK(t2b)
    CHECK(b2t)
    CHECK(bundle)
    CHECK(dejem)
    #undef CHECK
    
    cerr << "ERROR: Program requires a command.\n"; 
    help();
    return 1;
}