#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GraphAuthoringLibrary.generated.h"

class UEdGraph;
class UEdGraphNode;

/**
 * 블루프린트 그래프(이벤트 그래프·함수·애님 그래프)를 코드로 만드는 편집기 도우미.
 * 엔진 Python API에는 노드를 만들고 잇는 함수가 없어서, Python 원격 실행(Tools/ue_remote.py)이 이 함수들을 부른다.
 * 노드를 만들거나 바꾸면 블루프린트를 구조 변경으로 표시한다. 컴파일과 저장은 호출하는 쪽이 한다.
 * 핀 이름은 내부 이름(PinName)이나 표시 이름 어느 쪽이든 받는다(대소문자 무시). 모르면 DescribePins로 본다.
 */
UCLASS()
class FIRSTVREDITOR_API UGraphAuthoringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 노드 클래스만으로 만드는 노드(Branch, 애님 그래프 노드 등). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddNode(UEdGraph* Graph, TSubclassOf<UEdGraphNode> NodeClass, int32 X, int32 Y);

	/** 함수 호출 노드. FunctionOwner는 함수를 선언한 클래스(예: KismetMathLibrary, SkinnedMeshComponent). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddCallFunctionNode(UEdGraph* Graph, UClass* FunctionOwner, FName FunctionName, int32 X, int32 Y);

	/** 변수 Get/Set 노드. OwnerClass가 비었거나 그래프의 블루프린트 자신이면 자기 변수(컴포넌트 포함), 아니면 그 클래스의 변수(Target 핀이 생긴다). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddVariableNode(UEdGraph* Graph, FName VariableName, bool bSetter, UClass* OwnerClass, int32 X, int32 Y);

	/** Cast To 노드. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddCastNode(UEdGraph* Graph, UClass* TargetClass, int32 X, int32 Y);

	/** 재정의 이벤트 노드(예: Actor의 ReceiveBeginPlay, AnimInstance의 BlueprintUpdateAnimation). 이미 있으면 그 노드를 켜서 돌려준다. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddEventNode(UEdGraph* Graph, UClass* EventOwner, FName EventName, int32 X, int32 Y);

	/** 컴포넌트 델리게이트 이벤트 노드(디테일 패널 이벤트 +와 같음). 물려받은 컴포넌트도 된다. 이미 있으면 그 노드를 돌려준다. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static UEdGraphNode* AddComponentEventNode(UEdGraph* Graph, FName ComponentName, FName DelegateName, int32 X, int32 Y);

	/** 두 핀을 잇는다. 스키마가 허용하지 않으면 false(이유는 로그). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool LinkPins(UEdGraphNode* FromNode, const FString& FromPin, UEdGraphNode* ToNode, const FString& ToPin);

	/** 핀의 연결을 모두 끊는다. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool BreakPinLinks(UEdGraphNode* Node, const FString& PinName);

	/** 핀 기본값(숫자·이름·열거형·구조체 텍스트). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool SetPinDefault(UEdGraphNode* Node, const FString& PinName, const FString& Value);

	/** 핀 기본 객체(클래스·에셋). */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool SetPinDefaultObject(UEdGraphNode* Node, const FString& PinName, UObject* Value);

	/** 노드의 핀 목록: "방향 이름 (표시 이름) : 타입 = 기본값 -> 연결 수". */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static TArray<FString> DescribePins(UEdGraphNode* Node);

	/** 노드 한 줄 설명: "제목 [클래스] @(X, Y)". */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static FString DescribeNode(UEdGraphNode* Node);

	/** 그래프의 노드들. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static TArray<UEdGraphNode*> GetNodes(UEdGraph* Graph);

	/** 노드를 지운다. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool RemoveNode(UEdGraphNode* Node);

	/**
	 * 점으로 이은 경로의 속성 값을 텍스트로 넣는다(예: 애님 노드의 "Node.BoneToModify.BoneName").
	 * Python에 노출되지 않는 구조체 안쪽 값을 바꿀 때 쓴다. 그래프 노드면 노드를 다시 만들고 블루프린트를 변경으로 표시한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static bool SetPropertyByPath(UObject* Object, const FString& Path, const FString& Value);

	/** 점으로 이은 경로의 속성 값을 텍스트로 읽는다. 없으면 빈 문자열. */
	UFUNCTION(BlueprintCallable, Category = "FirstVR|GraphAuthoring")
	static FString GetPropertyByPath(UObject* Object, const FString& Path);
};
