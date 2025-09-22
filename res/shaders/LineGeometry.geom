#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 12) out;

in  vec3 vPosView[];
in  vec4 vPass_color[];
uniform mat4  projMatrix;
uniform vec2  viewportSize;   // pixels
uniform float thickness;      // pixels

// ---- NEW: varyings for AA ----
noperspective out float vDistPx;  // signed distance across width (pixels) for BODY
noperspective out vec2  vCapPx;   // local (x,y) in pixels for CAP quads
flat           out int   vMode;   // 0 = body, 1 = cap
out vec4 pass_color;

// helpers
vec2 ndc(vec4 c){ return c.xy / c.w; }
vec2 safeNorm(vec2 v){ float L = length(v); return (L > 1e-6) ? v/L : vec2(1.0,0.0); }

// emit one square cap; also sets vCapPx at each corner
void emitSquareCap(vec4 C, vec2 aNdc, vec2 bNdc, float halfW){
    vMode = 1;
    // (-a - b)
    gl_Position = C + vec4((-aNdc - bNdc) * C.w, 0, 0);
    vCapPx = vec2(-halfW, -halfW); EmitVertex();
    // (+a - b)
    gl_Position = C + vec4(( aNdc - bNdc) * C.w, 0, 0);
    vCapPx = vec2(+halfW, -halfW); EmitVertex();
    // (-a + b)
    gl_Position = C + vec4((-aNdc + bNdc) * C.w, 0, 0);
    vCapPx = vec2(-halfW, +halfW); EmitVertex();
    // (+a + b)
    gl_Position = C + vec4(( aNdc + bNdc) * C.w, 0, 0);
    vCapPx = vec2(+halfW, +halfW); EmitVertex();

    EndPrimitive();
}

void main()
{
    pass_color = vPass_color[0];
    vec4 c0 = projMatrix * vec4(vPosView[0], 1.0);
    vec4 c1 = projMatrix * vec4(vPosView[1], 1.0);
    if (c0.w <= 0.0 && c1.w <= 0.0) return;

    vec2 n0 = ndc(c0), n1 = ndc(c1);
    vec2 seg = n1 - n0;
    vec2 dir = safeNorm(seg);
    vec2 perp = vec2(-dir.y, dir.x);

    vec2 px2ndc = 2.0 / viewportSize;
    float halfW = max(0.0, thickness * 0.5);

    vec2 rPerp  = perp * (halfW * px2ndc);
    vec2 rAlong = dir  * (halfW * px2ndc);

    // ---- BODY quad (4 verts) with signed distance across width ----
    vMode = 0;

    vec4 o0 = vec4(rPerp * c0.w, 0, 0);
    vec4 o1 = vec4(rPerp * c1.w, 0, 0);

    gl_Position = c0 - o0; vDistPx = -halfW; EmitVertex();
    gl_Position = c0 + o0; vDistPx = +halfW; EmitVertex();
    gl_Position = c1 - o1; vDistPx = -halfW; EmitVertex();
    gl_Position = c1 + o1; vDistPx = +halfW; EmitVertex();
    EndPrimitive();

    // ---- CAP squares (4 verts each) with local pixel coords ----
    emitSquareCap(c0, rAlong, rPerp, halfW);
    emitSquareCap(c1, rAlong, rPerp, halfW);
}
