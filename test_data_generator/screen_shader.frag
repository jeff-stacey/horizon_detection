#version 330 

out vec4 Color;

uniform vec3 nadir;
uniform uint screen_width;
uniform uint screen_height;

void main()
{
    // Width of the image plane, for z = -1
    const float width = 1.0859114f;
    float height = width * screen_height / screen_width;
    float alpha = sqrt(1.0f - (6371.0f/6871.0f)*(6371.0f/6871.0f));

    // Direction of frag in space
    vec3 dir = vec3((gl_FragCoord.x / screen_width - 0.5f)
        * width, (gl_FragCoord.y / screen_height - 0.5f) * height, -1.0f);

    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    if (dot(nadir, normalize(dir)) > alpha)
    {
        color = vec3(1.0f, 1.0f, 1.0f);
    }

    Color = vec4(color, 1.0f);
}
