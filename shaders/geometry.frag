#version 410 core
out vec4 FragColor;
in vec3 fColor;

void main()
{
	//vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
	//if (dot(circCoord, circCoord) > 1.0) {
	//	discard;
	//}
	FragColor= vec4(fColor, 1.0f);
}