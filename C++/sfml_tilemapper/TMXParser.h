#ifndef TMXPARSER_H
#define TMXPARSER_H

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <sstream>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <pugixml.hpp>
#include <zlib.h>
#include <zconf.h>
#include <zstd.h>
#include <common.h>
#include "base64.hpp"
#include <iostream>

std::string getErrorCodeMsg(int code)
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

using Attribute = std::unordered_multimap<std::string,std::string>;

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

struct LayerTile
{
    Attribute attribute;
};

struct Data
{
    using Encoding = enum{XML,BASE64,CSV};
    using Compression = enum{None,gzip,zlib,zstd};
    std::vector<LayerTile> tiles;
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
    std::vector<unsigned> base64_tmx_decode_uncompressed(const std::string& encoded_string);
    bool decompress_zlib(const char* source, std::vector<unsigned char>& dest, std::size_t inSize, std::size_t expectedSize);
    std::vector<unsigned> base64_tmx_decode_zlib(const std::string& encoded_string, std::size_t expectedSize);
    bool decompress_gzip(const std::string& compressedBytes, std::string& uncompressedBytes);
    std::vector<unsigned> base64_tmx_decode_gzip(const std::string& encoded_string, std::size_t expectedSize);

    bool checkNode(const pugi::xml_node& node, const std::string& name);
    Attribute parseAttributes(const pugi::xml_node& node);

    Group parseGroup(const pugi::xml_node& node);

    Property parseProperties(const pugi::xml_node& node);
    ImageLayer parseImageLayer(const pugi::xml_node& node);
    Layer parseLayer(const pugi::xml_node& node);
    ObjectGroup parseObjectGroup(const pugi::xml_node& node);
    Tileset parseTileset(const pugi::xml_node& node, const std::string &tmxPath);

