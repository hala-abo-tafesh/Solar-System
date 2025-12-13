#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec2 texCoord;

uniform sampler2D textureSample;

uniform vec3 viewPos;

uniform vec3 sunPos;
uniform vec3 sunColor;
uniform float sunIntensity;

uniform vec3 moonPos;
uniform vec3 moonColor;
uniform float moonIntensity;

vec3 CalcLight(vec3 lightPos, vec3 lightColor, float intensity,
               vec3 fragPos, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);

    vec3 ambient  = 0.10 * lightColor * intensity;
    vec3 diffuse  = diff * lightColor * intensity;
    vec3 specular = spec * lightColor * intensity * 0.08;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 baseColor = texture(textureSample, texCoord).rgb;

    float sunRadius = 4.0;
    if (distance(fragPos, sunPos) < sunRadius + 0.1)
    {
        FragColor = vec4(baseColor * 1.5, 1.0);
        return;
    }

    vec3 sunLight  = CalcLight(sunPos,  sunColor,  sunIntensity,
                               fragPos, normal, viewDir);
    vec3 moonLight = CalcLight(moonPos, moonColor, moonIntensity,
                               fragPos, normal, viewDir);

   
    float moonRadiusSelf = 0.16;
    float distToMoon = distance(fragPos, moonPos);
    vec3 moonSelfLight = vec3(0.08, 0.08, 0.12);
    if (distToMoon < moonRadiusSelf + 0.1)
    {
        FragColor = vec4(baseColor + moonSelfLight, 1.0);
        return;
    }

    vec3 finalColor = baseColor * (sunLight + moonLight);

    FragColor = vec4(finalColor, 1.0);
}
