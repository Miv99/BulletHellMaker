#version 130

// Blur shader with equal weights for all samples.
// The alpha of the blurred pixel decreases linearly with distance from the nearest colored pixel.

// The texture. Must be straight alpha.
uniform sampler2D texture;
// Screen resolution
uniform vec2 resolution;
// Direction to blur. Usually (1, 0) or (0, 1).
uniform vec2 direction;
// Number of samples to take in a direction
// (so total number of samples is (2n + 1))
uniform int n;

vec4 blur(sampler2D image, vec2 uv, vec2 resolution, vec2 direction, int n) {
  vec3 color = vec3(0.0);
  float fullWeight = 0.0;
  float w = 1.0/(2*n + 1);
  
  float nearestPixelAlpha = 0.0;

  for (int i = -n; i < n + 1; i++) {	
	vec2 newCoord = uv;
	if (direction.x != 0) {
		newCoord.x += (direction.x * i)/resolution.x;
	}
	if (direction.y != 0) {
		newCoord.y += (direction.y * i)/resolution.y;
	}
	
	vec4 next = texture2D(image, newCoord);
	if (next.a > 0) {
		nearestPixelAlpha = max(nearestPixelAlpha, next.a * (n - abs(i))/n);
		fullWeight += next.a * w;
		color += (next.rgb * w * next.a);
	}
  }
  
  color /= fullWeight;
  
  return vec4(color, nearestPixelAlpha);
}

void main() {
	gl_FragColor = blur(texture, gl_TexCoord[0].xy, resolution, direction, n);
}