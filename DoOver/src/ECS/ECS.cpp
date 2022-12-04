#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

int IComponent::nextId = 0;

int Entity::GetId() const
{
    return id;
}

void Entity::Kill()
{
    registry->KillEntity(*this);
}

void System::AddEntityToSystem(Entity entity)
{
    entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity)
{
    entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
        return entity == other; 
    }), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const
{
    return entities;
}

const Signature &System::GetComponentSignature() const
{
    return componentSignature;
}

Entity Registry::CreateEntity()
{
    int entityId;
    if (freeIds.empty())
    {
        entityId = numEntities++;
        if (entityId >= static_cast<int>(entityComponentSignatures.size()))
        {
            entityComponentSignatures.resize(entityId + 1);
        }
    }
    else
    {
        // Reuse an id from the list of previously removed entities
        entityId = freeIds.front();
        freeIds.pop_front();
    }

    Entity entity(entityId);
    entity.registry = this;
    entitiesToBeAdded.insert(entity);

    Logger::Log("Entity created with id = " + std::to_string(entityId));
    
    return entity;
}

void Registry::KillEntity(Entity entity)
{
    entitiesToBeKilled.insert(entity);

    Logger::Log("Entity " + std::to_string(entity.GetId()) + " was killed");
}

void Registry::AddEntityToSystems(Entity entity)
{
    const auto entityId = entity.GetId();

    const auto& entityComponentSignature = entityComponentSignatures[entityId];

    for (auto& system: systems)
    {
        const auto& systemComponentSignature = system.second->GetComponentSignature();

        bool isInterested = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;
        if(isInterested)
        {
            system.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity)
{
    for (auto system: systems)
    {
        system.second->RemoveEntityFromSystem(entity);
    }
}

void Registry::Update()
{
    // Processing the entities that are waiting to be created to the active Systems
    for (auto entity: entitiesToBeAdded)
    {
        AddEntityToSystems(entity);
    }
    entitiesToBeAdded.clear();

    // Processing the entities that are waiting to be killed from the active Systems
    for (auto entity: entitiesToBeKilled)
    {
        RemoveEntityFromSystems(entity);
        entityComponentSignatures[entity.GetId()].reset();

        // Make the entity id available to be reused
        freeIds.push_back(entity.GetId());
    }
    entitiesToBeKilled.clear();
}
