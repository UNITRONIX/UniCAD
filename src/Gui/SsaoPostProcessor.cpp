/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 *                                                                         *
 *   Depth-based SSAO + outline. Portions adapted from PathSimulator       *
 *   AppGL shaders (OpenGL 4 Shading Language Cookbook / MIT).             *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <algorithm>
#include <cmath>
#include <random>
#endif

#include "SsaoPostProcessor.h"

#include <Base/Console.h>

using namespace Gui;

namespace
{

const char* VertShaderFullscreen = R"(
#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
    texCoord = aTexCoord;
}
)";

const char* FragShaderDepthSSAO = R"(
#version 330 core
layout(location = 0) out float AoData;
in vec2 texCoord;

uniform sampler2D DepthTex;
uniform sampler2D NoiseTex;
uniform mat4 projection;
uniform mat4 invProjection;
uniform float zNear;
uniform float zFar;
uniform float radius;
uniform float screenWidth;
uniform float screenHeight;

const int kernelSize = 32;
uniform vec3 SampleKernel[kernelSize];

vec3 viewPosFromDepth(vec2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 view = invProjection * clip;
    return view.xyz / view.w;
}

void main()
{
    float depth = texture(DepthTex, texCoord).r;
    if (depth >= 0.9999) {
        AoData = 1.0;
        return;
    }

    vec3 fragPos = viewPosFromDepth(texCoord, depth);
    vec2 texel = 1.0 / vec2(screenWidth, screenHeight);
    vec3 posR = viewPosFromDepth(texCoord + vec2(texel.x, 0.0),
                                  texture(DepthTex, texCoord + vec2(texel.x, 0.0)).r);
    vec3 posU = viewPosFromDepth(texCoord + vec2(0.0, texel.y),
                                  texture(DepthTex, texCoord + vec2(0.0, texel.y)).r);
    vec3 normal = normalize(cross(posR - fragPos, posU - fragPos));

    vec2 noiseScale = vec2(screenWidth / 4.0, screenHeight / 4.0);
    vec3 randomVec = normalize(texture(NoiseTex, texCoord * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        vec3 samplePos = fragPos + tbn * SampleKernel[i] * radius;
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(DepthTex, offset.xy).r;
        vec3 sampleView = viewPosFromDepth(offset.xy, sampleDepth);
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleView.z));
        occlusion += (sampleView.z >= samplePos.z + 0.002 ? 1.0 : 0.0) * rangeCheck;
    }

    AoData = 1.0 - (occlusion / float(kernelSize));
}
)";

const char* FragShaderBlur = R"(
#version 330 core
layout(location = 0) out float AoData;
in vec2 texCoord;
uniform sampler2D SsaoTex;
uniform float screenWidth;
uniform float screenHeight;

void main()
{
    vec2 texel = 1.0 / vec2(screenWidth, screenHeight);
    float result = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            result += texture(SsaoTex, texCoord + vec2(float(x), float(y)) * texel).r;
        }
    }
    AoData = result / 9.0;
}
)";

const char* FragShaderComposite = R"(
#version 330 core
layout(location = 0) out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D ColorTex;
uniform sampler2D DepthTex;
uniform sampler2D AoTex;
uniform bool ssaoActive;
uniform bool outlineActive;
uniform vec3 outlineColor;
uniform float outlineThreshold;
uniform float screenWidth;
uniform float screenHeight;
uniform float zNear;
uniform float zFar;

float linearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    vec3 color = texture(ColorTex, texCoord).rgb;
    float ao = ssaoActive ? texture(AoTex, texCoord).r : 1.0;
    ao = mix(1.0, ao, 0.65);
    color *= ao;

    if (outlineActive) {
        vec2 texel = 1.0 / vec2(screenWidth, screenHeight);
        float d = linearizeDepth(texture(DepthTex, texCoord).r);
        // Wider neighborhood for thicker Shapr-like silhouettes
        float dL1 = linearizeDepth(texture(DepthTex, texCoord + vec2(-texel.x, 0.0)).r);
        float dR1 = linearizeDepth(texture(DepthTex, texCoord + vec2( texel.x, 0.0)).r);
        float dD1 = linearizeDepth(texture(DepthTex, texCoord + vec2(0.0, -texel.y)).r);
        float dU1 = linearizeDepth(texture(DepthTex, texCoord + vec2(0.0,  texel.y)).r);
        float dL2 = linearizeDepth(texture(DepthTex, texCoord + vec2(-2.0 * texel.x, 0.0)).r);
        float dR2 = linearizeDepth(texture(DepthTex, texCoord + vec2( 2.0 * texel.x, 0.0)).r);
        float dD2 = linearizeDepth(texture(DepthTex, texCoord + vec2(0.0, -2.0 * texel.y)).r);
        float dU2 = linearizeDepth(texture(DepthTex, texCoord + vec2(0.0,  2.0 * texel.y)).r);
        float edge = abs(dL1 - dR1) + abs(dD1 - dU1)
                   + 0.5 * (abs(dL2 - dR2) + abs(dD2 - dU2));
        float thresh = outlineThreshold * max(abs(d), 1.0);
        float line = smoothstep(thresh, thresh * 1.6, edge);
        color = mix(color, outlineColor, clamp(line * 1.15, 0.0, 1.0));
    }

    FragColor = vec4(color, 1.0);
}
)";

bool invertMatrix4(const float* mIn, float* mOut)
{
    float m[4][4];
    float inv[4][4];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            m[r][c] = mIn[c * 4 + r];  // column-major -> row
            inv[r][c] = (r == c) ? 1.f : 0.f;
        }
    }

    for (int col = 0; col < 4; ++col) {
        int pivot = col;
        float best = std::fabs(m[col][col]);
        for (int row = col + 1; row < 4; ++row) {
            float v = std::fabs(m[row][col]);
            if (v > best) {
                best = v;
                pivot = row;
            }
        }
        if (best < 1e-8f) {
            return false;
        }
        if (pivot != col) {
            for (int k = 0; k < 4; ++k) {
                std::swap(m[col][k], m[pivot][k]);
                std::swap(inv[col][k], inv[pivot][k]);
            }
        }
        float diag = m[col][col];
        for (int k = 0; k < 4; ++k) {
            m[col][k] /= diag;
            inv[col][k] /= diag;
        }
        for (int row = 0; row < 4; ++row) {
            if (row == col) {
                continue;
            }
            float f = m[row][col];
            for (int k = 0; k < 4; ++k) {
                m[row][k] -= f * m[col][k];
                inv[row][k] -= f * inv[col][k];
            }
        }
    }

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            mOut[c * 4 + r] = inv[r][c];  // back to column-major
        }
    }
    return true;
}

}  // namespace

SsaoPostProcessor::SsaoPostProcessor() = default;

SsaoPostProcessor::~SsaoPostProcessor()
{
    release();
}

bool SsaoPostProcessor::initialize()
{
    if (m_ready) {
        return true;
    }
    if (m_failed) {
        return false;
    }

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        Base::Console().warning("SSAO: no current OpenGL context\n");
        m_failed = true;
        return false;
    }

    m_gl = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(ctx);
    if (!m_gl) {
        Base::Console().warning(
            "SSAO: OpenGL 3.3 Core functions unavailable — post-process disabled\n"
        );
        m_failed = true;
        return false;
    }
    m_gl->initializeOpenGLFunctions();

    if (!compileShaders()) {
        m_failed = true;
        release();
        return false;
    }

    const float quad[] = {
        -1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 1.f,
        -1.f, -1.f, 0.f, 0.f, 1.f, 1.f,  1.f, 1.f, -1.f, 1.f, 0.f, 1.f,
    };
    m_gl->glGenVertexArrays(1, &m_quadVao);
    m_gl->glGenBuffers(1, &m_quadVbo);
    m_gl->glBindVertexArray(m_quadVao);
    m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    m_gl->glEnableVertexAttribArray(0);
    m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    m_gl->glEnableVertexAttribArray(1);
    m_gl->glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(2 * sizeof(float))
    );
    m_gl->glBindVertexArray(0);

    createKernel();

    std::vector<float> noise(16 * 3);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    for (int i = 0; i < 16; ++i) {
        noise[i * 3 + 0] = dist(rng);
        noise[i * 3 + 1] = dist(rng);
        noise[i * 3 + 2] = 0.f;
    }
    m_gl->glGenTextures(1, &m_noiseTex);
    m_gl->glBindTexture(GL_TEXTURE_2D, m_noiseTex);
    m_gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data());
    m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_ready = true;
    return true;
}

