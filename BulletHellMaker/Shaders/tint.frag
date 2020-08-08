uniform sampler2D texture;

uniform vec4 flashColor;
uniform vec4 textureModulatedColor;

void main() {
    vec4 pixelColor = texture2D(texture, gl_TexCoord[0].xy);
    pixelColor.r = pixelColor.r * textureModulatedColor.r;
    pixelColor.g = pixelColor.g * textureModulatedColor.g;
    pixelColor.b = pixelColor.b * textureModulatedColor.b;
    
    float percent = flashColor.a;

    vec4 colorDifference = vec4(0,0,0,1);

    colorDifference.r = flashColor.r - pixelColor.r;
    colorDifference.g = flashColor.g - pixelColor.g;
    colorDifference.b = flashColor.b - pixelColor.b;
    pixelColor.r = pixelColor.r + colorDifference.r * percent;
    pixelColor.g = pixelColor.g + colorDifference.g * percent;
    pixelColor.b = pixelColor.b + colorDifference.b * percent;


    gl_FragColor = pixelColor; 
}