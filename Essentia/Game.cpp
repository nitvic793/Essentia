#include "Game.h"
#include "ModelLoader.h"

void Game::Setup()
{
	renderer = std::unique_ptr<Renderer>(new Renderer());
	keyboard = std::make_unique<DirectX::Keyboard>();
	mouse = std::make_unique<DirectX::Mouse>();

	renderer->Initialize();
	auto meshMgr = renderer->GetMeshManager();
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
			auto kb = keyboard->GetState();
			camera->Update();
			Update();
			Render();
			if (kb.IsKeyDown(DirectX::Keyboard::Escape))
			{
				PostQuitMessage(0);
			}
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
