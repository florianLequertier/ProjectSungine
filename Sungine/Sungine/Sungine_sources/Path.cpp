

#include "Path.h"

Path::Path(int pathId): m_pathId(pathId), m_mesh(GL_LINE_STRIP, Mesh::Vbo_usage::USE_VERTICES, 3, GL_STREAM_DRAW)
{

	for (int i = 0; i < 10; i++)
	{
		m_mesh.vertices.push_back(0);
		m_mesh.vertices.push_back(0);
		m_mesh.vertices.push_back(0);
	}

	m_mesh.initGl();

}

Path::~Path()
{
}

std::vector<PathPoint*>::iterator Path::begin()
{
	return std::vector<PathPoint*>::iterator();
}

std::vector<PathPoint*>::iterator Path::end()
{
	return std::vector<PathPoint*>::iterator();
}

void Path::push_back(PathPoint * p)
{
	m_pathPoints.push_back(p);
}

void Path::insert(int idx, PathPoint * p)
{
	auto findIt = std::find_if(m_pathPoints.begin(), m_pathPoints.end(), [idx](const PathPoint* p) { return p->getPointIdx() > idx; });

	if (findIt == m_pathPoints.begin())
		m_pathPoints.insert(m_pathPoints.begin(), p);
	else if (findIt == m_pathPoints.end())
		m_pathPoints.push_back(p);
	else
		m_pathPoints.insert(findIt-1, p);
}

void Path::pop_back()
{
	m_pathPoints.pop_back();
}

void Path::erase(int idx)
{
	m_pathPoints.erase(m_pathPoints.begin() + idx);
}

void Path::erase(PathPoint* p)
{
	auto findIt = std::find(m_pathPoints.begin(), m_pathPoints.end(), p);
	if(findIt != m_pathPoints.end())
		m_pathPoints.erase(findIt);
}

void Path::clear()
{
	m_pathPoints.clear();
}

void Path::append(const std::vector<PathPoint*>& points)
{
	for (auto& p : points)
	{
		m_pathPoints.push_back(p);
	}
}

int Path::size()
{
	return m_pathPoints.size();
}

void Path::sort()
{
	std::sort(m_pathPoints.begin(), m_pathPoints.end(), [](const PathPoint* a, const PathPoint* b) { return a->getPointIdx() < b->getPointIdx(); });
}

PathPoint * Path::getPathPoint(int idx) const
{
	assert(idx >= 0 && idx < m_pathPoints.size());

	return m_pathPoints[idx];
}

glm::vec3 Path::get(float t) const
{
	if (m_pathPoints.size() < 2)
	{
		return m_pathPoints[0]->getPosition();
	}
	else
	{
		float T = 1.f / (m_pathPoints.size() - 1);

		int idx = t / T;

		if (idx == m_pathPoints.size() - 1)
			return m_pathPoints[m_pathPoints.size() - 1]->getPosition();

		glm::vec3 P0;
		glm::vec3 P1 = m_pathPoints[idx]->getPosition();
		glm::vec3 P2 = m_pathPoints[idx + 1]->getPosition();
		glm::vec3 P3;


		if (idx == 0)
			P0 = glm::normalize(P1 - P2) + P1;
		else
			P0 = m_pathPoints[idx - 1]->getPosition();

		if (idx + 2 >= m_pathPoints.size())
			P3 = glm::normalize(P2 - P1) + P1;
		else
			P3 = m_pathPoints[idx + 2]->getPosition();

		float t2 = ((t - (idx)*T ) / T);

		return Math::getCSplinePoint<glm::vec3>(P0, P1, P2, P3, t2);
	}
}


int Path::getPathId() const
{
	return m_pathId;
}

void Path::setPathId(int id)
{
	m_pathId = id;
}

void Path::updateVisual()
{
	if (m_pathPoints.size() == 0)
		return;

	m_mesh.vertices.clear();

	glm::vec3 begin = m_pathPoints[0]->getPosition();
	glm::vec3 end = m_pathPoints[m_pathPoints.size() - 1]->getPosition();

	//m_mesh.vertices.push_back(begin.x);
	//m_mesh.vertices.push_back(begin.y);
	//m_mesh.vertices.push_back(begin.z);

	for (int i = 0; i <= 10 * m_pathPoints.size(); i++)
	{
		float t = i / (float)(10.f * m_pathPoints.size());
		glm::vec3 vertex = get(t);
		m_mesh.vertices.push_back(vertex.x);
		m_mesh.vertices.push_back(vertex.y);
		m_mesh.vertices.push_back(vertex.z);
	}

	//m_mesh.vertices.push_back(end.x);
	//m_mesh.vertices.push_back(end.y);
	//m_mesh.vertices.push_back(end.z);

	m_mesh.updateVBO(Mesh::Vbo_types::VERTICES);
}

void Path::draw()
{
	m_mesh.draw();
}
