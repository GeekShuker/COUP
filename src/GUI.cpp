//meirshuker159@gmail.com


#include "GUI.hpp"
#include "Game.hpp"
#include "Roles.hpp"
#include "Exceptions.hpp"
#include "ActionValidator.hpp"
#include <iostream>
#include <random>
#include <algorithm>

using namespace coup;

/**
 * @brief Constructs GUI with comprehensive initialization
 * @details Initializes all member variables, creates SFML window, loads assets,
 * and sets up action buttons. Uses exception handling to ensure robust startup.
 * Starts in setup phase where players can be added before game begins.
 */
GUI::GUI(std::shared_ptr<Game> game) : 
    window(),
    game(game),
    font(),
    playerTexts(),
    buttonTexts(),
    turnText(),
    treasuryText(),
    inputText(),
    promptText(),
    actionButtons(),
    actionNames(),
    isSetupPhase(true),
    isSelectingTarget(false),
    currentInput(""),
    pendingAction(""),
    errorMessage(""),
    errorMessageTimer(),
    blockers(),
    blockingActor(nullptr),
    blockingTarget(nullptr),
    isBlockPhase(false),
    blockingAction(""),
    showWinnerPopup(false),
    showEliminationPopup(false),
    winnerName(""),
    eliminatedPlayerName(""),
    popupTimer() {
    try {
        initializeWindow();
        if (!loadAssets()) {
            throw std::runtime_error("Failed to load required assets");
        }
        createButtons();
    } catch (const std::exception& e) {
        std::cerr << "Error initializing GUI: " << e.what() << std::endl;
        throw;
    }
}

/**
 * @brief Initializes SFML window with game-appropriate settings
 * @details Creates window with fixed dimensions, sets frame rate limit for
 * smooth performance, and prepares for game rendering.
 */
void GUI::initializeWindow() {
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Coup Game");
    window.setFramerateLimit(60);
}

/**
 * @brief Loads fonts and other assets with fallback system
 * @details Attempts to load fonts from multiple paths (local assets, system fonts)
 * for cross-platform compatibility. Initializes all text objects with proper
 * positioning and styling. Returns false if no fonts can be loaded.
 */
bool GUI::loadAssets() {
    const std::vector<std::string> fontPaths = {
        "assets/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    };

    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            std::cout << "Successfully loaded font from: " << path << std::endl;
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "Failed to load any font. Tried:\n";
        for (const auto& path : fontPaths) {
            std::cerr << "  - " << path << "\n";
        }
        return false;
    }

    turnText.setFont(font);
    turnText.setCharacterSize(30);
    turnText.setFillColor(sf::Color::White);
    turnText.setPosition(350, 10);

    treasuryText.setFont(font);
    treasuryText.setCharacterSize(24);
    treasuryText.setFillColor(sf::Color::Yellow);
    treasuryText.setPosition(10, 10);

    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);

    promptText.setFont(font);
    promptText.setString("Enter player name (press Enter to add):");
    promptText.setCharacterSize(24);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 50);

    return true;
}

/**
 * @brief Creates action buttons dynamically based on current player's role
 * @details Generates role-specific button sets (Spy gets Investigate/Block Arrest,
 * Baron gets Invest). Uses ActionValidator to determine button availability and
 * applies appropriate visual styling (enabled/disabled colors). Centers buttons
 * horizontally at bottom of screen.
 */
