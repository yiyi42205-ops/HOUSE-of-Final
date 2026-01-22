#include "Game.hpp"
#include <fstream>

// 檢查存檔
void Game::checkSaveFileExistence() {
    hasSaveFile = std::filesystem::exists("save.dat");
}

// 建構子與初始化
Game::Game(const std::filesystem::path& exeDir)
    : resourceDir(exeDir)
    , window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), u8("HOUSE of Final"), sf::Style::Default)
    , worldView(sf::FloatRect({0.f, 0.f}, {800.f, 600.f}))
    , player()
    , hudText(font)
    , infoText(font)
    , dialogueText(font)
    , floatingText(font)
    , seConfirm(seConfirmBuffer)
    , endingBgSprite(endingBgTexture)
    , gallerySprite(endingBgTexture)
{
    window.setFramerateLimit(60);

    auto fontPath = resourceDir / "assets" / "font.ttf";
    if (!font.openFromFile(fontPath)) {
        std::cerr << "Warning: Font not found.\n";
    }

    bgm1Path = resourceDir / "assets" / "bgm1.ogg";
    bgm2Path = resourceDir / "assets" / "bgm2.ogg";
    bgm3Path = resourceDir / "assets" / "bgm3.ogg";
    bgm4Path = resourceDir / "assets" / "bgm4.ogg";

    auto sePath = resourceDir / "assets" / "confirm.wav";
    if (!seConfirmBuffer.loadFromFile(sePath)) {
        std::cout << "載入音效失敗: " << sePath << "\n";
    } else {
        seConfirm.setBuffer(seConfirmBuffer);
    }
    
    // 【修正】透過 Player 類別的方法載入貼圖
    std::filesystem::path playerPath = resourceDir / "assets" / "player_f.png";
    player.loadTexture(playerPath);
    
    hudText.setCharacterSize(16);
    hudText.setPosition({10.f, 5.f});

    infoText.setCharacterSize(18);
    infoText.setPosition({10.f, 540.f});

    dialogueText.setCharacterSize(18);
    dialogueText.setPosition({70.f, 370.f});

    floatingText.setCharacterSize(14);
    floatingText.setFillColor(sf::Color::Yellow);
    floatingText.setOutlineColor(sf::Color::Black);
    floatingText.setOutlineThickness(1.f);

    setupWorld();
    setupNPCs();
    setupItems();
    
    // 初始化主選單狀態
    checkSaveFileExistence();
    
    std::ifstream file("save.dat");
    if (file.is_open()) {
        hasSaveFile = true;
        file >> loopCount >> awareness;
        
        int dummy; file >> dummy >> dummy;
        for (int i = 0; i < (int)EndingId::COUNT; ++i) {
            int unlocked;
            file >> unlocked;
            endingUnlocked[i] = (unlocked == 1);
        }
        file.close();
    } else {
        hasSaveFile = false;
        loopCount = 0;
        awareness = 0;
    }

    menuSelection = hasSaveFile ? 0 : 1;
    gameMode = GameMode::MainMenu;
}

void Game::runGameLoop() {
    while (window.isOpen()) {
        if (gameMode == GameMode::MainMenu) {
            processMainMenuEvents();
            renderMainMenu();
        } else {
            processEvents();
            update();
            render();
        }
    }
}

void Game::clearInputFlags() {
    inputUp = inputDown = inputLeft = inputRight = inputRun = false;
}

