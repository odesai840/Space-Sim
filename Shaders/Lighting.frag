#version 460 core
in vec3 FragPos;
in vec3 Normal;

uniform vec4 u_Color;
uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform float u_AmbientStrength;
uniform bool u_IsSun;
uniform float u_Time;

out vec4 FragColor;

float hash(float n) { return fract(sin(n) * 43758.5453123); }

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = p.x + p.y * 57.0 + p.z * 113.0;
    float res = mix(mix(mix(hash(n), hash(n + 1.0), f.x),
                        mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
                    mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                        mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

float fbm(vec3 p) {
    float f = 0.0;
    float amp = 0.5;
    for(int i = 0; i < 6; i++) {
        f += amp * noise(p);
        p *= 2.03;
        amp *= 0.5;
    }
    return f;
}

float turbulence(vec3 p, float frequency) {
    float t = 0.0;
    float f = 1.0;
    
    for(int i = 0; i < 7; i++) {
        t += abs(noise(p * f) / f);
        f *= 2.0;
    }
    
    return t;
}

void main()
{
    if (u_IsSun) {
        vec3 normal = normalize(Normal);
        vec3 viewDir = normalize(-FragPos);
        float NdotV = max(dot(normal, viewDir), 0.0);
        
        float limbDarkening = pow(NdotV, 0.6) * 0.5 + 0.5;
        
        vec3 baseColor = u_Color.rgb * limbDarkening;
        
        float largeDetail = fbm(FragPos * 1.5 + vec3(u_Time * 0.05));
        
        float mediumDetail = fbm(FragPos * 4.0 + vec3(0.0, u_Time * 0.1, 0.0));
        
        float smallDetail = fbm(FragPos * 12.0 + vec3(u_Time * 0.2));
        
        float surfaceDetail = largeDetail * 0.5 + mediumDetail * 0.3 + smallDetail * 0.2;
        
        vec3 hotSpotColor = vec3(1.0, 0.8, 0.2);
        vec3 mediumTempColor = vec3(1.0, 0.6, 0.1);
        vec3 coolerColor = vec3(0.9, 0.4, 0.1);
        
        vec3 tempColor = mix(
            mix(coolerColor, mediumTempColor, surfaceDetail * 1.2),
            hotSpotColor,
            pow(surfaceDetail, 2.0)
        );
        
        // Apply base color with temperature variations
        vec3 sunColor = mix(baseColor, tempColor, 0.75);
        
        float pulseIntensity = sin(u_Time * 0.4) * 0.07 + 1.0;
        sunColor *= pulseIntensity;
        
        float flareNoise = turbulence(FragPos * 2.0 + vec3(u_Time * 0.3), 2.0);
        float flareIntensity = pow(flareNoise, 5.0) * 2.0;
        
        float rim = pow(1.0 - NdotV, 4.0);
        flareIntensity *= rim * 2.0;
        
        sunColor += vec3(1.0, 0.7, 0.3) * flareIntensity;
        
        float corona = pow(1.0 - NdotV, 3.0);
        vec3 coronaColor = mix(vec3(1.0, 0.6, 0.1), vec3(1.0, 0.9, 0.7), corona);
        sunColor = mix(sunColor, coronaColor, corona * 0.6);
        
        if (corona > 0.3) {
            float chromatic = pow(corona, 1.5);
            sunColor.r *= 1.0 + chromatic * 0.5;
            sunColor.b *= 0.7 - chromatic * 0.3;
        }
        
        sunColor = sunColor * 1.3;
        
        sunColor = sunColor / (sunColor + vec3(1.0));
        
        FragColor = vec4(sunColor, 1.0);
    }
    else {
        vec3 ambient = u_AmbientStrength * u_LightColor;
        
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(u_LightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * u_LightColor;
        
        vec3 result = (ambient + diffuse) * u_Color.rgb;
        FragColor = vec4(result, u_Color.a);
    }
}