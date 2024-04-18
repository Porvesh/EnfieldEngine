#include "headers/MainHelper.h"
#include "headers/Scene.h"
#include "headers/RigidBody.h"
#include "headers/MyContactListener.h"
#include "headers/RayCasting.h"
#include "headers/Eventbus.h"


lua_State* LuaHelper::L;
// Definition for the static member variable
std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> LuaHelper::component_tables;

extern std::string gamePlaying;


LuaHelper::~LuaHelper() {
    closeLua();
}

void LuaHelper::closeLua() {
  //  lua_close(L);
}

LuaHelper::LuaHelper(){
    initialiseLuaState(); // Initialize Lua state with standard libraries
}

static void Debug_Log(const std::string& message) {
    std::cout << message << std::endl;
}

static void Debug_LogError(const std::string& message) {
    std::cerr << message << std::endl;
}

static void exposeDebugFunctions(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Debug")
        .addFunction("Log", &Debug_Log)
        .addFunction("LogError", &Debug_LogError)
        .endNamespace();
}

static void Application_Sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

static int Application_GetFrame() {
    return Helper::GetFrameNumber();
}

static void Application_Quit() {
        exit(0);
}

static void Application_OpenURL(const std::string& url) {
#if defined(_WIN32) // Windows
    std::string command = "start " + url;
#elif defined(__APPLE__) // macOS
    std::string command = "open " + url;
#else // Assuming Linux/Unix for others
    std::string command = "xdg-open " + url;
#endif

    std::system(command.c_str());
}

// Wrapper function to handle conversion to Lua table
luabridge::LuaRef FindAllActorsByNameLua(const std::string& name, lua_State* L) {
    auto foundActors = Actor::FindAll(name);
    luabridge::LuaRef table = luabridge::newTable(L);
    for (size_t i = 0; i < foundActors.size(); ++i) {
        table[i + 1] = foundActors[i];
    }
    return table;
}

void exposeActortoLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addStaticFunction("Find", &Actor::Find)
        .addStaticFunction("FindAll", &FindAllActorsByNameLua)
        .addFunction("AddComponent", &Actor::addComponentFromLua)
        .addFunction("RemoveComponent", &Actor::removeComponentFromLua)
        .addStaticFunction("Instantiate", &Actor::InstantiateNewActor)
        .addStaticFunction("Destroy", &Actor::DestoryActorFromLua)
        .endClass();
}

void exposeApplicationtoLua(lua_State* L) { 
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Application")
        .addFunction("Quit", &Application_Quit)
        .addFunction("Sleep", &Application_Sleep)
        .addFunction("GetFrame", &Application_GetFrame)
        .addFunction("OpenURL", &Application_OpenURL)
        .endNamespace();
}

void exposeInputtoLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<Input>("Input")
        .addStaticFunction("GetKey", &Input::GetKey)
        .addStaticFunction("GetKeyDown", &Input::GetKeyDown)
        .addStaticFunction("GetKeyUp", &Input::GetKeyUp)
        .addStaticFunction("GetMousePosition", &Input::GetMousePosition)
        .addStaticFunction("GetMouseButton", &Input::GetMouseButton)
        .addStaticFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
        .addStaticFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
        .addStaticFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .endClass()

        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();
}


void exposeTexttoLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<TextDB>("Text")
        .addStaticFunction("Draw", &TextDB::DrawText)
        .endClass();
}

void exportAudiotoLua(lua_State * L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<AudioDB>("Audio")
        .addStaticFunction("Play", &AudioDB::PlaySoundEffect)
        .addStaticFunction("Halt", &AudioDB::Halt)
        .addStaticFunction("SetVolume", &AudioDB::SetVolume)
        .endClass();
}



void exportImageToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<ImageDB>("Image")
        .addStaticFunction("DrawUI", &ImageDB::DrawUI)
        .addStaticFunction("DrawUIEx", &ImageDB::DrawUIEx)
        .addStaticFunction("Draw", &ImageDB::Draw)
        .addStaticFunction("DrawEx", &ImageDB::DrawEx)
        .addStaticFunction("DrawPixel", &ImageDB::DrawPixel)
        .endClass();
}

void exportCameraToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<CameraBounds>("Camera")
        .addStaticFunction("SetPosition", &CameraBounds::SetPosition)
        .addStaticFunction("GetPositionX", &CameraBounds::GetPositionX)
        .addStaticFunction("GetPositionY", &CameraBounds::GetPositionY)
        .addStaticFunction("SetZoom", &CameraBounds::SetZoom) // Assuming SetZoom is static
        .addStaticFunction("GetZoom", &CameraBounds::GetZoom)
        .endClass();
}

void exportSceneToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<Scene>("Scene")
        .addStaticFunction("Load", &Scene::loadNewScene)
        .addStaticFunction("GetCurrent", &Scene::GetCurrent)
        .addStaticFunction("DontDestroy", &Scene::DontDestroyOnLoad)
       .endClass();
}

void exportRigidBodyToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<RigidBody>("Rigidbody")
        .addData("x", &RigidBody::x) // Expose properties as needed
        .addData("y", &RigidBody::y)
        .addData("body_type", &RigidBody::body_type)
        .addData("precise", &RigidBody::precise)
        .addData("gravity_scale", &RigidBody::gravity_scale)
        .addData("density", &RigidBody::density)
        .addData("angular_friction", &RigidBody::angular_friction)
        .addData("rotation", &RigidBody::rotation)
        .addData("has_collider", &RigidBody::has_collider)
        .addData("has_trigger", &RigidBody::has_trigger)
        .addData("type", &RigidBody::type)
        .addData("key", &RigidBody::key)
        .addData("actor", &RigidBody::actor)
        .addData("enabled", &RigidBody::enabled)
        .addData("OnStartOver", &RigidBody::onStartOver)
        .addData("collider_type", &RigidBody::collider_type)
        .addData("width", &RigidBody::width)
        .addData("height", &RigidBody::height)
        .addData("radius", &RigidBody::radius)
        .addData("bounciness", &RigidBody::bounciness)
        .addData("friction", &RigidBody::friction)

        // Trigger Methods came 
        .addData("trigger_type", &RigidBody::trigger_type)
        .addData("trigger_width", &RigidBody::trigger_width)
        .addData("trigger_height", &RigidBody::trigger_height)
        .addData("trigger_radius", &RigidBody::trigger_radius)

        // Expose methods
        .addFunction("GetPosition", &RigidBody::GetPosition)
        .addFunction("GetRotation", &RigidBody::GetRotation)
        .addFunction("OnStart", &RigidBody::OnStart)

        .addFunction("AddForce", &RigidBody::AddForce)
        .addFunction("SetVelocity", &RigidBody::SetVelocity)
        .addFunction("SetPosition", &RigidBody::SetPosition)
        .addFunction("SetRotation", &RigidBody::SetRotation)
        .addFunction("SetAngularVelocity", &RigidBody::SetAngularVelocity)
        .addFunction("SetGravityScale", &RigidBody::SetGravityScale)
        .addFunction("SetUpDirection", &RigidBody::SetUpDirection) 
        .addFunction("SetRightDirection", &RigidBody::SetRightDirection) // Assuming implementation
        .addFunction("GetVelocity", &RigidBody::GetVelocity)
        .addFunction("GetAngularVelocity", &RigidBody::GetAngularVelocity)
        .addFunction("GetGravityScale", &RigidBody::GetGravityScale)
        .addFunction("GetUpDirection", &RigidBody::GetUpDirection)
        .addFunction("GetRightDirection", &RigidBody::GetRightDirection)
        .endClass();
}

void exportCollisionToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<b2Vec2>("b2Vec2")
        .addConstructor<void (*) ()>()
            .addData("x", &b2Vec2::x)
            .addData("y", &b2Vec2::y)
        .endClass()
        .beginClass<Collision>("Collision")
        .addConstructor<void (*) (void)>()
        .addProperty("other", &Collision::getOther, &Collision::setOther)
        .addProperty("point", &Collision::getPoint, &Collision::setPoint)
        .addProperty("relative_velocity", &Collision::getRelativeVelocity, &Collision::setRelativeVelocity)
        .addProperty("normal", &Collision::getNormal, &Collision::setNormal)
        .endClass();
}

