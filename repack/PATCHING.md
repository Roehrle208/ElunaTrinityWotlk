# 🢹 Patch-Anleitung für das BotEngine Repack

Diese Datei beschreibt, wie du deine Anpassungen (`core.patch` und `bot-engine.patch`) sauber verwaltest und auf einen frischen Upstream-Stand anwendest.

---

## ✅ Voraussetzungen

* Du hast [ElunaTrinityWotlk](https://github.com/ElunaLuaEngine/ElunaTrinityWotlk) geforkt und lokal geklont
* Du arbeitest mit folgenden Branches:

  * `master` – aktueller Upstream (niemals modifizieren)
  * `feature/core-adjustments` – Core-Änderungen
  * `feature/bot-engine` – BotEngine-Logik (baut auf core-adjustments auf)

---

## 📆 Patch-Dateien

| Datei                             | Inhalt                                                          |
| --------------------------------- | --------------------------------------------------------------- |
| `repack/patches/core.patch`       | Alle Änderungen am TrinityCore-Core (z. B. World.cpp, Player.h) |
| `repack/patches/bot-engine.patch` | Alle Dateien unter `src/server/scripts/custom/BotEngine`        |

---

## 🦪 Patch-Anwendung (z. B. nach Upstream-Merge)

```bash
# 1. Neuen Branch vom Upstream-Stand erstellen
git checkout -b integration
git fetch upstream
git reset --hard upstream/master

# 2. Core-Änderungen anwenden
git apply repack/patches/core.patch

# 3. BotEngine-Code anwenden
git apply repack/patches/bot-engine.patch

# 4. Kompilieren
make -j$(nproc)
```

---

## 🔄 Patches erzeugen (manuell)

```bash
# Core-Patch erzeugen
git diff master...feature/core-adjustments > repack/patches/core.patch

# BotEngine-Patch erzeugen (vergleicht gegen core-adjustments)
git diff feature/core-adjustments...feature/bot-engine > repack/patches/bot-engine.patch
```

---

## 🧼 Patches entfernen

```bash
# Core rückgängig machen
git apply -R repack/patches/core.patch

# BotEngine rückgängig machen
git apply -R repack/patches/bot-engine.patch
```

---

## 💬 Automatisierung

Für schnellere Operationen gibt es zwei Patch-Helper:

| Datei                     | Betriebssystem      |
| ------------------------- | ------------------- |
| `repack/patch-helper.sh`  | Linux / WSL / macOS |
| `repack/patch-helper.bat` | Windows CMD         |

Beide erzeugen, testen und wenden die Patch-Dateien an – und loggen in:

```
repack/patch.log
```

---

## 🛠️ Tool: `patch-helper.sh` (Linux/macOS/WSL)

```bash
chmod +x repack/patch-helper.sh

# Patches erzeugen
./repack/patch-helper.sh make

# Patches anwenden
./repack/patch-helper.sh apply

# Patches entfernen
./repack/patch-helper.sh clean

# Nur testen (Dry Run)
./repack/patch-helper.sh apply --dry-run
```

---

## 🛠️ Tool: `patch-helper.bat` (Windows CMD)

```bat
cd repack

.\patch-helper.bat make
.\patch-helper.bat apply
.\patch-helper.bat clean

:: Nur testen (Dry Run)
.\patch-helper.bat apply --dry-run
```

---

## ⚠️ Konflikte bei Upstream-Änderungen

Wenn sich der Upstream (TrinityCore/Eluna) verändert hat:

1. `master` updaten:

   ```bash
   git fetch upstream
   git reset --hard upstream/master
   ```

2. `feature/core-adjustments` neu auf `master` rebasen:

   ```bash
   git checkout feature/core-adjustments
   git rebase master
   ```

3. neuen `core.patch` erzeugen

4. `feature/bot-engine` auf neuen `core-adjustments` rebasen:

   ```bash
   git checkout feature/bot-engine
   git rebase feature/core-adjustments
   ```

5. neuen `bot-engine.patch` erzeugen

---

## 💡 Best Practices

* Halte `core.patch` so klein wie möglich
* Lege deinen kompletten Code nur unter:

  ```
  src/server/scripts/custom/BotEngine/
  ```
* Niemals Dateien im Root verändern (z. B. .gitignore, README.md)
* Alle Dokumentation, Patches und Tools **nur unter ****repack/**
* Nutze `patch-helper.sh` oder `.bat`, um den Workflow zu vereinfachen
