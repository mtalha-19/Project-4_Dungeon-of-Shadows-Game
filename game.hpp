#pragma once

#include <sstream>
#include <string>
#include <vector>

struct Player {
    int hp = 100;
    std::vector<std::string> inventory;
    std::string weapon = "Rusty Sword";

    Player() {
        inventory.push_back("Rusty Sword");
    }

    void takeDamage(int amount) {
        hp -= amount;
        if (hp < 0) {
            hp = 0;
        }
    }

    void heal(int amount) {
        hp += amount;
        if (hp > 100) {
            hp = 100;
        }
    }

    void addItem(const std::string& item) {
        inventory.push_back(item);
    }

    void setWeapon(const std::string& newWeapon) {
        weapon = newWeapon;
    }

    bool isAlive() const {
        return hp > 0;
    }
};

struct GameChoice {
    int id;
    std::string text;
};

struct GameView {
    std::string phase;
    int room = 0;
    std::string title;
    std::string narrative;
    std::vector<GameChoice> choices;
    Player player;
    std::string message;
    std::string ending;
    bool canContinue = false;
};

inline std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += ch;
                break;
        }
    }
    return escaped;
}

inline std::string gameViewToJson(const GameView& view) {
    std::ostringstream json;
    json << "{";
    json << "\"phase\":\"" << jsonEscape(view.phase) << "\",";
    json << "\"room\":" << view.room << ",";
    json << "\"title\":\"" << jsonEscape(view.title) << "\",";
    json << "\"narrative\":\"" << jsonEscape(view.narrative) << "\",";
    json << "\"message\":\"" << jsonEscape(view.message) << "\",";
    json << "\"ending\":\"" << jsonEscape(view.ending) << "\",";
    json << "\"canContinue\":" << (view.canContinue ? "true" : "false") << ",";
    json << "\"player\":{";
    json << "\"hp\":" << view.player.hp << ",";
    json << "\"weapon\":\"" << jsonEscape(view.player.weapon) << "\",";
    json << "\"inventory\":[";
    for (size_t i = 0; i < view.player.inventory.size(); ++i) {
        json << "\"" << jsonEscape(view.player.inventory[i]) << "\"";
        if (i + 1 < view.player.inventory.size()) {
            json << ",";
        }
    }
    json << "]},";
    json << "\"choices\":[";
    for (size_t i = 0; i < view.choices.size(); ++i) {
        json << "{\"id\":" << view.choices[i].id
             << ",\"text\":\"" << jsonEscape(view.choices[i].text) << "\"}";
        if (i + 1 < view.choices.size()) {
            json << ",";
        }
    }
    json << "]}";
    return json.str();
}

class DungeonGame {
public:
    void reset() {
        player_ = Player();
        room_ = 0;
        phase_ = "intro";
        message_.clear();
        ending_.clear();
    }

    GameView getView() const {
        GameView view;
        view.phase = phase_;
        view.room = room_;
        view.player = player_;
        view.message = message_;
        view.ending = ending_;
        view.canContinue = phase_ == "result";

        if (phase_ == "intro") {
            view.title = "DUNGEON OF SHADOWS";
            view.narrative =
                "Legends speak of a treasure beyond the iron door.\n"
                "You descend with nothing but courage and a rusty sword.";
            return view;
        }

        if (phase_ == "ended") {
            view.title = ending_ == "victory" ? "VICTORY!" : "DEFEAT";
            view.narrative = ending_ == "victory"
                ? "You escape the dungeon alive!"
                : "Your adventure ends in the dark...";
            return view;
        }

        populateRoom(view);
        return view;
    }

    bool start() {
        if (phase_ != "intro") {
            return false;
        }
        room_ = 1;
        phase_ = "playing";
        message_.clear();
        ending_.clear();
        return true;
    }

    bool makeChoice(int choice) {
        if (phase_ != "playing" || choice < 1 || choice > 3) {
            return false;
        }

        message_.clear();
        ending_.clear();

        switch (room_) {
            case 1:
                applyRoomOne(choice);
                break;
            case 2:
                applyRoomTwo(choice);
                break;
            case 3:
                applyRoomThree(choice);
                break;
            default:
                return false;
        }

        if (!player_.isAlive()) {
            phase_ = "ended";
            ending_ = "defeat";
            return true;
        }

        if (room_ >= 3) {
            phase_ = "ended";
            ending_ = "victory";
            return true;
        }

        phase_ = "result";
        return true;
    }

