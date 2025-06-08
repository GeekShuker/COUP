//meirshuker159@gmail.com


#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Exceptions.hpp"
#include "Game.hpp"
#include "Player.hpp"

namespace coup {
    class Game;
    class Player;

    /**
     * @brief Graphical User Interface for the Coup card game
     * @details Provides a complete SFML-based GUI with:
     * - Player information display
     * - Action buttons and target selection
     * - Turn management and game state visualization
     * - Blocking phases and special abilities
     * - Winner/elimination popups
     * - Action history logging
     * - Treasury and game state tracking
     */
    class GUI {
    private:
        static const int WINDOW_WIDTH = 1200; ///< Main window width in pixels
        static const int WINDOW_HEIGHT = 800; ///< Main window height in pixels
        
        sf::RenderWindow window; ///< SFML window for rendering
        std::shared_ptr<Game> game; ///< Reference to the game instance
        
        // GUI elements
        sf::Font font; ///< Font for text rendering
        std::vector<sf::Text> playerTexts; ///< Text objects for player information
        std::vector<sf::Text> buttonTexts; ///< Text objects for button labels
        sf::Text turnText; ///< Text showing current player's turn
        sf::Text treasuryText; ///< Text showing treasury amount
        sf::Text inputText; ///< Text for user input display
        sf::Text promptText; ///< Text for prompts and instructions
        sf::Text historyTitle; ///< Title for action history section
        std::vector<sf::Text> historyTexts; ///< Text objects for action history
        std::vector<sf::RectangleShape> actionButtons; ///< Button shapes for actions
        std::vector<std::string> actionNames; ///< Names of available actions

        // Game state
        bool isSetupPhase; ///< Whether in initial setup phase
        bool isSelectingTarget; ///< Whether waiting for target selection
        std::string currentInput; ///< Current user input string
        std::string pendingAction; ///< Action waiting for target selection
        std::string errorMessage; ///< Current error message to display
        sf::Clock errorMessageTimer; ///< Timer for error message display
        
        // Action history for CLI logging
        std::vector<std::string> actionHistory; ///< Log of all game actions
        
        // Bribe state - tracks remaining actions after bribe
        int remainingBribeActions; ///< Extra actions remaining from bribe
        std::string bribePlayerName; ///< Player currently in bribe mode
        
        // Block phase state
        std::vector<std::shared_ptr<Player>> blockers; ///< Players who can block current action
        std::shared_ptr<Player> blockingActor; ///< Player performing blockable action
        std::shared_ptr<Player> blockingTarget; ///< Target of blockable action
        bool isBlockPhase; ///< Whether in blocking phase
        std::string blockingAction; ///< Action that can be blocked

        // Winner and elimination popups
        bool showWinnerPopup; ///< Whether to show winner popup
        bool showEliminationPopup; ///< Whether to show elimination popup
        std::string winnerName; ///< Name of game winner
        std::string eliminatedPlayerName; ///< Name of eliminated player
        sf::Clock popupTimer; ///< Timer for popup display duration

        /**
         * @brief Initializes the SFML window with proper settings
         */
        void initializeWindow();
        
        /**
         * @brief Loads fonts and other assets required for GUI
         * @return true if all assets loaded successfully
         */
        bool loadAssets();
        
        /**
         * @brief Creates action buttons with proper layout
         */
        void createButtons();
        
        /**
         * @brief Updates player information display
         */
        void updatePlayerInfo();
        
        /**
         * @brief Handles mouse clicks on buttons and UI elements
         * @param mousePos Mouse position when clicked
         */
        void handleClick(const sf::Vector2i& mousePos);
        
        /**
         * @brief Processes SFML events (input, window events)
         */
        void handleEvents();
        
        /**
         * @brief Logs an action to the history display
         * @param action Description of the action to log
         */
        void logAction(const std::string& action);

        // Action handling
        
        /**
         * @brief Initiates blocking phase for blockable actions
         * @param action Name of the action that can be blocked
         * @param actor Player performing the action
         * @param target Target of the action (if applicable)
         */
        void startBlockPhase(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target);
        
        /**
         * @brief Handles when a player chooses to block an action
         * @param blocker Player who is blocking
         */
        void onBlockClicked(std::shared_ptr<Player> blocker);
        
        /**
         * @brief Handles when players choose not to block
         */
        void onNoBlockClicked();
        
        /**
         * @brief Executes a game action with proper validation
         * @param action Name of the action to perform
         * @param actor Player performing the action
         * @param target Target player (if action requires one)
         */
        void performAction(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target);
        
        /**
         * @brief Logs current coin counts for all players
         */
        void logAllPlayersCoins();

        /**
         * @brief Checks if game is over and shows winner popup
         */
        void checkForWinner();

        // Enhanced logging methods
        
        /**
         * @brief Logs the start of a player's turn
         * @param playerName Name of the player
         * @param role Role of the player
         * @param coins Current coin count
         */
        void logTurnStart(const std::string& playerName, const std::string& role, int coins);
        
        /**
         * @brief Logs status effect changes (sanctions, blocks)
         * @param playerName Name of affected player
         * @param effect Type of status effect
         * @param applied Whether effect was applied or removed
         */
        void logStatusEffect(const std::string& playerName, const std::string& effect, bool applied);
        
        /**
         * @brief Logs changes to treasury amount
         * @param oldAmount Previous treasury amount
         * @param newAmount New treasury amount
         * @param reason Reason for the change
         */
        void logTreasuryChange(int oldAmount, int newAmount, const std::string& reason);
        
        /**
         * @brief Logs usage of role-specific abilities
         * @param playerName Name of player using ability
         * @param role Role of the player
         * @param ability Name of the ability used
         */
        void logRoleAbility(const std::string& playerName, const std::string& role, const std::string& ability);
        
        /**
         * @brief Logs spy investigation results
         * @param spy Name of investigating spy
         * @param target Name of investigated player
         * @param targetCoins Target's coin count
         */
        void logInvestigation(const std::string& spy, const std::string& target, int targetCoins);
        
        /**
         * @brief Logs mandatory coup warning
         * @param playerName Name of player who must coup
         */
        void logMandatoryCoup(const std::string& playerName);
        
        /**
         * @brief Logs blocking attempts
         * @param blocker Name of blocking player
         * @param action Action being blocked
         * @param actor Name of player whose action is blocked
         */
        void logBlockAttempt(const std::string& blocker, const std::string& action, const std::string& actor);
        
        /**
         * @brief Logs automatic win conditions
         * @param playerName Name of winning player
         */
        void logAutoWin(const std::string& playerName);

    public:
        /**
         * @brief Constructs GUI with game instance
         * @param game Shared pointer to the game to display
         */
        GUI(std::shared_ptr<Game> game);
        
        /**
         * @brief Main GUI loop - runs until window closed
         * @details Handles events, updates game state, and renders display
         */
        void run();
        
        /**
         * @brief Updates GUI state and game logic
         */
        void update();
        
        /**
         * @brief Renders the main game interface
         */
        void render();
        
        /**
         * @brief Renders the action history panel
         */
        void renderHistory();
        
        /**
         * @brief Renders treasury information
         */
        void renderTreasury();
        
        /**
         * @brief Renders winner/elimination popups
         */
        void renderPopups();
    };
}
