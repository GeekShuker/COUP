# Coup Card Game

A C++ implementation of the Coup card game with a graphical user interface using SFML.

## Project Overview

This project implements the Coup card game, a strategic game for 2-6 players where each player takes on a unique role and competes to be the last player standing. The game features a complete GUI implementation using SFML for visual representation.

## Game Rules

### Basic Actions
- **Gather**: Take 1 coin from the treasury
- **Tax**: Take 2 coins from the treasury
- **Bribe**: Pay 4 coins to perform an additional action
- **Arrest**: Take 1 coin from another player
- **Sanction**: Pay 3 coins to prevent another player from using economic actions
- **Coup**: Pay 7 coins to eliminate another player

### Special Roles
- **Governor**: Takes 3 coins for tax, can block tax actions
- **Spy**: Can view other players' coins and block next turn arrest action
- **Baron**: Can invest 3 coins to get 6 coins, receives compensation for sanctions
- **General**: Can pay 5 coins to prevent coups, recovers coins from arrests
- **Judge**: Can block bribes, receives compensation for sanctions
- **Merchant**: Gets bonus coins, pays treasury instead of players for arrests

## Project Structure

```
├── include/
│   ├── Game.hpp         # Main game controller
│   ├── GUI.hpp          # Graphical user interface
│   ├── Player.hpp       # Base player class
│   ├── Roles.hpp        # Role-specific player classes
│   ├── ActionValidator.hpp # Action validation logic
│   └── Exceptions.hpp   # Custom exceptions
├── src/
│   ├── Game.cpp         # Game implementation
│   ├── GUI.cpp          # GUI implementation
│   ├── Player.cpp       # Player implementation
│   ├── Roles.cpp        # Role implementations
│   ├── ActionValidator.cpp # Action validation implementation
│   └── main.cpp         # Main entry point
├── tests/               # Unit tests
├── Makefile            # Build configuration
└── README.md           # This file
```

## Features

- Complete game logic implementation
- SFML-based graphical user interface
- Support for 2-6 players
- Role-specific abilities and actions
- Action blocking system
- Treasury management
- Turn-based gameplay
- Comprehensive error handling

## Building and Running

### Prerequisites
- C++ compiler with C++17 support
- SFML library
- Make

### Build Commands
```bash
make Main    # Build and run the game
make test    # Run unit tests
make valgrind # Check for memory leaks
make clean   # Clean build files
```

## Testing

The project includes comprehensive unit tests using the doctest framework. Tests cover:
- Game mechanics
- Player actions
- Role-specific abilities
- Error handling
- Edge cases

## Memory Management

The project uses smart pointers for memory management and has been tested with Valgrind to ensure there are no memory leaks.

## Author

Meir Shuker  
meirshuker159@gmail.com
