//meirshuker159@gmail.com

#include "Roles.hpp"
#include <algorithm>
#include "Exceptions.hpp"
#include "Game.hpp"
#include <iostream>

namespace coup {

// ================================
// GOVERNOR ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a Governor player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details Governor is the economic powerhouse role with enhanced tax collection
 * and the ability to block tax actions from other players.
 */
Governor::Governor(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

Governor::~Governor() = default;

/**
 * @brief Governor's enhanced tax action
 * @details Governors collect 3 coins instead of the standard 2 coins when using tax.
 * This calls the base Player::tax() method which already handles the role-specific logic.
 */
void Governor::tax() {
    Player::tax();  // Base implementation handles Governor's 3-coin tax
}

/**
 * @brief Determines if Governor can block a specific action
 * @param action The action name to check for blocking capability
 * @return true if Governor can block the action, false otherwise
 * @details Governor can block "tax" actions from other players, protecting
 * the treasury from excessive taxation by opponents.
 */
bool Governor::can_block(const std::string& action) const {
    std::string act = action;
    std::transform(act.begin(), act.end(), act.begin(), ::tolower);
    return act == "tax";
}

// ================================
// SPY ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a Spy player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details Spy is the information warfare role with abilities to gather intelligence
 * and disrupt opponent actions without ending their turn.
 */
Spy::Spy(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Investigates another player to reveal their status
 * @param target The player to investigate
 * @throws IllegalTargetException if trying to investigate self
 * @throws GameException if game instance is invalid
 * @details This is a non-turn-ending action that reveals:
 * - Target's coin count
 * - Target's role
 * - Target's sanction status
 * The spy can continue with other actions after investigating.
 * This provides crucial information for strategic decision-making.
 */
void Spy::investigate(Player& target) {
    // Standard action validation (turn, active status, game state)
    validate_action();
    validate_target(target);
    
    // Investigate cannot target self (no self-inspection allowed)
    if (&target == this) {
        throw IllegalTargetException("Cannot investigate yourself");
    }
    
    // Log the investigation results for all players to see
    std::cout << "[SPY] " << get_name() << " investigated " << target.get_name() << " (" << target.role() 
              << ") - discovered: " << target.get_coins() << " coins, " 
              << (target.is_sanctioned() ? "sanctioned" : "not sanctioned") << ", "
              << std::endl;
    
    // Note: Spy can see target's coins and role - GUI will handle detailed display
    // This is a non-turn-ending action - spy retains their remaining actions
    // *** NOT calling next_turn() - spy can continue with other actions ***
}

/**
 * @brief Blocks another player's arrest ability for the remainder of their turn
 * @param target The player whose arrest ability to block
 * @throws IllegalTargetException if trying to block own arrest ability
 * @throws GameException if game instance is invalid
 * @details This is a non-turn-ending action that prevents the target
 * from using arrest until their turn ends. The spy can continue with
 * other actions after blocking. This is a powerful defensive/disruptive ability.
 */
void Spy::block_arrest_ability(Player& target) {
    // Standard action validation (turn, active status, game state)
    validate_action();
    validate_target(target);
    
    // Block arrest cannot target self (cannot block own abilities)
    if (&target == this) {
        throw IllegalTargetException("Cannot block your own arrest ability");
    }
    
    // Apply the arrest block effect to the target
    target.set_arrest_blocked(true);  // Block target's arrest ability for this turn
    
    // Log the blocking action
    std::cout << "[SPY] " << get_name() << " blocked " << target.get_name() << " (" << target.role() 
              << ")'s arrest ability for this turn" << std::endl;
    
    // This is a non-turn-ending action - spy retains their remaining actions
    // NO next_turn() call - Spy can continue with other actions
}

// ================================
// BARON ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a Baron player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details Baron is the investment specialist role with economic abilities
 * and compensation mechanics when targeted by sanctions.
 */
Baron::Baron(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

Baron::~Baron() = default;

/**
 * @brief Baron's sanction action with compensation mechanics
 * @details Baron can sanction other players, and when sanctioned by others,
 * they receive 1 compensation coin. This calls the base implementation
 * which handles the Baron compensation logic.
 */
void Baron::sanction(Player& target) {
    Player::sanction(target);  // Base implementation handles Baron compensation
}

/**
 * @brief Investment action - pay 3 coins to get 6 from treasury (net +3 coins)
 * @throws IllegalMoveException if Baron doesn't have 3 coins or treasury is insufficient
 * @throws GameException if game instance is invalid
 * @details This is Baron's special ability that provides a guaranteed
 * return on investment. Requires:
 * - Baron has at least 3 coins to invest
 * - Treasury has at least 6 coins for the return
 * Net effect is +3 coins for the Baron, making it an efficient economic action.
 */
void Baron::invest() {
    // Standard action validation (turn, active status, game state)
    validate_action();
    
    // Ensure Baron has enough coins for the investment
    validate_coins(3);
    
    // Get valid game reference (weak_ptr safety check)
    auto game_ptr = get_game().lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Check if treasury has enough coins for the investment return
    if (game_ptr->get_treasury() < 6) {
        throw IllegalMoveException("Treasury doesn't have enough coins for investment return");
    }
    
    // Log the investment action before execution
    std::cout << "[BARON] " << get_name() << " invested 3 coins to get 6 coins from treasury (net +3) - now has " 
              << (get_coins() + 3) << " coins (Treasury: " << (game_ptr->get_treasury() - 3) << ")" << std::endl;
    
    // Execute the investment transaction
    remove_coins(3);                        // Baron pays 3 coins
    game_ptr->add_to_treasury(3);          // Add Baron's payment to treasury
    game_ptr->remove_from_treasury(6);     // Remove return payment from treasury
    add_coins(6);                          // Give Baron the 6-coin return
    
    // Handle turn progression using action counter system
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

// ================================
// GENERAL ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a General player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details General is the defensive specialist role with coup immunity
 * and arrest immunity, making them difficult to eliminate or exploit.
 */
General::General(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Determines if General can block a specific action
 * @param action The action name to check for blocking capability
 * @return true if General can block the action and has sufficient coins
 * @details General can block "coup" actions by paying 5 coins, providing
 * immunity from elimination. This is their signature defensive ability.
 * Requires at least 5 coins to activate the block.
 */
bool General::can_block(const std::string& action) const {
    std::string act = action;
    std::transform(act.begin(), act.end(), act.begin(), ::tolower);
    return act == "coup" && get_coins() >= 5;
}

// ================================
// JUDGE ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a Judge player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details Judge is the action control role with the ability to block bribe actions
 * and increased resistance to sanctions (costs 4 coins to sanction instead of 3).
 */
Judge::Judge(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

/**
 * @brief Determines if Judge can block a specific action
 * @param action The action name to check for blocking capability
 * @return true if Judge can block the action, false otherwise
 * @details Judge can block "bribe" actions, preventing players from
 * purchasing extra actions. This helps control the pace of the game
 * and limits turn extension strategies.
 */
bool Judge::can_block(const std::string& action) const {
    std::string act = action;
    std::transform(act.begin(), act.end(), act.begin(), ::tolower);
    return act == "bribe";
}

// ================================
// MERCHANT ROLE IMPLEMENTATION
// ================================

/**
 * @brief Constructs a Merchant player
 * @param game Shared pointer to the game instance
 * @param name The player's name/identifier
 * @details Merchant is the wealth accumulation role with bonus income mechanics
 * and special arrest payment rules that redirect coins to the treasury instead
 * of the arresting player.
 */
Merchant::Merchant(std::shared_ptr<Game> game, const std::string& name)
    : Player(game, name) {}

Merchant::~Merchant() = default;

/**
 * @brief Merchant's gather action with potential bonus income
 * @details Merchant can gather coins like any other player, but their
 * true economic advantage comes from the turn start bonus when wealthy.
 * This calls the base implementation for standard gather mechanics.
 */
void Merchant::gather() {
    Player::gather();  // Standard gather implementation
}

/**
 * @brief Merchant's turn start bonus - gains 1 coin if having 3+ coins
 * @details Automatically called when Merchant's turn begins.
 * If the Merchant has 3 or more coins at turn start, they receive
 * 1 bonus coin from the treasury. This bonus is applied before
 * any actions are taken, encouraging wealth accumulation strategies.
 * The rich get richer mechanic rewards successful economic play.
 */
void Merchant::on_turn_start() {
    // Check if Merchant qualifies for bonus income (wealth threshold)
    if (get_coins() >= 3) {
        // Get valid game reference and check treasury availability
        auto game_ptr = get_game().lock();
        if (game_ptr && game_ptr->get_treasury() >= 1) {
            // Execute bonus income transaction
            game_ptr->remove_from_treasury(1);
            add_coins(1);
            
            // Log the bonus income for tracking
            std::cout << "[MERCHANT BONUS] " << get_name() << " received bonus coin at turn start (had 3+ coins) - now has " 
                      << get_coins() << " coins (Treasury: " << game_ptr->get_treasury() << ")" << std::endl;
        }
    }
}

} // namespace coup
