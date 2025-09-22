#version 330 core

noperspective in float vDistPx;  // from body quad
noperspective in vec2  vCapPx;   // from cap quads
flat           in int   vMode;   // 0=body, 1=cap
in vec4 pass_color;

out vec4 outColor;

uniform float thickness;    // pixels

void main()
{
    float halfW = 0.5 * thickness;
    float coverage;

    if (vMode == 0) {
        // BODY: signed distance to edge in pixels
        float edge = halfW - abs(vDistPx);
        float fw   = max(fwidth(vDistPx), 1e-4);
        coverage   = clamp(edge / fw, 0.0, 1.0);
    } else {
        // CAP: make it ROUND via a circle mask on the square geometry
        float r    = length(vCapPx);
        float edge = halfW - r;
        float fw   = max(fwidth(r), 1e-4);
        coverage   = clamp(edge / fw, 0.0, 1.0);
        // If you prefer SQUARE caps, comment the 4 lines above and set:
        // coverage = 1.0;  // (let MSAA handle the square’s outer edge)
    }

    // Alpha output
    outColor = vec4(pass_color.rgb, pass_color.a);
}
