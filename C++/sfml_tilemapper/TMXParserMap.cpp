#include "TMXParser.h"

bool TMXParser::checkNode(const pugi::xml_node& node, const std::string& name)
{
    return (node.child(name.c_str())) ? true : false;
}

Attribute TMXParser::parseAttributes(const pugi::xml_node& node)
{
    Attribute rval;
    for(const auto& attrib: node.attributes())
    {
        auto pair = std::make_pair(attrib.name(), attrib.value());
        rval.insert(pair);
    }
    return rval;
}

/**
 * Adds one Property per <property> child of node.
 * @param out vector the properties are appended to.
 * @param node node holding the <property> children (a <properties> element).
 */

void TMXParser::appendProperties(std::vector<Property>& out, const pugi::xml_node& node)
{
    for(const auto& property: node.children())
    {
        if(std::string(property.name()) != "property")
        {
            continue;
        }
        Property prop;
        prop.attribute = parseAttributes(property);
        prop.pc_data = property.text().as_string();
        out.push_back(prop);
    }
}

ImageLayer TMXParser::parseImageLayer(const pugi::xml_node& node)
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
            appendProperties(il.properties, child);
        }
    }
    return il;
}

Layer TMXParser::parseLayer(const pugi::xml_node& node)
{
    Layer lyr;
    lyr.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            appendProperties(lyr.properties, child);
        }
        if(name == "data")
        {
            Data data;

            for(const auto& attrib: child.attributes())
            {
                const std::string aname = attrib.name();
                const std::string value = attrib.value();
                if(aname == "encoding")
                {
                    if(value == "base64")
                    {
                        data.encoding = Data::Encoding::BASE64;
                    }
                    else if(value == "csv")
                    {
                        data.encoding = Data::Encoding::CSV;
                    }
                    else
                    {
                        data.encoding = Data::Encoding::XML;
                    }
                }
                else if(aname == "compression")
                {
                    if(value == "zlib")
                    {
                        data.compression = Data::Compression::zlib;
                    }
                    else if(value == "gzip")
                    {
                        data.compression = Data::Compression::gzip;
                    }
                    else if(value == "zstd")
                    {
                        data.compression = Data::Compression::zstd;
                    }
                    else
                    {
                        data.compression = Data::Compression::None;
                    }
                }
            }

            if(child.text().empty())
            {
                // Uncompressed XML layer: one <tile> element per cell.
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

                std::size_t expectedSize = 0;
                if(node.attribute("width") && node.attribute("height"))
                {
                    expectedSize = node.attribute("width").as_uint() * node.attribute("height").as_uint();
                }
                data.tiles = decryptLayerTile(data, expectedSize);
            }

            lyr.encodingData = data;
        }
    }
    return lyr;
}

ObjectGroup TMXParser::parseObjectGroup(const pugi::xml_node& node)
{
    ObjectGroup rval;
    rval.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            appendProperties(rval.properties, child);
        }
        if(name == "object")
        {
            Object obj;
            obj.attribute = parseAttributes(child);
            for(const auto& cobj: child.children())
            {
                if(std::string(cobj.name()) == "properties")
                {
                    appendProperties(obj.properties, cobj);
                }
            }
            rval.objects.push_back(obj);
        }
    }
    return rval;
}

Group TMXParser::parseGroup(const pugi::xml_node& node)
{
    Group grp;
    grp.attribute = parseAttributes(node);
    for(const auto& child: node.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            appendProperties(grp.properties, child);
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

std::string TMXParser::resolveFilePath(std::string path, const std::string& workingDir)
{
    static const std::string match("../");
    std::size_t result = path.find(match);
    std::size_t count = 0;
    while(result != std::string::npos)
    {
        count++;
        path = path.substr(result + match.size());
        result = path.find(match);
    }

    if(workingDir.empty()) return path;

    std::string outPath = workingDir;
    for(auto i = 0u; i < count; ++i)
    {
        result = outPath.find_last_of('/');
        if(result != std::string::npos)
        {
            outPath = outPath.substr(0, result);
        }
    }
    return std::move(outPath += '/' + path);
}

bool TMXParser::load(const std::string& tmxPath, const std::string& tsxPath)
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

    for(const auto& attr: root.attributes())
    {
        auto pair = std::make_pair(attr.name(), attr.value());
        _map.attribute.insert(pair);
    }

    for(const auto& child: root.children())
    {
        std::string name = child.name();
        if(name == "properties")
        {
            appendProperties(_map.properties, child);
        }
        if(name == "group")
        {
            _map.groups.push_back(parseGroup(child));
        }
        if(name == "tileset")
        {
            if(tsxPath.empty())
            {
                _map.tilesets.push_back(parseTileset(child, tmxPath));
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
