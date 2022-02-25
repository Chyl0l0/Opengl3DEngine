#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 fPos;


layout (location = 0) out vec4 fColor;
layout (location = 1) out vec4 brightColor;



//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform	vec3 pointLightDir;
uniform	vec3 pointLightColor;
uniform	vec3 pointLightPos0;
uniform	vec3 pointLightColor0;
uniform	vec3 pointLightPos1;
uniform	vec3 pointLightColor1;
uniform	vec3 pointLightPos2;
uniform	vec3 pointLightColor2;
uniform	vec3 pointLightPos3;
uniform	vec3 pointLightColor3;


struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];


//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float shadow;


float computeFog()
{
 float fogDensity = 0.005f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 8));
 return clamp(fogFactor, 0.8f, 1.0f);
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

    if(normalizedCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float constant = 1.0f;
float linear = 0.7f;
float quadratic = 1.8f;

vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
//transform normal
vec3 normalEye = normalize(fNormal);
//compute view direction 
vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

void computePointLight(vec3 pointLightDir, vec3 pointLightColor,float ambientStrength, float specularStrength)
{
	vec3 lightDir = normalize(pointLightDir - fPos);
	vec3 reflection = reflect(-lightDir, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0), shininess);
	float dist = length(pointLightDir - fPos);
    float att = 1 / (constant + linear * dist + quadratic * (dist * dist)); 

	ambient += att * ambientStrength * pointLightColor;
	diffuse += att * max(dot(normalEye, lightDir), 0.0f) * pointLightColor;
	specular += att * specularStrength * specCoeff * pointLightColor;

}


void computeLightComponents()
{		
	//directional light

	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	//compute ambient light
	ambient = ambientStrength * lightColor;
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	//point lights
	computePointLight(pointLightDir, pointLightColor, 100.0f, 30.0f );
	computePointLight(pointLightPos0, pointLightColor0, 100.0f, 30.0f );
	computePointLight(pointLightPos1, pointLightColor1, 100.0f, 30.0f );
	computePointLight(pointLightPos2, pointLightColor2, 100.0f, 30.0f );
	computePointLight(pointLightPos3, pointLightColor3, 100.0f, 30.0f );
}


void main() 
{
	computeLightComponents();
	
	shadow = computeShadow();

	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	vec3 color = (ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular;
    

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.1f, 0.42f, 0.51f, 1.0f);




	fColor = vec4 (mix(fogColor.xyz, color, fogFactor),1.0f);

	// check whether result is higher than some threshold, if so, output as bloom threshold color
	
    float brightness = dot(fColor.xyz, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColor = fColor;
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);

   
}

/*

void main()
{           
    vec3 color = texture(diffuseTexture, fTexCoords).rgb;
    // ambient
    vec3 ambient = 0.0 * color;
    // lighting
    vec3 lighting = vec3(0.0);
 
        // diffuse
        vec3 lightDir = normalize(pointLightDir - fPos);
        float diff = max(dot(lightDir, normalEye), 0.0);
        vec3 diffuse = pointLightColor * diff * color;      
        vec3 result = diffuse;        
        // attenuation (use quadratic as we have gamma correction)
        float distance = length(fPos - pointLightDir);
        result *= 1.0 / (distance * distance);
        lighting += result;
                
		fColor = vec4(ambient + lighting, 1.0);
}
*/
