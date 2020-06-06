#pragma once
#include <vector>
#include <utility>

/*
A class used to generate unique IDs.
IDs will always be non-negative integers.
*/
class IDGenerator {
public:
	IDGenerator();

	/*
	Generates a non-negative ID unique among all existing IDs.
	If the maximum number of IDs are in use, this will return -1.
	*/
	int generateID();
	/*
	Deletes an ID so it can be reused. Undefined behavior if the ID does not exist.
	*/
	void deleteID(int id);
	/*
	Marks an ID as being used.
	If the ID was already being used, this function will do nothing.
	*/
	void markIDAsUsed(int id);
	/*
	Returns the next ID to be generated without using it.
	*/
	int getNextID() const;
	/*
	Returns whether id is being used.
	*/
	bool idInUse(int id);

private:
	std::vector<std::pair<int, int>> ranges;
};