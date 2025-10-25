












Maths and Physics:

https://orbital-mechanics.space/intro.html

https://en.wikipedia.org/wiki/Vis-viva_equation

https://spencermortensen.com/articles/bezier-circle/



Craft structure:


Probably not up to date with new Modular Vehicle system
root
 |- part_mesh_0
 |   |- part_mesh_1
 |   |- part_mesh_2
 |       |- part_mesh_3
 |- part_def_0
 |   |- part_def_1
 |   |- part_def_2
 |       |- part_def_3

Creating a geoemtry collection: 
uncheck "Generate Root Proxies", which disables the collection to use a invisible root as a wrapper, so collisions can be visualized
With root proxies thing should work fine, but no collision visualization since the root itself doesn't have any collision

