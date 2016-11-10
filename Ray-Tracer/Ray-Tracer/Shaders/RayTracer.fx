RWTexture2D<float4> output : register(u0);
#define M_PI 3.14159265358979323846f
struct Ray
{
	float3 Origin;
	float3 Dir;
};
struct Sphere
{
	float3 Position;
	float radius;
	float3 Color;
};
struct PointLight
{
	float3 Position;
	float luminosity;
};


cbuffer CountData : register(b0)
{
	uint g_numSpheres;
	uint g_numPointLights;
}
cbuffer CameraData : register(b1)
{
	matrix g_mViewInv;
	float g_aspect;
	float g_screenWidth;
	float g_screenHeight;
	float g_fov;
	float3 g_CameraPosition;
	float g_NearP;
	float3 g_CameraDir;	
	float g_FarP;
}


StructuredBuffer<Sphere> spheres : register(t0);
StructuredBuffer<PointLight> pointLights : register(t1);


bool RaySphereIntersect(Ray r, Sphere sph, out float t, out float3 p, out float q)
{
	float3 l = sph.Position - r.Origin;
	float s = dot(l, r.Dir);
	float l2 = dot(l, l);
	float r2 = sph.radius*sph.radius;
	if (s < 0.0f && l2 > r2)
		return false;

	float m2 = l2 - s*s;
	if (m2 > r2)
		return false;
	q = sqrt(r2 - m2);
	if (l2 > r2)
		t = s - q;
	else
		t = s + q;
	p = r.Origin + t*r.Dir;
	return true;
}


bool RaySphereIntersect(Ray r, Sphere sph)
{
	float3 l = sph.Position - r.Origin;
	float s = dot(l, r.Dir);
	float l2 = dot(l, l);
	float r2 = sph.radius*sph.radius;
	if (s < 0.0f && l2 > r2)
		return false;

	float m2 = l2 - s*s;
	if (m2 > r2)
		return false;
	return true;
}
float CalcShadow(PointLight light, uint i, float3 p, float3 normal)
{
	Ray sunR;
	sunR.Origin = p;
	sunR.Dir = normalize(light.Position - sunR.Origin);
	bool c = true;
	float v = dot(normal, sunR.Dir);
	float totalLum = 0.0f;
	if (v > 0.0f)
	{
		for (uint j = 0; j < g_numSpheres; j++)
		{
			if (j != i)
			{
				if (RaySphereIntersect(sunR, spheres[j]))
				{
					c = false;
				}
			}
		}
		if (c)
		{
			totalLum += light.luminosity * v;
		}
	}

	return totalLum;
}

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID)
{
	// Setup first ray
	Ray r;
	float dx = (2 * ((threadID.x + 0.5) / g_screenWidth) - 1) * tan(g_fov / 2 * M_PI / 180) * g_aspect;
	float dy = (1 - 2 * ((threadID.y + 0.5) / g_screenHeight)) * tan(g_fov / 2 * M_PI / 180);

	float3 p1 = float3(dx*g_NearP, dy*g_NearP, g_NearP);
	float3 p2 = float3(dx*g_FarP, dy*g_FarP, g_FarP);

	p1 = mul(float4(p1, 0.0f), g_mViewInv).xyz;
	p2 = mul(float4(p2, 0.0f), g_mViewInv).xyz;

	r.Origin = g_CameraPosition;
	r.Dir = normalize(p2 - p1);
	float t = 3.402823466e+38F;
	float3 p;
	float q;

	float3 myColor = float3(0.0f,0.0f,0.0f);
	int finalSI = -1;
	Sphere finalS;
	// Check intersect with spheres
	for (uint i = 0; i < g_numSpheres; i++)
	{

		float tt = 0.0f;
		float3 pp;
		if (RaySphereIntersect(r, spheres[i], tt, pp, q))
		{
			if (tt < t)
			{
				t = tt;
				p = pp;
				finalSI = i;
				finalS = spheres[i];

			}
			
		}
	}
	if (finalSI != -1)
	{
		float3 normal = normalize(p - finalS.Position);
		float totalShadow = 0.1f;

		for (uint i = 0; i < g_numPointLights; i++)
		{
			totalShadow += CalcShadow(pointLights[i], finalSI, p, normal);
		}

		myColor = finalS.Color * totalShadow;
	}
	output[threadID.xy] = saturate(float4(myColor, 1.0f));

	//float d = 1 / 800.0f;
//	output[threadID.xy] = float4(pointLights[0].Position.x, 0.0f, 0.0f, 1.0f);// float4(g_CameraDir * (1 - length(threadID.xy - float2(400, 400)) / 400.0f), 1);
}
