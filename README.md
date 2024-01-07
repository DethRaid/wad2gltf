# wad2gltf

## A WAD to glTF 2.0 converter

This tool extracts a map from a WAD file and writes it to a glTF file. It created one glTF Node and Mesh for each sector, and creates up to three Primitives for each sidedef (upper, middle, and lower). It also extracts the textures from the WAD and converts the textures to PNG 

This tool exports THINGS, but it does not preserve their IDs nor does it place them at the correct height in the world

This tool does not export any of the original culling information, and it's not likely to. Modern computers are able to render an entire DOOM level with ease

I've tested this on the DOOM WAD included with the DOOM 3 BFG edition. I expect it to work for any DOOM or DOOM 2 WAD, so please report any bugs you find with those. However, this tool does not support DOOM 64, Hexen, Heretic, Strife, or other id Tech games. Their WAD formats are too different
