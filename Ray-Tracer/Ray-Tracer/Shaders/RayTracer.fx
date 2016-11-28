RWTexture2D<float4> output : register(u0);
#define M_PI 3.14159265358979323846f
#define D_PI 0.31830988618f
#define MAX_PRIMITIVES 512
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
}

ByteAddressBuffer sphereData : register(t0);
ByteAddressBuffer triangleData: register(t1);
ByteAddressBuffer pointLightData: register(t2);
ByteAddressBuffer texTriangleData: register(t3);

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
bool RayTriangleIntersectFrontFaceCullNoData(Ray ray, float3 p0, float3 p1, float3 p2)
{
	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;

	float3 normal = normalize(cross(e2, e1));
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
	float t = f*dot(e2, r);
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
	normal = normalize(cross(e2, e1));
	return true;

}
//groupshared float3 Color[MAX_PRIMITIVES];
groupshared float4 TempCache[MAX_PRIMITIVES];
groupshared float4 TempCache1[MAX_PRIMITIVES];
groupshared float4 TempCache2[MAX_PRIMITIVES];
groupshared float4 TempCache3[MAX_PRIMITIVES];
//groupshared float4 TempCache4[MAX_PRIMITIVES];


#define NUM_BOUNCES 1

void Intersections(Ray ray, uint groupIndex, out uint numHits, out float4 hitPos[5], out float4 hitNormal[5], out float4 hitColor[5])
{
	numHits = 0;
	float t = 3.402823466e+38F;
	float3 p;
	uint numSets = g_numTexTriangles / 512 + 1;
	float4 ftemp;
	float4 ftemp1;
	float4 ftemp2;
	float4 ftemp3;


	// Load spheres into shared memory
	if (groupIndex < g_numSpheres)
	{
		ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
		ftemp1 = asfloat(sphereData.Load4(groupIndex * 16 + 16 * MAX_PRIMITIVES)); // Color
	}

	for (uint i = 0; i < NUM_BOUNCES; i++)
	{
		hitNormal[i] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	

	for (uint n = 0; n < NUM_BOUNCES; n++)
	{
		if (groupIndex < g_numSpheres)
		{
			TempCache[groupIndex] = ftemp;
			TempCache1[groupIndex] = ftemp1;
		}

		GroupMemoryBarrierWithGroupSync();


		if (groupIndex < g_numTriangles)
		{
			ftemp = asfloat(triangleData.Load4(groupIndex * 16)); // p0
			ftemp1 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 16)); // p1
			ftemp2 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 32)); // p2
			ftemp3 = asfloat(triangleData.Load4(groupIndex * 16 + MAX_PRIMITIVES * 48)); // Color
		}
		bool hit = false;
		if (numHits == n)
		{
			// Check intersect with spheres
			for (uint i = 0; i < g_numSpheres; i++)
			{

				float tt = 0.0f;
				float3 pp;
				float3 normal;
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

		if (groupIndex < g_numSpheres)
		{
			// Prefetch spheres
			ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
			ftemp1 = asfloat(sphereData.Load4(groupIndex * 16 + 16 * MAX_PRIMITIVES)); // Color
		}
		if (numHits == n)
		{
			for (uint i = 0; i < g_numTriangles; i++)
			{

				float tt = 0.0f;
				float3 pp;
				float3 normal;
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

		ray.Origin = hitPos[n].xyz;
		t = 3.402823466e+38F;
		ray.Dir = reflect(ray.Dir, hitNormal[n].xyz);
		ray.Origin += ray.Dir*0.01;
		if (hit)
		{
			numHits++;
			
		}



		GroupMemoryBarrierWithGroupSync();
		
	}

}



float3 Colorize(uint groupIndex, uint numHits, float4 hitPos[5], float4 hitNormal[5], float4 hitColor[5])
{
	float4 ftemp;
	float4 ftemp1;
	float4 ftemp2;
	bool LightBlock[MAX_PRIMITIVES*NUM_BOUNCES];

	if (groupIndex < g_numPointLights)
		TempCache3[groupIndex] = asfloat(pointLightData.Load4(groupIndex * 16)); // Load the pointlight data.

	if (groupIndex < g_numSpheres)
	{
		ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
	}


	

	for (uint i = 0; i < NUM_BOUNCES; i++)
	{
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

		
		Ray sunRay;
		sunRay.Origin = hitPos[i].xyz;
		sunRay.Origin += hitNormal[i].xyz*0.01f;


		if (i < numHits)
		{
			
			for (uint li = 0; li < g_numPointLights; li++)
			{

				LightBlock[li*MAX_PRIMITIVES + i] = true;

				sunRay.Dir = TempCache3[li].xyz - sunRay.Origin;
				float len = length(sunRay.Dir);
				sunRay.Dir = normalize(sunRay.Dir);
				float a = dot(hitNormal[i].xyz, sunRay.Dir);

				if (a > 0.0f)
				{
					
					for (uint j = 0; j < g_numSpheres; j++)
					{
						float t;
						if (RaySphereIntersect(sunRay, TempCache[j].xyz, TempCache[j].w, t))
						{
							if(t < len)
								LightBlock[li*MAX_PRIMITIVES + i] = false;
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

		if (groupIndex < g_numSpheres)
		{
			// Prefetch spheres
			ftemp = asfloat(sphereData.Load4(groupIndex * 16)); // Pos_radius
		}



		if (i < numHits)
		{

			for (uint li = 0; li < g_numPointLights; li++)
			{
				sunRay.Dir = normalize(TempCache3[li].xyz - sunRay.Origin);
				bool c = true;

				float a = dot(hitNormal[i].xyz, sunRay.Dir);

				if (a > 0.0f)
				{
					for (uint j = 0; j < g_numTriangles; j++)
					{
						if (RayTriangleIntersectFrontFaceCullNoData(sunRay, TempCache[j].xyz, TempCache1[j].xyz, TempCache2[j].xyz))
						{
							LightBlock[li*MAX_PRIMITIVES + i] = false;
						}
					}
				}


					
				

			}


		}

		GroupMemoryBarrierWithGroupSync();
		
	}
	float3 p = g_CameraPosition;
	for (uint i = 0; i < NUM_BOUNCES; i++)
	{
		if (i < numHits)
		{
			for (uint li = 0; li < g_numPointLights; li++)
			{
				if (LightBlock[li*MAX_PRIMITIVES + i])
				{



					float3 d = normalize(TempCache3[li].xyz - hitPos[i].xyz);
					float a = dot(hitNormal[i].xyz, d);

					if (a > 0.0f)
					{

						float3 v = normalize(p - hitPos[i].xyz);
						float3 h = normalize(v + d);
						float spec = pow(dot(hitNormal[i].xyz, h), 257);

						hitColor[i].w += TempCache3[li].w*a +spec;

						
					}
				}


			}
		}

		p = hitPos[i].xyz;
	}
	
	float3 color = float3(0.0f,0.0f,0.0f);
	for (uint i = 0; i < NUM_BOUNCES; i++)
	{
		if (i < numHits)
			color = saturate(color + hitColor[i].xyz * hitColor[i].w * ((NUM_BOUNCES-i)/ ((float)NUM_BOUNCES*((i*3.0f)+1))));


	}

	
	return color;

}

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	// Setup first ray
	Ray ray;
	float dx = (2 * ((threadID.x + 0.5) / g_screenWidth) - 1) * tan(g_fov / 2 * M_PI / 180) * g_aspect;
	float dy = (1 - 2 * ((threadID.y + 0.5) / g_screenHeight)) * tan(g_fov / 2 * M_PI / 180);

	float4 p1 = float4(dx*g_NearP, dy*g_NearP, g_NearP, 1.0f);
	float4 p2 = float4(dx*g_FarP, dy*g_FarP, g_FarP, 1.0f);

	p1 = mul(p1, g_mViewInv);
	p2 = mul(p2, g_mViewInv);

	ray.Origin = g_CameraPosition;
	ray.Dir = normalize(p2 - p1).xyz;

	uint numHits;
	float4 hitPos[5];
	float4 hitNormal[5];
	float4 hitColor[5];

	Intersections(ray, groupIndex, numHits, hitPos, hitNormal, hitColor);


	float3 myColor = Colorize(groupIndex, numHits, hitPos, hitNormal, hitColor);

	output[threadID.xy] = saturate(float4(myColor, 1.0f));

	//float d = 1 / 800.0f;
//	output[threadID.xy] = float4(pointLights[0].Position.x, 0.0f, 0.0f, 1.0f);// float4(g_CameraDir * (1 - length(threadID.xy - float2(400, 400)) / 400.0f), 1);
}
