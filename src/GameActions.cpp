#include "Game.hpp"

// ==========================================
// 結局邏輯處理：True Ending 分歧
// ==========================================
void Game::applyTrueEndingChoice(int choice) {
    bool hasPerfect = hasPerfectScoreSecret();
    if (choice == 3 && !hasPerfect) return;

    inTrueChoice = false;
    endingScenes.clear();
    endingSceneIndex = 0;

    // --- 選項 1: LOOP (保留記憶並重啟) ---
    if (choice == 1) {
        endingMode = EndingMode::TrueEndLoop;
        endingUnlocked[(int)EndingId::Normal] = true;
        trueEndingChoiceRemember = true;

        addEndingScene("觀察者看著你按下的按鈕，嘴角勾起一絲像是早就預料到的笑意。\n「我就知道。你也捨不得這裡，對吧？」", "cg_observer");
        addEndingScene("他走到玻璃前，手指輕輕劃過你的數據流：\n「外面的世界充滿了不可控的變數。但在這裡，只要你投入時間，分數就會回報你。」", "cg_observer");
        addEndingScene("畫面開始出現雜訊，傳輸線的電流聲變得尖銳。\n（我：我知道這一切都是假的。但這裡...很安全。）", "cg_observer_glitch");
        addEndingScene("【結局：沈溺者】\n你選擇了完美的牢籠。\n下一輪期末週，你將帶著記憶繼續扮演他的最佳樣本。", "");
    }
    // --- 選項 2: RESET (刪除紀錄重來) ---
    else if (choice == 2) {
        endingMode = EndingMode::TrueEndReset;
        awareness = 0; loopCount = -1; player.gender = PlayerGender::None;
        for(int i=0; i<(int)EndingId::COUNT; ++i) endingUnlocked[i] = false;

        addEndingScene("你顫抖著按下了「刪除」。\n記得太多事情，反而讓每一次的對話都像在演戲。", "cg_observer");
        addEndingScene("觀察者語氣變冷：\n「受不了了嗎？也是。確認執行：Factory Reset。」", "cg_observer");
        addEndingScene("警報聲大作。這幾天熬夜讀過的書、喝過的咖啡、在沙發上感受到的體溫......\n正在像沙子一樣從指縫流走。", "");
        addEndingScene("【系統已還原】\n你獲得了遺忘的慈悲。", "");
    }
    // --- 選項 3: REALITY (面對現實) ---
    else if (choice == 3) {
        endingMode = EndingMode::TrueEndReality;
        endingUnlocked[(int)EndingId::FinalEscape] = true;

        addEndingScene("你沒有按下任何按鈕，而是直接拔掉了連接在後頸的傳輸線。", "cg_observer");
        addEndingScene("刺耳的警報聲瞬間消失。\n周圍高科技的實驗室景象閃爍了兩下，像褪色的投影一樣消失。", "cg_observer");
        
        endingScenes.push_back({"........................", ""});
                currentBgmPath = resourceDir / "assets" / "bgm5.ogg";
                if (bgm.openFromFile(currentBgmPath)) {
                    bgm.setLooping(true);
                    bgm.setVolume(100.f);
                    bgm.play();
                }

                addEndingScene(
                    "好冷。\n\n"
                    "海風夾帶著未經濾鏡修飾的腥味，毫不留情地刮在臉上，像刀割一樣痛。\n"
                    "你坐在防波堤粗糙的水泥地上，眼前不是實驗室，\n"
                    "而是一片灰暗、混濁、波濤洶湧的冬之海。",
                    "cg_beach"
                );
                
                addEndingScene(
                    "這裡沒有恆溫空調，沒有舒適的bgm，只有真實得令人發抖的寒意。\n"
                    "你低下頭，看著螢幕裂了一角的舊手機。",
                    "cg_beach"
                );
                
                addEndingScene(
                    "沒有『期末模擬 APP』。沒有『觀察者』。沒有『數值面板』。\n"
                    "只有一排刺眼的通知卡在鎖定畫面上：\n\n"
                    "『程式設計 TA』：期末考解答已上傳，請同學確認分數...\n"
                    "『室友』：欸你人呢？要不要一起叫外送？\n"
                    "『分組作業群』：@我 你的部分改好了沒？",
                    "cg_beach"
                );
                
                addEndingScene(
                    "沒有滿分。沒有奇蹟。沒有人會因為你活下來而給你成就獎盃。\n"
                    "心臟在胸腔裡劇烈跳動，很痛，很沉重，每一口呼吸都帶著海水的苦澀。\n\n"
                    "但是，這是真的。",
                    "cg_beach"
                );

                addEndingScene(
                    "你撐著凍僵的膝蓋站起身，拍掉褲子上沾到的沙礫。\n"
                    "你最後看了一眼那片灰色的海，然後轉過身，背對這世界的盡頭，\n",
                    "cg_beach"
                );

                addEndingScene(
                    "【TRUE END：這就是現實】\n\n"
                    "恭喜通關。\n"
                    "歡迎回到這個充滿惡意、沒有攻略、卻唯一真實的世界。",
                    ""
                );
        addEndingScene(
            "【 Credits 】\n"
            "Music: 魔王魂 (https://maou.audio/)\n"
            "Graphics: DOT ILLUST (https://dot-illust.net/)\n\n"
            "Engine:SFML-3.02\n"
            "Special thanks to Google Gemini3",
            ""
        );
        addEndingScene(
            "and you.\n\n"
            ""
        );
            }
}