    std::vector<LayerTile> decryptLayerTile(const Data& data, std::size_t expectedSize);
    Tileset parseTSXFile(const std::string& path,const std::string& tmxBasePath);
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

void TMXParser::outputWangset(const std::vector<WangSet> &wset)
{
    for(const auto& i: wset)
    {
        outputAttribute(i.attribute);
        std::cout << std::endl;
        outputProperty(i.properties);
        std::cout << std::endl;
        for(const auto& j: i.wangedgecolors)
        {
            outputAttribute(j.attribute);
        }
        std::cout << std::endl;
        for(const auto& j: i.wangcornercolors)
        {
            outputAttribute(j.attribute);
        }
        std::cout << std::endl;
        for(const auto& j: i.wangtiles)
        {
            outputAttribute(j.attribute);
        }
    }
}

void TMXParser::outputAnimation(const Animation &anim)
{
    for(const auto& i: anim.frames)
    {
        for(const auto& j: i.attribute)
        {
            std::cout << j.first << " = " << j.second << " ";
        }
    }
}

void TMXParser::outputProperty(const std::vector<Property> &prop)
{
    for(const auto& i: prop)
    {
        if(!i.pc_data.empty())
        {
            std::cout << "PC Data: " << i.pc_data << std::endl;
        }
        outputAttribute(i.attribute);
    }
}

void TMXParser::outputAttribute(const Attribute &attrib)
{
    for(const auto& i: attrib)
    {
        std::cout << i.first << " = " << i.second << " ";
    }
}

void TMXParser::Test_outputTileset()
{
    for(const auto& i: _map.tilesets)
    {
        std::cout << "Tileset Properties:\n\t";
        outputProperty(i.properties);
        std::cout << "\nImage:\n\tAttributes:\n\t";
        outputAttribute(i.image.attribute);
        std::cout << "\n\tProperties:\n\t";
        outputProperty(i.image.properties);
        std::cout << "\nTerrain Types:\n\t";
        for(const auto& j: i.terrains.types)
        {
            std::cout << "Attributes:\n\t";
            outputAttribute(j.attribute);
            std::cout << "\n\tProperties:\n\t";
            outputProperty(j.properties);
        }
        std::cout << "\nTiles:\n";
        for(const auto& j: i.tiles)
        {
            std::cout << "\tAttributes:\n\t";
            outputAttribute(j.attribute);
            std::cout << "\n\tProperties:\n\t";
            outputProperty(j.properties);
            std::cout << "\n\tAnimations:\n\t";
            outputAnimation(j.animations);
            std::cout << "\n";
        }
        std::cout << "\nWang Sets:\n\t";
        outputWangset(i.wangsets);
    }
    std::cout << "\nEnd of Tileset\n";
}

Tileset TMXParser::parseTSXFile(const std::string &path, const std::string &tmxBasePath)
{
    std::size_t found = tmxBasePath.find_last_of("/\\");
    if(found == std::string::npos)
    {
        _error = true;
        _errorMsg = "Could not base filepath for TSX file.";
    }
    std::string buffer = tmxBasePath.substr(0,found+1);
    buffer += path;
    return parseTSXFile(buffer);
}

Tileset TMXParser::parseTSXFile(const std::string &path)
{
    Tileset tiles;
    std::vector<Tileset> rval;
    pugi::xml_document doc;
    _result = doc.load_file(path.c_str());
    if(!_result)
    {
        _error = true;
        _errorMsg = _result.description();
    }
    else
    {
        if(!doc.child("tileset"))
        {
            _error = true;
            _errorMsg = "Node note found. Expected node: tileset.";
        }
        else
        {
            auto root = doc.child("tileset");
            for(const auto& attrib: root.attributes())
            {
                std::string name = attrib.name();
                if(name != "version" && name != "tiledversion")
                {
                    auto pair = std::make_pair(attrib.name(),attrib.value());
                    tiles.attribute.insert(pair);
                }
            }
            for(const auto& child: root.children())
            {
                std::string name = child.name();
                if(name == "image")
                {
                    Image img;
                    img.attribute = parseAttributes(child);
                    tiles.image = img;
                }
                if(name == "tile")
                {
                    Tile t;
                    t.attribute = parseAttributes(child);
                    for(const auto& tile: child.children())
                    {
                        std::string tname = tile.name();
                        if(tname == "animation")
                        {
                            Animation animation;
                            for(const auto& atile: tile.children())
                            {
                                Frame frame;
                                frame.attribute = parseAttributes(atile);
                                animation.frames.push_back(frame);
                            }
                            t.animations = animation;
                        }
                        if(tname == "properties")
                        {
                            t.properties.push_back(parseProperties(tile));
                        }
                    }
                    tiles.tiles.push_back(t);
                }
                if(name == "properties")
                {
                    tiles.properties.push_back(parseProperties(child));
                }
                if(name == "terraintypes")
                {
                    TerrainTypes ttypes;
                    for(const auto& tchild: child.children())
                    {
                        Terrain ter;
                        for(const auto& terrain: tchild.children())
                        {
                            std::string tname = terrain.name();
                            ter.attribute = parseAttributes(terrain);
                            if(tname == "properties")
                            {
                                ter.properties.push_back(parseProperties(terrain));
                            }
                        }
                        ttypes.types.push_back(ter);
                    }
                    tiles.terrains = ttypes;
                }
                if(name == "wangsets")
                {
                    for(const auto& wchild: child.children())
                    {
                        WangSet wset;
                        wset.attribute = parseAttributes(wchild);
                        for(const auto& wschild: wchild.children())
                        {
                            std::string wname = wschild.name();
                            if(wname == "wangedgecolor")
                            {
                                WangEdgeColor wec;
                                wec.attribute = parseAttributes(wschild);
                                wset.wangedgecolors.push_back(wec);
                            }
                            if(wname == "wangcornercolor")
                            {
                                WangCornerColor wcc;
                                wcc.attribute = parseAttributes(wschild);
                                wset.wangcornercolors.push_back(wcc);
                            }
                            if(wname == "wangtile")
                            {
                                WangTile wt;
                                wt.attribute = parseAttributes(wschild);
                                wset.wangtiles.push_back(wt);
                            }
                            if(wname == "properties")
                            {
                                wset.properties.push_back(parseProperties(wschild));
                            }
                        }
                        tiles.wangsets.push_back(wset);
                    }
                }
            }
        }
    }
    return tiles;
}

std::vector<LayerTile> TMXParser::decryptLayerTile(const Data &data, std::size_t expectedSize)
{
    std::vector<LayerTile> rval;
    if(expectedSize > 0)
    {
        if(data.compression == Data::Compression::zlib)
        {
            auto inflate = base64_tmx_decode_zlib(data.encrypted,expectedSize);
            LayerTile lt;
            for(const auto& inflated: inflate)
            {
               auto pair = std::make_pair("gid",std::string(std::to_string(inflated)));
               lt.attribute.insert(pair);
            }
            rval.push_back(lt);
        }
        else if(data.compression == Data::Compression::gzip)
        {
            auto inflate = base64_tmx_decode_gzip(data.encrypted,expectedSize);
            LayerTile lt;
            for(const auto& inflated: inflate)
            {
                auto pair = std::make_pair("gid",std::string(std::to_string(inflated)));
                lt.attribute.insert(pair);
            }
            rval.push_back(lt);
        }
        else if(data.compression == Data::Compression::zstd)
        {

        }
        else if(data.encoding == Data::BASE64 && data.compression == Data::Compression::None)
        {
            auto inflate = base64_tmx_decode_uncompressed(data.encrypted);
            LayerTile lt;
            for(unsigned i = 0; i < inflate.size(); ++i)
            {
                auto pair = std::make_pair("gid",std::string(std::to_string(inflate[i])));
                lt.attribute.insert(pair);
            }
            rval.push_back(lt);
        }
        else if(data.encoding == Data::Encoding::CSV)
        {

        }
    }
    return rval;
}

bool TMXParser::checkNode(const pugi::xml_node &node, const std::string &name)
{
    return (node.child(name.c_str())) ? true:false;
}

Attribute TMXParser::parseAttributes(const pugi::xml_node &node)
{
    Attribute rval;
    for(const auto& attrib: node.attributes())
    {
        auto pair = std::make_pair(attrib.name(),attrib.value());
        rval.insert(pair);
    }
    return rval;
}

Property TMXParser::parseProperties(const pugi::xml_node &node)
{
    Property prop;
    for(const auto& property: node.children())
    {
        prop.attribute = parseAttributes(property);
    }
    prop.pc_data = std::string(node.text().get());
    return prop;
}

ImageLayer TMXParser::parseImageLayer(const pugi::xml_node &node)
{
    ImageLayer il;
    il.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "image")
        {
            Image img;
            img.attribute = parseAttributes(child);
            il.images.push_back(img);
        }
        if(name == "properties")
        {
            il.properties.push_back(parseProperties(child));
        }
    }
    return il;
}

