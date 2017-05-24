#pragma once

#include <vector>
#include "glm/common.hpp"

namespace Math {

	template<typename T>
	class BSpline
	{
	private:
		std::vector<T> m_points;

	public:
		BSpline();
		~BSpline();

		void push_back(T p);
		void insert(int idx, T p);
		void pop_back();
		void remove(int idx);

		void clear();
		void append(const std::vector<T>& points);

		int size();
	
		//return the position of the idx control point
		T getPoint(int idx) const;

		//return the position in the curve, betwwen the control points idx, idx + 1 and idx + 2, at position t (0 <= t < 1).
		T get(int idx, float t) const;

		//return the position in the curve, betwwen begin and end, at position t (0 <= t < 1).
		T get(float t) const;

	};

	//return the point in the spline, between the control points P0, P1 and P2, at position t
	template<typename T>
	T getBSplinePoint(const T& P0, const T& P1, const T& P2, float t);

}


namespace Math
{
	template<typename T>
	BSpline<T>::BSpline()
	{
	}

	template<typename T>
	BSpline<T>::~BSpline()
	{
	}

	template<typename T>
	void BSpline<T>::push_back(T p)
	{
		m_points.push_back(p);
	}

	template<typename T>
	void BSpline<T>::insert(int idx, T p)
	{
		m_points.insert(m_points.begin() + idx, p);
	}

	template<typename T>
	void BSpline<T>::pop_back()
	{
		m_points.pop_back();
	}

	template<typename T>
	void BSpline<T>::remove(int idx)
	{
		m_points.erase(m_points.begin() + idx);
	}

	template<typename T>
	void BSpline<T>::clear()
	{
		m_points.clear();
	}

	template<typename T>
	void BSpline<T>::append(const std::vector<T>& points)
	{
		for (auto& p : points)
			m_points.push_back(p);
	}

	template<typename T>
	int BSpline<T>::size()
	{
		return m_points.size();
	}

	template<typename T>
	T BSpline<T>::getPoint(int idx) const
	{
		assert(idx >= 0 && idx < m_points.size());

		return m_points[idx];
	}

	template<typename T>
	T BSpline<T>::get(int idx, float t) const
	{
		assert(idx >= 0 && idx + 2 < m_points.size());

		T P0 = m_points[idx];
		T = m_points[idx + 1];
		T = m_points[idx + 2];

		return (P0 + P1) + t*(-2.f * P0 + 2.f * P1) + t*t*(P0 - 2.f * P1 + P2);
	}

	template<typename T>
	T BSpline<T>::get(float t) const
	{
		int idx = (int)(t * m_points.size() - 0.999f);

		assert(idx >= 0 && idx + 2 < m_points.size());

		T P0 = m_points[idx];
		T P1 = m_points[idx + 1];
		T P2 = m_points[idx + 2];

		return (P0 + P1) + t*(-2.f * P0 + 2.f * P1) + t*t*(P0 - 2.f * P1 + P2);

	}

	template<typename T>
	T getBSplinePoint(const T& P0, const T& P1, const T& P2, float t)
	{
		return P0 + t*(-2.f * P0 + 2.f * P1) + t*t*(P0 - 2.f * P1 + P2);
	}

}


