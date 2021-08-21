#include <iostream>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "base64.hpp"

#include "TMXParser.h"

void zstd_test();
void zstd_decompress1(const char* src);

int main()
{
    zstd_test();
    return 0;
}

void zstd_test()
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_file("G:/Projects/Tiled_Maps/json/data/tset.tmx");
    if(!res)
    {
        std::cerr << res.description() << std::endl;
        return;
    }
    auto root = doc.child("map");
    if(!root)
    {
        std::cerr << "No Root.\n";
        return;
    }
    auto current = root;
    if(!(current = root.find_child_by_attribute("group","name","Tiles")))
    {
        std::cerr << "Can't find needed node.\n";
        return;
    }
    if(!(current = current.child("layer").child("data")))
    {
        std::cerr << "Can't find data node.\n";
        return;
    }
    auto parent = current.parent();
    std::size_t expectedSize = parent.attribute("width").as_uint()*parent.attribute("height").as_uint();
    std::string data = current.text().get();
    std::stringstream ss;
    ss << data;
    ss >> data;
    data = base64_decode(data);
    std::vector<unsigned char> dest;
    dest.reserve(expectedSize);

    zstd_decompress1(data.c_str());
    printf("&s correctly decoded (in memory). \n", data.c_str());
}

void zstd_decompress1(const char* src)
{
    size_t cSize = strlen(src);
    void* const cBuff = malloc(cSize);
    unsigned long long const rSize = ZSTD_getFrameContentSize(cBuff,cSize);
    CHECK(rSize != ZSTD_CONTENTSIZE_ERROR, "%s: not compressed by zstd!",src);
    CHECK(rSize != ZSTD_CONTENTSIZE_UNKNOWN,"%s: original size unknown!", src);
    void* const rBuff = malloc(rSize);
    size_t const dSize = ZSTD_decompress(rBuff,rSize,cBuff,cSize);
    CHECK_ZSTD(dSize);
    CHECK(dSize == rSize, "Impossible because zstd will check this condition!");
    printf("%25s : %6u -> %7u \n",src,(unsigned)cSize,(unsigned)rSize);
    free(rBuff);
    free(cBuff);
}