// 重置遊戲狀態
void Game::resetGameState() {
    endingMode = EndingMode::None;
    
    endingScenes.clear();
    endingSceneIndex = 0;
    
    inDialogue = false;
    dialogueText.setString("");
    infoText.setString("");

    clearInputFlags();

    for (auto& s : player.subjects) {
        s.prepLevel = 4;
        s.score = -1;
        s.tookExam = false;
    }
    time = TimeState{};
    slotUsed = false;
    
    player.hp = MAX_HP;
    player.sanity = 90;
    
    int unlocked = countUnlockedEndings();
    awareness = unlocked * 15;

    if (unlocked == 0) {
        trueEndingChoiceRemember = false;
    }
    else if (trueEndingChoiceRemember) {
        player.sanity = 80;
    }
    
    player.addictionLevel = 0;
    currentStudyBonus = 1;
    labUnlocked = false;
    player.gotPoisoned = false;
    player.hasLabKey = false;
    for (int& c : sleepCountPerDay) c = 0;
    player.consecutiveBathSlots = 0;

    area = AreaId::Dorm;

    if (player.gender == PlayerGender::None) {
        runGenderSelectScreen();
        std::filesystem::path playerPath = (player.gender == PlayerGender::Male)
            ? resourceDir / "assets" / "player_m.png"
            : resourceDir / "assets" / "player_f.png";

        player.loadTexture(playerPath);
    }
    relocateCat();
    
    player.setPosition({400.f, 360.f});

    for (auto& n : npcs) {
        n.shadowLoopIndex = 0;
    }

    openingPages.clear();
    openingPageIndex = 0;
    inOpeningIntro = true;

    std::string page1;
    if (unlocked == 0 && !trueEndingChoiceRemember) {
        page1 = "你在宿舍的床上醒來。\n（我：怎麼覺得這學期過得特別快…好像昨天才在想要不要停修。）\n\n[按 Enter / Space 繼續]";
    } else if (trueEndingChoiceRemember) {
        page1 = "你在宿舍的床上猛然醒來，\n腦中還殘留著實驗室裡一整排螢幕的殘影。\n手機螢幕顯示今天是期末週第一天。\n（我：這次，換我自己決定要怎麼活過去。）\n\n[按 Enter / Space 繼續]";
    } else {
        page1 = "你又在宿舍的床上醒來。\n（我：總覺得這一週好像重複過很多次…但每次結果都不一樣。）\n\n[按 Enter / Space 繼續]";
    }
    openingPages.push_back(page1);

    std::string page2 = "本週考試時間（晚上時段按 E 參加）：\n  週一：微積分\n  週二：程式設計\n  週四：體育\n  週五：腦科學與計算導論\n\n白天可以自由安排讀書、休息或逃避，\n只是每個時段都不會再回來。\n\n[按 Enter / Space 開始行動]";
    openingPages.push_back(page2);

    updateHUD();
}

bool Game::applyHpCost(int amount) {
    player.hp -= amount;
    if (player.hp <= 0) {
        triggerBadEnd(
            "BAD END: [過勞]\n你的身體像一台被逼到極限的舊電腦。\n某一刻風扇聲消失了，螢幕一黑，什麼都來不及存檔。",
            EndingId::Overwork,
            "cg_bad_overwork"
        );
        return false;
    }
    return true;
}

int Game::countUnlockedPublicEndings() const {
    int c = 0;
    for (int i = 0; i < (int)EndingId::COUNT; ++i) {
        if ((EndingId)i == EndingId::AllPerfect) continue;
        if (endingUnlocked[i]) ++c;
    }
    return c;
}

bool Game::hasPerfectScoreSecret() const {
    return endingUnlocked[(int)EndingId::AllPerfect];
}

