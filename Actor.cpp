#include "headers/Actor.h"
#include "headers/MainHelper.h"
#include "headers/Template.h"
#include "headers/RigidBody.h"
// Define the static member outside the class
int Actor::next_id = 0;
int Actor::globalComponentCounter = 0;


extern std::vector<std::shared_ptr<Actor>> hardcoded_actors;

std::vector<std::shared_ptr<Actor>> Actor::deferredActorAdditions;
std::vector<std::pair<std::string, int>> Actor::deferredActorDeletions;

extern void ReportError(const std::string& actor_name, const luabridge::LuaException& e);


void Actor::addComponent(std::string key, std::string componentName) {
    
    if (actor_components.find(key) == actor_components.end()) {
        actor_components[key] = LuaHelper::getComponentInstance(componentName, key);

    }
    
}

void Actor::deepCopyComponentsFrom(const Actor& other) {
    id = next_id++;
    for (const auto& pair : other.actor_components) {
        if ((*pair.second)["type"].cast<std::string>() == "Rigidbody") {
            RigidBody& original = (*pair.second).cast<RigidBody&>();
            RigidBody *copy = new RigidBody(original);
            luabridge::LuaRef RigidBodyRef(LuaHelper::L, copy);
            actor_components[pair.first] = std::make_shared<luabridge::LuaRef>(RigidBodyRef);
           
        }
        else {
            luabridge::LuaRef newInstance = luabridge::newTable(LuaHelper::L);
            auto& originalInstance = *pair.second;
            LuaHelper::EstablishInheritance(newInstance, originalInstance);
            actor_components[pair.first] = std::make_shared<luabridge::LuaRef>(newInstance);
        }
    }
}

std::string Actor::GetName() const {
    return actor_name;
}

int Actor::GetID() const {
    return id;
}

void Actor::InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref) {
    (*component_ref)["actor"] = this;
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) const {
    auto it = actor_components.find(key);
    if (it != actor_components.end()) {
        return luabridge::LuaRef(*it->second);
    }
    return luabridge::LuaRef(LuaHelper::L);// Return null if not found
}

// Function to get the first component of a given type.
luabridge::LuaRef Actor::GetComponent(const std::string& typeName) const {
    for (const auto& component : actor_components) {
        if (component.second && (*component.second)["type"].cast<std::string>() == typeName) {
            return luabridge::LuaRef(*component.second.get());
        }
    }
    return luabridge::LuaRef(LuaHelper::L);
}

// Function to get all components of a given type.
luabridge::LuaRef Actor::GetComponents(const std::string& typeName, lua_State* L) const {
    luabridge::LuaRef table = luabridge::newTable(L);
    int index = 1;
    std::cout << actor_name << std::endl;
    for (const auto& component : actor_components) {
        if (component.second && (*component.second)["type"].cast<std::string>() == typeName) {
            table[index++] = luabridge::LuaRef(*component.second);
        }
    }
    return (table); // Return the table
}

luabridge::LuaRef Actor::Find(const std::string& name) {
    for (const auto& actor : hardcoded_actors) {
        if (actor->GetName() == name) {
            return luabridge::LuaRef(LuaHelper::L, actor.get());
        }
    }

    for (const auto& actor : deferredActorAdditions) {
        if (actor->GetName() == name) {
            return luabridge::LuaRef(LuaHelper::L, actor.get());
        }
    }


    return luabridge::LuaRef(LuaHelper::L);
}

std::vector<luabridge::LuaRef> Actor::FindAll(const std::string& name) {
    std::vector<luabridge::LuaRef> foundActors;
    for (const auto& actor : hardcoded_actors) {
        if (actor->GetName() == name) {
            foundActors.push_back(luabridge::LuaRef(LuaHelper::L, actor.get()));
        }
    }

    for (const auto& actor : deferredActorAdditions) {
        if (actor->GetName() == name) {
            foundActors.push_back(luabridge::LuaRef(LuaHelper::L, actor.get()));
        }
    }
    return foundActors;
}


