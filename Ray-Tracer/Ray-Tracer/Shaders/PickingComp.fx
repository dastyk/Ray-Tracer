
cbuffer PickingData : register(b2)
{
	uint g_numPrimitives;
	uint g_offset; // Padding to get a multiple of 2.
}


struct PickingInfo
{
	float3 pos;
	float t;
	uint type;
	int ID;
};

StructuredBuffer<PickingInfo> pickingResultIn : register(t5);
RWStructuredBuffer<PickingInfo> pickingResultOut : register(u0);

[numthreads(1024, 1, 1)]
void main(uint threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, uint groupID : SV_GroupID)
{
	// We assume that we always compare over at lease 64 primitives.
	if (threadID < g_numPrimitives) // num is a multiple of 2. and halv the size of all primitives.
	{
		PickingInfo first = pickingResultIn[threadID + g_offset];
		PickingInfo sec = pickingResultIn[threadID + g_numPrimitives + g_offset]; // will always be < g_numPrimitives*2


		if (first.t < sec.t)
			pickingResultOut[threadID + g_offset] = first;
		else
			pickingResultOut[threadID + g_offset] = sec;
	}

	


}
