#version 400 core

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in ATTRIBS_TES_OUT
{
    vec3 worldPos;
} attribsIn[];

out ATTRIBS_GS_OUT
{
    float v; // paramètre de dégradé : 0 = base, 1 = pointe (E12)
} attribsOut;

uniform mat4 projView;

// Dimensions de base des brins (E11)
const float BASE_WIDTH  = 0.05;
const float VAR_WIDTH   = 0.04;
const float BASE_HEIGHT = 0.4;
const float VAR_HEIGHT  = 0.4;

// Pseudo-aléatoire stable par position (résultat identique à chaque frame)
float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 base = attribsIn[0].worldPos;

    // Quatre valeurs aléatoires avec graines décalées pour éviter les corrélations (E11)
    float r1 = rand(base.xz);
    float r2 = rand(base.xz + vec2(1.7, 4.3));
    float r3 = rand(base.xz + vec2(3.1, 7.9));
    float r4 = rand(base.xz + vec2(5.5, 2.1));

    float width  = BASE_WIDTH  + r1 * VAR_WIDTH;
    float height = BASE_HEIGHT + r2 * VAR_HEIGHT;

    // E11: rotation aléatoire autour de Y [0, 2π] et inclinaison en X [0, 0.1π]
    float angleY = r3 * 6.28318; // 2π
    float angleX = r4 * 0.31416; // 0.1π

    float cosY = cos(angleY), sinY = sin(angleY);
    float cosX = cos(angleX), sinX = sin(angleX);

    // Vecteur latéral du brin (perpendiculaire à sa direction dans XZ)
    vec3 right = vec3(-sinY, 0.0, cosY) * width;

    // Vecteur de la tige (direction principale, inclinée de angleX vers la direction angleY)
    vec3 stem = vec3(cosY * sinX, cosX, sinY * sinX) * height;

    // E10: émettre les trois sommets du brin comme triangle strip
    // E12: dégradé via v (0 = base sombre, 1 = pointe claire)
    attribsOut.v = 0.0;
    gl_Position = projView * vec4(base - right, 1.0);
    EmitVertex();

    attribsOut.v = 0.0;
    gl_Position = projView * vec4(base + right, 1.0);
    EmitVertex();

    attribsOut.v = 1.0;
    gl_Position = projView * vec4(base + stem, 1.0);
    EmitVertex();

    EndPrimitive();
}
