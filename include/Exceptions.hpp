//meirshuker159@gmail.com


#pragma once
#include <stdexcept>
#include <string>

namespace coup {

/**
 * @brief Base exception class for all Coup game-related errors
 * @details Inherits from std::runtime_error and serves as the base class
 * for all specific game exceptions. Provides a common interface for
 * catching any game-related error.
 */
class GameException : public std::runtime_error {
public:
    /**
     * @brief Constructs a GameException with error message
     * @param message Descriptive error message explaining what went wrong
     */
    explicit GameException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when a player attempts an illegal move
 * @details Used for rule violations such as:
 * - Acting when sanctioned (for gather/tax)
 * - Attempting arrest when blocked
 * - Other game rule violations
 */
class IllegalMoveException : public GameException {
public:
    /**
     * @brief Constructs an IllegalMoveException with error message
     * @param message Descriptive error message explaining the illegal move
     */
    explicit IllegalMoveException(const std::string& message) : GameException(message) {}
};

/**
 * @brief Exception thrown when a player doesn't have enough coins for an action
 * @details Used when players attempt actions that cost more coins than they have:
 * - Bribe (requires 4 coins)
 * - Coup (requires 7 coins)
 * - Sanction (requires 3 coins, 4 for Judge)
 * - Invest (requires 3 coins)
 */
class NotEnoughCoinsException : public GameException {
public:
    /**
     * @brief Constructs a NotEnoughCoinsException with error message
     * @param message Descriptive error message about insufficient coins
     */
    explicit NotEnoughCoinsException(const std::string& message) : GameException(message) {}
};

/**
 * @brief Exception thrown when an action targets an invalid player
 * @details Used for targeting errors such as:
 * - Self-targeting (all actions prevent this)
 * - Targeting inactive/eliminated players
 * - Missing target for actions that require one
 */
class IllegalTargetException : public GameException {
public:
    /**
     * @brief Constructs an IllegalTargetException with error message
     * @param message Descriptive error message about the invalid target
     */
    explicit IllegalTargetException(const std::string& message) : GameException(message) {}
};

/**
 * @brief Exception thrown when a player attempts to act when it's not their turn
 * @details Used to enforce turn order and prevent players from acting
 * out of sequence. Turn management is critical for fair gameplay.
 */
class NotYourTurnException : public GameException {
public:
    /**
     * @brief Constructs a NotYourTurnException with error message
     * @param message Descriptive error message about turn violation
     */
    explicit NotYourTurnException(const std::string& message) : GameException(message) {}
};

/**
 * @brief Exception thrown when trying to add more than the maximum number of players
 * @details The game supports 2-6 players maximum. This exception is thrown
 * when attempting to add a 7th player to maintain game balance.
 */
class TooManyPlayersException : public GameException {
public:
    /**
     * @brief Constructs a TooManyPlayersException with error message
     * @param message Descriptive error message about player limit
     */
    explicit TooManyPlayersException(const std::string& message) : GameException(message) {}
};

/**
 * @brief Exception thrown when trying to find a player that doesn't exist
 * @details Used when searching for players by name and the specified
 * player is not found in the current game.
 */
class PlayerNotFoundException : public GameException {
public:
    /**
     * @brief Constructs a PlayerNotFoundException with error message
     * @param message Descriptive error message about the missing player
     */
    explicit PlayerNotFoundException(const std::string& message) : GameException(message) {}
};

} // namespace coup 