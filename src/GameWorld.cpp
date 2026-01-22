#include "Game.hpp"

// 碰撞檢測邏輯
bool Game::checkCollision(const sf::Vector2f& pos) {
    sf::FloatRect playerBounds(
        {pos.x - PLAYER_RADIUS + 5, pos.y - PLAYER_RADIUS + 5},
        {(PLAYER_RADIUS*2)-10, (PLAYER_RADIUS*2)-10}
    );

    if (pos.x < PLAYER_RADIUS) return true;
    if (pos.x > WINDOW_WIDTH - PLAYER_RADIUS) return true;
    if (pos.y < 150.f) return true;
    if (pos.y > 580.f) return true;

    for (const auto& obs : obstacles) {
        if (obs.area == area) {
            if (obs.rect.findIntersection(playerBounds).has_value()) {
                return true;
            }
        }
    }

    for (const auto& npc : npcs) {
        if (npc.area == area) {
            float dx = pos.x - npc.pos.x;
            float dy = pos.y - npc.pos.y;
            float distSqr = dx*dx + dy*dy;
            float minD = PLAYER_RADIUS + npc.radius;
            if (distSqr < minD * minD) return true;
        }
    }
    return false;
}

// 初始化世界地圖與障礙物
void Game::setupWorld() {
    obstacles.clear();
    // 輔助函式：建立牆壁
    auto addWall = [&](AreaId a, float x, float y, float w, float h, sf::Color c, std::string tex = "") {
        obstacles.push_back({sf::FloatRect({x, y}, {w, h}), c, a, tex});
    };

    // ==========================================
    // 定義牆壁與地板素材
    // ==========================================
    std::string wallBlack = "maptile_renga_black_01_matt.png"; // 黑色磚牆 (戶外/地下室)
    std::string wallWhite = "maptile_renga_white_02.png";      // 白色磚牆 (室內)
    std::string marble    = "maptile_dairiseki_white.png";     // 大理石 (圖書館)
    std::string chaosTile = "pattern_blackberry_02.png";       // 雜訊磁磚

    // ===== Dorm (宿舍) =====
    addWall(AreaId::Dorm, 260, 280, 260, 60, sf::Color(120,120,120), ""); // 床
    addWall(AreaId::Dorm, 80, 340, 120, 120, sf::Color(90,90,90), "");    // 浴室背景
    addWall(AreaId::Dorm, 0, 0, 800, 150, sf::Color(40,40,40), wallWhite); // 牆壁

    // ===== Campus (校園) =====
    addWall(AreaId::Campus, 0, 0, 800, 150, sf::Color(30, 60, 80), wallBlack); // 戶外牆
    addWall(AreaId::Campus, 0, 500, 800, 100, sf::Color(40, 40, 40), wallBlack);
    addWall(AreaId::Campus, 300, 260, 200, 80, sf::Color(60, 80, 90), wallBlack);

    // ===== Cafe (咖啡廳) =====
    addWall(AreaId::Cafe, 0, 0, 800, 150, sf::Color(70, 50, 40), wallWhite); // 室內
    addWall(AreaId::Cafe, 100, 260, 600, 20, sf::Color(90, 70, 60), wallWhite);

    // ===== Foodcourt (美食街) =====
    addWall(AreaId::Foodcourt, 0, 0, 800, 150, sf::Color(90, 60, 40), wallWhite); // 室內
    addWall(AreaId::Foodcourt, 0, 500, 800, 100, sf::Color(60, 40, 30), wallWhite);
    addWall(AreaId::Foodcourt, 190, 260, 170, 60, sf::Color(130,130,130), wallWhite);
    addWall(AreaId::Foodcourt, 440, 260, 170, 60, sf::Color(110,110,110), wallWhite);

    // ===== Library (圖書館) =====
    addWall(AreaId::Library, 80, 160, 640, 50, sf::Color(150,150,150), wallWhite);
    addWall(AreaId::Library, 350, 350, 100, 100, sf::Color(100,100,100), marble);

    // ===== Library Study (自習室) =====
    addWall(AreaId::LibraryStudy, 100, 100, 600, 40, sf::Color(200,200,200), wallWhite);
    addWall(AreaId::LibraryStudy, 100, 250, 600, 20, sf::Color(120,120,120), wallWhite);

    // ===== CS North/South (系館) =====
    addWall(AreaId::CSNorth, 300, 200, 200, 150, sf::Color(140,140,140), wallWhite);
    addWall(AreaId::CSSouth, 160, 270, 480, 80, sf::Color(150,150,150), wallWhite);

    // ===== CS Basement (地下室) =====
    addWall(AreaId::CSBasement, 0, 0, 800, 150, sf::Color(20,20,20), wallBlack); // 地下室

    // ===== Chaos Rift (雜訊空間) =====
    addWall(AreaId::ChaosRift, 0, 0, 800, 250, sf::Color::Magenta, chaosTile);
    addWall(AreaId::ChaosRift, 0, 400, 800, 200, sf::Color::Green, "");
}

