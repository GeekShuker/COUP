//meirshuker159@gmail.com
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Game.hpp"
#include "Player.hpp"
#include "Roles.hpp"
#include "Exceptions.hpp"
#include "ActionValidator.hpp"
#include <memory>

using namespace coup;

// Helper function to count active players
int count_active_players(std::shared_ptr<Game> game) {
    int count = 0;
    auto all_players = game->all_players();
    for (const auto& player : all_players) {
        if (player && player->is_active()) {
            count++;
        }
    }
    return count;
}

TEST_CASE("Game initialization") {
    auto game = std::make_shared<Game>();
    CHECK(game->player_count() == 0);
    CHECK_FALSE(game->is_active());
}

TEST_CASE("Player management") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    
    CHECK(game->player_count() == 2);
    game->start_game();  // Start the game
    CHECK(game->turn() == "Governor");
}

TEST_CASE("Basic actions") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();  // Start the game
    
    // Test gather
    governor->gather();
    CHECK(governor->get_coins() == 1);
    
    // Test tax
    spy->tax();
    CHECK(spy->get_coins() == 2);
}

TEST_CASE("Special abilities") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();  // Start the game
    
    // Governor's special tax
    governor->tax();
    CHECK(governor->get_coins() == 3);  // Gets 3 coins instead of 2
    
    // Spy's ability doesn't cost coins
    spy->investigate(*governor);
    CHECK(spy->get_coins() == 0);
}

TEST_CASE("Game rules") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    
    game->add_player(governor);
    
    // Can't play with only one player
    CHECK_THROWS(game->start_game());
    
    // Add another player so we can test coup
    auto spy = std::make_shared<Spy>(game, "Spy");
    game->add_player(spy);
    game->start_game();
    
    // Can't coup without enough coins
    CHECK_THROWS_AS(governor->coup(*spy), NotEnoughCoinsException);
}

TEST_CASE("Turn management") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    auto baron = std::make_shared<Baron>(game, "Baron");
    
    game->add_player(governor);
    game->add_player(spy);
    game->add_player(baron);
    game->start_game();  // Start the game
    
    CHECK(game->turn() == "Governor");
    governor->gather();
    CHECK(game->turn() == "Spy");
    spy->gather();
    CHECK(game->turn() == "Baron");
}

TEST_CASE("Player elimination") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();  // Start the game
    
    // Give enough coins for coup
    for (int i = 0; i < 7; i++) {
        governor->gather();
        spy->gather();
    }
    
    governor->coup(*spy);
    CHECK_FALSE(spy->is_active());
    // With new logic, eliminated players stay in list but game detects active count
    CHECK(count_active_players(game) == 1); // Only 1 active player
    CHECK(game->is_game_over()); // Should be true because only 1 active player
    CHECK(game->winner() == "Governor");
}

// ==================== NEW COMPREHENSIVE TESTS ====================

TEST_CASE("ActionValidator - Basic validation") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();
    
    // Test action availability
    CHECK(ActionValidator::isActionAvailable("Gather", governor));
    CHECK(ActionValidator::isActionAvailable("Tax", governor));
    CHECK_FALSE(ActionValidator::isActionAvailable("Coup", governor)); // Not enough coins
    
    // Test action costs
    CHECK(ActionValidator::getActionCost("Gather", governor) == 0);
    CHECK(ActionValidator::getActionCost("Tax", governor) == 0);
    CHECK(ActionValidator::getActionCost("Bribe", governor) == 4);
    CHECK(ActionValidator::getActionCost("Coup", governor) == 7);
    CHECK(ActionValidator::getActionCost("Sanction", governor) == 3);
}

TEST_CASE("ActionValidator - Target requirements") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    
    game->add_player(governor);
    
    // Test actions that require targets
    CHECK(ActionValidator::requiresTarget("Arrest"));
    CHECK(ActionValidator::requiresTarget("Sanction"));
    CHECK(ActionValidator::requiresTarget("Coup"));
    CHECK(ActionValidator::requiresTarget("Investigate"));
    CHECK(ActionValidator::requiresTarget("Block Arrest"));
    
    // Test actions that don't require targets
    CHECK_FALSE(ActionValidator::requiresTarget("Gather"));
    CHECK_FALSE(ActionValidator::requiresTarget("Tax"));
    CHECK_FALSE(ActionValidator::requiresTarget("Bribe"));
    CHECK_FALSE(ActionValidator::requiresTarget("Invest"));
}

