#include "WowheadTalentEngine.h"
#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include <algorithm>
#include <sstream>

// Hilfsstruktur für das initiale Sortieren
struct RawTalent
{
    uint32 TalentID;
    uint32 SpellIDRank1;
    uint8 MaxRanks;
    uint8 Tier;
    uint8 Column;
};

WowheadTalentEngine::WowheadTalentEngine()
{
    LoadFromDBCTalents();
    LoadFromDBCEnchantments();
}

WowheadTalentEngine& WowheadTalentEngine::Instance()
{
    static WowheadTalentEngine instance;
    return instance;
}

void WowheadTalentEngine::LoadFromDBCEnchantments()
{
    m_enchantSpellMap.clear();

    /**
     * Verzauberungen von Berufen sammeln
     */    
    // temporäre liste für alle EnchantID die der SkillLine 333 zugeordnet sind
    std::vector<uint32> enchantIDs;
    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        SkillLineAbilityEntry const* entry = sSkillLineAbilityStore.LookupEntry(i);
        if (!entry) continue;

        // auf RequiredSkillID = 333 filtern
        if (entry->SkillLine == 333) {            
            enchantIDs.push_back(entry->Spell);
        }
    }

    // Jetzt für jede EnchantID die SpellID aus der Spell.dbc holen
    for (uint32 enchantID : enchantIDs)
    {
        SpellEntry const* pEnchant = sSpellStore.LookupEntry(enchantID);
        if (!pEnchant) continue;

        uint32 spellID = pEnchant->EffectMiscValue[0];
        if (spellID > 0 && enchantID > 0)
        {
            m_enchantSpellMap[spellID] = enchantID;
        }
    }

    /**
     * Verzauberungen die durch Items entstehen sammeln
     */
    for(uint32 i = 0; i < sSpellStore.GetNumRows(); ++i)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(i);
        if (!entry) continue;
        
        // es muss mind. ein Effekt SPELL_EFFECT_ENCHANT_ITEM vorhanden sein, sonst ist es keine Item-Verzauberung
        bool hasEnchantEffect = false;
        for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (entry->Effect[j] == SPELL_EFFECT_ENCHANT_ITEM)
            {
                hasEnchantEffect = true;
                break;
            }
        }
        if (!hasEnchantEffect) continue;

        // EffectMiscValue[0] muss gesetzt sein
        if (entry->EffectMiscValue[0] == 0) continue;

        // EquippedItemClass muss ITEM_CLASS_ARMOR sein
        if (entry->EquippedItemClass != ITEM_CLASS_ARMOR) continue;

        uint32 spellID = entry->EffectMiscValue[0];
        uint32 enchantID = entry->ID;
        if (enchantID > 0 && spellID > 0)
        {
            m_enchantSpellMap[spellID] = enchantID;
        }
    }
}

