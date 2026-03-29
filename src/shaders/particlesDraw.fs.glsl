#version 330 core

in ATTRIB_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
} attribIn;

out vec4 FragColor;

uniform sampler2D textureSampler;

void main()
{
    // TODO: Colorier les particules.
    //       La couleur finale est la couleur de la texture teinté
    //       par la couleur de la particule.
    //       Pour éviter de traiter des fragments invisibles, on discard
    //       les fragments dont le texel possède moins de 0.02 d'opacité.
}
