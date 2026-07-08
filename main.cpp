#include <iostream>
#include <limits>
#include <string>

#include "game.hpp"

int readChoice(int minChoice, int maxChoice) {
    int choice = 0;
    while (true) {
        std::cout << "Enter your choice (" << minChoice << "-" << maxChoice << "): ";
        if (std::cin >> choice) {
            if (choice >= minChoice && choice <= maxChoice) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return choice;
            }
            std::cout << "Invalid choice. Please enter a number between "
                      << minChoice << " and " << maxChoice << ".\n";
        } else {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void pauseAndContinue() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void printStatus(const Player& player) {
    std::cout << "\n========================================\n";
    std::cout << "  HP: " << player.hp << " / 100\n";
    std::cout << "  Weapon: " << player.weapon << "\n";
    std::cout << "  Inventory: ";
    for (size_t i = 0; i < player.inventory.size(); ++i) {
        std::cout << player.inventory[i];
        if (i + 1 < player.inventory.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "\n========================================\n\n";
}

void printRoom(const GameView& view) {
    std::cout << "=== " << view.title << " ===\n";
    std::cout << view.narrative << "\n\n";
    printStatus(view.player);
    std::cout << "What do you do?\n";
    for (const auto& choice : view.choices) {
        std::cout << "  " << choice.id << ". " << choice.text << "\n";
    }
}

void printEnding(const GameView& view) {
    std::cout << "\n========================================\n";
    if (view.ending == "victory") {
        std::cout << "         *** VICTORY! ***\n";
        std::cout << view.narrative << "\n";
    } else {
        std::cout << "         *** DEFEAT ***\n";
        std::cout << view.narrative << "\n";
    }
    std::cout << "========================================\n";
    printStatus(view.player);
}

int main() {
    DungeonGame game;
    game.reset();

    GameView view = game.getView();
    std::cout << "========================================\n";
    std::cout << "     DUNGEON OF SHADOWS - Adventure RPG\n";
    std::cout << "========================================\n\n";
    std::cout << view.narrative << "\n";
    pauseAndContinue();

    game.start();

    while (true) {
        view = game.getView();

        if (view.phase == "ended") {
            printEnding(view);
            break;
        }

        if (view.phase == "result") {
            std::cout << "\n" << view.message << "\n";
            pauseAndContinue();
            game.continueGame();
            continue;
        }

        printRoom(view);
        int choice = readChoice(1, 3);
        game.makeChoice(choice);

        view = game.getView();
        if (!view.message.empty()) {
            std::cout << "\n" << view.message << "\n";
        }

        if (view.phase == "ended") {
            pauseAndContinue();
            printEnding(view);
            break;
        }

        if (view.phase == "result") {
            pauseAndContinue();
            game.continueGame();
        }
    }

    return 0;
}