void SsaoPostProcessor::release()
{
    if (!m_gl) {
        m_ready = false;
        return;
    }

    auto delTex = [this](unsigned int& t) {
        if (t) {
            m_gl->glDeleteTextures(1, &t);
            t = 0;
        }
    };
    auto delFbo = [this](unsigned int& f) {
        if (f) {
            m_gl->glDeleteFramebuffers(1, &f);
            f = 0;
        }
    };
    auto delProg = [this](unsigned int& p) {
        if (p) {
            m_gl->glDeleteProgram(p);
            p = 0;
        }
    };

    delTex(m_colorTex);
    delTex(m_depthTex);
    delTex(m_ssaoTex);
    delTex(m_ssaoBlurTex);
    delTex(m_noiseTex);
    delFbo(m_captureFbo);
    delFbo(m_ssaoFbo);
    delFbo(m_blurFbo);
    delProg(m_progSsao);
    delProg(m_progBlur);
    delProg(m_progComposite);

    if (m_quadVbo) {
        m_gl->glDeleteBuffers(1, &m_quadVbo);
        m_quadVbo = 0;
    }
    if (m_quadVao) {
        m_gl->glDeleteVertexArrays(1, &m_quadVao);
        m_quadVao = 0;
    }

    m_width = m_height = 0;
    m_ready = false;
    m_gl = nullptr;
}

unsigned int SsaoPostProcessor::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = m_gl->glCreateShader(type);
    m_gl->glShaderSource(shader, 1, &source, nullptr);
    m_gl->glCompileShader(shader);
    int ok = 0;
    m_gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        m_gl->glGetShaderInfoLog(shader, 1024, nullptr, log);
        Base::Console().error("SSAO shader compile error: %s\n", log);
        m_gl->glDeleteShader(shader);
        return 0;
    }
    return shader;
}