// 初始化可互動道具
void Game::setupItems() {
    items.clear();
    
    // ==========================================
    // 1. 定義素材資源
    // ==========================================
    // 基礎物件
    std::string doorImg      = "door_okugai_02.png";
    std::string stairsImg    = "kaidan_front_black_right_carpet_blue.png";
    std::string trashImg     = "gomibako.png";
    std::string bathImg      = "maptile_mizu.png";
    std::string laptopImg    = "pc_note.png";
    
    // 建築物 (攤位)
    std::string houseRed     = "ie_side_01_red.png";
    std::string houseBlue    = "ie_side_03_blue.png";
    std::string houseBrown   = "ie_side_01_brown.png";

    // 家具
    std::string tableWood    = "table_square_01.png";
    std::string tableWhite   = "table_square_01_tablecloth.png";
    std::string shelfSmall   = "shelf_hondana_02.png";
    std::string shelfBig     = "shelf_hondana_03.png";

    // 海報
    std::string posterYel    = "shojo_tate.png";
    std::string posterRed    = "nengajo_yokogaki.png";
    std::string posterGreen  = "hagaki_yokogaki.png";

    // 食物與電器
    std::string foodNoodle   = "bento_aluminum_round_02.png";
    std::string jarPot       = "tsubo_brown_01.png";
    std::string drinkTea     = "petbottle_01_uroncha.png";
    std::string pieWhole     = "tart_blueberry_mint_whole.png";
    std::string toasterOn    = "toaster_on_black.png";

    // ==========================================
    // 2. 設定物件屬性
    // ==========================================

    // ===== Campus =====
    items.push_back({AreaId::Campus, {{40.f, 220.f}, {40.f, 80.f}}, sf::Color(120, 160, 200), "往宿舍", ItemEffectType::DoorCampusToDorm, {{"玻璃門後面是宿舍棟的樓梯間。\n（我：從這裡鑽回去，好像比正門還熟。）"}}, doorImg});
    items.push_back({AreaId::Campus, {{40.f, 340.f}, {40.f, 80.f}}, sf::Color(180, 140, 90), "往美食街", ItemEffectType::DoorCampusToFoodcourt, {{"門後是熱騰騰的油煙和吵鬧聲。\n（我：肚子叫得比鬧鐘還準時。）"}}, doorImg});
    items.push_back({AreaId::Campus, {{720.f, 220.f}, {40.f, 80.f}}, sf::Color(130, 180, 210), "往圖書館", ItemEffectType::DoorCampusToLibrary, {{"小門後面是圖書館一樓大廳。\n（我：門一打開，又是閱讀燈和翻頁聲。）"}}, doorImg});
    items.push_back({AreaId::Campus, {{720.f, 340.f}, {40.f, 80.f}}, sf::Color(150, 150, 200), "往系館", ItemEffectType::DoorCampusToCS, {{"玻璃門貼滿專題海報，門後是資工北館。\n（我：看起來裡面的人都不睡覺。）"}}, doorImg});
    items.push_back({AreaId::Campus, {{360.f, 160.f}, {80.f, 60.f}}, sf::Color(200, 200, 220), "校門", ItemEffectType::CampusGate, {{}}, ""});
    items.push_back({AreaId::Campus, {{320.f, 420.f}, {120.f, 40.f}}, sf::Color(140, 120, 90), "校園長椅", ItemEffectType::StudyGeneric, {{"木椅有一點磨損的痕跡。\n（我：在這裡讀書，容易一邊看雲一邊恍神。）"}}, tableWood});

    // ===== Dorm =====
    items.push_back({AreaId::Dorm, {{540.f, 420.f}, {60.f, 40.f}}, sf::Color(80, 160, 255), "手機", ItemEffectType::Phone, {{}}, ""});
    items.push_back({AreaId::Dorm, {{620.f, 360.f}, {60.f, 40.f}}, sf::Color(160, 140, 100), "宿舍書桌", ItemEffectType::StudyGeneric, {{"桌上堆著沒寫完的作業和咖啡漬。\n（我：在這裡讀書，很容易順手打開社群。）"}}, laptopImg});
    items.push_back({AreaId::Dorm, {{360.f, 520.f}, {80.f, 40.f}}, sf::Color(120, 160, 200), "往校園", ItemEffectType::DoorDormToCampus, {{"你推開宿舍一樓的側門，外面是微涼的空氣。\n（我：一出來就看到操場的燈光。）"}}, doorImg});
    items.push_back({AreaId::Dorm, {{80.f, 340.f}, {120.f, 120.f}}, sf::Color(0, 200, 200), "浴室", ItemEffectType::Bath, {{}}, bathImg});
    items.push_back({AreaId::Dorm, {{260.f, 280.f}, {260.f, 60.f}}, sf::Color(200, 200, 255), "床", ItemEffectType::Sleep, {{}}, ""});

    // ===== Library =====
    items.push_back({AreaId::Library, {{360.f, 520.f}, {80.f, 40.f}}, sf::Color(120, 160, 200), "往校園", ItemEffectType::DoorLibraryToCampus, {{"玻璃門外是校園廣場，幾盞路燈孤零零地亮著。"}}, doorImg});
    items.push_back({AreaId::Library, {{500.f, 420.f}, {120.f, 40.f}}, sf::Color(190, 190, 190), "閱覽桌", ItemEffectType::StudyGeneric, {{"桌面上有被翻得卷起來的參考書。\n（我：在大廳讀書，比自習室多一點人味。）"}}, tableWhite});
    items.push_back({AreaId::Library, {{580.f, 160.f}, {80.f, 50.f}}, sf::Color::Red, "實驗海報", ItemEffectType::Poster, {{}}, posterRed});
    items.push_back({AreaId::Library, {{120.f, 170.f}, {80.f, 40.f}}, sf::Color(150, 150, 200), "徵求助研", ItemEffectType::None, {{"海報上寫著：『徵求研究助理，條件：抗壓強、能熬夜。』\n（我：感覺每一條都在寫我現在的生活。）"}}, posterRed});
    items.push_back({AreaId::Library, {{220.f, 170.f}, {80.f, 40.f}}, sf::Color(150, 200, 150), "諮商中心宣傳", ItemEffectType::None, {{"海報上印著微笑的卡通腦袋：『覺得撐不下去的時候，我們在這裡。』\n（我：…好想打電話，又覺得沒那麼嚴重。）"}}, posterGreen});
    items.push_back({AreaId::Library, {{380.f, 230.f}, {40.f, 40.f}}, sf::Color::Yellow, "往自習室", ItemEffectType::DoorLibraryToStudy, {{"往 2F 自習室。"}}, stairsImg});
    items.push_back({AreaId::Library, {{200.f, 450.f}, {40.f, 40.f}}, sf::Color(80, 180, 255), "往便利商店", ItemEffectType::DoorLibraryToShop, {{"往 B1 便利商店。"}}, stairsImg});
    items.push_back({AreaId::Library, {{80.f, 380.f}, {80.f, 60.f}}, sf::Color(90, 90, 120), "置物櫃", ItemEffectType::None, {{"幾個置物櫃的門微微開著。\n（我：以前會把手機鎖在裡面，現在只會握緊不放。）"}}, shelfBig});

    // ===== Library Study =====
    items.push_back({AreaId::LibraryStudy, {{160.f, 280.f}, {480.f, 60.f}}, sf::Color::White, "自習桌", ItemEffectType::StudyBoost, {{"長桌被切割成一格一格，每一格都有人曾經崩潰過。\n（我：在這裡讀書，好像可以比較專心一點。）"}}, tableWhite});
    items.push_back({AreaId::LibraryStudy, {{380.f, 360.f}, {40.f, 40.f}}, sf::Color::Yellow, "回大廳", ItemEffectType::DoorStudyToLibrary, {{"回到圖書館大廳。"}}, stairsImg});
    items.push_back({AreaId::LibraryStudy, {{120.f, 320.f}, {30.f, 40.f}}, sf::Color(80, 80, 80), "垃圾桶", ItemEffectType::None, {{"垃圾桶裡有一堆揉爛的講義。\n（我：如果能一起把焦慮丟掉就好了。）"}}, trashImg});

    // ===== Library Shop =====
    items.push_back({AreaId::LibraryShop, {{200.f, 100.f}, {40.f, 40.f}}, sf::Color(80, 180, 255), "回大廳", ItemEffectType::DoorShopToLibrary, {{"回到圖書館大廳。"}}, stairsImg});
    items.push_back({AreaId::LibraryShop, {{350.f, 300.f}, {100.f, 80.f}}, sf::Color::Green, "販賣機", ItemEffectType::VendingMachine, {{}}, drinkTea});
    items.push_back({AreaId::LibraryShop, {{500.f, 260.f}, {80.f, 80.f}}, sf::Color(200, 180, 140), "餅乾架", ItemEffectType::None, {{"架上全是高熱量餅乾。\n（我：期末週的體脂率不在成績單上，先不管。）"}}, pieWhole});

    // ===== Foodcourt =====
    items.push_back({AreaId::Foodcourt, {{40.f, 260.f}, {40.f, 80.f}}, sf::Color(120, 160, 200), "回校園", ItemEffectType::DoorFoodcourtToCampus, {{"你推開側門，離開吵鬧的美食街，回到校園廣場。"}}, doorImg});
    items.push_back({AreaId::Foodcourt, {{720.f, 200.f}, {160.f, 140.f}}, sf::Color(140, 100, 80), "往咖啡廳", ItemEffectType::DoorFoodcourtToCafe, {{"你推開一扇窄窄的玻璃門，裡面是安靜的咖啡廳。"}}, houseBrown});
    items.push_back({AreaId::Foodcourt, {{200.f, 200.f}, {160.f, 140.f}}, sf::Color(180, 140, 80), "湯麵攤", ItemEffectType::Food, {{"湯麵散發正常的蒸氣味。\n（我：期末週能吃到一碗熱的，就已經是奢侈。）"}}, houseRed});
    items.push_back({AreaId::Foodcourt, {{450.f, 200.f}, {160.f, 140.f}}, sf::Color(160, 40, 40), "可疑攤位", ItemEffectType::Poison, {{}}, houseBlue});
    items.push_back({AreaId::Foodcourt, {{280.f, 400.f}, {240.f, 60.f}}, sf::Color(180, 170, 130), "美食街桌子", ItemEffectType::StudyGeneric, {{"桌面上有油漬和幾張發票。\n（我：在這裡讀書，會被別人的笑聲打斷。）"}}, tableWood});

    // ===== Cafe =====
    items.push_back({AreaId::Cafe, {{360.f, 520.f}, {80.f, 40.f}}, sf::Color(140, 100, 80), "回美食街", ItemEffectType::DoorCafeToFoodcourt, {{"你推開門，熱鬧的美食街聲音再次湧進來。"}}, doorImg});
    items.push_back({AreaId::Cafe, {{220.f, 280.f}, {120.f, 40.f}}, sf::Color(180, 160, 140), "窗邊小桌", ItemEffectType::StudyGeneric, {{"木桌上有微微的咖啡漬。\n（我：在這裡打開筆電，就算只是在裝忙，好像也比較像大人。）"}}, tableWhite});
    items.push_back({AreaId::Cafe, {{100.f, 180.f}, {200.f, 60.f}}, sf::Color(90, 70, 60), "吧台", ItemEffectType::None, {{"吧台上排著幾個寫了亂七八糟義式名字的咖啡壺。\n（我：看不懂，但聞起來很貴。）"}}, toasterOn});

    // ===== CS Basement =====
    items.push_back({AreaId::CSBasement, {{720.f, 260.f}, {40.f, 120.f}}, sf::Color(40, 40, 40), "B1 門", ItemEffectType::LabDoor, {{}}, ""});
    items.push_back({AreaId::CSBasement, {{100.f, 260.f}, {60.f, 120.f}}, sf::Color(60, 60, 80), "資料庫實驗室", ItemEffectType::None, {{"門縫底下滲出冷氣。\n（我：裡面的人可能在跟海量資料搏鬥。感覺也挺可怕。）"}}, doorImg});
    items.push_back({AreaId::CSBasement, {{220.f, 260.f}, {60.f, 120.f}}, sf::Color(60, 80, 80), "網安實驗室", ItemEffectType::None, {{"門把上有一道刮痕，像是有人暴力測試過。\n（我：也許有一天我會需要被他們保護…或者反過來。）"}}, doorImg});
    items.push_back({AreaId::CSBasement, {{340.f, 260.f}, {60.f, 120.f}}, sf::Color(80, 60, 80), "影像實驗室", ItemEffectType::None, {{"門上貼了幾張模糊的劇照。\n（我：畫面都有點失焦，跟我現在的專注力差不多。）"}}, doorImg});
    items.push_back({AreaId::CSBasement, {{380.f, 520.f}, {80.f, 40.f}}, sf::Color(60, 60, 80), "回南館", ItemEffectType::DoorBasementToCSSouth, {{"你沿著昏暗的樓梯走上去，回到南館走廊。"}}, stairsImg});
    
    // 特殊物件：沙發
    items.push_back({
            AreaId::CSBasement,
            {{350.f, 160.f}, {100.f, 60.f}},
            sf::Color(255, 105, 180),
            "突兀的沙發",
            ItemEffectType::Sofa,
            {{ "這張沙發看起來像是從某個溫馨家庭劇裡剪下貼上的，\n與充滿機櫃嗡鳴聲的地下室完全格格不入。" }},
            ""
    });

    // ===== CSNorth =====
    items.push_back({AreaId::CSNorth, {{520.f, 220.f}, {80.f, 60.f}}, sf::Color(100, 120, 160), "論文海報", ItemEffectType::None, {{"海報上全是密密麻麻的圖表和 p-value。\n（我：看不懂，但大概很重要。）"}}, posterYel});
    items.push_back({AreaId::CSNorth, {{200.f, 220.f}, {80.f, 60.f}}, sf::Color(120, 140, 170), "論文海報", ItemEffectType::None, {{"海報上佈滿了圖表和數學式。\n（我：標題聽起來很厲害，但我連摘要都看不完。)"}}, posterYel});
    items.push_back({AreaId::CSNorth, {{380.f, 520.f}, {80.f, 40.f}}, sf::Color(120, 160, 200), "回校園", ItemEffectType::DoorCSToCampus, {{"你走出系館，眼前又是熟悉的校園廣場。"}}, doorImg});
    items.push_back({AreaId::CSNorth, {{720.f, 260.f}, {40.f, 80.f}}, sf::Color(90, 90, 120), "往南館", ItemEffectType::DoorCSNorthToSouth, {{"你沿著走廊往更裡面走，來到南館。"}}, doorImg});

    // ===== CSSouth =====
    items.push_back({AreaId::CSSouth, {{520.f, 220.f}, {80.f, 60.f}}, sf::Color(110, 130, 150), "腦科實驗海報", ItemEffectType::Poster, {{}}, posterRed});
    items.push_back({AreaId::CSSouth, {{40.f, 260.f}, {40.f, 80.f}}, sf::Color(90, 90, 120), "回北館", ItemEffectType::DoorCSSouthToNorth, {{"你轉身回到比較亮的北館走廊。"}}, doorImg});
    items.push_back({AreaId::CSSouth, {{380.f, 520.f}, {80.f, 40.f}}, sf::Color(60, 60, 80), "往地下室", ItemEffectType::DoorCSSouthToBasement, {{"你搭上嘎吱作響的電梯，往 B1 緩緩下沉。"}}, stairsImg});

    // ===== Chaos Rift =====
    items.push_back({AreaId::ChaosRift, {{0.f, 150.f}, {800.f, 100.f}}, sf::Color(255, 0, 255, 0), "", ItemEffectType::None, {{"畫面像是被撕掉一角的講義。\n你看到自己坐在某個陌生實驗室裡，\n有人在你背後說話，但收音壞掉，只剩雜訊。\n（我：明明沒去過那裡，卻覺得那是很熟悉的一天。）"}}, ""});
    items.push_back({AreaId::ChaosRift, {{0.f, 400.f}, {800.f, 200.f}}, sf::Color(0, 255, 0, 0), "", ItemEffectType::None, {{"像是投影錯位的畫面閃過眼前。\n你站在校門口，外面一片白茫茫，\n只有螢幕角落寫著『FINAL WEEK SIM / build 2.29』。\n（我：…229？是哪一年的二月二十九？我有這一天的記憶嗎？）"}}, ""});
    items.push_back({AreaId::ChaosRift, {{360.f, 320.f}, {80.f, 80.f}}, sf::Color(50, 50, 50), "回到校園", ItemEffectType::DoorChaosToCampus, {{"你伸手摸向中央的黑色裂縫，視線一陣扭曲。"}}, ""});
}

