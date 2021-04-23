#pragma once

#include "Serialization.h"

class GameStateManager
{
public:
	GameStateManager();
	void				LoadScene(const char* sceneFile);
	std::string_view	GetCurrentScene();
	const bool			IsPlaying();
	void				SetIsPlaying(bool playing);
	void				UnloadScene();

private:
	std::string currentSceneName;
	bool		isPlaying;
	Scene		currentScene;

};