// ==========================================
// 對話與調查
// ==========================================
void Game::handleTalkOrInspect() {
    int   targetNPC  = -1;
    int   targetItem = -1;
    float minD2      = 1e9f;

    sf::Vector2f p = player.getPosition();

    sf::FloatRect pRect = player.getBounds();
    pRect.position.x -= 10.f; pRect.position.y -= 10.f;
    pRect.size.x     += 20.f; pRect.size.y     += 20.f;

    // 1. 搜尋最近的 NPC
    for (size_t i = 0; i < npcs.size(); ++i) {
        const NPC& n = npcs[i];
        if (n.area != area) continue;

        float dx = n.pos.x - p.x;
        float dy = n.pos.y - p.y;
        float d2 = dx*dx + dy*dy;
        float limit  = (PLAYER_RADIUS + n.radius + 16.f);
        
        if (d2 <= limit * limit && d2 < minD2) {
            minD2      = d2;
            targetNPC  = static_cast<int>(i);
            targetItem = -1;
        }
    }

    // 2. 搜尋最近的可互動物件
    for (size_t i = 0; i < items.size(); ++i) {
        const Interactable& it = items[i];
        if (it.area != area) continue;

        if (it.rect.findIntersection(pRect).has_value()) {
            sf::Vector2f center = it.rect.position + it.rect.size / 2.f;
            float dx = center.x - p.x;
            float dy = center.y - p.y;
            float d2 = dx*dx + dy*dy;
            if (d2 < minD2) {
                minD2      = d2;
                targetItem = static_cast<int>(i);
                targetNPC  = -1;
            }
        }
    }

    if (targetNPC == -1 && targetItem == -1) return;

    // ================= NPC 對話處理 =================
    if (targetNPC != -1) {
        NPC& n = npcs[targetNPC];

        // 特例：室友約會事件 (優先判定)
        if (n.name == "室友") {
            bool isDateNight =
                (time.day == Day::Wed || time.day == Day::Fri) &&
                (time.slot == Slot::Night || time.slot == Slot::LateNight);

            if (isDateNight) {
                std::string txt;
                if (player.sanity > 60) {
                    txt = "室友：欸我今天會帶人回來喔，你要不要先去圖書館待一下？\n"
                          "    不會很久啦，就…讀書讀累了，總要放鬆一下嘛。\n"
                          "（我：……原來有人可以把『帶對象回來』講得像在宣傳課外活動。）";
                } else {
                    txt = "室友：欸？你還在？我以為你又去圖書館通宵了。\n"
                          "    嗯…那個，我等一下會有『訪客』，你可以…自己找地方讀書嗎？\n"
                          "（我：這個房間好像愈來愈不像我的生活空間，只剩床位而已。）";
                }
                startDialogue(txt);
                setPortrait(n.portraitKey);
                return; // 特殊事件不消耗時間
            }
        }

        // 對話邏輯：優先觸發隱藏劇情 (Shadow) -> 否則觸發一般對話 (Normal)
        bool shadowTriggered = false;
        int selectedIndex = -1;

        // A. 篩選符合條件的隱藏對話
        std::vector<int> validIndices;
        for (size_t i = 0; i < n.shadowStages.size(); ++i) {
            const auto& s = n.shadowStages[i];
            // 【修正】player.sanity
            if (player.sanity <= s.requiredSanityMax && awareness >= s.requiredAwarenessMin) {
                validIndices.push_back(static_cast<int>(i));
            }
        }

        // B. 選擇第一個未讀的對話
        if (!validIndices.empty()) {
            for (int idx : validIndices) {
                if (idx >= (int)n.shadowStagesSeen.size()) {
                    n.shadowStagesSeen.resize(n.shadowStages.size(), false);
                }
                if (!n.shadowStagesSeen[idx]) {
                    selectedIndex = idx;
                    break;
                }
            }
        }

        // C. 播放隱藏對話
        if (selectedIndex != -1) {
            startDialogue(n.shadowStages[selectedIndex].text);
            shadowTriggered = true;
            n.shadowStagesSeen[selectedIndex] = true;
            }if (n.name == "學長" && awareness >= 60 && !player.hasLabKey) {
                player.hasLabKey = true;
                if (!shadowTriggered) {
                    startDialogue("（你趁著學長還在碎碎念，熟練地從他口袋摸走了學生證...）\n（我：反正根據經驗，他最後也是要給我的，不如我自己拿。）");
                    shadowTriggered = true;
                }   //笑死
        }

        // D. 播放一般對話 (循環)
        if (!shadowTriggered && !n.normalStages.empty()) {
            startDialogue(n.normalStages[n.dialogIndex].text);
            n.dialogIndex = (n.dialogIndex + 1) % n.normalStages.size();
        }

        setPortrait(n.portraitKey);

        // E. 時間與數值消耗邏輯
        if (n.spendsSlot && !slotUsed) {
            slotUsed = true;
            player.sanity = std::clamp(player.sanity + n.sanityDeltaOnTalk, 0, MAX_SANITY);
            checkSanityZero();
            if (endingMode != EndingMode::None) return;

            infoText.setString(u8("你和" + n.name + "的對話消耗了你的精神與時間。"));
            player.consecutiveBathSlots = 0;
            advanceSlot();
        }
        return;
    }

    // ================= 物件互動處理 =================
    if (targetItem != -1) {
        Interactable& it = items[targetItem];
        showPortrait      = false;

        switch (it.effect) {
            case ItemEffectType::Sofa:
                if (slotUsed) return;
                // 覺醒度高時觸發回憶
                if (awareness >= 90) {
                    startDialogue(
                        "【記憶碎片】\n"
                        "你躺在舊沙發上醒來，身上蓋著那件有洗衣精味的外套。\n"
                        "學長坐在不遠處。\n"
                        "他頭也沒回地說：「醒了？你的 SAN 值剛剛掉到了 5。」\n"
                        "「下次撐久一點，數據還沒跑完。」\n\n"
                        "（我：在這裡，我只是一個會呼吸的運算單元。）"
                    );
                    player.sanity = std::min(MAX_SANITY, player.sanity + 20);
                } else {
                    startDialogue(
                        "你陷進柔軟的沙發裡，周圍的伺服器運轉聲變成了白噪音。\n"
                        "雖然地點很詭異，但你卻睡得比在宿舍還安穩。"
                    );
                    player.sanity = std::min(MAX_SANITY, player.sanity + 20);
                    player.hp = std::min(MAX_HP, player.hp + 10);
                }
                slotUsed = true;
                advanceSlot();
                break;

            case ItemEffectType::Phone:          applyWasteAction();          break;
            case ItemEffectType::Bath:           applyBathAction();           break;
            case ItemEffectType::Sleep:          applySleepAction();          break;
            case ItemEffectType::Poison:         applyPoisonAction();         break;
            case ItemEffectType::Poster:         applyPosterAction();         break;
            case ItemEffectType::LabDoor:        applyLabDoorAction();        break;
            case ItemEffectType::CampusGate:     applyCampusGateAction();     break;
            case ItemEffectType::StudyGeneric:   openStudyChoice(false);      break;
            case ItemEffectType::StudyBoost:     openStudyChoice(true);       break;
            case ItemEffectType::VendingMachine: applyVendingAction();        break;
            case ItemEffectType::Food:           applyFoodAction();           break;
            
            // 場景傳送門邏輯
            case ItemEffectType::DoorCSToCampus:        warpTo(AreaId::Campus, {680.f, 380.f}, "你走出系館，眼前又是熟悉的校園廣場。"); break;
            case ItemEffectType::DoorCSNorthToSouth:    warpTo(AreaId::CSSouth, {400.f, 420.f}, "你沿著走廊往更裡面走，來到南館。"); break;
            case ItemEffectType::DoorCSSouthToNorth:    warpTo(AreaId::CSNorth, {400.f, 420.f}, "你轉身回到比較亮的北館走廊。"); break;
            case ItemEffectType::DoorCSSouthToBasement: warpTo(AreaId::CSBasement, {400.f, 420.f}, "你搭電梯往下，來到系館地下室。"); break;
            case ItemEffectType::DoorBasementToCSSouth: warpTo(AreaId::CSSouth, {400.f, 420.f}, "你沿著昏暗的樓梯走上去，回到南館走廊。"); break;
            case ItemEffectType::DoorChaosToCampus:     time.isFeb29 = false; warpTo(AreaId::Campus, {400.f, 380.f}, "你從裂縫裡跌回校園廣場，剛剛的畫面像是夢。"); break;
            case ItemEffectType::DoorLibraryToStudy:    warpTo(AreaId::LibraryStudy, {400.f, 220.f}, "你往樓上走，來到自習室。"); break;
            case ItemEffectType::DoorStudyToLibrary:    warpTo(AreaId::Library, {380.f, 260.f}, "你走下樓，回到圖書館大廳。"); break;
            case ItemEffectType::DoorLibraryToShop:     warpTo(AreaId::LibraryShop, {300.f, 150.f}, "你走下樓梯，來到 B1 便利商店。"); break;
            case ItemEffectType::DoorShopToLibrary:     warpTo(AreaId::Library, {200.f, 400.f}, "你搭電扶梯回到大廳。"); break;
            case ItemEffectType::DoorCampusToDorm:      warpTo(AreaId::Dorm, {400.f, 500.f}, "你穿過小門，回到宿舍棟樓下。"); break;
            case ItemEffectType::DoorDormToCampus:      warpTo(AreaId::Campus, {160.f, 260.f}, "你走出宿舍，腳踩在微涼的石板路上。"); break;
            case ItemEffectType::DoorCampusToLibrary:   warpTo(AreaId::Library, {260.f, 400.f}, "你推開玻璃門，冷氣和翻頁聲迎面而來。"); break;
            case ItemEffectType::DoorLibraryToCampus:   warpTo(AreaId::Campus, {680.f, 260.f}, "你走出圖書館，書頁味道被夜風沖淡了。"); break;
            case ItemEffectType::DoorCampusToCS:        warpTo(AreaId::CSNorth, {260.f, 420.f}, "你推開厚重的門，走進系館走廊。"); break;
            case ItemEffectType::DoorCampusToFoodcourt: warpTo(AreaId::Foodcourt, {160.f, 380.f}, "你穿過小門，被油煙味和人聲包圍。"); break;
            case ItemEffectType::DoorFoodcourtToCampus: warpTo(AreaId::Campus, {160.f, 380.f}, "你離開美食街，校園的空氣突然清爽很多。"); break;
            case ItemEffectType::DoorFoodcourtToCafe:   warpTo(AreaId::Cafe, {380.f, 520.f}, "玻璃門後是一個意外安靜的咖啡廳。"); break;
            case ItemEffectType::DoorCafeToFoodcourt:   warpTo(AreaId::Foodcourt, {400.f, 480.f}, "你回到吵鬧的美食街。"); break;

            default:
                if (!it.stages.empty()) {
                    startDialogue(it.stages[0].text);
                }
                break;
        }
    }
}