// 初始化 NPC 與對話
void Game::setupNPCs() {
    npcs.clear();

    // ==========================================
    // 1. 壓力源 (Stressors)
    // ==========================================

    // ===== 室友 =====
    {
        NPC r("室友", AreaId::Dorm, {520.f, 360.f}, "roommate", -5, true);
        r.addNormalStage(
            "室友：你又看到半夜才睡喔？\n    我是覺得，沒讀完就去考，才比較刺激啦。\n（我：他成績也沒有很差，這種人到底是怎麼活著的。）"
        );

        // 觸發條件：Sanity <= 90
        r.addShadowStage(
            "室友：你最近是不是都躺在床上讀書？\n    小心喔，再這樣下去系統會以為你是『靜止物件』，\n    被當成垃圾回收掉也說不定。\n（我：他說得很像笑話，可是我背脊有點發涼。）",
            90, 45
        );
        // 觸發條件：Sanity <= 80
        r.addShadowStage(
            "室友：你泡澡不要一次洗太久啦，\n    期末週大家都在浴室裡崩潰，\n    你要是待三個時段，地板可能會只剩泡泡跟你。\n（我：他到底是看過什麼畫面才會講這種話。）",
            80, 30
        );
        r.addShadowStage(
            "室友：你最近睡覺都不翻身欸，像屍體一樣，超安靜，讚喔。\n（我：因為系統為了節省運算資源，在我睡眠模式時關閉了物理碰撞判定。\n     你當然覺得安靜。）",
            100, 60
        );
        npcs.push_back(r);
    }

    // ===== 瘋狂學長 =====
    {
        NPC m("學長", AreaId::CSSouth, {400.f, 330.f}, "senpai", -5, true);
        m.addNormalStage(
            "學長：這個程式我已經改到不知道第幾版了，\n    以為終於穩定了，結果又蹦出新的 bug。\n    昨天半夜還在地下室重編，連警衛都記得我名字了==。\n（我：大概是他的研究吧，他看起來有點神經質，\n     但我卻有種──要是我也有這種可以拼命的東西就好了的感覺。）"
        );
        // 核心劇情對話
        m.addShadowStage(
            "學長：期中考那次，還在介意嗎？\n    期末翻盤就好啦。真正糟糕的是你不敢再試一次。\n（我：他講得很輕鬆，好像從來沒被考卷刺傷過。）",
            100, 10
        );
        m.addShadowStage(
            "學長：你那次期中考炸掉，其實不是世界末日。\n    至少在『這一輪』不是。上一輪你炸得更徹底。\n（我：等等，什麼叫上一輪？他是說上學期吧…？）",
            100, 30
        );
        m.addShadowStage(
            "學長：你有沒有覺得，有些失敗的方式你已經看過不只一次？\n    像在試驗不同的路線，紀錄哪一種崩潰比較精彩。\n（我：聽起來有點可怕，但好像也不是完全說錯。）",
            100, 40
        );
        m.addShadowStage(
            "學長：你的『期中考炸掉』事件已經重複發生好幾次了。\n"
            "    但不管失敗幾次，你總是會像設定好的一樣，走回這裡。\n"
            "    這種『歸巢本能』到底是 bug 還是 feature？\n  看在你這麼努力的份上……這個給你。\n    【你拿到了學長的學生證。】\n（我：…？）",
            100, 60
        );
        m.addShadowStage(
            "學長：為什麼要急著醒來？\n"
            "    外面的世界充滿了不可控的變數，你要面對隨機的失敗。\n"
            "    但在這裡，只要你乖乖當變數，我就能保證你的存在有意義。\n"
            "    當個快樂的數據，不好嗎？\n"
            "（我：恐懼感爬上背脊。）",
            100, 105
        );
        npcs.push_back(m);
    }

    // ===== 助教 =====
        {
            // 建構子: (名字, 區域, 座標, Key, Sanity變化, 消耗時間)
            NPC t("助教", AreaId::Library, {640.f, 430.f}, "ta", -3, true);
            
            t.addNormalStage("助教：期末加油喔。\n    這次當掉的比例抓在四成左右，還不算狠啦。\n（我：四成叫不狠？那我到底掉在那六成還是四成之間。）");
            t.addNormalStage("助教：安靜一點，資料正在備份。\n    嗯？我是說成績資料啦，你在緊張什麼？\n（我：被他這樣一提醒，反而更緊張了。）");
            
            // 注意：原本助教沒有 Shadow Stage，只有 resize，現在改用 class 就不需要 resize 了
            npcs.push_back(t);
        }

        // ===== 校園：認識的同學 =====
        {
            NPC c("認識的同學", AreaId::Campus, {260.f, 420.f}, "friend_known", -2, true);
            
            c.addNormalStage("同學：欸，好久不見，你最近看起來…有點累欸。\n    我以前都覺得你是那種考試永遠不會失常的人。\n（我：如果他有看到我的期中考卷，大概就不會這樣說了。）");
            c.addNormalStage("同學：你還在衝那個腦與計算實驗室喔？\n    你這種卷王進去應該很合理啦。\n（我：我突然很想問，他知道『合理』是誰定義的嗎。）");
            
            // 觸發條件：Sanity <= 90
            c.addShadowStage(
                "同學：以前看你就像學校的 benchmark，\n    現在想想，會不會我們只是同一個實驗裡不同版本的樣本？\n（我：他明明只是普通同學，為什麼開始講這種話。）",
                90, 45
            );
            npcs.push_back(c);
        }

        // ===== 熱舞社同學 =====
        {
            NPC d("熱舞社同學", AreaId::Campus, {520.f, 420.f}, "dance", -4, true);
            
            d.addNormalStage("熱舞社同學：我每天排舞排到半夜，\n    考試就當成上台表演，多失誤幾拍也沒差啦。\n（我：我永遠學不會這種正向思考）");
            d.addNormalStage("熱舞社同學：你也是一直待在圖書館吧？\n    有時候動一動身體，比多看一頁筆記還有用喔。\n（我：我不需要那種廉價的快樂。）");

            // 觸發條件：Sanity <= 100
            d.addShadowStage(
                "熱舞社同學：欸，你記不記得高三那時候？我們說好要一起考這間，\n    結果現在好像只有我在玩社團哈哈。\n（我：記得。那時候他的模擬考分數比我低 20 分。\n     現在他的快樂指數比我高 200%。這世界的回饋機制明顯有 Bug。）",
                100, 45
            );
            d.addShadowStage(
                "熱舞社同學：好久不見！你看起來...很累欸。\n我們舞展你有空來嗎？我幫你留公關票！\n（我：看著他在舞台上發光，會讓我確認自己正爛在泥沼裡。拒絕請求。）",
                100, 60
            );
            npcs.push_back(d);
        }

        // ===== 系上同學 =====
        {
            NPC c1("系上同學", AreaId::CSNorth, {460.f, 360.f}, "mis_a", -5, true);
            
            c1.addNormalStage("系上同學：我覺得與其讀到爆炸，不如把報告寫漂亮一點，\n    至少老師看起來比較開心。\n（我：可是成績單上只會印數字，沒人看你字有多漂亮。）");
            c1.addNormalStage("系上同學：你有發現嗎？\n    到了期末週，大家的夢想都退化成『不要被當』。\n（我：是啊，志向暫時被 GPA 壓在地上打。）");

            c1.addShadowStage(
                "系上同學：欸～姐妹～這禮拜的題目我真的看不懂啦QQ\n    借我參考一下架構就好，拜託～\n（我：她稱呼我為「姐妹」，但語法分析顯示這只是「請求工具支援」的包裝函式。\n     ...但我還是把檔案傳出去了。）",
                100, 45
            );
            npcs.push_back(c1);
        }

        // ===== 不熟的同學 =====
        {
            NPC c2("不熟的同學", AreaId::CSNorth, {360.f, 320.f}, "mis_b", -1, true);
            
            c2.addNormalStage("不熟的同學：我把 GPA 當成遊戲分數在刷，\n    但刷到後來會開始懷疑自己是不是人。\n（我：我大概已經懷疑很久了。）");
            c2.addNormalStage("不熟的同學：有時候覺得，我念書不是為了學東西，\n    只是為了不要被這個系淘汰。\n（我：被留下來，卻不知道自己想成為什麼樣的人。）");
            
            npcs.push_back(c2);
        }

        // ===== 教授 =====
        {
            NPC p("教授", AreaId::CSSouth, {260.f, 320.f}, "prof", -4, true);
            
            p.addNormalStage("TA：你期末專案做到哪了？\n記得要用我們給的那些 class 喔，不然我會很難改。\n（我：……其實我也很難寫。）");
            p.addNormalStage("資管教授：如果只看成績，很容易忘記你自己是誰。\n    但我也知道，要你們在這個系裡不看成績，很困難。\n（我：被理解的那一瞬間，反而更想哭。）");
            
            npcs.push_back(p);
        }

        // ===== Foodcourt 可疑同學 =====
        {
            NPC g("同學", AreaId::Foodcourt, {300.f, 350.f}, "student1", +2, true);
            
            g.addNormalStage("同學：那家紅色的店…我朋友說吃完直接在廁所待一晚。\n    你要是期末週還想進考場，就別亂挑戰啦。\n（我：聽起來就很像會變成考卷題目的都市傳說。）");
            g.addNormalStage("同學：你也是來趕報告喔？\n    我現在看到插座比看到同學還開心。\n（我：同感，充電器是期末週唯一的信仰。）");

            // 觸發條件：Sanity <= 85
            g.addShadowStage(
                "同學：其實那家紅色攤位賣的不是食物啦，\n    是一段寫得很爛的病毒程式碼，\n    你一吃下去，體內執行緒就被 while(true) 卡死。\n（我：……我突然覺得肚子有點痛，不知道是不是被暗示的。）",
                85, 45
            );
            g.addShadowStage(
                "同學：我上次不小心吃了，\n    覺得自己被強制觸發 BAD END：Food Poison Exception。\n（我：這種錯誤訊息在成績單上大概只會寫成『缺考』。）",
                85, 60
            );
            npcs.push_back(g);
        }

        // ==========================================
        // 2. 非壓力源 (Non-Stressors)
        // ==========================================

        // ===== 年長老教授 =====
        {
            // 注意：這裡是不消耗時間的 NPC (spendsSlot = false)
            NPC op("年長教授", AreaId::CSBasement, {520.f, 360.f}, "oldprof", -3, false);
            
            op.addNormalStage("年長教授：你迷路到這裡來了？\n    地下室在期末週特別安靜，只有伺服器的聲音不會睡。\n（我：他的語氣像是在說床邊故事，但內容一點也不溫柔。）");
            op.addNormalStage("年長教授：你們這一代，把自己當成學術機器在調校，\n    卻很少問自己：如果機器壞掉了，要怎麼善後。\n（我：我突然想到很多壞掉的地方。）");

            // 觸發條件：Sanity <= 90
            op.addShadowStage(
                "年長教授：CS 系館的走廊能被拉得很長，\n    期末週時就是最好的壓力測試場景。\n    如果你覺得怎麼走都走不到考場，\n    那只是演算法在檢查你能忍受多少重複。\n（我：原來不是我搞錯路，而是路本來就被寫壞了。）",
                90, 75
            );
            op.addShadowStage(
                "年長教授：B1 的那扇門，在表層故事裡是鎖死的，\n    但在真結局路線中，你會發現門其實沒有實體碰撞體。\n    你可以直接穿過去，去看看躺在病床上的自己。\n（我：……他剛剛好像劇透了什麼很重要的東西。）",
                100, 90
            );
            npcs.push_back(op);
        }

        // ===== 體育股長 =====
        {
            NPC s("體育股長", AreaId::CSNorth, {200.f, 400.f}, "sportrep", -5, false);
            
            s.addNormalStage("體育股長：週四要考體育，記得多動動手指！\n    不然小遊戲會輸到懷疑人生。\n（我：期末週還要顧心肺功能，真的是身心靈三重考驗。）");
            s.addNormalStage("體育股長：你最近有曬到太陽嗎？\n    期末週待在室內太久，會忘記自己是有身體的。\n（我：好像真的快忘了腿長什麼樣子。）");

            s.addShadowStage(
                "體育股長：欸...那個，同學，老師問說你這學期有來過嗎？\n（我：他是負責管理我肉體出勤率的 NPC。\n     只要我低頭不回應，這個對話視窗就會自動關閉。）",
                100, 60
            );
            npcs.push_back(s);
        }

        // ===== 乞丐 =====
        {
            NPC b("乞丐", AreaId::Foodcourt, {120.f, 380.f}, "beggar", 0, false);
            
            b.addNormalStage("乞丐：你有沒有注意到，這裡沒有真正的晚上。\n    燈光只會變暗一點點，好像有人懶得寫完整的日夜循環。\n（我：……他是在抱怨學校，還是在抱怨什麼程式？）");
            b.addNormalStage("乞丐：有些人以為自己在念大學，\n    其實只是被丟進一個很逼真的實驗場景。\n（我：…我假裝沒聽懂，心裡卻有點發冷。）");

            b.addShadowStage(
                "乞丐：給點錢吧...或者給點多餘的記憶體空間...\n    這裡的貼圖品質越來越低了。\n（我：他不是乞丐。他是上一輪被刪除失敗的殘留檔案。）",
                100, 70
            );
            npcs.push_back(b);
        }

        // ===== 店員 =====
        {
            NPC clerk("便利店店員", AreaId::LibraryShop, {420.f, 300.f}, "clerk", +2, false);
            
            clerk.addNormalStage("店員：需要發票嗎？說不定你之後會中個小獎，彌補一下期末的心理創傷。\n（我：如果只用幾百塊就能買到心理補償，感覺還蠻划算的。）");
            clerk.addNormalStage("店員：有些人每天晚上都來，\n    看起來比我這個打工的還常值班。\n（我：突然有點不確定，自己是在讀書還是在上夜班。）");

            clerk.addShadowStage(
                "店員：你知道嗎？從監視器看，你們走進來的樣子都差不多，\n    像是同一份樣本被重播很多次。\n（我：我不想知道自己是不是其中一個『樣本』。）",
                95, 50
            );
            clerk.addShadowStage(
                "店員：微波需要稍等喔。\n（我：他是這個世界裡少數按照既定邏輯運作的物件，\n     這讓我感到安心。）",
                100, 70
            );
            npcs.push_back(clerk);
        }

        // ===== 女僕 =====
        {
            NPC maid("打工女僕", AreaId::Cafe, {460.f, 360.f}, "maid", +5, false);
            
            maid.addNormalStage("女僕：歡迎光臨～今天也是努力活過期末的一天呢♪\n    這邊的插座可以用，不過請記得偶爾眨眨眼喔。\n（我：被女僕提醒眨眼，感覺有點好笑又有點被安慰。）");
            maid.addNormalStage("女僕：客人看起來壓力很大呢，要不要來一杯期末限定的\n    『學期結束就忘光光特調』？只是普通拿鐵啦。\n（我：如果真有那種飲料，我可能會先買一箱。）");

            maid.addShadowStage(
                "女僕：有時候我會想，\n    如果這裡只是背景資產，你們只是路過的 NPC，\n    那我是不是只是你壓力過大的時候刷新的安慰立繪而已？\n（我：她笑得很可愛，但眼神有點像在問我答案。）",
                90, 55
            );
            maid.addShadowStage(
                "女僕：主人唸書辛苦了～\n（我：這是商業話術。但我付了 150 元，\n     我有權利享受這 3 秒的幻覺。）",
                100, 75
            );
            npcs.push_back(maid);
        }

        // ===== 貓咪 =====
        {
            //  +15 Sanity, 不消耗時間
            NPC cat("貓咪", AreaId::LibraryStudy, {0.f, 0.f}, "cat", +15, false);
            
            cat.addNormalStage("貓咪：喵～（蹭蹭）\n（你摸了摸貓咪，感覺掌心的溫度是這個世界唯一的真實。）");
            cat.addNormalStage("貓咪：呼嚕...呼嚕...\n（聽著貓咪的呼嚕聲，你的心跳慢慢平靜下來。）");

            cat.addShadowStage(
                "貓咪：喵～\n（我：只有這個生物不會問我 GPA 多少。）",
                100, 30
            );
            cat.addShadowStage(
                "貓咪：喵。（牠蹭了蹭你的腳，項圈上的紅燈突然閃爍了一下。）\n（你驚訝地翻開牠的項圈，背面刻著：『B1-07 實驗室資產 』。）",
                100, 40
            );
            npcs.push_back(cat);
        }
    }

