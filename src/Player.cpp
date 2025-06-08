//meirshuker159@gmail.com
#include "Player.hpp"
#include "Game.hpp"
#include "Exceptions.hpp"
#include <iostream>

namespace coup {

/**
 * @brief Constructs a new Player object
 * @param game Shared pointer to the game instance this player belongs to
 * @param name The name/identifier for this player
 * @details Initializes a player with default values: 0 coins, active status,
 * no sanctions, and clears arrest tracking. Uses weak_ptr to avoid circular
 * dependencies with the Game object.
 */
Player::Player(std::shared_ptr<Game> game, const std::string& name)
    : name(name), coins(0), active(true), sanctioned(false), game(game), 
      last_arrested_player(""), arrest_blocked(false) {}

/**
 * @brief Gather action - takes 1 coin from the treasury
 * @throws IllegalMoveException if player is sanctioned
 * @throws GameException if game instance no longer exists
 * @details This is the basic economic action available to all players.
 * Transfers 1 coin from the game treasury to the player's personal funds.
 * Cannot be used while sanctioned. Consumes one action and may end turn.
 */
void Player::gather() {
    validate_action();
    if (sanctioned) {
        throw IllegalMoveException("Player is under sanctions");
    }

    
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Remove coin from treasury and give to player
    game_ptr->remove_from_treasury(1);
    add_coins(1);
    
    std::cout << "[ACTION] " << name << " (" << role() << ") gathered 1 coin - now has " 
              << coins << " coins (Treasury: " << game_ptr->get_treasury() << ")" << std::endl;
    

    
    // Use action counter system instead of direct turn advancement
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

/**
 * @brief Tax action - takes 2 coins from treasury (3 for Governor)
 * @throws IllegalMoveException if player is sanctioned
 * @throws GameException if game instance no longer exists
 * @details This economic action provides better return than gather but can be blocked.
 * Governors receive 3 coins instead of 2 due to their special ability.
 * Cannot be used while sanctioned. Consumes one action and may end turn.
 */
void Player::tax() {
    validate_action();
    if (sanctioned) {
        throw IllegalMoveException("Player is under sanctions");
    }

    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Governor gets 3 coins, other roles get 2
    int tax_amount = (role() == "Governor") ? 3 : 2;
    
    // Remove coins from treasury and give to player
    game_ptr->remove_from_treasury(tax_amount);
    add_coins(tax_amount);
    
    std::cout << "[ACTION] " << name << " (" << role() << ") taxed " << tax_amount << " coins - now has " 
              << coins << " coins (Treasury: " << game_ptr->get_treasury() << ")" << std::endl;
    

    
    // Use action counter system instead of direct turn advancement
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

/**
 * @brief Bribe action - pay 4 coins to gain 2 extra actions
 * @throws IllegalMoveException if player doesn't have enough coins
 * @throws GameException if game instance no longer exists
 * @details This action allows players to extend their turn by purchasing additional actions.
 * Costs 4 coins and grants 2 extra actions (net gain of 1 action after consumption).
 * The coins are returned to the treasury. Can be blocked by Judge role.
 */
void Player::bribe() {
    validate_action();
    validate_coins(4);

    
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    std::cout << "[ACTION] " << name << " (" << role() << ") used bribe (paid 4 coins) - now has " 
              << (coins - 4) << " coins and gets 2 extra actions" << std::endl;
    
    remove_coins(4);
    game_ptr->add_to_treasury(4);  // Return coins to treasury
    
    // Add 2 extra actions using simple counter
    game_ptr->add_extra_actions(2);
    

    
    // Use action counter system - bribe consumes 1 action but adds 2 extra
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

/**
 * @brief Arrests another player with role-specific behavior handling
 * @param target The player to arrest
 * @details Implementation handles three distinct arrest scenarios:
 * - General: Immune to coin transfer (logs immunity message)
 * - Merchant: Pays up to 2 coins to treasury instead of arrester
 * - Others: Standard 1 coin transfer from target to arrester
 * Also enforces global arrest restriction preventing consecutive arrests
 * of the same player and individual arrest blocking.
 */
void Player::arrest(Player& target) {
    validate_action();
    validate_target(target);
    
    // Arrest cannot target self
    if (&target == this) {
        throw IllegalTargetException("Cannot arrest yourself");
    }
    
    if (arrest_blocked) {
        throw IllegalMoveException("You are blocked from using arrest this turn");
    }
    
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Global arrest restriction: cannot arrest same player twice in a row
    if (game_ptr->get_last_arrested_player() == target.get_name()) {
        throw IllegalMoveException("Cannot arrest the same player twice in a row");
    }
    
    // General arrest immunity - can be arrested but no coin transfer
    if (target.role() == "General") {
        std::cout << "[ACTION] " << name << " (" << role() << ") arrested " << target.get_name() 
                  << " (General) - General immunity: no coins transferred" << std::endl;
    }
    // Merchant special case: pays 2 coins to the treasury, if he only has 1 coin he pays 1 and if he has 0 he pays 0
    else if (target.role() == "Merchant") {
        if (target.get_coins() > 0) {
            int coinsToTreasury = std::min(target.get_coins(), 2); // Pay max 2 coins or whatever they have
            target.remove_coins(coinsToTreasury);
            game_ptr->add_to_treasury(coinsToTreasury);
            std::cout << "[ACTION] " << name << " (" << role() << ") arrested " << target.get_name() 
                      << " (Merchant) - Merchant paid " << coinsToTreasury << " coins to treasury (now has " 
                      << target.get_coins() << " coins, Treasury: " << game_ptr->get_treasury() << ")" << std::endl;
        } else {
            std::cout << "[ACTION] " << name << " (" << role() << ") arrested " << target.get_name() 
                      << " (Merchant) - but Merchant had no coins to pay" << std::endl;
        }
    } else {
        // Normal arrest: transfer 1 coin from target to arrester
        if (target.get_coins() > 0) {
            target.remove_coins(1);
            add_coins(1);
            std::cout << "[ACTION] " << name << " (" << role() << ") arrested " << target.get_name() 
                      << " (" << target.role() << ") - stole 1 coin (" << name << ": " << coins 
                      << ", " << target.get_name() << ": " << target.get_coins() << ")" << std::endl;
        } else {
            std::cout << "[ACTION] " << name << " (" << role() << ") arrested " << target.get_name() 
                      << " (" << target.role() << ") - but target had no coins to steal" << std::endl;
        }
    }
    
    // Update global arrest tracking
    game_ptr->set_last_arrested_player(target.get_name());

    
    // Use action counter system instead of direct turn advancement
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

/**
 * @brief Sanctions another player with role-specific cost and compensation
 * @param target The player to sanction
 * @details Implementation handles special cases:
 * - Judge: Costs 4 coins instead of 3 to sanction
 * - Baron: Receives 1 compensation coin when sanctioned
 * - Others: Standard 3 coin cost, no compensation
 * Sanctioned players cannot gather or tax on their next turn.
 */
void Player::sanction(Player& target) {
    validate_action();
    validate_target(target);
    
    // Sanction cannot target self
    if (&target == this) {
        throw IllegalTargetException("Cannot sanction yourself");
    }
    
    // Special case: Sanctioning a Judge costs 4 coins instead of 3
    int cost = 3;
    if (target.role() == "Judge") {
        cost = 4;
        validate_coins(4);
        std::cout << "[ACTION] " << name << " sanctioning Judge costs 4 coins instead of 3" << std::endl;
    } else {
        validate_coins(3);
    }

    
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    remove_coins(cost);
    game_ptr->add_to_treasury(cost);  // Return coins to treasury
    
    // Baron gets compensation when sanctioned
    if (target.role() == "Baron") {
        if (game_ptr->get_treasury() >= 1) {
            game_ptr->remove_from_treasury(1);
            target.add_coins(1);
            std::cout << "[ACTION] " << name << " (" << role() << ") sanctioned " << target.get_name() 
                      << " (Baron) - paid " << cost << " coins to treasury, Baron got 1 compensation coin (" << name << ": " 
                      << coins << ", " << target.get_name() << ": " << target.get_coins() << ", Treasury: " 
                      << game_ptr->get_treasury() << ")" << std::endl;
        } else {
            std::cout << "[ACTION] " << name << " (" << role() << ") sanctioned " << target.get_name() 
                      << " (Baron) - paid " << cost << " coins to treasury, but no compensation available" << std::endl;
        }
    } else {
        std::cout << "[ACTION] " << name << " (" << role() << ") sanctioned " << target.get_name() 
                  << " (" << target.role() << ") - paid " << cost << " coins (" << name << ": " 
                  << coins << ", Treasury: " << game_ptr->get_treasury() << ")" << std::endl;
    }
    
    target.set_sanctioned(true);

    
    // Use action counter system instead of direct turn advancement
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}

/**
 * @brief Coup action - pay 7 coins to eliminate another player
 * @param target The player to eliminate
 * @throws IllegalMoveException if player doesn't have enough coins or targets self
 * @throws IllegalTargetException if target is not active
 * @throws GameException if game instance no longer exists
 * @details This is the most powerful action in the game, allowing instant elimination
 * of any player for 7 coins. Cannot be blocked except by General role (who can pay
 * 5 coins to prevent it). Once executed, the target is permanently removed from the game.
 * The coins are returned to the treasury.
 */
void Player::coup(Player& target) {
    // Validate that the player can perform this action
    validate_action();
    validate_target(target);
    
    // Ensure player has enough coins to pay the coup cost
    validate_coins(7);
    
    // Coup cannot target self (self-elimination prevention)
    if (&target == this) {
        throw IllegalTargetException("Cannot coup yourself");
    }

    // Get valid game reference (weak_ptr safety check)
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Log the coup action with detailed information
    std::cout << "[ACTION] " << name << " (" << role() << ") performed coup on " << target.get_name() 
              << " (" << target.role() << ") - paid 7 coins to treasury, target eliminated (" << name << " now has " 
              << (coins - 7) << " coins, Treasury: " << (game_ptr->get_treasury() + 7) << ")" << std::endl;
    
    // Execute payment: player -> treasury
    remove_coins(7);
    game_ptr->add_to_treasury(7);
    
    // Eliminate the target player permanently
    target.deactivate();

    // Handle turn progression using action counter system
    game_ptr->consume_action();
    if (game_ptr->get_actions_remaining() == 0) {
        game_ptr->next_turn();
    }
}



/**
 * @brief Adds coins to the player's personal treasury
 * @param amount The number of coins to add (must be positive)
 * @details This method safely adds coins to the player's balance without
 * overflow checking. Used by game actions and special abilities to grant
 * coins to players. The method assumes the amount is valid and positive.
 */
void Player::add_coins(int amount) {
    coins += amount;
}

/**
 * @brief Removes coins from the player's personal treasury
 * @param amount The number of coins to remove
 * @throws NotEnoughCoinsException if player doesn't have sufficient coins
 * @details This method safely removes coins from the player's balance with
 * underflow protection. Used by game actions that require payment. The method
 * validates that the player has enough coins before performing the subtraction.
 */
void Player::remove_coins(int amount) {
    if (coins < amount) {
        throw NotEnoughCoinsException("Not enough coins for action");
    }
    coins -= amount;
}

/**
 * @brief Validates that the player can perform an action
 * @throws GameException if game instance no longer exists
 * @throws IllegalMoveException if player is not active
 * @throws NotYourTurnException if it's not the player's turn
 * @details This comprehensive validation method checks multiple prerequisites:
 * - Game instance is still valid (weak_ptr check)
 * - Player is still active (not eliminated)
 * - It's currently the player's turn
 * - Game state is valid for actions
 * Should be called at the start of every action method.
 */
void Player::validate_action() const {
    // Ensure game instance still exists
    auto game_ptr = game.lock();
    if (!game_ptr) {
        throw GameException("Game no longer exists");
    }
    
    // Check if player is still in the game
    if (!active) {
        throw IllegalMoveException("Player is not active");
    }
    
    // Verify it's the player's turn
    if (!game_ptr->is_player_turn(this)) {
        throw NotYourTurnException("Not your turn");
    }
    
    // Validate overall game state consistency
    game_ptr->validate_game_state();
}

/**
 * @brief Validates that a target player can be targeted by an action
 * @param target The player being targeted
 * @throws IllegalTargetException if target is not active
 * @details This method performs basic target validation that applies to all
 * targeting actions. Specific actions may have additional targeting restrictions
 * (like self-targeting) that are handled in their individual methods or by
 * the ActionValidator class.
 */
void Player::validate_target(const Player& target) const {
    // Ensure target is still active in the game
    if (!target.is_active()) {
        throw IllegalTargetException("Target player is not active");
    }
    // Note: Self-targeting validation for specific actions is handled 
    // by ActionValidator or in individual action methods
}

/**
 * @brief Validates that the player has sufficient coins for an action
 * @param required The minimum number of coins needed
 * @throws NotEnoughCoinsException if player doesn't have enough coins
 * @details This method checks if the player has at least the required number
 * of coins to perform a paid action. Used by actions like bribe, sanction,
 * and coup to ensure the player can afford the cost before execution.
 */
void Player::validate_coins(int required) const {
    if (coins < required) {
        throw NotEnoughCoinsException("Not enough coins for action");
    }
}

} // namespace coup 