#include "TMXParser.h"

void TMXParser::outputWangset(const std::vector<WangSet>& wset)
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

void TMXParser::outputAnimation(const Animation& anim)
{
    for(const auto& i: anim.frames)
    {
        for(const auto& j: i.attribute)
        {
            std::cout << j.first << " = " << j.second << " ";
        }
    }
}

void TMXParser::outputProperty(const std::vector<Property>& prop)
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

void TMXParser::outputAttribute(const Attribute& attrib)
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
