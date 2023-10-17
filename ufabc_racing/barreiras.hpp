#ifndef ASTEROIDS_HPP_
#define ASTEROIDS_HPP_

#include <list>
#include <random>

#include "abcgOpenGL.hpp"

#include "gamedata.hpp"
#include "carrinho.hpp"

class Barreiras {
public:
  void create(GLuint program, int quantity);
  void paint();
  void destroy();
  void update(const Carrinho &carrinho, float deltaTime);

  struct Barreira {
    GLuint m_VAO{};
    GLuint m_VBO{};
    GLuint m_EBO{};

    float m_angularVelocity{};
    glm::vec4 m_color{1};
    int m_polygonSides{};
    float m_rotation{};
    float m_scale{};
    glm::vec2 m_translation{};
    glm::vec2 m_velocity{};
    bool m_hit{};
  };

  std::list<Barreira> m_barreiras;

  Barreira makeBarreira(glm::vec2 translation = {}, float scale = 0.25f);

private:
  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_rotationLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};
  GLuint m_EBO{};

  std::default_random_engine m_randomEngine;
  std::uniform_real_distribution<float> m_randomDist{-0.8f, +0.8f};
};

#endif