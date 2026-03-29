#version 430 core

// TODO: À remplir
// layout() in;

struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;
    vec4 color;
    vec2 size;
    float timeToLive;
    float maxTimeToLive;
};

layout(std140, binding = 0) readonly restrict buffer ParticlesInputBlock
{
    Particle particles[];
} dataIn;

layout(std140, binding = 1) writeonly restrict buffer ParticlesOutputBlock
{
    Particle particles[];
} dataOut;

uniform float time;
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform vec3 emitterDirection;

// Random [0, 1]
float rand01()
{
    return fract(sin(dot(vec2(time*100, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // TODO: Mise à jour des particules. À partir de l'entrée, metter à jour la particule.
    //
    //       Si le temps de vie est à 0 ou en dessous, on procède à l'initialisation:
    //           - La particule commence à la position de l'émetteur.
    //           - Son orientation est aléatoire entre [0, 2pi].
    //           - La particule va dans la direction à une vitesse de 0.3 unité/s.
    //           - La particule monte naturellement vers le haut à une vitesse de 0.2 unité/s.
    //           - La couleur initiale est grise (0.5).
    //           - La taille initiale est de 0.2 unité pour chaque segment.
    //           - Le temps de vie de la particule est entre [1.5, 2.0].
    //
    //      Pour la mise à jour:
    //          - Réduiser le temps de vie.
    //          - Mise à jour de la position par la méthode d'Euler.
    //          - L'orientation a une vitesse angulaire constante de 0.5.
    //          - La couleur devient blanche de façon linéaire selon le temps de vie.
    //          - L'opacité est à 0.2 et a un effet de fade in/out de [0, 0.2] et [0.8, 1].
    //          - La taille augmente de façon linéaire jusqu'à 0.5 unité en fonction du temps de vie.
}

