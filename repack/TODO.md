
---

## 📝 `repack/TODO.md` → **Nur Aufgaben & Ideen**

```md
# 📝 TODO – BotEngine Entwicklung

---

## 🎯 Aktuell geplante Tasks

🔲 `.bot create <name>` – GM-Command zum Erzeugen eines Bots  
🔲 `World.cpp`: Einstiegspunkt für Bot-Login vorbereiten  
🔲 `Player.h`: Kennzeichnung für Bot-Spieler (Flag?)  
🔲 `BotManager`: Dummy-Implementierung zum Testen

---

## 🔁 Regelmäßige Tasks

🔲 Nach jedem Upstream-Merge: `core.patch` neu erzeugen  
🔲 Nach Feature-Fortschritt: `bot-engine.patch` neu erzeugen  
🔲 Patch-Anwendung regelmäßig testen (auch bei frischem Clone)  
🔲 Doku (README/PATCHING) aktuell halten

---

## 💡 Ideen / Visionen

🔲 `.bot list` – aktive Bots anzeigen  
🔲 `.bot remove <name>` – Bot löschen  
🔲 Bewegungslogik oder Pathfinding  
🔲 PvE-Rollen (Tank, Heal, DPS)  
🔲 PvP-Testmodi (Battleground-Simulation)

---

## 🧪 Testumgebung

🔲 Eigener Testrealm (Debug-Build, GM-Mode)  
🔲 Logging aktivieren (z. B. Bot-Init, Bot-Aktionen)  
🔲 Automatischer Bot-Login beim Start

---

## 🧩 Offene Fragen

🔲 Müssen Bots echte Accounts sein? Oder Dummy-Datenbankeinträge?  
🔲 Trigger via Lua oder rein in C++?
🔲 Wie viele Bots gleichzeitig stabil möglich?

---

*Diese Datei wird laufend erweitert und aktualisiert.*
