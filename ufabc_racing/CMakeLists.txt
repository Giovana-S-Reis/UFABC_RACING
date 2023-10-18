project(UFABC_RACING)
add_executable(${PROJECT_NAME} main.cpp window.cpp barreiras.cpp carrinho.cpp)
enable_abcg(${PROJECT_NAME})
