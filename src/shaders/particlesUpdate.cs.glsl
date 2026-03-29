#version 430 core

// DONE: Compute shader de mise à jour des particules (Partie 3)
layout(local_size_x = 64) in;

// Doit correspondre exactement au struct C++ Particle (layout std140)
struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;       // vec3 padded à 16 octets dans std140 (w inutilisé)
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

// Constantes de simulation des particules (E18–E23)
const uint  MAX_PARTICLES    = 64u;
const float TWO_PI           = 6.28318530718; // 2π pour les angles aléatoires
const float EMITTER_SPEED    = 0.3;           // vitesse dans la direction d'émission (u/s)
const float UPWARD_SPEED     = 0.2;           // montée naturelle (u/s)
const float VELOCITY_SPREAD  = 0.1;           // dispersion latérale aléatoire (u/s)
const float MIN_LIFETIME     = 1.5;           // durée de vie minimale (s)
const float MAX_LIFETIME     = 2.0;           // durée de vie maximale (s)
const float ANGULAR_VELOCITY = 0.5;           // vitesse angulaire de rotation (rad/s, E20)
const float BASE_OPACITY     = 0.2;           // opacité de base (E22)
const float MIN_SIZE         = 0.2;           // taille initiale du brin de fumée (E23)
const float MAX_SIZE         = 0.5;           // taille maximale du brin de fumée (E23)

// Nombre pseudo-aléatoire dans [0, 1] basé sur le temps et l'index de l'invocation
float rand01()
{
    return fract(sin(dot(vec2(time*100, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

// Variante avec sel pour générer plusieurs valeurs aléatoires indépendantes
float rand01(float salt)
{
    return fract(sin(dot(vec2(time*100 + salt, gl_GlobalInvocationID.x), vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= MAX_PARTICLES)
        return;

    Particle p = dataIn.particles[idx];

    // E18: réinitialisation si la particule est morte (timeToLive <= 0)
    if (p.timeToLive <= 0.0)
    {
        // Position à l'émetteur avec léger décalage aléatoire (±0.05 unité)
        p.position = emitterPosition + vec3(rand01(1.0) * 0.1 - 0.05,
                                            rand01(2.0) * 0.1 - 0.05,
                                            rand01(3.0) * 0.1 - 0.05);
        // Orientation aléatoire dans [0, 2π]
        p.zOrientation = rand01(4.0) * TWO_PI;
        // Vitesse : direction d'émission + montée naturelle + dispersion aléatoire en XZ
        p.velocity = normalize(emitterDirection) * EMITTER_SPEED
                   + vec3(0.0, UPWARD_SPEED, 0.0)
                   + vec3(rand01(6.0) * VELOCITY_SPREAD - VELOCITY_SPREAD * 0.5,
                          0.0,
                          rand01(7.0) * VELOCITY_SPREAD - VELOCITY_SPREAD * 0.5);
        // Couleur initiale grise, transparente
        p.color = vec4(0.5, 0.5, 0.5, 0.0);
        // Taille initiale minimale
        p.size = vec2(MIN_SIZE, 0.0);
        // Temps de vie aléatoire dans [MIN_LIFETIME, MAX_LIFETIME]
        p.timeToLive    = MIN_LIFETIME + rand01(5.0) * (MAX_LIFETIME - MIN_LIFETIME);
        p.maxTimeToLive = p.timeToLive;
    }
    else
    {
        // Fraction de vie écoulée (0.0 = neuf, 1.0 = mort)
        float lifeFrac = 1.0 - p.timeToLive / p.maxTimeToLive;

        // E19: réduction du temps de vie et intégration d'Euler
        p.timeToLive -= deltaTime;
        p.position   += p.velocity * deltaTime;

        // E20: rotation constante à vitesse angulaire ANGULAR_VELOCITY (rad/s)
        p.zOrientation += ANGULAR_VELOCITY * deltaTime;

        // E21: couleur grise (0.5) → blanche (1.0) de façon linéaire
        p.color.rgb = mix(vec3(0.5), vec3(1.0), lifeFrac);

        // E22: opacité BASE_OPACITY avec fondu entrant [0, 0.2] et sortant [0.8, 1.0]
        p.color.a = BASE_OPACITY * smoothstep(0.0, 0.2, lifeFrac)
                                 * (1.0 - smoothstep(0.8, 1.0, lifeFrac));

        // E23: taille croît linéairement de MIN_SIZE à MAX_SIZE selon le temps de vie
        p.size.x = mix(MIN_SIZE, MAX_SIZE, lifeFrac);
    }

    dataOut.particles[idx] = p;
}