void exportPhysicsToLua(lua_State* L) {

    luabridge::getGlobalNamespace(L)
        .beginClass<HitResult>("HitResult")
        .addConstructor<void (*)(Actor*, b2Vec2, b2Vec2, float, bool)>()
        .addData("actor", &HitResult::actor)
        .addData("point", &HitResult::point)
        .addData("normal", &HitResult::normal)
        .addData("fraction", &HitResult::fraction)
        .addData("is_trigger", &HitResult::is_trigger)
        .endClass()


        .beginClass<Physics>("Physics")
        .addStaticFunction("Raycast", &Physics::Raycast)
        .addStaticFunction("RaycastAll", &Physics::RaycastAll)
        .endClass();

}


void exportEventBusToLua(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<EventBus>("Event")
        .addConstructor<void (*) (void)>()
        .addStaticFunction("Publish", &EventBus::Publish)
        .addStaticFunction("Subscribe", &EventBus::Subscribe)
        .addStaticFunction("Unsubscribe", &EventBus::Unsubscribe)
        .endClass();
}

void LuaHelper::initialiseLuaState() {
    L = luaL_newstate();
    luaL_openlibs(L);
    exposeDebugFunctions(L); 
    exposeActortoLua(L);
    exposeApplicationtoLua(L);
    exposeInputtoLua(L);
    exposeTexttoLua(L);
    exportAudiotoLua(L);
    exportImageToLua(L);
    exportCameraToLua(L);
    exportSceneToLua(L);
    exportCollisionToLua(L);
    Vector2::Vector2ToLua(L);
    exportRigidBodyToLua(L);
    exportPhysicsToLua(L);
    exportEventBusToLua(L);
    loadAndCacheAllBaseTables();
}


void LuaHelper::loadAndCacheAllBaseTables() {

    const std::filesystem::path componentsDir("resources/" + gamePlaying + "/component_types");

    // Check if the directory exists
    if (!std::filesystem::exists(componentsDir) || !std::filesystem::is_directory(componentsDir)) {   
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(componentsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            // Convert the file path to a string and attempt to run the Lua file
            if (luaL_dofile(L, entry.path().string().c_str()) != LUA_OK) {
                std::cout << "problem with lua file " << entry.path().stem().string();
                exit(0);
            }
            else {
                std::string baseComponentName = entry.path().stem().string();
                luabridge::LuaRef tableRef = luabridge::getGlobal(L, baseComponentName.c_str());
                auto tableRefPtr = std::make_shared<luabridge::LuaRef>(tableRef);
                component_tables[baseComponentName] = tableRefPtr;
            }
        }
    }
}

void LuaHelper::EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef &parent_table) {
    luabridge::LuaRef metaTable = luabridge::newTable(L);
    metaTable["__index"] = parent_table;

    instance_table.push(L);
    metaTable.push(L);
    lua_setmetatable(L, -2);
    lua_pop(L, 1); // Pop the metatable from the stack
}

std::shared_ptr<luabridge::LuaRef> LuaHelper::getComponentInstance(const std::string& componentName, const std::string& key) {
    // Check if the component instance already exists.
    auto search = component_tables.find(componentName);
    std::shared_ptr<luabridge::LuaRef> newInstanceRef;

    if (search != component_tables.end()) {
        // If found, create a new instance that inherits from the existing instance.
        luabridge::LuaRef newInstance = luabridge::newTable(L);
        // Setting up inheritance from the existing instance.
        EstablishInheritance(newInstance, *(search->second));
        // Wrap the new instance in a shared_ptr.
        newInstanceRef = std::make_shared<luabridge::LuaRef>(newInstance);
    }
    else if(componentName == "Rigidbody") {
        RigidBody* temp = new RigidBody();
        luabridge::LuaRef RigidBodyRef(L, temp);
        RigidBody::InitializeWorld();
        newInstanceRef = std::make_shared<luabridge::LuaRef>(RigidBodyRef);
       // return newInstanceRef;
    }
    else {
        std::cout << "hows the base table not cached alraedy?" << std::endl;
    }

    (*newInstanceRef)["key"] = key;  // Set the component's name/key.
    
    // Always sent enabled to true
    if ((*newInstanceRef)["enabled"].isNil()) {
        (*newInstanceRef)["enabled"] = true;
    }

    (*newInstanceRef)["OnStartOver"] = false;
    (*newInstanceRef)["type"] = componentName;
   
    return newInstanceRef;
}
