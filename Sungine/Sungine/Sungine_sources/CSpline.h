#pragma once

#include <vector>
#include "glm/common.hpp"

#include "jsoncpp/json/json.h"

namespace Math {

	template<typename T>
	class CSpline : public ISerializable
	{
	private:
		std::vector<T> m_points;

	public:
		CSpline();
		~CSpline();

		void push_back(T p);
		void insert(int idx, T p);
		void pop_back();
		void remove(int idx);
		void resize(int _size);

		void clear();
		void append(const std::vector<T>& points);

		int size();

		//return the position of the idx control point
		T getPoint(int idx) const;

		//return the position in the curve, betwwen begin and end, at position t (0 <= t < 1).
		T get(float t) const;

		T& operator[](int index);

		virtual void save(Json::Value& objectRoot) const override;
		virtual void load(const Json::Value& objectRoot) override;

	};

	//return the point in the spline, between the control points P0, P1 and P2, at position t
	template<typename T>
	T getCSplinePoint(const T& P0, const T& P1, const T& P2, const T& P3, float t);

}


namespace Math
{
	template<typename T>
	CSpline<T>::CSpline()
	{
	}

	template<typename T>
	CSpline<T>::~CSpline()
	{
	}

	template<typename T>
	void CSpline<T>::push_back(T p)
	{
		m_points.push_back(p);
	}

	template<typename T>
	void CSpline<T>::insert(int idx, T p)
	{
		m_points.insert(m_points.begin() + idx, p);
	}

	template<typename T>
	void CSpline<T>::pop_back()
	{
		m_points.pop_back();
	}

	template<typename T>
	void CSpline<T>::remove(int idx)
	{
		m_points.erase(m_points.begin() + idx);
	}

	template<typename T>
	inline void CSpline<T>::resize(int _size)
	{
		m_points.resize(_size);
	}

	template<typename T>
	void CSpline<T>::clear()
	{
		m_points.clear();
	}

	template<typename T>
	void CSpline<T>::append(const std::vector<T>& points)
	{
		for (auto& p : points)
			m_points.push_back(p);
	}

	template<typename T>
	int CSpline<T>::size()
	{
		return m_points.size();
	}

	template<typename T>
	T CSpline<T>::getPoint(int idx) const
	{
		assert(idx >= 0 && idx < m_points.size());

		return m_points[idx];
	}

	template<typename T>
	T CSpline<T>::get(float t) const
	{

		if (m_points.size() < 2)
		{
			return m_points[0];
		}
		else
		{
			float U = 1.f / (m_points.size() - 1);

			int idx = t / U;

			if (idx == m_points.size() - 1)
				return m_points[m_points.size() - 1];

			T P0;
			T P1 = m_points[idx];
			T P2 = m_points[idx + 1];
			T P3;


			if (idx == 0)
				P0 = glm::normalize(P1 - P2) + P1;
			else
				P0 = m_points[idx - 1];

			if (idx + 2 >= m_points.size())
				P3 = glm::normalize(P2 - P1) + P1;
			else
				P3 = m_points[idx + 2];

			float t2 = ((t - (idx)*U) / U);

			return Math::getCSplinePoint<T>(P0, P1, P2, P3, t2);
		}

	}

	template<typename T>
	T& CSpline<T>::operator[](int index)
	{
		assert(index >= 0 && index < m_points.size());

		return m_points[index];
	}

	template<typename T>
	void CSpline<T>::save(Json::Value& objectRoot) const
	{
		objectRoot["size"] = m_points.size();
		for (int i = 0; i < m_points.size(); i++)
		{
			objectRoot["data"][i] = toJsonValue(m_points[i]);
		}
	}

	template<typename T>
	void CSpline<T>::load(const Json::Value& objectRoot)
	{
		int size = objectRoot.get("size", 0).asInt();
		for (int i = 0; i < size; i++)
		{
			m_points.push_back( fromJsonValue<T>(objectRoot["data"][i], T()) );
		}
	}



	template<typename T>
	T getCSplinePoint(const T& P0, const T& P1, const T& P2, const T& P3, float t)
	{
		return P1 + (t*(-0.5f * P0 + 0.5f * P2)) + (t*t*(P0 - 2.5f * P1 + 2.f * P2 - 0.5f * P3)) + (t*t*t*(-0.5f * P0 + 1.5f * P1 - 1.5f * P2 + 0.5f * P3));
	}

}

