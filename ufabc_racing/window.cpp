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

  // Create program to render the stars
  m_starsProgram =
      abcg::createOpenGLProgram({{.source = assetsPath + "stars.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "stars.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

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

  m_starLayers.create(m_starsProgram, 25);
  m_ship.create(m_objectsProgram);
  m_asteroids.create(m_objectsProgram, m_randomDist(m_randomEngine));
  m_bullets.create(m_objectsProgram);

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
    m_ship.m_translation.x -= velocidade_de_deslocamento * deltaTime;
  }

  if (m_gameData.m_input[static_cast<size_t>(Input::Right)]) {
    // Movimente o objeto para a direita
    m_ship.m_translation.x += velocidade_de_deslocamento * deltaTime;
  }

  m_ship.update(m_gameData, deltaTime);
  m_starLayers.update(m_ship, deltaTime);
  m_asteroids.update(m_ship, deltaTime);
  m_bullets.update(m_ship, m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }

  if (control_time > 2.5f){
    m_asteroids.create(m_objectsProgram, (m_randomDist(m_randomEngine) + score/10));
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

  m_starLayers.paint();
  m_asteroids.paint();
  m_bullets.paint();
  m_ship.paint(m_gameData);
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

  m_asteroids.destroy();
  m_bullets.destroy();
  m_ship.destroy();
  m_starLayers.destroy();
}

void Window::checkCollisions() {
  // Check collision between ship and asteroids
  for (auto const &asteroid : m_asteroids.m_asteroids) {
    auto const asteroidTranslation{asteroid.m_translation};
    auto const distance{
        glm::distance(m_ship.m_translation, asteroidTranslation)};

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 0.85f) {
      m_gameData.m_state = State::GameOver;
      m_restartWaitTimer.restart();
    }
  }

  // Check collision between bullets and asteroids
  for (auto &bullet : m_bullets.m_bullets) {
    if (bullet.m_dead)
      continue;

    for (auto &asteroid : m_asteroids.m_asteroids) {
      for (auto const i : {-2, 0, 2}) {
        for (auto const j : {-2, 0, 2}) {
          auto const asteroidTranslation{asteroid.m_translation +
                                         glm::vec2(i, j)};
          auto const distance{
              glm::distance(bullet.m_translation, asteroidTranslation)};

          if (distance < m_bullets.m_scale + asteroid.m_scale * 0.85f) {
            asteroid.m_hit = true;
            bullet.m_dead = true;
          }
        }
      }
    }

    // Break asteroids marked as hit
    for (auto const &asteroid : m_asteroids.m_asteroids) {
      if (asteroid.m_hit && asteroid.m_scale > 0.10f) {
        std::uniform_real_distribution randomDist{-1.0f, 1.0f};
        std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 3, [&]() {
          glm::vec2 const offset{randomDist(m_randomEngine),
                                 randomDist(m_randomEngine)};
          auto const newScale{asteroid.m_scale * 0.5f};
          return m_asteroids.makeAsteroid(
              asteroid.m_translation + offset * newScale, newScale);
        });
      }
    }

    m_asteroids.m_asteroids.remove_if([](auto const &a) { return a.m_hit; });
  }
}

void Window::checkWinCondition() {
  if (m_asteroids.m_asteroids.empty() || score >= 30) {
    m_gameData.m_state = State::Win;
    m_restartWaitTimer.restart();
  }
}