#include <iostream>
#include <set>
#include <cstdlib>
#include <memory>
#include <vector>
#include "libs/glm/glm/glm.hpp"
#include "unordered_map"
#include <algorithm>
#include <queue>
#include <cmath>
#include <filesystem>
#include <thread>
#include <cstdint>
#include "headers/Helper.h"
#include "headers/AudioHelper.h"
#include "headers/Scene.h"
#include "headers/EngineUtils.h"
#include "headers/Actor.h"
#include "headers/Renderer.h"
#include "headers/GameState.h"
#include "headers/RigidBody.h"
#include "headers/Camera.h"
#include "Lua/lua.hpp"  
#include "LuaBridge/LuaBridge.h"
#include "headers/MainHelper.h"
#include "headers/Eventbus.h"
#include "headers/Game.h"
#include "box2d-2.4.1/box2d-2.4.1/include/box2d/box2d.h"

#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

std::set<int> scoredActors;
std::set<int> playedDialogueSFX;
bool gameWon = false;
bool gameOver = false;
bool load_new_scene = false;    

std::string gamePlaying = "ball_game";

std::vector<std::shared_ptr<Actor>> hardcoded_actors;

// Check dialogue every frame
std::vector<std::pair<std::shared_ptr<Actor>, std::string>> contactdialogueEntries; // Pair actor ID with dialogue
std::vector<std::pair<int, std::string>> dialogueEntries;

// Initialize region sizes with zero to begin with.
glm::vec2 REGION_SIZE_COLLISION(0, 0);
glm::vec2 REGION_SIZE_TRIGGER(0, 0);

unsigned long lastDamageFrame = 0;
unsigned long damageCooldownFrames = 0;

extern enum class VerticalDirection;
extern enum class HorizontalDirection;
extern enum class ActorMode;

// Camera Consts
const int DEFAULT_CAMERA_WIDTH = 13;
const int DEFAULT_CAMERA_HEIGHT = 9;
int CAMERA_WIDTH = DEFAULT_CAMERA_WIDTH;
int CAMERA_HEIGHT = DEFAULT_CAMERA_HEIGHT;

void addActorsToGameWorld();


// Function to create a composite key from two integers
static uint64_t create_composite_key(const int x, const int y) {
    auto x_ = static_cast<uint32_t>(x);
    auto y_ = static_cast<uint32_t>(y);
    return (static_cast<uint64_t>(x_) << 32) | static_cast<uint64_t>(y_);
}

static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase) {
    // Find the position of the phrase in the string
    size_t pos = input.find(phrase);

    // If phrase is not found, return an empty string
    if (pos == std::string::npos) return "";

    // Find the starting position of the next word (skip spaces after the phrase)
    pos += phrase.length();
    while ((pos < input.size()) && std::isspace(input[pos])) {
        ++pos;
    }

    // If we're at the end of the string, return an empty string
    if (pos == input.size()) return "";

    // Find the end position of the word (until a space or the end of the string)
    size_t endPos = pos;
    while ((endPos < input.size()) && !std::isspace(input[endPos])) {
        ++endPos;
    }

    // Extract and return the word
    return input.substr(pos, endPos - pos);
}


void addActorsToGameWorld() {
   
}




bool process_dialogue(const std::shared_ptr<Actor>& actor, const std::string& dialogue, 
                Renderer& Renderer, std::string& newSceneName, bool& newSceneExists, AudioDB& audioDB) {
        
    unsigned long currentFrame = Helper::GetFrameNumber();

    if (dialogue.find("health down") != std::string::npos && currentFrame >= lastDamageFrame + damageCooldownFrames) {
       
    }
    else if (dialogue.find("score up") != std::string::npos && scoredActors.insert(actor->id).second){
   
    }
    else if (dialogue.find("you win") != std::string::npos) {
        gameWon = true;
        Renderer.currentState = GameState::Ending;
        return true; // Indicate that a significant game state change occurred
    }
    else if (dialogue.find("game over") != std::string::npos) {
        gameOver = true;
        Renderer.currentState = GameState::Ending;
        return true; // Indicate that a significant game state change occurred
    }
    else if (dialogue.find("proceed to ") != std::string::npos) {
        newSceneName = obtain_word_after_phrase(dialogue, "proceed to ");
        load_new_scene = true;
        newSceneExists = Scene::isLoadScene(newSceneName);
        return true; // Indicate that a significant game state change occurred
    }

    return false; // No significant game state change occurred
}


