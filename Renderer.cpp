#include "headers/Renderer.h"
#include "headers/EngineUtils.h"
#include <iostream>
#include "headers/Helper.h"
#include "headers/Input.h"
#include "headers/Scene.h"
#include "headers/Game.h"
#include "headers/Template.h"
#include "headers/MainHelper.h"
#include "headers/Eventbus.h"
#include "headers/RigidBody.h"
#include "headers/GameManager.h"
#include <direct.h>  // Required for _mkdir on Windows
#include <sys/stat.h>  // Required for mkdir on UNIX/Linux
#include <sys/types.h>  // Additional types might be required
#include <fstream>
#include <map>


extern std::vector<std::shared_ptr<Actor>> hardcoded_actors;

extern enum class VerticalDirection;
extern enum class HorizontalDirection;
enum class ActorMode;
enum class PlayerMode;

float CameraBounds::zoom_factor;
float CameraBounds::cam_x_pos, CameraBounds::cam_y_pos;
int CameraBounds::halfWidth, CameraBounds::halfHeight;
float CameraBounds::cam_ease_factor;
float CameraBounds::user_camera_x_pos, CameraBounds::user_camera_y_pos;

extern std::string gamePlaying;

Renderer::Renderer() :  windowWidth(640), windowHeight(360), clearColor{ 255, 255, 255, 255 }, windowTitle("") {
  
}

Renderer::~Renderer() {
    if (hpTexture) {
        SDL_DestroyTexture(hpTexture);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();

}

bool Renderer::Initialize() {
    LoadConfigurations();
    CreateWindowAndRenderer();
    return true;
}

void Renderer::LoadConfigurations() {
    rapidjson::Document gameConfig;
    EngineUtils::ReadJsonFile("resources/" + gamePlaying + "/game.config", gameConfig); // Load game config
    if (gameConfig.HasMember("game_title") && gameConfig["game_title"].IsString()) {
        windowTitle = gameConfig["game_title"].GetString();
    }
//    ImageDB::checkAllIntroImagesExists(gameConfig);
    TextDB::checkAllIntroFontsExists(gameConfig);

    rapidjson::Document renderingConfig;
    const std::filesystem::path configFile{ "resources/" + gamePlaying + "/rendering.config" };
    if (std::filesystem::exists(configFile)) {


        EngineUtils::ReadJsonFile("resources/" + gamePlaying + "/rendering.config", renderingConfig); // Load rendering config
        if (renderingConfig.HasMember("x_resolution") && renderingConfig["x_resolution"].IsInt()) {
            windowWidth = renderingConfig["x_resolution"].GetInt();
        }
        if (renderingConfig.HasMember("y_resolution") && renderingConfig["y_resolution"].IsInt()) {
            windowHeight = renderingConfig["y_resolution"].GetInt();
        }
        if (renderingConfig.HasMember("cam_offset_x") && renderingConfig["cam_offset_x"].IsNumber()) {
            camera_offset_x = renderingConfig["cam_offset_x"].GetFloat(); // Assuming offset is stored as a float
        }
        if (renderingConfig.HasMember("cam_offset_y") && renderingConfig["cam_offset_y"].IsNumber()) {
            camera_offset_y = renderingConfig["cam_offset_y"].GetFloat();
        }

        if (renderingConfig.HasMember("clear_color_r") && renderingConfig["clear_color_r"].IsInt()) {
            clearColor.r = static_cast<Uint8>(renderingConfig["clear_color_r"].GetInt());
        }
        if (renderingConfig.HasMember("clear_color_g") && renderingConfig["clear_color_g"].IsInt()) {
            clearColor.g = static_cast<Uint8>(renderingConfig["clear_color_g"].GetInt());
        }
        if (renderingConfig.HasMember("clear_color_b") && renderingConfig["clear_color_b"].IsInt()) {
            clearColor.b = static_cast<Uint8>(renderingConfig["clear_color_b"].GetInt());
        }
        
        clearColor.a = 255;
    }
    // Clear the renderer with the set color
}

void Renderer::RenderIntro(ImageDB& imageDB, TextDB& textDB, AudioDB& audioDB, std::vector<std::string>& introImages, std::vector<std::string>& introTexts) {
       
    bool hasIntroContent = !introImages.empty() || !introTexts.empty();

    // Check if both intro images and texts have been exhausted
    if (currentIndex >= introImages.size() && currentIndex >= introTexts.size()) {
        hasIntroContent = false;
        currentState = GameState::Scene;
    }

    // Render the current intro image
    if (currentIndex < introImages.size()) {
        SDL_Texture* texture = imageDB.loadImage(introImages[currentIndex], renderer); // Ensure loadImage is properly implemented
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
    else if (!introImages.empty() && currentIndex < introTexts.size()) {
        SDL_Texture* texture = imageDB.loadImage(introImages[introImages.size() - 1], renderer); // Ensure loadImage is properly implemented
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }

    //// Render the current intro text if available
    //if (currentIndex < introTexts.size()) {
    //    SDL_Color whiteColor = { 255, 255, 255, 255 }; // White color
    //    std::string defaultFontName = "default"; // Ensure this matches the key used in fontCache
    //    textDB.DrawText(introTexts[currentIndex], 25, windowHeight - 50, defaultFontName, whiteColor);
    //}
    //else if (!introTexts.empty() && currentIndex < introImages.size()) {
    //    SDL_Color whiteColor = { 255, 255, 255, 255 }; // White color
    //    std::string defaultFontName = "default"; // Ensure this matches the key used in fontCache
    //    textDB.DrawText(introTexts[introTexts.size() - 1], 25, windowHeight - 50, defaultFontName, whiteColor);
    //}
    //textDB.RenderAllText(renderer);

}

void Renderer::CreateWindowAndRenderer() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        exit(0);
    }

     window = SDL_CreateWindow(windowTitle.c_str(),
           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(0); // Or handle more gracefully
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        exit(0); // Or handle more gracefully
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
  //  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImGui::StyleColorsDark(); // Start with the dark theme

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f; // Set window corners to be rounded
    style.FrameRounding = 3.0f;  // Set button and field corners to be rounded
    style.ItemSpacing = ImVec2(10, 10); // Add more space between items

    // Customize colors
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 0.9f); // Background color for the window
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);    // Background color for headers
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.25f, 0.3f, 1.0f);      // Background color for buttons
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f); // Background color for button hover

}