TEST_CASE("Self-targeting prevention") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();
    
    // Give governor enough coins for all actions
    governor->add_coins(10);
    
    // Test that all actions properly prevent self-targeting (on governor's turn)
    CHECK_THROWS_AS(governor->arrest(*governor), IllegalTargetException);
    CHECK_THROWS_AS(governor->sanction(*governor), IllegalTargetException);
    CHECK_THROWS_AS(governor->coup(*governor), IllegalTargetException);
    
    // Test spy abilities - need to be spy's turn
    game->next_turn(); // Switch to spy's turn
    CHECK_THROWS_AS(spy->investigate(*spy), IllegalTargetException);
    CHECK_THROWS_AS(spy->block_arrest_ability(*spy), IllegalTargetException);
}

TEST_CASE("Role-specific abilities - Governor") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();
    
    // Governor gets 3 coins from tax instead of 2
    int initial_coins = governor->get_coins();
    governor->tax();
    CHECK(governor->get_coins() == initial_coins + 3);
    
    // Governor can block tax actions
    CHECK(governor->can_block("Tax"));
    CHECK(governor->can_block("tax")); // Case insensitive
    CHECK_FALSE(governor->can_block("Bribe"));
    CHECK_FALSE(governor->can_block("Coup"));
}

TEST_CASE("Role-specific abilities - Spy") {
    auto game = std::make_shared<Game>();
    auto spy = std::make_shared<Spy>(game, "Spy");
    auto governor = std::make_shared<Governor>(game, "Governor");
    
    game->add_player(spy);
    game->add_player(governor);
    game->start_game();
    
    // Spy can investigate other players
    spy->investigate(*governor);
    CHECK(spy->get_coins() == 0); // Investigation costs nothing
    
    // Spy can block arrest abilities
    spy->block_arrest_ability(*governor);
    CHECK(governor->is_arrest_blocked());
    
    // Governor should not be able to arrest while blocked
    governor->add_coins(5);
    game->next_turn(); // Switch to governor's turn
    CHECK_THROWS_AS(governor->arrest(*spy), IllegalMoveException);
}

TEST_CASE("Role-specific abilities - Baron") {
    auto game = std::make_shared<Game>();
    auto baron = std::make_shared<Baron>(game, "Baron");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(baron);
    game->add_player(spy);
    game->start_game();
    
    // Baron can invest: pay 3 coins to get 6 coins (net +3)
    baron->add_coins(3);
    int initial_coins = baron->get_coins();
    int initial_treasury = game->get_treasury();
    
    baron->invest();
    CHECK(baron->get_coins() == initial_coins + 3); // Net gain of 3
    CHECK(game->get_treasury() == initial_treasury - 3); // Treasury loses 3 net
    
    // Baron gets compensation when sanctioned - make sure we're on spy's turn
    spy->add_coins(4);
    CHECK(game->turn() == "Spy"); // Should be spy's turn after baron's investment
    
    int baron_coins_before = baron->get_coins();
    spy->sanction(*baron);
    CHECK(baron->get_coins() == baron_coins_before + 1); // Gets 1 compensation coin
    CHECK(baron->is_sanctioned());
}

TEST_CASE("Role-specific abilities - General") {
    auto game = std::make_shared<Game>();
    auto general = std::make_shared<General>(game, "General");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(general);
    game->add_player(spy);
    game->start_game();
    
    // General can block coup only when having 5+ coins
    CHECK_FALSE(general->can_block("Coup")); // Has 0 coins
    
    general->add_coins(5);
    CHECK(general->can_block("Coup")); // Now has 5 coins
    CHECK(general->can_block("coup")); // Case insensitive
    CHECK_FALSE(general->can_block("Tax"));
    CHECK_FALSE(general->can_block("Bribe"));
    
    // General has arrest immunity (no coin transfer)
    spy->add_coins(2);
    game->next_turn(); // Switch to spy's turn
    
    int spy_coins_before = spy->get_coins();
    int general_coins_before = general->get_coins();
    spy->arrest(*general);
    
    // No coins should be transferred due to General immunity
    CHECK(spy->get_coins() == spy_coins_before);
    CHECK(general->get_coins() == general_coins_before);
}

