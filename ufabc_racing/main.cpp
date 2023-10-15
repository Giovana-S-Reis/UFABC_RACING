#include "window.hpp"

int main(int argc, char **argv) {
  try {
    abcg::Application app(argc, argv);

    Window window;
    window.setOpenGLSettings({.samples = 16});
    window.setWindowSettings({
        .width = 900,
        .height = 900,
        .showFPS = false,
        .showFullscreenButton = false,
        .title = "Carrinho do Sr. Omar",
    });

    app.run(window);
  } catch (std::exception const &exception) {
    fmt::print(stderr, "{}\n", exception.what());
    return -1;
  }
  return 0;
}