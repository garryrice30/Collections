#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "TMXParser.h"

namespace
{

std::string encodingName(Data::Encoding encoding)
{
    switch(encoding)
    {
    case Data::Encoding::BASE64:
        return "base64";
    case Data::Encoding::CSV:
        return "csv";
    case Data::Encoding::XML:
    default:
        return "xml";
    }
}

std::string compressionName(Data::Compression compression)
{
    switch(compression)
    {
    case Data::Compression::gzip:
        return "gzip";
    case Data::Compression::zlib:
        return "zlib";
    case Data::Compression::zstd:
        return "zstd";
    case Data::Compression::None:
    default:
        return "none";
    }
}

std::vector<std::pair<std::string, std::string>> sortedAttributes(const Attribute& attribute)
{
    std::vector<std::pair<std::string, std::string>> values(attribute.begin(), attribute.end());
    std::sort(values.begin(), values.end());
    return values;
}

void printAttributes(const Attribute& attribute, const std::string& indent)
{
    for(const auto& value: sortedAttributes(attribute))
    {
        std::cout << indent << value.first << '=' << value.second << '\n';
    }
}

void printProperties(const std::vector<Property>& properties, const std::string& indent)
{
    for(const auto& property: properties)
    {
        std::string name;
        std::string value;
        for(const auto& attribute: property.attribute)
        {
            if(attribute.first == "name") name = attribute.second;
            if(attribute.first == "value") value = attribute.second;
        }
        if(!property.pc_data.empty())
        {
            value = property.pc_data;
        }
        std::cout << indent << name << '=' << value << '\n';
    }
}

void printTileset(const Tileset& tileset, std::size_t index)
{
    std::string firstGid;
    std::string source;
    std::string name;
    for(const auto& attribute: tileset.attribute)
    {
        if(attribute.first == "firstgid") firstGid = attribute.second;
        if(attribute.first == "source") source = attribute.second;
        if(attribute.first == "name") name = attribute.second;
    }

    std::cout << "  tileset[" << index << "] firstgid=" << firstGid;
    if(!name.empty()) std::cout << " name=" << name;
    if(!source.empty()) std::cout << " source=" << source;
    std::cout << " tiles=" << tileset.tiles.size();

    std::string image;
    for(const auto& attribute: tileset.image.attribute)
    {
        if(attribute.first == "source") image = attribute.second;
    }
    if(!image.empty()) std::cout << " image=" << image;
    std::cout << '\n';
}

void printLayer(const Layer& layer, std::size_t index)
{
    std::string name;
    std::string width;
    std::string height;
    for(const auto& attribute: layer.attribute)
    {
        if(attribute.first == "name") name = attribute.second;
        if(attribute.first == "width") width = attribute.second;
        if(attribute.first == "height") height = attribute.second;
    }

    const Data& data = layer.encodingData;
    std::cout << "  layer[" << index << "] name=" << name
              << " width=" << width << " height=" << height
              << " encoding=" << encodingName(data.encoding)
              << " compression=" << compressionName(data.compression)
              << " tiles=" << data.tiles.size() << '\n';

    std::cout << "    gids=";
    for(std::size_t i = 0; i < data.tiles.size(); ++i)
    {
        if(i) std::cout << ' ';
        std::cout << data.tiles[i].gid();
    }
    std::cout << '\n';

    printProperties(layer.properties, "    property ");
}

} // namespace

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "Usage: tilemapper_demo <map.tmx> [tileset.tsx]\n";
        return 1;
    }

    const std::string tmxPath = argv[1];
    const std::string tsxPath = (argc > 2) ? std::string(argv[2]) : std::string();

    TMXParser parser;
    if(!parser.load(tmxPath, tsxPath))
    {
        std::cerr << "Error: " << parser.errorMsg() << '\n';
        return 1;
    }
    if(parser.hasError())
    {
        std::cerr << "Warning: " << parser.errorMsg() << '\n';
    }

    const Map map = parser.getMap();

    std::cout << "map " << tmxPath << '\n';
    printAttributes(map.attribute, "  ");
    printProperties(map.properties, "  property ");

    std::cout << "  tilesets=" << map.tilesets.size() << '\n';
    for(std::size_t i = 0; i < map.tilesets.size(); ++i)
    {
        printTileset(map.tilesets[i], i);
    }

    std::size_t layerIndex = 0;
    for(const auto& layer: map.layers)
    {
        printLayer(layer, layerIndex++);
    }

    for(const auto& group: map.groups)
    {
        std::string name;
        for(const auto& attribute: group.attribute)
        {
            if(attribute.first == "name") name = attribute.second;
        }
        std::cout << "  group name=" << name << " layers=" << group.layers.size() << '\n';
        for(const auto& layer: group.layers)
        {
            printLayer(layer, layerIndex++);
        }
    }

    std::cout << "  objectgroups=" << map.objectgroups.size()
              << " imagelayers=" << map.imagelayers.size() << '\n';

    return 0;
}
