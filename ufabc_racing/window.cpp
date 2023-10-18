#include "window.hpp"

void Window::onEvent(SDL_Event const &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(gsl::narrow<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(gsl::narrow<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(gsl::narrow<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(gsl::narrow<size_t>(Input::Right));
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  // Load a new font
  auto const filename{assetsPath + "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError("Cannot load font file");
  }

  // Create program to render the other objects
  m_objectsProgram =
      abcg::createOpenGLProgram({{.source = assetsPath + "objects.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "objects.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  // // Create program to render the stars
  // m_starsProgram =
  //     abcg::createOpenGLProgram({{.source = assetsPath + "stars.vert",
  //                                 .stage = abcg::ShaderStage::Vertex},
  //                                {.source = assetsPath + "stars.frag",
  //                                 .stage = abcg::ShaderStage::Fragment}});

  abcg::glClearColor(0.5f, 0.5f, 0.5f, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restart();
}

void Window::restart() {
  m_gameData.m_state = State::Playing;

  m_carrinho.create(m_objectsProgram);
  m_barreiras.create(m_objectsProgram, m_randomDist(m_randomEngine));
  //m_faixas.create(m_objectsProgram, 3);

  control_time = 0;
  score = 0;
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  // Wait 2 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 2) {
    restart();
    return;
  }

  // Atualize a posição do objeto com base nos comandos de teclado
  float velocidade_de_deslocamento = 1.0f; // Ajuste a velocidade conforme necessário

  if (m_gameData.m_input[static_cast<size_t>(Input::Left)]) {
    // Movimente o objeto para a esquerda
    m_carrinho.m_translation.x -= velocidade_de_deslocamento * deltaTime;
  }

  if (m_gameData.m_input[static_cast<size_t>(Input::Right)]) {
    // Movimente o objeto para a direita
    m_carrinho.m_translation.x += velocidade_de_deslocamento * deltaTime;
  }

  m_carrinho.update(m_gameData, deltaTime);
  m_barreiras.update(m_carrinho, deltaTime);
  //m_faixas.update(m_carrinho, deltaTime);


  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }

  if (control_time > 2.5f){
    m_barreiras.create(m_objectsProgram, (m_randomDist(m_randomEngine) + score/10));
    control_time = 0;
    score++;
  }
  else{
    control_time += deltaTime;
  }
}


void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  //m_faixas.paint();
  m_barreiras.paint();
  m_carrinho.paint(m_gameData);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    
    auto const size{ImVec2(300, 85)};
    auto const position{ImVec2((m_viewportSize.x - size.x) / 2.0f,
                               (m_viewportSize.y - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("Game Over!");
    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("*You Win!*");
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_starsProgram);
  abcg::glDeleteProgram(m_objectsProgram);

  m_barreiras.destroy();
  m_carrinho.destroy();
  //m_faixas.destroy();
}

void Window::checkCollisions() {
  // Check collision between carrinhos and barreiras
  for (auto const &barreira : m_barreiras.m_barreiras) {
    auto const barreiraTranslation{barreira.m_translation};
    auto const distance{
        glm::distance(m_carrinho.m_translation, barreiraTranslation)};

    if (distance < m_carrinho.m_scale * 0.9f + barreira.m_scale * 0.85f) {
      m_gameData.m_state = State::GameOver;
      m_restartWaitTimer.restart();
    }
  }
}

void Window::checkWinCondition() {
  if (m_barreiras.m_barreiras.empty() || score >= 10) {
    m_gameData.m_state = State::Win;
    m_restartWaitTimer.restart();
  }
}