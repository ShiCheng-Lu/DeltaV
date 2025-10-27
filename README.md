

Maths and Physics:

https://orbital-mechanics.space/intro.html

https://en.wikipedia.org/wiki/Vis-viva_equation

https://spencermortensen.com/articles/bezier-circle/



Craft structure:

Craft (ModularVehiclePawn)
 |- ClusterUnionComponent
 |   |- part_0 (root)
 |       |- part_1
 |       |- part_2
 |           |- part_3
 |- VehicleSimBaseComponent
 |   ... simulation components
 |
 |- part_def_0 (root)
 |   |- part_def_1
 |   |- part_def_2
 |       |- part_def_3

Craft Json:
{
  "name": <craft name>,
  "location": [ 0, 0, 0 ],
  "rotation": [ 0, 0, 0, 1 ],
  "structure": {
    "part_0": {
      "part_1": {
      },
      "part_2": {
        "part_3": {
        }
      }
    }
  },
  "parts": {
    "part_0": {
      "type": "command_module ?",
      "location": [ 0, 0, 0 ],
      "rotation": [ 0, 0, 0, 1 ],
      "scale": [ 1, 1, 1 ],
      "attached_at": 1,
      "attached_with": 1
    },
    "part_1": {
      "type": "parachute ?",
      "location": [ 0, -150, -50 ],
      "rotation": [ 0, 0, 0, 1 ],
      "scale": [ 1, 1, 1 ],
      "attached_at": 1
    },
    "part_2": {
      "type": "fuel",
      "location": [ 0, 150, -50 ],
      "rotation": [ 0, 0, 0, 1 ],
      "scale": [ 1, 1, 1 ],
      "attached_at": 1
    },
    "part_3": {
      "type": "engine",
      "location": [ 0, 0, -50 ],
      "rotaiton": [ 0, 0, 0, 1 ],
      "scale": [ 1, 1, 1 ],
      "attached_at": 1
    }
  },
  "stages": [
    {
      "attachments": [],
      "fuel_tanks": [],
      "engines": []
    }
  ]
}

Creating a geoemtry collection: 
uncheck "Generate Root Proxies", which disables the collection to use a invisible root as a wrapper, so collisions can be visualized
With root proxies thing should work fine, but no collision visualization since the root itself doesn't have any collision

if encountering gravity issues: check gravity in "Level", "World"


@self use custom mesh component for generate once and use (for wings) at sim time. dynamic mesh are for meshes that can be constantly changing, so custom mesh is more performant