Layer TMXParser::parseLayer(const pugi::xml_node &node)
{
    Layer lyr;
    lyr.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            lyr.properties.push_back(parseProperties(child));
        }
        if(name == "data")
        {
            Data data;
            if(child.text().empty())
            {
               for(const auto& dchild: child.children())
               {
                   if(std::string(dchild.name()) == "tile")
                   {
                       LayerTile lt;
                       lt.attribute = parseAttributes(dchild);
                       data.tiles.push_back(lt);
                   }
               }
            }
            else
            {
                data.encrypted = std::string(child.text().get());
                for(const auto& attrib: child.attributes())
                {
                    if(std::string(attrib.name()) == "encoding")
                    {
                        if(std::string(attrib.value()) == "base64")
                        {
                            data.encoding = Data::Encoding::BASE64;
                        }
                        else if(std::string(attrib.value()) == "csv")
                        {
                            data.encoding = Data::Encoding::CSV;
                        }
                    }
                    if(std::string(attrib.name()) == "compression")
                    {
                        if(std::string(attrib.value()) == "zlib")
                        {
                            data.compression = Data::Compression::zlib;
                        }
                        else if(std::string(attrib.value()) == "gzip")
                        {
                            data.compression = Data::Compression::gzip;
                        }
                        else if(std::string(attrib.value()) == "zstd")
                        {
                            data.compression = Data::Compression::zstd;
                        }
                    }
                }
                std::size_t expectedSize = 0;
                if(node.attribute("width") && node.attribute("height"))
                {
                    expectedSize = node.attribute("width").as_uint() * node.attribute("height").as_uint();
                }
                data.tiles = decryptLayerTile(data,expectedSize);
            }
            lyr.encodingData = data;
        }
    }
    return lyr;
}

