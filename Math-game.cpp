#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <cctype>
#include <limits>

using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void waitForEnter() {
    cout << "\nPress Enter to continue...";
    clearInput();
    cin.get();
}

// ── Level 1: Math Riddle ──────────────────────────────────────────
bool level1_math() {
    cout << "\n=== LEVEL 1: Math Riddle ===\n";
    cout << "Solve: 15 + 27 = ?\n";
    cout << "Answer: ";
    int ans;
    cin >> ans;
    if (ans == 42) {
        cout << "Correct!\n";
        return true;
    }
    cout << "Wrong! The answer was 42.\n";
    return false;
}

// ── Level 2: Number Sequence ──────────────────────────────────────
bool level2_sequence() {
    cout << "\n=== LEVEL 2: Number Sequence ===\n";
    cout << "Find the next number: 2, 4, 8, 16, ?\n";
    cout << "Answer: ";
    int ans;
    cin >> ans;
    if (ans == 32) {
        cout << "Correct! (Each number doubles.)\n";
        return true;
    }
    cout << "Wrong! The answer was 32.\n";
    return false;
}

// ── Level 3: Word Unscramble ───────────────────────────────────────
bool level3_anagram() {
    cout << "\n=== LEVEL 3: Word Unscramble ===\n";
    string scrambled = "LEPUZ";
    string answer    = "PUZZLE";
    cout << "Unscramble: " << scrambled << "\n";
    cout << "Answer: ";
    string guess;
    cin >> guess;
    transform(guess.begin(), guess.end(), guess.begin(), ::toupper);
    if (guess == answer) {
        cout << "Correct!\n";
        return true;
    }
    cout << "Wrong! The word was PUZZLE.\n";
    return false;
}

// ── Level 4: Code Breaker ─────────────────────────────────────────
bool level4_codebreaker() {
    cout << "\n=== LEVEL 4: Code Breaker ===\n";
    cout << "Guess the 3-digit secret code (digits 1-6).\n";
    cout << "You get hints: +N = correct digit in correct place.\n";
    cout << "               -N = correct digit, wrong place.\n";
    cout << "You have 6 attempts.\n\n";

    mt19937 rng(static_cast<unsigned>(time(nullptr)));
    vector<int> secret(3);
    for (int& d : secret) d = uniform_int_distribution<>(1, 6)(rng);

    for (int attempt = 1; attempt <= 6; ++attempt) {
        cout << "Attempt " << attempt << "/6 — Enter 3 digits: ";
        string guess;
        cin >> guess;
        if (guess.size() != 3 || !all_of(guess.begin(), guess.end(), ::isdigit)) {
            cout << "Invalid input. Use exactly 3 digits.\n";
            --attempt;
            continue;
        }

        int exact = 0, misplaced = 0;
        vector<bool> usedS(3, false), usedG(3, false);

        for (int i = 0; i < 3; ++i)
            if (guess[i] - '0' == secret[i]) { ++exact; usedS[i] = usedG[i] = true; }

        for (int i = 0; i < 3; ++i) {
            if (usedG[i]) continue;
            for (int j = 0; j < 3; ++j) {
                if (!usedS[j] && guess[i] - '0' == secret[j]) {
                    ++misplaced; usedS[j] = usedG[i] = true; break;
                }
            }
        }

        cout << "Hint: +" << exact << " -" << misplaced << "\n";

        if (exact == 3) {
            cout << "You cracked the code!\n";
            return true;
        }
    }
    cout << "Out of attempts! Code was "
         << secret[0] << secret[1] << secret[2] << ".\n";
    return false;
}

// ── Level 5: ASCII Maze ───────────────────────────────────────────
bool level5_maze() {
    cout << "\n=== LEVEL 5: The Maze ===\n";
    cout << "Reach 'E' from 'S'. Walls are #. Use W/A/S/D to move.\n\n";

    // 7x7 maze: S at (0,0), E at (6,6)
    vector<string> maze = {
        "S...#..",
        ".###.#.",
        "...#...",
        "#.#.###",
        "...#...",
        ".###.#.",
        "..#...E"
    };

    int row = 0, col = 0;
    int endR = 6, endC = 6;
    int moves = 25;

    auto printMaze = [&]() {
        cout << "\n";
        for (int r = 0; r < (int)maze.size(); ++r) {
            for (int c = 0; c < (int)maze[r].size(); ++c) {
                if (r == row && c == col) cout << '@';
                else cout << maze[r][c];
            }
            cout << "\n";
        }
        cout << "Moves left: " << moves << "\n";
    };

    printMaze();

    while (moves > 0) {
        cout << "Move (W/A/S/D): ";
        char dir;
        cin >> dir;
        dir = toupper(dir);

        int nr = row, nc = col;
        if      (dir == 'W') --nr;
        else if (dir == 'S') ++nr;
        else if (dir == 'A') --nc;
        else if (dir == 'D') ++nc;
        else { cout << "Use W, A, S, or D.\n"; continue; }

        if (nr < 0 || nr >= (int)maze.size() ||
            nc < 0 || nc >= (int)maze[nr].size() ||
            maze[nr][nc] == '#') {
            cout << "Can't move there!\n";
            --moves;
            continue;
        }

        row = nr; col = nc;
        --moves;
        printMaze();

        if (row == endR && col == endC) {
            cout << "You escaped the maze!\n";
            return true;
        }
    }
    cout << "Out of moves!\n";
    return false;
}

// ── Main ──────────────────────────────────────────────────────────
using LevelFunc = bool(*)();

int main() {
    cout << "╔══════════════════════════════════╗\n";
    cout << "║         M I N D   Q U E S T      ║\n";
    cout << "║   A Console Puzzle Adventure     ║\n";
    cout << "╚══════════════════════════════════╝\n";
    cout << "\nComplete all 5 levels to win!\n";
    cout << "You get 2 lives — fail twice and it's game over.\n";

    waitForEnter();

    vector<LevelFunc> levels = {
        level1_math,
        level2_sequence,
        level3_anagram,
        level4_codebreaker,
        level5_maze
    };

    int lives = 2;
    int current = 0;

    while (current < (int)levels.size()) {
        cout << "\n────────────────────────────────\n";
        cout << "  Level " << (current + 1) << " of " << levels.size()
             << "  |  Lives: " << lives << "\n";
        cout << "────────────────────────────────\n";

        if (levels[current]()) {
            ++current;
            if (current < (int)levels.size())
                waitForEnter();
        } else {
            --lives;
            if (lives <= 0) {
                cout << "\n*** GAME OVER ***\n";
                return 0;
            }
            cout << "Try again! (" << lives << " life/lives left)\n";
            waitForEnter();
        }
    }

    cout << "\n╔══════════════════════════════════╗\n";
    cout << "║     CONGRATULATIONS! YOU WIN!    ║\n";
    cout << "║   You completed all 5 levels!      ║\n";
    cout << "╚══════════════════════════════════╝\n";
    return 0;
} 