#include "shader_program.hpp"

#include <glm/glm.hpp>

class TransformShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorModULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};



class EdgeEffect : public ShaderProgram
{
public:
    GLuint mvpULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class Sky : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint textureSamplerULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


// Partie 2 : pipeline de tessellation pour l'effet de gazon procédural
class GrassShader : public ShaderProgram
{
public:
    GLuint cameraPosULoc;
    GLuint projViewULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


// Partie 3 : shader de dessin des particules (VS→GS→FS)
class ParticleDrawShader : public ShaderProgram
{
public:
    GLuint modelViewULoc;
    GLuint projectionULoc;
    GLuint smokeTextureULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


// Partie 3 : compute shader de mise à jour des particules
class ParticleComputeShader : public ShaderProgram
{
public:
    GLuint timeULoc;
    GLuint deltaTimeULoc;
    GLuint emitterPositionULoc;
    GLuint emitterDirectionULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


// Partie 1 : shader non-éclairé pour la spline Bézier
class BezierShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint lineColorULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class CelShading : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint viewULoc;
    GLuint modelViewULoc;
    GLuint normalULoc;
    
    GLuint nSpotLightsULoc;
    
    GLuint globalAmbientULoc;
    GLuint diffuseSamplerULoc;

public:
    void setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};