// ==========================================
// 系統：讀書選單
// ==========================================
void Game::openStudyChoice(bool boosted) {
    if (slotUsed) return;
    inDialogue = true;
    inStudyChoice = true;
    clearInputFlags();

    currentStudyBonus = boosted ? 2 : 1;

    std::string extra;
    if (boosted) {
        extra = "\n（在自習室讀書，比其他地方更有效率，難度會額外降低一些。）";
    } else {
        extra = "\n（隨便找個桌子翻翻講義，至少比什麼都不做好。）";
    }

    std::string line =
        "選擇複習科目 (Level 越低越好)：\n"
        "1.微積分 (Lv." + std::to_string(player.subjects[0].prepLevel) + ")  "
        "2.程式設計 (Lv."   + std::to_string(player.subjects[1].prepLevel) + ")\n"
        "3.體育   (Lv." + std::to_string(player.subjects[2].prepLevel) + ")  "
        "4.腦科學與計算導論 (Lv."+ std::to_string(player.subjects[3].prepLevel) + ")\n"
        "（Enter 取消）";

    dialogueText.setString(u8(line + extra));
    showPortrait = false;
}

// ==========================================
// 系統：執行讀書 (包含 Meta/覺醒文字)
// ==========================================
void Game::applyStudyOnSubject(Subject s) {
    if (slotUsed) return;
    if (!applyHpCost(20)) return;

    player.consecutiveBathSlots = 0;

    int idx = static_cast<int>(s);
    player.subjects[idx].prepLevel = std::max(0, player.subjects[idx].prepLevel - currentStudyBonus);
    player.sanity                 = std::max(0, player.sanity - 5);
    checkSanityZero();
    if (endingMode != EndingMode::None) return;

    std::string flavorText;

    switch (s) {
        case Subject::Brain:
            if (awareness < 20) flavorText = "課本裡詳細介紹了神經元運作與認知模型。\n（我：雖然內容很硬，但邏輯還算清晰。）";
            else if (awareness < 50) flavorText = "書中提到『虛假記憶植入』的章節：\n大腦其實無法分辨「真實經歷」與「高精度的外部電子訊號輸入」。";
            else flavorText = "書末探討『遞迴意識悖論』：\n所謂的「自我意識」，可能只是神經網路為了避免陷入局部最小值而產生的一種錯誤修正機制。\n（我：原來我的焦慮...只是演算法運算中的副作用？）";
            break;
        case Subject::Calc:
            if (awareness < 20) flavorText = "你試著解開多重積分，算式在紙上拉得很長。";
            else if (awareness < 50) flavorText = "極限符號 (lim) 看起來像是某種無限逼近卻無法到達的牆。";
            else flavorText = "這個公式不是用來計算面積，\n而是用來計算一個封閉迴圈的曲率。\n（我：我是不是被困在這些積分裡？）";
            break;
        case Subject::Prog:
            if (awareness < 20) flavorText = "你複習了物件導向的繼承結構，確保每個類別都有正確的解構函式。";
            else if (awareness < 50) flavorText = "遞迴函式寫得太深，你突然忘記遞迴的終止條件是什麼。";
            else flavorText = "關於『偽隨機數生成器』的章節指出：\n只要種子 (Seed) 固定，所有的「隨機」災難其實都是預先算好的確定性結果。\n（我：所以...這一切都是早就寫死的？）";
            break;
        case Subject::PE:
            if (awareness < 20) flavorText = "你複習了運動生理學與比賽規則。";
            else if (awareness < 50) flavorText = "講義上寫著：『適度痛覺有助於保持受試者清醒。』";
            else flavorText = "這不是在教你怎麼運動，而是在教你『如何逃跑』。\n所有的插圖，畫的都是一個人在被某種東西追逐的姿勢。";
            break;
        default: break;
    }

    std::string tail;
    if (currentStudyBonus >= 2) tail = "\n（在自習室讀書效率稍微高了一點。）";

    startDialogue("你複習了 " + subjectName(s) + "。\n\n" + flavorText + tail);
    slotUsed = true;
    advanceSlot();
}