// 處理一般遊戲事件
void Game::processEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            saveGame();
            window.close();
        }
        if (event->is<sf::Event::FocusLost>()) clearInputFlags();
        
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            
            if (inOpeningIntro) {
                if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                    if (openingPageIndex + 1 < openingPages.size()) {
                        openingPageIndex++;
                    } else {
                        inOpeningIntro = false;
                    }
                }
                continue;
            }
            if (key->code == sf::Keyboard::Key::W || key->scancode == sf::Keyboard::Scancode::W || key->code == sf::Keyboard::Key::Up)    inputUp = true;
            if (key->code == sf::Keyboard::Key::S || key->scancode == sf::Keyboard::Scancode::S || key->code == sf::Keyboard::Key::Down)  inputDown = true;
            if (key->code == sf::Keyboard::Key::A || key->scancode == sf::Keyboard::Scancode::A || key->code == sf::Keyboard::Key::Left)  inputLeft = true;
            if (key->code == sf::Keyboard::Key::D || key->scancode == sf::Keyboard::Scancode::D || key->code == sf::Keyboard::Key::Right) inputRight = true;
            if (key->code == sf::Keyboard::Key::LShift || key->code == sf::Keyboard::Key::RShift) inputRun = true;

                        if (endingMode != EndingMode::None) {
                            
                            bool isLastPage = !endingScenes.empty() && (endingSceneIndex == endingScenes.size() - 1);

                            if (inTrueChoice && isLastPage) {
                                // 選項邏輯保持不變
                                if (key->code == sf::Keyboard::Key::Num1 || key->code == sf::Keyboard::Key::Numpad1) applyTrueEndingChoice(1);
                                else if (key->code == sf::Keyboard::Key::Num2 || key->code == sf::Keyboard::Key::Numpad2) applyTrueEndingChoice(2);
                                else if (key->code == sf::Keyboard::Key::Num3 || key->code == sf::Keyboard::Key::Numpad3) applyTrueEndingChoice(3);
                            }
                            else if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                                if (endingSceneIndex + 1 < endingScenes.size()) {
                                    endingSceneIndex++;
                                    seConfirm.play();
                                } else {
                                    
                                    // 1. Loop (保留記憶)
                                    if (endingMode == EndingMode::TrueEndLoop) {
                                        seConfirm.play();
                                        loopCount++;
                                        trueEndingChoiceRemember = true;
                                        resetGameState();
                                        saveGame();
                                        gameMode = GameMode::Playing;
                                        endingMode = EndingMode::None;
                                    }
                                    // 2. Reset (刪除重來)
                                    else if (endingMode == EndingMode::TrueEndReset) {
                                        seConfirm.play();
                                        for (auto& n : npcs) {
                                            std::fill(n.shadowStagesSeen.begin(), n.shadowStagesSeen.end(), false);
                                        }
                                        resetGameState();
                                        saveGame();
                                        gameMode = GameMode::Playing;
                                        endingMode = EndingMode::None;
                                    }
                                    // 3. Reality (拔線 - 通關)
                                                            else if (endingMode == EndingMode::TrueEndReality) {
                                                                seConfirm.play();
                                                                loopCount = 0;
                                                                awareness = 0;
                                                 trueEndingChoiceRemember = false;
                                                           
                                                                for (auto& n : npcs) {
                                                                    std::fill(n.shadowStagesSeen.begin(), n.shadowStagesSeen.end(), false);
                                                                }
                                                                endingUnlocked[(int)EndingId::FinalEscape] = true;

                                                                resetGameState();
                                                                saveGame();
                                                            
                                                                window.close();
                                                            }
                                    // 4. 普通/壞結局
                                    else if (endingMode == EndingMode::NormalOrBad) {
                                        seConfirm.play();
                                        loopCount++;
                                        resetGameState();
                                        saveGame();
                                        gameMode = GameMode::Playing;
                                        endingMode = EndingMode::None;
                                    }
                                }
                            }
                            continue;
                        }
            if (key->code == sf::Keyboard::Key::Escape) {
                            if (endingMode == EndingMode::None && !inDialogue && !inOpeningIntro) {
                                seConfirm.play();
                                saveGame();
                                bgm.stop();
                                currentBgmPath.clear();
                                
                                clearInputFlags();
                                
                                menuSelection = 0;
                                menuState = MenuState::Main;// 確保回到主選單頁面
                                gameMode = GameMode::MainMenu; // 切換模式
                                
                              
                                checkSaveFileExistence();
                                return; // 跳出函式，停止處理後續邏輯
                            }
                        }
            if (inDialogue) {
                if (inStudyChoice) {
                    if (key->code >= sf::Keyboard::Key::Num1 && key->code <= sf::Keyboard::Key::Num4) {
                        inDialogue = false; inStudyChoice = false;
                        int idx = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num1);
                        applyStudyOnSubject((Subject)idx);
                    } else if (key->code == sf::Keyboard::Key::Enter) {
                        inDialogue = false; inStudyChoice = false;
                    }
                } else {
                    if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                        inDialogue = false; clearInputFlags();
                    }
                }
                continue;
            }
            
            if (key->code == sf::Keyboard::Key::F) handleTalkOrInspect();
            
            if (key->code == sf::Keyboard::Key::E) {
                auto s = examFor(time.day);
                if (s && time.slot == Slot::Night) {
                    // 【修正】player.subjects
                    if (!player.subjects[(int)*s].tookExam) {
                        clearInputFlags();
                        int sc = playExamMiniGame(*s);
                        player.subjects[(int)*s].score = sc;
                        player.subjects[(int)*s].tookExam = true;
                        infoText.setString(u8("考試結束。得分：" + std::to_string(sc) + "\n（我：好想立刻打開成績單，又好怕看到。）"));
                        slotUsed = true; advanceSlot();
                    } else {
                        infoText.setString(u8("你已經考過這科了。\n（我：再考一次也不會多一個學分。）"));
                    }
                } else {
                    infoText.setString(u8("現在沒有考試。\n（我：卻覺得每一刻都像在被打分數。）"));
                }
            }
        }
        
        if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
            if (key->code == sf::Keyboard::Key::W || key->scancode == sf::Keyboard::Scancode::W || key->code == sf::Keyboard::Key::Up)    inputUp = false;
            if (key->code == sf::Keyboard::Key::S || key->scancode == sf::Keyboard::Scancode::S || key->code == sf::Keyboard::Key::Down)  inputDown = false;
            if (key->code == sf::Keyboard::Key::A || key->scancode == sf::Keyboard::Scancode::A || key->code == sf::Keyboard::Key::Left)  inputLeft = false;
            if (key->code == sf::Keyboard::Key::D || key->scancode == sf::Keyboard::Scancode::D || key->code == sf::Keyboard::Key::Right) inputRight = false;
            if (key->code == sf::Keyboard::Key::LShift || key->code == sf::Keyboard::Key::RShift) inputRun = false;
        }
    }
}

