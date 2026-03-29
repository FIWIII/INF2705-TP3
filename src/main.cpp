#include "car.hpp"
#include "happly.h"
#include "model.hpp"
#include "model_data.hpp"
#include "shader_storage_buffer.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <inf2705/OpenGLApplication.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// DONE: Ajout du struct Particle, des shaders
// ParticleDrawShader/ParticleComputeShader,
//       des SSBOs double-buffered, du VAO de particules et de la texture de
//       fumée (Partie 3)

// Partie 3
// Ne pas modifier
struct Particle {
  glm::vec3 position;
  GLfloat zOrientation;
  glm::vec4 velocity; // vec3, but padded
  glm::vec4 color;
  glm::vec2 size;
  GLfloat timeToLive;
  GLfloat maxTimeToLive;
};

struct Material {
  glm::vec4 emission;
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec3 specular;
  GLfloat shininess;
};

struct DirectionalLight {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 direction;
};

struct SpotLight {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;

  glm::vec4 position;
  glm::vec3 direction;
  GLfloat exponent;
  GLfloat openingAngle;

  GLfloat padding[3];
};

// Partie 1
Material bezierMat = {{1.0f, 1.0f, 1.0f, 0.0f},
                      {0.0f, 0.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, 0.0f},
                      1.0f};

struct BezierCurve {
  glm::vec3 p0;
  glm::vec3 c0;
  glm::vec3 c1;
  glm::vec3 p1;
};

BezierCurve curves[5] = {
    {glm::vec3(-28.7912, 1.4484, -1.7349), glm::vec3(-28.0654, 1.4484, 6.1932),
     glm::vec3(-10.3562, 8.8346, 6.5997), glm::vec3(-7.6701, 8.8346, 8.9952)},
    {glm::vec3(-7.6701, 8.8346, 8.9952), glm::vec3(-3.9578, 8.8346, 12.3057),
     glm::vec3(-2.5652, 2.4770, 13.6914), glm::vec3(2.5079, 1.4484, 11.6581)},
    {glm::vec3(2.5079, 1.4484, 11.6581), glm::vec3(7.5810, 0.4199, 9.6248),
     glm::vec3(16.9333, 3.3014, 5.7702), glm::vec3(28.4665, 6.6072, 3.9096)},
    {glm::vec3(28.4665, 6.6072, 3.9096), glm::vec3(39.9998, 9.9131, 2.0491),
     glm::vec3(30.8239, 5.7052, -15.2108), glm::vec3(21.3852, 5.7052, -9.0729)},
    {glm::vec3(21.3852, 5.7052, -9.0729), glm::vec3(11.9464, 5.7052, -2.9349),
     glm::vec3(-1.0452, 1.4484, -12.4989),
     glm::vec3(-12.2770, 1.4484, -13.2807)}};

// Partie 1 : évaluation d'un point sur une courbe de Bézier cubique
glm::vec3 evalBezier(const BezierCurve &c, float t) {
  float u = 1.0f - t;
  return u * u * u * c.p0 + 3.0f * u * u * t * c.c0 + 3.0f * u * t * t * c.c1 +
         t * t * t * c.p1;
}

// Matériels

Material defaultMat = {{0.0f, 0.0f, 0.0f, 0.0f},
                       {1.0f, 1.0f, 1.0f, 0.0f},
                       {1.0f, 1.0f, 1.0f, 0.0f},
                       {0.7f, 0.7f, 0.7f},
                       10.0f};

Material grassMat = {{0.0f, 0.0f, 0.0f, 0.0f},
                     {0.8f, 0.8f, 0.8f, 0.0f},
                     {1.0f, 1.0f, 1.0f, 0.0f},
                     {0.05f, 0.05f, 0.05f},
                     100.0f};

Material streetMat = {{0.0f, 0.0f, 0.0f, 0.0f},
                      {0.7f, 0.7f, 0.7f, 0.0f},
                      {1.0f, 1.0f, 1.0f, 0.0f},
                      {0.025f, 0.025f, 0.025f},
                      300.0f};

Material streetlightMat = {{0.0f, 0.0f, 0.0f, 0.0f},
                           {0.8f, 0.8f, 0.8f, 0.0f},
                           {1.0f, 1.0f, 1.0f, 0.0f},
                           {0.7f, 0.7f, 0.7f},
                           10.0f};

Material streetlightLightMat = {{0.8f, 0.7f, 0.5f, 0.0f},
                                {1.0f, 1.0f, 1.0f, 0.0f},
                                {1.0f, 1.0f, 1.0f, 0.0f},
                                {0.7f, 0.7f, 0.7f},
                                10.0f};

Material windowMat = {{0.0f, 0.0f, 0.0f, 0.0f},
                      {1.0f, 1.0f, 1.0f, 0.0f},
                      {1.0f, 1.0f, 1.0f, 0.0f},
                      {1.0f, 1.0f, 1.0f},
                      2.0f};

Material treeMat = {
    {0.0f, 0.0f, 0.0f, 0.0f}, // emission
    {1.0f, 1.0f, 1.0f, 0.0f}, // ambient
    {1.0f, 1.0f, 1.0f, 0.0f}, // diffuse
    {0.0f, 0.0f, 0.0f},       // specular
    1.0f                      // shininess
};

struct App : public OpenGLApplication {
  App()
      : isDay_(true), cameraPosition_(0.f, 0.f, 0.f),
        cameraOrientation_(0.f, 0.f), isMouseMotionEnabled_(false),
        isAutopilotEnabled_(true), trackDistance_(0.0f), currentScene_(0),
        totalTime(0.0), timerParticles_(0.0) // Initialiser à 0
        ,
        nParticles_(0) // Initialiser à 0
  {
    car_.position = glm::vec3(0.0f, 0.0f, 15.0f);
    car_.orientation.y = glm::radians(180.0f);
    car_.speed = 4.0f;
  }

