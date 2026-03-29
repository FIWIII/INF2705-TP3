#version 330 core

// DONE: Génération de quads billboard orientés vers la caméra (Partie 3)
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    vec4  color;
    float size;
    float angle;
} attribIn[];

out ATTRIB_GS_OUT
{
    vec4 color;
    vec2 texCoord;
} attribOut;

uniform mat4 projection;

void main()
{
    vec4  center = gl_in[0].gl_Position; // position en espace vue
    float hs     = attribIn[0].size * 0.5;
    float a      = attribIn[0].angle;
    float ca     = cos(a);
    float sa     = sin(a);

    // E25/E26: quatre coins du quad, tournés de `angle` autour de l'axe Z de la vue
    // Ordre triangle_strip : BL, BR, TL, TR (deux triangles)
    vec2 offsets[4] = vec2[4](
        vec2(-hs, -hs),
        vec2( hs, -hs),
        vec2(-hs,  hs),
        vec2( hs,  hs)
    );
    vec2 uvs[4] = vec2[4](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    for (int i = 0; i < 4; i++)
    {
        float dx = offsets[i].x;
        float dy = offsets[i].y;
        // Rotation 2D en espace vue (plan XY de la caméra)
        vec2 rotated = vec2(ca * dx - sa * dy,
                            sa * dx + ca * dy);
        gl_Position      = projection * (center + vec4(rotated, 0.0, 0.0));
        attribOut.color    = attribIn[0].color;
        attribOut.texCoord = uvs[i];
        EmitVertex();
    }
    EndPrimitive();
}
