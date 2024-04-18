#include "headers/EngineUtils.h"


void EngineUtils::ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
{
    FILE* file_pointer = nullptr;
#ifdef _WIN32
    fopen_s(&file_pointer, path.c_str(), "rb");
#else
    file_pointer = fopen(path.c_str(), "rb");
#endif
    char buffer[65536];
    rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
    out_document.ParseStream(stream);
    std::fclose(file_pointer);

    if (out_document.HasParseError()) {
        rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
        std::cerr << "JSON parse error code: " << errorCode << std::endl;

        std::cout << "error parsing json at [" << path << "]" << std::endl;
        exit(0);
    }
}


void EngineUtils::checkResourcesDirectory() {
    const std::filesystem::path resourcesPath{"resources/"};

    if (!std::filesystem::exists(resourcesPath)) {
         std::cout << "error: resources/ missing";
         std::exit(0);
    }
}

 void EngineUtils::checkGameConfigFile() {

     // WINDOWS
    const std::filesystem::path configFile{"resources/game.config"};

     if (!std::filesystem::exists(configFile)) {
         std::cout << "error: resources/game.config missing";
         std::exit(0);
     }

}

void EngineUtils::checkComponentDirectory() {
    
    // WINDOWS
    const std::filesystem::path configFile{"resources/component_types"};

    if (!std::filesystem::exists(configFile)) {
        std::cout << "error: resources/component_types missing";
        std::exit(0);
    }
}


void EngineUtils::loadGameMessages(std::string &gameStartMessage, std::string &gameOverBadMessage, std::string &gameOverGoodMessage) {
    // Path to the game configuration file
    const std::string configFilePath = "resources/game.config";

    // Document object to store the parsed JSON
    rapidjson::Document document;

    // Read and parse the JSON file
    ReadJsonFile(configFilePath, document);

    // Extracting game_start_message if it exists
    if (document.HasMember("game_start_message") && document["game_start_message"].IsString()) {
        gameStartMessage = document["game_start_message"].GetString();
    }

    // Extracting game_over_bad_message if it exists
    if (document.HasMember("game_over_bad_message") && document["game_over_bad_message"].IsString()) {
        gameOverBadMessage = document["game_over_bad_message"].GetString();
    }

    // Extracting game_over_good_message if it exists
    if (document.HasMember("game_over_good_message") && document["game_over_good_message"].IsString()) {
        gameOverGoodMessage = document["game_over_good_message"].GetString();
    }
}

void EngineUtils::loadCameraConfig(int &CAMERA_WIDTH, int &CAMERA_HEIGHT) {
    const std::filesystem::path configFile{"resources/rendering.config"};

    if (std::filesystem::exists(configFile)) {
        // Document object to store the parsed JSON
        rapidjson::Document document;

        // Read and parse the JSON file
        EngineUtils::ReadJsonFile(configFile.string(), document);

        // Extract x_resolution and y_resolution if they exist
        if (document.HasMember("x_resolution") && document["x_resolution"].IsInt()) {
            CAMERA_WIDTH = document["x_resolution"].GetInt();
        }

        if (document.HasMember("y_resolution") && document["y_resolution"].IsInt()) {
            CAMERA_HEIGHT = document["y_resolution"].GetInt();
        }
    }
}