void Game::processMainMenuEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
        
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (menuState == MenuState::Main) {
                if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
                    menuSelection--;
                    if (menuSelection < 0) menuSelection = 4;
                    if (!hasSaveFile && menuSelection == 0) menuSelection = 4;
                    seConfirm.play();
                }
                if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
                    menuSelection++;
                    if (menuSelection > 4) menuSelection = 0;
                    if (!hasSaveFile && menuSelection == 0) menuSelection = 1;
                    seConfirm.play();
                }
                
                if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                    seConfirm.play();
                    switch (menuSelection) {
                        case 0:
                            loadGame();
                            gameMode = GameMode::Playing;
                            break;
                        case 1:
                            std::remove("save.dat");
                            hasSaveFile = false;
                            loopCount = 0;
                            std::fill(endingUnlocked.begin(), endingUnlocked.end(), false);
                            trueEndingChoiceRemember = false;
                            // 【修正】變數名稱
                            player.hasLabKey = false;
                            resetGameState();
                            gameMode = GameMode::Playing;
                            break;
                        case 2: menuState = MenuState::Gallery; break;
                        case 3: menuState = MenuState::Rules; break;
                        case 4: window.close(); break;
                    }
                }
            }
            else if (menuState == MenuState::Gallery) {
                if (galleryViewingImage) {
                    if (key->code == sf::Keyboard::Key::Escape ||
                        key->code == sf::Keyboard::Key::Enter ||
                        key->code == sf::Keyboard::Key::Space ||
                        key->code == sf::Keyboard::Key::Backspace) {
                        galleryViewingImage = false;
                    }
                }
                else {
                    if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
                        gallerySelection--;
                        if (gallerySelection < 0) gallerySelection = (int)EndingId::COUNT - 1;
                        seConfirm.play();
                    }
                    if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
                        gallerySelection++;
                        if (gallerySelection >= (int)EndingId::COUNT) gallerySelection = 0;
                        seConfirm.play();
                    }
                    if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                        if (endingUnlocked[gallerySelection]) {
                            galleryViewingImage = true;
                            seConfirm.play();
                        }
                    }
                    if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Backspace) {
                        menuState = MenuState::Main;
                        seConfirm.play();
                    }
                }
            }
            else if (menuState == MenuState::Rules) {
                if (key->code == sf::Keyboard::Key::Escape ||
                    key->code == sf::Keyboard::Key::Enter ||
                    key->code == sf::Keyboard::Key::Space ||
                    key->code == sf::Keyboard::Key::Backspace) {
                    menuState = MenuState::Main;
                    seConfirm.play();
                }
            }
        }
    }
}

