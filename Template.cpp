
#include <unordered_map>
#include <string>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <filesystem>
#include <memory>
#include "headers/Template.h"
#include "headers/EngineUtils.h"
#include "headers/MainHelper.h"
#include "headers/Scene.h"
#include "headers/RigidBody.h"
#include <vector>

std::unordered_map<std::string, Actor*> TemplateDB::templates = {};
extern std::string gamePlaying;

Actor* TemplateDB::getTemplate(std::string& templateName){
    auto it = templates.find(templateName);
    if (it != templates.end()) {
        return (it->second);  // Return a copy of the template
    }
    return nullptr;
}

void TemplateDB::loadTemplate(std::string& templateName) {
    std::string path = "resources/" + gamePlaying + "/actor_templates/" + templateName + ".template";
    rapidjson::Document d;

      // WINDOWS
     const std::filesystem::path configFile{path};
     if (!std::filesystem::exists(configFile)) {
         std::cout << "error: template " <<templateName << " is missing";
         std::exit(0);
    }

    EngineUtils::ReadJsonFile(path, d); 

    if (d.HasParseError()) {
        std::cerr << "Failed to parse template: " << templateName << std::endl;
        return;
    }

    // Assuming 'd' is of type rapidjson::Document or rapidjson::Value containing actor details
    std::string name = d.HasMember("name") && d["name"].IsString() ? d["name"].GetString() : "";
   
    Actor* actorTemplate = new Actor(name);
   
    // Component Verification
    if (d.HasMember("components") && d["components"].IsObject()) {
        const auto& components = d["components"].GetObject();
        for (const auto& component : components) {
            std::string key = component.name.GetString();
            std::string componentName = component.value["type"].GetString();
            if (componentName == "Rigidbody") {
                RigidBody::InitializeWorld();
            }
            else {
                Scene::verifyComponentType(componentName);
            }
            actorTemplate->addComponent(key, componentName);
    
            std::shared_ptr<luabridge::LuaRef> instanceRef = actorTemplate->actor_components[key];
    
            // Assuming you want to iterate over properties within each component
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
        }
    }
    
    templates[templateName] = actorTemplate;
    
}