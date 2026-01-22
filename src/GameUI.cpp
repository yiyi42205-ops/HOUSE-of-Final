#include "Game.hpp"

void Game::updateHUD() {
    std::string dName = time.isFeb29 ? "2月29日" : dayName(time.day);
    std::string line = "日期：" + dName + " | 時段：" + slotName(time.slot) + "\n";
    line += "地點：" + areaName(area) + "\n";
    
    line += "HP：" + std::to_string(player.hp) + " | Sanity：" + std::to_string(player.sanity) + "\n";

    line += "\n[準備度]\n";
    line += "微積:" + std::to_string(player.subjects[0].prepLevel) + " ";
    line += "程式:" + std::to_string(player.subjects[1].prepLevel) + "\n";
    line += "體育:" + std::to_string(player.subjects[2].prepLevel) + " ";
    line += "腦科:" + std::to_string(player.subjects[3].prepLevel) + "\n";
    bool useGlitch = (player.sanity <= 40);

    sf::String finalStr = useGlitch ? glitchText(line, player.sanity) : u8(line);
    hudText.setString(finalStr);
    hudText.setFillColor(player.sanity < 30 ? sf::Color::Red : sf::Color::White);
    updateBgmForCurrentArea();
}

// 開啟對話框模式
void Game::startDialogue(const std::string& msg) {
    inDialogue = true;
    clearInputFlags();

    int unlocked = countUnlockedEndings();
    bool useGlitch = (player.sanity <= 50) || (unlocked > static_cast<int>(EndingId::COUNT) / 2);

    sf::String s = useGlitch ? glitchText(msg, player.sanity) : u8(msg);
    dialogueText.setString(s);
}

// 推進時間時段
void Game::advanceSlot() {
    slotUsed = false;

    int s = static_cast<int>(time.slot) + 1;
    if (s >= static_cast<int>(Slot::COUNT)) {
        s = 0;
        int d = static_cast<int>(time.day) + 1;
        if (d >= static_cast<int>(Day::COUNT)) {
            checkWeekEnd();
            return;
        }
        time.day = static_cast<Day>(d);
    }
    time.slot = static_cast<Slot>(s);
    relocateCat();
    
    // 隨機觸發雜訊空間事件
    if (awareness >= 70 && std::rand() % 20 == 0) {
        time.isFeb29 = true;
        sf::Vector2f chaosSpawn{400.f, 320.f};
        warpTo(
            AreaId::ChaosRift,
            chaosSpawn,
            "牆上排列著一整片顯示器，每一個都播放著不同的期末週。\n"
            "（我：……？）"
        );
        return;
    }

    updateHUD();
}

// 加入結局場景頁面
void Game::addEndingScene(const std::string& txt, const std::string& imgKey) {
    endingScenes.push_back({txt, imgKey});
}

// 檢查是否到達週末結算點
void Game::checkWeekEnd() {
    int totalSleep = 0;
    for (int c : sleepCountPerDay) totalSleep += c;

    if (totalSleep >= 20) {
        triggerBadEnd(
            "BAD END: [植物人]\n"
            "你放棄了，在宿舍和床的迴圈裡打轉，\n"
            "期末考、作業、訊息通通變成遠方模糊的噪音。\n"
            "最後，只有點名系統還記得你的學號。",
            EndingId::SleepTooMuch,
            "cg_bad_sleep"
        );
    } else {
        triggerNormalEnding();
    }
}

// 觸發壞結局邏輯
void Game::triggerBadEnd(const std::string& text, EndingId id, const std::string& imgKey) {
    endingUnlocked[(int)id] = true;
    endingMode = EndingMode::NormalOrBad;
    clearInputFlags();

    endingScenes.clear();
    endingSceneIndex = 0;
    
    addEndingScene(text, imgKey);
    
    int unlockedCount = countUnlockedEndings();
    std::string stats = "\n\n目前解鎖進度： " + std::to_string(unlockedCount) + " / " + std::to_string((int)EndingId::COUNT);

    std::string resetMsg =
        "（我：……這不是我要的結果。應該可以做得更好吧？）\n"
        + stats +
        "\n\n(按 Enter 醒來，開始新的一週)";
    
    addEndingScene(resetMsg, "");
}

