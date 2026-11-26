#include "TMXParser.h"
#include "base64.hpp"

std::vector<unsigned char> TMXParser::base64_decode_bytes(const std::string& encoded_string)
{
    // The payload of a <data> element is indented by Tiled and may be wrapped
    // over several lines, so strip all whitespace before decoding.
    std::string token;
    token.reserve(encoded_string.size());
    for(const auto& c: encoded_string)
    {
        if(!std::isspace(static_cast<unsigned char>(c)))
        {
            token.push_back(c);
        }
    }

    std::string decoded = base64_decode(token);
    std::vector<unsigned char> bytes;
    bytes.reserve(decoded.size());
    for(const auto& c: decoded)
    {
        bytes.push_back(static_cast<unsigned char>(c));
    }
    return bytes;
}

std::vector<unsigned> TMXParser::decode_gids(const std::vector<unsigned char>& bytes, std::size_t expectedSize)
{
    std::size_t count = bytes.size() / 4u;
    if(expectedSize > 0 && count > expectedSize)
    {
        count = expectedSize;
    }

    std::vector<unsigned> gids;
    gids.reserve(count);
    for(std::size_t i = 0; i < count; ++i)
    {
        const std::size_t o = i * 4u;
        const unsigned gid = static_cast<unsigned>(bytes[o])
                           | (static_cast<unsigned>(bytes[o + 1]) << 8)
                           | (static_cast<unsigned>(bytes[o + 2]) << 16)
                           | (static_cast<unsigned>(bytes[o + 3]) << 24);
        gids.push_back(gid);
    }
    return gids;
}

std::vector<LayerTile> TMXParser::toLayerTiles(const std::vector<unsigned>& gids)
{
    std::vector<LayerTile> rval;
    rval.reserve(gids.size());
    for(const auto& gid: gids)
    {
        LayerTile tile;
        tile.attribute.insert(std::make_pair(std::string("gid"), std::to_string(gid)));
        rval.push_back(tile);
    }
    return rval;
}

bool TMXParser::inflate_stream(const unsigned char* source, std::size_t inSize, int windowBits,
                               std::vector<unsigned char>& dest, std::size_t expectedSize)
{
    if(!source || inSize == 0)
    {
        return false;
    }

    z_stream stream;
    std::memset(&stream, 0, sizeof(stream));
    if(inflateInit2(&stream, windowBits) != Z_OK)
    {
        return false;
    }

    // Start with the size the map says it needs, and grow while inflating.
    std::size_t capacity = expectedSize > 0 ? expectedSize : (inSize * 4u + 64u);
    std::vector<unsigned char> buffer(capacity);

    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(source));
    stream.avail_in = static_cast<uInt>(inSize);

    int status = Z_OK;
    while(true)
    {
        if(stream.total_out >= buffer.size())
        {
            buffer.resize(buffer.size() * 2u);
        }
        stream.next_out = buffer.data() + stream.total_out;
        stream.avail_out = static_cast<uInt>(buffer.size() - stream.total_out);

        status = inflate(&stream, Z_NO_FLUSH);
        if(status == Z_STREAM_END)
        {
            break;
        }
        if(status == Z_OK)
        {
            continue;
        }
        // Z_BUF_ERROR with no input left means the compressed data was truncated.
        if(status == Z_BUF_ERROR && stream.avail_in == 0)
        {
            break;
        }

        _errorMsg = std::string("Could not decompress layer data: ") + getErrorCodeMsg(status);
        inflateEnd(&stream);
        return false;
    }

    const std::size_t outSize = static_cast<std::size_t>(stream.total_out);
    inflateEnd(&stream);

    if(outSize == 0)
    {
        return false;
    }

    buffer.resize(outSize);
    dest = std::move(buffer);
    return true;
}