void GUI::createButtons() {
    std::vector<std::string> baseActions = {"Gather", "Tax", "Bribe", "Arrest", "Sanction", "Coup", "End Turn"};
    auto currentPlayer = game->get_current_player();
    if (currentPlayer && currentPlayer->role() == "Spy") {
        actionNames = {"Gather", "Tax", "Bribe", "Arrest", "Sanction", "Coup", "Investigate", "Block Arrest", "End Turn"};
    } else if (currentPlayer && currentPlayer->role() == "Baron") {
        actionNames = {"Gather", "Tax", "Bribe", "Arrest", "Sanction", "Coup", "Invest", "End Turn"};
    } else {
        actionNames = baseActions;
    }
    float buttonWidth = 100.f;
    float buttonHeight = 50.f;
    float spacing = 20.f;
    float startY = WINDOW_HEIGHT - buttonHeight - spacing;
    actionButtons.clear();
    buttonTexts.clear();
    float startX = (WINDOW_WIDTH - (actionNames.size() * (buttonWidth + spacing) - spacing)) / 2;
    for (size_t i = 0; i < actionNames.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(startX + i * (buttonWidth + spacing), startY);
        sf::Color buttonColor = sf::Color(100, 100, 100);
        bool isActionAvailable = true;
        if (currentPlayer) {
            const std::string& action = actionNames[i];
            // Use button-specific validation logic (doesn't check targets)
            isActionAvailable = ActionValidator::isActionAvailableForButton(action, currentPlayer);
            if (!isActionAvailable) {
                buttonColor = sf::Color(70, 70, 70);
            }
        }
        button.setFillColor(buttonColor);
        button.setOutlineThickness(2.f);
        button.setOutlineColor(sf::Color::White);
        actionButtons.push_back(button);
        sf::Text text;
        text.setFont(font);
        text.setString(actionNames[i]);
        text.setCharacterSize(20);
        text.setFillColor(isActionAvailable ? sf::Color::White : sf::Color(150, 150, 150));
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.left + textBounds.width/2.0f,
                      textBounds.top + textBounds.height/2.0f);
        text.setPosition(
            button.getPosition().x + buttonWidth/2.0f,
            button.getPosition().y + buttonHeight/2.0f
        );
        buttonTexts.push_back(text);
    }
}

/**
 * @brief Main GUI loop that runs until window is closed
 * @details Continuously processes events, updates game state, and renders
 * the display at 60 FPS. This is the core game loop that keeps the interface
 * responsive and the game running smoothly.
 */
void GUI::run() {
    while (window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void GUI::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape && showWinnerPopup) {
                window.close();
                return;
            }
            else if (event.key.code == sf::Keyboard::Space && isSetupPhase) {
                try {
                    size_t playerCount = game->players().size();
                    if (playerCount < 2) {
                        throw GameException("Need at least 2 players to start the game");
                    }
                    isSetupPhase = false;
                    game->start_game();
                } catch (const std::exception& e) {
                    errorMessage = e.what();
                    errorMessageTimer.restart();
                }
            }
        }
        else if (event.type == sf::Event::TextEntered && isSetupPhase) {
            if (event.text.unicode < 128) {
                if (event.text.unicode == '\b' && !currentInput.empty()) {
                    currentInput.pop_back();
                }
                else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    try {
                        if (currentInput.empty()) {
                            throw GameException("Player name cannot be empty");
                        }
                        std::vector<std::string> existingPlayers = game->players();
                        if (std::find(existingPlayers.begin(), existingPlayers.end(), currentInput) != existingPlayers.end()) {
                            throw GameException("Player name already exists");
                        }
                        auto player = game->create_random_player(currentInput);
                        game->add_player(player);
                        currentInput.clear();
                        size_t playerCount = game->players().size();
                        if (playerCount < 2) {
                            promptText.setString("Need at least " + std::to_string(2 - playerCount) + 
                                               " more players to start. Enter player name:");
                        } else if (playerCount >= 6) {
                            isSetupPhase = false;
                            game->start_game();
                        } else {
                            promptText.setString("Press Space to start game or enter more names (max 6)");
                        }
                    } catch (const std::exception& e) {
                        errorMessage = e.what();
                        errorMessageTimer.restart();
                    }
                }
                else if (event.text.unicode != '\b') {
                    currentInput += static_cast<char>(event.text.unicode);
                }
                inputText.setString(currentInput);
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed && !isSetupPhase && !showWinnerPopup) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                handleClick(sf::Mouse::getPosition(window));
            }
        }
    }
}

void GUI::update() {
    updatePlayerInfo();
    static std::string lastPlayerName = "";
    auto currentPlayer = game->get_current_player();
    std::string currentPlayerName = currentPlayer ? currentPlayer->get_name() : "";
    if (currentPlayerName != lastPlayerName) {
        createButtons();
        lastPlayerName = currentPlayerName;
    }
    checkForWinner();
}

