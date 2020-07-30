#include "pch.h"
#include "GameStateManager.h"

GameStateManager::GameStateManager()
{
}

void GameStateManager::LoadScene(const char* sceneFile)
{
    LoadLevel(sceneFile);
    currentScene = sceneFile;
}

std::string_view GameStateManager::GetCurrentScene()
{
    return currentScene;
}

const bool GameStateManager::IsPlaying()
{
    return isPlaying;
}

void GameStateManager::SetIsPlaying(bool playing)
{
    isPlaying = playing;
}
