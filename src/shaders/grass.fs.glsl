#version 400 core

in ATTRIBS_GS_OUT
{
    float v; // paramètre de dégradé (0 = base, 1 = pointe)
} attribsIn;

out vec4 fragColor;

void main()
{
    // E12: dégradé du vert foncé (base) au vert clair (pointe)
    const vec3 darkGreen  = vec3(0.1, 0.3, 0.05);
    const vec3 lightGreen = vec3(0.3, 0.8, 0.1);
    fragColor = vec4(mix(darkGreen, lightGreen, attribsIn.v), 1.0);
}