void Game::applySleepAction() {
    if (slotUsed) return;
    player.consecutiveBathSlots = 0;
    int dayIndex = static_cast<int>(time.day);
    if (dayIndex < 0 || dayIndex >= 7) dayIndex = 0;
    sleepCountPerDay[dayIndex]++;
    player.sanity = std::min(MAX_SANITY, player.sanity + 10);
    startDialogue("你躺在床上，決定小睡一會。\n夢裡也在寫筆記，醒來時不確定剛剛算的是不是現實考題。");
    slotUsed = true;
    advanceSlot();
    player.hp = MAX_HP;
    updateHUD();
}

void Game::applyWasteAction() {
    if (slotUsed) return;
    player.consecutiveBathSlots = 0;
    player.addictionLevel++;
    player.sanity = std::min(100, player.sanity + 15);

    if (!applyHpCost(5)) return;

    std::string txt;
    if (countUnlockedEndings() <= 2)
        txt = "你打開手機，聊天室和社群的紅點像一片警告的海洋。\n你回了幾則訊息、看了幾篇限動，\n又不小心一路滑到一個不知道是誰轉貼的梗圖。\n（我：……再滑一下就去讀書。真的。）";
    else
        txt = "通知永遠看不完，頁面一直往下刷。\n（我：我是不是…又在逃避考試？感覺自己像被演算法牽著走。）";
    
    startDialogue(txt);

    // 觸發成癮結局
    if (player.addictionLevel > 8) {
        triggerBadEnd(
            "BAD END: [數位成癮]\n"
            "你反射性地打開手機，卻又反射性地關掉。\n"
            "醒來時，只有雲端備份裡還記得你的存在。",
            EndingId::Addict,
            "cg_bad_phone"
        );
        return;
    }
    slotUsed = true;
    advanceSlot();
}

