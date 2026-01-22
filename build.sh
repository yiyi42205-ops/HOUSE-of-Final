#!/bin/zsh
set -e

# 確保在專案資料夾裡
cd "$(dirname "$0")"

# 編譯：把 src 底下的所有 cpp 都丟進去
clang++ -std=c++20 -Wall -Wextra -O2 \
  src/main.cpp \
  src/GameCore.cpp \
  src/GameWorld.cpp \
  src/GameActions.cpp \
  src/GameUI.cpp \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio \
  -o game

echo "✅ Build finished."
