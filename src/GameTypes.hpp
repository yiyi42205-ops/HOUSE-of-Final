// src/GameTypes.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <random>
#include <iomanip>
#include <ctime>
#include <map>

// ================= 字串與編碼輔助工具 =================

// 將 C-style 字串轉換為 SFML 支援的 UTF-8 字串
inline sf::String u8(const char* s) {
    return sf::String::fromUtf8(s, s + std::strlen(s));
}

// 將 std::string 轉換為 SFML 支援的 UTF-8 字串
inline sf::String u8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

// 根據 Sanity 數值對文字進行雜訊化處理 (Glitch Effect)
inline sf::String glitchText(const std::string& s, int sanity) {
    if (sanity >= 80) return u8(s);

    std::string res = s;
    int corruptionLevel = (100 - sanity) / 8;
    for (auto& c : res) {
        // 僅對 ASCII 字元進行隨機替換，保留 UTF-8 多字節字元
        if ((c & 0x80) == 0 && std::rand() % 20 < corruptionLevel) {
            const char noise[] = "#?%&@$!......";
            c = noise[std::rand() % (sizeof(noise) - 1)];
        }
    }
    return u8(res);
}

// ================= 遊戲狀態枚舉 =================

enum class MenuState {
    Main,       // 主選單
    Gallery,    // 回想畫廊
    Rules       // 規則說明
};

// ================= 基礎常數設定 =================

constexpr unsigned WINDOW_WIDTH  = 800;
constexpr unsigned WINDOW_HEIGHT = 600;
constexpr float    PLAYER_RADIUS = 16.f;
constexpr float    WALK_SPEED    = 3.0f;
constexpr float    RUN_SPEED     = 5.5f;
constexpr int      MAX_HP        = 100;
constexpr int      MAX_SANITY    = 100;

// =============== 時間與排程系統 =================

enum class Day { Sun = 0, Mon, Tue, Wed, Thu, Fri, Sat, COUNT };
enum class Slot { Morning = 0, Afternoon, Night, LateNight, COUNT };

inline std::string dayName(Day d) {
    static const char* names[] = {"週日", "週一", "週二", "週三", "週四", "週五", "週六"};
    return names[static_cast<int>(d)];
}

inline std::string slotName(Slot s) {
    static const char* names[] = {"早上", "下午", "晚上", "半夜"};
    return names[static_cast<int>(s)];
}

struct TimeState {
    Day  day  = Day::Sun;
    Slot slot = Slot::Morning;
    bool isFeb29 = false; // 特殊時間狀態標記
};

// =============== 學業與角色系統 =================

enum class Subject { Calc = 0, Prog, PE, Brain, COUNT };
enum class PlayerGender {
    None,   // 未選擇 (初始狀態)
    Female,
    Male
};

inline std::string subjectName(Subject s) {
    static const char* names[] = {"微積分", "程式設計", "體育", "腦科學與計算導論"};
    return names[static_cast<int>(s)];
}

struct SubjectState {
    int prepLevel = 4; // 準備程度 (0=最佳, 4=最差)
    int score     = -1;
    bool tookExam = false;
};

// 取得特定日期對應的考試科目
inline std::optional<Subject> examFor(Day d) {
    switch (d) {
        case Day::Mon: return Subject::Calc;
        case Day::Tue: return Subject::Prog;
        case Day::Thu: return Subject::PE;
        case Day::Fri: return Subject::Brain;
        default:       return std::nullopt;
    }
}

// ================= 地圖區域系統 =================

enum class AreaId {
    Dorm = 0,
    Campus,
    Foodcourt,
    Cafe,
    Library,
    LibraryStudy,
    LibraryShop,
    CSNorth,
    CSSouth,
    CSBasement,
    Lab,
    ChaosRift,
    COUNT
};

inline std::string areaName(AreaId a) {
    switch (a) {
        case AreaId::Dorm:         return "宿舍區";
        case AreaId::Campus:       return "校園廣場";
        case AreaId::Foodcourt:    return "美食街";
        case AreaId::Cafe:         return "校園咖啡廳";
        case AreaId::Library:      return "圖書館大廳";
        case AreaId::LibraryStudy: return "2F 自習室";
        case AreaId::LibraryShop:  return "B1 便利商店";
        case AreaId::CSNorth:      return "資工北館";
        case AreaId::CSSouth:      return "資工南館";
        case AreaId::CSBasement:   return "系館地下室";
        case AreaId::Lab:          return "實驗室 (禁區)";
        case AreaId::ChaosRift:    return "系統邊界 (? ? ?)";
        default:                   return "未定義";
    }
}

// ================== 遊戲物件結構 ==================

struct Obstacle {
    sf::FloatRect rect;
    sf::Color color;
    AreaId area;
    std::string textureKey; // 紋理資產索引鍵
};

struct DialogueStage {
    std::string text;
    int requiredSanityMax  = 100;  // 觸發條件：Sanity 上限
    int requiredAwarenessMin = 0;  // 觸發條件：Awareness 下限
};

struct Option {
    std::string text;
    int sanityDelta = 0;
    bool isKillAction = false;
    std::string reaction;
};
class Player {
    friend class Game;

private:
    // 1. 視覺與物理
        sf::CircleShape shape;
        sf::Texture texture;
        sf::Sprite sprite;
        
        bool hasTexture = false;
    
