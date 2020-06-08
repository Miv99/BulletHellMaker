// Darkens pixels
uniform float minBright;
uniform sampler2D texture;

void main(void) {
  vec4 bright = max(vec4(0.0), (texture2D(texture, gl_TexCoord[0].xy) - minBright));
  gl_FragColor = bright;
}