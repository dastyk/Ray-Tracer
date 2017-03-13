RWTexture2D<float4> output : register(u0);
#define M_PI 3.14159265358979323846f
#define D_PI 0.31830988618f
#define MAX_PRIMITIVES 128
#define MAX_LIGHTS 16
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
	uint g_numTexTriangles;
	uint g_numSpotLights;
}

ByteAddressBuffer sphereData : register(t0);
ByteAddressBuffer triangleData: register(t1);
ByteAddressBuffer pointLightData: register(t2);
ByteAddressBuffer texTriangleData: register(t3);
ByteAddressBuffer spotLightData: register(t4);
StructuredBuffer<matrix> translationBuffer: register(t5);
// t6 is for picking
Texture2DArray<float4> textures : register(t7);
Texture2DArray<float4> normals : register(t8);


SamplerState texSampler : register(s0);

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


bool RaySphereIntersect(Ray r, float3 center, float radius, out float t)
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
	return true;
}

bool RayTriangleIntersectBackFaceCullTexture(Ray ray, float3 p0, float3 p1, float3 p2, out float t, out float3 p, out float3 normal, out float u, out float v, out float3 e1, out float3 e2)
{
	e1 = p1 - p0;
	e2 = p2 - p0;

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



//groupshared float3 Color[MAX_PRIMITIVES];
groupshared float4 TempCache[MAX_PRIMITIVES];
groupshared float4 TempCache1[MAX_PRIMITIVES];
groupshared float4 TempCache2[MAX_PRIMITIVES];
groupshared float4 TempCache3[MAX_PRIMITIVES];
groupshared float2 TempCache4[MAX_PRIMITIVES];
groupshared float4 TempCache5[MAX_LIGHTS];


#define NUM_BOUNCES 3
#define TEX_STRIDE 128
[numthreads(16, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint minTexNum = min(g_numTexTriangles, TEX_STRIDE);
	float4 ftemp;
	float4 ftemp1;
	float4 ftemp2;
	float4 ftemp3;

	float2 f2temp;
	float2 f2temp1;
	float2 f2temp2;
	uint i, li, n, set;

	// Prefetch tex tri data
	if (groupIndex < minTexNum)
	{
		ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
		ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
		ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2

		f2temp = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 48)); // t0
		f2temp1 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 56)); // t1
		f2temp2 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 64)); // t2
	}




	// Setup first ray
	Ray ray;
	float u = (2 * ((threadID.x + 0.5) / g_screenWidth) - 1) * tan(g_fov / 2 * M_PI / 180) * g_aspect;
	float v = (1 - 2 * ((threadID.y + 0.5) / g_screenHeight)) * tan(g_fov / 2 * M_PI / 180);

	float4 p1 = float4(u*g_NearP, v*g_NearP, g_NearP, 1.0f);
	float4 p2 = float4(u*g_FarP, v*g_FarP, g_FarP, 1.0f);

	p1 = mul(p1, g_mViewInv);
	p2 = mul(p2, g_mViewInv);

	ray.Origin = g_CameraPosition;
	ray.Dir = normalize(p2 - p1).xyz;

	uint numHits = 0;
	float4 hitPos[NUM_BOUNCES];
	float4 hitNormal[NUM_BOUNCES];
	float4 hitColor[NUM_BOUNCES];


	///////////////////////
	/*Do intersection*/
	///////////////////////
	float t = 3.402823466e+38F;
	float3 p;
	uint numSets = (g_numTexTriangles / (TEX_STRIDE + 1)) + 1;
	uint overflow = g_numTexTriangles % TEX_STRIDE;
	float tt = 0.0f;
	float3 pp;
	float3 normal;
	uint numTexTri;
	uint nNumTexTri;
	uint nset;
	bool hit;

	[unroll(NUM_BOUNCES)]
	for (i = 0; i < NUM_BOUNCES; i++)
	{
		hitNormal[i] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}


	// Start checking for intersections
	// NOTE: We have to do the loop for all threads in a group, otherwise if one hit and one not we might not load all the primitives.
	for (n = 0; n < NUM_BOUNCES; n++)
	{
		hit = false;

		// Intersection with textured triangles.
		for (set = 0; set < numSets; set++)
		{
			numTexTri = min((numSets - set - 1)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in this set
			nNumTexTri = min((numSets - set - 2)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in next set
			nset = set + 1;


			// Share the prefetched data with group.
			if (groupIndex < numTexTri)
			{

				matrix mat = translationBuffer[ftemp1.w];
				ftemp.xyz = mul(float4(ftemp.xyz, 1.0f), mat).xyz;
				ftemp1.xyz = mul(float4(ftemp1.xyz, 1.0f), mat).xyz;
				ftemp2.xyz = mul(float4(ftemp2.xyz, 1.0f), mat).xyz;

				TempCache[groupIndex] = ftemp;
				TempCache1[groupIndex] = ftemp1;
				TempCache2[groupIndex] = ftemp2;

				TempCache3[groupIndex] = float4(f2temp, f2temp1);
				TempCache4[groupIndex] = f2temp2;
			}
			GroupMemoryBarrierWithGroupSync();

			if (nset == numSets) // If this is the last set
			{
				if (groupIndex < g_numSpheres)
				{
					// Prefetch spheres
					ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
					ftemp1 = asfloat(sphereData.Load4(groupIndex * 16 + 16 * MAX_PRIMITIVES)); // Color
				}
			}
			else
			{
				// Prefetch next set of tex tri data
				if (groupIndex < nNumTexTri)
				{
					ftemp = asfloat(texTriangleData.Load4(groupIndex * 16 + TEX_STRIDE*16 * nset)); // p0
					ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16 + TEX_STRIDE * 16 * nset)); // p1
					ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32 + TEX_STRIDE * 16 * nset)); // p2

					f2temp = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 48 + TEX_STRIDE * 8 * nset)); // t0
					f2temp1 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 56 + TEX_STRIDE * 8 * nset)); // t1
					f2temp2 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 64 + TEX_STRIDE * 8 * nset)); // t2
				}
			}

			// Only do calc if previous iteration was a hit.
			if (numHits == n)
			{
				for (i = 0; i < numTexTri; i++)
				{
					float3 e1;
					float3 e2;
					if (RayTriangleIntersectBackFaceCullTexture(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt, pp, normal, u, v, e1, e2))
					{
						if (tt < t)
						{
							hit = true;
							t = tt;
							hitPos[n] = float4(pp, tt);
							

							float3 texC = float3(TempCache3[i].xy * (1 - u - v) + TempCache3[i].zw* u + TempCache4[i] * v, TempCache[i].w);
							hitColor[n] = float4(textures.SampleLevel(texSampler, texC, 0).rgb, 0.1f);
							hitColor[n].xyz = pow(abs(hitColor[n].xyz), 1.3f);

							float2 UV1 = TempCache3[i].zw - TempCache3[i].xy;
							float2 UV2 =  TempCache4[i] - TempCache3[i].xy;

							float r = 1.0f / (UV1.x*UV2.y - UV1.y * UV2.x);
							float3 tangent = normalize((e1*UV2.y - e2*UV1.y)*r);
							float3 bitangent = normalize((e2*UV1.x - e1*UV2.x)*r);

							float3x3 TBN = float3x3(tangent, bitangent, normal);
							float3 bumpNormal = normals.SampleLevel(texSampler, texC, 0).rgb;

							bumpNormal = normalize(bumpNormal * 2.0f - 1.0f);

							//hitColor[n] = float4(mul(bumpNormal, TBN), 0.1f);
							hitNormal[n] = float4(mul(bumpNormal, TBN), 0.0f);
						}

					}
				}
			}
			GroupMemoryBarrierWithGroupSync(); // Wait for all groupmembers to finished before we go again.(If not we might change tempcache before one thread isn't done.
		}

		// Copy spheres into shared mem
		if (groupIndex < g_numSpheres)
		{
			TempCache[groupIndex] = ftemp;
			TempCache1[groupIndex] = ftemp1;
		}

		GroupMemoryBarrierWithGroupSync();

		// Prefetch triangle data
		if (groupIndex < g_numTriangles)
		{
			ftemp = asfloat(triangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 16)); // p1
			ftemp2 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 32)); // p2
			ftemp3 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 48)); // Color
		}

		// Only if previous it was a hit
		if (numHits == n)
		{
			// Check intersect with spheres
			for (i = 0; i < g_numSpheres; i++)
			{			
				if (RaySphereIntersect(ray, TempCache[i].xyz, TempCache[i].w, tt, pp, normal))
				{
					if (tt < t)
					{
						hit = true;
						t = tt;
						hitPos[n] = float4(pp, tt);
						hitNormal[n] = float4(normal, 0.0f);
						hitColor[n] = float4(TempCache1[i].xyz, 0.1f);
					}
				}
			}
		}

		GroupMemoryBarrierWithGroupSync();

		// Load triangles into shared memory
		if (groupIndex < g_numTriangles)
		{
			TempCache[groupIndex] = ftemp;
			TempCache1[groupIndex] = ftemp1;
			TempCache2[groupIndex] = ftemp2;
			TempCache3[groupIndex] = ftemp3;


		}

		GroupMemoryBarrierWithGroupSync();

		if (n + 1 == NUM_BOUNCES) //  If this is the last possible bounce, we prefetch point light data.
		{
			if (groupIndex < g_numPointLights)
				ftemp = asfloat(pointLightData.Load4(groupIndex * 16)); // Prefetch the pointlight data.

		}
		else if (groupIndex < minTexNum) // Prefetch textured triangles again.
		{
			ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
			ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2

			f2temp = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 48)); // t0
			f2temp1 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 56)); // t1
			f2temp2 = asfloat(texTriangleData.Load2(groupIndex * 8 + g_numTexTriangles * 64)); // t2
		}

		// only if prev it hit
		if (numHits == n)
		{
			for (i = 0; i < g_numTriangles; i++)
			{
				if (RayTriangleIntersectBackFaceCull(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt, pp, normal))
				{
					if (tt < t)
					{
						hit = true;
						t = tt;
						hitPos[n] = float4(pp, tt);
						hitNormal[n] = float4(normal, 0.0f);
						hitColor[n] = float4(TempCache3[i].xyz, 0.1f);
					}

				}
			}

		}


		// Setup new ray
		if (hit)
		{
			ray.Origin = hitPos[n].xyz;
			t = 3.402823466e+38F;
			ray.Dir = reflect(ray.Dir, hitNormal[n].xyz);
			ray.Origin += ray.Dir*0.001;
		
			numHits++;

		}


		GroupMemoryBarrierWithGroupSync();


	}


	//////////////////////
	// Color stuff
	///////////////////////

	bool LightNotBlocked[MAX_LIGHTS]; // Used to remember if a light is blocked by a primitive.

	for (i = 0; i < g_numPointLights; i++) // Init the array
		LightNotBlocked[i] = true;

	if (groupIndex < g_numPointLights)
		TempCache3[groupIndex] = ftemp; // Load the pointlight data.

	// Prefetch tex tri data
	if (groupIndex < minTexNum)
	{
		ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
		ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
		ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2
	}

	p = g_CameraPosition; // Used at the end of the loop.

	// Start checking if a point light is blocked, if not do light calcs.
	for ( n = 0; n < NUM_BOUNCES; n++)
	{
		ray.Origin = hitPos[n].xyz;
		ray.Origin += hitNormal[n].xyz*0.001f;

		// Check to see if any textured triangles are blocking any light
		for (set = 0; set < numSets; set++)
		{
			numTexTri = min((numSets - set - 1)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in this set
			nNumTexTri = min((numSets - set - 2)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in next set
			nset = set + 1;


			// Share the prefetched data with group.
			if (groupIndex < numTexTri)
			{
				matrix mat = translationBuffer[ftemp1.w];
				ftemp.xyz = mul(float4(ftemp.xyz, 1.0f), mat).xyz;
				ftemp1.xyz = mul(float4(ftemp1.xyz, 1.0f), mat).xyz;
				ftemp2.xyz = mul(float4(ftemp2.xyz, 1.0f), mat).xyz;


				TempCache[groupIndex] = ftemp;
				TempCache1[groupIndex] = ftemp1;
				TempCache2[groupIndex] = ftemp2;
			}
			GroupMemoryBarrierWithGroupSync();

			if (nset == numSets)
			{
				if (groupIndex < g_numSpheres)
				{
					// Prefetch spheres
					ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
				}
			}
			else
			{
				// Prefetch next set of tex tri data
				if (groupIndex < nNumTexTri)
				{
					ftemp = asfloat(texTriangleData.Load4(groupIndex * 16 + TEX_STRIDE * 16 * nset)); // p0
					ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16 + TEX_STRIDE * 16 * nset)); // p1
					ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32 + TEX_STRIDE * 16 * nset)); // p2
				}
			}

			if (n < numHits)
			{
				for (li = 0; li < g_numPointLights; li++)
				{
					ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
					t = length(ray.Dir);
					ray.Dir = normalize(ray.Dir);

					u = dot(hitNormal[n].xyz, ray.Dir);

					if (u > 0.0f)
					{
						for (i = 0; i < numTexTri; i++)
						{
							if (RayTriangleIntersectFrontFaceCullNoData(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt))
							{
								if (tt < t)
									LightNotBlocked[li] = false; // The light is blocked.
							}
						}
					}





				}
			}
			GroupMemoryBarrierWithGroupSync();
		}




		if (groupIndex < g_numSpheres)
		{
			TempCache[groupIndex] = ftemp;
		}

		GroupMemoryBarrierWithGroupSync();


		if (groupIndex < g_numTriangles)
		{
			ftemp = asfloat(triangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 16)); // p1
			ftemp2 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 32)); // p2
		}

		 
		// Check if any spheres block any of the lights
		if (n < numHits)
		{

			for ( li = 0; li < g_numPointLights; li++)
			{

				ray.Dir = TempCache3[li].xyz - hitPos[n].xyz;
				t = length(ray.Dir);
				ray.Dir = normalize(ray.Dir);
				u = dot(hitNormal[n].xyz, ray.Dir);

				if (u > 0.0f)
				{

					for (i = 0; i < g_numSpheres; i++)
					{

						if (RaySphereIntersect(ray, TempCache[i].xyz, TempCache[i].w, tt))
						{
							if (tt < t)
								LightNotBlocked[li] = false;
						}
					}



				}




			}
		}


		GroupMemoryBarrierWithGroupSync();

		// Load triangles into shared memory
		if (groupIndex < g_numTriangles)
		{
			TempCache[groupIndex] = ftemp;
			TempCache1[groupIndex] = ftemp1;
			TempCache2[groupIndex] = ftemp2;
		}

		GroupMemoryBarrierWithGroupSync();

		if (n + 1 == NUM_BOUNCES)
		{
			if (groupIndex < g_numSpotLights)
			{
				ftemp = asfloat(spotLightData.Load4(groupIndex * 16)); // Pos_lum
				ftemp1 = asfloat(spotLightData.Load4(groupIndex * 16 + MAX_LIGHTS * 16)); // Dir_radius
				f2temp = asfloat(spotLightData.Load2(groupIndex * 8 + MAX_LIGHTS * 32)); // theta_phi
			}
		}
		// Prefetch tex tri data
		else if (groupIndex < minTexNum)
		{
			ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
			ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2
		}


		// Check the triangles
		if (n < numHits)
		{

			for ( li = 0; li < g_numPointLights; li++)
			{
				ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
				t = length(ray.Dir);
				ray.Dir = normalize(ray.Dir);

				u = dot(hitNormal[n].xyz, ray.Dir);

				if (u > 0.0f)
				{
					for (i = 0; i < g_numTriangles; i++)
					{
						if (RayTriangleIntersectFrontFaceCullNoData(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt))
						{
							if(tt < t)
								LightNotBlocked[li] = false;
						}
					}
				}





			}


		}

		// If the light was not blocked, calculate the light attenuation and sum it up.
		if (n < numHits)
		{
			for (li = 0; li < g_numPointLights; li++)
			{
				if (LightNotBlocked[li])
				{



					pp = normalize(TempCache3[li].xyz - hitPos[n].xyz);
					u = dot(hitNormal[n].xyz, pp);

					if (u > 0.0f)
					{

						normal = normalize(p - hitPos[n].xyz);
						normal = normalize(normal + pp);
						v = pow(dot(hitNormal[n].xyz, normal), 257); // Spec

						hitColor[n].w += TempCache3[li].w*(u + v);


					}
				}


			}
		}



		p = hitPos[n].xyz; // Set the current pos as camera pos.(This is so we can do specular lighting for all bounces.





		GroupMemoryBarrierWithGroupSync();

	}

	// Share the spotlights with group
	if (groupIndex < g_numSpotLights)
	{
		TempCache3[groupIndex] = ftemp;
		TempCache5[groupIndex] = ftemp1;
		TempCache4[groupIndex] = f2temp;
	}

	GroupMemoryBarrierWithGroupSync();

	for (i = 0; i < g_numSpotLights; i++) // Init the array
		LightNotBlocked[i] = true;


	// Prefetch tex tri data
	if (groupIndex < minTexNum)
	{
		ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
		ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
		ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2
	}
	p = g_CameraPosition; // Used at the end of the loop.

	// Start spot light calcs.
	for (n = 0; n < NUM_BOUNCES; n++)
	{
		ray.Origin = hitPos[n].xyz;
		ray.Origin += hitNormal[n].xyz*0.001f;

		// Check to see if any textured triangles are blocking any light
		for (set = 0; set < numSets; set++)
		{
			numTexTri = min((numSets - set - 1)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in this set
			nNumTexTri = min((numSets - set - 2)* TEX_STRIDE + overflow, TEX_STRIDE); // Number of triangles in next set
			nset = set + 1;


			// Share the prefetched data with group.
			if (groupIndex < numTexTri)
			{
				matrix mat = translationBuffer[ftemp1.w];
				ftemp.xyz = mul(float4(ftemp.xyz, 1.0f), mat).xyz;
				ftemp1.xyz = mul(float4(ftemp1.xyz, 1.0f), mat).xyz;
				ftemp2.xyz = mul(float4(ftemp2.xyz, 1.0f), mat).xyz;

				TempCache[groupIndex] = ftemp;
				TempCache1[groupIndex] = ftemp1;
				TempCache2[groupIndex] = ftemp2;
			}
			GroupMemoryBarrierWithGroupSync();

			if (nset == numSets)
			{
				if (groupIndex < g_numSpheres)
				{
					// Prefetch spheres
					ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
				}
			}
			else
			{
				// Prefetch next set of tex tri data
				if (groupIndex < nNumTexTri)
				{
					ftemp = asfloat(texTriangleData.Load4(groupIndex * 16 + TEX_STRIDE * 16 * nset)); // p0
					ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16 + TEX_STRIDE * 16 * nset)); // p1
					ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32 + TEX_STRIDE * 16 * nset)); // p2
				}
			}

			if (n < numHits)
			{
				for (li = 0; li < g_numSpotLights; li++)
				{
					ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
					t = length(ray.Dir);
					ray.Dir = normalize(ray.Dir);

					if (t < TempCache5[li].w) // If in range
					{
						u = dot(hitNormal[n].xyz, ray.Dir);

						if (u > 0.0f) // If facing the light
						{
							u = dot(ray.Dir, -TempCache5[li].xyz);

							if (u > TempCache4[li].y) // If inside the outer cone.
							{
								for (i = 0; i < numTexTri; i++)
								{
									if (RayTriangleIntersectFrontFaceCullNoData(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt))
									{
										if (tt < t)
											LightNotBlocked[li] = false; // The light is blocked.
									}
								}
							}
						
							
						}
						

					}
				
					

					





				}
			}
			GroupMemoryBarrierWithGroupSync();
		}




		if (groupIndex < g_numSpheres)
		{
			TempCache[groupIndex] = ftemp;
		}

		GroupMemoryBarrierWithGroupSync();


		if (groupIndex < g_numTriangles)
		{
			ftemp = asfloat(triangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 16)); // p1
			ftemp2 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 32)); // p2
		}


		// Check if any spheres block any of the lights
		if (n < numHits)
		{

			for (li = 0; li < g_numSpotLights; li++)
			{
				ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
				t = length(ray.Dir);
				ray.Dir = normalize(ray.Dir);

				if (t < TempCache5[li].w) // If in range
				{
					u = dot(hitNormal[n].xyz, ray.Dir);

					if (u > 0.0f) // If facing the light
					{
						u = dot(ray.Dir, -TempCache5[li].xyz);

						if (u > TempCache4[li].y) // If inside the outer cone.
						{

							for (i = 0; i < g_numSpheres; i++)
							{

								if (RaySphereIntersect(ray, TempCache[i].xyz, TempCache[i].w, tt))
								{
									if (tt < t)
										LightNotBlocked[li] = false;
								}
							}
						}
						

					}
					

				}
		




			}
		}


		GroupMemoryBarrierWithGroupSync();

		// Load triangles into shared memory
		if (groupIndex < g_numTriangles)
		{
			TempCache[groupIndex] = ftemp;
			TempCache1[groupIndex] = ftemp1;
			TempCache2[groupIndex] = ftemp2;
		}

		GroupMemoryBarrierWithGroupSync();


		// Prefetch tex tri data
		if (groupIndex < minTexNum)
		{
			ftemp = asfloat(texTriangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 16)); // p1
			ftemp2 = asfloat(texTriangleData.Load4(groupIndex * 16 + g_numTexTriangles * 32)); // p2
		}


		 //Check the triangles
		if (n < numHits)
		{

			for (li = 0; li < g_numSpotLights; li++)
			{
				ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
				t = length(ray.Dir);
				ray.Dir = normalize(ray.Dir);

				if (t < TempCache5[li].w) // If in range
				{
					u = dot(hitNormal[n].xyz, ray.Dir);

					if (u > 0.0f) // If facing the light
					{
						u = dot(ray.Dir, -TempCache5[li].xyz);

						if (u > TempCache4[li].y) // If inside the outer cone.
						{
							for (i = 0; i < g_numTriangles; i++)
							{
								if (RayTriangleIntersectFrontFaceCullNoData(ray, TempCache[i].xyz, TempCache1[i].xyz, TempCache2[i].xyz, tt))
								{
									if (tt < t)
										LightNotBlocked[li] = false;
								}
							}
						}
						else
						{
							LightNotBlocked[li] = false;
						}

					}
					else
					{
						LightNotBlocked[li] = false;
					}

				}
				else
				{
					LightNotBlocked[li] = false;
				}




			}


		}


		 //If the light was not blocked, calculate the light attenuation and sum it up.
		if (n < numHits)
		{
			
			for (li = 0; li < g_numSpotLights; li++)
			{
				if (LightNotBlocked[li])
				{
					ray.Dir = (TempCache3[li].xyz - hitPos[n].xyz);
					ray.Dir = normalize(ray.Dir);

					u = dot(hitNormal[n].xyz, ray.Dir);


					v = dot(ray.Dir, -TempCache5[li].xyz);
					if (v > TempCache4[li].x) // If inside the inner cone
					{
						normal = normalize(p - hitPos[n].xyz);
						normal = normalize(normal + ray.Dir);
						t = pow(dot(hitNormal[n].xyz, normal), 257); // Spec

						hitColor[n].w += TempCache3[li].w*(u + t);
					}
					else // If inside the outer cone.
					{
						normal = normalize(p - hitPos[n].xyz);
						normal = normalize(normal + ray.Dir);
						t = pow(dot(hitNormal[n].xyz, normal), 257); // Spec

						//TempCache3[li].w*u + v;
						hitColor[n].w += TempCache3[li].w*(u + t)*(pow((v - TempCache4[li].y) / (TempCache4[li].x - TempCache4[li].y), 3));// TempCache3[li].w*u + v;
					}



				}


			}
		}



		p = hitPos[n].xyz; // Set the current pos as camera pos.(This is so we can do specular lighting for all bounces.





		GroupMemoryBarrierWithGroupSync();

	}


	// Finaly add all the bounces to a final color.
	float3 color = float3(0.0f, 0.0f, 0.0f);
	[unroll(NUM_BOUNCES)]
	for ( n = 0; n < NUM_BOUNCES; n++)
	{
		// Only count if it was an actual hit.
		if (n < numHits)												// This is an attempt at making the surfaces reflect less light per bounce.
			color = color + hitColor[n].xyz * hitColor[n].w *((NUM_BOUNCES - n) / (float)NUM_BOUNCES);//((NUM_BOUNCES - n) / ((float)NUM_BOUNCES*((n*1.0f) + 1)));


	}

	
	//ftemp = spotLightData.Load4(0 * 16 + MAX_LIGHTS * 16);
	//float2 f2temp = asfloat(texTriangleData.Load2(1 * 8 + g_numTexTriangles * 56)); // t0
	
	float exposure = -1.00f;
	color = float3(1.0f,1.0f,1.0f) - exp(color * exposure);



		//float3 diffuseCol = f3tex2D(diffTex, texCoord);
		//fuseCol = diffuseCol * diffuseCol;
	//float4 ftempasd = asfloat(texTriangleData.Load4(0 * 16)); // p0

	output[threadID.xy] = float4(color, 1.0f);// float4(ftemp.x, 0.0f, 0.0f, 1.0f);//

														  //float d = 1 / 800.0f;
														  //	output[threadID.xy] = float4(pointLights[0].Position.x, 0.0f, 0.0f, 1.0f);// float4(g_CameraDir * (1 - length(threadID.xy - float2(400, 400)) / 400.0f), 1);
}
