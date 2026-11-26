This is a library that is a current work in process. The purpose of sfml_tilemapper is to parse TMX and TSX files to be able to correctly provide information to be
able to draw TMX files as intended. The library is completely dependent depending on the current version of Tiled being used.

https://www.mapeditor.org/ - Tiled

Supported now: every layer encoding Tiled writes. XML, CSV, and base64 with no compression, zlib, gzip or zstd compression. Each layer ends up in
Layer::encodingData.tiles as one LayerTile per cell, in reading order (left to right, top to bottom), so the position of a tile in the vector is its position in the
map. Use LayerTile::gid() to read a cell's gid; mask it with & 0x1FFFFFFFu if you want to ignore the flip/rotation flags Tiled stores in the top four bits.

If a layer cannot be decoded, TMXParser::hasError() is set and errorMsg() explains why, instead of the layer silently coming back empty.

The following libraries are utilized in this library:

https://pugixml.org/ - PugiXML Library

https://zlib.net/ - ZLib Library

https://github.com/facebook/zstd - ZStd Library

(If the base64.h file does not state information on it's original creator, bare with me.
IIRC, I was troubleshooting and for some reason I removed the header and planned to
add it back before submitting it into the wild. This will be fixed ASAP as I feel ALL authors deserve credit entirely for all their creations! Another words, I don't condone
in theft of code. I will give credit where credit is due.)

Even though this is labeled to be used with SFML, there are no SFML libraries utilized. It is just the main library that I Was using when I began thinking about the creation
of this library. Regardless, here is the link to SFML:

https://www.sfml-dev.org/ - SFML Library

This library is also inspired by another library provided in the link below.
If you look through the comments with in the code, there is even code pulled from the library and
respectfully credited inside the comments. IIRC, I know for sure the bit shifting used to see if the tile is rotated is one of the things borrowed. This was due to bit shifting
still being a new concept to me at the time.
Even if I redesign my own way to do so, I will always credit this library for teaching me the concept of detecting if tiles have
been rotated or not:

https://github.com/fallahn/tmxlite

At the current time of creating the README, the version of Tiled that the library is being created for is 1.7.1.
I can not garantee if this library will correctly parse TMX
files from newer versions. Please keep this in mind.

This file will be edited as progress moves forward in developing this library. I will keep the parts that are currently in progress at the bottom of this README.

Building:

You need pugixml, zlib and zstd, then:

    cmake -S . -B build
    cmake --build build

    ./build/tilemapper_demo path/to/map.tmx

The demo prints what was parsed for a map, including each layer's gids in order. If your pugixml/zlib/zstd are somewhere CMake does not look by default, pass
-DCMAKE_PREFIX_PATH=/path/to/them (or -DPUGIXML_INCLUDE_DIR=... -DPUGIXML_LIBRARY=... and the same for ZLIB and ZSTD).

TMXParser.h holds the structures and the class declaration; the parsing itself lives in TMXParserMap.cpp, TMXParserData.cpp, TMXParserTileset.cpp, TMXParserTSX.cpp and TMXParserDebug.cpp. Compile those alongside your own sources, or use this CMakeLists.

In Progress:

Redesign library interface for better simplicity and reduce redundancy.
