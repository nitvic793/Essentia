#include "Interface.h"
#include "Serialization.h"

#include <cereal/cereal.hpp>

void Save(const TransformRef& transform, const char* fname)
{
	{
		TransformInterface transformInt;
		transformInt.Position = *transform.Position;
		transformInt.Scale = *transform.Scale;
		transformInt.Rotation = *transform.Rotation;
		std::ofstream file(fname);
		cereal::JSONOutputArchive archive(file);
		archive(transformInt);
	}
}

template<class Archive>
void serialize(Archive& archive, Vector3& vector)
{
	archive(vector.X, vector.Y, vector.Z);
}

template<class Archive>
void serialize(Archive& archive, TransformInterface& transform)
{
	archive(transform.Position, transform.Rotation, transform.Scale);
}