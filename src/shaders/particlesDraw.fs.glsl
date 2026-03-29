#version 330 core

// DONE: Coloration des particules avec texture de fumée (Partie 3)
in ATTRIB_GS_OUT
{
    vec4 color;
    vec2 texCoord;
} attribIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, attribIn.texCoord);

    // E27: rejeter les fragments quasi-transparents
    if (texColor.a < 0.02)
        discard;

    // E28: couleur finale = texture teintée par la couleur de la particule
    FragColor = texColor * attribIn.color;
}