TEST_CASE("Role-specific abilities - Judge") {
    auto game = std::make_shared<Game>();
    auto judge = std::make_shared<Judge>(game, "Judge");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(judge);
    game->add_player(spy);
    game->start_game();
    
    // Judge can block bribe actions
    CHECK(judge->can_block("Bribe"));
    CHECK(judge->can_block("bribe")); // Case insensitive
    CHECK_FALSE(judge->can_block("Tax"));
    CHECK_FALSE(judge->can_block("Coup"));
    
    // Sanctioning a Judge costs 4 coins instead of 3
    spy->add_coins(4);
    game->next_turn(); // Switch to spy's turn
    
    int spy_coins_before = spy->get_coins();
    spy->sanction(*judge);
    CHECK(spy->get_coins() == spy_coins_before - 4); // Costs 4 instead of 3
}

TEST_CASE("Role-specific abilities - Merchant") {
    auto game = std::make_shared<Game>();
    auto merchant = std::make_shared<Merchant>(game, "Merchant");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(merchant);
    game->add_player(spy);
    game->start_game();
    
    // Merchant gets bonus coin at turn start if has 3+ coins
    merchant->add_coins(3);
    int coins_before = merchant->get_coins();
    merchant->on_turn_start();
    CHECK(merchant->get_coins() == coins_before + 1);
    
    // Merchant with less than 3 coins doesn't get bonus
    auto merchant2 = std::make_shared<Merchant>(game, "Merchant2");
    merchant2->add_coins(2);
    coins_before = merchant2->get_coins();
    merchant2->on_turn_start();
    CHECK(merchant2->get_coins() == coins_before); // No bonus
    
    // Test Merchant arrest behavior - avoid the bonus by testing with a merchant that has < 3 coins
    auto fresh_game = std::make_shared<Game>();
    auto fresh_merchant = std::make_shared<Merchant>(fresh_game, "FreshMerchant");
    auto fresh_spy = std::make_shared<Spy>(fresh_game, "FreshSpy");
    
    fresh_game->add_player(fresh_spy); // Add spy first so they start
    fresh_game->add_player(fresh_merchant);
    fresh_game->start_game();
    
    // Give coins - keep merchant below 3 to avoid bonus
    fresh_spy->add_coins(2);
    fresh_merchant->add_coins(2); // Only 2 coins, so no bonus
    
    int spy_coins_before = fresh_spy->get_coins();
    int merchant_coins_before = fresh_merchant->get_coins();
    int treasury_before = fresh_game->get_treasury();
    
    fresh_spy->arrest(*fresh_merchant);
    
    // Spy should not gain coins, merchant pays treasury (max 2 coins, but only has 2)
    CHECK(fresh_spy->get_coins() == spy_coins_before);
    CHECK(fresh_merchant->get_coins() == merchant_coins_before - 2); // Pays exactly 2 coins
    CHECK(fresh_game->get_treasury() == treasury_before + 2); // Treasury gains 2 coins
}

