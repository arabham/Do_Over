#ifndef ASSETSTORE_H
#define ASSETSTORE_H

#include <string>
#include <map>
#include <SDL2/SDL.h>

class AssetStore
{
private:
    std::map<std::string, SDL_Texture*> textures;
    // TODO: std::map<std::string, TTF_Font* fonts
    // TODO: std::map<std::string, SDL_Audio* sounds

public:
    void ClearAssets();
    void AddTexture(const std::string& assetId, const std::string& filePath);
    SDL_Texture* GetTexture(const std::string& assetId);
};

#endif