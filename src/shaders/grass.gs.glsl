#version 330 core

// TODO: À remplir
// layout() in;
// layout() out;

in ATTRIBS_TES_OUT
{
    // TODO: Ajouter les attributs si nécessaire
} attribsIn[];

out ATTRIBS_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
} attribsOut;


uniform mat4 mvp;

// Fonction pseudo aléatoire, utiliser le paramètre co pour avoir une valeur différente en sortie
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // TODO: Générer un brin d'herbe à partir du point en entrée
    
    // En premier lieu, on pourra générer des triangles avec le point comme étant le centre
    // de la base du triangle.
    // Par la suite, on ajoutera un facteur aléatoire de rotation en y [0, 2pi], puis une
    // rotation en x [0, 0.1pi] pour "plier" les brins. Utiliser le facteur aléatoire.
    
    // On pourra aussi faire varié la taille avec la taille de base et la variation [0, 0.04].
    const float baseWidth = 0.05;
    const float varWidth = 0.04;
    
    // Idem avec la hauteur.
    const float baseHeight = 0.4;
    const float varHeight = 0.4;
    
    // Pour finir, émission des différents points. N'oublier pas d'écrire chaque attribut de sortie.
}
