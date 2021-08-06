# Helmi rendering engine

## Goal
Goal is to developed realtime and physically based rendering algorithms and also tools for building and debugging these algorithms
## structure
1. project consists of two visual studio projects Helmi and helmirt
2. Helmi contains realtime graphics with opengl and a gui for controls
3. helmirt contains ray tracing algorithms for physically based rendering
## features implemented
1. blinnphong shading with normal, diffuse, specular and emission maps
2. pcss soft shadows for directional and spotlights
3. UI (imgui) with docking enabled 
4. windows file dialog for opening new models
5. hdr rendering and bloom
6. headlight ray tracing with normal maps and diffuse maps
7. BVH building with Surface area heuristic and spatial median
#### TO DO:
1. path tracer for diffuse BRDF
2. skeletal animation
3. scene editor with multiple models
4. real-time pbr shading
## images:
![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/chesterfield.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/conference.png)

![alt text](https://github.com/SampoImmonen/Helmi/blob/dockinggui/media/shadowstest.png)


