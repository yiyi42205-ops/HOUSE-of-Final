#pragma once
#include "GameTypes.hpp"

// 結局場景結構定義
struct EndingScene {
    std::string text;
    std::string imageKey;
};

class Game {
public:
    Game(const std::filesystem::path& exeDir);
    void runGameLoop();

private:
    std::filesystem::path resourceDir;
    sf::RenderWindow window;
    sf::View worldView;
    sf::Font font;

    Player player;

    sf::Music bgm;
    std::filesystem::path bgm1Path, bgm2Path, bgm3Path, bgm4Path;
    std::filesystem::path currentBgmPath;
    bool inExamMiniGame = false;

    void updateBgmForCurrentArea();
    void playExamBgm();

    sf::Text hudText;
    sf::Text infoText;
    sf::Text dialogueText;
    sf::Text floatingText;
    
    sf::SoundBuffer seConfirmBuffer;
    sf::Sound seConfirm;

    std::map<std::string, sf::Texture> portraitTextures;
    std::optional<sf::Sprite> portraitSprite;
    bool showPortrait = false;
    std::string currentPortraitKey;

    GameMode gameMode  = GameMode::MainMenu;
    EndingMode endingMode = EndingMode::None;

    bool inputUp = false;
    bool inputDown = false;
    bool inputLeft = false;
    bool inputRight = false;
    bool inputRun = false;
    
    int loopCount = 0;
    int awareness = 0;
    std::array<bool, static_cast<size_t>(EndingId::COUNT)> endingUnlocked{};

    AreaId area = AreaId::Dorm;
    TimeState time;
    bool slotUsed = false;
    
    // subjects, hp, sanity 等變數已移至 Player，此處刪除
    int currentStudyBonus = 1;
    
    // 主選單與存檔系統
    MenuState menuState = MenuState::Main;
    int menuSelection = 0;
    bool hasSaveFile = false;

    // 輔助函式
    void checkSaveFileExistence();
    void renderGallery();
    void renderRules();
        
    int sleepCountPerDay[7] = {0};
    
    bool labUnlocked = false;
    bool trueEndingChoiceRemember = false;

    std::vector<Obstacle> obstacles;
    std::vector<NPC> npcs;
    std::vector<Interactable> items;

    bool inDialogue = false;
    bool inStudyChoice = false;
    bool inTrueChoice = false;

    std::vector<EndingScene> endingScenes;
    size_t endingSceneIndex = 0;
    std::map<std::string, sf::Texture> objectTextures;
    
    sf::Texture endingBgTexture;
    sf::Sprite endingBgSprite;

    bool inOpeningIntro = false;
    std::vector<std::string> openingPages;
    std::size_t openingPageIndex = 0;
    
    // 畫廊專用變數
    int gallerySelection = 0;
    bool galleryViewingImage = false;
    sf::Sprite gallerySprite;

    void setupWorld();
    void setupNPCs();
    void setupItems();
    void resetGameState();
    void clearInputFlags();

    void processMainMenuEvents();
    void renderMainMenu();
    void processEvents();
    void update();
    void render();

    bool checkCollision(const sf::Vector2f& pos);
    void drawFloatingLabels();
    void warpTo(AreaId target, const sf::Vector2f& pos, const std::string& msg);

    void updateHUD();
    void startDialogue(const std::string& msg);
    void advanceSlot();
    void checkWeekEnd();
    void saveGame();
    void loadGame();
    
    void triggerBadEnd(const std::string& text, EndingId id, const std::string& imgKey = "");
    void triggerNormalEnding();
    void startTrueEndingIntro();
    void applyTrueEndingChoice(int choice);

    void addEndingScene(const std::string& txt, const std::string& imgKey = "");

    int  countUnlockedPublicEndings() const;
    bool hasPerfectScoreSecret() const;

    void setPortrait(const std::string& key);
    void checkSanityZero();
    void relocateCat();
    void runGenderSelectScreen();
    
    bool applyHpCost(int amount);
    void handleTalkOrInspect();
    void openStudyChoice(bool boosted);
    void applyStudyOnSubject(Subject s);
    void applySleepAction();
    void applyWasteAction();
    void applyBathAction();
    void applyPoisonAction();
    void applyPosterAction();
    void applyLabDoorAction();
    void applyCampusGateAction();
    void applyVendingAction();
    void applyFoodAction();
    int playExamMiniGame(Subject subj);

    int countUnlockedEndings() const {
        int c = 0;
        for (bool b : endingUnlocked) if (b) ++c;
        return c;
    }
};