luabridge::LuaRef Actor::addComponentFromLua(std::string componentName) {
    std::string key = "r" + std::to_string(globalComponentCounter++);
    std::shared_ptr<luabridge::LuaRef> componentRef = LuaHelper::getComponentInstance(componentName, key);
    (*componentRef)["enabled"] = false;
    actor_components.insert({ key, componentRef });
    deferredComponentAdditions.push_back(componentRef);
    InjectConvenienceReferences(componentRef);
    return *componentRef;
}

void Actor::ProcessDeferredComponentAdditions() {
    for (auto& componentRef : deferredComponentAdditions) {
        (*componentRef)["enabled"] = true;
    }
    // Clear the list of deferred additions after processing
    deferredComponentAdditions.clear();
}

void Actor::removeComponentFromLua(luabridge::LuaRef componentRef) {
    deferredComponentRemovals.push_back(componentRef);
}

void Actor::ProcessDeferredComponentRemovals() {
    for (auto& componentRef : deferredComponentRemovals) {
        std::string key = (componentRef)["key"].cast<std::string>();
        auto it = actor_components.find(key);

        if ((componentRef)["OnDestroy"].isFunction()) {
            (componentRef)["OnDestroy"](componentRef);
        }
        // Special handling for specific component types
        if ((componentRef)["type"].cast<std::string>() == "Rigidbody") {
            RigidBody* rigidbody = (componentRef).cast<RigidBody*>();
            if (rigidbody && rigidbody->m_body && RigidBody::world) {
                RigidBody::world->DestroyBody(rigidbody->m_body);
            }
        }

        // Remove the component from the actor's components container
        if (it != actor_components.end()) {
            actor_components.erase(it);
        }
    }
    // Clear the list of deferred removals after processing
    deferredComponentRemovals.clear();
}


luabridge::LuaRef Actor::InstantiateNewActor(std::string templateName) {
    std::shared_ptr<Actor> actorInstance;
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
    else {
        return luabridge::LuaRef(LuaHelper::L);
    }

    for (const auto& component : actorInstance->actor_components) {
        actorInstance->InjectConvenienceReferences(component.second);
    }
    deferredActorAdditions.push_back(actorInstance);

    return luabridge::LuaRef(LuaHelper::L, actorInstance.get());
}

void Actor::ProcessDeferredActorAdditions() {
    for (const auto &actor : deferredActorAdditions) {
        hardcoded_actors.push_back(actor);
    }
    deferredActorAdditions.clear();
}


void Actor::DestoryActorFromLua(Actor* DeleteMe) {
    std::pair<std::string, int> toDelete;
    toDelete.first = DeleteMe->actor_name;
    toDelete.second = DeleteMe->GetID();
    deferredActorDeletions.push_back(toDelete);
    DeleteMe->actor_name = "";

    for (auto& componentRef : DeleteMe->actor_components) {
        (*componentRef.second)["enabled"] = false;
    }
}

void Actor::ProcessActorDeletions() {

    for (const auto& deletionPair : deferredActorDeletions) {
        // Find the actor to delete
        auto it = std::find_if(hardcoded_actors.begin(), hardcoded_actors.end(),
            [&deletionPair](const std::shared_ptr<Actor>& actor) {
                return actor->GetID() == deletionPair.second;
            });

        if (it != hardcoded_actors.end()) {
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

            // After calling OnDestroy, erase the actor from the list
            hardcoded_actors.erase(it);
        }
    }

    deferredActorDeletions.clear();
}


void Actor::UpdateActors() {
    for (const auto& actor : hardcoded_actors) {
        actor->ProcessDeferredComponentAdditions();
        actor->ProcessDeferredComponentRemovals();

    }

    ProcessDeferredActorAdditions();
    ProcessActorDeletions();
}


void Actor::clearAll() {
    deferredActorAdditions.clear();
    deferredActorDeletions.clear();
}

