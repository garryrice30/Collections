#ifndef TMXPARSER_H
#define TMXPARSER_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <pugixml.hpp>
#include <zlib.h>
#include <zstd.h>


/**
 * Turns a zlib return code into a readable string.
 * @param code value returned by inflate()/inflateInit().
 * @return name of the code.
 */
inline std::string getErrorCodeMsg(int code)
{
    std::string rval;

    switch(code)
    {
    case Z_STREAM_END:
        rval = "Z_STREAM_END";
        break;
    case Z_NEED_DICT:
        rval = "Z_NEED_DICT";
        break;
    case Z_ERRNO:
        rval = "Z_ERRNO";
        break;
    case Z_STREAM_ERROR:
        rval = "Z_STREAM_ERROR";
        break;
    case Z_DATA_ERROR:
        rval = "Z_DATA_ERROR";
        break;
    case Z_MEM_ERROR:
        rval = "Z_MEM_ERROR";
        break;
    case Z_BUF_ERROR:
        rval = "Z_BUF_ERROR";
        break;
    case Z_VERSION_ERROR:
        rval = "Z_VERSION_ERROR";
        break;
    default:
        rval = "Z_OK";
        break;
    };

    return rval;
}

using Attribute = std::unordered_multimap<std::string, std::string>;

struct Property
{
    Attribute attribute;
    std::string pc_data;
};

struct Terrain
{
    Attribute attribute;
    std::vector<Property> properties;
};

struct TerrainTypes
{
    std::vector<Terrain> types;
};

struct Image
{
    Attribute attribute;
    std::vector<Property> properties;
};

struct ImageLayer
{
    Attribute attribute;
    std::vector<Image> images;
    std::vector<Property> properties;
};

struct WangTile
{
    Attribute attribute;
};

struct WangCornerColor
{
    Attribute attribute;
};

struct WangEdgeColor
{
    Attribute attribute;
};

struct WangSet
{
    Attribute attribute;
    std::vector<WangEdgeColor> wangedgecolors;
    std::vector<WangCornerColor> wangcornercolors;
    std::vector<WangTile> wangtiles;
    std::vector<Property> properties;
};

struct Object
{
    Attribute attribute;
    std::vector<Property> properties;
};

struct ObjectGroup
{
    std::vector<Object> objects;
    Attribute attribute;
    std::vector<Property> properties;
};

struct Frame
{
    Attribute attribute;
};

struct Animation
{
    std::vector<Frame> frames;
};

struct Tile
{
    Animation animations;
    Attribute attribute;
    std::vector<Property> properties;
};

struct Tileset
{
    Attribute attribute;
    Image image;
    std::vector<Tile> tiles;
    std::vector<WangSet> wangsets;
    std::vector<Property> properties;
    TerrainTypes terrains;
};

/**
 * @brief A single cell of a layer.
 *
 * Every supported encoding ends up in the same representation: one
 * LayerTile per cell, stored in Layer::encodingData.tiles in reading order
 * (left to right, top to bottom). The cell's index inside that vector is the
 * cell's position in the map, so the order of the map is never destroyed.
 * For XML encoded layers the XML attributes of the <tile> element are kept,
 * for every other encoding the gid attribute is filled in.
 */
struct LayerTile
{
    Attribute attribute;

    /**
     * @return the gid stored for this cell, or 0 when the cell has none.
     * @note Tiled stores the flip/rotation flags in the top four bits of the
     * gid, so mask them off with & 0x1FFFFFFFu if you only want the tile id.
     */
    unsigned gid() const
    {
        auto it = attribute.find("gid");
        if(it == attribute.end())
        {
            return 0u;
        }
        try
        {
            return static_cast<unsigned>(std::stoul(it->second));
        }
        catch(...)
        {
            return 0u;
        }
    }
};

struct Data
{
    using Encoding = enum {XML, BASE64, CSV};
    using Compression = enum {None, gzip, zlib, zstd};

    std::vector<LayerTile> tiles; /**<One entry per cell, in reading order.*/
    std::string encrypted;
    Encoding encoding = XML;
    Compression compression = None;
};

struct Layer
{
    Attribute attribute;
    Data encodingData;
    std::vector<Property> properties;
};

struct Group
{
    Attribute attribute;
    std::vector<Layer> layers;
    std::vector<ImageLayer> imagelayers;
    std::vector<ObjectGroup> objectgroups;
    std::vector<Property> properties;
};

struct Map
{
    std::vector<Tileset> tilesets;
    std::vector<Layer> layers;
    std::vector<ImageLayer> imagelayers;
    std::vector<ObjectGroup> objectgroups;
    std::vector<Property> properties;
    std::vector<Group> groups;
    Attribute attribute;
};

class TMXParser
{
    Map _map;
    bool _error = false;
    std::string _errorMsg;

    pugi::xml_document _doc;
    pugi::xml_parse_result _result;

    std::string resolveFilePath(std::string path, const std::string& workingDir);

    // Byte level helpers
    std::vector<unsigned char> base64_decode_bytes(const std::string& encoded_string);
    std::vector<unsigned> decode_gids(const std::vector<unsigned char>& bytes, std::size_t expectedSize);
    std::vector<LayerTile> toLayerTiles(const std::vector<unsigned>& gids);

    // Decompression. windowBits follows zlib's rules (MAX_WBITS for zlib, MAX_WBITS + 16 for gzip).
    bool inflate_stream(const unsigned char* source, std::size_t inSize, int windowBits,
                        std::vector<unsigned char>& dest, std::size_t expectedSize);
    bool decompress_zstd(const unsigned char* source, std::size_t inSize,
                         std::vector<unsigned char>& dest, std::size_t expectedSize);

    std::vector<unsigned> parse_csv_tiles(const std::string& text);

    bool checkNode(const pugi::xml_node& node, const std::string& name);
    Attribute parseAttributes(const pugi::xml_node& node);
    void appendProperties(std::vector<Property>& out, const pugi::xml_node& node);

    Group parseGroup(const pugi::xml_node& node);
    ImageLayer parseImageLayer(const pugi::xml_node& node);
    Layer parseLayer(const pugi::xml_node& node);
    ObjectGroup parseObjectGroup(const pugi::xml_node& node);
    Tileset parseTileset(const pugi::xml_node& node, const std::string& tmxPath);
    std::vector<LayerTile> decryptLayerTile(const Data& data, std::size_t expectedSize);
    Tileset parseTSXFile(const std::string& path, const std::string& tmxBasePath);
    Tileset parseTSXFile(const std::string& path);

    void outputAttribute(const Attribute& attrib);
    void outputProperty(const std::vector<Property>& prop);
    void outputAnimation(const Animation& anim);
    void outputWangset(const std::vector<WangSet>& wset);

public:
    TMXParser() = default;
    bool load(const std::string& tmxPath, const std::string& tsxPath = std::string());
    bool hasError() const {return _error;}
    std::string errorMsg() const {return _errorMsg;}
    Map getMap() const {return _map;}

    void Test_outputTileset();
    ~TMXParser() = default;
};

#endif // TMXPARSER_H
