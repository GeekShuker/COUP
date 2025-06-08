//meirshuker159@gmail.com


#pragma once
#include "Player.hpp"
#include <memory>

namespace coup {

/**
 * @brief Governor role class - enhanced tax collection and tax blocking
 * @details Special abilities:
 * - Tax action gives 3 coins instead of 2
 * - Can block tax actions from other players
 */
class Governor : public Player {
public:
    /**
     * @brief Constructs a new Governor player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    Governor(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Governor();
    
    /**
     * @brief Gets the role name
     * @return "Governor"
     */
    std::string role() const override { return "Governor"; }
    
    /**
     * @brief Enhanced tax action that gives 3 coins instead of 2
     * @throws IllegalMoveException if player is sanctioned
     * @throws NotYourTurnException if not player's turn
     * @throws GameException if treasury insufficient
     */
    void tax() override;
    
    /**
     * @brief Checks if Governor can block specific actions
     * @param action The action name to check
     * @return true if action is "tax" (case insensitive)
     */
    bool can_block(const std::string& action) const override;
};

/**
 * @brief Spy role class - investigation and arrest blocking abilities
 * @details Special abilities:
 * - Can investigate other players to see their coins and status
 * - Can block other players' arrest abilities for one turn
 * - Investigation and blocking don't advance turn (can do multiple actions)
 */
class Spy : public Player {
public:
    /**
     * @brief Constructs a new Spy player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    Spy(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Investigates another player to reveal their status
     * @param target Player to investigate (cannot be self)
     * @throws IllegalTargetException if target is self or inactive
     * @throws NotYourTurnException if not player's turn
     * @details Reveals target's coins, sanction status. Doesn't advance turn
     */
    void investigate(Player& target);
    
    /**
     * @brief Gets the role name
     * @return "Spy"
     */
    std::string role() const override { return "Spy"; }
    
    /**
     * @brief Blocks target player's arrest ability for one turn
     * @param target Player whose arrest ability to block (cannot be self)
     * @throws IllegalTargetException if target is self or inactive
     * @throws NotYourTurnException if not player's turn
     * @details Doesn't advance turn, allowing multiple actions
     */
    void block_arrest_ability(Player& target);
};

/**
 * @brief Baron role class - investment ability and sanction compensation
 * @details Special abilities:
 * - Can invest 3 coins to get 6 coins from treasury (net +3)
 * - Receives 1 compensation coin when sanctioned by other players
 * - Investment requires sufficient treasury funds
 */
class Baron : public Player {
public:
    /**
     * @brief Constructs a new Baron player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    Baron(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Baron();
    
    /**
     * @brief Investment action - pay 3 coins to get 6 from treasury
     * @throws NotEnoughCoinsException if player has less than 3 coins
     * @throws IllegalMoveException if treasury has less than 6 coins
     * @throws NotYourTurnException if not player's turn
     * @details Net gain of 3 coins, requires treasury to have sufficient funds
     */
    void invest();
    
    /**
     * @brief Gets the role name
     * @return "Baron"
     */
    std::string role() const override { return "Baron"; }
    
    /**
     * @brief Enhanced sanction that gives Baron compensation when targeted
     * @param target Player to sanction (cannot be self)
     * @throws NotEnoughCoinsException if insufficient coins
     * @throws IllegalTargetException if target is self or inactive
     * @details If Baron is sanctioned, they receive 1 compensation coin
     */
    void sanction(Player& target) override;
    
    /**
     * @brief Resets turn-based effects (currently no effects to reset)
     * @details Placeholder for future turn-based effects
     */
    void reset_turn_effects();
};

/**
 * @brief General role class - coup blocking and arrest immunity
 * @details Special abilities:
 * - Can block coup actions when having 5+ coins
 * - Immune to arrest coin transfer (can be arrested but no coins lost/stolen)
 */
class General : public Player {
public:
    /**
     * @brief Constructs a new General player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    General(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Gets the role name
     * @return "General"
     */
    std::string role() const override { return "General"; }
    
    /**
     * @brief Checks if General can block specific actions
     * @param action The action name to check
     * @return true if action is "coup" (case insensitive) and player has 5+ coins
     */
    bool can_block(const std::string& action) const override;
};

/**
 * @brief Judge role class - bribe blocking and increased sanction cost
 * @details Special abilities:
 * - Can block bribe actions from other players
 * - Costs 4 coins to sanction a Judge instead of the normal 3
 */
class Judge : public Player {
public:
    /**
     * @brief Constructs a new Judge player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    Judge(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Gets the role name
     * @return "Judge"
     */
    std::string role() const override { return "Judge"; }
    
    /**
     * @brief Checks if Judge can block specific actions
     * @param action The action name to check
     * @return true if action is "bribe" (case insensitive)
     */
    bool can_block(const std::string& action) const override;
};

/**
 * @brief Merchant role class - bonus coins and treasury payment on arrest
 * @details Special abilities:
 * - Receives 1 bonus coin at turn start when having 3+ coins
 * - When arrested, pays coins to treasury instead of arrester (max 2 coins)
 */
class Merchant : public Player {
public:
    /**
     * @brief Constructs a new Merchant player
     * @param game Shared pointer to the game instance
     * @param name Display name for the player
     */
    Merchant(std::shared_ptr<Game> game, const std::string& name);
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Merchant();
    
    /**
     * @brief Gets the role name
     * @return "Merchant"
     */
    std::string role() const override { return "Merchant"; }
    
    /**
     * @brief Enhanced gather action (currently same as base class)
     * @throws IllegalMoveException if player is sanctioned
     * @throws NotYourTurnException if not player's turn
     * @details Placeholder for potential future enhancements
     */
    void gather() override;
    
    /**
     * @brief Turn start bonus - gain 1 coin if having 3+ coins
     * @details Automatically called when player's turn begins.
     * Grants 1 bonus coin from treasury if player has 3+ coins
     */
    void on_turn_start() override;
};

} // namespace coup 