void checkAdjacentAndContactActors(CameraBounds& Camera, Renderer &Renderer, TextDB &textDB, AudioDB &audioDB) {

    bool newSceneExists = true;
    std::string newSceneName;

    for (const auto& entry : contactdialogueEntries) {
        if (process_dialogue(entry.first, entry.second, Renderer, newSceneName, newSceneExists, audioDB)) {
            if (gameOver || gameWon) {
                return;
            }
            assert(load_new_scene == true);
            break;
        }
    }
    contactdialogueEntries.clear();

    if (load_new_scene) {

        if (!newSceneExists) {
            std::cout << "error: scene " << newSceneName << " is missing";
            std::exit(0);
        }
        else if (newSceneExists && !gameOver) {
            //grid.clear();
            hardcoded_actors.clear();
            contactdialogueEntries.clear();
            dialogueEntries.clear();
            load_new_scene = false;
            gameOver = false;
            gameWon = false;
            Scene::sparseSceneFile("resources/scenes/" + newSceneName + ".scene", hardcoded_actors);
            Camera.calculateCameraPositions(Renderer.camera_offset_x, Renderer.camera_offset_y);
            addActorsToGameWorld();
            Renderer.RenderActors(CameraBounds::cam_x_pos, CameraBounds::cam_y_pos, CameraBounds::zoom_factor);
            return;
        }
    }

    // Sort dialogues by actor ID
    std::sort(dialogueEntries.begin(), dialogueEntries.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
        });
}

void updateActorPositions(glm::vec2& movement, AudioDB& audioDB) {

}





int main(int argc, char* argv[])
{
    // Load Lua
    LuaHelper lua_obj;

    /*EngineUtils.checkResourcesDirectory();
    EngineUtils.checkGameConfigFile();*/
   // EngineUtils.checkComponentDirectory();
    EngineUtils::loadCameraConfig(CAMERA_WIDTH, CAMERA_HEIGHT);
    
    rapidjson::Document config;
    EngineUtils::ReadJsonFile("resources/" + gamePlaying + "/game.config", config);
    // Load and check initial scene

    rapidjson::Document RenderingConfig;
    // Load and parse game.config
    EngineUtils::ReadJsonFile("resources/" + gamePlaying + "/rendering.config", RenderingConfig);

    // All scene stuff
    Scene::loadAndCheckInitialScene(config);

    // Get the initial scene name
    Scene::currentScene = config["initial_scene"].GetString();
    // Construct the path for the scene file
    std::string sceneFilePath = "resources/" + gamePlaying + "/scenes/" + Scene::currentScene + ".scene";

    Scene::sparseSceneFile(sceneFilePath, hardcoded_actors);

    addActorsToGameWorld();

    Renderer Renderer;
    Renderer.Initialize();

    CameraBounds::Intialize();
    // Get all the views
    CameraBounds::calculateCameraPositions(Renderer.camera_offset_x, Renderer.camera_offset_y);
    CameraBounds::updateZoomFactorFromDocument(RenderingConfig);

    ImageDB imageDB;
   // std::vector<std::string> introImages = imageDB.getIntroImages(config, Renderer.getRenderer());

    TextDB textDB;
    // Before entering Renderer::RunLoop
    std::vector<std::string> introTexts = textDB.getIntroTexts(config);

    AudioDB audioDB;
    Renderer.currentState = GameState::Scene;
    glm::vec2 movementDirection(0.0f, 0.0f);
 
    while (Renderer.game_running) {

        Renderer.ProcessInput(movementDirection);
        // SDL_RenderClear(Renderer.getRenderer());
         // Render clear
        SDL_SetRenderDrawColor(Renderer.getRenderer(), Renderer.clearColor.r, Renderer.clearColor.g, Renderer.clearColor.b, Renderer.clearColor.a);
        SDL_RenderClear(Renderer.getRenderer());
        if (Renderer.currentState == GameState::Intro) {
          //  Renderer.RenderIntro(imageDB, textDB, audioDB, introImages, introTexts);
            if (Renderer.currentState == GameState::Scene) { continue; }
        }
        else if (Renderer.currentState == GameState::Scene) {
            
            // checkAdjacentAndContactActors(Camera, Renderer, textDB, audioDB);

            // Check if the new position is blocked and update the player's position if it's not
            CameraBounds::calculateCameraPositions(Renderer.camera_offset_x, Renderer.camera_offset_y);

            if (!hardcoded_actors.empty()) { Renderer.RenderActors(CameraBounds::cam_x_pos, CameraBounds::cam_y_pos, CameraBounds::zoom_factor); }

            if (!ImageDB::requests.empty()) { Renderer.RenderFrame(); }
            else { TextDB::RenderAllText(Renderer.getRenderer()); }

            if (Scene::loadRequested) {
                SDL_RenderPresent(Renderer.getRenderer());
                Scene::Update(Renderer);
                Scene::loadRequested = false;
                continue;
            }

            if (Renderer.currentState == GameState::Ending) {
                continue;
            }
        }
        else if (Renderer.currentState == GameState::Ending) {

            if (gameWon) {
               
            }
            else if (gameOver) {
                
            }

        }

        if (gameState == Game::Running) {
            Input::LateUpdate();
            EventBus::ProcessSubscriptions();
            RigidBody::Step();
            Actor::UpdateActors();
            SDL_RenderPresent(Renderer.getRenderer());
        }

    }
    
    return 0;
}