// Method to get the SDL_Renderer
SDL_Renderer* Renderer::getRenderer() {
    return renderer;
}

static bool showCreateGameWindow = false;
static int activeSubMenu = -1;
static bool restartGame = false
;
void RenderImGui() {
    ImGui::Begin("Game Controls");

    // Button to toggle game state
    if (gameState == Game::Paused) {
        if (ImGui::Button("Play")) {
            gameState = Game::Running;
        }
    }
    else if (gameState == Game::Running) {
        if (ImGui::Button("Pause")) {
            gameState = Game::Paused;
        }
    }

    if (ImGui::Button("Restart current game")) {
        restartGame = true;
    }

    ImGui::End();
}

void RenderCreateGames() {
    ImGui::Begin("Render Create Games");

    // Button to open the game creation window
    if (ImGui::Button("Wanna create a game?")) {
        showCreateGameWindow = true;  // Set the flag to show the game creation window
    }

    ImGui::End();
}

void RenderGameCreationWindow() {
    if (showCreateGameWindow) {
        ImGui::Begin("Create Your Game", &showCreateGameWindow); // Pass a pointer to showCreateGameWindow to allow closing with the ImGui close button
        
        // Buttons for 13 sub-menus
        for (int i = 0; i < 14; i++) {
            char buf[32];  // Buffer to hold the button label

            if (i == 0) {
                sprintf(buf, "Actor %d", i + 1);
            }
            else if (i == 1) {
                sprintf(buf, "Application %d", i + 1);
            }
            else if (i == 2) {
                sprintf(buf, "Input %d", i + 1);
            }
            else if (i == 3) {
                sprintf(buf, "Text %d", i + 1);
            }
            else if (i == 4) {
                sprintf(buf, "AudioDB %d", i + 1);
            }
            else if (i == 5) {
                sprintf(buf, "ImageDB %d", i + 1);
            }
            else if (i == 6) {
                sprintf(buf, "CameraBounds %d", i + 1);
            }
            else if (i == 7) {
                sprintf(buf, "Scene %d", i + 1);
            }
            else if (i == 8) {
                sprintf(buf, "Rigidbody %d", i + 1);
            }
            else if (i == 9) {
                sprintf(buf, "Collision %d", i + 1);
            }
            else if (i == 10) {
                sprintf(buf, "Physics %d", i + 1);
            }
            else if (i == 11) {
                sprintf(buf, "EventBus %d", i + 1);
            }
            else if (i == 12) {
                sprintf(buf, "Vector2 %d", i + 1);
            }
            else if (i == 13) {
                sprintf(buf, "GameManager %d", i + 1);
            }

            // Create a button with the label from buf
            if (ImGui::Button(buf)) {
                activeSubMenu = i;  // Set the active sub-menu when a button is clicked
            }
        }


        if (activeSubMenu == 0) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Actor Class");
            ImGui::Text("This section details the methods of the Actor class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("GetName() - Retrieve the actor's name.");
            ImGui::BulletText("GetID() - Fetch the unique identifier of the actor.");
            ImGui::BulletText("GetComponentByKey(key) - Get a component of the actor by its key.");
            ImGui::BulletText("GetComponent(type) - Get a component based on its type.");
            ImGui::BulletText("GetComponents() - Retrieve all components attached to the actor.");
            ImGui::BulletText("Find(name) - Static function to find an actor by name.");
            ImGui::BulletText("FindAll(name) - Static function to find all actors by name.");
            ImGui::BulletText("AddComponent(component) - Add a component to the actor from Lua.");
            ImGui::BulletText("RemoveComponent(component) - Remove a component from the actor from Lua.");
            ImGui::BulletText("Instantiate() - Static function to create a new actor instance.");
            ImGui::BulletText("Destroy() - Static function to destroy an actor.");

            ImGui::Text("These bindings allow Lua scripts to directly manipulate game objects.");
        }

        // Check if the active sub-menu is the second one (index 1, but we display as Sub-Menu 2)
        if (activeSubMenu == 1) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Application Namespace");
            ImGui::Text("This section details the global application functions exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("Quit() - Exits the application.");
            ImGui::BulletText("Sleep(milliseconds) - Delays the application for a specified number of milliseconds.");
            ImGui::BulletText("GetFrame() - Returns the current frame count since the application started.");
            ImGui::BulletText("OpenURL(url) - Opens a URL in the default web browser.");

            ImGui::Text("These functions provide basic application control and utility features accessible from Lua scripts.");
        }

        // Depending on activeSubMenu, show more detailed info
        if (activeSubMenu == 2) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Input Class");
            ImGui::Text("This section details the methods of the Input class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("GetKey(keyCode) - Returns true if the specified key is currently being pressed.");
            ImGui::BulletText("GetKeyDown(keyCode) - Returns true if the specified key was pressed down in the current frame.");
            ImGui::BulletText("GetKeyUp(keyCode) - Returns true if the specified key was released in the current frame.");
            ImGui::BulletText("GetMousePosition() - Returns the current position of the mouse cursor as a 'vec2'.");
            ImGui::BulletText("GetMouseButton(button) - Returns true if the specified mouse button is currently being pressed.");
            ImGui::BulletText("GetMouseButtonDown(button) - Returns true if the specified mouse button was pressed down in the current frame.");
            ImGui::BulletText("GetMouseButtonUp(button) - Returns true if the specified mouse button was released in the current frame.");
            ImGui::BulletText("GetMouseScrollDelta() - Returns the amount the mouse wheel has been scrolled in the current frame.");

            ImGui::Separator();
            ImGui::Text("Lua Integration with the vec2 Class");
            ImGui::Text("This section details the properties of the vec2 class exposed to Lua:");

            // Explanation for glm::vec2
            ImGui::BulletText("x - The x coordinate of the vector.");
            ImGui::BulletText("y - The y coordinate of the vector.");

            ImGui::Text("These functions and properties allow Lua scripts to handle user input and manipulate 2D vector data.");
            
        }

        // Check if the active sub-menu is the third one (index 2, but we display as Sub-Menu 3)
        if (activeSubMenu == 3) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the TextDB Class");
            ImGui::Text("This section details the methods of the TextDB class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("Draw(text, x, y, scale, color) - Draws text at the specified position with given scale and color.");

            ImGui::Text("This function allows Lua scripts to render text on the screen, providing basic text drawing capabilities. The parameters are:");
            ImGui::BulletText("text - The string of text to draw.");
            ImGui::BulletText("x - The x coordinate on the screen where the text starts.");
            ImGui::BulletText("y - The y coordinate on the screen where the text starts.");
            ImGui::BulletText("scale - How much to scale the text from its original size.");
            ImGui::BulletText("color - The color to draw the text in, usually specified as RGBA.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Text.Draw('Hello, world!', 100, 200, 1, {255, 255, 255, 255})");

            ImGui::Text("This function is particularly useful for creating dynamic interfaces, debugging displays, or any in-game text rendering.");
        }

        // Check if the active sub-menu is the fifth one (index 4, but we display as Sub-Menu 5)
        if (activeSubMenu == 4) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the AudioDB Class");
            ImGui::Text("This section details the audio manipulation methods of the AudioDB class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("Play(soundID) - Plays the sound effect associated with the given sound ID.");
            ImGui::BulletText("Halt(soundID) - Halts playback of the sound effect associated with the given sound ID.");
            ImGui::BulletText("SetVolume(soundID, volume) - Sets the volume for the sound effect associated with the given sound ID.");

            ImGui::Text("These functions allow Lua scripts to control audio playback, useful for dynamic sound effects in-game. The parameters are:");
            ImGui::BulletText("soundID - The identifier for the sound effect. Usually defined during the game setup.");
            ImGui::BulletText("volume - A value between 0 and 100, where 100 is the maximum volume.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Audio.Play('explosion')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Audio.Halt('background_music')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Audio.SetVolume('game_music', 75)");

            ImGui::Text("These methods are integral for creating immersive game environments and managing interactive soundscapes.");
        }

        // Check if the active sub-menu is the sixth one (index 5, but we display as Sub-Menu 6)
        if (activeSubMenu == 5) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the ImageDB Class");
            ImGui::Text("This section details the methods of the ImageDB class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("DrawUI(imageID, x, y, width, height) - Draws a UI image at the specified coordinates.");
            ImGui::BulletText("DrawUIEx(imageID, x, y, width, height, rotation, color) - Draws a UI image with additional parameters like rotation and color.");
            ImGui::BulletText("Draw(imageID, x, y) - Draws an image at the specified screen coordinates.");
            ImGui::BulletText("DrawEx(imageID, x, y, scaleX, scaleY, rotation, color) - Draws an image with scaling, rotation, and color adjustments.");
            ImGui::BulletText("DrawPixel(x, y, color) - Draws a single pixel at the specified screen coordinates.");

            ImGui::Text("These functions provide various ways to render images and pixels directly from Lua scripts, allowing for dynamic visual effects. The parameters are:");
            ImGui::BulletText("imageID - A unique identifier for the image.");
            ImGui::BulletText("x, y - Screen coordinates for image placement.");
            ImGui::BulletText("width, height - Dimensions for the image when applicable.");
            ImGui::BulletText("scaleX, scaleY - Scaling factors for the image.");
            ImGui::BulletText("rotation - Rotation angle in degrees.");
            ImGui::BulletText("color - Color to apply, typically RGBA.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Image.DrawUI('logo', 100, 200, 300, 150)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Image.DrawEx('sprite', 400, 300, 1.5, 1.5, 90, {255, 255, 255, 255})");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Image.DrawPixel(500, 500, {0, 255, 0, 255})");

            ImGui::Text("These methods are integral for creating and managing dynamic visual content within the game, offering extensive control over graphics rendering.");
        }

        // Check if the active sub-menu is the seventh one (index 6, but we display as Sub-Menu 7)
        if (activeSubMenu == 6) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the CameraBounds Class");
            ImGui::Text("This section details the camera manipulation methods of the CameraBounds class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("SetPosition(x, y) - Sets the camera's position in the game world.");
            ImGui::BulletText("GetPositionX() - Returns the current X-coordinate of the camera's position.");
            ImGui::BulletText("GetPositionY() - Returns the current Y-coordinate of the camera's position.");
            ImGui::BulletText("SetZoom(zoomLevel) - Sets the zoom level of the camera.");
            ImGui::BulletText("GetZoom() - Retrieves the current zoom level of the camera.");

            ImGui::Text("These functions allow Lua scripts to control the camera's position and zoom, providing dynamic views within the game. The parameters are:");
            ImGui::BulletText("x, y - The new coordinates for the camera.");
            ImGui::BulletText("zoomLevel - A numerical value specifying the zoom level, where higher values denote a closer view.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Camera.SetPosition(100, 200)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Camera.SetZoom(1.5)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local x = Camera.GetPositionX()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local y = Camera.GetPositionY()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local zoom = Camera.GetZoom()");

            ImGui::Text("This functionality is crucial for games that require a variable perspective or need to focus on specific areas or objects dynamically.");
        }

        // Check if the active sub-menu is the eighth one (index 7, but we display as Sub-Menu 8)
        if (activeSubMenu == 7) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Scene Class");
            ImGui::Text("This section details the methods of the Scene class exposed to Lua:");

            // Detailed explanations
            ImGui::BulletText("Load(sceneName) - Loads a new scene, transitioning from the current scene.");
            ImGui::BulletText("GetCurrent() - Returns the name of the current active scene.");
            ImGui::BulletText("DontDestroy(object) - Marks an object to not be destroyed when loading a new scene.");

            ImGui::Text("These functions allow Lua scripts to control scene transitions and management, crucial for game flow. The parameters are:");
            ImGui::BulletText("sceneName - The name of the scene to load.");
            ImGui::BulletText("object - The game object to persist across scene changes.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Scene.Load('Level2')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local currentScene = Scene.GetCurrent()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Scene.DontDestroy(playerObject)");

            ImGui::Text("These methods are fundamental for creating a seamless gameplay experience, allowing for dynamic transitions and preservation of crucial game state across scenes.");
        }

        // Check if the active sub-menu is the ninth one (index 8, but we display as Sub-Menu 9)
        if (activeSubMenu == 8) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Rigidbody Class");
            ImGui::Text("This section details the properties and methods of the Rigidbody class exposed to Lua:");

            // Properties Explanations
            ImGui::Text("Properties:");
            ImGui::BulletText("x, y - The position coordinates of the Rigidbody in the game world.");
            ImGui::BulletText("body_type - The type of physics body (static, dynamic, kinematic).");
            ImGui::BulletText("gravity_scale - Scale of the gravity applied to this Rigidbody.");
            ImGui::BulletText("density - The density of the Rigidbody, affecting its mass.");
            ImGui::BulletText("rotation - The rotation angle of the Rigidbody.");
            ImGui::BulletText("width, height, radius - Dimensions of the Rigidbody's collider.");
            ImGui::BulletText("bounciness - The restitution coefficient of the Rigidbody.");
            ImGui::BulletText("friction - The friction coefficient of the Rigidbody.");
            ImGui::BulletText("trigger_type, trigger_width, trigger_height, trigger_radius - Properties defining trigger areas if any.");

            // Methods Explanations
            ImGui::Text("Methods:");
            ImGui::BulletText("GetPosition() - Returns the current position.");
            ImGui::BulletText("GetRotation() - Returns the current rotation angle.");
            ImGui::BulletText("AddForce(forceVec) - Applies a force vector to the Rigidbody.");
            ImGui::BulletText("SetVelocity(velocityVec) - Sets the velocity vector of the Rigidbody.");
            ImGui::BulletText("SetPosition(x, y) - Sets the position of the Rigidbody.");
            ImGui::BulletText("SetRotation(angle) - Sets the rotation of the Rigidbody.");
            ImGui::BulletText("GetVelocity() - Returns the current velocity vector.");
            ImGui::BulletText("SetUpDirection(directionVec) - Sets the 'up' direction vector.");
            ImGui::BulletText("GetUpDirection() - Gets the current 'up' direction vector.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local rb = Rigidbody()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "rb:SetPosition(100, 200)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "rb:AddForce({x=0, y=-10})");

            ImGui::Text("These functionalities are crucial for handling complex physics interactions in the game, providing a realistic and dynamic environment.");
        }

        // Check if the active sub-menu is the tenth one (index 9, but we display as Sub-Menu 10)
        if (activeSubMenu == 9) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Collision and b2Vec2 Classes");
            ImGui::Text("This section details the properties and usage of the Collision class and the b2Vec2 class exposed to Lua:");

            // Detailed explanations for b2Vec2
            ImGui::Text("b2Vec2 Class:");
            ImGui::BulletText("b2Vec2 - A class representing a 2D vector.");
            ImGui::BulletText("x, y - The x and y coordinates of the vector.");

            // Detailed explanations for Collision
            ImGui::Text("Collision Class:");
            ImGui::BulletText("other - The other object involved in the collision.");
            ImGui::BulletText("point - The point at which the collision occurred.");
            ImGui::BulletText("relative_velocity - The velocity relative to the other object at the point of impact.");
            ImGui::BulletText("normal - The collision normal vector.");

            ImGui::Text("These properties are used to manage and respond to collisions in game dynamics. They provide crucial information for resolving collisions and for triggering game logic related to interactions between objects.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local col = Collision()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local position = b2Vec2()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "position.x = 10; position.y = 20");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "col.point = position");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "print('Collision at:', col.point.x, col.point.y)");

            ImGui::Text("This functionality is fundamental for creating interactive and responsive environments where objects react realistically to collisions.");
        }

       // Check if the active sub-menu is the tenth one (index 9, but we display as Sub-Menu 10)
        if (activeSubMenu == 10) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Physics and HitResult Classes");
            ImGui::Text("This section details the methods and data structures for physics calculations exposed to Lua:");

            // HitResult class description
            ImGui::Text("HitResult Class:");
            ImGui::BulletText("actor - The Actor involved in the physics interaction.");
            ImGui::BulletText("point - The point at which the interaction occurs.");
            ImGui::BulletText("normal - The normal vector at the point of interaction.");
            ImGui::BulletText("fraction - The fraction of the distance along the ray where the interaction occurs.");
            ImGui::BulletText("is_trigger - Indicates if the interaction was with a trigger.");

            // Physics class description
            ImGui::Text("Physics Class:");
            ImGui::BulletText("Raycast(startPoint, endPoint) - Performs a raycast from start to end point, returning a HitResult.");
            ImGui::BulletText("RaycastAll(startPoint, endPoint) - Performs a raycast that returns all hits along the ray's path.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local hit = Physics.Raycast({x=0, y=0}, {x=100, y=100})");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "if hit then");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "    print('Hit at:', hit.point.x, hit.point.y)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "end");

            ImGui::Text("This functionality is crucial for physics calculations and game interactions involving collision detection.");
        }

        // Check if the active sub-menu is the eleventh one (index 10, but we display as Sub-Menu 11)
        if (activeSubMenu == 11) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the EventBus Class");
            ImGui::Text("This section explains the methods for managing event interactions within Lua:");

            // EventBus class description
            ImGui::Text("EventBus Class:");
            ImGui::BulletText("Publish(eventType, eventArgs) - Publishes an event to all subscribed listeners.");
            ImGui::BulletText("Subscribe(eventType, listener) - Subscribes a listener function to an event type.");
            ImGui::BulletText("Unsubscribe(eventType, listener) - Unsubscribes a listener from an event type.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Event.Subscribe('PlayerJump', onPlayerJump)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Event.Publish('PlayerJump', {height = 2})");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "Event.Unsubscribe('PlayerJump', onPlayerJump)");

            ImGui::Text("These methods facilitate a robust system for handling events, allowing scripts to interact dynamically with game logic.");
        }

        // Check if the active sub-menu is the twelfth one (index 11, but we display as Sub-Menu 12)
        if (activeSubMenu == 12) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Lua Integration with the Vector2 Class");
            ImGui::Text("This section details the properties, methods, and operators of the Vector2 class exposed to Lua:");

            // Detailed explanations
            ImGui::Text("Properties:");
            ImGui::BulletText("x, y - The horizontal and vertical components of the vector.");

            ImGui::Text("Methods:");
            ImGui::BulletText("Normalize() - Normalizes the vector so that its length is 1.");
            ImGui::BulletText("Length() - Returns the length (magnitude) of the vector.");
            ImGui::BulletText("Distance(v1, v2) - Calculates the distance between two vectors.");
            ImGui::BulletText("Dot(v1, v2) - Computes the dot product of two vectors.");

            ImGui::Text("Operators:");
            ImGui::BulletText("__add - Allows adding two Vector2 objects with the '+' operator.");
            ImGui::BulletText("__sub - Allows subtracting two Vector2 objects with the '-' operator.");
            ImGui::BulletText("__mul - Allows multiplying a Vector2 object with a scalar or another Vector2 with the '*' operator.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local v1 = Vector2(10, 20)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local v2 = Vector2(5, 5)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local v3 = v1 + v2");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "print(v3.x, v3.y)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local dotProduct = Vector2.Dot(v1, v2)");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "print('Dot Product:', dotProduct)");

            ImGui::Text("This class provides foundational mathematical functionalities for handling 2D vectors in game development, supporting both simple arithmetic and complex geometric calculations.");
        }

        // Check if the active sub-menu is the thirteenth one (index 12, but we display as Sub-Menu 13)
        if (activeSubMenu == 13) {
            ImGui::Text("You are viewing Sub-Menu %d", activeSubMenu + 1);
            ImGui::Separator();
            ImGui::Text("Exporting GameManager Functions to Lua");
            ImGui::Text("This section details exporting game management functions to Lua, enabling script-based interaction:");

            // Detailed explanations
            ImGui::Text("Available Functions:");
            ImGui::BulletText("AddGame(game) - Adds a game to the list.");
            ImGui::BulletText("DeleteGame(game) - Removes a game from the list.");
            ImGui::BulletText("ListGames() - Displays all games in the list.");
            ImGui::BulletText("FindGame(game_name) - Checks if a game exists in the list.");
            ImGui::BulletText("SortGames() - Sorts the list of games alphabetically.");
            ImGui::BulletText("RandomizeGames() - Shuffles the list of games.");
            ImGui::BulletText("LoadGamesFromFile(filename) - Loads games from a specified file.");
            ImGui::BulletText("SaveGamesToFile(filename) - Saves the current list of games to a file.");
            ImGui::BulletText("CountGames() - Returns the number of games in the list.");
            ImGui::BulletText("RemoveAllGames() - Clears the list of games.");
            ImGui::BulletText("MergeLists(otherList) - Compares and merges another list into the current list.");

            ImGui::Text("Example usage in Lua:");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.AddGame('New Game')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.DeleteGame('Old Game')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local games = UI.ListGames()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local exists = UI.FindGame('Chess')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.SortGames()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.RandomizeGames()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.LoadGamesFromFile('games.txt')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.SaveGamesToFile('saved_games.txt')");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "local gameCount = UI.CountGames()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.RemoveAllGames()");
            ImGui::TextColored(ImVec4(0.5, 0.8, 0.1, 1), "UI.MergeLists({'Game1', 'Game2'})");

            ImGui::Text("These Lua bindings provide a robust interface for managing game lists through scripts, enhancing modularity and script-based controls.");
        }


        ImGui::End();
    }
}


