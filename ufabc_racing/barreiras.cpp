#include "barreiras.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Barreiras::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create barreiras
  m_barreiras.clear();
  m_barreiras.resize(quantity);

  for (auto &barreira : m_barreiras) {
    barreira = makeBarreira();

    // Make sure the barreira won't collide with the carrinho
      barreira.m_translation = {m_randomDist(m_randomEngine), 1.5f};
  }
}

void Barreiras::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &barreira : m_barreiras) {
    abcg::glBindVertexArray(barreira.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &barreira.m_color.r);
    abcg::glUniform1f(m_scaleLoc, barreira.m_scale);

    for (auto i : {0, 0, 0}) {
      for (auto j : {0, 0, 0}) {
        abcg::glUniform2f(m_translationLoc, barreira.m_translation.x + j, barreira.m_translation.y + i);

        abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, 14*3);
      }
    }

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}


void Barreiras::destroy() {
  for (auto &barreira : m_barreiras) {
    abcg::glDeleteBuffers(1, &barreira.m_VBO);
    abcg::glDeleteVertexArrays(1, &barreira.m_VAO);
  }
}

void Barreiras::update(const Carrinho &carrinho, float deltaTime) {
  for (auto &barreira : m_barreiras) {
    // Atualize a posição no eixo Y para fazer os barreiraes deslizarem para baixo
    barreira.m_translation.y -= deltaTime * 1.2f;
  }
}


Barreiras::Barreira Barreiras::makeBarreira(glm::vec2 translation,
                                            float scale) {
  Barreira barreira;

  // Define os vértices do barreira com formato fixo
  std::array positions{
      glm::vec2{-3.0f, -1.0f}, glm::vec2{-3.0f, +1.0f},
      glm::vec2{+3.0f, +1.0f}, glm::vec2{+3.0f, -1.0f},
  };

  // Normalize os vértices para escala
  for (auto &position : positions) {
    position /= glm::vec2{5.0f, 5.0f}; 
  }

     std::array const indices{0, 1, 2,
                              0, 3, 2,
                              };
  // clang-format on


  // Defina a escala, translação, velocidade e outras propriedades do barreira
  barreira.m_polygonSides = positions.size() - 1;
  barreira.m_color = glm::vec4{1,0,0,1}; // Cor verde
  barreira.m_color.a = 1.0f;
  barreira.m_rotation = 0.0f;
  barreira.m_scale = scale;
  barreira.m_translation = translation;
  barreira.m_angularVelocity = m_randomDist(m_randomEngine);

  // Crie o VBO (Buffer de Vértices)
  abcg::glGenBuffers(1, &barreira.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, barreira.m_VBO);
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
  abcg::glGenVertexArrays(1, &barreira.m_VAO);

  // Vincule os atributos de vértices ao VAO
  abcg::glBindVertexArray(barreira.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, barreira.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Fim da vinculação ao VAO
  abcg::glBindVertexArray(0);

  return barreira;
}