void GUI::updatePlayerInfo() {
    playerTexts.clear();
    float startY = 50.f;
    float spacing = 30.f;
    if (!isSetupPhase) {
        try {
            auto allPlayers = game->all_players();
            auto currentPlayer = game->get_current_player();
            std::string currentPlayerName = currentPlayer ? currentPlayer->get_name() : "";
            for (size_t i = 0; i < allPlayers.size(); ++i) {
                auto player = allPlayers[i];
                if (!player) continue;
                std::string playerInfo = player->get_name();
                if (currentPlayer && player->get_name() == currentPlayer->get_name()) {
                    playerInfo += " [" + player->role() + "]";
                    playerInfo += " (" + std::to_string(player->get_coins()) + " coins)";
                } else {
                    playerInfo += " [Hidden]";
                }
                if (player->is_sanctioned()) {
                    playerInfo += " [SANCTIONED]";
                }
                if (player->is_arrest_blocked()) {
                    playerInfo += " [ARREST BLOCKED]";
                }
                sf::Text playerText;
                playerText.setFont(font);
                playerText.setString(playerInfo);
                playerText.setCharacterSize(20);
                if (!player->is_active()) {
                    playerText.setFillColor(sf::Color(100, 100, 100));
                    playerText.setString(player->get_name() + " [ELIMINATED]");
                } else if (game->turn() == player->get_name()) {
                    playerText.setFillColor(sf::Color::Yellow);
                } else {
                    playerText.setFillColor(sf::Color::White);
                }
                playerText.setPosition(10, startY + i * spacing);
                playerTexts.push_back(playerText);
            }
            if (currentPlayer) {
                turnText.setString("Current Turn: " + currentPlayer->get_name() + 
                                 " [" + currentPlayer->role() + "]" +
                                 " (" + std::to_string(currentPlayer->get_coins()) + " coins)");
            }
        } catch (const std::exception& e) {
            std::cerr << "Error updating player info: " << e.what() << std::endl;
        }
    }
}

void GUI::render() {
    window.clear(sf::Color(50, 50, 50));
    if (isSetupPhase) {
        window.draw(promptText);
        window.draw(inputText);
        float startY = 50.f;
        float spacing = 30.f;
        std::vector<std::string> playerNames = game->players();
        for (size_t i = 0; i < playerNames.size(); ++i) {
            sf::Text playerText;
            playerText.setFont(font);
            playerText.setString(playerNames[i]);
            playerText.setCharacterSize(20);
            playerText.setFillColor(sf::Color::White);
            playerText.setPosition(10, startY + i * spacing);
            window.draw(playerText);
        }
    } else {
        window.draw(turnText);
        for (const auto& text : playerTexts) {
            window.draw(text);
        }
        for (size_t i = 0; i < actionButtons.size(); ++i) {
            window.draw(actionButtons[i]);
            window.draw(buttonTexts[i]);
        }
        if (isSelectingTarget) {
            window.draw(promptText);
        }
        if (isBlockPhase) {
            float buttonWidth = 120.f, buttonHeight = 40.f, spacing = 10.f;
            float y = 120.f;
            for (size_t i = 0; i < blockers.size(); ++i) {
                sf::RectangleShape blockBtn(sf::Vector2f(buttonWidth, buttonHeight));
                blockBtn.setPosition(250, y + i * (buttonHeight + spacing));
                blockBtn.setFillColor(sf::Color(160, 40, 40));
                window.draw(blockBtn);
                sf::Text btnText;
                btnText.setFont(font);
                btnText.setString("Block: " + blockers[i]->get_name());
                btnText.setCharacterSize(18);
                btnText.setFillColor(sf::Color::White);
                btnText.setPosition(255, y + i * (buttonHeight + spacing) + 8);
                window.draw(btnText);
            }
            sf::RectangleShape continueBtn(sf::Vector2f(buttonWidth, buttonHeight));
            continueBtn.setPosition(250, y + blockers.size() * (buttonHeight + spacing) + 20);
            continueBtn.setFillColor(sf::Color(40, 160, 40));
            window.draw(continueBtn);
            sf::Text contText;
            contText.setFont(font);
            contText.setString("Continue");
            contText.setCharacterSize(18);
            contText.setFillColor(sf::Color::White);
            contText.setPosition(270, y + blockers.size() * (buttonHeight + spacing) + 28);
            window.draw(contText);
        }
        renderTreasury();
    }
    if (!errorMessage.empty() && errorMessageTimer.getElapsedTime().asSeconds() < 3.0f) {
        sf::Text errorText;
        errorText.setFont(font);
        errorText.setString(errorMessage);
        errorText.setCharacterSize(24);
        errorText.setFillColor(sf::Color::Red);
        sf::FloatRect textBounds = errorText.getLocalBounds();
        float x = (WINDOW_WIDTH - textBounds.width) / 2;
        float y = WINDOW_HEIGHT - 150;
        errorText.setPosition(x, y);
        window.draw(errorText);
    } else if (!errorMessage.empty()) {
        errorMessage = "";
    }
    renderPopups();
    window.display();
}