int selectedFolderIndex = 0;
static char newFolderName[128] = ""; // Buffer for folder name input
static bool createFolderFlag = false; // Flag to trigger folder creation

// In your ImGui rendering function
void Renderer::RenderFolderSelection() {
    ImGui::Begin("Game Selection");

    std::vector<const char*> items;
    for (const auto& folderName : GameManager::folderList) {
        items.push_back(folderName.c_str());
    }


    if (ImGui::ListBox("Select Game", &selectedFolderIndex, &items[0], items.size())) {

        if (GameManager::folderList[selectedFolderIndex] == gamePlaying) {
            gameState = Game::Running;
            ImGui::End();
            return;
        }
       
        gamePlaying = GameManager::folderList[selectedFolderIndex];
        Actor::clearAll();
        hardcoded_actors.clear();
        AudioDB::clearAll();
        EventBus::clearAll();
        ImageDB::clearAll();
        LuaHelper::component_tables.clear();
        TemplateDB::templates.clear();
        TextDB::fontCache.clear();

        if (hpTexture) {
            SDL_DestroyTexture(hpTexture);
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }


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
          
        // Word removal
        RigidBody::world = nullptr;

        LoadConfigurations();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
            exit(0);
        }

        window = SDL_CreateWindow(windowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            exit(0); // Or handle more gracefully
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            exit(0); // Or handle more gracefully
        }


        LuaHelper::loadAndCacheAllBaseTables();
        Scene::sparseSceneFile(sceneFilePath, hardcoded_actors);
        CameraBounds::Intialize();
        // Get all the views
        CameraBounds::calculateCameraPositions(camera_offset_x, camera_offset_y);
        CameraBounds::updateZoomFactorFromDocument(RenderingConfig);

        gameState = Game::Running;

    }

    ImGui::End();
}