// 觸發普通結局結算
void Game::triggerNormalEnding() {
    endingUnlocked[(int)EndingId::Normal] = true;
    endingMode = EndingMode::NormalOrBad;
    clearInputFlags();
    
    endingScenes.clear();
    endingSceneIndex = 0;

    double scoreSum = 0;
    for (auto& s : player.subjects) scoreSum += (s.score == -1 ? 0 : s.score);
    double avg = scoreSum / 4.0;

    std::string rank;
    std::string rankComment;
    
    if (avg < 60) {
        rank = "D";
        rankComment = "這一週幾乎是被考試追著跑，\n你只是勉強把每一天撐過去，連看成績的勇氣都沒有。";
    }
    else if (avg < 75) {
        rank = "C";
        rankComment = "有幾科差點爆炸，但總算沒有整個崩盤，\n你知道自己其實可以表現得更穩一點。";
    }
    else if (avg < 85) {
        rank = "B";
        rankComment = "大部分時候都還算掌握節奏，\n偶爾失控，卻也在極限前把分數拉了回來。";
    }
    else if (avg < 100) {
        rank = "A";
        rankComment = "從外表看起來一切都在掌握之中，\n只有你自己知道那些被壓下去的焦慮雜訊。";
    }
    else {
            rank = "A+ (Perfect)";
            rankComment = "你看著螢幕上的學期成績單，上面整齊地排列著：\n微積分：100 / 程式設計：100 / 體育：100 / 腦科學與計算導論：100\n\n（我：完美的數據。沒有一題失誤，沒有一秒浪費。\n     這張成績單乾淨得像是由機器產出的報表......\n     雖然拿到了滿分，卻有一種說不出的違和感。）";
            endingUnlocked[(int)EndingId::AllPerfect] = true;
        }
    std::string page1 = "【這一週的結果】\n\n本週平均成績：約 " + std::to_string((int)avg) + "\n";
    page1 += "總評等級： " + rank + "\n\n";
  
    addEndingScene(page1, "");
    addEndingScene(rankComment, "");
    addEndingScene("考完最後一科，你慢慢走出教室。\n走廊上的空氣混雜著興奮的討論聲與懊悔的嘆息，\n有人在對答案，有人已經在揪團去吃慶功宴。你沒有加入任何一個話題，只是把帽T的帽子戴上，順著人潮往外走。\n", "cg_dorm_messy");
    addEndingScene("回到宿舍時，房間安靜得像另一個世界。\n桌上還維持著『戰場』的模樣：散亂的講義、乾掉的咖啡漬和貼滿螢幕的便利貼。", "cg_dorm_messy");
    addEndingScene("你躺回床上，滑著這週的行事曆，\n那些以為會過不去的日子，現在都壓縮成了成績單上的一個數字。", "cg_dorm_messy");
    addEndingScene("沒有人會知道你在那些半夜崩潰過幾次，系統只在意最後的結果。\n但你很清楚，這些混亂、焦慮與掙扎，都是真的發生過。", "cg_dorm_messy");

    int unlockedCount = countUnlockedEndings();
    std::string stats = "\n[解鎖進度： " + std::to_string(unlockedCount) + " / " + std::to_string((int)EndingId::COUNT) + "]";

    std::string lastThought = (avg >= 100)
        ? "（我：做到這樣......應該就有資格去見「他」了吧？）"
        : "（我：......下一次，應該可以做得更好吧。）";
    
    addEndingScene(lastThought, "cg_dorm_messy");
    addEndingScene("(按 Enter 結束本週)" + stats, "");
}

