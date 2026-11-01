#include "Enemy/EnemyDataAsset.h"

float UEnemyDataAsset::GetBoneMultiplier(FName BoneName) const
{
	const float* Multiplier = BoneMultipliers.Find(BoneName);
	return Multiplier ? *Multiplier : 1.f;
}

bool UEnemyDataAsset::IsLegBone(FName BoneName) const
{
	return GetLegSide(BoneName) != ELegSide::None;
}

ELegSide UEnemyDataAsset::GetLegSide(FName BoneName) const
{
	if (LeftLegBones.Contains(BoneName))
	{
		return ELegSide::Left;
	}
	if (RightLegBones.Contains(BoneName))
	{
		return ELegSide::Right;
	}
	return ELegSide::None;
}
