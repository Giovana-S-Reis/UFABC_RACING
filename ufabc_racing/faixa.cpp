#include "faixa.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Faixas::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create faixa
  m_faixa.clear();
  m_faixa.resize(quantity);

  for (auto &faixa : m_faixa) {
    faixa = makeFaixa();

    // Make sure the faixa won't collide with the carrinho
      faixa.m_translation = {m_randomDist(m_randomEngine), 1.5f};
  }
}

void Faixas::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &faixa : m_faixa) {
    abcg::glBindVertexArray(faixa.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &faixa.m_color.r);
    abcg::glUniform1f(m_scaleLoc, faixa.m_scale);

    for (auto i : {0, 0, 0}) {
      for (auto j : {0, 0, 0}) {
        abcg::glUniform2f(m_translationLoc, faixa.m_translation.x + j, faixa.m_translation.y + i);

        abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, 14*3);
      }
    }

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}


void Faixas::destroy() {
  for (auto &faixa : m_faixa) {
    abcg::glDeleteBuffers(1, &faixa.m_VBO);
    abcg::glDeleteVertexArrays(1, &faixa.m_VAO);
  }
}

void Faixas::update(const Carrinho &carrinho, float deltaTime) {
  for (auto &faixa : m_faixa) {
    // Atualize a posição no eixo Y para fazer os faixaes deslizarem para baixo
    faixa.m_translation.y -= deltaTime * 1.2f;
  }
}


Faixas::Faixa Faixas::makeFaixa(glm::vec2 translation,
                                            float scale) {
  Faixa faixa;

  // Define os vértices do faixa com formato fixo
  std::array positions{
      glm::vec2{-1.0f, -3.0f}, glm::vec2{-1.0f, +3.0f},
      glm::vec2{+1.0f, +3.0f}, glm::vec2{+1.0f, -3.0f},
  };

  // Normalize os vértices para escala
  for (auto &position : positions) {
    position /= glm::vec2{5.0f, 5.0f}; 
  }

     std::array const indices{0, 1, 2,
                              0, 3, 2,
                              };
  // clang-format on


  // Defina a escala, translação, velocidade e outras propriedades do faixa
  faixa.m_polygonSides = positions.size() - 1;
  faixa.m_color = glm::vec4{255,255,255,1}; // Cor verde
  faixa.m_color.a = 1.0f;
  faixa.m_rotation = 0.0f;
  faixa.m_scale = scale;
  faixa.m_translation = translation;
  faixa.m_angularVelocity = m_randomDist(m_randomEngine);

  // Crie o VBO (Buffer de Vértices)
  abcg::glGenBuffers(1, &faixa.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, faixa.m_VBO);
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
  abcg::glGenVertexArrays(1, &faixa.m_VAO);

  // Vincule os atributos de vértices ao VAO
  abcg::glBindVertexArray(faixa.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, faixa.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Fim da vinculação ao VAO
  abcg::glBindVertexArray(0);

  return faixa;
}