void createNewGame() {

    ImGui::Begin("Game Creation");

    ImGui::Separator();
    ImGui::Text("Create New Game Folder:");
    ImGui::InputText("Game Folder Name", newFolderName, IM_ARRAYSIZE(newFolderName));
    if (ImGui::Button("Create")) {
        if (strlen(newFolderName) > 0) { // Ensure the name is not empty
            createFolderFlag = true;
        }
    }

    // Handle new folder creation
    if (createFolderFlag) {
        std::string folderPath = "resources/" + std::string(newFolderName);
        if (_mkdir(folderPath.c_str()) != 0) {
            std::cerr << "Failed to create folder: " << strerror(errno) << std::endl;
        }
        else {
            std::cout << "Folder created successfully: " << folderPath << std::endl;
            // Optionally, add the new folder to your folder list and reset input
            GameManager::folderList.push_back(strdup(newFolderName)); // Make sure to manage the memory appropriately
            memset(newFolderName, 0, sizeof(newFolderName)); // Reset the input buffer
        }
        createFolderFlag = false; // Reset flag
    }

    ImGui::End();
}

void SeeGameCode(std::string currentPath) {
    static std::string currentDir = currentPath; // Maintain the current directory path
    static std::string selectedFile; // Path to the currently selected file
    static std::string fileContents; // Contents of the selected file

    ImGui::Begin("Directory Viewer");

    // Button to go up one directory level
    if (ImGui::Button("Go Up") && currentDir != std::filesystem::path(currentDir).root_path()) {
        currentDir = std::filesystem::path(currentDir).parent_path().string();
        selectedFile.clear(); // Clear selected file when changing directory
        fileContents.clear(); // Clear contents
    }

    ImGui::Separator();

    // Iterate over the directory entries
    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            const auto& path = entry.path();
            std::string filename = path.filename().string();

            // Push a unique ID to avoid conflicts in ImGui
            ImGui::PushID(filename.c_str());

            if (entry.is_directory()) {
                // Directory entries as buttons
                if (ImGui::Button(filename.c_str())) {
                    currentDir = path.string();  // Navigate into the directory
                    selectedFile.clear(); // Clear selected file when changing directory
                    fileContents.clear(); // Clear contents
                }
            }
            else {
                // File entries as clickable buttons
                if (ImGui::Button(filename.c_str())) {
                    selectedFile = path.string();
                    // Open and read the file contents
                    std::ifstream file(selectedFile);
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    fileContents = buffer.str(); // Store the contents in a string
                }
            }

            ImGui::PopID();
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        ImGui::Text("Error accessing path: %s", e.what());
    }

    // Declare a buffer size constant for new file creation and file editing
    const size_t bufferSize = 1024 * 16;  
    static char buffer[bufferSize] = "";

    if (!selectedFile.empty()) {
        ImGui::Separator();
        ImGui::BeginChild("FileEditor", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true); // Begin a scrollable region

        // Copy the file contents to the buffer for editing
        std::strncpy(buffer, fileContents.c_str(), sizeof(buffer));
        // Ensure null termination
        buffer[sizeof(buffer) - 1] = '\0';

        // Editable text box
        if (ImGui::InputTextMultiline("##editor", buffer, bufferSize, ImVec2(-1, -1))) {
            // Set the flag that the content has been edited
            // Later you can use this to prompt for saving if needed
            fileContents.assign(buffer);  // Update fileContents with the edited text
        }

        ImGui::EndChild(); // End scrollable region

        // Button to save the edited content back to the file
        if (ImGui::Button("Save")) {
            // Save the buffer to the file
            std::ofstream out(selectedFile);
            out << fileContents;
            out.close();
        }
    }
    ImGui::End();

    // Input for creating new file
    static char newFileName[256] = "";
    ImGui::InputText("New File Name", newFileName, IM_ARRAYSIZE(newFileName));
    ImGui::SameLine();
    if (ImGui::Button("Create File")) {
        std::string newFilePath = currentDir + "/" + std::string(newFileName);
        std::ofstream outFile(newFilePath);
        if (outFile) {
            // Optionally clear the buffer if you want to reuse it for the new file
            memset(buffer, 0, sizeof(buffer));
            outFile.close();
            selectedFile = newFilePath; // Select the new file for editing
            fileContents.clear();       // Clear the previous contents
        }
        else {
            // Handle the error, e.g., show an error message
        }
        memset(newFileName, 0, sizeof(newFileName)); // Clear the new file name buffer
    }
}



