#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float constant = 0.05f;
float linear = 0.0045f;
float quadratic = 0.0075f;
float shininess = 32.0f;

vec3 spotlightDirection;
float spotlightCutoffAngle;
float spotlightSoftness;

uniform int dirLight;
uniform int spotLigh;
uniform int fogOn;

in vec4 fPosEye;
in vec4 lightPosEye;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

void computePointLight()
{	
	float dist = length(lightPosEye.xyz - vec3(fPosEye));
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	// camera pos
	vec3 cameraPosEye = vec3(0.0f);
	
	vec3 normalEye = normalize(fNormal);
	
	vec3 lightDirN = normalize(lightPosEye.xyz - vec3(fPosEye.x,fPosEye.y,fPosEye.z));
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	ambient += att * ambientStrength * lightColor;
	
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular += att * specularStrength * specCoeff * lightColor;	
	
}

void computeSpotLight()
{	
    // Calculate the vector from the fragment to the light source
    //lightDir = normalize(pos - fPosEye.xyz);

    // Calculate the angle between the light direction and the spotlight direction
    float cosTheta = dot(lightDir, normalize(spotlightDirection));

    // Calculate the spotlight cone factor based on the cone angle
    float spotlightFactor = smoothstep(spotlightCutoffAngle, spotlightCutoffAngle + spotlightSoftness, cosTheta);

    // Calculate the distance from the fragment to the light source
    float dist = length(lightPosEye.xyz - vec3(fPosEye));

    // Calculate attenuation based on distance
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    // Camera position
    vec3 cameraPosEye = vec3(0.0f);

    // Normalize the normal
    vec3 normalEye = normalize(fNormal);

    // Normalize the light direction
    vec3 lightDirN = normalize(lightPosEye.xyz - vec3(fPosEye.x, fPosEye.y, fPosEye.z));

    // Compute view direction
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    // Compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    // Ambient
    ambient += att * ambientStrength * spotlightFactor * lightColor;

    // Diffuse
    diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * spotlightFactor * lightColor;

    // Compute reflection
    vec3 reflection = reflect(-lightDirN, normalEye);
    
    // Specular
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular += att * specularStrength * specCoeff * spotlightFactor * lightColor;
}


 float computeFog(){
 float fogDensity = 0.01f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}



void main() 
{
    //computeDirLight();

    //compute final vertex color
    //vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    //fColor = vec4(color, 1.0f);

	vec4 color;
    if(dirLight == 1){
		//computeDirLight();
		computeLightComponents();
			
	} 
	else computePointLight();

	//else computeSpotLight();

	vec3 ambientFinal = 0.6*ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 specularFinal = specular * texture(specularTexture, fTexCoords).rgb;
	vec3 diffuseFinal = 0.5*diffuse * texture(diffuseTexture, fTexCoords).rgb;

	//color = vec4(min((0.6*ambient + 0.5*diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f),1.0f);
	//color = vec4(min(0.6*ambient* texture(diffuseTexture, fTexCoords).rgb  + 0.5*diffuse* texture(diffuseTexture, fTexCoords).rgb )  + specular * texture(specularTexture, fTexCoords).rgb, 1.0f),1.0f);
	//color = vec4(min(0.6 * ambient * texture(diffuseTexture, fTexCoords).rgb + 0.5 * diffuse * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0), 1.0);
	color = vec4(min(vec3(ambientFinal + diffuseFinal + specularFinal), vec3(1.0)),1.0);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	//Color = mix(fogColor, color, fogFactor);

	if(fogOn) fColor = vec4(mix(fogColor.rgb, color.rgb, fogFactor),1.0);
    else fColor = vec4(color.rgb, 1.0f);
	//fColor = mix(fogColor, color, fogFactor);

}
