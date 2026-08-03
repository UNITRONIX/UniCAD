/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 *                                                                         *
 *   Depth-based SSAO + screen-space outline post-process for the main     *
 *   Coin3D viewport. Shader approach inspired by PathSimulator AppGL.     *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 ***************************************************************************/

#ifndef GUI_SSAOPOSTPROCESSOR_H
#define GUI_SSAOPOSTPROCESSOR_H

#include <memory>
#include <vector>

#include <FCGlobal.h>

class QOpenGLContext;
class QOpenGLFunctions_3_3_Core;

namespace Gui
{

/**
 * Captures the current color+depth buffers after Coin renders the scene,
 * then applies depth-based SSAO and optional Sobel outline, compositing
 * back to the default framebuffer.
 */
class GuiExport SsaoPostProcessor
{
public:
    SsaoPostProcessor();
    ~SsaoPostProcessor();

    SsaoPostProcessor(const SsaoPostProcessor&) = delete;
    SsaoPostProcessor& operator=(const SsaoPostProcessor&) = delete;

    /// Initialize GL resources; returns false if GL 3.3 is unavailable
    bool initialize();
    void release();
    bool isReady() const
    {
        return m_ready;
    }

    void setSSAOEnabled(bool on)
    {
        m_ssaoEnabled = on;
    }
    bool isSSAOEnabled() const
    {
        return m_ssaoEnabled;
    }

    void setOutlineEnabled(bool on)
    {
        m_outlineEnabled = on;
    }
    bool isOutlineEnabled() const
    {
        return m_outlineEnabled;
    }

    void setOutlineColor(float r, float g, float b)
    {
        m_outlineColor[0] = r;
        m_outlineColor[1] = g;
        m_outlineColor[2] = b;
    }

    void setRadius(float r)
    {
        m_radius = r;
    }
    void setOutlineThreshold(float t)
    {
        m_outlineThreshold = t;
    }

    /**
     * Apply post-process to the current backbuffer contents.
     * @param width  viewport width in pixels
     * @param height viewport height in pixels
     * @param proj   4x4 column-major projection matrix (Coin/OpenGL)
     * @param zNear  camera near plane
     * @param zFar   camera far plane
     */
    void apply(int width, int height, const float* proj, float zNear, float zFar);

private:
    bool compileShaders();
    void ensureSize(int width, int height);
    void createKernel();
    unsigned int compileShader(unsigned int type, const char* source);
    unsigned int linkProgram(unsigned int vs, unsigned int fs);

    QOpenGLFunctions_3_3_Core* m_gl = nullptr;
    bool m_ready = false;
    bool m_failed = false;
    bool m_ssaoEnabled = true;
    bool m_outlineEnabled = true;

    int m_width = 0;
    int m_height = 0;

    unsigned int m_quadVao = 0;
    unsigned int m_quadVbo = 0;

    unsigned int m_colorTex = 0;
    unsigned int m_depthTex = 0;
    unsigned int m_ssaoTex = 0;
    unsigned int m_ssaoBlurTex = 0;
    unsigned int m_noiseTex = 0;
    unsigned int m_captureFbo = 0;
    unsigned int m_ssaoFbo = 0;
    unsigned int m_blurFbo = 0;

    unsigned int m_progSsao = 0;
    unsigned int m_progBlur = 0;
    unsigned int m_progComposite = 0;

    std::vector<float> m_kernel;
    float m_radius = 0.35f;
    float m_outlineThreshold = 0.018f;
    float m_outlineColor[3] = {0.03f, 0.03f, 0.04f};
};

}  // namespace Gui

#endif  // GUI_SSAOPOSTPROCESSOR_H