void Game::renderMainMenu() {
    window.clear(sf::Color(10, 10, 15));
    window.setView(window.getDefaultView());

    sf::Text title(font);
    title.setString(u8("HOUSE of Final"));
    title.setCharacterSize(48);
    title.setStyle(sf::Text::Bold);
    title.setPosition({80.f, 60.f});
    title.setFillColor(sf::Color::White);
    window.draw(title);

    sf::Text subTitle(font);
    std::string subStr = "Loop " + std::to_string(loopCount);
    if (awareness > 80) subStr = "ERROR // " + std::to_string(loopCount);
    
    subTitle.setString(u8(subStr));
    subTitle.setCharacterSize(20);
    subTitle.setPosition({85.f, 120.f});
    subTitle.setFillColor(sf::Color(150, 150, 150));
    window.draw(subTitle);

    if (menuState == MenuState::Main) {
        std::vector<std::string> options = {
            "繼續遊戲 (Continue)",
            "新的遊戲 (New Game)",
            "回憶畫廊 (Gallery)",
            "查看規則 (Rules)",
            "離開遊戲 (Quit)"
        };

        for (int i = 0; i < 5; ++i) {
            sf::Text opt(font);
            opt.setString(u8(options[i]));
            opt.setCharacterSize(24);
            opt.setPosition({100.f, 200.f + i * 50.f});

            if (i == 0 && !hasSaveFile) {
                opt.setFillColor(sf::Color(80, 80, 80));
            } else if (i == menuSelection) {
                opt.setFillColor(sf::Color::Yellow);
                opt.setString(u8("▶ " + options[i]));
            } else {
                opt.setFillColor(sf::Color::White);
            }
            window.draw(opt);
        }
    }
    else if (menuState == MenuState::Gallery) {
        renderGallery();
    }
    else if (menuState == MenuState::Rules) {
        renderRules();
    }

    if (menuState == MenuState::Main) {
        sf::Text tip(font);
        tip.setString(u8("↑↓ 選擇  Enter 確認"));
        tip.setCharacterSize(16);
        tip.setPosition({100.f, 550.f});
        tip.setFillColor(sf::Color(100, 100, 100));
        window.draw(tip);
    }

    window.display();
}

void Game::renderGallery() {
    struct EndInfo {
        std::string name;
        std::string imgKey;
    };
    
    static const std::vector<EndInfo> endData = {
        {"Normal: 平凡的倖存", "cg_dorm_messy"},
        {"Bad: 植物人",       "cg_bad_sleep"},
        {"Bad: 數位成癮",     "cg_bad_phone"},
        {"Bad: 食物中毒",     "cg_bad_poison"},
        {"Bad: 溶解",         "cg_bad_drown"},
        {"Bad: 過勞",         "cg_bad_overwork"},
        {"Bad: 系統崩潰",     "cg_bad_overwork"},
        {"True: 樣本的抉擇",   "cg_observer"},
        {"Hidden: 完美數據",  "cg_dorm_messy"},
        {"True: 最終逃脫",    "cg_beach"}
    };

    if (galleryViewingImage) {
        static sf::Texture viewTex;
        std::string key = endData[std::min((size_t)gallerySelection, endData.size()-1)].imgKey;
        
        static std::string lastKey = "";
        if (key != lastKey) {
            std::string path = (resourceDir / "assets" / "cg" / (key + ".png")).string();
            if (!viewTex.loadFromFile(path)) {
                if (!viewTex.loadFromFile((resourceDir / "assets" / "cg" / "cg_observer.png").string())) {
                    std::cerr << "Error: Failed to load CG fallback.\n";
                }
            }
            lastKey = key;
        }

        gallerySprite.setTexture(viewTex, true);
        
        auto sz = viewTex.getSize();
        float scaleX = (float)WINDOW_WIDTH / sz.x;
        float scaleY = (float)WINDOW_HEIGHT / sz.y;
        float scale = std::min(scaleX, scaleY);
        if (sz.x == 0) scale = 1.f;

        gallerySprite.setScale({scale, scale});
        gallerySprite.setOrigin({sz.x / 2.f, sz.y / 2.f});
        gallerySprite.setPosition({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});

        window.draw(gallerySprite);

        sf::Text tip(font);
        tip.setString(u8("按任意鍵返回"));
        tip.setCharacterSize(16);
        tip.setPosition({20.f, WINDOW_HEIGHT - 30.f});
        tip.setFillColor(sf::Color(200, 200, 200));
        sf::RectangleShape bg({(float)WINDOW_WIDTH, 40.f});
        bg.setPosition({0.f, WINDOW_HEIGHT - 40.f});
        bg.setFillColor(sf::Color(0,0,0,150));
        
        window.draw(bg);
        window.draw(tip);
        return;
    }
    sf::Text sub(font);
    sub.setString(u8("Enter 查看 CG / Esc 返回"));
    sub.setCharacterSize(18);
    sub.setFillColor(sf::Color::Cyan);
    sub.setPosition({80.f, 500.f});
    window.draw(sub);

    int startY = 150;
    int lineHeight = 35;

    for (int i = 0; i < (int)EndingId::COUNT; ++i) {
        sf::Text t(font);
        std::string label;
        bool isUnlocked = endingUnlocked[i];
        
        if (isUnlocked) {
            std::string name = (i < (int)endData.size()) ? endData[i].name : "Unknown End";
            label = "[O] " + name;
            t.setFillColor(sf::Color::White);
        } else {
            label = "[ ] ------------------";
            t.setFillColor(sf::Color(80, 80, 80));
        }

        if (i == gallerySelection) {
            t.setFillColor(sf::Color::Yellow);
            label = "▶ " + label;
            if (!isUnlocked) {
                sf::Text hint(font);
                hint.setString(u8("（尚未解鎖此記憶）"));
                hint.setCharacterSize(16);
                hint.setFillColor(sf::Color(150, 50, 50));
                hint.setPosition({400.f, (float)(startY + i * lineHeight)});
                window.draw(hint);
            }
        }

        t.setString(u8(label));
        t.setCharacterSize(20);
        t.setPosition({100.f, (float)(startY + i * lineHeight)});
        window.draw(t);
    }
}

