//meirshuker159@gmail.com

#include "Game.hpp"
#include "Exceptions.hpp"
#include "Roles.hpp"
#include <algorithm>
#include <random>
#include <iostream>

namespace coup {

Game::Game() : current_turn(0), game_started(false), treasury(50), last_arrested_player(""), 
               actions_remaining(1) {}

void Game::add_player(std::shared_ptr<Player> player) {
    validate_player_count();
    // Players start with 0 coins as per original tests
    this->player_list.push_back(player);
}

void Game::start_game() {
    if (this->player_list.size() < 2) {
        throw GameException("Not enough players to start game");
    }
    game_started = true;
}

/**
 * @brief Advances the game to the next player's turn with comprehensive state management
 * @details This function handles:
 * - Turn-based effect cleanup (sanctions, arrest blocks)
 * - Active player counting and game over detection
 * - Mandatory coup warning for players with 10+ coins
 * - Finding next active player and wraparound
 * - Calling on_turn_start() for role-specific effects
 * - Resetting action counter for new turn
 */
void Game::next_turn() {
    // Don't cleanup inactive players immediately - let GUI show elimination first
    // cleanup_inactive_players();
    
    // Count active players for game over check
    int active_count = 0;
    for (const auto& player : player_list) {
        if (player && player->is_active()) {
            active_count++;
        }
    }
    
    if (active_count <= 1) {
        // Game is ending - cleanup will happen when winner() is called
        return;
    }
    
    // Check for mandatory coup
    auto currentPlayer = get_current_player();
    if (currentPlayer && currentPlayer->get_coins() >= 10) {
        std::cout << "[GAME] ⚠️ " << currentPlayer->get_name() << " (" << currentPlayer->role() 
                  << ") MUST COUP (has " << currentPlayer->get_coins() << " coins)" << std::endl;
    }
    
    // Clear turn-based effects for current player before moving to next
    if (currentPlayer) {
        // Log status effect cleanup
        if (currentPlayer->is_sanctioned()) {
            std::cout << "[CLEANUP] " << currentPlayer->get_name() << " is no longer sanctioned" << std::endl;
        }
        if (currentPlayer->is_arrest_blocked()) {
            std::cout << "[CLEANUP] " << currentPlayer->get_name() << " is no longer arrest blocked" << std::endl;
        }
        currentPlayer->set_sanctioned(false);  // Clear sanctions at end of turn
        currentPlayer->set_arrest_blocked(false); // Clear arrest block at end of turn
    }
    
    // Find next active player
    size_t starting_turn = current_turn;
    do {
        current_turn = (current_turn + 1) % this->player_list.size();
    } while (current_turn != starting_turn && 
             (!player_list[current_turn] || !player_list[current_turn]->is_active()));

    // Call on_turn_start for current player and reset actions
    auto nextPlayer = get_current_player();
    if (nextPlayer && nextPlayer->is_active()) {
        std::cout << "[TURN] === " << nextPlayer->get_name() << " (" << nextPlayer->role() 
                  << ")'s turn begins - " << nextPlayer->get_coins() << " coins ===" << std::endl;
        nextPlayer->on_turn_start();
        start_turn_actions();  // Reset to 1 action for new turn
    }
}

bool Game::is_game_over() const {
    int active_count = 0;
    for (const auto& player : player_list) {
        if (player && player->is_active()) {
            active_count++;
        }
    }
    return active_count <= 1;
}

std::string Game::turn() const {
    if (this->player_list.empty()) {
        throw GameException("No players in game");
    }
    return this->player_list[current_turn]->get_name();
}

std::vector<std::string> Game::players() const {
    std::vector<std::string> player_names;
    for (const auto& player : this->player_list) {
        if (player) {
            player_names.push_back(player->get_name());
        }
    }
    return player_names;
}

std::vector<std::shared_ptr<Player>> Game::all_players() const {
    return this->player_list;
}

std::string Game::winner() const {
    if (!is_game_over()) {
        throw GameException("Game is not over yet");
    }
    
    // Find the last active player
    for (const auto& player : player_list) {
        if (player && player->is_active()) {
            return player->get_name();
        }
    }
    
    throw GameException("No winner - all players eliminated");
}

void Game::add_to_treasury(int amount) {
    if (amount < 0) {
        throw GameException("Cannot add negative amount to treasury");
    }
    treasury += amount;
}

void Game::remove_from_treasury(int amount) {
    if (amount < 0) {
        throw GameException("Cannot remove negative amount from treasury");
    }
    if (treasury < amount) {
        throw GameException("Not enough coins in treasury");
    }
    treasury -= amount;
}

std::shared_ptr<Player> Game::get_current_player() const {
    if (this->player_list.empty()) {
        return nullptr;
    }
    return this->player_list[current_turn];
}

std::shared_ptr<Player> Game::get_player_by_name(const std::string& name) const {
    auto it = std::find_if(this->player_list.begin(), this->player_list.end(),
        [&name](const std::shared_ptr<Player>& p) { return p->get_name() == name; });
    
    if (it == this->player_list.end()) {
        throw PlayerNotFoundException("Player not found: " + name);
    }
    return *it;
}

bool Game::is_player_turn(const Player* player) const {
    if (this->player_list.empty()) {
        return false;
    }
    return this->player_list[current_turn].get() == player;
}

void Game::validate_game_state() const {
    if (!game_started) {
        throw GameException("Game has not started");
    }
}

void Game::validate_player_count() const {
    if (this->player_list.size() >= 6) {
        throw TooManyPlayersException("Maximum 6 players allowed");
    }
}

/**
 * @brief Removes inactive players and adjusts turn index accordingly
 * @details This function:
 * - Counts players being removed before current turn index
 * - Removes all inactive players from the list
 * - Adjusts current_turn to maintain proper turn order
 * - Ensures current_turn never goes below 0
 * - Handles edge case where current player is removed
 */
void Game::cleanup_inactive_players() {
    // Track which players were removed and adjust current_turn accordingly
    size_t removedBefore = 0;
    
    // Count how many players before current_turn are being removed
    for (size_t i = 0; i < current_turn && i < player_list.size(); ++i) {
        if (!player_list[i]->is_active()) {
            removedBefore++;
        }
    }
    
    // Remove inactive players
    this->player_list.erase(
        std::remove_if(this->player_list.begin(), this->player_list.end(),
            [](const std::shared_ptr<Player>& p) { return !p->is_active(); }),
        this->player_list.end()
    );
    
    // Adjust current_turn index to account for removed players
    if (current_turn >= removedBefore) {
        current_turn -= removedBefore;
    }
    
    // If current_turn is now out of bounds, wrap to beginning
    if (current_turn >= player_list.size() && !player_list.empty()) {
        current_turn = 0;
    }
}

/**
 * @brief Creates a player with a random role assignment
 * @param name The name to assign to the new player
 * @return Shared pointer to the newly created player
 * @details Uses std::random_device and std::mt19937 for cryptographically
 * secure random role selection from all 6 available roles:
 * Governor, Spy, Baron, General, Judge, Merchant
 */
std::shared_ptr<Player> Game::create_random_player(const std::string& name) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 5);
    
    int role_index = dis(gen);
    auto game_ptr = shared_from_this();
    
    switch (role_index) {
        case 0: return std::make_shared<Governor>(game_ptr, name);
        case 1: return std::make_shared<Spy>(game_ptr, name);
        case 2: return std::make_shared<Baron>(game_ptr, name);
        case 3: return std::make_shared<General>(game_ptr, name);
        case 4: return std::make_shared<Judge>(game_ptr, name);
        case 5: return std::make_shared<Merchant>(game_ptr, name);
        default: return std::make_shared<Governor>(game_ptr, name);  // Fallback
    }
}

void Game::force_cleanup_inactive_players() {
    cleanup_inactive_players();
}

} // namespace coup 