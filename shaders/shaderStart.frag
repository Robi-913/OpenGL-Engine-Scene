#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

//textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//fog
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 16.0f;

//point light
uniform vec3 pointLightPos;
uniform bool pointLightEnabled;
float pointLightConstant = 1.0f;
float pointLightLinear = 0.09f;
float pointLightQuadratic = 0.032f;


//compute lighting: ambient, diffuse, specular
void computeLightComponents(out vec3 ambient, out vec3 diffuse, out vec3 specular) {
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	//ambient
	ambient = ambientStrength * lightColor;

	//diffuse
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//specular
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadowPCF() {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5; //transform to [0,1]

	//check if the frag is outside the lights far plane
	if (projCoords.z > 1.0) return 0.0;

	float shadow = 0.0;
	float bias = 0.005;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	//Percentage-Closer Filtering (PCF) for smoother shadows
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	return shadow;
}

vec3 pointLightCompute()
{
	if (!pointLightEnabled) {
		return vec3(0.0f); //night
	}

	vec3 cameraPosEye = vec3(0.0f);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(pointLightPos - fPosEye.xyz);

	float diff = max(dot(normalEye, lightDirN), 0.0);

	vec3 reflectDir = reflect(-lightDirN, normalEye);
	float spec = pow(max(dot(viewDirN, reflectDir), 0.0), shininess);

	float distance = length(pointLightPos - fPosEye.xyz);
	float attenuation = 1.0 / (pointLightConstant + pointLightLinear * distance +
	pointLightQuadratic * (distance * distance));

	vec3 pointLightColor = vec3(0.0f, 1.0f, 0.0f); //green

	vec3 ambientPoint = ambientStrength * pointLightColor * vec3(texture(diffuseTexture, fTexCoords));
	vec3 diffusePoint = diff * pointLightColor * vec3(texture(diffuseTexture, fTexCoords));
	vec3 specularPoint = specularStrength * spec * vec3(texture(specularTexture, fTexCoords));

	ambientPoint *= attenuation;
	diffusePoint *= attenuation;
	specularPoint *= attenuation;

	return (ambientPoint + diffusePoint + specularPoint);
}



void main() {
	vec3 ambient, diffuse, specular;
	computeLightComponents(ambient, diffuse, specular);

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	if (texture(specularTexture, fTexCoords).a < 0.1f) {
		discard;
	}

	float shadow = computeShadowPCF();

	vec3 color = min((ambient + (1.0 - shadow) * diffuse) + (1.0 - shadow) * specular, 1.0);

	float distance = length(fPosEye.xyz);
	float fogFactor = clamp(pow((fogEnd - distance) / (fogEnd - fogStart), 1.5), 0.0, 1.0);

	vec3 finalColor = mix(fogColor, color, fogFactor);
	vec3 finalPointColor = pointLightCompute();

	fColor = vec4(finalColor, 1.0f);

	if (pointLightEnabled){
		fColor = vec4(finalPointColor, 1.0f);
	}
}