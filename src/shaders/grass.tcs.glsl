#version 400 core

// TODO: À remplir
//layout() out;

uniform mat4 modelView;

void main()
{
    // TODO: Définir le niveau de tessellation une fois par patch.
    
    vec4 v0 = gl_in[0].gl_Position; // (1, 0, 0)
    vec4 v1 = gl_in[1].gl_Position; // (0, 1, 0)
    vec4 v2 = gl_in[2].gl_Position; // (0, 0, 1)
        
    const float MIN_TESS = 2;
    const float MAX_TESS = 32;

    const float MIN_DIST = 10.0f;
    const float MAX_DIST = 40.0f;

    // Utiliser la distance du point milieu de l'arête traité par rapport à la caméra
    // comme facteur pour déterminer le niveau de tessellation.
    
    // Une fonction par palié donnera un meilleur résultat pour déterminer le niveau de tessellation
    // selon la distance.
        
    // La tessellation de l'interieur est le maximum des tessellations extérieurs
}