// 觸發真結局前導
void Game::startTrueEndingIntro() {
    endingUnlocked[(int)EndingId::TrueEnd] = true;
    endingMode = EndingMode::TrueIntro;
    inTrueChoice = true;
    clearInputFlags();
    showPortrait = false;

    endingScenes.clear();
    endingSceneIndex = 0;

    bool hasPerfect = hasPerfectScoreSecret();

    std::string p1;
    p1 += "【TRUE END：樣本 #01 的抉擇】\n\n";
    p1 += "你推開厚重的門，冷氣和機器的噪音同時撲面而來。\n";
    p1 += "牆上一整排螢幕，播著不同角度、不同輪迴的「你」。";
    addEndingScene(p1, "cg_observer");

    std::string p2;
    p2 += "玻璃後面的那個身影轉過身來，螢幕的光映在他的眼鏡上：\n";
    p2 += "「樣本 #01，第 " + std::to_string(loopCount + 1) + " 輪壓力測試結束。」\n";
    p2 += "「你的數據很漂亮，讓我捨不得結束這個專案。」";
    addEndingScene(p2, "cg_observer");

    std::string p3;
    if (!hasPerfect) {
        p3 += "（觀察者看著你的成績單，輕輕搖頭。）\n";
        p3 += "「可惜。如果你能把每一科都做到『完美』(All Perfect)，或許才有資格跟我談條件。」\n";
        p3 += "「但現在，你只能從這兩個爛蘋果裡選一個。」\n\n";
    } else {
        p3 += "（觀察者看著你的全滿分成績單，眼神變得警惕。）\n";
        p3 += "「...沒想到樣本能進化到這種程度。看來你的變異已經超出了預期。」\n\n";
    }

    p3 += "1) 【LOOP】保留記憶並重啟 (解鎖結局數保留)\n";
    p3 += "2) 【RESET】刪除紀錄重來 (清除存檔)\n";

    if (hasPerfect) {
        p3 += "3) 【REALITY】拔掉傳輸線 (面對現實)\n";
    }

    p3 += hasPerfect
        ? "\n觀察者正等待你的輸入。請按 1、2 或 3。"
        : "\n觀察者正等待你的輸入。請按 1 或 2。";
    
    addEndingScene(p3, "cg_observer");
}

// 設定當前立繪
void Game::setPortrait(const std::string& key) {
    currentPortraitKey.clear();
    showPortrait = false;
    if (key.empty()) return;

    std::string finalKey = key;
    if (key.rfind("MC_", 0) == 0) {
        if (player.gender == PlayerGender::Male) {
            finalKey = key + "_m";
        } else if (player.gender == PlayerGender::Female ||
                   player.gender == PlayerGender::None) {
            finalKey = key + "_f";
        }
    }

    auto it = portraitTextures.find(finalKey);
    if (it == portraitTextures.end()) {
        sf::Texture tex;
        auto path = resourceDir / "assets" / "portraits" / (finalKey + ".png");
        if (!tex.loadFromFile(path)) {
            std::cerr << "Failed to load portrait: " << path << "\n";
            if (finalKey != key) {
                auto fallbackPath = resourceDir / "assets" / "portraits" / (key + ".png");
                if (tex.loadFromFile(fallbackPath)) {
                    tex.setSmooth(true);
                    it = portraitTextures.emplace(key, std::move(tex)).first;
                } else {
                    return;
                }
            } else {
                return;
            }
        } else {
            tex.setSmooth(true);
            it = portraitTextures.emplace(finalKey, std::move(tex)).first;
        }
    }

    if (!portraitSprite) {
        portraitSprite.emplace(it->second);
    } else {
        portraitSprite->setTexture(it->second, true);
    }

    auto sz = it->second.getSize();
    float targetHeight = 220.f;
    float scale = targetHeight / static_cast<float>(sz.y == 0 ? 1 : sz.y);
    portraitSprite->setScale({scale, scale});

    showPortrait = true;
    currentPortraitKey = finalKey;
}

// 檢查 Sanity 歸零狀況
void Game::checkSanityZero() {
    if (player.sanity <= 0 && endingMode == EndingMode::None) {
        triggerBadEnd(
            "BAD END: [系統崩潰]\n"
            "你的思緒像被強制關機的電腦，\n"
            "所有科目、考試日期和人名在腦中全變成扭曲的符號。\n"
            "等你回過神，只剩下螢幕前微弱的反光，\n"
            "沒有人記得這一輪的你是哪一個版本。",
            EndingId::SystemFailure,
            "cg_bad_overwork"
        );
    }
}

