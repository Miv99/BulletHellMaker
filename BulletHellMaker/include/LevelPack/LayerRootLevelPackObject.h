#pragma once
#include <LevelPack/LevelPackObject.h>

/*
A LevelPackObject that is a root of a layer in the
LevelPackObject hierarchy. LevelPackObjects of these types 
can reference each other only by ID.
*/
class LayerRootLevelPackObject : public LevelPackObject {
public:

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	/*
	This shouldn't be used if the LevelPackObject already belongs to a LevelPack.
	*/
	inline void setID(int id) { this->id = id; }
	inline void setName(std::string name) { this->name = name; }
	/*
	Marks this object as having failed its load().
	*/
	inline void setFailedLoadAttempt(std::string formattedString) { failedLoadAttempt = formattedString; }

	inline int getID() const { return id; }
	inline std::string getName() const { return name; }
	inline bool isFailedLoad() const { return failedLoadAttempt.size() != 0; }

protected:
	// ID unique to all other LevelPackObjects of the same derived class. Only used for non-unique objects (see references for definition of a unique object).
	// Negative IDs should be reserved for object previews in the editor.
	int id;
	// User-defined name. Only used for non-unique objects.
	std::string name;

	std::string failedLoadAttempt;
};