void Game::applyVendingAction() {
    if (slotUsed) return;
    player.consecutiveBathSlots = 0;
    player.hp = std::min(MAX_HP, player.hp + 25);
    player.sanity = std::max(0, player.sanity - 10);
    checkSanityZero();
    if (endingMode != EndingMode::None) return;
    startDialogue("你買了一罐高濃度咖啡因飲料，一口氣灌下去。\n（我：心跳快到像在考場上發言，但至少眼皮撐得住。）");
    slotUsed = true;
    advanceSlot();
}

void Game::applyFoodAction() {
    if (slotUsed) return;
    player.consecutiveBathSlots = 0;
    player.hp = std::min(MAX_HP, player.hp + 35);
    player.sanity = std::min(MAX_SANITY, player.sanity + 5);
    startDialogue("你點了一碗熱騰騰的湯麵，\n蒸氣暫時遮住手機上的行事曆和考試日期。\n（我：至少這一刻，胃比大腦重要一點。）");
    slotUsed = true;
    advanceSlot();
}

void Game::applyBathAction() {
    if (slotUsed) return;
    // 【修正】player.consecutiveBathSlots, player.hp, player.sanity
    player.consecutiveBathSlots++;
    player.hp = std::min(MAX_HP, player.hp + 20);
    std::string txt = (player.sanity > 60) ? "熱水從肩膀一路流到指尖，肌肉慢慢鬆開。\n（我：如果期末週可以只剩這個行程就好了。）" : "水聲在狹小的浴室裡迴盪，像是有人在耳邊講悄悄話。\n（我：不對，只是水聲而已…吧。）";
    startDialogue(txt);

    if (player.consecutiveBathSlots >= 3) {
        triggerBadEnd(
            "BAD END: [溶解]\n"
            "你一遍又一遍把自己泡進熱水裡，\n"
            "直到皮膚與水的界線變得模糊。",
            EndingId::Drown,
            "cg_bad_drown"
        );
        return;
    }
    slotUsed = true;
    advanceSlot();
}

