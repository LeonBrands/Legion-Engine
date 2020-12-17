#include <rendering/pipeline/default/stages/submitstage.hpp>

using namespace legion::core::filesystem::literals;

namespace legion::rendering
{

    void SubmitStage::setup(app::window& context)
    {
        app::context_guard guard(context);
        float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
          // positions   // texCoords
          -1.0f,  1.0f,  0.0f, 1.0f,
          -1.0f, -1.0f,  0.0f, 0.0f,
           1.0f, -1.0f,  1.0f, 0.0f,

          -1.0f,  1.0f,  0.0f, 1.0f,
           1.0f, -1.0f,  1.0f, 0.0f,
           1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        m_screenShader = ShaderCache::create_shader("screen shader", "engine://shaders/screenshader.shs"_view);
    }

    void SubmitStage::render(app::window& context, camera& cam, const camera::camera_input& camInput, time::span deltaTime)
    {
        (void)deltaTime;
        (void)camInput;

        static id_type mainId = nameHash("main");

        auto fbo = getFramebuffer(mainId);
        if (!fbo)
        {
            log::error("Main frame buffer is missing.");
            abort();
            return;
        }

        app::context_guard guard(context);
        if (!guard.contextIsValid())
        {
            abort();
            return;
        }

        auto viewportSize = context.framebufferSize();

        auto colorAttachment = fbo->getAttachment(GL_COLOR_ATTACHMENT0);
        if (std::holds_alternative<std::monostate>(colorAttachment) || !std::holds_alternative<texture_handle>(colorAttachment))
        {
            log::error("Color attachment of main FBO was invalid.");
            return;
        }

        auto screenTexture = std::get<texture_handle>(colorAttachment);

        if (cam.renderTarget.id() != 0)
        {
            auto [success, message] = cam.renderTarget.verify();
            if (!success)
            {
                log::error(message);
                return;
            }
        }

        cam.renderTarget.bind();

        glViewport(0, 0, viewportSize.x, viewportSize.y);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        m_screenShader.bind();
        m_screenShader.get_uniform<texture_handle>("screenTexture").set_value(screenTexture);
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST);

        cam.renderTarget.release();
    }

    priority_type SubmitStage::priority()
    {
        return submit_priority;
    }
}