//meirshuker159@gmail.com


#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "Player.hpp"
#include "Roles.hpp"

namespace coup {

/**
 * @brief Main game controller class for the Coup card game
 * @details Manages players, turns, treasury, and game state. Supports 2-6 players
 * with action management system including extra actions from bribe ability.
 */
class Game : public std::enable_shared_from_this<Game> {
private:
    std::vector<std::shared_ptr<Player>> player_list; ///< All players in the game (active and inactive)
    size_t current_turn; ///< Index of current player's turn
    bool game_started; ///< Whether the game has been started
    int treasury; ///< Current coins in the treasury
    std::string last_arrested_player; ///< Global arrest restriction tracking
    
    // Simple action counter for bribe system
    int actions_remaining; ///< Number of actions remaining for current player

public:
    /**
     * @brief Constructs a new Game instance
     * @details Initializes treasury with 50 coins, no players, game not started
     */
    Game();
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Game() = default;

    // Game management
    
    /**
     * @brief Adds a player to the game
     * @param player Shared pointer to the player to add
     * @throws TooManyPlayersException if already at maximum 6 players
     * @throws GameException if game has already started
     */
    void add_player(std::shared_ptr<Player> player);
    
    /**
     * @brief Starts the game
     * @throws GameException if less than 2 players or more than 6 players
     * @details Sets first player's turn and marks game as started
     */
    void start_game();
    
    /**
     * @brief Advances to the next player's turn
     * @details Skips inactive players, clears turn-based effects, calls on_turn_start()
     */
    void next_turn();
    
    /**
     * @brief Checks if the game is over
     * @return true if 1 or fewer active players remain
     */
    bool is_game_over() const;
    
    // Required methods - exact names from specification
    
    /**
     * @brief Gets the name of the current player whose turn it is
     * @return String name of current player
     * @throws GameException if no players in game
     */
    std::string turn() const;
    
    /**
     * @brief Gets list of all player names (active only)
     * @return Vector of player name strings
     */
    std::vector<std::string> players() const;
    
    /**
     * @brief Gets all players including inactive ones for GUI display
     * @return Vector of shared pointers to all players
     */
    std::vector<std::shared_ptr<Player>> all_players() const;
    
    /**
     * @brief Gets the winner of the game
     * @return String name of the winning player
     * @throws GameException if game is not over yet or no winner found
     */
    std::string winner() const;

    // Game state methods
    
    /**
     * @brief Checks if the game is active (started)
     * @return true if game has been started
     */
    bool is_active() const { return game_started; }
    
    /**
     * @brief Gets the total number of players (active and inactive)
     * @return Number of players in the game
     */
    size_t player_count() const { return player_list.size(); }
    
    /**
     * @brief Gets the current treasury amount
     * @return Number of coins in treasury
     */
    int get_treasury() const { return treasury; }
    
    /**
     * @brief Adds coins to the treasury
     * @param amount Number of coins to add (must be positive)
     * @throws GameException if amount is negative
     */
    void add_to_treasury(int amount);
    
    /**
     * @brief Removes coins from the treasury
     * @param amount Number of coins to remove (must be positive)
     * @throws GameException if amount is negative or exceeds treasury
     */
    void remove_from_treasury(int amount);
    
    // Player management
    
    /**
     * @brief Gets the current player whose turn it is
     * @return Shared pointer to current player, or nullptr if no players
     */
    std::shared_ptr<Player> get_current_player() const;
    
    /**
     * @brief Finds a player by their name
     * @param name Name of the player to find
     * @return Shared pointer to the player
     * @throws PlayerNotFoundException if player not found
     */
    std::shared_ptr<Player> get_player_by_name(const std::string& name) const;
    
    /**
     * @brief Checks if it's a specific player's turn
     * @param player Pointer to the player to check
     * @return true if it's the player's turn
     */
    bool is_player_turn(const Player* player) const;

    // Global arrest restriction methods
    
    /**
     * @brief Gets the name of the last arrested player
     * @return Name of last arrested player (for preventing consecutive arrests)
     */
    std::string get_last_arrested_player() const { return last_arrested_player; }
    
    /**
     * @brief Sets the name of the last arrested player
     * @param name Name of the player who was just arrested
     */
    void set_last_arrested_player(const std::string& name) { last_arrested_player = name; }

    // Simple action management
    
    /**
     * @brief Gets the number of actions remaining for current player
     * @return Number of actions left this turn
     */
    int get_actions_remaining() const { return actions_remaining; }
    
    /**
     * @brief Adds extra actions for current player (e.g., from bribe)
     * @param count Number of extra actions to add
     */
    void add_extra_actions(int count) { actions_remaining += count; }
    
    /**
     * @brief Consumes one action from current player's remaining actions
     */
    void consume_action() { if (actions_remaining > 0) actions_remaining--; }
    
    /**
     * @brief Starts a new turn with 1 action available
     */
    void start_turn_actions() { actions_remaining = 1; }

    // Validation methods
    
    /**
     * @brief Validates that the game is in a valid state
     * @throws GameException if game hasn't been started
     */
    void validate_game_state() const;
    
    /**
     * @brief Validates the current player count
     * @throws TooManyPlayersException if at maximum capacity (6 players)
     */
    void validate_player_count() const;

    /**
     * @brief Creates a random player with specified name
     * @param name Name for the new player
     * @return Shared pointer to newly created random player
     */
    std::shared_ptr<Player> create_random_player(const std::string& name);
    
    /**
     * @brief Forces cleanup of inactive players (for GUI after elimination display)
     * @details Public method to remove eliminated players from the list
     */
    void force_cleanup_inactive_players();

private:
    /**
     * @brief Removes inactive players from the game
     * @details Adjusts current_turn index to account for removed players
     */
    void cleanup_inactive_players();
};

} // namespace coup 