void Game::applyPoisonAction() {
    if (slotUsed) return;
    player.consecutiveBathSlots = 0;
    player.gotPoisoned = true;
    player.hp = 1; // 設定為 1，但隨即觸發 Bad End
    
    triggerBadEnd(
                  "BAD END: [食物中毒]\n"
                          "你點了一份看起來便宜又份量十足的套餐，\n"
                          "第一口就嚐到奇怪的金屬味。\n"
                          "接下來的一整晚，你的人生只剩馬桶和天花板。",
        EndingId::Poison,
        "cg_bad_poison"
    );
}

void Game::applyPosterAction() {
    std::string txt;
    if (awareness < 20) txt = "海報標題：『學術壓力下的記憶重塑研究』。\n小字寫著：『招募大學生受試者，可換禮券。』";
    else if (awareness < 50) txt = "海報內容下面多了一段細小的說明文字：\n"
        "『受試者將在模擬情境中重複經歷期末週……』\n"
        "你盯著那幾個字，腦中閃過幾張模糊的畫面：\n"
        "自己坐在陌生的機器前、有人在玻璃後面說話。\n"
        "（我：奇怪…明明沒進過實驗室，為什麼好像看過這個場景。）";
    else txt = "海報上的字開始往外散開，像被人用橡皮擦輕輕擦過：\n"
        "『樣本 #01……迭代……記憶覆寫中……』\n"
        "你突然記起，自己好像曾經在某個房間裡，\n"
        "看著一整排螢幕，螢幕上的人也是你。\n"
        "（我：是夢？還是那一段記憶被刻意放淡了？）";
    
    player.sanity = std::max(0, player.sanity - 10);
    checkSanityZero();
    if (endingMode != EndingMode::None) return;
    startDialogue(txt);
}

