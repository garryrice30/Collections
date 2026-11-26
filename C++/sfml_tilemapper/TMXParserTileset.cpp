#include "TMXParser.h"

Tileset TMXParser::parseTSXFile(const std::string& path, const std::string& tmxBasePath)
{
    std::size_t found = tmxBasePath.find_last_of("/\\");
    if(found == std::string::npos)
    {
        _error = true;
        _errorMsg = "Could not base filepath for TSX file.";
    }
    std::string buffer = tmxBasePath.substr(0, found + 1);
    buffer += path;
    return parseTSXFile(buffer);
}

Tileset TMXParser::parseTileset(const pugi::xml_node& node, const std::string& tmxPath)
{
    Tileset rval;
    rval.attribute = parseAttributes(node);
    for(const auto& attrib: rval.attribute)
    {
        if(attrib.first == "source")
        {
            // The tileset lives in its own TSX file, but the attributes of the
            // <tileset> reference (firstgid, source, ...) still have to be kept
            // or a gid cannot be mapped back to this tileset.
            const Attribute reference = rval.attribute;
            rval = parseTSXFile(attrib.second, tmxPath);
            for(const auto& ref: reference)
            {
                rval.attribute.insert(ref);
            }
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
                appendProperties(rval.properties, child);
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
                        if(std::string(tprop.name()) == "properties")
                        {
                            appendProperties(ter.properties, tprop);
                        }
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
                        appendProperties(t.properties, tchild);
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
                            appendProperties(wset.properties, wschild);
                        }
                    }
                    rval.wangsets.push_back(wset);
                }
            }
        }
    }
    return rval;
}
