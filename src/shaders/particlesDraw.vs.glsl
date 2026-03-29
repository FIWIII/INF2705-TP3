#version 330 core

// DONE: Attributs des particules lus depuis le SSBO via le VAO (Partie 3)
// Les offsets correspondent au struct C++ Particle :
//   location 0 : position (vec3)     offset  0
//   location 1 : color    (vec4)     offset 32
//   location 2 : size.x   (float)    offset 48
//   location 3 : zOrientation(float) offset 12
layout(location = 0) in vec3  aPosition;
layout(location = 1) in vec4  aColor;
layout(location = 2) in float aSize;
layout(location = 3) in float aAngle;

out ATTRIB_VS_OUT
{
    vec4  color;
    float size;
    float angle;
} attribOut;

uniform mat4 modelView;

void main()
{
    // E24: position de la particule en espace vue (le GS crée le quad billboard)
    gl_Position = modelView * vec4(aPosition, 1.0);
    attribOut.color = aColor;
    attribOut.size  = aSize;
    attribOut.angle = aAngle;
}
