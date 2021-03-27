#include "pch.h"
#include "ScriptingSystem.h"
#include "WrenScriptingSystemImpl.h"
#include <EventTypes.h>
#include <Trace.h>
#include <Game.h>

constexpr const char* CBaseScriptsPath = "../../Assets/Scripts/";

ScriptingSystem::ScriptingSystem() :
	fileWatcher(MakeScopedArgs<FileWatcher>(CBaseScriptsPath, std::chrono::milliseconds(500)))
{
	auto game = GContext->GameInstance;
	watcherThread = std::thread([this, game]()
		{
			fileWatcher->start([game](auto file, auto status)
				{
					if (!std::filesystem::is_regular_file(std::filesystem::path(file)) && status != FileStatus::erased) {
						return;
					}

					switch (status) {
					case FileStatus::created:
					case FileStatus::modified:
						game->AddEventCallback([&]()
							{
								game->ReloadSystems();
							});
						break;
					default:
						std::cout << "Error! Unknown file status.\n";
					}
				});
		});
}

void ScriptingSystem::Initialize()
{
	systemImpl = Mem::Alloc<ScriptingSystemImpl>();
	systemImpl->InitializeAllocators();
	systemImpl->Initialize(CBaseScriptsPath);
	es::GEventBus->Subscribe(this, &ScriptingSystem::OnReload);
}

void ScriptingSystem::Update(float deltaTime, float totalTime)
{
	systemImpl->Update(deltaTime, totalTime);
}

void ScriptingSystem::Destroy()
{
	fileWatcher->stop();
	watcherThread.join();
	systemImpl->Destroy();
	Mem::Free(systemImpl);
}

void ScriptingSystem::OnReload(ReloadScriptSystemEvent* reloadEvent)
{
	es::Log("Reloading Scripts");
	es::Log("----------------");
	systemImpl->Destroy();
	systemImpl->Initialize(CBaseScriptsPath);
}
