
#define M_PI 3.14159265358979323846f
#define D_PI 0.31830988618f
#define MAX_PRIMITIVES 128
#define MAX_LIGHTS 16
struct Ray
{
	float4 Origin;
	float4 Dir;
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
	uint g_numTexTriangles;
	uint g_numSpotLights;
}

cbuffer PickingData : register(b2)
{
	Ray g_MouseRay;
}


ByteAddressBuffer sphereData : register(t0);
ByteAddressBuffer triangleData: register(t1);
ByteAddressBuffer pointLightData: register(t2);
ByteAddressBuffer texTriangleData: register(t3);
ByteAddressBuffer spotLightData: register(t4);
StructuredBuffer<matrix> translationBuffer: register(t5);

struct PickingInfo
{
	float3 pos;
	float t;
	uint type;
	int ID;
};

RWStructuredBuffer<PickingInfo> pickingResult : register(u0);

bool RaySphereIntersect(Ray r, float3 center, float radius, out float t, out float3 p, out float3 normal)
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
	float q = sqrt(r2 - m2);
	if (l2 > r2)
		t = s - q;
	else
		t = s + q;
	p = r.Origin + t*r.Dir;
	normal = normalize(p - center);
	return true;
}


bool RaySphereIntersect(Ray r, float3 center, float radius, out float t, out float3 p)
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
	float q = sqrt(r2 - m2);
	if (l2 > r2)
		t = s - q;
	else
		t = s + q;
	p = r.Origin + t*r.Dir;
	return true;
}

bool RayTriangleIntersectBackFaceCullTexture(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p, out float3 normal, out float u, out float v)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	normal = normalize(cross(e1, e2));
	float theta = dot(ray.Dir, normal);
	if (theta > 0.0f)
		return false;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	u = f*dot(s, q);
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	p = ray.Origin + ray.Dir * t;
	return true;

}

bool RayTriangleIntersectBackFaceCull(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p, out float3 normal)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	normal = normalize(cross(e1, e2));
	float theta = dot(ray.Dir, normal);
	if (theta > 0.0f)
		return false;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	float u = f*dot(s, q);
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	p = ray.Origin + ray.Dir * t;
	return true;

}
bool RayTriangleIntersectBackFaceCullNoData(Ray ray, float3 p0, float3 p1, float3 p2, out float t)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	float3 normal = normalize(cross(e1, e2));
	float theta = dot(ray.Dir, normal);
	if (theta > 0.0f)
		return false;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	float u = f*dot(s, q);
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	return true;

}
bool RayTriangleIntersectFrontFaceCullNoData(Ray ray, float3 p0, float3 p1, float3 p2,out float t)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	float3 normal = normalize(cross(e1, e2));
	float theta = dot(ray.Dir, normal);
	if (theta < 0.0f)
		return false;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	float u = f*dot(s, q);
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;

	return true;

}
bool RayTriangleIntersectFrontFaceCull(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p, out float3 normal)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	normal = normalize(cross(e1, e2));
	float theta = dot(ray.Dir, normal);
	if (theta < 0.0f)
		return false;

	float3 q = cross(ray.Dir, e2);
	float a = dot(e1, q);

	if (a > -0.000000000001f && a < 0.000000000001f)
		return false;
	float f = 1 / a;
	float3 s = ray.Origin - p0;
	float u = f*dot(s, q);
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	p = ray.Origin + ray.Dir * t;
	return true;

}
bool RayTriangleIntersectNoFaceCull(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p, out float3 normal)
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
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	p = ray.Origin + ray.Dir * t;
	normal = normalize(cross(e1, e2));
	return true;

}
bool RayTriangleIntersectNoFaceCullMinData(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p)
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
	if (u < 0.0f)
		return false;
	float3 r = cross(s, e1);
	float v = f*dot(ray.Dir, r);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	t = f*dot(e2, r);
	if (t < 0.0f)
		return false;
	p = ray.Origin + ray.Dir * t;
	return true;

}

[numthreads(1024, 1, 1)]
void main(uint threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint groupID : SV_GroupID)
{
	///////////////////////
	/*Do intersection*/
	///////////////////////
	PickingInfo localInfo;
	localInfo.t = 3.402823466e+38F;
	localInfo.ID = -1;
	localInfo.type = 4;
	localInfo.pos = float3(0.0f, 0.0f, 0.0f);

	if (threadID < g_numTexTriangles)
	{
		float tt;
		float3 p;
		float3  ftemp = asfloat(texTriangleData.Load4(threadID * 16)).xyz; // p0
		float4 ftemp1 = asfloat(texTriangleData.Load4(threadID * 16 + g_numTexTriangles * 16)).xyzw; // p1
		float3 ftemp2 = asfloat(texTriangleData.Load4(threadID * 16 + g_numTexTriangles * 32)).xyz; // p2

		matrix mat = translationBuffer[ftemp1.w];
		ftemp.xyz = mul(float4(ftemp.xyz, 1.0f), mat).xyz;
		ftemp1.xyz = mul(float4(ftemp1.xyz, 1.0f), mat).xyz;
		ftemp2.xyz = mul(float4(ftemp2.xyz, 1.0f), mat).xyz;

		//float3 ftemp = float3(-10.0f, -100.0f, 0.0f);// asfloat(texTriangleData.Load4(threadID * 16)).xyz; // p0
		//float3 ftemp1 = float3(-10.0f, 100.0f, 0.0f);// asfloat(texTriangleData.Load4(threadID * 16 + g_numTexTriangles * 16)).xyz; // p1
		//float3 ftemp2 = float3(10.0f, 0.0f, 0.0f);// asfloat(texTriangleData.Load4(threadID * 16 + g_numTexTriangles * 32)).xyz; // p2
		if (RayTriangleIntersectNoFaceCullMinData(g_MouseRay, ftemp, ftemp1.xyz, ftemp2, tt, p))
		{
			
			if (tt < localInfo.t)
			{
				localInfo.t = tt;
				localInfo.type = 2;
				localInfo.ID = threadID;
				localInfo.pos = p;
			}
		}
	}

	if (threadID < g_numSpheres)
	{
		float tt;
		float3 p;
		float4 ftemp = asfloat(sphereData.Load4(threadID.x * 16)); // pos_radius

		if (RaySphereIntersect(g_MouseRay, ftemp.xyz, ftemp.w, tt,p ))
		{
			if (tt < localInfo.t)
			{
				localInfo.t = tt;
				localInfo.type = 0;
				localInfo.ID = threadID;
				localInfo.pos = p;
			}
		}

	}

	if (threadID < g_numTriangles)
	{
		float tt;
		float3 p;
		float3 ftemp = asfloat(triangleData.Load4(threadID * 16)).xyz; // p0
		float3 ftemp1 = asfloat(triangleData.Load4(threadID * 16 + MAX_PRIMITIVES * 16)).xyz; // p1
		float3 ftemp2 = asfloat(triangleData.Load4(threadID * 16 + MAX_PRIMITIVES * 32)).xyz; // p2

		if (RayTriangleIntersectNoFaceCullMinData(g_MouseRay, ftemp, ftemp1, ftemp2, tt, p))
		{
			if (tt < localInfo.t)
			{
				localInfo.t = tt;
				localInfo.type = 1;
				localInfo.ID = threadID;
				localInfo.pos = p;
			}
		}

	}


	pickingResult[threadID] = localInfo;

}
