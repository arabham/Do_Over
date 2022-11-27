#ifndef ASSETSTORE_H
#define ASSETSTORE_H

#include <map>
#include <string>
#include <SDL2/SDL.h>

class AssetStore
{
private:
    std::map<std::string, SDL_Texture*> textures;
    // TODO: std::map<std::string, TTF_Font* fonts
    // TODO: std::map<std::string, SDL_Audio* sounds

public:
    AssetStore();
    ~AssetStore();

    void ClearAssets();
    void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
    SDL_Texture* GetTexture(const std::string& assetId);
};

#endif