#pragma once
#include <core/types/types.hpp>
#include <core/ecs/ecsregistry.hpp>
#include <core/ecs/component_handle.hpp>

namespace args::core::ecs
{
	template<typename component_type>
	class rw_component_buffer
	{
	private:
		EcsRegistry* registry;

	public:
		rw_component_buffer(EcsRegistry* registry) : registry(registry) {}

		component_handle<component_type> operator[](id_type entityId)
		{
			return component_handle<component_type>(entityId, registry);
		}

		/**@todo throw exception instead of assert
		 */
		component_type read(id_type entityId, std::memory_order order = std::memory_order_acquire)
		{
			std::atomic<component_type>* comp = registry->getFamily<component_type>()->get_component(entityId);
			assert_msg("Component no longer exists.", comp);

			return comp->load(order);
		}

		/**@todo throw exception instead of assert
		 */
		void write(id_type entityId, component_type&& value, std::memory_order order = std::memory_order_release)
		{
			std::atomic<component_type>* comp = registry->getFamily<component_type>()->get_component(entityId);
			assert_msg("Component no longer exists.", comp);

			comp->store(value, order);

			return value;
		}

		/**@todo throw exception instead of assert
		 */
		void fetch_add(id_type entityId, component_type&& value,
			std::memory_order loadOrder = std::memory_order_acquire,
			std::memory_order successOrder = std::memory_order_release,
			std::memory_order failureOrder = std::memory_order_relaxed)
		{
			std::atomic<component_type>* comp = registry->getFamily<component_type>()->get_component(entityId);
			assert_msg("Component no longer exists.", comp);

			component_type oldVal = comp->load(loadOrder);
			component_type newVal = oldVal + value;

			while (!comp->compare_exchange_weak(oldVal, newVal, successOrder, failureOrder))
				newVal = oldVal + value;

			return newVal;
		}

		/**@todo throw exception instead of assert
		 */
		void fetch_multiply(id_type entityId, component_type&& value,
			std::memory_order loadOrder = std::memory_order_acquire,
			std::memory_order successOrder = std::memory_order_release,
			std::memory_order failureOrder = std::memory_order_relaxed)
		{
			std::atomic<component_type>* comp = registry->getFamily<component_type>()->get_component(entityId);
			assert_msg("Component no longer exists.", comp);

			component_type oldVal = comp->load(loadOrder);
			component_type newVal = oldVal * value;

			while (!comp->compare_exchange_weak(oldVal, newVal, successOrder, failureOrder))
				newVal = oldVal * value;

			return newVal;
		}

		void destroy(id_type entityId) { registry->getFamily<component_type>()->destroy_component(entityId); }
	};
}