TEST_CASE("Status effects - Sanctions") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    auto baron = std::make_shared<Baron>(game, "Baron"); // Add a third player for coup target
    
    game->add_player(governor);
    game->add_player(spy);
    game->add_player(baron);
    game->start_game();
    
    // Start with governor's turn, then switch to spy to sanction
    CHECK(game->turn() == "Governor");
    governor->gather(); // Governor acts to advance turn (now has 1 coin)
    
    CHECK(game->turn() == "Spy");
    spy->add_coins(4);
    spy->sanction(*governor);
    CHECK(governor->is_sanctioned());
    
    // Skip baron's turn to get back to governor
    CHECK(game->turn() == "Baron");
    baron->gather();
    
    // Now it should be governor's turn and they are sanctioned
    CHECK(game->turn() == "Governor");
    CHECK_THROWS_AS(governor->gather(), IllegalMoveException);
    CHECK_THROWS_AS(governor->tax(), IllegalMoveException);
    
    // But they can still do other actions like coup
    governor->add_coins(7); // Governor now has 1 + 7 = 8 coins
    governor->coup(*baron); // Coup should work even when sanctioned
    CHECK_FALSE(baron->is_active()); // Baron should be eliminated
    
    // Now it should be spy's turn and sanctions should be cleared
    CHECK(game->turn() == "Spy");
    CHECK_FALSE(governor->is_sanctioned()); // Sanctions cleared at end of governor's turn
}

TEST_CASE("Treasury interactions") {
    auto game = std::make_shared<Game>();
    auto baron = std::make_shared<Baron>(game, "Baron");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(baron);
    game->add_player(spy);
    game->start_game();
    
    int initial_treasury = game->get_treasury();
    CHECK(initial_treasury == 50); // Default starting treasury
    
    // Adding and removing from treasury
    game->add_to_treasury(10);
    CHECK(game->get_treasury() == initial_treasury + 10);
    
    game->remove_from_treasury(5);
    CHECK(game->get_treasury() == initial_treasury + 5);
    
    // Can't remove more than treasury has
    CHECK_THROWS_AS(game->remove_from_treasury(1000), GameException);
    
    // Baron investment requires sufficient treasury
    game->remove_from_treasury(game->get_treasury() - 5); // Leave only 5 coins
    baron->add_coins(3);
    CHECK_THROWS_AS(baron->invest(), IllegalMoveException); // Not enough treasury for return
}

TEST_CASE("Action costs and validation") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    auto baron = std::make_shared<Baron>(game, "Baron");
    
    game->add_player(governor);
    game->add_player(spy);
    game->add_player(baron);
    game->start_game();
    
    // Test insufficient coins for various actions (on governor's turn)
    CHECK_THROWS_AS(governor->bribe(), NotEnoughCoinsException); // Costs 4
    CHECK_THROWS_AS(governor->sanction(*spy), NotEnoughCoinsException); // Costs 3
    CHECK_THROWS_AS(governor->coup(*spy), NotEnoughCoinsException); // Costs 7
    
    // Baron investment costs 3 (switch to baron's turn)
    game->next_turn(); // spy's turn
    game->next_turn(); // baron's turn
    CHECK_THROWS_AS(baron->invest(), NotEnoughCoinsException);
}

TEST_CASE("Mandatory coup rule") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    
    game->add_player(governor);
    game->add_player(spy);
    game->start_game();
    
    // Give governor 10 coins
    governor->add_coins(10);
    
    // With 10+ coins, most actions should be blocked except coup
    // The actual exception message is more specific than expected
    CHECK_THROWS_WITH(ActionValidator::validateActionExecution("Gather", governor), "Must perform coup when having 10 or more coins");
    CHECK_THROWS_WITH(ActionValidator::validateActionExecution("Tax", governor), "Must perform coup when having 10 or more coins");
    CHECK_THROWS_WITH(ActionValidator::validateActionExecution("Bribe", governor), "Must perform coup when having 10 or more coins");
    
    // Coup should still be allowed
    ActionValidator::validateActionExecution("Coup", governor, spy);
    
    // End Turn should also be allowed
    ActionValidator::validateActionExecution("End Turn", governor);
}