void WowheadTalentEngine::LoadFromDBCTalents()
{
    TC_LOG_INFO("custom", "[GearPlanner] Lade Talent-Daten aus DBCs....");

    struct TabInfo {
        uint32 ID;
        uint32 ClassID;
        uint32 OrderIndex; // Wichtig für Sortierung!
    };
    std::vector<TabInfo> tabList;

    for (uint32 i = 0; i < sTalentTabStore.GetNumRows(); ++i)
    {
        TalentTabEntry const* tab = sTalentTabStore.LookupEntry(i);
        if (!tab) continue;
        
        uint32 classMask = tab->ClassMask;
        uint32 classID = 0;
        if (classMask > 0) {
            uint32 mask = classMask;
            uint32 bitPos = 0;
            while (mask > 0) { mask >>= 1; bitPos++; }
            classID = bitPos;
        }

        TabInfo info;
        info.ID = tab->ID;
        info.ClassID = classID;
        info.OrderIndex = tab->OrderIndex; // Hier den OrderIndex speichern
        tabList.push_back(info);
    }

    std::map<uint32, std::vector<RawTalent>> talentsByTabID;

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talent = sTalentStore.LookupEntry(i);
        if (!talent) continue;

        uint32 tabID = talent->TabID;
        // Nur wenn wir diesen Tab kennen (aus Schritt 1)
        bool found = false;
        for(auto const& t : tabList) if(t.ID == tabID) { found = true; break; }
        if (!found) continue;

        uint8 ranks = 0;
        for (int r = 0; r < 5; ++r) {
            if (talent->SpellRank[r] > 0) ranks++;
            else break;
        }
        if (ranks == 0 && talent->SpellRank[0] > 0) ranks = 1;

        RawTalent rt;
        rt.TalentID = talent->ID;
        rt.SpellIDRank1 = talent->SpellRank[0];
        rt.MaxRanks = ranks;
        rt.Tier = talent->TierID;
        rt.Column = talent->ColumnIndex;

        talentsByTabID[tabID].push_back(rt);
    }

    std::map<uint32, std::vector<SpecTemplate>> tempClassMap;

    for (auto const& tab : tabList)
    {
        SpecTemplate spec;
        spec.SpecID = tab.ID;
        
        auto it = talentsByTabID.find(tab.ID);
        if (it != talentsByTabID.end())
        {
            std::vector<RawTalent> rawList = it->second;
            
            // Talente innerhalb des Specs sortieren (Tier -> Column)
            std::sort(rawList.begin(), rawList.end(), [](const RawTalent& a, const RawTalent& b) {
                if (a.Tier != b.Tier) return a.Tier < b.Tier;
                return a.Column < b.Column;
            });

            for (auto const& rt : rawList)
            {
                TalentTemplate tt;
                tt.TalentID = rt.TalentID;
                tt.SpellIDRank1 = rt.SpellIDRank1;
                tt.MaxRanks = rt.MaxRanks;
                tt.Tier = rt.Tier;
                tt.Column = rt.Column;
                spec.Talents.push_back(tt);
            }
        }
        
        // Spec der Klasse hinzufügen
        tempClassMap[tab.ClassID].push_back(spec);
    }

    for (auto& pair : tempClassMap)
    {
        std::sort(pair.second.begin(), pair.second.end(), [](const SpecTemplate& a, const SpecTemplate& b) {
            // Wir brauchen den OrderIndex hier wieder. 
            // Da wir ihn im SpecTemplate nicht gespeichert haben, müssen wir ihn kurz nachschlagen
            // oder ihn ins SpecTemplate aufnehmen (empfohlen).
            // Einfacher: Wir nehmen ihn ins Struct auf (siehe unten).
            return false; // Platzhalter
        });
        
        // Besser: Wir haben den OrderIndex schon im TabInfo. 
        // Wir können die Sortierung direkt im Vector "tabList" machen, BEVOR wir die Specs bauen.
    }

    // B. Iteriere durch die SORTIERTE Liste und baue die Struktur
    for (auto const& tab : tabList)
    {
        SpecTemplate spec;
        spec.SpecID = tab.ID;
        // Optional: spec.OrderIndex = tab.OrderIndex; falls du es später brauchst

        auto it = talentsByTabID.find(tab.ID);
        if (it != talentsByTabID.end())
        {
            std::vector<RawTalent> rawList = it->second;
            std::sort(rawList.begin(), rawList.end(), [](const RawTalent& a, const RawTalent& b) {
                if (a.Tier != b.Tier) return a.Tier < b.Tier;
                return a.Column < b.Column;
            });

            for (auto const& rt : rawList)
            {
                TalentTemplate tt;
                tt.TalentID = rt.TalentID;
                tt.SpellIDRank1 = rt.SpellIDRank1;
                tt.MaxRanks = rt.MaxRanks;
                tt.Tier = rt.Tier;
                tt.Column = rt.Column;
                spec.Talents.push_back(tt);
            }
        }
        // Push_back behält die Reihenfolge von tabList (also OrderIndex) bei!
        m_ClassTemplates[tab.ClassID].push_back(spec);
    }

    // --- NEUER ABPLAN ---
    m_ClassTemplates.clear();
    
    // A. Sortiere die Tab-Liste nach OrderIndex
    std::sort(tabList.begin(), tabList.end(), [](const TabInfo& a, const TabInfo& b) {
        return a.OrderIndex < b.OrderIndex;
    });

    // B. Iteriere durch die SORTIERTE Liste und baue die Struktur
    for (auto const& tab : tabList)
    {
        SpecTemplate spec;
        spec.SpecID = tab.ID;

        auto it = talentsByTabID.find(tab.ID);
        if (it != talentsByTabID.end())
        {
            std::vector<RawTalent> rawList = it->second;
            std::sort(rawList.begin(), rawList.end(), [](const RawTalent& a, const RawTalent& b) {
                if (a.Tier != b.Tier) return a.Tier < b.Tier;
                return a.Column < b.Column;
            });

            for (auto const& rt : rawList)
            {
                TalentTemplate tt;
                tt.TalentID = rt.TalentID;
                tt.SpellIDRank1 = rt.SpellIDRank1;
                tt.MaxRanks = rt.MaxRanks;
                tt.Tier = rt.Tier;
                tt.Column = rt.Column;
                spec.Talents.push_back(tt);
            }
        }
        // Push_back behält die Reihenfolge von tabList (also OrderIndex) bei!
        m_ClassTemplates[tab.ClassID].push_back(spec);
    }

    TC_LOG_INFO("custom", "[GearPlanner] Erfolgreich {} Klassen geladen.", m_ClassTemplates.size());
}