  ~App() override {
    glDeleteBuffers(1, &vboSpline_);
    glDeleteVertexArrays(1, &vaoSpline_);
    glDeleteBuffers(1, &grassVBO_);
    glDeleteVertexArrays(1, &grassVAO_);
    glDeleteVertexArrays(1, &vaoParticles_); // Partie 3
  }

  void init() override {

    setKeybindMessage(
        "ESC : quitter l'application."
        "\n"
        "T : changer de scène."
        "\n"
        "W : déplacer la caméra vers l'avant."
        "\n"
        "S : déplacer la caméra vers l'arrière."
        "\n"
        "A : déplacer la caméra vers la gauche."
        "\n"
        "D : déplacer la caméra vers la droite."
        "\n"
        "Q : déplacer la caméra vers le bas."
        "\n"
        "E : déplacer la caméra vers le haut."
        "\n"
        "Flèches : tourner la caméra."
        "\n"
        "Souris : tourner la caméra"
        "\n"
        "Espace : activer/désactiver la souris."
        "\n"
        "PageUp/PageDown : augmenter/diminuer les divisions de la spline "
        "Bézier."
        "\n"
        "B : lancer l'animation de la caméra le long de la spline."
        "\n"
        "G : activer/désactiver le mode filaire (debug tessellation gazon)."
        "\n");

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    edgeEffectShader_.create();
    celShadingShader_.create();
    skyShader_.create();

    car_.edgeEffectShader = &edgeEffectShader_;
    car_.celShadingShader = &celShadingShader_;
    car_.material = &material_;
    car_.carTexture = &carTexture_;
    car_.carWindowTexture = &carWindowTexture_;
    car_.lightTexture = &streetlightLightTexture_;

    grassTexture_.load("../textures/grass.jpg");
    streetTexture_.load("../textures/street.jpg");
    streetcornerTexture_.load("../textures/streetcorner.jpg");
    carTexture_.load("../textures/car.png");
    carWindowTexture_.load("../textures/window.png");
    treeTexture_.load("../textures/pine.jpg");
    streetlightTexture_.load("../textures/streetlight.jpg");
    streetlightLightTexture_.load("../textures/light.png");

    const char *pathes[] = {
        "../textures/skybox/Daylight Box_Right.bmp",
        "../textures/skybox/Daylight Box_Left.bmp",
        "../textures/skybox/Daylight Box_Top.bmp",
        "../textures/skybox/Daylight Box_Bottom.bmp",
        "../textures/skybox/Daylight Box_Front.bmp",
        "../textures/skybox/Daylight Box_Back.bmp",
    };

    const char *nightPathes[] = {
        "../textures/skyboxNight/right.png",
        "../textures/skyboxNight/left.png",
        "../textures/skyboxNight/top.png",
        "../textures/skyboxNight/bottom.png",
        "../textures/skyboxNight/front.png",
        "../textures/skyboxNight/back.png",
    };

    skyboxTexture_.load(pathes);
    skyboxNightTexture_.load(nightPathes);
    loadModels();
    initStaticMatrices();

    grassTexture_.setWrap(GL_REPEAT);
    grassTexture_.setFiltering(GL_LINEAR);
    grassTexture_.enableMipmap();

    streetTexture_.setWrap(GL_REPEAT);
    streetTexture_.setFiltering(GL_LINEAR);
    streetTexture_.enableMipmap();

    streetcornerTexture_.setWrap(GL_CLAMP_TO_EDGE);
    streetcornerTexture_.setFiltering(GL_LINEAR);
    streetcornerTexture_.enableMipmap();

    carTexture_.setWrap(GL_CLAMP_TO_EDGE);
    carTexture_.setFiltering(GL_LINEAR);

    carWindowTexture_.setWrap(GL_CLAMP_TO_EDGE);
    carWindowTexture_.setFiltering(GL_NEAREST);

    treeTexture_.setWrap(GL_REPEAT);
    treeTexture_.setFiltering(GL_NEAREST);

    streetlightTexture_.setWrap(GL_REPEAT);
    streetlightTexture_.setFiltering(GL_LINEAR);

    streetlightLightTexture_.setWrap(GL_CLAMP_TO_EDGE);
    streetlightLightTexture_.setFiltering(GL_NEAREST);

    // Partie 3
    material_.allocate(&defaultMat, sizeof(Material));
    material_.setBindingIndex(0);

    lightsData_.dirLight = {{0.2f, 0.2f, 0.2f, 0.0f},
                            {1.0f, 1.0f, 1.0f, 0.0f},
                            {0.5f, 0.5f, 0.5f, 0.0f},
                            {0.5f, -1.0f, 0.5f, 0.0f}};

    for (unsigned int i = 0; i < N_STREETLIGHTS; i++) {
      lightsData_.spotLights[i].position =
          glm::vec4(streetlightLightPositions[i], 1.0f);
      lightsData_.spotLights[i].direction = glm::vec3(0, -1, 0);
      lightsData_.spotLights[i].exponent = 6.0f;
      lightsData_.spotLights[i].openingAngle = 60.f;
    }

    // Intialisation basique des spots de la voiture (phares avant)
    lightsData_.spotLights[N_STREETLIGHTS].position =
        glm::vec4(-1.6, 0.64, -0.45, 1.0f);
    lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);
    lightsData_.spotLights[N_STREETLIGHTS].exponent = 4.0f;
    lightsData_.spotLights[N_STREETLIGHTS].openingAngle = 30.f;

    lightsData_.spotLights[N_STREETLIGHTS + 1].position =
        glm::vec4(-1.6, 0.64, 0.45, 1.0f);
    lightsData_.spotLights[N_STREETLIGHTS + 1].direction =
        glm::vec3(-10, -1, 0);
    lightsData_.spotLights[N_STREETLIGHTS + 1].exponent = 4.0f;
    lightsData_.spotLights[N_STREETLIGHTS + 1].openingAngle = 30.f;

