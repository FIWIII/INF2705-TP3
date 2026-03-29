#version 330 core

// TODO: À remplir
// layout() in;
// layout() out;


in ATTRIB_VS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
} attribIn[];

out ATTRIB_GS_OUT
{
    // TODO: Ajouter les attributs si nécessaire
} attribOut;

uniform mat4 projection;


void main()
{

    // TODO: Convertiser les points en entrée en carrées.
    //       Un côté est de la taille particle::size.
    //       Les carrées sont toujours orientés vers la caméra et tourne
    //       sur eux même sur l'axe des z.
    //       Les coordonnées de texture peuvent être généré à partir de la position du point.
    //
    //       Il est important d'écrire chaque attribut de sortie du shader de géometrie,
    //       autrement c'est un comportement non défini.
}
