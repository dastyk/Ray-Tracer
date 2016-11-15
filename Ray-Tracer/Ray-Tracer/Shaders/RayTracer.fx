RWTexture2D<float4> output : register(u0);
#define M_PI 3.14159265358979323846f
#define D_PI 0.31830988618f
#define MAX_PRIMITIVES 1024

struct Ray
{
	float3 Origin;
	float3 Dir;
};

cbuffer CameraData : register(b0)
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

cbuffer CountData : register(b1)
{
	uint g_numSpheres;
	uint g_numTriangles;
	uint g_numPointLights;
}

ByteAddressBuffer sphereData : register(t0);
ByteAddressBuffer triangleData: register(t1);
ByteAddressBuffer pointLightData: register(t2);
//
//cbuffer SphereData : register(b2)
//{
//	struct
//	{
//		float4 Position3_Radius_1[MAX_PRIMITIVES];
//		float4 Color[MAX_PRIMITIVES];
//	}g_spheres;
//
//	
//};
//
//cbuffer TriangleData : register(b3)
//{
//	struct
//	{
//		float4 P0[MAX_PRIMITIVES];
//		float4 P1[MAX_PRIMITIVES];
//		float4 P2[MAX_PRIMITIVES];
//		float4 Color[MAX_PRIMITIVES];
//	}g_triangles;
//};
//
//cbuffer PointLightData : register(b4)
//{
//	struct
//	{
//		float4 Position3_Luminosity1[MAX_PRIMITIVES];
//	}g_pointLights;
//};



bool RaySphereIntersect(Ray r, float3 center, float radius, out float t, out float3 p, out float q)
{
	float3 l = center - r.Origin;
	float s = dot(l, r.Dir);
	float l2 = dot(l, l);
	float r2 = radius*radius;
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


bool RaySphereIntersect(Ray r, float3 center, float radius)
{
	float3 l = center - r.Origin;
	float s = dot(l, r.Dir);
	float l2 = dot(l, l);
	float r2 = radius*radius;
	if (s < 0.0f && l2 > r2)
		return false;

	float m2 = l2 - s*s;
	if (m2 > r2)
		return false;
	return true;
}

bool RayTriangleIntersect(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	float u = f*dot(s, q);
	if (u < 0.0)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0 || u + v > 1.0)
		return false;
	t = f*dot(e2, r);
	return true;

}


//groupshared float3 Color[MAX_PRIMITIVES];
groupshared float4 TempCache[MAX_PRIMITIVES];

float CalcShadow(float3 LightPos, float lum, uint i, float3 p, float3 normal, float3 v)
{
	Ray sunR;
	sunR.Origin = p;
	sunR.Dir = normalize(LightPos - sunR.Origin);
	float3 halfv = normalize(v + sunR.Dir);
	bool c = true;
	float a = dot(normal, sunR.Dir);
	float totalLum = 0.0f;
	if (a > 0.0f)
	{
		for (uint j = 0; j < g_numSpheres; j++)
		{
			if (j != i)
			{
				if (RaySphereIntersect(sunR, TempCache[j].xyz, TempCache[j].w))
				{
					c = false;
				}
			}
		}
		if (c)
		{
			float spec = pow(dot(normal, halfv), 17);
			totalLum += lum * a + spec;
		}
	}

	return totalLum;
}
[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	// Setup first ray
	Ray ray;
	float dx = (2 * ((threadID.x + 0.5) / g_screenWidth) - 1) * tan(g_fov / 2 * M_PI / 180) * g_aspect;
	float dy = (1 - 2 * ((threadID.y + 0.5) / g_screenHeight)) * tan(g_fov / 2 * M_PI / 180);

	float3 p1 = float3(dx*g_NearP, dy*g_NearP, g_NearP);
	float3 p2 = float3(dx*g_FarP, dy*g_FarP, g_FarP);

	p1 = mul(float4(p1, 0.0f), g_mViewInv).xyz;
	p2 = mul(float4(p2, 0.0f), g_mViewInv).xyz;

	ray.Origin = g_CameraPosition;
	ray.Dir = normalize(p2 - p1);
	float t = 3.402823466e+38F;
	float3 p;
	float q;

	float3 myColor = float3(0.0f,0.0f,0.0f);
	int finalSI = -1;

	// Load spheres into shared memory
	if (groupIndex < g_numSpheres)
		TempCache[groupIndex] = asfloat(sphereData.Load4(groupIndex * 16));
	
	GroupMemoryBarrierWithGroupSync();

	
	// Prefetch a pointlight
	/*float4 pointLight;
	if (groupIndex < g_numPointLights)
		pointLight = pointLightData.Load4(groupIndex*16);*/

	//GroupMemoryBarrierWithGroupSync();

	//myColor = float3(TempCache[0].w, 0.0f, 0.0f);

	// Check intersect with spheres
	for (uint i = 0; i < g_numSpheres; i++)
	{

		float tt = 0.0f;
		float3 pp;
		if (RaySphereIntersect(ray, TempCache[i].xyz, TempCache[i].w, tt, pp, q))
		{
			if (tt < t)
			{
				t = tt;
				p = pp;
				finalSI = i;
			}
			
		}
	}

	//// Load triangles into shared memory
	//if (groupIndex < g_numSpheres)
	//	groupS[groupIndex] = spheres[groupIndex];
	//GroupMemoryBarrierWithGroupSync();
	//for (uint i = 0; i < g_numTriangles; i++)
	//{

	//	float tt = 0.0f;
	//	float3 pp;
	//	if (RayTriangleIntersect(r, groupS[i], tt, pp))
	//	{
	//		if (tt < t)
	//		{
	//			t = tt;
	//			p = pp;
	//			finalSI = i;
	//			finalS = groupS[i];

	//		}

	//	}
	//}

	if (finalSI != -1)
	{
		float3 normal = normalize(p - TempCache[finalSI].xyz);
		float3 v = normalize(g_CameraPosition - p);
		float totalLight = 0.1f;

		//// Share prefetched light with rest of group.
		//if (groupIndex < g_numPointLights)
		//	TempCache[groupIndex] = pointLight;

		//GroupMemoryBarrierWithGroupSync();

		// Prefetch the color of my sphere
		float4 scolor = asfloat(sphereData.Load4(finalSI * 16 + 16 * MAX_PRIMITIVES));

		for (uint i = 0; i < g_numPointLights; i++)
		{
			float4 pointLight = asfloat(pointLightData.Load4(i * 16));
			totalLight += CalcShadow(pointLight.xyz , pointLight.w, finalSI, p, normal, v);
		}

		myColor = scolor.xyz * totalLight;
	}

	output[threadID.xy] = saturate(float4(myColor, 1.0f));

	//float d = 1 / 800.0f;
//	output[threadID.xy] = float4(pointLights[0].Position.x, 0.0f, 0.0f, 1.0f);// float4(g_CameraDir * (1 - length(threadID.xy - float2(400, 400)) / 400.0f), 1);
}
