#include "IDGenerator.h"
#include <limits>
#include <algorithm>
#include <cassert>

IDGenerator::IDGenerator() {
	ranges.push_back(std::make_pair(0, std::numeric_limits<int>::max()));
}

int IDGenerator::generateID() {
	// No more IDs are possible
	if (ranges.size() == 0) {
		return -1;
	}

	int id = ranges[0].first;
	ranges[0].first++;
	if (ranges[0].first > ranges[0].second) {
		ranges.erase(ranges.begin());
	}

	return id;
}

void IDGenerator::deleteID(int id) {
	std::pair<int, int> idRange = std::make_pair(id, id);
	// Binary search to find insertion position
	auto it = std::lower_bound(ranges.begin(), ranges.end(), idRange);
	assert(it != ranges.end()); // Tried to delete an ID that doesn't exist
	// Insert the new range
	it = ranges.insert(it, idRange);
	// Index of the new range in ranges
	int size = ranges.size();

	if (it != ranges.begin() && it != ranges.end() - 1 && (it - 1)->second + 1 == it->first && it->second + 1 == (it + 1)->first) {
		// New range is sandwiched between 2 ranges

		(it - 1)->second = (it + 1)->second;
		it = ranges.erase(it);
		ranges.erase(it);
	} else if (it != ranges.begin() && (it - 1)->second + 1 == it->first) {
		// New range overlaps with the range to its left

		(it - 1)->second = it->second;
		ranges.erase(it);
	} else if (it != ranges.end() - 1 && it->second + 1 == (it + 1)->first) {
		// New range overlaps with the range to its right

		(it + 1)->first = it->first;
		ranges.erase(it);
	}
}

void IDGenerator::markIDAsUsed(int id) {
	if (ranges.size() == 0) {
		// Every ID has already been used, so do nothing
		return;
	}

	std::pair<int, int> idRange = std::make_pair(id, id);
	// Binary search to find first range where range.start >= id
	auto it = std::lower_bound(ranges.begin(), ranges.end(), idRange);
	// id can fit into the range of either it, (it - 1), or nothing else
	if ((it == ranges.begin() && it->first > id) || it == ranges.end()) {
		// id can't fit anywhere
		return;
	}
	// Check if id fits into it
	if (id == it->first && id == it->second) {
		ranges.erase(it);
	} else if (id == it->first) {
		it->first++;
	} else if (id == it->second) {
		it->second--;
	} else if (id >= it->first && id <= it->second) {
		// id is in the middle of some range so split it up

		it->second = id - 1;
		ranges.insert(it + 1, std::make_pair(id + 1, it->second));
	} else {
		// Check if id fits into (it - 1)
		it--;
		if (id == it->first && id == it->second) {
			ranges.erase(it);
		} else if (id == it->first) {
			it->first++;
		} else if (id == it->second) {
			it->second--;
		} else if (id >= it->first && id <= it->second) {
			// id is in the middle of some range so split it up

			it->second = id - 1;
			ranges.insert(it + 1, std::make_pair(id + 1, it->second));
		}
	}
}

int IDGenerator::getNextID() const {
	// No more IDs are possible
	if (ranges.size() == 0) {
		return -1;
	}
	return ranges[0].first;
}

std::set<int> IDGenerator::getNextIDs(int count) const {
	 std::set<int> ids;
	 int i = 0;
	 while (i < ranges.size() && ids.size() < count) {
		 for (int j = ranges[i].first; j <= ranges[i].second && ids.size() < count; j++) {
			 ids.insert(j);
		 }
		 i++;
	 }
	 return ids;
}

bool IDGenerator::idInUse(int id) {
	if (ranges.size() == 0) {
		// Every ID has already been used
		return true;
	}

	std::pair<int, int> idRange = std::make_pair(id, id);
	// Binary search to find first range where range.start >= id
	auto it = std::lower_bound(ranges.begin(), ranges.end(), idRange);
	// id can fit into the range of either it, (it - 1), or nothing else
	if ((it == ranges.begin() && it->first > id) || it == ranges.end()) {
		// id can't fit anywhere
		return true;
	}
	// Check if id fits into it
	if (id == it->first && id == it->second) {
		return false;
	} else if (id == it->first) {
		return false;
	} else if (id == it->second) {
		return false;
	} else if (id >= it->first && id <= it->second) {
		return false;
	} else {
		// Check if id fits into (it - 1)
		it--;
		if (id == it->first && id == it->second) {
			return false;
		} else if (id == it->first) {
			return false;
		} else if (id == it->second) {
			return false;
		} else if (id >= it->first && id <= it->second) {
			return false;
		}
	}
	
	return true;
}