// 重新隨機放置貓咪位置
void Game::relocateCat() {
    static const std::vector<AreaId> catAreas = {
        AreaId::Dorm, AreaId::Campus, AreaId::LibraryStudy,
        AreaId::Cafe, AreaId::Foodcourt, AreaId::CSSouth
    };

    int idx = std::rand() % catAreas.size();
    AreaId targetArea = catAreas[idx];

    for (auto& n : npcs) {
        if (n.name == "貓咪") {
            n.area = targetArea;
            float rx = 200.f + static_cast<float>(std::rand() % 400);
            float ry = 250.f + static_cast<float>(std::rand() % 200);
            n.pos = {rx, ry};
            break;
        }
    }
}

// 傳送玩家至指定區域
void Game::warpTo(AreaId target, const sf::Vector2f& pos, const std::string& msg) {
    area = target;
    player.setPosition(pos);
    if (!msg.empty()) {
        showPortrait = false;
        startDialogue(msg);
    }

    updateHUD();
}

// 繪製互動標籤與浮動文字
void Game::drawFloatingLabels() {
    sf::FloatRect pRect = player.getBounds();
    pRect.position.x -= 30.f; pRect.position.y -= 30.f;
    pRect.size.x += 60.f; pRect.size.y += 60.f;

    for (const auto& it : items) {
        if (it.area != area) continue;
        if (it.rect.findIntersection(pRect).has_value()) {
            if (it.label.empty()) continue;
            floatingText.setString(u8(it.label));
            floatingText.setFillColor(sf::Color::Yellow);
            floatingText.setPosition({it.rect.position.x, it.rect.position.y - 20.f});
            window.draw(floatingText);
        }
    }

    sf::Vector2f p = player.getPosition();
    for (const auto& n : npcs) {
        if (n.area != area) continue;
        float dx = n.pos.x - p.x;
        float dy = n.pos.y - p.y;
        float d2 = dx*dx + dy*dy;
        
        if (d2 < 3000.f) {
            bool hasUnreadSecret = false;
            for (size_t i = 0; i < n.shadowStages.size(); ++i) {
                const auto& stage = n.shadowStages[i];
                bool conditionMet = (player.sanity <= stage.requiredSanityMax && awareness >= stage.requiredAwarenessMin);
                bool seen = (i < n.shadowStagesSeen.size()) ? n.shadowStagesSeen[i] : false;
                if (conditionMet && !seen) { hasUnreadSecret = true; break; }
            }

            sf::Color textColor = sf::Color::Yellow;
            float jitterX = 0.f, jitterY = 0.f;
            if (hasUnreadSecret) {
                textColor = sf::Color::Red;
                jitterX = (std::rand() % 3) - 1.5f;
                jitterY = (std::rand() % 3) - 1.5f;
            }

            floatingText.setString(u8(n.name));
            floatingText.setFillColor(textColor);
            floatingText.setPosition({
                n.pos.x - 20.f + jitterX,
                n.pos.y - n.radius - 25.f + jitterY
            });
            window.draw(floatingText);
        }
    }
}