    // 2. 身份與狀態 (Identity & Status)
    PlayerGender gender = PlayerGender::None;
    AreaId currentArea = AreaId::Dorm; // 主角所在區域
    int hp = 100;
    int sanity = 100;
    int addictionLevel = 0;
    int consecutiveBathSlots = 0;
    bool gotPoisoned = false;
    bool hasLabKey = false; // 關鍵道具

    // 3. 學業狀態 (Academics)
    std::array<SubjectState, 4> subjects{};

public:
    Player()
        : texture()
        , sprite(texture)
    {
        shape.setRadius(16.f);
        shape.setOrigin({16.f, 16.f});
        shape.setFillColor(sf::Color(230, 230, 230));
        for (auto& s : subjects) {
            s.prepLevel = 4;
            s.score = -1;
            s.tookExam = false;
        }
    }
    bool loadTexture(const std::filesystem::path& path) {
        if (texture.loadFromFile(path)) {
            texture.setSmooth(false);
            sprite.setTexture(texture, true);
            
            auto sz = texture.getSize();
            float size = 16.f * 2.f;
            float scale = size / static_cast<float>(std::max(sz.x, sz.y));
            sprite.setScale({scale, scale});
            sprite.setOrigin({static_cast<float>(sz.x) / 2.f, static_cast<float>(sz.y) / 2.f});
            
            hasTexture = true;
            return true;
        }
        return false;
    }

    void setPosition(const sf::Vector2f& pos) {
        shape.setPosition(pos);
        if (hasTexture) sprite.setPosition(pos);
    }

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }

    void draw(sf::RenderWindow& window) {
        if (hasTexture) {
            window.draw(sprite);
        } else {
            window.draw(shape);
        }
    }
    
    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }
};
class NPC {
    friend class Game;

public:
    struct StageData {
        std::string text;
        std::vector<Option> options;
        int reqSanMax = 100;
        int reqAwMin = 0;
    };

private:
  
    AreaId area;
    sf::Vector2f pos;
    std::string name;
    std::string portraitKey;
    
    // 數值設定
    float radius = 13.f;
    sf::Color color;
    bool isStressor = true;
    bool spendsSlot = false;
    int sanityDeltaOnTalk = 0;
    // 對話資料
    std::vector<DialogueStage> normalStages;
    std::vector<DialogueStage> shadowStages;
    std::vector<bool> shadowStagesSeen;
   
    std::vector<StageData> dynamicStages;

    // 狀態
    int shadowLoopIndex = 0;
    int dialogIndex = 0;
    bool isDead = false;
    std::string killDescription;

public:
    NPC(std::string n, AreaId a, sf::Vector2f p, std::string key, int sanityDelta, bool timeCost)
        : area(a)               // 1. 對應 area
        , pos(p)                // 2. 對應 pos
        , name(n)               // 3. 對應 name
        , portraitKey(key)      // 4. 對應 portraitKey
        , spendsSlot(timeCost)          // 5. 對應 spendsSlot
        , sanityDeltaOnTalk(sanityDelta)// 6. 對應 sanityDeltaOnTalk
    {
        if (name == "貓咪") {
            radius = 10.f;
            color = sf::Color(255, 200, 150);
        } else {
            color = sf::Color(200, 200, 255);
        }
        isStressor = (sanityDelta < 0);
    }
    
    void addNormalStage(const std::string& text) {
        normalStages.push_back({text});
    }

    void addShadowStage(const std::string& text, int reqSan, int reqAw) {
        shadowStages.push_back({text, reqSan, reqAw});
        shadowStagesSeen.push_back(false);
    }
};

enum class ItemEffectType {
    None,
    StudyGeneric,
    StudyBoost,
    Sleep,
    Waste,
    Bath,
    Poison,
    Poster,
    LabDoor,
    CampusGate,
    // 傳送點定義
    DoorLibraryToStudy,
    DoorLibraryToShop,
    DoorStudyToLibrary,
    DoorShopToLibrary,
    DoorCampusToDorm,
    DoorDormToCampus,
    DoorCampusToLibrary,
    DoorLibraryToCampus,
    DoorCampusToCS,
    DoorCSToCampus,
    DoorCampusToFoodcourt,
    DoorFoodcourtToCampus,
    DoorCSNorthToSouth,
    DoorCSSouthToNorth,
    DoorCSSouthToBasement,
    DoorBasementToCSSouth,
    DoorChaosToCampus,
    DoorFoodcourtToCafe,
    DoorCafeToFoodcourt,
    // 功能性物件
    Phone,
    VendingMachine,
    Food,
    Sofa,
    SanityDrain
};

struct Interactable {
    AreaId area;
    sf::FloatRect rect;
    sf::Color color;
    std::string label;
    ItemEffectType effect = ItemEffectType::None;
    std::vector<DialogueStage> stages;
    std::string textureKey;
};

// ================== 結局與遊戲模式 ==================

enum class EndingId {
    Normal = 0,
    SleepTooMuch,
    Addict,
    Poison,
    Drown,
    Overwork,
    SystemFailure,
    TrueEnd,
    AllPerfect,     // 隱藏結局：全科滿分
    FinalEscape,    // 真結局：脫離循環
    COUNT
};

enum class GameMode  { MainMenu, Playing };
enum class EndingMode {
    None,
    NormalOrBad,
    TrueIntro,
    TrueEndLoop,    // 選項 1: 保留記憶
    TrueEndReset,   // 選項 2: 刪除重來
    TrueEndReality  // 選項 3: 拔線 
};
