#include "GlHelper.h"


namespace GlHelper {

void transferDepth(const GlHelper::Framebuffer& from, GlHelper::Framebuffer& to, const glm::vec2& depthTextureSize)
{
	from.bind(GL_READ_FRAMEBUFFER);
	to.bind(GL_DRAW_FRAMEBUFFER);
	glBlitFramebuffer(0, 0, depthTextureSize.x, depthTextureSize.y, 0, 0, depthTextureSize.x, depthTextureSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


glm::mat4 reflectedLookAt(glm::vec3& eye, glm::vec3& forward, const glm::vec3& upBeforeReflection, const glm::vec3& planePosition, const glm::vec3& planeNormal)
{
	eye = eye - 2.f * glm::dot((eye - planePosition), planeNormal) * planeNormal;
	glm::vec3 reflectedUp = glm::reflect(upBeforeReflection, planeNormal);
	forward = glm::reflect(glm::normalize(forward), planeNormal);

	const  glm::vec3 f(forward);
	const glm::vec3 s(normalize(cross(f, reflectedUp)));
	const glm::vec3 u(cross(s, f));

	glm::mat4 Result(1);
	Result[0][0] = -s.x;
	Result[1][0] = -s.y;
	Result[2][0] = -s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;
	Result[3][0] = -dot(-s, eye);
	Result[3][1] = -dot(u, eye);
	Result[3][2] = dot(f, eye);
	return Result;
}


}