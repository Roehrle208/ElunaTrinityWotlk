# 🤖 BotEngine Repack – Übersicht

Dieses Repack erweitert Eluna/TrinityCore um eine eigene Bot-Engine.

## 🔍 Projektaufbau

| Ordner / Datei               | Beschreibung |
|-----------------------------|--------------|
| `repack/README.md`          | Diese Datei. Überblick & Struktur |
| `repack/PATCHING.md`        | Anleitung zum Anwenden von Patches |
| `repack/TODO.md`            | Aufgabenliste und Ideen |
| `repack/patches/core.patch` | Core-Eingriffe in TrinityCore/Eluna |
| `repack/patches/bot-engine.patch` | BotEngine-spezifische Ergänzungen |

---

## 💡 Prinzip

- **Eigenständige Engine:** Bot-Logik liegt unter `src/server/scripts/custom/BotEngine/`
- **Core minimal verändern:** Nur was nötig ist (z. B. Worldmanager, Player etc.)
- **Upstream-freundlich:** Kein Vermischen mit Upstream-Dateien – alle Patches und Dokus liegen sauber im `repack/`-Ordner

---

## 📄 Wichtige Dokumente

👉 Lies `PATCHING.md`, wenn du das Repack anwenden willst  
🛠 Siehe `TODO.md`, um den aktuellen Entwicklungsstand zu prüfen oder weiterzuarbeiten

---

## 📌 Nächste Schritte

1. Patches anwenden → siehe [`PATCHING.md`](./PATCHING.md)
2. Code erweitern oder testen
3. Tasks im [`TODO.md`](./TODO.md) ergänzen oder abhaken