    // Feux arrière (freins)
    lightsData_.spotLights[N_STREETLIGHTS + 2].position =
        glm::vec4(1.6, 0.64, -0.45, 1.0f);
    lightsData_.spotLights[N_STREETLIGHTS + 2].direction = glm::vec3(10, -1, 0);
    lightsData_.spotLights[N_STREETLIGHTS + 2].exponent = 4.0f;
    lightsData_.spotLights[N_STREETLIGHTS + 2].openingAngle = 30.f;

    lightsData_.spotLights[N_STREETLIGHTS + 3].position =
        glm::vec4(1.6, 0.64, 0.45, 1.0f);
    lightsData_.spotLights[N_STREETLIGHTS + 3].direction = glm::vec3(10, -1, 0);
    lightsData_.spotLights[N_STREETLIGHTS + 3].exponent = 4.0f;
    lightsData_.spotLights[N_STREETLIGHTS + 3].openingAngle = 30.f;

    toggleStreetlight();
    updateCarLight();
    setLightingUniform();

    lights_.allocate(&lightsData_, sizeof(lightsData_));
    lights_.setBindingIndex(1);

    // DONE: Création de GrassShader pour le gazon procédural (Partie 2)
    grassShader_.create();

    // DONE: Initialisation du maillage de patches triangulaires du gazon
    // (Partie 2)
    initGrassMesh();

    // DONE: Création de BezierShader pour la courbe Bézier (Partie 1)
    bezierShader_.create();

    // DONE: Initialisation du VAO/VBO de la spline Bézier (Partie 1)
    // Le buffer est alloué une seule fois à la taille maximale (E1).
    glGenVertexArrays(1, &vaoSpline_);
    glBindVertexArray(vaoSpline_);

