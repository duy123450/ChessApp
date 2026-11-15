cmake_minimum_required(VERSION 3.28)

project(ChessApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.1 COMPONENTS Graphics Window System REQUIRED)

add_executable(ChessApp
    src/main.cpp
    src/PieceTextures.cpp
    src/MoveValidator.cpp
    src/MoveNotation.cpp
    src/GameState.cpp
    src/SaveManager.cpp
    src/BoardRenderer.cpp
    src/UI.cpp
)

target_link_libraries(ChessApp PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_custom_command(TARGET ChessApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:ChessApp>/assets"
)
