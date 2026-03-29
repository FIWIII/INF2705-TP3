#version 400 core

layout (location = 0) in vec3 position;

void main()
{
    // Les positions du patch sont en coordonnées monde, passées directement au TCS
    gl_Position = vec4(position, 1.0);
}
