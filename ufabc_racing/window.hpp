#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

#include "barreiras.hpp"
#include "carrinho.hpp"
#include "faixa.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  glm::ivec2 m_viewportSize{};

  GLuint m_starsProgram{};
  GLuint m_objectsProgram{};

  GameData m_gameData;

  Barreiras m_barreiras;
  Carrinho m_carrinho;
  Faixas m_faixas;


  abcg::Timer m_restartWaitTimer;

  ImFont *m_font{};

  std::default_random_engine m_randomEngine;
  std::uniform_int_distribution<int> m_randomDist{1, 3};

  float control_time;
  int score;

  void restart();
  void checkCollisions();
  void checkWinCondition();
};

#endif