TEST_CASE("Complex game scenario") {
    auto game = std::make_shared<Game>();
    auto governor = std::make_shared<Governor>(game, "Governor");
    auto spy = std::make_shared<Spy>(game, "Spy");
    auto baron = std::make_shared<Baron>(game, "Baron");
    auto general = std::make_shared<General>(game, "General");
    
    game->add_player(governor);
    game->add_player(spy);
    game->add_player(baron);
    game->add_player(general);
    game->start_game();
    
    CHECK(count_active_players(game) == 4);
    CHECK_FALSE(game->is_game_over());
    
    // Round 1 - each player acts once
    CHECK(game->turn() == "Governor");
    governor->tax(); // Governor gets 3 coins
    
    CHECK(game->turn() == "Spy"); 
    spy->gather(); // Spy gets 1 coin
    
    CHECK(game->turn() == "Baron");
    baron->gather(); // Baron gets 1 coin
    
    CHECK(game->turn() == "General");
    general->gather(); // General gets 1 coin
    
    CHECK(governor->get_coins() == 3);
    CHECK(spy->get_coins() == 1);
    CHECK(baron->get_coins() == 1);
    CHECK(general->get_coins() == 1);
    
    // Round 2 - more actions
    CHECK(game->turn() == "Governor"); 
    governor->gather(); // Give governor another coin (now has 4)
    
    CHECK(game->turn() == "Spy");
    spy->investigate(*governor); // Spy investigates (no turn advancement for investigate)
    spy->gather(); // Spy also gathers to advance turn
    
    CHECK(game->turn() == "Baron");
    baron->add_coins(2); // Now has 3 total
    baron->invest(); // Spends 3, gets 6, net +3
    CHECK(baron->get_coins() == 6);
    
    // General gets enough coins to block coup
    CHECK(game->turn() == "General");
    general->add_coins(4); // Now has 5 total
    CHECK(general->can_block("Coup"));
    general->gather(); // End general's turn
    
    // Governor gets coup money and eliminates spy
    CHECK(game->turn() == "Governor");
    governor->add_coins(3); // Governor now has 4+3=7 total
    governor->coup(*spy);
    CHECK_FALSE(spy->is_active());
    CHECK(count_active_players(game) == 3); // 3 active players left
    
    // Next active turn - spy is skipped, so baron
    CHECK(game->turn() == "Baron");
    baron->add_coins(1); // Now has 7 total (baron has 6+1)
    baron->coup(*general);
    CHECK_FALSE(general->is_active());
    CHECK(count_active_players(game) == 2); // 2 active players left
    
    // Final coup - governor needs more coins
    CHECK(game->turn() == "Governor");
    governor->add_coins(7); // Give governor enough coins for coup
    governor->coup(*baron);
    CHECK_FALSE(baron->is_active());
    CHECK(count_active_players(game) == 1); // 1 active player left
    CHECK(game->is_game_over());
    CHECK(game->winner() == "Governor");
}

TEST_CASE("Edge cases and error handling") {
    // Test insufficient players
    auto game1 = std::make_shared<Game>();
    auto governor1 = std::make_shared<Governor>(game1, "Governor");
    game1->add_player(governor1);
    CHECK_THROWS(game1->start_game());
    
    // Test player limit separately to avoid interference
    auto game2 = std::make_shared<Game>();
    // Add exactly 6 players (the maximum)
    for (int i = 0; i < 6; i++) {
        auto player = std::make_shared<Governor>(game2, "Player" + std::to_string(i));
        game2->add_player(player);
    }
    // Try to add one more - should fail
    auto extra_player = std::make_shared<Governor>(game2, "Extra");
    CHECK_THROWS_AS(game2->add_player(extra_player), TooManyPlayersException);
    
    // Test turn management with fresh game
    auto game3 = std::make_shared<Game>();
    auto governor3 = std::make_shared<Governor>(game3, "Governor");
    auto spy3 = std::make_shared<Spy>(game3, "Spy");
    game3->add_player(governor3);
    game3->add_player(spy3);
    game3->start_game();
    
    CHECK(game3->turn() == "Governor");
    CHECK_THROWS_AS(spy3->gather(), NotYourTurnException);
    
    // Can't target inactive players
    governor3->add_coins(7);
    governor3->coup(*spy3);
    CHECK_FALSE(spy3->is_active());
    
    auto baron3 = std::make_shared<Baron>(game3, "Baron");
    game3->add_player(baron3);
    CHECK_THROWS_AS(governor3->arrest(*spy3), IllegalTargetException); // Spy is inactive
} 