void Game::applyLabDoorAction() {
    if (time.slot >= Slot::Night && player.hasLabKey) {
        labUnlocked = true;
        area        = AreaId::Lab;
        startTrueEndingIntro();
    } else {
        std::string txt;
        if (!player.hasLabKey) txt = "門牌上寫著一串正式的實驗室名稱，還有一些看不懂的英文縮寫。\n（我：就是這裡…撐過這次期末考是第一步。）";
        else if (time.slot < Slot::Night)
                txt="雖然有卡片，但現在是大白天，進去太顯眼了。\n（我：等到晚上沒人的時候再來吧。）";
        else txt = "門上多了一張貼紙：『未經許可之人員禁止進入實驗區』。\n（我：…實驗「區」？聽起來不像一般教學實驗室。）";
        showPortrait = false;
        startDialogue(txt);
    }
}

void Game::applyCampusGateAction() {
    showPortrait = false;
    std::string txt;
    if (awareness < 30) txt = "……\n你站在校門口，看著外面稀稀落落的車和行人。\n（我：這週先撐過期末吧。等考完再好好離開這裡也不遲。）";
    else if (awareness < 60) txt = "……\n"
        "你往外多看了幾秒，突然有種奇怪的違和感。\n"
        "路人走路的節奏、車子經過的頻率，像是同一段影片在重播。\n"
        "（我：是不是太累了，才會覺得畫面卡住。）";
    else txt = "……\n"
        "你往外踏出一步，畫面卻像被誰按下了暫停。\n"
        "下一瞬間，你又站回校門內側，腳印只留在同一塊地板上。\n"
        "校門之外的景色像是貼在牆上的大照片，永遠不會再近一點。\n"
        "（我：原來不是我不敢離開校園，而是程式根本沒寫出去的世界。）";
    startDialogue(txt);
}

// ==========================================
// 小遊戲邏輯：考試模組
// ==========================================
void Game::playExamBgm() {
    if (bgm4Path.empty()) return;
    if (!bgm.openFromFile(bgm4Path)) {
        std::cerr << "載入考試 BGM 失敗: " << bgm4Path << "\n";
        return;
    }
    currentBgmPath = bgm4Path;
    bgm.setLooping(true);
    bgm.play();
}

