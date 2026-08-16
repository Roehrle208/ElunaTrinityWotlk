#ifndef WOWHEAD_TALENT_ENGINE_H
#define WOWHEAD_TALENT_ENGINE_H

#include "Common.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Player.h"
#include "Item.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <map>
#include <vector>
#include <string>

// Struktur für ein einzelnes Talent in der korrekten Reihenfolge
struct TalentTemplate
{
    uint32 TalentID;      // Interne TalentID aus DBC
    uint32 SpellIDRank1;  // SpellID des ersten Rangs (zum Lookup)
    uint8 MaxRanks;       // Maximale Ränge dieses Talents
    uint8 Tier;           // Für Sortierung/Debug
    uint8 Column;         // Für Sortierung/Debug
};

// Struktur für einen ganzen Talentbaum (Spec)
struct SpecTemplate
{
    uint32 SpecID;        // TalentTabID
    uint32 OrderIndex; // Neu hinzufügen
    std::string Name;     // Name der Spec (z.B. "Feuer")
    std::vector<TalentTemplate> Talents; // Sortierte Talente
};

class WowheadTalentEngine
{
private:
    // Map: ClassID -> Liste der Specs (z.B. Mage -> [Feuer, Frost, Arkan])
    std::map<uint32, std::vector<SpecTemplate>> m_ClassTemplates;
    // Lookup Map für Verzauberungen: EnchantID -> SpellID (für die Anzeige im GearPlanner)
    std::unordered_map<uint32, uint32> m_enchantSpellMap; 

    // Privater Konstruktor für Singleton
    WowheadTalentEngine();

    // Initialisiert die Daten aus den DBCs
    void LoadFromDBCTalents();
    void LoadFromDBCEnchantments();

    // Generiert den Talent-Hash für einen Spieler (nur Talente, keine Glyphen)
    void BuildGenerateTalentHash(Player* player, std::string& talentHash);
    void BuildSlotsData(Player* player, rapidjson::Value& slots, rapidjson::Document::AllocatorType& allocator);

public:
    // Singleton Zugriff
    static WowheadTalentEngine& Instance();

    // Löscht Kopier-Konstruktor (Singleton Sicherheit)
    WowheadTalentEngine(const WowheadTalentEngine&) = delete;
    void operator=(const WowheadTalentEngine&) = delete;

    // --- Hauptfunktionen ---

    // Exportiert Talente eines Spielers als JSON String
    std::string ExportJson(Player* player);
};

#endif // WOWHEAD_TALENT_ENGINE_H   