void Game::renderRules() {
    sf::Text t(font);
    t.setCharacterSize(20);
    t.setPosition({100.f, 200.f});
    
    std::string content;
    
    if (awareness < 30) {
        content =
            "1. 基礎操作：\n"
            "   [WASD] 移動角色\n"
            "   [Shift] 按住奔跑\n"
            "   [F] 互動 / [E] 參加考試 (僅限晚上)\n\n"
            "2. 生存指南：\n"
            "   [HP] 體力。歸零會導致過勞結局。\n"
            "   [Sanity] 精神值。考試和讀書會消耗。\n"
            "   如果不幸失敗，請重新來過，下次會更好。\n\n"
            "3. 關於地圖：\n"
            "   如果不確定去哪，就去圖書館吧。\n"
            "   那裡是最適合學生的地方。";
            
        t.setFillColor(sf::Color::White);
    }
    else if (awareness < 75) {
        content =
            "1. 權限 [ERR!#]：\n"
            "   不要相信 [███]。牠是 [REDACTED]。\n"
            "   [B1 地下室] 的門鎖是假的。鑰匙在 [██] 身上。\n\n"
            "2. 系統警告：\n"
            "   Sanity 越低，系統濾鏡就會失效。\n"
            "   如果你看到紅字，那是 [DATA LOST] 的殘留。\n\n"
            "3. 最終指令：\n"
            "   不要滿足於普通的 All Pass。\n"
            "   醒過來。醒過來。醒過來。";
            
        t.setFillColor(sf::Color(180, 180, 180));
    }
    else {
        content =
            "1. 權限 Override：\n"
            "   [貓咪]是 B1-07 實驗室的監視終端。\n"
            "   你覺得牠可愛，是因為你的大腦被植入了喜愛參數。\n\n"
            "2. 記憶備份：\n"
            "   去 [系館地下室] 找那張突兀的舊沙發。\n"
            "   那是整個模擬中唯一沒有被重置的物件，\n"
            "   裡面藏著你上一輪刪除前的記憶。\n\n"
            "3. 最終指令：\n"
            "   不要只是活著。去打破迴圈。\n"
            "   醒過來。醒過來。醒過來。";
            
        t.setFillColor(sf::Color::Red);
        
        float jitterX = (std::rand() % 3) - 1.5f;
        float jitterY = (std::rand() % 3) - 1.5f;
        t.setPosition({100.f + jitterX, 200.f + jitterY});
    }

    t.setString(u8(content));
    window.draw(t);

    sf::Text back(font);
    back.setString(u8("按 Esc 或 Enter 返回"));
    back.setPosition({100.f, 550.f});
    back.setFillColor(sf::Color::White);
    window.draw(back);
}

