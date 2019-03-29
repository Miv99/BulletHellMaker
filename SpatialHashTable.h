#pragma once
#include "Components.h"
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>

/*
Spatial hash table

The spatial hash table is composed of square cells (except possibly the right/topmost cells, which may be truncated
if the map width/height are not multiples of the cell size).
When objects are inserted into the table, they are inserted into every cell that overlap with the object's hitbox.

The table will not work for getting objects nearby objects that are outside map bounds.
*/
template<class T>
class SpatialHashTable {
public:
	inline SpatialHashTable() {}
	inline SpatialHashTable(float mapWidth, float mapHeight, float cellSize) : mapWidth(mapWidth), mapHeight(mapHeight), cellSize(cellSize) {
		buckets = std::vector<std::vector<T>>(int(ceil(mapWidth / cellSize)) * int(ceil(mapHeight / cellSize)));
		cellsPerMapWidth = int(mapWidth / cellSize);
		cellsPerMapHeight = int(mapHeight / cellSize);
	}

	inline void clear() {
		for (std::vector<T>& bucket : buckets) {
			bucket.clear();
		}
	}

	inline void insert(T object, const HitboxComponent& hitbox, const PositionComponent& position) {
		int leftmostXCell = std::max(0, (int)((position.getX() + hitbox.getX() - hitbox.getRadius()) / cellSize));
		int rightmostXCell = std::min(cellsPerMapWidth - 1, (int)((position.getX() + hitbox.getX() + hitbox.getRadius()) / cellSize));
		int topmostYCell = std::min(cellsPerMapHeight - 1, (int)((position.getY() + hitbox.getY() + hitbox.getRadius()) / cellSize));
		int bottommostYCell = std::max(0, (int)((position.getY() + hitbox.getY() - hitbox.getRadius()) / cellSize));
		
		for (int xCell = leftmostXCell; xCell <= rightmostXCell; xCell++) {
			for (int yCell = bottommostYCell; yCell <= topmostYCell; yCell++) {
				buckets[xCell + yCell * cellsPerMapWidth].push_back(object);
			}
		}
	}

	inline std::vector<T> getNearbyObjects(const HitboxComponent& hitbox, const PositionComponent& position) {
		int leftmostXCell = std::max(0, (int)((position.getX() + hitbox.getX() - hitbox.getRadius()) / cellSize));
		int rightmostXCell = std::min(cellsPerMapWidth - 1, (int)((position.getX() + hitbox.getX() + hitbox.getRadius()) / cellSize));
		int topmostYCell = std::min(cellsPerMapHeight - 1, (int)((position.getY() + hitbox.getY() + hitbox.getRadius()) / cellSize));
		int bottommostYCell = std::max(0, (int)((position.getY() + hitbox.getY() - hitbox.getRadius()) / cellSize));

		std::vector<T> all;
		for (int xCell = leftmostXCell; xCell <= rightmostXCell; xCell++) {
			for (int yCell = bottommostYCell; yCell <= topmostYCell; yCell++) {
				int i = xCell + yCell * cellsPerMapWidth;
				all.insert(all.end(), buckets[i].begin(), buckets[i].end());
			}
		}
		return all;
	}

private:
	float mapWidth;
	float mapHeight;
	float cellSize;
	int cellsPerMapWidth;
	int cellsPerMapHeight;
	std::vector<std::vector<T>> buckets;

	int hash(float x, float y) {
		return int(x / cellSize) + (int(y / cellSize) * cellsPerMapWidth);
	}
};