ObjectGroup TMXParser::parseObjectGroup(const pugi::xml_node &node)
{
    ObjectGroup rval;
    rval.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            rval.properties.push_back(parseProperties(child));
        }
        if(name == "object")
        {
            Object obj;
            obj.attribute = parseAttributes(child);
            for(const auto& cobj: child.children())
            {
                if(std::string(cobj.name()) == "properties")
                {
                    obj.properties.push_back(parseProperties(cobj));
                }
            }
            rval.objects.push_back(obj);
        }
    }
    return rval;
}

Group TMXParser::parseGroup(const pugi::xml_node &node)
{
    Group grp;
    grp.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            grp.properties.push_back(parseProperties(child));
        }
        if(name == "imagelayer")
        {
            grp.imagelayers.push_back(parseImageLayer(child));
        }
        if(name == "layer")
        {
            grp.layers.push_back(parseLayer(child));
        }
        if(name == "objectgroup")
        {
            grp.objectgroups.push_back(parseObjectGroup(child));
        }
    }
    return grp;
}

Tileset TMXParser::parseTileset(const pugi::xml_node &node, const std::string& tmxPath)
{
    Tileset rval;
    rval.attribute = parseAttributes(node);
    for(const auto& attrib: rval.attribute)
    {
        if(attrib.first == "source")
        {
            rval = parseTSXFile(attrib.second,tmxPath);
            break;
        }
    }
    if(rval.tiles.empty())
    {
        for(const auto& child: node.children())
        {
            std::string name = child.name();
            if(name == "properties")
            {
                rval.properties.push_back(parseProperties(child));
            }
            if(name == "image")
            {
                Image img;
                img.attribute = parseAttributes(child);
                rval.image = img;
            }
            if(name == "terraintypes")
            {
                TerrainTypes ttypes;
                for(const auto& tchild: child.children())
                {
                    Terrain ter;
                    ter.attribute = parseAttributes(tchild);
                    for(const auto& tprop: tchild.children())
                    {
                        ter.properties.push_back(parseProperties(tprop));
                    }
                    ttypes.types.push_back(ter);
                }
                rval.terrains = ttypes;
            }
            if(name == "tile")
            {
                Tile t;
                t.attribute = parseAttributes(child);
                for(const auto& tchild: child.children())
                {
                    std::string tname = tchild.name();
                    if(tname == "properties")
                    {
                        t.properties.push_back(parseProperties(tchild));
                    }
                    if(tname == "animation")
                    {
                        Animation anim;
                        for(const auto& achild: tchild.children())
                        {
                            Frame frame;
                            frame.attribute = parseAttributes(achild);
                            anim.frames.push_back(frame);
                        }
                        t.animations = anim;
                    }
                }
                rval.tiles.push_back(t);
            }
            if(name == "wangsets")
            {
                for(const auto& wchild: child.children())
                {
                    WangSet wset;
                    wset.attribute = parseAttributes(wchild);
                    for(const auto& wschild: wchild.children())
                    {
                        std::string wname = wschild.name();
                        if(wname == "wangedgecolor")
                        {
                            WangEdgeColor wec;
                            wec.attribute = parseAttributes(wschild);
                            wset.wangedgecolors.push_back(wec);
                        }
                        if(wname == "wangcornercolor")
                        {
                            WangCornerColor wcc;
                            wcc.attribute = parseAttributes(wschild);
                            wset.wangcornercolors.push_back(wcc);
                        }
                        if(wname == "wangtile")
                        {
                            WangTile wt;
                            wt.attribute = parseAttributes(wschild);
                            wset.wangtiles.push_back(wt);
                        }
                        if(wname == "properties")
                        {
                            wset.properties.push_back(parseProperties(wschild));
                        }
                    }
                    rval.wangsets.push_back(wset);
                }
            }
        }
    }
    return rval;
}

