#include "asteroids.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Asteroids::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create asteroids
  m_asteroids.clear();
  m_asteroids.resize(quantity);

  for (auto &asteroid : m_asteroids) {
    asteroid = makeAsteroid();

    // Make sure the asteroid won't collide with the ship
    do {
      asteroid.m_translation = {m_randomDist(m_randomEngine),
                                m_randomDist(m_randomEngine)};
    } while (glm::length(asteroid.m_translation) < 0.5f);
  }
}

void Asteroids::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &asteroid : m_asteroids) {
    abcg::glBindVertexArray(asteroid.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &asteroid.m_color.r);
    abcg::glUniform1f(m_scaleLoc, asteroid.m_scale);
    
    for (auto i : {-2, 0, 2}) {
      for (auto j : {-2, 0, 2}) {
        abcg::glUniform2f(m_translationLoc, asteroid.m_translation.x + j,
                          asteroid.m_translation.y + i);

        abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, asteroid.m_polygonSides + 2);
      }
    }

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}


void Asteroids::destroy() {
  for (auto &asteroid : m_asteroids) {
    abcg::glDeleteBuffers(1, &asteroid.m_VBO);
    abcg::glDeleteVertexArrays(1, &asteroid.m_VAO);
  }
}

void Asteroids::update(const Ship &ship, float deltaTime) {
  for (auto &asteroid : m_asteroids) {
    // Atualize a posição no eixo Y para fazer os asteroides deslizarem para baixo
    asteroid.m_translation.y -= deltaTime * 0.5f;

    // Wrap-around
    if (asteroid.m_translation.x < -1.0f)
      asteroid.m_translation.x += 2.0f;
    if (asteroid.m_translation.x > +1.0f)
      asteroid.m_translation.x -= 2.0f;
    if (asteroid.m_translation.y < -1.0f)
      asteroid.m_translation.y += 2.0f;
    if (asteroid.m_translation.y > +1.0f)
      asteroid.m_translation.y -= 2.0f;
  }
}


Asteroids::Asteroid Asteroids::makeAsteroid(glm::vec2 translation,
                                            float scale) {
  Asteroid asteroid;

  // Define os vértices do asteroid com formato fixo
  std::array positions{
      glm::vec2{-02.5f, +15.5f}, glm::vec2{+2.5f, +15.5f},
      glm::vec2{+2.5f, -15.5f}, glm::vec2{-02.5f, -15.5f},
      glm::vec2{-02.5f, +10.0f}, glm::vec2{-08.0f, +05.5f},
      glm::vec2{-08.0f, +10.0f}, glm::vec2{+02.5f, +10.0f},
      glm::vec2{+08.0f, +05.5f}, glm::vec2{+08.0f, +10.0f},
      glm::vec2{-02.5f, -10.0f}, glm::vec2{-08.0f, -15.5f},
      glm::vec2{-08.0f, -10.0f}, glm::vec2{+02.5f, -10.0f},
      glm::vec2{+08.0f, -15.5f}, glm::vec2{+08.0f, -10.0f},
      glm::vec2{-02.5f, +05.5f}, glm::vec2{+02.5f, +05.5f},
  };

  // Normalize os vértices para escala
  for (auto &position : positions) {
    position /= glm::vec2{15.5f, 15.5f};
  }

     std::array const indices{0, 1, 2,
                           0, 2, 3,
                           5,4,6,
                           5,4,16,
                           7,8,9,
                           7 ,8,17,
                           3,10,11,
                           10,11,12,
                           2,13,14,
                           13,14,15};
  // clang-format on


  // Defina a escala, translação, velocidade e outras propriedades do asteroid
  asteroid.m_polygonSides = positions.size() - 1;
  asteroid.m_color = glm::vec4{0,1,0,1}; // Cor verde
  asteroid.m_color.a = 1.0f;
  asteroid.m_rotation = 0.0f;
  asteroid.m_scale = scale;
  asteroid.m_translation = translation;
  asteroid.m_angularVelocity = m_randomDist(m_randomEngine);

  // Crie o VBO (Buffer de Vértices)
  abcg::glGenBuffers(1, &asteroid.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Obtenha a localização dos atributos no programa
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Crie o VAO (Array de Vértices)
  abcg::glGenVertexArrays(1, &asteroid.m_VAO);

  // Vincule os atributos de vértices ao VAO
  abcg::glBindVertexArray(asteroid.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Fim da vinculação ao VAO
  abcg::glBindVertexArray(0);

  return asteroid;
}