#include "Game.h"

void Game::Setup()
{
	renderer = std::unique_ptr<Renderer>(new Renderer());
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	renderer->Initialize();
	Initialize();
	renderer->EndInitialization();

	auto windowSize = renderer->GetWindow()->GetWindowSize();
	camera = std::make_unique<Camera>((float)windowSize.Width, (float)windowSize.Height);
}

void Game::Run()
{
	auto window = renderer->GetWindow();
	window->StartMessagePump([&] 
		{
			camera->Update();
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
	FrameContext frameContext = { camera.get() };
	renderer->Clear();
	renderer->Render(frameContext);
	renderer->Present();
}