bool TMXParser::decompress_zstd(const unsigned char* source, std::size_t inSize,
                                std::vector<unsigned char>& dest, std::size_t expectedSize)
{
    if(!source || inSize == 0)
    {
        return false;
    }

    ZSTD_DStream* stream = ZSTD_createDStream();
    if(!stream)
    {
        _errorMsg = "Could not create zstd decompression stream.";
        return false;
    }
    if(ZSTD_isError(ZSTD_initDStream(stream)))
    {
        _errorMsg = "Could not initialise zstd decompression stream.";
        ZSTD_freeDStream(stream);
        return false;
    }

    ZSTD_inBuffer input{source, inSize, 0};
    std::size_t capacity = expectedSize > 0 ? expectedSize : (inSize * 4u + 64u);
    std::vector<unsigned char> buffer(capacity);
    std::size_t produced = 0;

    while(input.pos < input.size)
    {
        if(produced >= buffer.size())
        {
            buffer.resize(buffer.size() * 2u);
        }
        ZSTD_outBuffer output{buffer.data() + produced, buffer.size() - produced, 0};
        const std::size_t remaining = ZSTD_decompressStream(stream, &output, &input);
        if(ZSTD_isError(remaining))
        {
            _errorMsg = std::string("Could not decompress layer data: ") + ZSTD_getErrorName(remaining);
            ZSTD_freeDStream(stream);
            return false;
        }
        if(output.pos == 0 && input.pos == input.size)
        {
            // Nothing more to read out of this frame.
            break;
        }
        produced += output.pos;
    }

    ZSTD_freeDStream(stream);

    if(produced == 0)
    {
        _errorMsg = "Could not decompress layer data: empty zstd frame.";
        return false;
    }

    buffer.resize(produced);
    dest = std::move(buffer);
    return true;
}

std::vector<unsigned> TMXParser::parse_csv_tiles(const std::string& text)
{
    // Tiled writes CSV layers as plain decimal gids separated by commas,
    // usually one row per line, with an optional trailing comma.
    std::vector<unsigned> gids;
    const char* p = text.c_str();
    const char* end = p + text.size();

    while(p < end)
    {
        while(p < end && (std::isspace(static_cast<unsigned char>(*p)) || *p == ','))
        {
            ++p;
        }
        if(p >= end)
        {
            break;
        }

        char* next = nullptr;
        const unsigned long value = std::strtoul(p, &next, 10);
        if(next == p)
        {
            // Not a number: skip the offending character instead of looping.
            ++p;
            continue;
        }
        gids.push_back(static_cast<unsigned>(value));
        p = next;
    }

    return gids;
}

std::vector<LayerTile> TMXParser::decryptLayerTile(const Data& data, std::size_t expectedSize)
{
    if(expectedSize == 0 || data.encrypted.empty())
    {
        return std::vector<LayerTile>();
    }

    // CSV layers are never compressed and are not base64 encoded.
    if(data.encoding == Data::Encoding::CSV)
    {
        std::vector<unsigned> gids = parse_csv_tiles(data.encrypted);
        if(gids.size() != expectedSize)
        {
            _error = true;
            _errorMsg = "CSV layer has " + std::to_string(gids.size()) + " tiles, expected "
                      + std::to_string(expectedSize) + ".";
        }
        return toLayerTiles(gids);
    }

    const std::vector<unsigned char> bytes = base64_decode_bytes(data.encrypted);
    if(bytes.empty())
    {
        _error = true;
        _errorMsg = "Layer data could not be base64 decoded.";
        return std::vector<LayerTile>();
    }

    const std::size_t expectedBytes = expectedSize * sizeof(std::uint32_t);
    std::vector<unsigned char> decoded;

    switch(data.compression)
    {
    case Data::Compression::None:
        decoded = bytes;
        break;
    case Data::Compression::zlib:
        if(!inflate_stream(bytes.data(), bytes.size(), MAX_WBITS, decoded, expectedBytes))
        {
            _error = true;
            if(_errorMsg.empty()) _errorMsg = "zlib layer decompression failed.";
            return std::vector<LayerTile>();
        }
        break;
    case Data::Compression::gzip:
        if(!inflate_stream(bytes.data(), bytes.size(), MAX_WBITS + 16, decoded, expectedBytes))
        {
            _error = true;
            if(_errorMsg.empty()) _errorMsg = "gzip layer decompression failed.";
            return std::vector<LayerTile>();
        }
        break;
    case Data::Compression::zstd:
        if(!decompress_zstd(bytes.data(), bytes.size(), decoded, expectedBytes))
        {
            _error = true;
            if(_errorMsg.empty()) _errorMsg = "zstd layer decompression failed.";
            return std::vector<LayerTile>();
        }
        break;
    }

    std::vector<unsigned> gids = decode_gids(decoded, expectedSize);
    if(gids.size() != expectedSize)
    {
        _error = true;
        _errorMsg = "Decoded layer has " + std::to_string(gids.size()) + " tiles, expected "
                  + std::to_string(expectedSize) + ".";
    }
    return toLayerTiles(gids);
}