void GUI::renderTreasury() {
    treasuryText.setString("Treasury: " + std::to_string(game->get_treasury()) + " coins");
    window.draw(treasuryText);
}

void GUI::renderPopups() {
    if (showWinnerPopup) {
        sf::RectangleShape popup(sf::Vector2f(400, 200));
        popup.setPosition(WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2 - 100);
        popup.setFillColor(sf::Color(0, 100, 0, 200));
        popup.setOutlineThickness(3);
        popup.setOutlineColor(sf::Color::Green);
        window.draw(popup);
        sf::Text winnerText;
        winnerText.setFont(font);
        winnerText.setString("GAME OVER!\n\nWinner: " + winnerName + "\n\nPress ESC to close");
        winnerText.setCharacterSize(24);
        winnerText.setFillColor(sf::Color::White);
        sf::FloatRect textBounds = winnerText.getLocalBounds();
        winnerText.setOrigin(textBounds.left + textBounds.width/2.0f,
                           textBounds.top + textBounds.height/2.0f);
        winnerText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        window.draw(winnerText);
    }
    if (showEliminationPopup && popupTimer.getElapsedTime().asSeconds() < 3.0f) {
        // Popup dimensions and position
        float popupWidth = 300.0f;
        float popupHeight = 150.0f;
        float popupX = WINDOW_WIDTH/2 - popupWidth/2;
        float popupY = 200.0f;
        
        sf::RectangleShape popup(sf::Vector2f(popupWidth, popupHeight));
        popup.setPosition(popupX, popupY);
        popup.setFillColor(sf::Color(100, 0, 0, 200));
        popup.setOutlineThickness(3);
        popup.setOutlineColor(sf::Color::Red);
        window.draw(popup);
        
        sf::Text eliminatedText;
        eliminatedText.setFont(font);
        eliminatedText.setString("ELIMINATED!\n\n" + eliminatedPlayerName + "\nhas been eliminated!");
        eliminatedText.setCharacterSize(20);
        eliminatedText.setFillColor(sf::Color::White);
        sf::FloatRect textBounds = eliminatedText.getLocalBounds();
        eliminatedText.setOrigin(textBounds.left + textBounds.width/2.0f,
                               textBounds.top + textBounds.height/2.0f);
        // Position text in the center of the popup box, not the center of the window
        eliminatedText.setPosition(popupX + popupWidth/2, popupY + popupHeight/2);
        window.draw(eliminatedText);
    } else if (showEliminationPopup) {
        showEliminationPopup = false;
        // Keep eliminated players in the display - don't clean them up
    }
}

