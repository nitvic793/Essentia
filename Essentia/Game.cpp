#include "Game.h"

void Game::Setup()
{
	renderer = std::unique_ptr<Renderer>(new Renderer());
	renderer->Initialize();
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();
	Initialize();
}

void Game::Run()
{
	auto window = renderer->GetWindow();
	window->StartMessagePump([&] 
		{
			Update();
			Render();
		});
}

Game::~Game()
{
	renderer->CleanUp();
}

void Game::Render()
{
	renderer->Clear();
	renderer->Render();
	renderer->Present();
}