void Renderer::ProcessInput(glm::vec2 &movementDirection) {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) { // Use the custom event polling function

        
        // std::cout << "Before Process frame" << Helper::GetFrameNumber() << std::endl;
        Input::ProcessEvent(e);
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) {
            
            game_running = false;
            break;
        }
        else if (e.type == SDL_KEYDOWN && currentState == GameState::Intro) {
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE || e.key.keysym.scancode == SDL_SCANCODE_RETURN) {  
                currentIndex++;    
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && currentState == GameState::Intro) {
            currentIndex++;
        }
        // Update non-player actors every 60 frames, starting from frame 60.
        else if (currentState == GameState::Scene){
  
            //glm::vec2 direction(0.0f, 0.0f);
            //if (Input::GetKey(SDL_SCANCODE_W) || Input::GetKey(SDL_SCANCODE_UP)) {
            //    direction.y -= 1.0f;
            //}
            //if (Input::GetKey(SDL_SCANCODE_S) || Input::GetKey(SDL_SCANCODE_DOWN)) {
            //    direction.y += 1.0f;
            //}
            //if (Input::GetKey(SDL_SCANCODE_A) || Input::GetKey(SDL_SCANCODE_LEFT)) {
            //    direction.x -= 1.0f;
            //}
            //if (Input::GetKey(SDL_SCANCODE_D) || Input::GetKey(SDL_SCANCODE_RIGHT)) {
            //    direction.x += 1.0f;
            //}

            //if (direction.x != 0.0f || direction.y != 0.0f) {
            //    // Normalize if direction is not zero
            //    direction = glm::normalize(direction);
            //}
            //if (glm::length(direction) > 0.0f) {
            //    direction = glm::normalize(direction);
            //}
            //else {
            //    movementDirection = glm::vec2(0.0f, 0.0f); // No movement if direction is zero
            //}
            
        }
        if (e.key.keysym.scancode == SDL_SCANCODE_P) {
            if (gameState == Game::Running) {
                gameState = Game::Paused;
                break;
            }
            else if (gameState == Game::Paused) {
                gameState = Game::Running;
                break;
            }
        }
    }
    // Perform any late updates for the input system
 /*   Input::LateUpdate();*/
   // std::cout << "After Late frame" << Helper::GetFrameNumber() << std::endl;

   
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    RenderImGui(); // Your ImGui rendering function
    RenderCreateGames();
    RenderFolderSelection();
    RenderGameCreationWindow();
    createNewGame();
    SeeGameCode("C:\\Users\\porve\\source\\repos\\game_engine\\resources");
 
    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    if (gameState == Game::Paused) { SDL_RenderPresent(renderer); }
}

