#version 400 core

layout (vertices = 3) out;

uniform vec3 cameraPos; // position caméra en coordonnées monde

// Constantes LOD (E7)
const float MIN_TESS = 1.0;
const float MAX_TESS = 16.0;
const float MIN_DIST = 5.0;
const float MAX_DIST = 40.0;

// Niveau de tessellation selon la distance caméra ↔ milieu d'arête
float tessLevel(vec3 edgeMid)
{
    float dist = distance(edgeMid, cameraPos);
    float t = clamp((dist - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);
    return mix(MAX_TESS, MIN_TESS, t);
}

void main()
{
    // Passer les positions des sommets au TES sans modification
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // E8: Calcul des niveaux effectué une seule fois par patch (invocation 0 uniquement)
    if (gl_InvocationID == 0)
    {
        vec3 v0 = gl_in[0].gl_Position.xyz;
        vec3 v1 = gl_in[1].gl_Position.xyz;
        vec3 v2 = gl_in[2].gl_Position.xyz;

        // E6: Milieu de chaque arête → même valeur pour les deux patches qui partagent l'arête
        // gl_TessLevelOuter[i] correspond à l'arête opposée au sommet i
        float l0 = tessLevel((v1 + v2) * 0.5); // arête v1-v2, opposée à v0
        float l1 = tessLevel((v0 + v2) * 0.5); // arête v0-v2, opposée à v1
        float l2 = tessLevel((v0 + v1) * 0.5); // arête v0-v1, opposée à v2

        gl_TessLevelOuter[0] = l0;
        gl_TessLevelOuter[1] = l1;
        gl_TessLevelOuter[2] = l2;

        // E7: niveau intérieur = maximum des niveaux extérieurs
        gl_TessLevelInner[0] = max(max(l0, l1), l2);
    }

    // Synchroniser toutes les invocations du patch après l'écriture des niveaux (E8)
    barrier();
}
