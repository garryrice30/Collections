#include "TMXParser.h"

Tileset TMXParser::parseTSXFile(const std::string& path)
{
    Tileset tiles;
    pugi::xml_document doc;
    _result = doc.load_file(path.c_str());
    if(!_result)
    {
        _error = true;
        _errorMsg = std::string("Could not load TSX file: ") + path + " (" + _result.description() + ")";
    }
    else
    {
        auto root = doc.child("tileset");
        if(!root)
        {
            _error = true;
            _errorMsg = "Node note found. Expected node: tileset.";
        }
        else
        {
            for(const auto& attrib: root.attributes())
            {
                std::string name = attrib.name();
                if(name != "version" && name != "tiledversion")
                {
                    auto pair = std::make_pair(attrib.name(), attrib.value());
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
                    appendProperties(img.properties, child);
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
                            appendProperties(t.properties, tile);
                        }
                    }
                    tiles.tiles.push_back(t);
                }
                if(name == "properties")
                {
                    appendProperties(tiles.properties, child);
                }
                if(name == "terraintypes")
                {
                    TerrainTypes ttypes;
                    for(const auto& tchild: child.children())
                    {
                        Terrain ter;
                        ter.attribute = parseAttributes(tchild);
                        for(const auto& terrain: tchild.children())
                        {
                            if(std::string(terrain.name()) == "properties")
                            {
                                appendProperties(ter.properties, terrain);
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
                                appendProperties(wset.properties, wschild);
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