// 遊戲渲染主循環
void Game::render() {
    window.clear(sf::Color(20, 20, 20));

    // === 結局或過場模式 ===
    if (endingMode != EndingMode::None) {
        window.setView(window.getDefaultView());

        // 1. 繪製背景圖 (CG)
        if (!endingScenes.empty() && endingSceneIndex < endingScenes.size()) {
            const auto& page = endingScenes[endingSceneIndex];
            static std::string lastLoadedKey = "";
            
            if (!page.imageKey.empty()) {
                if (page.imageKey != lastLoadedKey) {
                    auto path = resourceDir / "assets" / "cg" / (page.imageKey + ".png");
                    if (endingBgTexture.loadFromFile(path)) {
                        endingBgSprite.setTexture(endingBgTexture, true);
                        auto sz = endingBgTexture.getSize();
                        float scaleX = (float)WINDOW_WIDTH / sz.x;
                        float scaleY = (float)WINDOW_HEIGHT / sz.y;
                        float scale = std::max(scaleX, scaleY);
                        endingBgSprite.setScale({scale, scale});
                        endingBgSprite.setPosition({0.f, 0.f});
                        
                        lastLoadedKey = page.imageKey;
                    }
                }
                window.draw(endingBgSprite);
            } else {
                window.clear(sf::Color::Black);
                lastLoadedKey = "";
            }
        } else {
            window.clear(sf::Color::Black);
        }

        // 2. 繪製對話框
        if (!endingScenes.empty()) {
            sf::RectangleShape box;
            box.setSize({(float)WINDOW_WIDTH - 40.f, 200.f});
            box.setPosition({20.f, (float)WINDOW_HEIGHT - 220.f});
            box.setFillColor(sf::Color(0, 0, 0, 200));
            box.setOutlineColor(sf::Color::White);
            box.setOutlineThickness(2.f);
            window.draw(box);

            sf::Text t(font);
            t.setString(u8(endingScenes[endingSceneIndex].text));
            t.setCharacterSize(20);
            t.setFillColor(sf::Color::White);
            t.setPosition({box.getPosition().x + 20.f, box.getPosition().y + 20.f});
            window.draw(t);
            
            sf::Text tip(font);
            tip.setString(u8("▼"));
            tip.setCharacterSize(16);
            tip.setPosition({box.getPosition().x + box.getSize().x - 30.f,
                             box.getPosition().y + box.getSize().y - 30.f});
            
            if ((int)(loopCount * 0.1f) % 2 == 0) window.draw(tip);
        }
        window.display();
        return;
    }

    // === 一般遊戲畫面 ===
    window.setView(worldView);

    // 1. 繪製障礙物 (牆壁、地板)
    for (const auto& o : obstacles) {
        if (o.area != area) continue;

        bool drawn = false;
        if (!o.textureKey.empty()) {
            auto it = objectTextures.find(o.textureKey);
            if (it == objectTextures.end()) {
                sf::Texture tex;
                if (tex.loadFromFile((resourceDir / "assets" / o.textureKey).string())) {
                    // 設定紋理重複模式 (Repeated)
                    tex.setRepeated(true);
                    it = objectTextures.emplace(o.textureKey, std::move(tex)).first;
                }
            }

            if (it != objectTextures.end()) {
                it->second.setRepeated(true);

                sf::Sprite spr(it->second);
                spr.setPosition(o.rect.position);

                // 調整磁磚縮放比例
                float tileZoom = 0.1f;

                // 設定 Sprite 縮放比例與紋理區域
                spr.setScale({tileZoom, tileZoom});
                spr.setTextureRect(sf::IntRect(
                    {0, 0},
                    {(int)(o.rect.size.x / tileZoom), (int)(o.rect.size.y / tileZoom)}
                ));
                spr.setColor(sf::Color(180, 180, 180));
                
                window.draw(spr);
                drawn = true;
            }
        }

        if (!drawn) {
            sf::RectangleShape r(o.rect.size);
            r.setPosition(o.rect.position);
            r.setFillColor(o.color);
            window.draw(r);
        }
    }

    // 2. 繪製物件
    for (const auto& itItem : items) {
        if (itItem.area != area) continue;

        bool drawn = false;
        if (!itItem.textureKey.empty()) {
            auto it = objectTextures.find(itItem.textureKey);
            if (it == objectTextures.end()) {
                sf::Texture tex;
                if (tex.loadFromFile((resourceDir / "assets" / itItem.textureKey).string())) {
                     it = objectTextures.emplace(itItem.textureKey, std::move(tex)).first;
                }
            }

            if (it != objectTextures.end()) {
                sf::Sprite spr(it->second);
                auto sz = it->second.getSize();
                spr.setPosition(itItem.rect.position);
                spr.setScale({
                    itItem.rect.size.x / (float)sz.x,
                    itItem.rect.size.y / (float)sz.y
                });
                window.draw(spr);
                drawn = true;
            }
        }

        if (!drawn) {
            sf::RectangleShape r(itItem.rect.size);
            r.setPosition(itItem.rect.position);
            r.setFillColor(itItem.color);
            window.draw(r);
        }
    }
    
    // 3. 繪製 NPC
    for (const auto& n : npcs) {
        if (n.area != area) continue;
        bool drewSprite = false;
        
        if (!n.portraitKey.empty()) {
            auto it = portraitTextures.find(n.portraitKey);
            if (it == portraitTextures.end()) {
                sf::Texture tex;
                auto path = resourceDir / "assets" / "portraits" / (n.portraitKey + ".png");
                if (tex.loadFromFile(path)) {
                    tex.setSmooth(true);
                    it = portraitTextures.emplace(n.portraitKey, std::move(tex)).first;
                }
            }
            if (it != portraitTextures.end()) {
                sf::Sprite npcSprite(it->second);
                auto sz = it->second.getSize();
                float targetSize = PLAYER_RADIUS * 2.f;
                if (n.name == "貓咪") targetSize = 20.f;
                float scale = targetSize / static_cast<float>(std::max(sz.x, sz.y));
                
                npcSprite.setScale({scale, scale});
                npcSprite.setOrigin({static_cast<float>(sz.x)/2.f, static_cast<float>(sz.y)/2.f});
                npcSprite.setPosition(n.pos);
                window.draw(npcSprite);
                drewSprite = true;
            }
        }
        if (!drewSprite) {
            float r = (n.name == "貓咪") ? 10.f : PLAYER_RADIUS;
            sf::CircleShape c(r);
            c.setOrigin({r, r});
            c.setPosition(n.pos);
            c.setFillColor(n.color);
            window.draw(c);
        }
    }

    player.draw(window);
    
    drawFloatingLabels();
    
    window.setView(window.getDefaultView());
    window.draw(hudText);
    window.draw(infoText);
    
    if (inOpeningIntro) {
        sf::RectangleShape box;
        box.setSize({720.f, 260.f});
        box.setPosition({40.f, (WINDOW_HEIGHT - 260.f) / 2.f});
        box.setFillColor(sf::Color(0, 0, 0, 220));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        window.draw(box);
        
        sf::Text t(font);
        t.setCharacterSize(20);
        t.setFillColor(sf::Color::White);
        std::string pageStr;
        if (!openingPages.empty() && openingPageIndex < openingPages.size()) pageStr = openingPages[openingPageIndex];
        t.setString(u8(pageStr));
        t.setPosition({box.getPosition().x + 24.f, box.getPosition().y + 24.f});
        window.draw(t);
        window.display();
        return;
    }
    
    if (inDialogue) {
        sf::RectangleShape box;
        box.setSize({720.f, 200.f});
        box.setPosition({40.f, WINDOW_HEIGHT - 220.f});
        box.setFillColor(sf::Color(0, 0, 0, 220));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        window.draw(box);
        
        dialogueText.setCharacterSize(18);
        dialogueText.setFillColor(sf::Color::White);
        dialogueText.setPosition({box.getPosition().x + 20.f, box.getPosition().y + 24.f});
        window.draw(dialogueText);
    }
    window.display();
}
