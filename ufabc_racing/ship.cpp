#include "ship.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Ship::create(GLuint program) {
  destroy();

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Reset ship attributes
  m_rotation = 0.0f;
  m_translation = glm::vec2(0.0f,-0.5f);
  m_velocity = glm::vec2(0);

  // clang-format off
  std::array positions{
      // Ship body
      glm::vec2{-10.0f, +06.0f}, glm::vec2{-10.0f, +09.0f},
      glm::vec2{-10.0f, +12.0f}, glm::vec2{-04.0f, +12.0f},
      glm::vec2{+04.0f, +12.0f}, glm::vec2{+10.0f, +12.0f},
      glm::vec2{+10.0f, +09.0f}, glm::vec2{+10.0f, +06.0f},
      glm::vec2{+04.0f, +09.0f}, glm::vec2{-04.0f, +09.0f},
      glm::vec2{-04.0f, -06.0f}, glm::vec2{+04.0f, -06.0f},
      glm::vec2{+04.0f, -08.0f}, glm::vec2{-04.0f, -08.0f},
      glm::vec2{+09.0f, -08.0f}, glm::vec2{-09.0f, -08.0f},
      glm::vec2{+09.0f, -12.0f}, glm::vec2{-09.0f, -12.0f},
      };

  for (auto &position : positions) {
    position /= glm::vec2{10.0f, 10.0f}; 
  }

   std::array const indices{0, 1, 9,
                           1, 2, 9,
                           2,3,9,
                           3,9,8,
                           4,3,8,
                           4,5,6,
                           4,6,8,
                           6,7,8,
                           9,10,11,
                           8,9,11,
                           10,11,12,
                           10,13,12,
                           14,16,17,
                           14,15,17};
  // clang-format on

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Ship::paint(const GameData &gameData) {
  if (gameData.m_state != State::Playing)
    return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_VAO);

  abcg::glUniform1f(m_scaleLoc, m_scale);
  abcg::glUniform1f(m_rotationLoc, m_rotation);
  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 14 * 3, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Ship::destroy() {
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Ship::update(GameData const &gameData, float deltaTime) {
  if (gameData.m_state != State::Playing) {
    // Stop ship's movement when not playing
    m_velocity = glm::vec2(0.0f);
  } else {
    // Move the ship forward in its current direction
    auto const forward = glm::rotate(glm::vec2{0.0f, 1.0f}, m_rotation);
    m_translation += forward * m_velocity * deltaTime;
  }
}
