#pragma once
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>

#include <Game/Components/HitboxComponent.h>
#include <Game/Components/PositionComponent.h>

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
	SpatialHashTable() {}
	SpatialHashTable(float mapWidth, float mapHeight, float cellSize) : mapWidth(mapWidth), mapHeight(mapHeight), cellSize(cellSize) {
		buckets = std::vector<std::vector<T>>(int(ceil(mapWidth / cellSize)) * int(ceil(mapHeight / cellSize)));
		cellsPerMapWidth = int(mapWidth / cellSize);
		cellsPerMapHeight = int(mapHeight / cellSize);
	}

	void clear() {
		for (std::vector<T>& bucket : buckets) {
			bucket.clear();
		}
	}

	void insert(T object, float hitboxX, float hitboxY, float hitboxRadius, const PositionComponent& position) {
		int leftmostXCell = std::max(0, (int)((position.getX() + hitboxX - hitboxRadius) / cellSize));
		int rightmostXCell = std::min(cellsPerMapWidth, (int)((position.getX() + hitboxX + hitboxRadius) / cellSize));
		int topmostYCell = std::min(cellsPerMapHeight, (int)((position.getY() + hitboxY + hitboxRadius) / cellSize));
		int bottommostYCell = std::max(0, (int)((position.getY() + hitboxY - hitboxRadius) / cellSize));
		
		for (int xCell = leftmostXCell; xCell <= rightmostXCell; xCell++) {
			for (int yCell = bottommostYCell; yCell <= topmostYCell; yCell++) {
				buckets[xCell + yCell * cellsPerMapWidth].push_back(object);
			}
		}
	}

	void insert(T object, const HitboxComponent& hitbox, const PositionComponent& position) {
		insert(object, hitbox.getX(), hitbox.getY(), hitbox.getRadius(), position);
	}

	std::vector<T> getNearbyObjects(const HitboxComponent& hitbox, const PositionComponent& position) {
		int leftmostXCell = std::max(0, (int)((position.getX() + hitbox.getX() - hitbox.getRadius()) / cellSize));
		int rightmostXCell = std::min(cellsPerMapWidth, (int)((position.getX() + hitbox.getX() + hitbox.getRadius()) / cellSize));
		int topmostYCell = std::min(cellsPerMapHeight, (int)((position.getY() + hitbox.getY() + hitbox.getRadius()) / cellSize));
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

