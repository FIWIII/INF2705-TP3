#version 400 core

// E9: point_mode → chaque vertex tessellé devient un point envoyé au geometry shader
layout (triangles, equal_spacing, ccw, point_mode) in;

out ATTRIBS_TES_OUT
{
    vec3 worldPos; // position interpolée en coordonnées monde
} attribsOut;

void main()
{
    // Interpolation barycentrique à partir des trois sommets du patch
    vec3 pos = gl_TessCoord.x * gl_in[0].gl_Position.xyz
             + gl_TessCoord.y * gl_in[1].gl_Position.xyz
             + gl_TessCoord.z * gl_in[2].gl_Position.xyz;

    attribsOut.worldPos = pos;

    // gl_Position requis par le pipeline; le GS génère les positions finales en espace clip
    gl_Position = vec4(pos, 1.0);
}