    bool continueGame() {
        if (phase_ != "result") {
            return false;
        }
        ++room_;
        phase_ = "playing";
        message_.clear();
        return true;
    }

private:
    Player player_;
    int room_ = 0;
    std::string phase_ = "intro";
    std::string message_;
    std::string ending_;

    void populateRoom(GameView& view) const {
        switch (room_) {
            case 1:
                view.title = "ROOM 1: THE DUNGEON ENTRANCE";
                view.narrative =
                    "You stand before a crumbling stone archway. Cold air drifts out "
                    "from the darkness below. A flickering torch hangs on the wall, "
                    "and you hear something skittering in the shadows.";
                view.choices = {
                    {1, "Grab the torch and march in boldly (-5 HP, gain Torch)"},
                    {2, "Sneak in quietly, hugging the wall (-10 HP)"},
                    {3, "Search the entrance for supplies (+5 HP, gain Health Potion)"},
                };
                break;
            case 2:
                view.title = "ROOM 2: THE TRAPPED CORRIDOR";
                view.narrative =
                    "The corridor narrows. Ahead, a pit yawns open, and to your right "
                    "a rusted lever is mounted on the wall. Scraps of old armor lie "
                    "scattered on the floor.";
                view.choices = {
                    {1, "Jump across the pit (-15 HP)"},
                    {2, "Pull the lever to raise a bridge (-5 HP, gain Iron Shield)"},
                    {3, "Examine the armor and equip a better blade (+0 HP, upgrade weapon)"},
                };
                break;
            case 3:
                view.title = "ROOM 3: THE GUARDIAN CHAMBER";
                view.narrative =
                    "A massive iron door blocks the exit. Before it stands the Dungeon "
                    "Guardian — a hulking knight wreathed in shadow. The only way out "
                    "is through him, or perhaps through the door itself.";
                view.choices = {
                    {1, "Charge the Guardian head-on (-25 HP)"},
                    {2, "Try to pick the lock on the iron door (-10 HP)"},
                    {3, "Use your wits: distract him (+0 HP, gain Master Key)"},
                };
                break;
            default:
                break;
        }
    }

    void applyRoomOne(int choice) {
        switch (choice) {
            case 1:
                message_ =
                    "You seize the torch and stride forward. Embers singe your arm, "
                    "but the light reveals a safer path ahead.";
                player_.takeDamage(5);
                player_.addItem("Torch");
                break;
            case 2:
                message_ =
                    "You creep through the entrance. A hidden tripwire snaps a blade "
                    "across your leg before you can react.";
                player_.takeDamage(10);
                break;
            case 3:
                message_ =
                    "Behind a loose stone you find a dusty health potion and drink it. "
                    "Warmth spreads through your body.";
                player_.heal(5);
                player_.addItem("Health Potion");
                break;
            default:
                break;
        }
    }

    void applyRoomTwo(int choice) {
        switch (choice) {
            case 1:
                message_ =
                    "You sprint and leap! Your foot catches the far edge and you "
                    "tumble forward, bruised but alive.";
                player_.takeDamage(15);
                break;
            case 2:
                message_ =
                    "The lever groans and a stone bridge grinds into place. A loose "
                    "gear clips your shoulder as you cross.";
                player_.takeDamage(5);
                player_.addItem("Iron Shield");
                break;
            case 3:
                message_ =
                    "You pry a sharp shortsword from a skeleton's grip. It feels "
                    "far sturdier than your rusty blade.";
                player_.setWeapon("Shortsword");
                player_.addItem("Shortsword");
                break;
            default:
                break;
        }
    }

    void applyRoomThree(int choice) {
        switch (choice) {
            case 1:
                message_ =
                    "You raise your " + player_.weapon +
                    " and attack! The Guardian strikes back with crushing force, "
                    "but you land the final blow. The Guardian falls. "
                    "The iron door creaks open on its own.";
                player_.takeDamage(25);
                break;
            case 2:
                message_ =
                    "You work the lock with trembling hands. A poison dart shoots "
                    "from the door frame, but the lock clicks open! "
                    "You slip through the door into daylight.";
                player_.takeDamage(10);
                break;
            case 3:
                message_ =
                    "You shout and hurl a loose stone as a distraction, then dash "
                    "for the key ring on the Guardian's belt. You snatch the Master Key "
                    "and unlock the door before he can react!";
                player_.addItem("Master Key");
                break;
            default:
                break;
        }
    }
};
