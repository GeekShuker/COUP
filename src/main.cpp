//meirshuker159@gmail.com


#include "Game.hpp"
#include "Player.hpp"
#include "Roles.hpp"
#include "GUI.hpp"
#include <memory>
#include <iostream>

using coup::GUI;
using coup::Game;

/**
 * @brief Main entry point for the Coup card game
 * @return 0 on successful execution, 1 on error
 * @details Creates a new game instance and launches the GUI.
 * All exceptions are caught and reported to stderr.
 * The GUI handles player setup, game logic, and display.
 */
int main() {
    try {
        // Create game instance
        auto game = std::make_shared<coup::Game>();
        
        // Create and run GUI
        GUI gui(game);
        gui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 