#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

const unsigned int MAX_COMPONENTS = 32;

/////////////////////////////////////////////////////////////////////////////////////////////
// Signature
/////////////////////////////////////////////////////////////////////////////////////////////
// We use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
protected:
    static int nextId;
};

// Used to assign a unique id to a component type
template <typename TComponent>
class Component : public IComponent
{
    // Returns the unique id of Component<T>
    static int GetId()
    {
        static auto id = nextId++;
        return id;
    }
};

class Entity
{
public:
    Entity(int id) : id(id){};
    Entity(const Entity &entity) = default;
    int GetId() const;

    Entity &operator=(const Entity &other) = default;
    bool operator==(const Entity &other) const { return id == other.id; }
    bool operator!=(const Entity &other) const { return id != other.id; }
    bool operator>(const Entity &other) const { return id > other.id; }
    bool operator<(const Entity &other) const { return id < other.id; }

private:
    int id;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// System
/////////////////////////////////////////////////////////////////////////////////////////////
// The system processes entities that contain a specific signature
/////////////////////////////////////////////////////////////////////////////////////////////
class System
{
public:
    System() = default;
    ~System() = default;

    void AddEntityToSystem(Entity entity);
    void RemoveEntityFromSystem(Entity entity);
    std::vector<Entity> GetSystemEntities() const;
    const Signature &GetComponentSignature() const;

    // Defines the component type that entities must have to be considered by the system
    template <typename TComponent>
    void RequireComponent();

private:
    Signature componentSignature;
    std::vector<Entity> entities;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Pool
/////////////////////////////////////////////////////////////////////////////////////////////
// A pool is just a vector (contiguous data) of objects of type T
/////////////////////////////////////////////////////////////////////////////////////////////
class IPool
{
public:
    virtual ~IPool() {}
};

template <typename T>
class Pool : public IPool
{
public:
    Pool(int size = 100)
    {
        data.reserve(size);
    }
    virtual ~Pool() = default;

    bool isEmpty() const
    {
        return data.empty();
    }

    int GetSize() const
    {
        return data.size();
    }

    void Resize(int n)
    {
        data.resize(n);
    }

    void Clear()
    {
        data.clear();
    }

    void Add(T object)
    {
        data.push_back(object);
    }

    void Set(int index, T object)
    {
        data[index] = object;
    }

    T &Get(int index)
    {
        return static_cast<T &>(data[index]);
    }

    T &operator[](unsigned int index)
    {
        return data[index];
    }

private:
    std::vector<T> data;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////////////////////////////
// The Registry manages the creation and destruction of entities, as well as
// adding systems and adding components to entities.
/////////////////////////////////////////////////////////////////////////////////////////////
class Registry
{
public:
    Registry() = default;

    void Update();

    // Management of entities, systems, and components
    Entity CreateEntity();

    // Function template to add a component of type T to a given entity
    template<typename T, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);


    void AddEntityToSystem(Entity entity);
    void KillEntity(Entity entity);
    void AddSystem();
    void RemoveComponent();

private:
    // Keep track of how many entities were added to the scene
    int numEntities = 0;

    // Vector of component pools.
    // each pool contains all the data for a certain component type
    // [vector index = componentId], [pool index = entityId]
    std::vector<IPool *> componentPools;

    // Vector of component signatures.
    // The signature lets us know which components are turned "on" for an entity
    // [vector index = entity id]
    std::vector<Signature> entityComponentSignatures;

    // Map of active systems [index = system typeid]
    std::unordered_map<std::type_index, System*> systems;

    std::set<Entity> entitiesToBeAdded; // Entities awaiting creation in the next Registry Update()
    std::set<Entity> entitiesToBeKilled; // Entities awaiting destruction in the next Registry Update()
};

template <typename TComponent>
void System::RequireComponent()
{
    const auto compontentId = Component<TComponent>::GetId();
    componentSignature.set(compontentId);
}

template <typename T, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
    const auto componentId = Component<T>::GetId();
    const auto entityId = entity.GetId();

    // If the component id is greater than the current size of the componentPools, the resize the vector
    if (componentId >= componentPools.size())
    {
        componentPools.resize(componentId + 1, nullptr);
    }

    // If we still don't have a Pool for that component type
    if (!componentPools[componentId])
    {
        Pool<T>* newComponentPool = new Pool<T>();
        componentPools[componentId] = newComponentPool;
    }

    // Get the pool of component values for that component type
    Pool<T>* componentPool = Pool<T>(componentPools[componentId]);

    // If the entity id is greater than the current size of the component pool, then resize the pool
    if (entityId >= componentPool->GetSize())
    {
        componentPool->Resize(numEntities);
    }

    // Create a new Component object of the type T, and forward the various parameters to the constructor
    T newComponent(std::forward<TArgs>(args)...);

    // Add the new component to the component pool list, using the entity id as index
    componentPool->Set(entityId, newComponent);

    // Finally, change the component signature of the entity and set the component id on the bitset to 1
    entityComponentSignatures[entityId].set(componentId);
}

#endif