std::string WowheadTalentEngine::ExportJson(Player* player)
{
    if (!player) return "{\"error\":\"No player\"}";
    
    rapidjson::Value slots(rapidjson::kObjectType);
    std::string talentHash;

    // JSON mit RapidJSON bauen
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    BuildSlotsData(player, slots, allocator);
    BuildGenerateTalentHash(player, talentHash);

    rapidjson::Value arr(rapidjson::kArrayType);
    doc.AddMember("buffs", arr, allocator);
    doc.AddMember("classId", player->GetClass(), allocator);
    doc.AddMember("genderId", player->GetGender(), allocator);
    doc.AddMember("level", player->GetLevel(), allocator);
    doc.AddMember("phase", 6, allocator);
    doc.AddMember("raceId", player->GetRace(), allocator);
    doc.AddMember("shapeshiftForm", 0, allocator);
    doc.AddMember("slots", slots, allocator);
    rapidjson::Value talentHashVal(talentHash.c_str(), allocator);
    doc.AddMember("talentHash", talentHashVal, allocator);
    doc.AddMember("version", 1, allocator);

    // add debug data
    // m_enchantSpellMap
    //doc.AddMember("debug_enchantSpellMap_size", static_cast<uint32>(m_enchantSpellMap.size()), allocator);
    //rapidjson::Value dbcDebug(rapidjson::kArrayType);
    //for (const auto& pair : m_enchantSpellMap)
    //{
    //    rapidjson::Value enchantEntry(rapidjson::kObjectType);
    //    enchantEntry.AddMember("enchantID", pair.second, allocator);
    //    enchantEntry.AddMember("spellID", pair.first, allocator);
    //    dbcDebug.PushBack(enchantEntry, allocator);
    //}
    //doc.AddMember("debug_enchantSpellMap", dbcDebug, allocator);

    // Serialisieren
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

void WowheadTalentEngine::BuildGenerateTalentHash(Player* player, std::string& talentHash)
{
    uint32 classID = player->GetClass();
    auto it = m_ClassTemplates.find(classID);
    if (it == m_ClassTemplates.end()) {
        throw std::runtime_error("ClassID " + std::to_string(classID) + " not found in class templates.");
        return;
    }

    std::vector<uint32> points(3, 0);
    std::string ranksStr = "";

    // Iteriere durch alle Specs dieser Klasse (meist 3)
    for (size_t i = 0; i < it->second.size(); ++i)
    {
        SpecTemplate const& spec = it->second[i];
        uint32 specPoints = 0;
        if(i > 0) ranksStr += "-"; // Trennzeichen zwischen Specs

        for (auto const& talent : spec.Talents)
        {
            uint8 rank = 0;
            // Finde TalentEntry über TalentID oder SpellID
            TalentEntry const* tEntry = sTalentStore.LookupEntry(talent.TalentID);
            if (tEntry)
            {
                // Prüfe von oben nach unten
                for (uint8 r = talent.MaxRanks; r >= 1; --r)
                {
                    if (tEntry->SpellRank[r-1] > 0 && player->HasSpell(tEntry->SpellRank[r-1]))
                    {
                        rank = r;
                        break;
                    }
                }
            }
            specPoints += rank;
            ranksStr += std::to_string(rank);
        }
        if (i < 3) points[i] = specPoints;
        
        while (ranksStr.length() > 0 && ranksStr.back() == '0')
        {
            ranksStr.pop_back();
        }
    }
    
    // abschließenden "-" entfernen, falls vorhanden
    if(ranksStr[ranksStr.length()-1] == '-') ranksStr.pop_back();

    // Glyphen auslesen
    std::string glyphsStr = "0";
    std::string lookupStr = "0123456789abcdefghjkmnpqrstvwxyz";
    const uint8 majorSlots[] = { 0, 3, 5 };
    const uint8 minorSlots[] = { 1, 2, 4 };

    uint8 wowHeadSlotIterator = -1;
    for (uint8 slot : majorSlots)
    {
        wowHeadSlotIterator++;
        uint32 glyphSpellID = player->GetGlyph(player->GetActiveTalentGroup(), slot);
        if (glyphSpellID > 0)
        {
            glyphsStr += std::to_string(wowHeadSlotIterator);
            GlyphPropertiesEntry const* glyphInfo = sGlyphPropertiesStore.LookupEntry(glyphSpellID);
            glyphsStr += lookupStr[glyphInfo->SpellID >> 15 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 10 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 5 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 0 & 31];
        }
    }
    for(uint8 slot : minorSlots)
    {
        wowHeadSlotIterator++;
        uint32 glyphSpellID = player->GetGlyph(player->GetActiveTalentGroup(), slot);
        if (glyphSpellID > 0)
        {
            glyphsStr += std::to_string(wowHeadSlotIterator);
            GlyphPropertiesEntry const* glyphInfo = sGlyphPropertiesStore.LookupEntry(glyphSpellID);
            //glyphsStr += "___"+std::to_string(glyphInfo->SpellID)+"___";
            glyphsStr += lookupStr[glyphInfo->SpellID >> 15 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 10 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 5 & 31];
            glyphsStr += lookupStr[glyphInfo->SpellID >> 0 & 31];
        }
    }

    talentHash = ranksStr+"_"+glyphsStr;
}

void WowheadTalentEngine::BuildSlotsData(Player* player, rapidjson::Value& slots, rapidjson::Document::AllocatorType& allocator)
{
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (item)
        {
            rapidjson::Value slotKey(std::to_string(i + 1).c_str(), allocator);
            rapidjson::Value slotData(rapidjson::kObjectType);

            uint32 itemIDValue = item->GetEntry();
            uint32 enchantIDValue = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT);
            rapidjson::Value gemsObj(rapidjson::kObjectType);
            bool hasGems = false;

            for (uint8 socketIndex = 0; socketIndex < 3; ++socketIndex) {
                uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(socketIndex + SOCK_ENCHANTMENT_SLOT));
                if(!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;
                
                std::string key = std::to_string(socketIndex);
                rapidjson::Value keyVal(key.c_str(), allocator);
                
                // Value ist die Item-ID des Gems
                gemsObj.AddMember(keyVal, enchantEntry->SrcItemID, allocator);
                hasGems = true;
            }

            // item id
            if(itemIDValue)
            {
                slotData.AddMember("item", itemIDValue, allocator);
                int32 randomPropertyId = item->GetItemRandomPropertyId();
                if (randomPropertyId != 0)
                {
                    slotData.AddMember("randomEnchant", randomPropertyId, allocator);
                }
            }
            // enchant id
            if(enchantIDValue)
            {
                // suche in der enchantSpellMap nach dem SpellID für das EnchantID
                auto it = m_enchantSpellMap.find(enchantIDValue);
                if (it != m_enchantSpellMap.end())
                {
                    uint32 spellID = it->second;
                    //rapidjson::Value spellIDVal(std::to_string(spellID).c_str(), allocator);
                    slotData.AddMember("enchant", spellID, allocator);
                } else {
                    
                    TC_LOG_ERROR("custom", "[GearPlanner] EnchantID {} nicht in m_enchantSpellMap gefunden!", enchantIDValue);
                    slotData.AddMember("enchantNOTFOUND", enchantIDValue, allocator);
                }
            }
            // gems
            if (hasGems) {
                slotData.AddMember("gems", gemsObj, allocator);
            }
            
            slots.AddMember(slotKey, slotData, allocator);
        }
    }
}