void printLuaRefValue(const luabridge::LuaRef& ref) {
    if (!ref.isNil()) { // Check if the LuaRef is not nil
        if (ref.isNumber()) {
            std::cout << "Number: " << ref.cast<double>() << std::endl;
        }
        else if (ref.isString()) {
            std::cout << "String: " << ref.cast<std::string>() << std::endl;
        }
        else if (ref.isBool()) {
            std::cout << "Boolean: " << (ref.cast<bool>() ? "true" : "false") << std::endl;
        }
        else if (ref.isTable()) {
            std::cout << "Table: " << std::endl;
            for (luabridge::Iterator iter(ref); !iter.isNil(); ++iter) {
                std::cout << "\tKey: ";
                printLuaRefValue(iter.key()); // Recursively print the key
                std::cout << "\tValue: ";
                printLuaRefValue(iter.value()); // Recursively print the value
                std::cout << std::endl;
            }
        }
        // Add more type checks as needed (e.g., functions, userdata)
    }
    else {
        std::cout << "Nil" << std::endl;
    }
}


void ReportError(const std::string& actor_name, const luabridge::LuaException& e)
{
    std::string error_message = e.what();

    /* Normalize file paths across platforms */
    std::replace(error_message.begin(), error_message.end(), '\\', '/');

    /* Display (with color codes) */
    std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}


