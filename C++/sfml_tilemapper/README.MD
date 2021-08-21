This is a library that is a current work in process. The purpose of sfml_tilemapper is to parse TMX and TSX files to be able to correctly provide information to be
able to draw TMX files as intended. The library is completely dependent depending on the current version of Tiled being used.

https://www.mapeditor.org/ - Tiled

Some of the intended features of this library includes the ability to be able to correctly deduce which ever encryption is used when TMX files is wrote and still be
to accurately provided needed information on the fly. It is likely not all TMX encryption features are not implimented yet. The ability to read tile ids using CSV is
not provided yet due it being the simplest process. I tend to get the harder hurdles out of the way first before tackling simpler hurdles. One such hurdle that is
difficult is the use of ZSTD encryption. I will not impliment CSV until I feel confident that ZSTD is implimented correctly (if someone chooses to provide the code
needed to accurately parse CSV files, I'm good with that. I just already know the largest portion of my focus will have to be utilized towards ZSTD.).

I also realize that the base of the library could be designed MUCH better. I do plan to redo the design of the main class for a cleaner interface (largely to reduce redundancy
and for simplicity of use as wel).

The following libraries are utilized in this library:

https://pugixml.org/ - PugiXML Library
https://zlib.net/ - ZLib Library
https://github.com/facebook/zstd - ZStd Library

(If the base64.h file does not state information on it's original creator, bare with me. IIRC, I was troubleshooting and for some reason I removed the header and planned to
add it back before submitting it into the wild. This will be fixed ASAP as I feel ALL authors deserve credit entirely for all their creations! Another words, I don't condone
in theft of code. I will give credit where credit is due.)

Even though this is labeled to be used with SFML, there are no SFML libraries utilized. It is just the main library that I Was using when I began thinking about the creation
of this library. Regardless, here is the link to SFML:

https://www.sfml-dev.org/ - SFML Library

This library is also inspired by another library provided in the link below. If you look through the comments with in the code, there is even code pulled from the library and
respectfully credited inside the comments. IIRC, I know for sure the bit shifting used to see if the tile is rotated is one of the things borrowed. This was due to bit shifting
still being a new concept to me at the time. Even if I redesign my own way to do so, I will always credit this library for teaching me the concept of detecting if tiles have 
been rotated or not:

https://github.com/fallahn/tmxlite

At the current time of creating the README, the version of Tiled that the library is being created for is 1.7.1. I can not garantee if this library will correctly parse TMX
files from newer versions. Please keep this in mind.

This file will be edited as progress moves forward in developing this library. I will keep the parts that are currently in progress at the bottom of this README.

In Progress:

Correctly parsing tile ids encrypted using ZSTD.

Correctly parsing tile ids encrypted using CSV.

Redesign library interface for better simplicity and reduce redundancy.