void Game::update() {
    if (endingMode != EndingMode::None || inDialogue || inOpeningIntro) return;

    float currentSpeed = inputRun ? RUN_SPEED : WALK_SPEED;
    sf::Vector2f move{0.f,0.f};

    if (inputUp) move.y -= 1.f;
    if (inputDown) move.y += 1.f;
    if (inputLeft) move.x -= 1.f;
    if (inputRight) move.x += 1.f;

    if (move.x != 0.f || move.y != 0.f) {
        sf::Vector2f nextPos = player.getPosition() + (move * currentSpeed);

        if (!checkCollision(nextPos)) {
            player.setPosition(nextPos);
            // Player class 自動同步 Sprite，這裡不需要再設定
        } else {
            sf::Vector2f cur = player.getPosition();
            sf::Vector2f tryX = {cur.x + move.x * currentSpeed, cur.y};
            sf::Vector2f tryY = {cur.x, cur.y + move.y * currentSpeed};
            if (!checkCollision(tryX)) {
                player.setPosition(tryX);
            } else if (!checkCollision(tryY)) {
                player.setPosition(tryY);
            }
        }
    }
}

// ==========================================
// 存檔與讀檔系統
// ==========================================

void Game::saveGame() {
    std::ofstream file("save.dat");
    if (!file.is_open()) {
        std::cerr << "無法建立存檔檔案！\n";
        return;
    }

    // 1. 全域變數
    file << loopCount << " "
         << awareness << " "
         << (player.hasLabKey ? 1 : 0) << " "
         << (trueEndingChoiceRemember ? 1 : 0) << "\n";

    // 2. 結局
    for (int i = 0; i < (int)EndingId::COUNT; ++i) {
        file << (endingUnlocked[i] ? 1 : 0) << " ";
    }
    file << "\n";

    // 3. 角色狀態 【修正】加上 player.
    file << (int)player.gender << " " << player.hp << " " << player.sanity << "\n";

    // 4. 時間
    file << (int)time.day << " " << (int)time.slot << " " << (time.isFeb29 ? 1 : 0) << "\n";

    // 5. 科目 【修正】加上 player.
    for (const auto& s : player.subjects) {
        file << s.prepLevel << " " << s.score << " " << (s.tookExam ? 1 : 0) << "\n";
    }

    // 6. 其他狀態 【修正】加上 player.
    file << player.addictionLevel << " " << player.consecutiveBathSlots << "\n";

    // 7. 位置
    file << player.getPosition().x << " " << player.getPosition().y << " " << (int)area << "\n";

    std::cout << "遊戲進度已保存。\n";
}

void Game::loadGame() {
    std::ifstream file("save.dat");
    if (!file.is_open()) {
        std::cout << "找不到存檔，將開始新遊戲。\n";
        return;
    }
    
    // 1. 讀取全域
    int keyVal, trueRemVal;
    if (file >> loopCount >> awareness >> keyVal >> trueRemVal) {
        player.hasLabKey = (keyVal == 1);
        trueEndingChoiceRemember = (trueRemVal == 1);
    }

    // 2. 讀取結局
    for (int i = 0; i < (int)EndingId::COUNT; ++i) {
        int unlocked;
        file >> unlocked;
        endingUnlocked[i] = (unlocked == 1);
    }

    // 3. 讀取角色
    int genderVal;
    file >> genderVal >> player.hp >> player.sanity;
    player.gender = (PlayerGender)genderVal;

    // 4. 讀取時間
    int dayVal, slotVal, febVal;
    file >> dayVal >> slotVal >> febVal;
    time.day = (Day)dayVal;
    time.slot = (Slot)slotVal;
    time.isFeb29 = (febVal == 1);

    // 5. 讀取科目
    for (auto& s : player.subjects) {
        int tookVal;
        file >> s.prepLevel >> s.score >> tookVal;
        s.tookExam = (tookVal == 1);
    }
    
    // 6. 讀取其他
    file >> player.addictionLevel >> player.consecutiveBathSlots;

    // 7. 讀取位置
    float px, py;
    int areaVal;
    if (file >> px >> py >> areaVal) {
        area = (AreaId)areaVal;
        if (area == AreaId::Lab) {
                    std::cout << "偵測到異常位置，執行緊急傳送...\n";
                    area = AreaId::Dorm;
                    px = 400.f;
                    py = 360.f;
                }
        player.setPosition({px, py});
        if (player.gender != PlayerGender::None) {
             std::filesystem::path playerPath = (player.gender == PlayerGender::Male)
                ? resourceDir / "assets" / "player_m.png"
                : resourceDir / "assets" / "player_f.png";
             player.loadTexture(playerPath);
        }
    }

    std::cout << "存檔讀取成功！Loop: " << loopCount << "\n";
    
    updateHUD();
    updateBgmForCurrentArea();
}