void GUI::handleClick(const sf::Vector2i& mousePos) {
    // ==========================================
    // PRIORITY 1: BLOCK PHASE HANDLER
    // ==========================================
    // When an action is being blocked, only blocking-related clicks are processed
    // This has the highest priority to prevent other actions during blocking
    if (isBlockPhase) {
        float buttonWidth = 120.f, buttonHeight = 40.f, spacing = 10.f;
        float y = 120.f;
        
        // Check if user clicked on any "Block" button (one for each potential blocker)
        for (size_t i = 0; i < blockers.size(); ++i) {
            sf::FloatRect btnRect(250, y + i * (buttonHeight + spacing), buttonWidth, buttonHeight);
            if (btnRect.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                onBlockClicked(blockers[i]); // Execute the block with this player
                return; // Exit immediately - no other actions allowed
            }
        }
        
        // Check if user clicked "Continue" button (proceed without blocking)
        sf::FloatRect continueRect(250, y + blockers.size() * (buttonHeight + spacing) + 20, buttonWidth, buttonHeight);
        if (continueRect.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            onNoBlockClicked(); // Execute the original action without blocking
            return;
        }
        return; // Block all other clicks during block phase
    }

    // ==========================================
    // GUARDS: PREVENT INVALID STATES
    // ==========================================
    // Don't process clicks during setup phase or when no current player exists
    if (isSetupPhase) return;
    auto currentPlayer = game->get_current_player();
    if (!currentPlayer) return;
    
    // ==========================================
    // PRIORITY 2: TARGET SELECTION HANDLER
    // ==========================================
    // When an action requiring a target has been chosen, handle player selection
    if (isSelectingTarget) {
        float startY = 50.f;
        float spacing = 30.f;
        float playerHeight = 30.f;
        std::vector<std::string> playerNames = game->players();
        
        // Check if user clicked on any player in the list
        for (size_t i = 0; i < playerNames.size(); ++i) {
            sf::FloatRect playerBounds(10.f, startY + i * spacing, 200.f, playerHeight);
            if (playerBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                try {
                    auto targetPlayer = game->get_player_by_name(playerNames[i]);
                    if (targetPlayer) {
                        // === ACTIONS THAT TRIGGER BLOCKING PHASE ===
                        // These actions can be blocked by other players, so start the blocking process
                        if (pendingAction == "Arrest" || pendingAction == "Sanction" || pendingAction == "Coup" ||
                         pendingAction == "Tax" || pendingAction == "Bribe") {
                            // Validate with target before starting block phase
                            ActionValidator::validateActionExecution(pendingAction, currentPlayer, targetPlayer);
                            startBlockPhase(pendingAction, currentPlayer, targetPlayer);
                            isSelectingTarget = false;
                            pendingAction = "";
                            return;
                        }  
                
                        // === SPY-SPECIFIC ACTIONS (NO BLOCKING) ===
                        // These are immediate actions that cannot be blocked
                        if (pendingAction == "Investigate") {
                            auto spy = std::dynamic_pointer_cast<Spy>(currentPlayer);
                            spy->investigate(*targetPlayer);
                            std::string coinsInfo = targetPlayer->get_name() + " has " + std::to_string(targetPlayer->get_coins()) + " coins";
                            errorMessage = coinsInfo; // Display result to current player
                            errorMessageTimer.restart();
                            std::cout << "[ACTION LOG] " << currentPlayer->get_name() + " (" + currentPlayer->role() + ") investigated " << targetPlayer->get_name() + " and saw " << std::to_string(targetPlayer->get_coins()) << " coins" << std::endl;
                        }
                        else if (pendingAction == "Block Arrest") {
                            auto spy = std::dynamic_pointer_cast<Spy>(currentPlayer);
                            if (spy) {
                                spy->block_arrest_ability(*targetPlayer);
                                errorMessage = targetPlayer->get_name() + " is blocked from using arrest this turn!";
                                errorMessageTimer.restart();
                                std::cout << "[ACTION LOG] " << currentPlayer->get_name() + " (" + currentPlayer->role() + ") blocked " << targetPlayer->get_name() << "'s arrest ability" << std::endl;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    errorMessage = e.what();
                    errorMessageTimer.restart();
                }
                // Reset target selection state after processing
                isSelectingTarget = false;
                pendingAction = "";
                return;
            }
        }
        // If clicked outside player list, cancel target selection
        isSelectingTarget = false;
        pendingAction = "";
        return;
    }
    
    // ==========================================
    // PRIORITY 3: ACTION BUTTON HANDLER
    // ==========================================
    // Handle clicks on the main action buttons (bottom of screen)
    for (size_t i = 0; i < actionButtons.size(); ++i) {
        if (actionButtons[i].getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                        static_cast<float>(mousePos.y))) {
            try {
                const std::string& action = actionNames[i];
                
                // === VALIDATION: CONDITIONAL BASED ON ACTION TYPE ===
                if (!ActionValidator::requiresTarget(action)) {
                    // Full validation for actions that don't need targets
                    ActionValidator::validateActionExecution(action, currentPlayer);
                } else {
                    // Basic validation only for target-requiring actions (no target validation)
                    if (!ActionValidator::isActionAvailableForButton(action, currentPlayer)) {
                        throw IllegalMoveException("Action not available");
                    }
                }
                
                // === IMMEDIATE ACTIONS (ROUTE THROUGH PERFORMACTION) ===
                if (action == "Gather") {
                    // Validation already done above - route through performAction
                    performAction("Gather", currentPlayer, nullptr);
                    return;
                    
                // === ACTIONS THAT TRIGGER BLOCKING (NO TARGET) ===
                } else if (action == "Tax") {
                    // Validation already done above - start blocking phase
                    startBlockPhase("Tax", currentPlayer, nullptr);
                    return;
                } else if (action == "Bribe") {
                    startBlockPhase("Bribe", currentPlayer, nullptr);
                    return;
                    
                // === ACTIONS THAT REQUIRE TARGET SELECTION ===
                } else if (action == "Arrest" || action == "Sanction" || action == "Coup") {
                    // Switch to target selection mode
                    isSelectingTarget = true;
                    pendingAction = action;
                    promptText.setString("Select a target player");
                    return;
                } else if (action == "Investigate") {
                    // Spy ability: investigate another player's coins
                    isSelectingTarget = true;
                    pendingAction = action;
                    promptText.setString("Select a player to investigate");
                    return;
                } else if (action == "Block Arrest") {
                    // Spy ability: prevent another player from using arrest
                    isSelectingTarget = true;
                    pendingAction = action;
                    promptText.setString("Select a player to block their arrest ability");
                    return;
                    
                // === ROLE-SPECIFIC ACTIONS ===
                } else if (action == "Invest") {
                    if (currentPlayer->role() == "Baron") {
                        // Route through performAction for consistency
                        performAction("Invest", currentPlayer, nullptr);
                        return;
                    } else if (currentPlayer->role() == "Spy") {
                        // Note: This seems like legacy code - Spy doesn't use "Invest" button
                        isSelectingTarget = true;
                        pendingAction = "SpyMenu";
                        promptText.setString("Spy Abilities: Click player to Investigate OR right-click to Block Arrest");
                        return;
                    }
                    
                // === TURN MANAGEMENT ===
                } else if (action == "End Turn") {
                    // Force end turn regardless of remaining actions
                    std::cout << "[ACTION LOG] " << currentPlayer->get_name() + " (" + currentPlayer->role() + ") ended turn" << std::endl;
                    game->next_turn();
                    return;
                }
            } catch (const std::exception& e) {
                // Display any errors to the player (insufficient coins, invalid moves, etc.)
                errorMessage = e.what();
                errorMessageTimer.restart();
            }
            break; // Only process one button click per call
        }
    }
}

/**
 * @brief Initiates blocking phase for blockable actions
 * @details Identifies all active players who can block the current action,
 * sets up blocking state variables, and either proceeds immediately (if no
 * blockers exist) or enters blocking phase for player decisions.
 */
void GUI::startBlockPhase(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target) {
    blockers.clear();
    isBlockPhase = true;
    this->blockingAction = action;
    this->blockingActor = actor;
    this->blockingTarget = target;
    auto allPlayers = game->all_players();
    for (const auto& player : allPlayers) {
        if (!player || player == actor || !player->is_active()) continue;  // Skip eliminated players
        if (player->can_block(action)) {
            blockers.push_back(player);
        }
    }
    if (blockers.empty()) {
        performAction(action, actor, target);
        isBlockPhase = false;
    }
}

void GUI::onBlockClicked(std::shared_ptr<Player> blocker) {
    std::cout << "[ACTION LOG] " << blocker->get_name() + " (" + blocker->role() + ") blocked " << this->blockingAction + " from " << this->blockingActor->get_name() + " (" + this->blockingActor->role() + ")" << std::endl;
    try {
        if (this->blockingAction == "Bribe") {
            this->blockingActor->remove_coins(4);
            game->add_to_treasury(4);
            std::cout << "[ACTION LOG] " << this->blockingActor->get_name() + " (" + this->blockingActor->role() + ") lost 4 coins from blocked Bribe (returned to treasury)" << std::endl;
        } else if (this->blockingAction == "Sanction") {
            this->blockingActor->remove_coins(3);
            game->add_to_treasury(3);
            std::cout << "[ACTION LOG] " << this->blockingActor->get_name() + " (" + this->blockingActor->role() + ") lost 3 coins from blocked Sanction (returned to treasury)" << std::endl;
        } else if (this->blockingAction == "Coup") {
            this->blockingActor->remove_coins(7);
            game->add_to_treasury(7);
            std::cout << "[ACTION LOG] " << this->blockingActor->get_name() + " (" + this->blockingActor->role() + ") lost 7 coins from blocked Coup (returned to treasury)" << std::endl;
            if (blocker->role() == "General") {
                blocker->remove_coins(5);
                game->add_to_treasury(5);
                std::cout << "[ACTION LOG] " << blocker->get_name() + " (" + blocker->role() + ") paid 5 coins to treasury to block coup" << std::endl;
            }
        }
    } catch (const std::exception& e) {
         std::cerr << "ERROR in blocking: " + std::string(e.what()) << std::endl;
    }
    errorMessage = blocker->get_name() + " (" + blocker->role() + ") blocked " + this->blockingAction + "!";
    errorMessageTimer.restart();
    isBlockPhase = false;
    
    game->next_turn();
    pendingAction = "";
}

void GUI::onNoBlockClicked() {
    if (!isBlockPhase) return;
    isBlockPhase = false;
    performAction(this->blockingAction, this->blockingActor, this->blockingTarget);
}

/**
 * @brief Executes validated game actions by calling appropriate player methods
 * @details Routes actions to proper Player class methods, handles role-specific
 * actions (Baron's invest), manages special feedback (bribe extra actions),
 * triggers elimination popups (coup), and logs coin summaries. Uses exception
 * handling to display errors to players.
 */
void GUI::performAction(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target) {
    try {
        // Call the actual Player methods instead of duplicating logic
        if (action == "Gather") {
            actor->gather();
        } else if (action == "Tax") {
            actor->tax();
        } else if (action == "Bribe") {
            actor->bribe();
            errorMessage = actor->get_name() + " used Bribe! Choose " + std::to_string(game->get_actions_remaining()) + " more actions (or End Turn).";
            errorMessageTimer.restart();
        } else if (action == "Invest") {
            // Cast to Baron and call invest method
            auto baron = std::dynamic_pointer_cast<Baron>(actor);
            if (baron) {
                baron->invest();
            } else {
                throw IllegalMoveException("Only Baron can invest");
            }
        } else if (action == "Arrest" && target) {
            actor->arrest(*target);
        } else if (action == "Sanction" && target) {
            actor->sanction(*target);
        } else if (action == "Coup" && target) {
            actor->coup(*target);
            eliminatedPlayerName = target->get_name();
            showEliminationPopup = true;
            popupTimer.restart();
        }
        
        // Print coin summary after action
        std::string coinSummaryForCLI = "COINS: ";
        std::vector<std::string> cliPlayerNames = game->players();
        for (size_t k = 0; k < cliPlayerNames.size(); ++k) {
            auto player = game->get_player_by_name(cliPlayerNames[k]);
            if (player) {
                coinSummaryForCLI += cliPlayerNames[k] + "(" + std::to_string(player->get_coins()) + ")";
                if (k < cliPlayerNames.size() - 1) coinSummaryForCLI += ", ";
            }
        }
        std::cout << "[ACTION LOG] " << coinSummaryForCLI << std::endl;
    } catch (const std::exception& e) {
        errorMessage = e.what();
        errorMessageTimer.restart();
        std::cerr << "ERROR performing action: " + std::string(e.what()) << std::endl;
    }
}

void GUI::checkForWinner() {
    if (!isSetupPhase && game->is_active() && game->is_game_over() && !showWinnerPopup) {
        try {
            winnerName = game->winner();
            showWinnerPopup = true;
            std::cout << "GAME OVER! Winner: " << winnerName << std::endl;
        } catch (const std::exception&) {
        }
    }
}