// 根據區域更新背景音樂
void Game::updateBgmForCurrentArea() {
    if (inExamMiniGame) return;

    if (time.isFeb29 || area == AreaId::ChaosRift) {
        bgm.stop();
        currentBgmPath.clear();
        return;
    }

    const std::filesystem::path* newPath = nullptr;

    if (player.sanity <= 30) {
        newPath = &bgm3Path;
    } else if (area == AreaId::CSNorth ||
               area == AreaId::CSSouth ||
               area == AreaId::CSBasement ||
               area == AreaId::LibraryStudy) {
        newPath = &bgm2Path;
    } else {
        newPath = &bgm1Path;
    }

    if (!newPath || newPath->empty()) {
        bgm.stop();
        currentBgmPath.clear();
        return;
    }

    if (*newPath == currentBgmPath) return;

    currentBgmPath = *newPath;

    if (!bgm.openFromFile(currentBgmPath)) {
        std::cerr << "載入 BGM 失敗: " << currentBgmPath << "\n";
        currentBgmPath.clear();
        return;
    }

    bgm.setLooping(true);
    bgm.play();
}

// 執行性別選擇畫面
void Game::runGenderSelectScreen() {
    sf::Texture femaleTex, maleTex;

    auto femalePath = resourceDir / "assets" / "player_f.png";
    auto malePath   = resourceDir / "assets" / "player_m.png";

    if (!femaleTex.loadFromFile(femalePath)) { std::cerr << "Failed to load female sprite\n"; }
    if (!maleTex.loadFromFile(malePath)) { std::cerr << "Failed to load male sprite\n"; }

    sf::Sprite femaleSpr{femaleTex};
    sf::Sprite maleSpr{maleTex};

    const float targetHeight = 120.f;

    if (femaleTex.getSize().y > 0) {
        float fScale = targetHeight / static_cast<float>(femaleTex.getSize().y);
        femaleSpr.setScale({fScale, fScale});
    }
    if (maleTex.getSize().y > 0) {
        float mScale = targetHeight / static_cast<float>(maleTex.getSize().y);
        maleSpr.setScale({mScale, mScale});
    }

    auto sz = window.getSize();
    float y = sz.y / 2.f - targetHeight;
    
    femaleSpr.setPosition({sz.x / 2.f - 160.f, y});
    maleSpr.setPosition  ({sz.x / 2.f +  40.f, y});

    sf::RectangleShape selector;
    selector.setFillColor(sf::Color::Transparent);
    selector.setOutlineColor(sf::Color::White);
    selector.setOutlineThickness(4.f);

    sf::Text title{font};
    title.setCharacterSize(34);
    auto tb = title.getGlobalBounds();
    title.setPosition({sz.x / 2.f - tb.size.x / 2.f, 80.f});

    sf::Text tip{font};
    tip.setString(u8("← → 切換　Enter 確認"));
    tip.setCharacterSize(22);
    auto tipb = tip.getGlobalBounds();
    tip.setPosition({sz.x / 2.f - tipb.size.x / 2.f, sz.y - 90.f});

    int  selected = 0;
    bool done     = false;

    while (!done && window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Left  || key->code == sf::Keyboard::Key::A ||
                    key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
                    selected = 1 - selected;
                }
                if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                    player.gender = (selected == 0) ? PlayerGender::Female : PlayerGender::Male;
                    done = true;
                }
            }
        }

        const sf::Sprite& spr = (selected == 0) ? femaleSpr : maleSpr;
        auto b = spr.getGlobalBounds();
        
        selector.setSize({b.size.x + 20.f, b.size.y + 20.f});
        selector.setPosition({b.position.x - 10.f, b.position.y - 10.f});

        window.clear(sf::Color(15, 15, 25));
        window.draw(femaleSpr);
        window.draw(maleSpr);
        window.draw(selector);
        window.draw(title);
        window.draw(tip);
        window.display();
    }
}
