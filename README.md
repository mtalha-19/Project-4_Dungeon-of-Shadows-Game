# Dungeon of Shadows - Text-Based RPG

A lightweight, interactive text-based Adventure RPG game written in C++ and built using Cursor AI. The project follows Object-Oriented Programming (OOP) principles to manage player states, inventory mechanics, dynamic choices, and basic input validation to prevent crashes.

## 🚀 Features
* **Player State Tracking:** Dynamically manages Health Points (HP), current weapons, and collected inventory items.
* **Interactive Narrative:** A 3-room branching storyline where user choices completely alter the player's status and outcome.
* **Robust Input Validation:** Built-in error handling to gracefully handle incorrect or invalid menu selections.
* **Clean Code Architecture:** Structured using an object-oriented approach with dedicated methods for player actions (`takeDamage`, `heal`, `printStatus`).

---

## 🛠️ Tech Stack & Tools
* **Language:** C++
* **Compiler:** GCC / G++
* **Development Environment:** Cursor AI (Composer 2.5)

---

## 💻 How to Compile and Run

To play the game directly from your terminal, make sure you have a C++ compiler installed, then run the following commands:

```bash
# Clone the repository
git clone [https://github.com/Malik_Talha /Project-4_Dungeon-of-Shadows-Game
.git](https://github.com/mtalha19/Project-4_Dungeon-of-Shadows-Game
.git)
cd Project-4_Dungeon-of-Shadows-Game


# Compile the source code
g++ main.cpp -o adventure

# Execute the game
./adventure
