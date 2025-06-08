//meirshuker159@gmail.com

#include "ActionValidator.hpp"
#include "Player.hpp"
#include "Game.hpp"
#include "Roles.hpp"
#include "Exceptions.hpp"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace coup {

/**
 * @brief Determines if an action is available for a player (simplified check)
 * @details Uses getValidationResult internally to check basic availability
 * without requiring target specification. Used for quick availability checks.
 */
bool ActionValidator::isActionAvailable(const std::string& action, std::shared_ptr<Player> player) {
    if (!player) return false;
    
    ValidationResult result = getValidationResult(action, player);
    return result.isValid;
}

/**
 * @brief Enhanced availability check specifically for UI button states
 * @details Performs comprehensive validation excluding target requirements,
 * which is perfect for determining if action buttons should be enabled.
 * Includes mandatory coup rule enforcement and role-specific restrictions.
 */
bool ActionValidator::isActionAvailableForButton(const std::string& action, std::shared_ptr<Player> player) {
    if (!player) return false;
    
    // For button states, we don't check targets - just basic availability
    if (!player) {
        return false;
    }
    
    // Check mandatory coup rule first
    if (player->get_coins() >= 10 && action != "Coup" && action != "End Turn") {
        return false;
    }
    
    // Check player state
    ValidationResult playerState = validatePlayerState(player);
    if (!playerState.isValid) return false;
    
    // Check game state
    ValidationResult gameState = validateGameState(player);
    if (!gameState.isValid) return false;
    
    // Check coin requirements
    ValidationResult coinCheck = validateCoins(action, player);
    if (!coinCheck.isValid) return false;
    
    // Check role-specific requirements
    ValidationResult roleCheck = validateRoleSpecificRequirements(action, player);
    if (!roleCheck.isValid) return false;
    
    // Don't check target requirements for button availability
    return true;
}

/**
 * @brief Validates action execution and throws appropriate typed exceptions
 * @details Uses getValidationResult for validation, then analyzes error messages
 * to throw the most appropriate exception type. This provides precise error
 * handling for different validation failure scenarios.
 */
void ActionValidator::validateActionExecution(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target) {
    ValidationResult result = getValidationResult(action, actor, target);
    if (!result.isValid) {
        // Determine the appropriate exception type based on the error message
        if (result.errorMessage.find("coins") != std::string::npos) {
            throw NotEnoughCoinsException(result.errorMessage);
        } else if (result.errorMessage.find("turn") != std::string::npos) {
            throw NotYourTurnException(result.errorMessage);
        } else if (result.errorMessage.find("target") != std::string::npos || result.errorMessage.find("Target") != std::string::npos) {
            throw IllegalTargetException(result.errorMessage);
        } else if (result.errorMessage.find("game") != std::string::npos || result.errorMessage.find("Game") != std::string::npos) {
            throw GameException(result.errorMessage);
        } else {
            throw IllegalMoveException(result.errorMessage);
        }
    }
}

/**
 * @brief Returns coin cost for specified action using static lookup table
 * @details Uses unordered_map for O(1) lookup performance. Handles base costs,
 * with special cases (like Judge sanction costing 4) handled in validation logic.
 * Returns 0 for unknown actions or free actions.
 */
int ActionValidator::getActionCost(const std::string& action, [[maybe_unused]] std::shared_ptr<Player> player) {
    // Use a static map for better performance and organization
    static const std::unordered_map<std::string, int> actionCosts = {
        {"Gather", 0},
        {"Tax", 0},
        {"Bribe", 4},
        {"Arrest", 0},
        {"Coup", 7},
        {"Investigate", 0},
        {"Block Arrest", 0},
        {"End Turn", 0},
        {"Sanction", 3},  // Default cost, actual validation handles Judge case
        {"Invest", 3}     // Baron ability
    };
    
    auto it = actionCosts.find(action);
    return (it != actionCosts.end()) ? it->second : 0;
}

/**
 * @brief Determines if an action requires a target player using static set lookup
 * @details Uses unordered_set for fast O(1) lookup. Actions like arrest, sanction,
 * coup, investigate, and block arrest all require valid target players.
 */
bool ActionValidator::requiresTarget(const std::string& action) {
    static const std::unordered_set<std::string> targetRequiredActions = {
        "Arrest", "Sanction", "Coup", "Investigate", "Block Arrest"
    };
    return targetRequiredActions.find(action) != targetRequiredActions.end();
}

/**
 * @brief Comprehensive validation pipeline for action execution
 * @details Performs validation in logical order:
 * 1. Actor existence check
 * 2. Mandatory coup rule enforcement  
 * 3. Player state validation
 * 4. Game state validation
 * 5. Coin requirement validation
 * 6. Role-specific requirement validation
 * 7. Target validation (if required)
 * Early returns on first validation failure for efficiency.
 */