    glGenBuffers(1, &vboSpline_);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpline_);
    glBufferData(GL_ARRAY_BUFFER, MAX_BEZIER_POINTS * sizeof(glm::vec3),
                 nullptr, GL_DYNAMIC_DRAW);

    // Attribut 0 : position (3 flottants)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier
                                     // gl_PointSize dans les shaders

    // DONE: Allocation des SSBOs double-buffered pour les particules (Partie 3,
    // E13/E14)
    //       Le buffer d'entrée (0) est initialisé à 0 (timeToLive=0 → tous
    //       morts au départ). Le buffer de sortie (1) n'a pas besoin d'être
    //       initialisé. E14: GL_DYNAMIC_COPY — données modifiées fréquemment
    //       par des commandes OpenGL (compute shader = commande GL) et
    //       utilisées comme source de rendu vertex. Le suffixe COPY indique que
    //       l'écriture vient du GPU (pas du CPU), ce qui correspond exactement
    //       au schéma compute → draw de notre pipeline.
    {
      std::vector<uint8_t> zeroes(MAX_PARTICLES_ * sizeof(Particle), 0);
      particles_[0].allocate(zeroes.data(), MAX_PARTICLES_ * sizeof(Particle),
                             GL_DYNAMIC_COPY);
      particles_[1].allocate(nullptr, MAX_PARTICLES_ * sizeof(Particle),
                             GL_DYNAMIC_COPY);
    }

    // DONE: VAO pour le dessin des particules — les attributs lisent depuis le
    // SSBO (Partie 3, E16)
    //       Stride = sizeof(Particle) = 64 octets.
    //       Les pointeurs sont mis à jour chaque frame pour suivre le buffer
    //       ping-pong.
    glGenVertexArrays(1, &vaoParticles_);
    glBindVertexArray(vaoParticles_);
    particles_[0].bindAsArray(); // buffer initial (mis à jour chaque frame)
    // location 0 : position (vec3)       offset  0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)0);
    glEnableVertexAttribArray(0);
    // location 1 : color (vec4)          offset 32
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)32);
    glEnableVertexAttribArray(1);
    // location 2 : size.x (float)        offset 48
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)48);
    glEnableVertexAttribArray(2);
    // location 3 : zOrientation (float)  offset 12
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)12);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // DONE: Création des shaders de particules (Partie 3)
    particleDrawShader_.create();
    particleComputeShader_.create();

    // DONE: Chargement de la texture de fumée (Partie 3)
    smokeTexture_.load("../textures/smoke.png");
    smokeTexture_.setWrap(GL_CLAMP_TO_EDGE);
    smokeTexture_.setFiltering(GL_LINEAR);

    CHECK_GL_ERROR;
  }

  void drawFrame() override {
    // DONE: Recharge des shaders de particules ajoutée (Partie 3)

    CHECK_GL_ERROR;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ImGui::Begin("Scene Parameters");
    ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);

    if (ImGui::Button("Reload Shaders")) {
      CHECK_GL_ERROR;
      edgeEffectShader_.reload();
      celShadingShader_.reload();
      skyShader_.reload();
      grassShader_.reload();           // Partie 2
      bezierShader_.reload();          // Partie 1
      particleDrawShader_.reload();    // Partie 3
      particleComputeShader_.reload(); // Partie 3

      setLightingUniform();
      CHECK_GL_ERROR;
    }
    ImGui::End();

    switch (currentScene_) {
    case 0:
      sceneMain();
      break;
    }
    CHECK_GL_ERROR;
  }

  void onKeyPress(const sf::Event::KeyPressed &key) override {
    using enum sf::Keyboard::Key;
    switch (key.code) {
    case Escape:
      window_.close();
      break;
    case Space:
      isMouseMotionEnabled_ = !isMouseMotionEnabled_;
      if (isMouseMotionEnabled_) {
        window_.setMouseCursorGrabbed(true);
        window_.setMouseCursorVisible(false);
      } else {
        window_.setMouseCursorGrabbed(false);
        window_.setMouseCursorVisible(true);
      }
      break;
    case T:
      break;
    // Partie 1 : ajustement dynamique des divisions de la spline
    case PageUp:
      if (bezierNPoints < MAX_BEZIER_DIVISIONS)
        bezierNPoints++;
      break;
    case PageDown:
      if (bezierNPoints > 1)
        bezierNPoints--;
      break;
    // Partie 1 : lancement de l'animation caméra le long de la spline
    case B:
      isAnimatingCamera = true;
      cameraMode = 1;
      break;
    // Partie 2 : bascule du mode filaire pour déboguer la tessellation
    case G:
      wireframeMode_ = !wireframeMode_;
      break;
    default:
      break;
    }
  }

  void onResize(const sf::Event::Resized &event) override {}

  void onMouseMove(const sf::Event::MouseMoved &mouseDelta) override {
    if (!isMouseMotionEnabled_)
      return;

    const float MOUSE_SENSITIVITY = 0.1;
    float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
    float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
    cameraOrientation_.x -= cameraMouvementX * deltaTime_;
  }

  void updateCameraInput() {
    if (!window_.hasFocus())
      return;

    if (isMouseMotionEnabled_) {
      sf::Vector2u windowSize = window_.getSize();
      sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
      sf::Mouse::setPosition(windowHalfSize, window_);
    }

    float cameraMouvementX = 0;
    float cameraMouvementY = 0;

    const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
      cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
      cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
      cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
      cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
    cameraOrientation_.x -= cameraMouvementX * deltaTime_;

    glm::vec3 positionOffset = glm::vec3(0.0);
    const float SPEED = 10.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
      positionOffset.z -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
      positionOffset.z += SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
      positionOffset.x -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
      positionOffset.x += SPEED;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
      positionOffset.y -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
      positionOffset.y += SPEED;

    positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y,
                                 glm::vec3(0.0, 1.0, 0.0)) *
                     glm::vec4(positionOffset, 1);
    cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
  }

  void loadModels() {
    car_.loadModels();
    tree_.load("../models/pine.ply");
    streetlight_.load("../models/streetlight.ply");
    streetlightLight_.load("../models/streetlight_light.ply");
    skybox_.load("../models/skybox.ply");

    grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements));
    street_.load(street, sizeof(street), planeElements, sizeof(planeElements));
    streetcorner_.load(streetcorner, sizeof(streetcorner), planeElements,
                       sizeof(planeElements));
  }

  void drawModel(const Model &model, glm::mat4 &projView, glm::mat4 &view,
                 glm::mat4 modelMatrix) {
    celShadingShader_.use();
    glm::mat4 mvp = projView * modelMatrix;
    celShadingShader_.setMatrices(mvp, view, modelMatrix);
    model.draw();
  }

  void drawOutlinedModel(const Model &model, glm::mat4 &projView,
                         glm::mat4 &view, glm::mat4 modelMatrix) {
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    celShadingShader_.use();
    glm::mat4 mvp = projView * modelMatrix;
    celShadingShader_.setMatrices(mvp, view, modelMatrix);
    model.draw();

    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_CULL_FACE);
    edgeEffectShader_.use();
    glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE,
                       glm::value_ptr(mvp));
    model.draw();
    glEnable(GL_CULL_FACE);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
  }

  void drawStreetlights(glm::mat4 &projView, glm::mat4 &view) {

    for (unsigned int i = 0; i < N_STREETLIGHTS; i++) {
      if (!isDay_)
        setMaterial(streetlightLightMat);
      else
        setMaterial(streetlightMat);

      streetlightLightTexture_.use();
      drawOutlinedModel(streetlightLight_, projView, view,
                        streetlightModelMatrices_[i]);

      setMaterial(streetlightMat);
      streetlightTexture_.use();
      drawOutlinedModel(streetlight_, projView, view,
                        streetlightModelMatrices_[i]);
    }
  }

  void drawTree(glm::mat4 &projView, glm::mat4 &view) {
    glDisable(GL_CULL_FACE);
    for (unsigned int i = 0; i < N_TREES; i++) {
      treeTexture_.use();
      drawOutlinedModel(tree_, projView, view, treeModelMatrices_[i]);
    }
    glEnable(GL_CULL_FACE);
  }

  void drawGround(glm::mat4 &projView, glm::mat4 &view) {

    setMaterial(streetMat);
    streetcornerTexture_.use();
    for (int i = 0; i < 4; ++i)
      drawModel(streetcorner_, projView, view, streetPatchesModelMatrices_[i]);
    streetTexture_.use();
    for (int i = 4; i < N_STREET_PATCHES; ++i)
      drawModel(street_, projView, view, streetPatchesModelMatrices_[i]);

    setMaterial(grassMat);
    grassTexture_.use();
    drawModel(grass_, projView, view, groundModelMatrice_);
  }

  glm::mat4 getViewMatrix() {
    if (cameraMode == 1) {
      // Partie 1 (E3) : la caméra regarde toujours la voiture pendant
      // l'animation
      glm::vec3 target = car_.position;
      glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
      if (glm::length(target - cameraPosition_) < 0.001f)
        return glm::mat4(1.0f);
      glm::vec3 dir = glm::normalize(target - cameraPosition_);
      // Si la direction est quasi-verticale, éviter l'ambiguïté du vecteur up
      if (glm::abs(glm::dot(dir, up)) > 0.99f)
        up = glm::vec3(0.0f, 0.0f, 1.0f);
      return glm::lookAt(cameraPosition_, target, up);
    }
    glm::mat4 view = glm::mat4(1.0f);
    view =
        glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
    view =
        glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, -cameraPosition_);
    return view;
  }

  glm::mat4 getPerspectiveProjectionMatrix() {
    const float far = 300.f;

    float fov = glm::radians(70.0f);
    float aspect = getWindowAspect();
    float nearPlane = 0.1f;
    float farPlane = 300.0f;
    return glm::perspective(fov, aspect, nearPlane, farPlane);
  }

  void setLightingUniform() {
    celShadingShader_.use();
    glUniform1i(celShadingShader_.nSpotLightsULoc, N_STREETLIGHTS + 4);

    float ambientIntensity = 0.05;
    glUniform3f(celShadingShader_.globalAmbientULoc, ambientIntensity,
                ambientIntensity, ambientIntensity);
  }

  void toggleSun() {
    if (isDay_) {
      lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
      lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
      lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
    } else {
      lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
  }

  void toggleStreetlight() {
    if (isDay_) {
      for (unsigned int i = 0; i < N_STREETLIGHTS; i++) {
        lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
        lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
        lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
      }
    } else {
      for (unsigned int i = 0; i < N_STREETLIGHTS; i++) {
        lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
        lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
        lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
      }
    }
  }

  void updateCarLight() {
    if (car_.isHeadlightOn) {
      lightsData_.spotLights[N_STREETLIGHTS].ambient =
          glm::vec4(glm::vec3(0.01), 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS].diffuse =
          glm::vec4(glm::vec3(1.0), 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS].specular =
          glm::vec4(glm::vec3(0.4), 0.0f);

      lightsData_.spotLights[N_STREETLIGHTS + 1].ambient =
          glm::vec4(glm::vec3(0.01), 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse =
          glm::vec4(glm::vec3(1.0), 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 1].specular =
          glm::vec4(glm::vec3(0.4), 0.0f);

      lightsData_.spotLights[N_STREETLIGHTS].position =
          car_.carModel * glm::vec4(-1.6, 0.64, -0.45, 1.0f);
      lightsData_.spotLights[N_STREETLIGHTS].direction =
          glm::mat3(car_.carModel) * glm::vec3(-10, -1, 0);

      lightsData_.spotLights[N_STREETLIGHTS + 1].position =
          car_.carModel * glm::vec4(-1.6, 0.64, 0.45, 1.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 1].direction =
          glm::mat3(car_.carModel) * glm::vec3(-10, -1, 0);
    } else {
      lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(0.0f);

      lightsData_.spotLights[N_STREETLIGHTS + 1].ambient = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 1].specular = glm::vec4(0.0f);
    }

    if (car_.isBraking) {
      lightsData_.spotLights[N_STREETLIGHTS + 2].ambient =
          glm::vec4(0.01, 0.0, 0.0, 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse =
          glm::vec4(0.9, 0.1, 0.1, 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 2].specular =
          glm::vec4(0.35, 0.05, 0.05, 0.0f);

      lightsData_.spotLights[N_STREETLIGHTS + 3].ambient =
          glm::vec4(0.01, 0.0, 0.0, 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse =
          glm::vec4(0.9, 0.1, 0.1, 0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 3].specular =
          glm::vec4(0.35, 0.05, 0.05, 0.0f);

      lightsData_.spotLights[N_STREETLIGHTS + 2].position =
          car_.carModel * glm::vec4(1.6, 0.64, -0.45, 1.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 2].direction =
          glm::mat3(car_.carModel) * glm::vec3(10, -1, 0);

      lightsData_.spotLights[N_STREETLIGHTS + 3].position =
          car_.carModel * glm::vec4(1.6, 0.64, 0.45, 1.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 3].direction =
          glm::mat3(car_.carModel) * glm::vec3(10, -1, 0);
    } else {
      lightsData_.spotLights[N_STREETLIGHTS + 2].ambient = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 2].specular = glm::vec4(0.0f);

      lightsData_.spotLights[N_STREETLIGHTS + 3].ambient = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse = glm::vec4(0.0f);
      lightsData_.spotLights[N_STREETLIGHTS + 3].specular = glm::vec4(0.0f);
    }
  }
  void drawSkybox(glm::mat4 &view, glm::mat4 &proj) {
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    skyShader_.use();

    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    glm::mat4 mvp = proj * viewNoTranslation;
    glUniformMatrix4fv(skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));

    glActiveTexture(GL_TEXTURE0);
    if (isDay_)
      skyboxTexture_.use();
    else
      skyboxNightTexture_.use();

    skybox_.draw();

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
  }

  void setMaterial(Material &mat) {
    material_.updateData(&mat, 0, sizeof(Material));
  }

  // Partie 1 (E1/E2) : recalcule les points de la spline et met à jour le VBO
  // via glBufferSubData sans réallouer (le buffer a la taille maximale depuis
  // init).
  void updateSplineBuffer() {
    if (bezierNPoints == 0) {
      bezierSplineVertexCount_ = 0;
      return;
    }

    // Avec N divisions par courbe : première courbe donne N+1 points,
    // chaque courbe suivante ajoute N points (l'extrémité finale est partagée).
    // Total : 5*N + 1 points.
    std::vector<glm::vec3> pts;
    pts.reserve(5 * bezierNPoints + 1);

    for (unsigned int ci = 0; ci < 5; ++ci) {
      // Sauter t=0 pour les courbes suivantes (point partagé avec la
      // précédente)
      unsigned int startJ = (ci == 0) ? 0 : 1;
      for (unsigned int j = startJ; j <= bezierNPoints; ++j) {
        float t = (float)j / (float)bezierNPoints;
        pts.push_back(evalBezier(curves[ci], t));
      }
    }

    bezierSplineVertexCount_ = (GLsizei)pts.size();
    glBindBuffer(GL_ARRAY_BUFFER, vboSpline_);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    bezierSplineVertexCount_ * sizeof(glm::vec3), pts.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // Partie 2 (E4): génère un maillage de triangles couvrant la zone de gazon.
  // Alloué une seule fois à l'initialisation — jamais réalloué.
  void initGrassMesh() {
    std::vector<glm::vec3> verts;
    verts.reserve(GRASS_GRID_RES * GRASS_GRID_RES *
                  6); // 2 triangles × 3 sommets par cellule

    const float step = (2.0f * GRASS_HALF_SIZE) / (float)GRASS_GRID_RES;

    for (int row = 0; row < GRASS_GRID_RES; ++row) {
      for (int col = 0; col < GRASS_GRID_RES; ++col) {
        float x0 = -GRASS_HALF_SIZE + col * step;
        float x1 = x0 + step;
        float z0 = -GRASS_HALF_SIZE + row * step;
        float z1 = z0 + step;

        // Triangle 1
        verts.push_back({x0, 0.0f, z0});
        verts.push_back({x1, 0.0f, z0});
        verts.push_back({x1, 0.0f, z1});

        // Triangle 2
        verts.push_back({x0, 0.0f, z0});
        verts.push_back({x1, 0.0f, z1});
        verts.push_back({x0, 0.0f, z1});
      }
    }

    grassVertexCount_ = (GLsizei)verts.size();

    glGenVertexArrays(1, &grassVAO_);
    glBindVertexArray(grassVAO_);

    glGenBuffers(1, &grassVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO_);
    glBufferData(GL_ARRAY_BUFFER, grassVertexCount_ * sizeof(glm::vec3),
                 verts.data(), GL_STATIC_DRAW);

    // Attribut 0 : position (3 flottants)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void sceneMain() {
    // Partie 1-2

    ImGui::Begin("Scene Parameters");

    // DONE: Ajouté - slider pour le nombre de divisions de la spline Bézier
    ImGui::SliderInt("Bezier Divisions Per Curve", (int *)&bezierNPoints, 1,
                     MAX_BEZIER_DIVISIONS);
    if (ImGui::Button("Animate Camera")) {
      isAnimatingCamera = true;
      cameraMode = 1;
    }
    //

    if (ImGui::Button("Toggle Day/Night")) {
      isDay_ = !isDay_;
      toggleSun();
      toggleStreetlight();
      lights_.updateData(&lightsData_, 0,
                         sizeof(DirectionalLight) +
                             N_STREETLIGHTS * sizeof(SpotLight));
    }
    ImGui::SliderFloat("Car Speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
    ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f,
                       "%.2f°");
    if (ImGui::Button("Reset Steering"))
      car_.steeringAngle = 0.f;
    ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
    ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
    ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
    ImGui::Checkbox("Brake", &car_.isBraking);
    ImGui::Checkbox("Auto drive", &isAutopilotEnabled_);
    ImGui::End();

    updateCameraInput();
    if (isAutopilotEnabled_) {
      updateCarOnTrack(deltaTime_);
    }

    car_.update(deltaTime_);

    if (isAnimatingCamera) {
      if (cameraAnimation < 5.0f) {
        // DONE: Animation de la caméra le long de la spline Bézier (E3)
        // cameraAnimation ∈ [0, 5) : chaque unité entière = une courbe
        int curveIdx = (int)cameraAnimation;
        if (curveIdx >= 5)
          curveIdx = 4;
        float localT = cameraAnimation - (float)curveIdx;
        localT = glm::clamp(localT, 0.0f, 1.0f);
        cameraPosition_ = evalBezier(curves[curveIdx], localT);

        cameraAnimation += deltaTime_ / 3.0f; // chaque courbe dure 3 secondes
      } else {
        // Remise à 0 de l'orientation
        glm::vec3 distance = car_.position - cameraPosition_;

        float horizontalDistance =
            sqrt(distance.x * distance.x + distance.z * distance.z);

        cameraOrientation_.y = atan2(-distance.x, -distance.z);
        cameraOrientation_.x = atan2(distance.y, horizontalDistance);

        cameraAnimation = 0.f;
        isAnimatingCamera = false;
        cameraMode = 0;
      }
    }

    updateCarLight();

    lights_.updateData(&lightsData_.spotLights[N_STREETLIGHTS],
                       sizeof(DirectionalLight) +
                           N_STREETLIGHTS * sizeof(SpotLight),
                       4 * sizeof(SpotLight));

    bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
    if (hasNumberOfSidesChanged) {
      oldBezierNPoints = bezierNPoints;
      // DONE: Mise à jour du buffer de la spline via glBufferSubData (E1/E2)
      updateSplineBuffer();
    }

    // DONE: Dessin du gazon procédural effectué ci-dessous après le calcul des
    // matrices (Partie 2)

    glm::mat4 view = getViewMatrix();
    glm::mat4 proj = getPerspectiveProjectionMatrix();
    glm::mat4 projView = proj * view;

    drawSkybox(view, proj);

    celShadingShader_.use();

    glActiveTexture(GL_TEXTURE0);

    setMaterial(streetMat);
    drawGround(projView, view);

    setMaterial(treeMat);
    drawTree(projView, view);

    setMaterial(streetlightMat);
    drawStreetlights(projView, view);

    setMaterial(defaultMat);
    carTexture_.use();
    car_.draw(projView, view);

    setMaterial(windowMat);
    carWindowTexture_.use();
    car_.drawWindows(projView, view);

    // Partie 2 (E4/E5/E6/E7/E10/E11/E12): dessin du gazon procédural par
    // tessellation
    glDisable(GL_CULL_FACE); // E5: brins d'herbe visibles des deux côtés
    glPatchParameteri(GL_PATCH_VERTICES, 3); // 3 sommets par patch triangulaire

    grassShader_.use();
    glUniform3fv(grassShader_.cameraPosULoc, 1,
                 glm::value_ptr(cameraPosition_));
    glUniformMatrix4fv(grassShader_.projViewULoc, 1, GL_FALSE,
                       glm::value_ptr(projView));

    if (wireframeMode_)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(grassVAO_);
    glDrawArrays(GL_PATCHES, 0, grassVertexCount_);
    glBindVertexArray(0);

    if (wireframeMode_)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_CULL_FACE);

    // Partie 1 (E2) : dessin de la spline Bézier avec shader non-éclairé
    if (bezierSplineVertexCount_ > 0) {
      bezierShader_.use();
      glUniformMatrix4fv(bezierShader_.mvpULoc, 1, GL_FALSE,
                         glm::value_ptr(projView));
      glUniform3f(bezierShader_.lineColorULoc, 1.0f, 1.0f, 1.0f); // blanc

      glBindVertexArray(vaoSpline_);
      glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)bezierSplineVertexCount_);
      glBindVertexArray(0);
    }

    // DONE: Mise à jour du compteur de particules actives (Partie 3)
    totalTime += deltaTime_;
    timerParticles_ += deltaTime_;
    const float particlesSpawnInterval = 0.2f;

    unsigned int particlesToAdd = timerParticles_ / particlesSpawnInterval;
    timerParticles_ -= particlesToAdd * particlesSpawnInterval;

    nParticles_ += particlesToAdd;
    if (nParticles_ > MAX_PARTICLES_)
      nParticles_ = MAX_PARTICLES_;

    // DONE: Position et direction de l'échappement en espace monde (Partie 3,
    // E15)
    glm::vec3 exhaustPos =
        glm::vec3(car_.carModel * glm::vec4(2.0f, 0.24f, -0.43f, 1.0f));
    glm::vec3 exhaustDir =
        glm::mat3(car_.carModel) * glm::vec3(1.0f, 0.0f, 0.0f);

    // DONE: Passe de calcul — compute shader met à jour les particules (Partie
    // 3, E15/E18–E23)
    particleComputeShader_.use();
    glUniform1f(particleComputeShader_.timeULoc, (float)totalTime);
    glUniform1f(particleComputeShader_.deltaTimeULoc, deltaTime_);
    glUniform3fv(particleComputeShader_.emitterPositionULoc, 1,
                 glm::value_ptr(exhaustPos));
    glUniform3fv(particleComputeShader_.emitterDirectionULoc, 1,
                 glm::value_ptr(exhaustDir));

    // particles_[ssboIndex_] = lecture, particles_[1-ssboIndex_] = écriture
    particles_[ssboIndex_].setBindingIndex(0);
    particles_[1 - ssboIndex_].setBindingIndex(1);

    // Un work group de 64 invocations couvre exactement MAX_PARTICLES_
    glDispatchCompute(1, 1, 1);

    // Barrière : attendre que le compute ait fini avant de lire les données en
    // vertex
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // DONE: Passe de dessin — billboard quads avec blending (Partie 3,
    // E16/E17/E24–E28) Attention : la texture de fumée est transparente —
    // désactiver l'écriture de profondeur et activer le blending alpha pour un
    // rendu correct.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    particleDrawShader_.use();
    glUniformMatrix4fv(particleDrawShader_.modelViewULoc, 1, GL_FALSE,
                       glm::value_ptr(view));
    glUniformMatrix4fv(particleDrawShader_.projectionULoc, 1, GL_FALSE,
                       glm::value_ptr(proj));
    glUniform1i(particleDrawShader_.smokeTextureULoc, 0);

    glActiveTexture(GL_TEXTURE0);
    smokeTexture_.use();

    // Rebinder le buffer de sortie du compute comme source des attributs vertex
    int drawBuf = 1 - ssboIndex_;
    glBindVertexArray(vaoParticles_);
    particles_[drawBuf].bindAsArray();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)32);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)48);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (void *)12);
    glDrawArrays(GL_POINTS, 0, nParticles_);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Restaurer l'état du contexte OpenGL
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // DONE: Interchanger les deux buffers pour la prochaine frame (Partie 3)
    ssboIndex_ = 1 - ssboIndex_;
  }

  void initStaticMatrices() {
    const float ROAD_HALF_LENGTH = 15.0f;
    const float ROAD_WIDTH = 5.0f;
    const float SEGMENT_LENGTH = 30.0f / 7.0f;

    groundModelMatrice_ =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
    groundModelMatrice_ =
        glm::scale(groundModelMatrice_, glm::vec3(50.0f, 1.0f, 50.0f));
    treeModelMatrices_[0] =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    treeModelMatrices_[0] =
        glm::scale(treeModelMatrices_[0], glm::vec3(15.0f, 15.0f, 15.0f));

    const float lightOffset = ROAD_HALF_LENGTH - (ROAD_WIDTH * 0.5f) - 0.25f;
    const glm::vec3 positions[] = {
        {-7.5f, -0.15f, -lightOffset}, {7.5f, -0.15f, -lightOffset},
        {-7.5f, -0.15f, lightOffset},  {7.5f, -0.15f, lightOffset},
        {-lightOffset, -0.15f, -7.5f}, {-lightOffset, -0.15f, 7.5f},
        {lightOffset, -0.15f, -7.5f},  {lightOffset, -0.15f, 7.5f}};

    for (int i = 0; i < N_STREETLIGHTS; ++i) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), positions[i]);
      glm::vec3 toCenter = glm::normalize(glm::vec3(0.0f) - positions[i]);
      float rotation =
          std::atan2(-toCenter.x, -toCenter.z) + glm::radians(90.0f);
      streetlightModelMatrices_[i] =
          glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
      streetlightLightPositions[i] = glm::vec3(streetlightModelMatrices_[i] *
                                               glm::vec4(-2.77, 5.2, 0.0, 1.0));
    }

    int patchIndex = 0;
    const glm::vec3 corners[] = {{-ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH},
                                 {ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH},
                                 {-ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH},
                                 {ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH}};

    for (const auto &pos : corners) {
      glm::mat4 cornerModel = glm::translate(glm::mat4(1.0f), pos);
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(cornerModel, glm::vec3(ROAD_WIDTH, 1.0f, ROAD_WIDTH));
    }

    for (int i = 0; i < 7; ++i) {
      float x =
          -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

      glm::mat4 top = glm::translate(glm::mat4(1.0f),
                                     glm::vec3(x, 0.0f, -ROAD_HALF_LENGTH));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(top, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

      glm::mat4 bottom =
          glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, ROAD_HALF_LENGTH));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(bottom, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
    }

    for (int i = 0; i < 7; ++i) {
      float z =
          -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

      glm::mat4 left = glm::translate(glm::mat4(1.0f),
                                      glm::vec3(-ROAD_HALF_LENGTH, 0.0f, z));
      left =
          glm::rotate(left, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(left, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

      glm::mat4 right =
          glm::translate(glm::mat4(1.0f), glm::vec3(ROAD_HALF_LENGTH, 0.0f, z));
      right =
          glm::rotate(right, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      streetPatchesModelMatrices_[patchIndex++] =
          glm::scale(right, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
    }
  }

  void updateCarOnTrack(float deltaTime) {
    const float ROAD_HALF_LENGTH = 15.0f;
    const float SEGMENT_LENGTH = ROAD_HALF_LENGTH * 2.0f;
    const float PERIMETER = 4.0f * SEGMENT_LENGTH;

    trackDistance_ += car_.speed * deltaTime;
    trackDistance_ = std::fmod(trackDistance_, PERIMETER);
    if (trackDistance_ < 0.0f)
      trackDistance_ += PERIMETER;

    float d = trackDistance_;

    if (d < SEGMENT_LENGTH) {
      car_.position = glm::vec3(-ROAD_HALF_LENGTH + d, 0.0f, ROAD_HALF_LENGTH);
      car_.orientation.y = glm::radians(180.0f);
    } else if (d < 2.0f * SEGMENT_LENGTH) {
      d -= SEGMENT_LENGTH;
      car_.position = glm::vec3(ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH - d);
      car_.orientation.y = glm::radians(-90.0f);
    } else if (d < 3.0f * SEGMENT_LENGTH) {
      d -= 2.0f * SEGMENT_LENGTH;
      car_.position = glm::vec3(ROAD_HALF_LENGTH - d, 0.0f, -ROAD_HALF_LENGTH);
      car_.orientation.y = 0.0f;
    } else {
      d -= 3.0f * SEGMENT_LENGTH;
      car_.position = glm::vec3(-ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH + d);
      car_.orientation.y = glm::radians(90.0f);
    }
  }

private:
  // Shaders
  EdgeEffect edgeEffectShader_;
  CelShading celShadingShader_;
  Sky skyShader_;
  // DONE: GrassShader ajouté (Partie 2)
  GrassShader grassShader_;
  // DONE: BezierShader ajouté (Partie 1)
  BezierShader bezierShader_;
  // DONE: Shaders de particules ajoutés (Partie 3)
  ParticleDrawShader particleDrawShader_;
  ParticleComputeShader particleComputeShader_;

  // Textures
  Texture2D grassTexture_;
  Texture2D streetTexture_;
  Texture2D streetcornerTexture_;
  Texture2D carTexture_;
  Texture2D carWindowTexture_;
  Texture2D treeTexture_;
  Texture2D streetlightTexture_;
  Texture2D streetlightLightTexture_;
  TextureCubeMap skyboxTexture_;
  TextureCubeMap skyboxNightTexture_;
  // DONE: Texture de fumée pour les particules (Partie 3)
  Texture2D smokeTexture_;

  struct {
    DirectionalLight dirLight;
    SpotLight spotLights[12];
  } lightsData_;

  // Uniform buffers
  UniformBuffer material_;
  UniformBuffer lights_;

  Model tree_;
  Model streetlight_;
  Model streetlightLight_;
  Model grass_;
  Model street_;
  Model streetcorner_;
  Model skybox_;

  Car car_;

  glm::vec3 cameraPosition_;
  glm::vec2 cameraOrientation_;

  // Matrices
  static constexpr unsigned int N_TREES = 1;
  static constexpr unsigned int N_STREETLIGHTS = 8;
  static constexpr unsigned int N_STREET_PATCHES = 7 * 4 + 4;
  glm::mat4 treeModelMatrices_[N_TREES];
  glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
  glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
  glm::mat4 groundModelMatrice_;
  glm::vec3 streetlightLightPositions[N_STREETLIGHTS];

  bool isDay_;
  bool isMouseMotionEnabled_;
  bool isAutopilotEnabled_;
  float trackDistance_;
  double totalTime;
  double timerParticles_;
  int nParticles_;

  unsigned int bezierNPoints = 20; // divisions par courbe (défaut 20)
  unsigned int oldBezierNPoints = 0;

  int cameraMode = 0;
  float cameraAnimation = 0.f;
  bool isAnimatingCamera = false;

  // Partie 2
  // DONE: grassVAO_ et grassVBO_ pour le maillage de patches du gazon (alloués
  // une seule fois)
  GLuint grassVAO_ = 0;
  GLuint grassVBO_ = 0;
  GLsizei grassVertexCount_ = 0;
  bool wireframeMode_ = false;

  // Grille couvrant la zone intérieure de la route (±12 unités, en deçà de
  // l'arête interne à ±12.5)
  static constexpr int GRASS_GRID_RES =
      20; // cellules par côté (20×20 = 800 patches)
  static constexpr float GRASS_HALF_SIZE = 12.0f; // demi-côté en unités monde

  // Partie 1
  // DONE: vaoSpline_ et vboSpline_ pour la spline Bézier (alloués une seule
  // fois)
  GLuint vaoSpline_ = 0;
  GLuint vboSpline_ = 0;
  GLsizei bezierSplineVertexCount_ = 0;

  // Nombre maximal de divisions par courbe (pour le pré-allocation du buffer)
  static constexpr unsigned int MAX_BEZIER_DIVISIONS = 100;
  // 5 courbes * MAX_BEZIER_DIVISIONS divisions + 1 point final
  static constexpr unsigned int MAX_BEZIER_POINTS =
      5 * MAX_BEZIER_DIVISIONS + 1;

  // Partie 3
  // DONE: VAO et SSBOs double-buffered pour les particules
  GLuint vaoParticles_ = 0;
  int ssboIndex_ = 0; // index du buffer en lecture (l'autre est en écriture)

  static const unsigned int MAX_PARTICLES_ = 64;

  // SSBOs ping-pong : particles_[ssboIndex_] = entrée, particles_[1-ssboIndex_]
  // = sortie
  ShaderStorageBuffer particles_[2];

  // Imgui var
  const char *const SCENE_NAMES[1] = {"Main scene"};
  const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
  int currentScene_;
};

int main(int argc, char *argv[]) {
  WindowSettings settings = {};
  settings.fps = 60;
  settings.context.depthBits = 24;
  settings.context.stencilBits = 8;
  settings.context.antiAliasingLevel = 4;
  settings.context.majorVersion =
      4; // Partie 3 : compute shader nécessite OpenGL 4.3+
  settings.context.minorVersion = 3;
  settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

  App app;
  app.run(argc, argv, "Tp2", settings);
}
