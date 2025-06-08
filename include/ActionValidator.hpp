//meirshuker159@gmail.com

#pragma once
#include <string>
#include <memory>

namespace coup {

class Player;
class Game;

/**
 * @brief Structure to hold validation results with success status and error message
 * @details Used internally by ActionValidator to return validation results
 * without throwing exceptions, allowing for more flexible error handling.
 */
struct ValidationResult {
    bool isValid; ///< Whether the validation passed
    std::string errorMessage; ///< Error message if validation failed
    
    /**
     * @brief Constructs a ValidationResult
     * @param valid Whether the validation passed (default: true)
     * @param message Error message if validation failed (default: empty)
     */
    ValidationResult(bool valid = true, const std::string& message = "") 
        : isValid(valid), errorMessage(message) {}
    
    /**
     * @brief Creates a valid result
     * @return ValidationResult indicating success
     */
    static ValidationResult valid() { return ValidationResult(true); }
    
    /**
     * @brief Creates an invalid result with error message
     * @param message The error message describing why validation failed
     * @return ValidationResult indicating failure with message
     */
    static ValidationResult invalid(const std::string& message) { return ValidationResult(false, message); }
};

/**
 * @brief Static utility class for validating game actions before execution
 * @details Provides comprehensive validation for all game actions including:
 * - Coin requirements and availability
 * - Turn management and player state
 * - Target validation and requirements
 * - Role-specific restrictions and abilities
 * - Game state and rule enforcement
 */
class ActionValidator {
public:
    /**
     * @brief Checks if an action is available for a player (basic validation only)
     * @param action Name of the action to check
     * @param player Player attempting the action
     * @return true if action is available (doesn't check targets)
     * @details Used for determining if action buttons should be enabled.
     * Performs basic checks but doesn't validate targets.
     */
    static bool isActionAvailable(const std::string& action, std::shared_ptr<Player> player);
    
    /**
     * @brief Checks if action is available including basic validation for UI buttons
     * @param action Name of the action to check
     * @param player Player attempting the action
     * @return true if action should be available in UI
     * @details More comprehensive than isActionAvailable, includes turn and state checks
     */
    static bool isActionAvailableForButton(const std::string& action, std::shared_ptr<Player> player);
    
    /**
     * @brief Validates action execution and throws appropriate exceptions on failure
     * @param action Name of the action to validate
     * @param actor Player performing the action
     * @param target Target player for actions that require one (default: nullptr)
     * @throws NotEnoughCoinsException if insufficient coins
     * @throws NotYourTurnException if not player's turn
     * @throws IllegalTargetException if target is invalid
     * @throws IllegalMoveException for other rule violations
     * @throws GameException for game state errors
     * @details Complete validation including all requirements and restrictions
     */
    static void validateActionExecution(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target = nullptr);
    
    /**
     * @brief Gets the coin cost for a specific action
     * @param action Name of the action
     * @param player Player performing action (used for role-specific costs)
     * @return Number of coins required (0 if no cost or unknown action)
     * @details Handles special cases like Judge sanction costing 4 instead of 3
     */
    static int getActionCost(const std::string& action, std::shared_ptr<Player> player = nullptr);
    
    /**
     * @brief Checks if an action requires a target player
     * @param action Name of the action to check
     * @return true if action needs a target (arrest, sanction, coup, investigate, block)
     */
    static bool requiresTarget(const std::string& action);
    
    /**
     * @brief Gets detailed validation result without throwing exceptions
     * @param action Name of the action to validate
     * @param actor Player performing the action
     * @param target Target player for actions that require one (default: nullptr)
     * @return ValidationResult with success status and error message
     * @details Non-throwing version of validateActionExecution for error handling
     */
    static ValidationResult getValidationResult(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target = nullptr);

private:
    /**
     * @brief Validates coin requirements for an action
     * @param action Name of the action
     * @param player Player performing the action
     * @return ValidationResult indicating if player has sufficient coins
     */
    static ValidationResult validateCoins(const std::string& action, std::shared_ptr<Player> player);
    
    /**
     * @brief Validates player state (active, etc.)
     * @param player Player to validate
     * @return ValidationResult indicating if player can perform actions
     */
    static ValidationResult validatePlayerState(std::shared_ptr<Player> player);
    
    /**
     * @brief Validates target player for targeted actions
     * @param action Name of the action
     * @param actor Player performing the action
     * @param target Target player to validate
     * @return ValidationResult indicating if target is valid
     */
    static ValidationResult validateTarget(const std::string& action, std::shared_ptr<Player> actor, std::shared_ptr<Player> target);
    
    /**
     * @brief Validates game state and turn management
     * @param player Player attempting action
     * @return ValidationResult indicating if game state allows action
     */
    static ValidationResult validateGameState(std::shared_ptr<Player> player);
    
    /**
     * @brief Validates role-specific requirements and restrictions
     * @param action Name of the action
     * @param player Player performing the action
     * @return ValidationResult indicating if role allows this action
     * @details Checks sanctions, arrest blocks, role abilities, etc.
     */
    static ValidationResult validateRoleSpecificRequirements(const std::string& action, std::shared_ptr<Player> player);
};

} // namespace coup 