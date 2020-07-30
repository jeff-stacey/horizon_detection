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

    vec2 frag_coord_centred = gl_FragCoord.xy - 0.5f*vec2(screen_width, screen_height);

    // Add lens distortion to frag coord
    {
        float r_sq = frag_coord_centred.x*frag_coord_centred.x
            + frag_coord_centred.y*frag_coord_centred.y;

        float screen_radius_sq = screen_width*screen_width + screen_height*screen_height;

        frag_coord_centred *= 1.0f + (K1*r_sq + K2*r_sq*r_sq) / screen_radius_sq;
    }

    vec3 dir = vec3(frag_coord_centred * (width / screen_width), -1.0f);

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
