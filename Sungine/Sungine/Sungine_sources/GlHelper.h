#pragma once

#include "FrameBuffer.h"

namespace GlHelper {

void transferDepth(const GlHelper::Framebuffer& from, GlHelper::Framebuffer& to, const glm::vec2& depthTextureSize);

glm::mat4 reflectedLookAt(glm::vec3& eye, glm::vec3& forward, const glm::vec3& upBeforeReflection, const glm::vec3& planePosition, const glm::vec3& planeNormal);

}