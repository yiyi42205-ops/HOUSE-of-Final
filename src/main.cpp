#include "Game.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        // 取得執行檔路徑並初始化遊戲
        std::filesystem::path exeDir = std::filesystem::current_path();
        Game game(exeDir);
        game.runGameLoop();
    }
    catch (const std::exception& e) {
        // 捕捉標準例外
        std::cerr << "遊戲發生嚴重錯誤 (Exception): " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
    catch (...) {
        // 捕捉未知例外
        std::cerr << "發生未知錯誤！" << std::endl;
        return -1;
    }
    return 0;
}
