
#include "headers/Scene.h"
#include "headers/RigidBody.h"

// Initialize region sizes with zero to begin with.
extern glm::vec2 REGION_SIZE_COLLISION;
extern glm::vec2 REGION_SIZE_TRIGGER;

std::string Scene::currentScene;
std::string Scene::nextScene;
bool Scene::loadRequested = false;

extern std::vector<std::shared_ptr<Actor>> hardcoded_actors;

std::queue<std::shared_ptr<Actor>> Scene::dontKillMe;

extern std::string gamePlaying;

extern void ReportError(const std::string& actor_name, const luabridge::LuaException& e);

void Scene::loadAndCheckInitialScene(const rapidjson::Document &config){
    // Check for initial_scene in game.config
    if (!config.HasMember("initial_scene") || !config["initial_scene"].IsString()) {
        std::cout << "error: initial_scene unspecified";
        std::exit(0);
    }

    std::string initialSceneName = config["initial_scene"].GetString();
    std::filesystem::path sceneFilePath = "resources/" + gamePlaying + "/scenes/" + initialSceneName + ".scene";

    // Check if the scene file exists
    if (!std::filesystem::exists(sceneFilePath)) {
         std::cout << "error: scene " << initialSceneName << " is missing";
         std::exit(0);
    }

}


void Scene::verifyComponentType(const std::string& type) {
    const std::filesystem::path resourcesPath{ "resources/" + gamePlaying + "/component_types/" + type + ".lua" };

    if (!std::filesystem::exists(resourcesPath)) {
        std::cout << "error: failed to locate component " + type;
        std::exit(0);
    }

}




void Scene::sparseSceneFile(const std::string& sceneFilePath, std::vector<std::shared_ptr<Actor>>& hardcoded_actors){
    
    rapidjson::Document document;
    EngineUtils::ReadJsonFile(sceneFilePath, document);
       
    if (document.HasParseError()) {
        std::cerr << "JSON parse error in scene file" << std::endl;
        exit(0);
    }

    const auto& actors = document["actors"];
      
    if (!actors.IsArray()) {
        std::cerr << "Actors field is not an array" << std::endl;
        return;
    }
        
    hardcoded_actors.reserve(actors.Size());
    std::shared_ptr<Actor> actorInstance;

    for (const auto& actor : actors.GetArray()) {
        std::string templateName = actor.HasMember("template") ? actor["template"].GetString() : "";
      
        if (!templateName.empty()) {
            Actor* templateRawInstance = TemplateDB::getTemplate(templateName);
            if (!templateRawInstance) {
                // Attempt to load the template if it's not found
                TemplateDB::loadTemplate(templateName);
                templateRawInstance = TemplateDB::getTemplate(templateName);
                if (!templateRawInstance) {
                    std::cout << "error: template " << templateName << " is missing";
                    std::exit(0);
                }
            }
            actorInstance = std::make_shared<Actor>(*templateRawInstance);
            actorInstance->deepCopyComponentsFrom(*templateRawInstance);
                 
        }


        std::string name = actor.HasMember("name") ? actor["name"].GetString() :
            (actorInstance ? actorInstance->actor_name : "");

        if (templateName.empty()) {
            actorInstance = std::make_shared<Actor>(name);
        }
        else {
            actorInstance->actor_name = name;
        }

        // Component Verification
        if (actor.HasMember("components") && actor["components"].IsObject()) {
            const auto& components = actor["components"].GetObject();
            for (const auto& component : components) {
                std::string key = component.name.GetString();

                // Check if the "type" exists within the component
                std::string componentName;
                if (component.value.HasMember("type") && component.value["type"].IsString()) {
                    componentName = component.value["type"].GetString();

                    if (componentName == "Rigidbody") {
                        RigidBody::InitializeWorld();
                    }
                    else {
                        verifyComponentType(componentName);
                    }
                    actorInstance->addComponent(key, componentName);
                }

                std::shared_ptr<luabridge::LuaRef> instanceRef = actorInstance->actor_components[key];


                // Assuming you want to iterate over properties within each comonent
                for (auto propertyIter = component.value.MemberBegin(); propertyIter != component.value.MemberEnd(); ++propertyIter) {
                     
                    std::string propertyName = propertyIter->name.GetString();
                    if (propertyIter->value.IsString()) {
                        std::string value = propertyIter->value.GetString();
                        (*instanceRef)[propertyName] = value;
                    }
                    else if (propertyIter->value.IsInt()) {
                        int value = propertyIter->value.GetInt();
                        (*instanceRef)[propertyName] = value;
                    }
                    else if (propertyIter->value.IsFloat()) {
                        float value = propertyIter->value.GetFloat();    
                        (*instanceRef)[propertyName] = value;
                    }
                    else if (propertyIter->value.IsBool()) {
                        bool value = propertyIter->value.GetBool();
                        (*instanceRef)[propertyName] = value;
                    }
                    
                }
                actorInstance->InjectConvenienceReferences(instanceRef);
            }
        }

  
        hardcoded_actors.push_back(actorInstance);

    }
   
}


