//meirshuker159@gmail.com


#pragma once
#include <string>
#include <memory>

namespace coup {

class Game;  // Forward declaration

/**
 * @brief Abstract base class for all player types in the Coup game
 * @details Provides common functionality for all roles including basic actions,
 * coin management, and status tracking. Each role inherits from this class.
 */
class Player {
protected:
    // Core properties
    std::string name; ///< Player's display name
    int coins; ///< Current coin count
    bool active; ///< Whether player is still in the game
    bool sanctioned; ///< Whether player is under sanctions this turn
    std::weak_ptr<Game> game; ///< Reference to the game instance
    
    // Game state tracking
    std::string last_arrested_player; ///< Last player this player arrested
    bool arrest_blocked; ///< Whether arrest ability is blocked this turn

public:
    // Constructor and destructor
    
    /**
     * @brief Constructs a new Player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     * @details Initializes player with 0 coins, active status, no sanctions
     */
    Player(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~Player() = default;

    // Core game actions
    
    /**
     * @brief Performs the gather action (gain 1 coin from treasury)
     * @throws IllegalMoveException if player is sanctioned
     * @throws NotYourTurnException if not player's turn
     * @throws GameException if treasury is empty
     * @details Adds 1 coin to player, removes 1 from treasury, advances turn
     */
    virtual void gather();
    
    /**
     * @brief Performs the tax action (gain coins from treasury)
     * @throws IllegalMoveException if player is sanctioned
     * @throws NotYourTurnException if not player's turn
     * @throws GameException if treasury insufficient
     * @details Governor gets 3 coins, others get 2. Advances turn
     */
    virtual void tax();
    
    /**
     * @brief Performs the bribe action (pay 4 coins for 2 extra actions)
     * @throws NotEnoughCoinsException if player has less than 4 coins
     * @throws NotYourTurnException if not player's turn
     * @details Costs 4 coins, grants 2 additional actions this turn
     */
    virtual void bribe();
    
    /**
     * @brief Performs arrest action against target player
     * @param target Player to arrest (cannot be self)
     * @throws IllegalTargetException if target is self or inactive
     * @throws IllegalMoveException if arrest blocked or same target as last arrest
     * @throws NotYourTurnException if not player's turn
     * @details General: no coin transfer, Merchant: pays treasury, Others: steal 1 coin
     */
    virtual void arrest(Player& target);
    
    /**
     * @brief Performs sanction action against target player
     * @param target Player to sanction (cannot be self)
     * @throws NotEnoughCoinsException if insufficient coins (3 for normal, 4 for Judge)
     * @throws IllegalTargetException if target is self or inactive
     * @throws NotYourTurnException if not player's turn
     * @details Prevents target from gathering/taxing next turn. Baron gets compensation
     */
    virtual void sanction(Player& target);
    
    /**
     * @brief Performs coup action against target player (eliminates them)
     * @param target Player to coup (cannot be self)
     * @throws NotEnoughCoinsException if player has less than 7 coins
     * @throws IllegalTargetException if target is self or inactive
     * @throws NotYourTurnException if not player's turn
     * @details Costs 7 coins, eliminates target player, advances turn
     */
    virtual void coup(Player& target);

    // Role-specific methods
    
    /**
     * @brief Gets the role name of this player
     * @return String name of the player's role
     * @details Pure virtual function - must be implemented by derived classes
     */
    virtual std::string role() const = 0;
    
    /**
     * @brief Checks if this player can block a specific action
     * @param action Name of the action to potentially block
     * @return true if this role can block the specified action
     * @details Default implementation returns false. Override in specific roles
     */
    virtual bool can_block([[maybe_unused]] const std::string& action) const { return false; }
    
    /**
     * @brief Called at the start of this player's turn
     * @details Hook for role-specific turn start effects (e.g., Merchant bonus)
     */
    virtual void on_turn_start() {}

    // Getters
    
    /**
     * @brief Gets the player's name
     * @return Player's display name
     */
    std::string get_name() const { return name; }
    
    /**
     * @brief Gets the player's current coin count
     * @return Number of coins the player has
     */
    int get_coins() const { return coins; }
    
    /**
     * @brief Checks if the player is still active in the game
     * @return true if player has not been eliminated
     */
    bool is_active() const { return active; }
    
    /**
     * @brief Checks if the player is currently sanctioned
     * @return true if player cannot gather or tax this turn
     */
    bool is_sanctioned() const { return sanctioned; }
    
    /**
     * @brief Gets the name of the last player this player arrested
     * @return Name of last arrested player (for preventing consecutive arrests)
     */
    std::string get_last_arrested_player() const { return last_arrested_player; }
    
    /**
     * @brief Gets weak reference to the game instance
     * @return Weak pointer to the game object
     */
    std::weak_ptr<Game> get_game() const { return game; }
    
    /**
     * @brief Checks if the player's arrest ability is blocked
     * @return true if arrest action is blocked this turn
     */
    bool is_arrest_blocked() const { return arrest_blocked; }

    // Setters
    
    /**
     * @brief Sets the player's sanction status
     * @param value true to sanction player, false to clear sanctions
     */
    void set_sanctioned(bool value) { sanctioned = value; }
    
    /**
     * @brief Sets the last player this player arrested
     * @param player Name of the player that was arrested
     */
    void set_last_arrested_player(const std::string& player) { last_arrested_player = player; }
    
    /**
     * @brief Sets whether the player's arrest ability is blocked
     * @param val true to block arrest, false to unblock
     */
    void set_arrest_blocked(bool val) { arrest_blocked = val; }
    
    /**
     * @brief Eliminates the player from the game
     * @details Sets active status to false, player can no longer take actions
     */
    void deactivate() { active = false; }

    // Utility methods
    
    /**
     * @brief Adds coins to the player's total
     * @param amount Number of coins to add (must be positive)
     */
    void add_coins(int amount);
    
    /**
     * @brief Removes coins from the player's total
     * @param amount Number of coins to remove
     * @throws NotEnoughCoinsException if player doesn't have enough coins
     */
    void remove_coins(int amount);

protected:
    // Validation methods
    
    /**
     * @brief Validates that the player can perform an action
     * @throws GameException if game no longer exists
     * @throws IllegalMoveException if player is not active
     * @throws NotYourTurnException if it's not the player's turn
     */
    void validate_action() const;
    
    /**
     * @brief Validates that a target player is valid for actions
     * @param target The target player to validate
     * @throws IllegalTargetException if target is not active
     */
    void validate_target(const Player& target) const;
    
    /**
     * @brief Validates that the player has enough coins for an action
     * @param required Number of coins required
     * @throws NotEnoughCoinsException if player has insufficient coins
     */
    void validate_coins(int required) const;
};

} // namespace coup 