#include "headers/GameManager.h"

std::vector<std::string> GameManager::folderList = { "ball_game", "donna_game" };

void GameManager::AddGame(const std::string& game) {
    folderList.push_back(game);
}

void GameManager::DeleteGame(const std::string& game) {
    auto it = std::find(folderList.begin(), folderList.end(), game);
    if (it != folderList.end()) {
        folderList.erase(it);
    }
}

void GameManager::ListGames() {
    for (const auto& game : folderList) {
        std::cout << game << std::endl;
    }
}

bool GameManager::FindGame(const std::string& game_name) {
    return std::find(folderList.begin(), folderList.end(), game_name) != folderList.end();
}

void GameManager::SortGames() {
    std::sort(folderList.begin(), folderList.end());
}

void GameManager::RandomizeGames() {
    std::srand(unsigned(std::time(0)));
  //  std::random_shuffle(folderList.begin(), folderList.end());
}

void GameManager::LoadGamesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string game;
    while (getline(file, game)) {
        folderList.push_back(game);
    }
}

void GameManager::SaveGamesToFile(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& game : folderList) {
        file << game << std::endl;
    }
}

size_t GameManager::CountGames() {
    return folderList.size();
}

void GameManager::RemoveAllGames() {
    folderList.clear();
}

void GameManager::CompareAndMergeLists(const std::vector<std::string>& otherList) {
    for (const auto& game : otherList) {
        if (std::find(folderList.begin(), folderList.end(), game) == folderList.end()) {
            folderList.push_back(game);
        }
    }
}
