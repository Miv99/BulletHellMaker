// Brightens pixels
uniform sampler2D texture;
uniform float strength;

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = vec4(pixel.rgb * strength, pixel.a);
}