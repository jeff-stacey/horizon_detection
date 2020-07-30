#version 330 

out vec4 Color;

uniform vec3 nadir;
uniform uint screen_width;
uniform uint screen_height;
uniform float alpha;
uniform float alpha_atmosphere;

// Distortion coeffs
uniform float K1;
uniform float K2;

uniform sampler2D noise_texture;

in vec2 tex_coord;

void main()
{
    // Width of the image plane, for z = -1, assuming FOV of 57 degrees
    const float width = 1.0859114f;
    float height = width * screen_height / screen_width;

    // Direction of frag in space
    vec3 dir = vec3((gl_FragCoord.x / screen_width - 0.5f) * width,
                    (gl_FragCoord.y / screen_height - 0.5f) * height,
                    -1.0f);
    
    // Add distortion to dir (just need to distory xy coordinates)
    {
        float r_sq = dir.x*dir.x + dir.y*dir.y;
        dir.x *= 1.0f / (1.0f + K1*r_sq + K2*r_sq*r_sq);
        dir.x *= 1.0f / (1.0f + K1*r_sq + K2*r_sq*r_sq);
    }

    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    if (dot(nadir, normalize(dir)) > alpha)
    {
        color = vec3(0.5f, 0.5f, 0.5f);
    }
    else if (dot(nadir, normalize(dir)) > alpha_atmosphere)
    {
        color = ((dot(nadir, normalize(dir)) - alpha_atmosphere) / (alpha - alpha_atmosphere)) * vec3(0.5f, 0.5f, 0.5f);
    }

    Color = vec4(color + texture(noise_texture, tex_coord).r * vec3(1.0f, 1.0f, 1.0f), 1.0f);
}
