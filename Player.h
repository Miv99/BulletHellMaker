#pragma once
#include <string>
#include "TextMarshallable.h"

class EditorPlayer : public TextMarshallable {
public:
	std::string format() override;
	void load(std::string formattedString) override;

private:
	int initialHealth;
	int maxHealth;
	float speed;
	float focusedSpeed;
};