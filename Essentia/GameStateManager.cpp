#include "pch.h"
#include "GameStateManager.h"

GameStateManager::GameStateManager()
{
}

void GameStateManager::LoadScene(const char* sceneFile)
{
    currentScene = LoadLevel(sceneFile);
    currentSceneName = sceneFile;
}

std::string_view GameStateManager::GetCurrentScene()
{
    return currentSceneName;
}

const bool GameStateManager::IsPlaying()
{
    return isPlaying;
}

void GameStateManager::SetIsPlaying(bool playing)
{
    isPlaying = playing;
}

void GameStateManager::UnloadScene()
{
    isPlaying = false;
    GContext->EntityManager->Reset();
}