void Renderer::RenderActors(float& cam_x_pos, float& cam_y_pos, float &zoom_factor) {
    const int PIXELS_PER_UNIT = 100; // 100 pixels represent one in-game unit


    for (auto& actor : hardcoded_actors) {
        for (const auto& [key, componentRef] : actor->actor_components) {

            try {
                // Call OnStart if it's the first frame
                if (!(*componentRef)["OnStartOver"].cast<bool>() && (*componentRef)["enabled"].cast<bool>() && (*componentRef)["OnStart"].cast<bool>() && (*componentRef)["OnStart"].isFunction()) {
                    (*componentRef)["OnStart"](*componentRef);
                    (*componentRef)["OnStartOver"] = true;
                }

            }
            catch (const luabridge::LuaException& e) {
                ReportError(actor->actor_name, e);
                (*componentRef)["OnStartOver"] = true;
            }
        }
    }
    

    // OnUpdate pass
    for (auto& actor : hardcoded_actors) {
        for (const auto& [key, componentRef] : actor->actor_components) {
            try {
                if ((*componentRef)["enabled"].cast<bool>() && (*componentRef)["OnUpdate"].isFunction()) {
                    (*componentRef)["OnUpdate"](*componentRef);
                }
            }
            catch (const luabridge::LuaException& e) {
                ReportError(actor->actor_name, e);
            }
        }
    }

    // OnLateUpdate pass
    for (auto& actor : hardcoded_actors) {
        for (const auto& [key, componentRef] : actor->actor_components) {
            try {
                if ((*componentRef)["enabled"].cast<bool>() && (*componentRef)["OnLateUpdate"].isFunction()) {
                    (*componentRef)["OnLateUpdate"](*componentRef);
                }
            }
            catch (const luabridge::LuaException& e) {
                ReportError(actor->actor_name, e);
            }
        }
    }

}



