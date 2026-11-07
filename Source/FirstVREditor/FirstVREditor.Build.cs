using UnrealBuildTool;

// 편집기 전용 모듈. Claude가 Python 원격 실행으로 블루프린트 그래프를 만들 때 쓰는 도우미가 들어 있다.
// 게임(배포 빌드)에는 들어가지 않는다.
public class FirstVREditor : ModuleRules
{
	public FirstVREditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "BlueprintGraph", "Kismet" });
	}
}