bool Scene::isLoadScene(const std::string& sceneName) {
    std::filesystem::path sceneFilePath = "resources/" + gamePlaying + "scenes/" + sceneName + ".scene";

    //Check if the scene file exists
    if (!std::filesystem::exists(sceneFilePath)) {
        return false;
    }
    return true;
}


void Scene::loadNewScene(const std::string& sceneName) {
    nextScene = sceneName;
    loadRequested = true;
}


bool ActorExistsInQueue(const std::queue<std::shared_ptr<Actor>>& searchQueue, const std::shared_ptr<Actor>& targetActor) {
    std::queue<std::shared_ptr<Actor>> tempQueue = searchQueue; // Copy the queue to avoid modifying the original
    while (!tempQueue.empty()) {
        if (tempQueue.front()->GetID() == targetActor->GetID()) {
            return true;
        }
        tempQueue.pop();
    }
    return false;
}

// Must be called at the beginning of each frame
void Scene::Update(Renderer &renderer) {
    if (loadRequested) {
        currentScene = nextScene;
        hardcoded_actors.clear();


        auto it = hardcoded_actors.begin();
        while (it != hardcoded_actors.end()) {
            // Check if the current actor is in the dontKillMe queue
            bool shouldSurvive = ActorExistsInQueue(dontKillMe, *it);

                if (!shouldSurvive) {
                    // Call OnDestroy on all components of the actor
                    for (const auto& [key, componentRef] : (*it)->actor_components) {
                        try {
                            if ((*componentRef)["OnDestroy"].isFunction()) {
                                (*componentRef)["OnDestroy"](*componentRef);

                                // If it's a Rigidbody, call the Box2D world's DestroyBody method
                                if ((*componentRef)["type"].cast<std::string>() == "Rigidbody") {
                                    RigidBody* rigidbody = (*componentRef).cast<RigidBody*>();
                                    RigidBody::world->DestroyBody(rigidbody->m_body);
                                }
                            }
                        }
                        catch (const luabridge::LuaException& e) {
                            ReportError((*it)->actor_name, e);
                        }
                    }

                    // After calling OnDestroy, remove the actor from the vector
                    it = hardcoded_actors.erase(it);
                }
                else {
                    // If the actor is not to be destroyed, move to the next one
                    ++it;
                }
        }
        
        std::queue<std::shared_ptr<Actor>>().swap(dontKillMe);
        Scene::sparseSceneFile("resources/" + gamePlaying +  "/scenes/" + currentScene + ".scene", hardcoded_actors);
        CameraBounds::calculateCameraPositions(renderer.camera_offset_x, renderer.camera_offset_y);
       // renderer.RenderActors(CameraBounds::cam_x_pos, CameraBounds::cam_y_pos, CameraBounds::zoom_factor);
    }
}

std::string Scene::GetCurrent(){
    return Scene::currentScene;
}


std::vector<std::shared_ptr<Actor>> FindAll(const std::string& name) {
    std::vector<std::shared_ptr<Actor>> foundActors;
    for (const auto& actor : hardcoded_actors) {
        if (actor->GetName() == name) {
            foundActors.push_back(actor);
        }
    }

    for (const auto& actor : Actor::deferredActorAdditions) {
        if (actor->GetName() == name) {
            foundActors.push_back(actor);
        }
    }
    return foundActors;
}

void Scene::DontDestroyOnLoad(Actor* MainActor) {
    std::vector<std::shared_ptr<Actor>> actors = FindAll(MainActor->actor_name);

    for (const auto& actor : actors) {
        if (actor->id == MainActor->id) {
            dontKillMe.push(actor);
            return;
        }
    }
}