std::string TMXParser::resolveFilePath(std::string path, const std::string &workingDir)
{
    static const std::string match("../");
    std::size_t result = path.find(match);
    std::size_t count = 0;
    while (result != std::string::npos)
    {
        count++;
        path = path.substr(result + match.size());
        result = path.find(match);
    }

    if (workingDir.empty()) return path;

    std::string outPath = workingDir;
    for (auto i = 0u; i < count; ++i)
    {
        result = outPath.find_last_of('/');
        if (result != std::string::npos)
        {
            outPath = outPath.substr(0, result);
        }
    }
    return std::move(outPath += '/' + path);
}

std::vector<unsigned> TMXParser::base64_tmx_decode_uncompressed(const std::string& encoded_string)
{
    std::vector<unsigned> result;
    std::string data = encoded_string;
    std::stringstream ss;
    ss << data;
    ss >> data;
    std::string decoded = base64_decode(data);
    std::vector<unsigned char> byteArray;

    for(const auto& i: decoded)
    {
        byteArray.push_back(i);
    }

    for(unsigned i = 0u; i < byteArray.size() - 3; i += 4)
    {
        result.push_back(byteArray[i] | byteArray[i+1] << 8 | byteArray[i+2] << 16 | byteArray[i+3] << 24);
    }
    return result;
}

bool TMXParser::decompress_zlib(const char* source, std::vector<unsigned char>& dest, std::size_t inSize, std::size_t expectedSize)
{
    if (!source)
    {
        return false;
    }

    int currentSize = static_cast<int>(expectedSize);
    std::vector<unsigned char> byteArray(expectedSize / sizeof(unsigned char));
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = (Bytef*)source;
    stream.avail_in = static_cast<unsigned int>(inSize);
    stream.next_out = (Bytef*)byteArray.data();
    stream.avail_out = static_cast<unsigned int>(expectedSize);

    if (inflateInit(&stream) != Z_OK)
    {
        return false;
    }

    int result = 0;
    do
    {
        result = inflate(&stream, Z_SYNC_FLUSH);

        switch (result)
        {
        case Z_NEED_DICT:
        case Z_STREAM_ERROR:
            result = Z_DATA_ERROR;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&stream);
            return false;
        }

        if (result != Z_STREAM_END)
        {
            int oldSize = currentSize;
            currentSize *= 2;
            std::vector<unsigned char> newArray(currentSize / sizeof(unsigned char));
            std::memcpy(newArray.data(), byteArray.data(), currentSize / 2);
            byteArray = std::move(newArray);

            stream.next_out = (Bytef*)(byteArray.data() + oldSize);
            stream.avail_out = oldSize;

        }
    } while (result != Z_STREAM_END);

    if (stream.avail_in != 0)
    {
        return false;
    }

    const int outSize = currentSize - stream.avail_out;
    inflateEnd(&stream);

    std::vector<unsigned char> newArray(outSize / sizeof(unsigned char));
    std::memcpy(newArray.data(), byteArray.data(), outSize);
    byteArray = std::move(newArray);

    //copy bytes to vector
    dest.insert(dest.begin(), byteArray.begin(), byteArray.end());

    return true;
}

std::vector<unsigned> TMXParser::base64_tmx_decode_zlib(const std::string& encoded_string, std::size_t expectedSize)
{
    std::vector<unsigned> result;
    if(!encoded_string.empty())
    {
        std::string data = encoded_string;
        std::stringstream ss;
        ss << data;
        ss >> data;
        data = base64_decode(data);
        std::vector<unsigned char> dest;
        dest.reserve(expectedSize);
        size_t dataSize = data.length() * sizeof(unsigned char);
        if(decompress_zlib(data.c_str(),dest,dataSize,expectedSize))
        {
            result.reserve(expectedSize/4);
            for(auto i = 0u; i < expectedSize - 3u; i += 4u)
            {
                unsigned id = dest[i] | dest[i+1] << 8 | dest[i+2] << 16 | dest[i+3] << 24;
                result.push_back(id);
            }
        }
    }
    return result;
}

