#include "GeometryFactory.h"


void GeometryFactory::SetContext(GraphicsContext* _pContext)
{
    
}

GeometryFactory* GeometryFactory::Instance()
{
	if (g_instance == nullptr)
		g_instance = new GeometryFactory();
	return g_instance;
}

GeometryFactory::GeometryFactory()
{
    
}

#pragma region HELPERS

std::vector<Vertex> GeometryFactory::CalculateNormalsAndTangentsSeamless(std::vector<Vertex>& _vertices,
    std::vector<Index>& _indices)
{
	struct AccumData
     {
	    glm::vec3 normal = {0.0f, 0.0f, 0.0f };
		glm::vec3 tangent = {0.0f, 0.0f, 0.0f };
     };

	static constexpr float MERGE_EPSILON = 1e-5f;
	
	struct Vec3Hash {
	 	size_t operator()(const glm::vec3& v) const {
	 		auto quantize = [](float f) { return static_cast<int>(std::round(f / MERGE_EPSILON)); };
	 		size_t h1 = std::hash<int>{}(quantize(v.x));
	 		size_t h2 = std::hash<int>{}(quantize(v.y));
	 		size_t h3 = std::hash<int>{}(quantize(v.z));
	 		return h1 ^ (h2 << 16) ^ (h3 << 32);
	 	}
	};

	struct Vec3Eq
	{
	 	bool operator()(const glm::vec3& a, const glm::vec3& b) const {
	 		return std::abs(a.x - b.x) <= MERGE_EPSILON &&
	 			   std::abs(a.y - b.y) <= MERGE_EPSILON &&
	 			   std::abs(a.z - b.z) <= MERGE_EPSILON;
	 	}
	};

	std::unordered_map<glm::vec3, AccumData, Vec3Hash, Vec3Eq> accum;

	uint32_t numTriangle = (uint32_t)_indices.size() / 3;

	for (uint32_t i = 0; i < numTriangle; i++)
	{
	    uint32_t i0 = _indices[i * 3 + 0];
	    uint32_t i1 = _indices[i * 3 + 1];
	    uint32_t i2 = _indices[i * 3 + 2];

	    Vertex& vertex0 = _vertices[i0];
	    Vertex& vertex1 = _vertices[i1];
	    Vertex& vertex2 = _vertices[i2];

	    glm::vec3 e0 = vertex1.position - vertex0.position;
	    glm::vec3 e1 = vertex2.position - vertex0.position;

	    glm::vec3 normal = glm::cross(e0, e1);

	    glm::vec2 deltaUV1 = { vertex1.uv.x - vertex0.uv.x, vertex1.uv.y - vertex0.uv.y };
	    glm::vec2 deltaUV2 = { vertex2.uv.x - vertex0.uv.x, vertex2.uv.y - vertex0.uv.y };

	    glm::vec3 tangent;
	    float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	    if (std::abs(denom) >= 1e-6f)
	    {
	        float f = 1.0f / denom;
	        glm::vec3 t;
	        tangent.x = f * (deltaUV2.y * e0.x - deltaUV1.y * e1.x);
	        tangent.y = f * (deltaUV2.y * e0.y - deltaUV1.y * e1.y);
	        tangent.z = f * (deltaUV2.y * e0.z - deltaUV1.y * e1.z);
	    }

	    // Accumulate directly into the map for all 3 vertices
	    for (uint32_t idx : { i0, i1, i2 })
	    {
	        AccumData& data = accum[_vertices[idx].position];
	        data.normal += normal;
	        data.tangent += tangent;
	    }
	}

	for (uint32_t i = 0; i < _vertices.size(); i++)
	{
	    AccumData& data = accum[_vertices[i].position];

	    glm::vec3 N = glm::normalize(data.normal);
	    glm::vec3 T = data.tangent;
		
	 	T = T - (glm::dot(T, N) * N);
	 	T = glm::normalize(T);

	    _vertices[i].normal  = N;
	    _vertices[i].tangent = T;
	}

	return _vertices;
}

