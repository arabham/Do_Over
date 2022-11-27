#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include <SDL2/SDL.h>
#include <algorithm>

class RenderSystem: public System
{
public:
    RenderSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    static bool comp(const Entity& a, const Entity& b)
    {
        if (a.GetComponent<SpriteComponent>().zIndex < b.GetComponent<SpriteComponent>().zIndex)
        {
            return true;
        }
        return false;
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore)
    {
        // TODO: Sort all the entities of our system by z-index
        std::vector<Entity> sortedEntities = GetSystemEntities();
        std::sort(sortedEntities.begin(), sortedEntities.end(), comp);

        // Loop all entities that the system is interested in
        for (auto entity: sortedEntities)
        {
            // Update entity position based on its velocity
            const auto transform = entity.GetComponent<TransformComponent>();
            const auto sprite = entity.GetComponent<SpriteComponent>();

            // Set the source rectangle of our orignal sprite texture
            SDL_Rect srcRect = sprite.srcRect;

            // Set the destination rectangle with the x,y position to be rendered
            SDL_Rect dstRect = 
            {
                static_cast<int>(transform.position.x),
                static_cast<int>(transform.position.y),
                static_cast<int>(sprite.width * transform.scale.x),
                static_cast<int>(sprite.height * transform.scale.y)
            };

            SDL_RenderCopyEx
            (
                renderer,
                assetStore->GetTexture(sprite.assetId),
                &srcRect,
                &dstRect,
                transform.rotation,
                NULL,
                SDL_FLIP_NONE
            );
        }

    }
};

#endif