bool TMXParser::decompress_gzip(const std::string &compressedBytes, std::string &uncompressedBytes)
{
    if ( compressedBytes.size() == 0 ) {
      uncompressedBytes = compressedBytes ;
      return true ;
    }

    uncompressedBytes.clear() ;

    unsigned full_length = compressedBytes.size() ;
    unsigned half_length = compressedBytes.size() / 2;

    unsigned uncompLength = full_length ;
    char* uncomp = (char*) calloc( sizeof(char), uncompLength );

    z_stream strm;
    strm.next_in = (Bytef *) compressedBytes.c_str();
    strm.avail_in = compressedBytes.size() ;
    strm.total_out = 0;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;

    bool done = false ;
    auto res = inflateInit2(&strm, (16+MAX_WBITS));
    if (res != Z_OK) {
      free( uncomp );
      return false;
    }

    while (!done) {
      // If our output buffer is too small
      if (strm.total_out >= uncompLength ) {
        // Increase size of output buffer
        char* uncomp2 = (char*) calloc( sizeof(char), uncompLength + half_length );
        memcpy( uncomp2, uncomp, uncompLength );
        uncompLength += half_length ;
        free( uncomp );
        uncomp = uncomp2 ;
      }

      strm.next_out = (Bytef *) (uncomp + strm.total_out);
      strm.avail_out = uncompLength - strm.total_out;

      // Inflate another chunk.
      int err = inflate (&strm, Z_SYNC_FLUSH);
      if (err == Z_STREAM_END) done = true;
      else if (err != Z_OK)  {
        break;
      }
    }

    if (inflateEnd (&strm) != Z_OK) {
      free( uncomp );
      return false;
    }

    for ( size_t i=0; i<strm.total_out; ++i ) {
      uncompressedBytes += uncomp[ i ];
    }

    free( uncomp );
    return true ;
}

std::vector<unsigned> TMXParser::base64_tmx_decode_gzip(const std::string& encoded_string, std::size_t expectedSize)
{
    std::vector<unsigned> result;
    if(!encoded_string.empty())
    {
        std::string data = encoded_string;
        std::stringstream ss;
        ss << data;
        ss >> data;
        data = base64_decode(data);
        std::string outString;
        if(decompress_gzip(data,outString))
        {
            result.reserve(expectedSize/4);
            for(auto i = 0u; i < expectedSize - 3u; i += 4u)
            {
                unsigned id = outString[i] | outString[i+1] << 8 | outString[i+2] << 16 | outString[i+3] << 24;
                result.push_back(id);
            }
        }
    }
    return result;
}

bool TMXParser::load(const std::string& tmxPath, const std::string &tsxPath)
{
    if(_error)
    {
        _error = false;
        _errorMsg.clear();
    }

    _result = _doc.load_file(tmxPath.c_str());
    if(!_result)
    {
        _error = true;
        _errorMsg = _result.description();
        return false;
    }
    auto root = _doc.child("map");
    if(!root)
    {
        _error = true;
        _errorMsg = "Could not parse document. Node not found: map";
        return false;
    }
    auto current = root;
    for(const auto& attr: current.attributes())
    {
        auto pair = std::make_pair(attr.name(),attr.value());
        _map.attribute.insert(pair);
    }
    for(const auto& child: root.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            _map.properties.push_back(parseProperties(child));
        }
        if(name == "group")
        {
            _map.groups.push_back(parseGroup(child));
        }
        if(name == "tileset")
        {
            if(tsxPath.empty())
            {
                _map.tilesets.push_back(parseTileset(child,tmxPath));
                if(_error)
                {
                    return false;
                }
            }
            else
            {
                _map.tilesets.push_back(parseTSXFile(tsxPath));
                if(_error)
                {
                    return false;
                }
            }
        }
        if(name == "imagelayer")
        {
            _map.imagelayers.push_back(parseImageLayer(child));
        }
        if(name == "layer")
        {
            _map.layers.push_back(parseLayer(child));
        }
        if(name == "objectgroup")
        {
            _map.objectgroups.push_back(parseObjectGroup(child));
        }
    }
    return true;
}

#endif // TMXPARSER_H
