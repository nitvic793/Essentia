#pragma once
//Event System Reference: https://medium.com/@savas/nomad-game-engine-part-7-the-event-system-45a809ccb68f

#include "StringHash.h"
#include "Memory.h"
#include <unordered_map>
#include <vector>

namespace es
{
	struct IEvent {};

	class EventHandlerBase
	{
	public:
		inline void Execute(IEvent* event)
		{
			CallEvent(event);
		}
		virtual ~EventHandlerBase() {}
	protected:
		virtual void CallEvent(IEvent* event) = 0;
	};

	template<typename T, typename EventType>
	class EventHandler : public EventHandlerBase
	{
	public:
		typedef void (T::* MemberFunction)(EventType*);
		virtual void CallEvent(IEvent* event) override;
		EventHandler(T* instancePtr, MemberFunction function) :
			instance(instancePtr), eventFunction(function) {}
	private:
		T* instance;
		MemberFunction eventFunction;
	};


	class EventBus
	{
	public:
		template<typename EventType>
		void Publish(EventType* event);

		template<typename T, typename EventType>
		void Subscribe(T* instance, void (T::* memberFunction)(EventType*));
	private:
		std::unordered_map<std::string_view, std::vector<ScopedPtr<EventHandlerBase>>> subscribers;
	};

	template<typename EventType>
	inline void EventBus::Publish(EventType* event)
	{
		static constexpr auto typeName = TypeName<EventType>();
		std::vector<ScopedPtr<EventHandlerBase>>& handlers = subscribers[typeName];

		for (auto& handler : handlers)
		{
			handler->Execute(event);
		}
	}

	template<typename T, typename EventType>
	inline void EventBus::Subscribe(T* instance, void(T::* memberFunction)(EventType*))
	{
		static constexpr auto typeName = TypeName<EventType>();
		std::vector<ScopedPtr<EventHandlerBase>>& handlers = subscribers[typeName];
		void* buffer = GContext->DefaultAllocator->Alloc(sizeof(EventHandler<T, EventType>));
		EventHandler<T, EventType>* handler = new(buffer) EventHandler<T, EventType>(instance, memberFunction);
		ScopedPtr<EventHandlerBase> eventHandler = ScopedPtr<EventHandlerBase>((EventHandlerBase*)handler);
		handlers.push_back(std::move(eventHandler));
	}

	template<typename T, typename EventType>
	inline void EventHandler<T, EventType>::CallEvent(IEvent* event)
	{
		(instance->*eventFunction)(static_cast<EventType*>(event));
	}

	extern EventBus* GEventBus;
}