unsigned int SsaoPostProcessor::linkProgram(unsigned int vs, unsigned int fs)
{
    unsigned int prog = m_gl->glCreateProgram();
    m_gl->glAttachShader(prog, vs);
    m_gl->glAttachShader(prog, fs);
    m_gl->glLinkProgram(prog);
    int ok = 0;
    m_gl->glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        m_gl->glGetProgramInfoLog(prog, 1024, nullptr, log);
        Base::Console().error("SSAO program link error: %s\n", log);
        m_gl->glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

bool SsaoPostProcessor::compileShaders()
{
    unsigned int vs = compileShader(GL_VERTEX_SHADER, VertShaderFullscreen);
    if (!vs) {
        return false;
    }

    unsigned int fsSsao = compileShader(GL_FRAGMENT_SHADER, FragShaderDepthSSAO);
    unsigned int fsBlur = compileShader(GL_FRAGMENT_SHADER, FragShaderBlur);
    unsigned int fsComp = compileShader(GL_FRAGMENT_SHADER, FragShaderComposite);
    if (!fsSsao || !fsBlur || !fsComp) {
        m_gl->glDeleteShader(vs);
        if (fsSsao) {
            m_gl->glDeleteShader(fsSsao);
        }
        if (fsBlur) {
            m_gl->glDeleteShader(fsBlur);
        }
        if (fsComp) {
            m_gl->glDeleteShader(fsComp);
        }
        return false;
    }

    m_progSsao = linkProgram(vs, fsSsao);
    m_progBlur = linkProgram(vs, fsBlur);
    m_progComposite = linkProgram(vs, fsComp);

    m_gl->glDeleteShader(vs);
    m_gl->glDeleteShader(fsSsao);
    m_gl->glDeleteShader(fsBlur);
    m_gl->glDeleteShader(fsComp);

    return m_progSsao && m_progBlur && m_progComposite;
}

void SsaoPostProcessor::createKernel()
{
    m_kernel.clear();
    m_kernel.reserve(32 * 3);
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(-1.f, 1.f);
    std::uniform_real_distribution<float> dist01(0.f, 1.f);

    for (int i = 0; i < 32; ++i) {
        float x = dist(rng);
        float y = dist(rng);
        float z = dist01(rng);
        float len = std::sqrt(x * x + y * y + z * z);
        if (len < 1e-5f) {
            len = 1.f;
        }
        x /= len;
        y /= len;
        z /= len;
        float scale = static_cast<float>(i) / 32.f;
        scale = 0.1f + 0.9f * scale * scale;
        m_kernel.push_back(x * scale);
        m_kernel.push_back(y * scale);
        m_kernel.push_back(z * scale);
    }
}

void SsaoPostProcessor::ensureSize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    if (width == m_width && height == m_height && m_colorTex) {
        return;
    }

    m_width = width;
    m_height = height;

    auto remakeTex = [this](unsigned int& tex, int internalFmt, int fmt, int type) {
        if (tex) {
            m_gl->glDeleteTextures(1, &tex);
        }
        m_gl->glGenTextures(1, &tex);
        m_gl->glBindTexture(GL_TEXTURE_2D, tex);
        m_gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, m_width, m_height, 0, fmt, type, nullptr);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    };

    remakeTex(m_colorTex, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    remakeTex(m_depthTex, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
    remakeTex(m_ssaoTex, GL_R16F, GL_RED, GL_FLOAT);
    remakeTex(m_ssaoBlurTex, GL_R16F, GL_RED, GL_FLOAT);

    if (m_captureFbo) {
        m_gl->glDeleteFramebuffers(1, &m_captureFbo);
    }
    m_gl->glGenFramebuffers(1, &m_captureFbo);
    m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_captureFbo);
    m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);
    m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

    if (m_ssaoFbo) {
        m_gl->glDeleteFramebuffers(1, &m_ssaoFbo);
    }
    m_gl->glGenFramebuffers(1, &m_ssaoFbo);
    m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFbo);
    m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTex, 0);

    if (m_blurFbo) {
        m_gl->glDeleteFramebuffers(1, &m_blurFbo);
    }
    m_gl->glGenFramebuffers(1, &m_blurFbo);
    m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_blurFbo);
    m_gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurTex, 0);

    m_gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SsaoPostProcessor::apply(int width, int height, const float* proj, float zNear, float zFar)
{
    if ((!m_ssaoEnabled && !m_outlineEnabled) || width <= 0 || height <= 0 || !proj) {
        return;
    }
    if (!m_ready && !initialize()) {
        return;
    }

    GLint drawFbo = 0;
    m_gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);

    ensureSize(width, height);

    // Resolve / copy color+depth from the widget FBO (handles MSAA via blit)
    m_gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
    m_gl->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_captureFbo);
    m_gl->glBlitFramebuffer(
        0,
        0,
        width,
        height,
        0,
        0,
        width,
        height,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );

    float invProj[16];
    if (!invertMatrix4(proj, invProj)) {
        for (int i = 0; i < 16; ++i) {
            invProj[i] = (i % 5 == 0) ? 1.f : 0.f;
        }
    }

    m_gl->glDisable(GL_DEPTH_TEST);
    m_gl->glDisable(GL_BLEND);
    m_gl->glDisable(GL_CULL_FACE);

    auto drawQuad = [this]() {
        m_gl->glBindVertexArray(m_quadVao);
        m_gl->glDrawArrays(GL_TRIANGLES, 0, 6);
        m_gl->glBindVertexArray(0);
    };

    if (m_ssaoEnabled) {
        m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFbo);
        m_gl->glViewport(0, 0, width, height);
        m_gl->glUseProgram(m_progSsao);
        m_gl->glActiveTexture(GL_TEXTURE0);
        m_gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
        m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progSsao, "DepthTex"), 0);
        m_gl->glActiveTexture(GL_TEXTURE1);
        m_gl->glBindTexture(GL_TEXTURE_2D, m_noiseTex);
        m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progSsao, "NoiseTex"), 1);
        m_gl->glUniformMatrix4fv(m_gl->glGetUniformLocation(m_progSsao, "projection"), 1, GL_FALSE, proj);
        m_gl->glUniformMatrix4fv(
            m_gl->glGetUniformLocation(m_progSsao, "invProjection"),
            1,
            GL_FALSE,
            invProj
        );
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progSsao, "zNear"), zNear);
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progSsao, "zFar"), zFar);
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progSsao, "radius"), m_radius);
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progSsao, "screenWidth"), float(width));
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progSsao, "screenHeight"), float(height));
        m_gl->glUniform3fv(
            m_gl->glGetUniformLocation(m_progSsao, "SampleKernel"),
            32,
            m_kernel.data()
        );
        drawQuad();

        m_gl->glBindFramebuffer(GL_FRAMEBUFFER, m_blurFbo);
        m_gl->glUseProgram(m_progBlur);
        m_gl->glActiveTexture(GL_TEXTURE0);
        m_gl->glBindTexture(GL_TEXTURE_2D, m_ssaoTex);
        m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progBlur, "SsaoTex"), 0);
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progBlur, "screenWidth"), float(width));
        m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progBlur, "screenHeight"), float(height));
        drawQuad();
    }

    // Composite back onto the original draw FBO (QOpenGLWidget)
    m_gl->glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
    m_gl->glViewport(0, 0, width, height);
    m_gl->glUseProgram(m_progComposite);
    m_gl->glActiveTexture(GL_TEXTURE0);
    m_gl->glBindTexture(GL_TEXTURE_2D, m_colorTex);
    m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progComposite, "ColorTex"), 0);
    m_gl->glActiveTexture(GL_TEXTURE1);
    m_gl->glBindTexture(GL_TEXTURE_2D, m_depthTex);
    m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progComposite, "DepthTex"), 1);
    m_gl->glActiveTexture(GL_TEXTURE2);
    m_gl->glBindTexture(GL_TEXTURE_2D, m_ssaoBlurTex);
    m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progComposite, "AoTex"), 2);
    m_gl->glUniform1i(m_gl->glGetUniformLocation(m_progComposite, "ssaoActive"), m_ssaoEnabled ? 1 : 0);
    m_gl->glUniform1i(
        m_gl->glGetUniformLocation(m_progComposite, "outlineActive"),
        m_outlineEnabled ? 1 : 0
    );
    m_gl->glUniform3fv(m_gl->glGetUniformLocation(m_progComposite, "outlineColor"), 1, m_outlineColor);
    m_gl->glUniform1f(
        m_gl->glGetUniformLocation(m_progComposite, "outlineThreshold"),
        m_outlineThreshold
    );
    m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progComposite, "screenWidth"), float(width));
    m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progComposite, "screenHeight"), float(height));
    m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progComposite, "zNear"), zNear);
    m_gl->glUniform1f(m_gl->glGetUniformLocation(m_progComposite, "zFar"), zFar);
    drawQuad();

    m_gl->glUseProgram(0);
    m_gl->glActiveTexture(GL_TEXTURE0);
    m_gl->glEnable(GL_DEPTH_TEST);
}
