#version 130

// Gaussian blur

// The texture. Must be straight alpha.
uniform sampler2D texture;
// Screen resolution
uniform vec2 resolution;
// Direction to blur. Usually (1, 0) or (0, 1).
uniform vec2 direction;
// Set mode to 0 for low sample count blur, 1 for medium sample count blur, 2 for high sample count
uniform int mode;

// sigma = 3
//float weights0[5] = float[5](0.1784,0.210431,0.222338,0.210431,0.1784);
// sigma = 10
float weights0[5] = float[5](0.2,0.2,0.2,0.2,0.2);
// sigma = 3
float weights1[11] = float[11](0.035822,0.05879,0.086425,0.113806,0.13424,0.141836,0.13424,0.113806,0.086425,0.05879,0.035822);
// sigma = 10
float weights2[21] = float[21](0.03426,0.037671,0.04101,0.044202,0.047168,0.049832,0.052124,0.053979,0.055344,0.05618,0.056461,0.05618,0.055344,0.053979,0.052124,0.049832,0.047168,0.044202,0.04101,0.037671,0.03426);
// sigma = 2
//float weights[21] = float[21](0.000001,0.00001,0.000078,0.000489,0.002403,0.009245,0.027835,0.065591,0.120978,0.174666,0.197413,0.174666,0.120978,0.065591,0.027835,0.009245,0.002403,0.000489,0.000078,0.00001,0.000001);

vec4 blurCustom2(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec3 color = vec3(0.0);
  float fullWeight = 0.0;
  float a = 0.0;
  
  float nearestPixelAlpha = 0.0;

  for (int i = -10; i < 11; i++) {
	float w = 0.0;
	
	if (i == -10) w = weights2[0];
	else if (i == -9) w = weights2[1];
	else if (i == -8) w = weights2[2];
	else if (i == -7) w = weights2[3];
	else if (i == -6) w = weights2[4];
	else if (i == -5) w = weights2[5];
	else if (i == -4) w = weights2[6];
	else if (i == -3) w = weights2[7];
	else if (i == -2) w = weights2[8];
	else if (i == -1) w = weights2[9];
	else if (i == 0) w = weights2[10];
	else if (i == 1) w = weights2[11];
	else if (i == 2) w = weights2[12];
	else if (i == 3) w = weights2[13];
	else if (i == 4) w = weights2[14];
	else if (i == 5) w = weights2[15];
	else if (i == 6) w = weights2[16];
	else if (i == 7) w = weights2[17];
	else if (i == 8) w = weights2[18];
	else if (i == 9) w = weights2[19];
	else if (i == 10) w = weights2[20];
	
	vec2 newCoord = uv;
	if (direction.x != 0) {
		newCoord.x += (direction.x * i)/resolution.x;
	}
	if (direction.y != 0) {
		newCoord.y += (direction.y * i)/resolution.y;
	}
	
	vec4 next = texture2D(image, newCoord);
	if (next.a > 0) {
		nearestPixelAlpha = max(nearestPixelAlpha, next.a * (10.0 - abs(i))/10.0);
		fullWeight += next.a * w;
		color += (next.rgb * w * next.a);
	}
  }
  
  color /= fullWeight;
  
  return vec4(color, nearestPixelAlpha);
}

vec4 blurCustom1(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec3 color = vec3(0.0);
  float fullWeight = 0.0;
  float a = 0.0;
  
  float nearestPixelAlpha = 0.0;

  for (int i = -5; i < 6; i++) {
	float w = 0.0;
	
	if (i == -5) w = weights1[0];
	else if (i == -4) w = weights1[1];
	else if (i == -3) w = weights1[2];
	else if (i == -2) w = weights1[3];
	else if (i == -1) w = weights1[4];
	else if (i == 0) w = weights1[5];
	else if (i == 1) w = weights1[6];
	else if (i == 2) w = weights1[7];
	else if (i == 3) w = weights1[8];
	else if (i == 4) w = weights1[9];
	else if (i == 5) w = weights1[10];
	
	vec2 newCoord = uv;
	if (direction.x != 0) {
		newCoord.x += (direction.x * i)/resolution.x;
	}
	if (direction.y != 0) {
		newCoord.y += (direction.y * i)/resolution.y;
	}
	
	vec4 next = texture2D(image, newCoord);
	if (next.a > 0) {
		int dist = i;
		if (dist < 0) dist = -dist;
		nearestPixelAlpha = max(nearestPixelAlpha, next.a * (10.0 - dist)/10.0);
	}
	fullWeight += next.a * w;
	color += (next.rgb * w * next.a);
  }
  
  color /= fullWeight;
  
  return vec4(color, nearestPixelAlpha);
}

vec4 blurCustom0(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec3 color = vec3(0.0);
  float fullWeight = 0.0;
  float a = 0.0;
  
  float nearestPixelAlpha = 0.0;

  for (int i = -2; i < 2; i++) {
	float w = 0.0;
	
	if (i == -2) w = weights0[0];
	else if (i == -1) w = weights0[1];
	else if (i == 0) w = weights0[2];
	else if (i == 1) w = weights0[3];
	else if (i == 2) w = weights0[4];
	
	vec2 newCoord = uv;
	if (direction.x != 0) {
		newCoord.x += (direction.x * i)/resolution.x;
	}
	if (direction.y != 0) {
		newCoord.y += (direction.y * i)/resolution.y;
	}
	
	vec4 next = texture2D(image, newCoord);
	if (newCoord.x < 0 || newCoord.y < 0 || newCoord.x > 1 || newCoord.y > 1) {
		continue;
	}
	if (next.a > 0) {
		int dist = i;
		if (dist < 0) dist = -dist;
		nearestPixelAlpha = max(nearestPixelAlpha, next.a * (10.0 - dist)/10.0);
	}
	fullWeight += next.a * w;
	color += (next.rgb * w * next.a);
  }
  
  color /= fullWeight;
  
  return vec4(color, nearestPixelAlpha);
}

void main() {
	if (mode == 0) {
		gl_FragColor = blurCustom0(texture, gl_TexCoord[0].xy, resolution, direction);
	} else if (mode == 1) {
		gl_FragColor = blurCustom1(texture, gl_TexCoord[0].xy, resolution, direction);
	} else {
		gl_FragColor = blurCustom2(texture, gl_TexCoord[0].xy, resolution, direction);
	}
}