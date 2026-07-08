const API_BASE = "";

let sessionId = localStorage.getItem("dungeonSessionId") || "";

const hpBar = document.getElementById("hp-bar");
const hpText = document.getElementById("hp-text");
const weaponText = document.getElementById("weapon-text");
const inventoryText = document.getElementById("inventory-text");
const roomTitle = document.getElementById("room-title");
const roomNarrative = document.getElementById("room-narrative");
const roomMessage = document.getElementById("room-message");
const choicesPanel = document.getElementById("choices");
const primaryAction = document.getElementById("primary-action");
const gameShell = document.querySelector(".game-shell");

async function apiRequest(path, method = "GET", body = null) {
  const headers = {
    "Content-Type": "application/json",
  };

  if (sessionId) {
    headers["X-Session-Id"] = sessionId;
  }

  const response = await fetch(`${API_BASE}${path}`, {
    method,
    headers,
    body: body ? JSON.stringify(body) : null,
  });

  const nextSession = response.headers.get("X-Session-Id");
  if (nextSession) {
    sessionId = nextSession;
    localStorage.setItem("dungeonSessionId", sessionId);
  }

  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || "Request failed");
  }

  return data;
}

function updateStatus(player) {
  const hp = player.hp;
  const maxHp = 100;
  const percent = Math.max(0, Math.min(100, (hp / maxHp) * 100));

  hpBar.style.width = `${percent}%`;
  hpBar.style.background =
    percent <= 25
      ? "linear-gradient(90deg, var(--hp-low), #e74c3c)"
      : "linear-gradient(90deg, var(--hp-full), #58d68d)";

  hpText.textContent = `${hp} / ${maxHp}`;
  weaponText.textContent = player.weapon;
  inventoryText.textContent = player.inventory.join(", ");
}

function clearChoices() {
  choicesPanel.innerHTML = "";
}

function renderChoices(choices, onSelect) {
  clearChoices();

  choices.forEach((choice) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "choice-btn";
    button.textContent = `${choice.id}. ${choice.text}`;
    button.addEventListener("click", () => onSelect(choice.id));
    choicesPanel.appendChild(button);
  });
}

function setShellState(state) {
  gameShell.classList.remove("ended-victory", "ended-defeat");
  if (state === "victory") {
    gameShell.classList.add("ended-victory");
  } else if (state === "defeat") {
    gameShell.classList.add("ended-defeat");
  }
}

function renderGame(state) {
  updateStatus(state.player);

  roomTitle.textContent = state.title || "DUNGEON OF SHADOWS";
  roomNarrative.textContent = state.narrative || "";

  if (state.message) {
    roomMessage.textContent = state.message;
    roomMessage.classList.remove("hidden");
  } else {
    roomMessage.textContent = "";
    roomMessage.classList.add("hidden");
  }

  clearChoices();
  primaryAction.classList.remove("hidden");

  if (state.phase === "intro") {
    setShellState("");
    primaryAction.textContent = "BEGIN QUEST";
    primaryAction.onclick = async () => {
      primaryAction.disabled = true;
      try {
        const next = await apiRequest("/api/start", "POST");
        renderGame(next);
      } catch (error) {
        alert(error.message);
      } finally {
        primaryAction.disabled = false;
      }
    };
    return;
  }

  if (state.phase === "playing") {
    setShellState("");
    primaryAction.classList.add("hidden");
    renderChoices(state.choices, async (choiceId) => {
      clearChoices();
      try {
        const next = await apiRequest("/api/choice", "POST", { choice: choiceId });
        renderGame(next);
      } catch (error) {
        alert(error.message);
        renderGame(state);
      }
    });
    return;
  }

  if (state.phase === "result") {
    setShellState("");
    primaryAction.textContent = "CONTINUE";
    primaryAction.onclick = async () => {
      primaryAction.disabled = true;
      try {
        const next = await apiRequest("/api/continue", "POST");
        renderGame(next);
      } catch (error) {
        alert(error.message);
      } finally {
        primaryAction.disabled = false;
      }
    };
    return;
  }

  if (state.phase === "ended") {
    setShellState(state.ending);
    primaryAction.textContent = "PLAY AGAIN";
    primaryAction.onclick = async () => {
      primaryAction.disabled = true;
      try {
        const next = await apiRequest("/api/new", "POST");
        renderGame(next);
      } catch (error) {
        alert(error.message);
      } finally {
        primaryAction.disabled = false;
      }
    };
  }
}

async function bootGame() {
  try {
    if (!sessionId) {
      const state = await apiRequest("/api/new", "POST");
      renderGame(state);
      return;
    }

    const state = await apiRequest("/api/state", "GET");
    renderGame(state);
  } catch (error) {
    sessionId = "";
    localStorage.removeItem("dungeonSessionId");
    const state = await apiRequest("/api/new", "POST");
    renderGame(state);
  }
}

bootGame();