void Renderer::RenderHUD(const std::string& hpImagePath, TextDB& textDB, std::vector<std::pair<int, std::string>>& dialogueEntries) {
   
    int hpImageWidth, hpImageHeight;
    SDL_QueryTexture(hpTexture, NULL, NULL, &hpImageWidth, &hpImageHeight);

    
    std::string scoreText = "score : ";
    SDL_Color whiteColor = { 255, 255, 255, 255 };
//    textDB.DrawText(scoreText, 5, 5, "default", whiteColor); // Make sure to use the same font and color as intro text

    // Rendering dialogues
    int dialogueIndex = 0;
    for (const auto& entry : dialogueEntries) {
        int yPos = windowHeight - 50 - (dialogueEntries.size() - 1 - dialogueIndex) * 50;
     //   textDB.DrawText(entry.second, 25, yPos, "default", { 255, 255, 255, 255 }); // Assuming white color
        dialogueIndex++;
    }

    textDB.RenderAllText(renderer);
    dialogueEntries.clear();
}


void Renderer::RenderFrame() {

    bool textDone = false;

    std::stable_sort(ImageDB::requests.begin(), ImageDB::requests.end(), [](const RenderRequest& a, const RenderRequest& b) {
        if (a.type != b.type) return a.type < b.type;
        return a.sorting_order < b.sorting_order;
        });

    for (const auto& request : ImageDB::requests) {
        switch (request.type) {

        case RenderRequest::ImageType::Scene:
            renderImage(request);
            break;

        case RenderRequest::ImageType::UI:
            renderUI(request);
            break;
            
        case RenderRequest::ImageType::Pixel:
            if (!textDone) {
                TextDB::RenderAllText(getRenderer());
                textDone = true;
            }

            drawPixel(request);
            break;
        }   
    }

    if (!textDone) {
        TextDB::RenderAllText(getRenderer());
        textDone = true;
    }
   
    ImageDB::requests.clear();
}

void Renderer::renderImage(const RenderRequest& request) {

    SDL_RenderSetScale(renderer, CameraBounds::zoom_factor, CameraBounds::zoom_factor);
    // Retrieve the texture from your image cache/database
    SDL_Texture* texture = ImageDB::loadImage(request.image_name, getRenderer());
    if (!texture) {
        return; // Texture not found, skip rendering
    }

    // Query the texture size
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
    
    float pivotX;
    float pivotY;
    
    if (request.pivot_x == -1) {
        pivotX = static_cast<int>(textureWidth * 0.5);
    }
    else {
        pivotX = static_cast<int>(request.pivot_x * textureWidth * request.scale_x);
    }

    if (request.pivot_y == -1) {
        pivotY = static_cast<int>(textureHeight * 0.5);
    }
    else {
        pivotY = static_cast<int>(request.pivot_y * textureHeight * request.scale_y);
    }
  
    // Should just take from Camera
    int halfWidth = windowWidth / 2;
    int halfHeight = windowHeight / 2;

    const int PIXELS_PER_UNIT = 100;
    // Then apply the camera adjustments after calculating the pivot
    SDL_Rect destRect = {
        static_cast<int>(halfWidth / CameraBounds::zoom_factor + (request.x - CameraBounds::cam_x_pos) * PIXELS_PER_UNIT - (pivotX)),
        static_cast<int>(halfHeight / CameraBounds::zoom_factor + (request.y - CameraBounds::cam_y_pos) * PIXELS_PER_UNIT - (pivotY)),
        static_cast<int>(textureWidth * fabs(request.scale_x)),
        static_cast<int>(textureHeight * fabs(request.scale_y))
    };

    // Set texture color modulation and alpha
    SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
    SDL_SetTextureAlphaMod(texture, request.a);

    // Rotation center: SDL_Point represents the point around which to rotate; 
    SDL_Point rotationCenter = { static_cast<int>(pivotX), static_cast<int>(pivotY) };

    // Render the texture with rotation and flip
//    SDL_RenderCopyEx(getRenderer(), texture, NULL, &destRect, request.rotation_degrees, &rotationCenter, SDL_FLIP_NONE);
    Helper::SDL_RenderCopyEx498(0, "", getRenderer(), texture, NULL, &destRect, request.rotation_degrees, &rotationCenter, SDL_FLIP_NONE);


    // Reset texture color modulation and alpha to default if necessary
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);

    SDL_RenderSetScale(renderer, 1, 1);
}

void Renderer::renderUI(const RenderRequest& request) {
    // Retrieve texture from image name
    SDL_Texture* texture = ImageDB::loadImage(request.image_name, getRenderer());
    if (!texture) return; // Texture not found, skip rendering

    // Calculate destination rectangle
    SDL_Rect destRect = {
        static_cast<int>(request.x),
        static_cast<int>(request.y),
        0, 0 // Width and height will be set after querying texture
    };

    // Query the texture to get its width and height
    SDL_QueryTexture(texture, NULL, NULL, &destRect.w, &destRect.h);

    // Set texture color modulation and alpha for DrawUIEx
    SDL_SetTextureColorMod(texture, request.r, request.g, request.b);
    SDL_SetTextureAlphaMod(texture, request.a);

    // Render the texture
    SDL_RenderCopy(getRenderer(), texture, NULL, &destRect);

    // Reset texture color modulation and alpha to default if necessary
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);
}


void Renderer::drawPixel(const RenderRequest& request) {
    // Set render draw color with the RGBA values provided
    SDL_SetRenderDrawColor(getRenderer(), request.r, request.g, request.b, request.a);

    // Ensure blend mode is set to blend to handle the alpha channel properly
    SDL_SetRenderDrawBlendMode(getRenderer(), SDL_BLENDMODE_BLEND);

    // Draw the point at the specified coordinates
    SDL_RenderDrawPoint(getRenderer(), request.x, request.y);

    // Reset the render draw color and blend mode to defaults
    SDL_SetRenderDrawColor(getRenderer(), 0, 0, 0, 255); // Example: resetting to opaque black
    SDL_SetRenderDrawBlendMode(getRenderer(), SDL_BLENDMODE_NONE);
}
