#include "Enemy/EnemyDataAsset.h"

float UEnemyDataAsset::GetBoneMultiplier(FName BoneName) const
{
	const float* Multiplier = BoneMultipliers.Find(BoneName);
	return Multiplier ? *Multiplier : 1.f;
}

bool UEnemyDataAsset::IsLegBone(FName BoneName) const
{
	return LegBones.Contains(BoneName);
}