std::vector<Vertex> GeometryFactory::CalculateNormalsAndTangents(std::vector<Vertex>& _vertices,
	const std::vector<uint32_t>& _indices)
{
	// 1. Utiliser un vecteur simple au lieu d'une map (beaucoup plus rapide)
	struct Accum { glm::vec3 n = {0.0f, 0.0f, 0.0f}; glm::vec3 t = {0.0f, 0.0f, 0.0f}; };
	std::vector<Accum> accum(_vertices.size());
	
	// 2. Premier passage : Accumulation par face
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		uint32_t i0 = _indices[i], i1 = _indices[i+1], i2 = _indices[i+2];
		Vertex& v0 = _vertices[i0]; Vertex& v1 = _vertices[i1]; Vertex& v2 = _vertices[i2];
	
		glm::vec3 p0 = v0.position, p1 = v1.position, p2 = v2.position;
		glm::vec3 e1 = p1 - p0, e2 = p2 - p0;
	
		// Normale de la face (non normalisée pour pondérer par l'aire de la face)
		glm::vec3 faceNormal = glm::cross(e1, e2);
	
		// Tangente
		float du1 = v1.uv.x - v0.uv.x, dv1 = v1.uv.y - v0.uv.y;
		float du2 = v2.uv.x - v0.uv.x, dv2 = v2.uv.y - v0.uv.y;
		float f = 1.0f / (du1 * dv2 - du2 * dv1);
	
		glm::vec3 faceTangent = f * (dv2 * e1 - dv1 * e2);
	
		for (uint32_t idx : {i0, i1, i2}) {
			accum[idx].n += faceNormal;
			accum[idx].t += faceTangent;
		}
	}
	
	// 3. Second passage : Orthogonalisation et stockage
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 N = glm::normalize(accum[i].n);
		glm::vec3 T = accum[i].t;
	
		// Gram-Schmidt : T' = Normalize(T - (N.T) * N)
		T = glm::normalize(T - glm::dot(T, N) * N);
	
		_vertices[i].normal = N;
		// On stocke souvent la tangente en float4 (x, y, z, w) pour la bitangente
		_vertices[i].tangent = T; 
	}
	
	return _vertices;
}

Vertex GeometryFactory::MidPoint(Vertex const& v0, Vertex const& v1)
{
	glm::vec3 pos;
	pos.x = ( v0.position.x + v1.position.x ) * 0.5f;
	pos.y = ( v0.position.y + v1.position.y ) * 0.5f;
	pos.z = ( v0.position.z + v1.position.z ) * 0.5f;

	glm::vec2 uv;
	uv.x = (v0.uv.x + v1.uv.x) * 0.5f;
	uv.y = (v0.uv.y + v1.uv.y) * 0.5f;

	Vertex v = { pos, uv };

	return v;
}

void GeometryFactory::Subdivide(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
{
	std::vector<uint32_t> indicesCopy = indices;
	std::vector<Vertex> verticesCopy = vertices;

	vertices.resize(0);
	indices.resize(0);

	uint32_t numTris = static_cast<uint32_t>(indicesCopy.size()) / 3;
	for (uint32_t i = 0; i < numTris; ++i)
	{
		Vertex v0 = verticesCopy[indicesCopy[i * 3 + 0]];
		Vertex v1 = verticesCopy[indicesCopy[i * 3 + 1]];
		Vertex v2 = verticesCopy[indicesCopy[i * 3 + 2]];

		Vertex m0 = MidPoint(v0, v1);
		Vertex m1 = MidPoint(v1, v2);
		Vertex m2 = MidPoint(v0, v2);
			
		vertices.push_back(v0); // 0
		vertices.push_back(v1); // 1
		vertices.push_back(v2); // 2
		vertices.push_back(m0); // 3
		vertices.push_back(m1); // 4
		vertices.push_back(m2); // 5

		indices.push_back(i * 6 + 0);
		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 5);

		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 4);
		indices.push_back(i * 6 + 5);

		indices.push_back(i * 6 + 5);
		indices.push_back(i * 6 + 4);
		indices.push_back(i * 6 + 2);

		indices.push_back(i * 6 + 3);
		indices.push_back(i * 6 + 1);
		indices.push_back(i * 6 + 4);
	}
}

void GeometryFactory::Circle(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int _stackCount,
	float _posY, bool _isTop)
{
	vertices.push_back(
		Vertex{ glm::vec3(0.0f, _posY, 0.0f), glm::vec2(0.5f, 0.5f) }
	);

	int indexCenter = (UINT)vertices.size() - 1;
	float angle = MathUtils::PI / (float)_stackCount * 2.0f;
    	
	for (int i = 0; i < _stackCount + 1; ++i)
	{
		float c = cosf(angle * (float)i);
		float s = sinf(angle * (float)i);
		float posX = c * 0.5f;
		float posZ = s * 0.5f;

		glm::vec2 uv = { 0.5f + c * 0.5f, 0.5f + s * 0.5f };
		vertices.push_back(Vertex{ glm::vec3(posX, _posY, posZ), uv });
	}
	for (int i = indexCenter; i < indexCenter + _stackCount; ++i)
	{
		indices.push_back(indexCenter);
		if (_isTop)
		{
			indices.push_back(i + 2);
			indices.push_back(i + 1);
		}
		else
		{
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}
	}
}

#pragma endregion
