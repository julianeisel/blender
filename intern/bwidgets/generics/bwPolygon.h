#pragma once

#include <vector>

namespace bWidgets {

using bwPointVec = std::vector<class bwPoint>;

class bwPolygon
{
public:
	bwPolygon();
	bwPolygon(const bwPolygon& poly);
	bwPolygon(const bwPointVec& vertices);
	explicit bwPolygon(const unsigned int reserve_vertex_count);

	void addVertex(class bwPoint vertex);
	void addVertex(const float x, const float y);
	void reserve(const unsigned int count);
	const bwPointVec& getVertices() const;

	bwPoint& operator[](const unsigned int index);

	bool isDrawable() const;

protected:
	bwPointVec vertices{};
	unsigned int vert_count{0};
};

} // namespace bWidgets
