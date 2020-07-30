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

private:
	std::string currentScene;
	bool		isPlaying;
};