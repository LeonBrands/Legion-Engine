#include <rendering/pipeline/default/stages/meshbatchingstage.hpp>

namespace  legion::rendering
{
    void MeshBatchingStage::eraseInstance(ecs::entity_handle entity)
    {
        if (!m_instances.contains(entity))
            return;

        static id_type batchesId = nameHash("mesh batches");
        auto* batches = get_meta<sparse_map<material_handle, sparse_map<model_handle, std::unordered_set<ecs::entity_handle>>>>(batchesId);

        if (!batches)
            return;

        auto& [material, model] = m_instances[entity];

        (*batches)[material][model].erase(entity);
    }

    void MeshBatchingStage::insertInstance(ecs::entity_handle entity)
    {
        static id_type batchesId = nameHash("mesh batches");
        auto* batches = get_meta<sparse_map<material_handle, sparse_map<model_handle, std::unordered_set<ecs::entity_handle>>>>(batchesId);

        if (!batches)
            return;

        mesh_renderable renderable = entity.get_component_handles<mesh_renderable>();

        if (!renderable)
            return;

        if (renderable.get_material() == invalid_material_handle)
        {
            log::warn("Entity {} has an invalid material.", entity.get_id());
            return;
        }
        if (renderable.get_model() == invalid_model_handle)
        {
            log::warn("Entity {} has an invalid model.", entity.get_id());
            return;
        }

        auto material = renderable.get_material();
        auto model = renderable.get_model();

        m_instances[entity] = std::make_pair(material, model);
        (*batches)[material][model].insert(entity);
    }

    void MeshBatchingStage::reinsertInstance(ecs::entity_handle entity)
    {
        if (!m_instances.contains(entity))
            return;

        static id_type batchesId = nameHash("mesh batches");
        auto* batches = get_meta<sparse_map<material_handle, sparse_map<model_handle, std::unordered_set<ecs::entity_handle>>>>(batchesId);

        if (!batches)
            return;

        auto& [material, model] = m_instances[entity];

        (*batches)[material][model].erase(entity);

        mesh_renderable renderable = entity.get_component_handles<mesh_renderable>();

        if (!renderable)
            return;

        if (renderable.get_material() == invalid_material_handle)
        {
            log::warn("Entity {} has an invalid material.", entity.get_id());
            return;
        }
        if (renderable.get_model() == invalid_model_handle)
        {
            log::warn("Entity {} has an invalid model.", entity.get_id());
            return;
        }

        material = renderable.get_material();
        model = renderable.get_model();

        m_instances[entity] = std::make_pair(material, model);
        (*batches)[material][model].insert(entity);
    }

    void MeshBatchingStage::onRendererCreate(events::component_creation<mesh_renderer>* event)
    {
        insertInstance(event->entity);
    }

    void MeshBatchingStage::onRendererDestroy(events::component_destruction<mesh_renderer>* event)
    {
        eraseInstance(event->entity);
    }

    void MeshBatchingStage::onRendererModified(events::component_modification<mesh_renderer>* event)
    {
        reinsertInstance(event->entity);
    }

    void MeshBatchingStage::onFilterCreate(events::component_creation<mesh_filter>* event)
    {
        insertInstance(event->entity);
    }

    void MeshBatchingStage::onFilterDestroy(events::component_destruction<mesh_filter>* event)
    {
        eraseInstance(event->entity);
    }

    void MeshBatchingStage::onFilterModified(events::component_modification<mesh_filter>* event)
    {
        reinsertInstance(event->entity);
    }

    void MeshBatchingStage::setup(app::window& context)
    {
        create_meta<sparse_map<material_handle, sparse_map<model_handle, std::unordered_set<ecs::entity_handle>>>>("mesh batches");

        bindToEvent<events::component_creation<mesh_renderer>, &MeshBatchingStage::onRendererCreate>();
        bindToEvent<events::component_destruction<mesh_renderer>, &MeshBatchingStage::onRendererDestroy>();
        bindToEvent<events::component_modification<mesh_renderer>, &MeshBatchingStage::onRendererModified>();
        bindToEvent<events::component_creation<mesh_filter>, &MeshBatchingStage::onFilterCreate>();
        bindToEvent<events::component_destruction<mesh_filter>, &MeshBatchingStage::onFilterDestroy>();
        bindToEvent<events::component_modification<mesh_filter>, &MeshBatchingStage::onFilterModified>();

        static auto renderablesQuery = createQuery<mesh_filter, mesh_renderer>();
        renderablesQuery.queryEntities();

        for (auto ent : renderablesQuery)
            insertInstance(ent);
    }

    void MeshBatchingStage::render(app::window& context, camera& cam, const camera::camera_input& camInput, time::span deltaTime)
    {
        (void)deltaTime;
        (void)camInput;
        (void)cam;
        (void)context;
    }

    priority_type MeshBatchingStage::priority()
    {
        return setup_priority;
    }

}