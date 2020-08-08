uniform sampler2D source;

const float Threshold = 0.7;
const float Factor   = 4.0;

#define saturate(x) clamp(x, 0.f, 1.f)

void main()
{
	//vec4 sourceFragment = texture2D(source, gl_TexCoord[0].xy);
	//float luminance = sourceFragment.r * 0.2126 + sourceFragment.g * 0.7152 + sourceFragment.b * 0.0722;
	//sourceFragment *= clamp(luminance - Threshold, 0, 1) * Factor;
	//gl_FragColor = sourceFragment;
    
    vec3 current_color = texture2D(source, gl_TexCoord[0].xy).rgb;
    vec4 pixel =  vec4(0.0, 0.0, 0.0, 1.0);
    float brightness = dot(current_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness >= Threshold){
        pixel = texture2D(source, gl_TexCoord[0].xy);
    }
    gl_FragColor = pixel;
}