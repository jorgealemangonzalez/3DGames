varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec3 camera_position;

void main()
{
    vec3 E = v_world_position - camera_position;
	float yaw = 0.0;
    yaw = atan2(E.x,E.z) / 6.28318531 - 0.25;
    float pitch = asin(E.y) / 1.57079633;
    uv = vec2(yaw,abs(pitch));
    gl_FragColor = texture2D(u_texture, uv);
}