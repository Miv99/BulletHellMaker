#include <Editor/Util/TguiUtils.h>

sf::RenderStates toSFMLRenderStates(tgui::RenderStates renderStates) {
	const float* matrix = renderStates.transform.getMatrix();
	sf::Transform transform = sf::Transform(matrix[0], matrix[4], matrix[12], matrix[1], matrix[5], matrix[13], matrix[3], matrix[7], matrix[15]);
	return sf::RenderStates(transform);
}

sf::Vector2f operator+(const sf::Vector2f v1, const tgui::Vector2f v2) {
	return sf::Vector2f(v1.x + v2.x, v1.y + v2.y);
}

sf::Vector2f operator-(const sf::Vector2f v1, const tgui::Vector2f v2) {
	return sf::Vector2f(v1.x - v2.x, v1.y - v2.y);
}