int Game::playExamMiniGame(Subject subj) {
    inExamMiniGame = true;
    playExamBgm();

    int prep = player.subjects[(int)subj].prepLevel;
    float duration = 30.f;
    sf::Clock timer;

    sf::Color enemyColor = sf::Color::Red;
    float baseEnemySpeed = 320.f + (prep * 260.f);
    float spawnRate      = std::max(2.f, 18.f - prep * 3);
    float enemySize      = 28.f + prep * 7.f;

    if (subj == Subject::Prog) enemyColor = sf::Color::Green;
    if (subj == Subject::PE) { baseEnemySpeed *= 1.3f; enemyColor = sf::Color::Yellow; }
    if (awareness > 50) enemyColor = sf::Color(0, 255, 0, 180);

    std::vector<sf::RectangleShape> enemies;
    sf::RectangleShape ship({22.f, 22.f});
    ship.setFillColor(sf::Color::Cyan);
    ship.setOrigin({11.f, 11.f});
    ship.setPosition({400.f, 550.f});

    bool dead = false;
    bool miniLeft = false, miniRight = false;
    bool miniFast = false;

    while (window.isOpen() && timer.getElapsedTime().asSeconds() < duration && !dead) {
        float dt = 0.016f;
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (event->is<sf::Event::FocusLost>()) {
                    miniLeft = false;
                    miniRight = false;
                    miniFast = false;
                }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Left  || key->scancode == sf::Keyboard::Scancode::A || key->code == sf::Keyboard::Key::A) miniLeft  = true;
                if (key->code == sf::Keyboard::Key::Right || key->scancode == sf::Keyboard::Scancode::D || key->code == sf::Keyboard::Key::D) miniRight = true;
                if (key->code == sf::Keyboard::Key::LShift || key->code == sf::Keyboard::Key::RShift) miniFast = true;
            }
            if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->code == sf::Keyboard::Key::Left  || key->scancode == sf::Keyboard::Scancode::A || key->code == sf::Keyboard::Key::A) miniLeft  = false;
                if (key->code == sf::Keyboard::Key::Right || key->scancode == sf::Keyboard::Scancode::D || key->code == sf::Keyboard::Key::D) miniRight = false;
                if (key->code == sf::Keyboard::Key::LShift || key->code == sf::Keyboard::Key::RShift) miniFast = false;
            }
        }

        float speed = miniFast ? 500.f : 300.f;
        if (miniLeft)  ship.move({-speed * dt, 0.f});
        if (miniRight) ship.move({ speed * dt, 0.f});

        if (ship.getPosition().x < 10)  ship.setPosition({10.f, 550.f});
        if (ship.getPosition().x > 790) ship.setPosition({790.f, 550.f});

        if (std::rand() % static_cast<int>(spawnRate) == 0) {
            sf::RectangleShape e({enemySize, enemySize});
            e.setPosition({static_cast<float>(std::rand() % 780 + 10), -enemySize});
            e.setFillColor(enemyColor);
            enemies.push_back(e);
        }

        for (auto& e : enemies) {
            e.move({0.f, baseEnemySpeed * dt});
            if (e.getGlobalBounds().findIntersection(ship.getGlobalBounds()).has_value()) {
                dead = true;
            }
        }

        window.clear(sf::Color::Black);
        if (prep == 4) window.clear(sf::Color(30, 0, 0));

        float remainingTime = duration - timer.getElapsedTime().asSeconds();
        if (remainingTime < 0) remainingTime = 0;
        
        float barWidth = (remainingTime / duration) * static_cast<float>(WINDOW_WIDTH);
        
        sf::RectangleShape timeBar({barWidth, 5.f});
        timeBar.setFillColor(sf::Color::White);
        window.draw(timeBar);

        window.draw(ship);
        for (const auto& e : enemies) window.draw(e);
        window.display();
        sf::sleep(sf::seconds(dt));
    }

    float survivedTime = timer.getElapsedTime().asSeconds();
    int finalScore = static_cast<int>((survivedTime / duration) * 100.f);
    if (dead && finalScore == 100) finalScore = 99;

    inExamMiniGame = false;
    updateBgmForCurrentArea();

    return finalScore;
}
