# Helmi rendering engine

## Goal
Goal is to developed realtime and physically based rendering algorithms and also tools for building and debugging these algorithms. Uses C++ and Opengl.
File explorer is windows specific
## structure
1. project consists of two visual studio projects Helmi and helmirt
2. Helmi contains realtime graphics with opengl and a gui for controls
3. helmirt contains ray tracing algorithms for physically based rendering
## features implemented
1. Blinn-Phong shading with normal, diffuse, specular and emission maps
2. PCSS soft shadows for directional and spotlights
3. UI (imgui) with docking enabled 
4. Windows file dialog for opening new models
5. HDR rendering and bloom
6. Headlight ray tracing with normal maps and diffuse maps
7. BVH building with Surface area heuristic and spatial median
8. Simple whitted ray tracing with an area light
9. Simple Scene editor
10. PBR shading
#### TO DO:
1. path tracer for diffuse BRDF
2. skeletal animation
3. scene editor camera
4. scene serialization
5. real-time pbr shading
6. Icons for light positions
## images:
![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/chesterfield.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/conference.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/shadowstest.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/multimodel.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/pbr.png)

## Bugs:

1. bugs related to Imgui Controls
2. Spotlight shadows do not work if position and direction is controlled by InputFloat3 instead of slider float (why??)