ValidationResult ActionValidator::getValidationResult(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target) {
    if (!actor) {
        return ValidationResult::invalid("No actor specified");
    }
    
    // Check mandatory coup rule first
    if (actor->get_coins() >= 10 && action != "Coup" && action != "End Turn") {
        return ValidationResult::invalid("Must perform coup when having 10 or more coins");
    }
    
    // Check player state
    ValidationResult playerState = validatePlayerState(actor);
    if (!playerState.isValid) return playerState;
    
    // Check game state
    ValidationResult gameState = validateGameState(actor);
    if (!gameState.isValid) return gameState;
    
    // Check coin requirements
    ValidationResult coinCheck = validateCoins(action, actor);
    if (!coinCheck.isValid) return coinCheck;
    
    // Check role-specific requirements
    ValidationResult roleCheck = validateRoleSpecificRequirements(action, actor);
    if (!roleCheck.isValid) return roleCheck;
    
    // Check target requirements
    if (requiresTarget(action)) {
        ValidationResult targetCheck = validateTarget(action, actor, target);
        if (!targetCheck.isValid) return targetCheck;
    }
    
    return ValidationResult::valid();
}

/**
 * @brief Validates coin requirements with special case handling
 * @details Checks basic coin requirements using getActionCost, with special
 * handling for Sanction action where Judge targets cost 4 instead of 3.
 * For button availability, uses base cost of 3 for sanction.
 */
ValidationResult ActionValidator::validateCoins(const std::string& action, std::shared_ptr<Player> player) {
    int requiredCoins = getActionCost(action, player);
    
    // Special case for Sanction: Judge costs 4, others cost 3
    if (action == "Sanction" && requiredCoins == 3) {
        // We need the target to determine the exact cost, but for button availability we use base cost
        if (player->get_coins() < 3) {
            return ValidationResult::invalid("Need at least 3 coins for sanction");
        }
    } else if (requiredCoins > 0 && player->get_coins() < requiredCoins) {
        return ValidationResult::invalid("Need " + std::to_string(requiredCoins) + " coins for " + action);
    }
    
    return ValidationResult::valid();
}

/**
 * @brief Validates player state (active status)
 * @details Simple check to ensure player is still active in the game.
 * Eliminated players cannot perform any actions.
 */
ValidationResult ActionValidator::validatePlayerState(std::shared_ptr<Player> player) {
    if (!player->is_active()) {
        return ValidationResult::invalid("Player is not active");
    }
    
    return ValidationResult::valid();
}

/**
 * @brief Validates target player for targeted actions
 * @details Ensures target exists, is active, and enforces universal
 * self-targeting prohibition. All actions in Coup that require targets
 * prevent players from targeting themselves.
 */
ValidationResult ActionValidator::validateTarget(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target) {
    if (!target) {
        return ValidationResult::invalid("Target required for " + action);
    }
    
    if (!target->is_active()) {
        return ValidationResult::invalid("Target player is not active");
    }
    
    // Self-targeting rules - no action can target self
    if (actor == target) {
        return ValidationResult::invalid("Cannot target yourself with " + action);
    }
    
    return ValidationResult::valid();
}

/**
 * @brief Validates game state and turn management
 * @details Checks game instance validity, turn ownership, and overall
 * game state through the game's validate_game_state method. Uses
 * exception handling to convert game validation exceptions to ValidationResult.
 */
ValidationResult ActionValidator::validateGameState(std::shared_ptr<Player> player) {
    auto game_ptr = player->get_game().lock();
    if (!game_ptr) {
        return ValidationResult::invalid("Game no longer exists");
    }
    
    if (!game_ptr->is_player_turn(player.get())) {
        return ValidationResult::invalid("Not your turn");
    }
    
    try {
        game_ptr->validate_game_state();
    } catch (const std::exception& e) {
        return ValidationResult::invalid(e.what());
    }
    
    return ValidationResult::valid();
}

/**
 * @brief Validates role-specific restrictions and status effects
 * @details Handles sanctions (blocking gather/tax), arrest blocks,
 * role abilities, and other special restrictions. This is where
 * most game rule enforcement happens at the role level.
 */
ValidationResult ActionValidator::validateRoleSpecificRequirements(const std::string& action, std::shared_ptr<Player> player) {
    // Sanction restrictions
    if ((action == "Gather" || action == "Tax") && player->is_sanctioned()) {
        return ValidationResult::invalid("You are under sanctions and cannot gather or tax");
    }
    
    // Arrest block restrictions
    if (action == "Arrest" && player->is_arrest_blocked()) {
        return ValidationResult::invalid("Your arrest ability is blocked this turn");
    }
    
    // Role-specific action availability
    if (action == "Invest" && player->role() != "Baron") {
        return ValidationResult::invalid("Only Baron can invest");
    }
    
    if (action == "Investigate" && player->role() != "Spy") {
        return ValidationResult::invalid("Only Spy can investigate");
    }
    
    if (action == "Block Arrest" && player->role() != "Spy") {
        return ValidationResult::invalid("Only Spy can block arrest abilities");
    }
    
    return ValidationResult::valid();
}

} // namespace coup 