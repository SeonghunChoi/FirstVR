#include "GraphAuthoringLibrary.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "K2Node_ComponentBoundEvent.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(LogGraphAuthoring, Log, All);

namespace
{
	UBlueprint* BlueprintOf(const UEdGraph* Graph)
	{
		return Graph ? FBlueprintEditorUtils::FindBlueprintForGraph(Graph) : nullptr;
	}

	UBlueprint* BlueprintOf(const UEdGraphNode* Node)
	{
		return Node ? BlueprintOf(Node->GetGraph()) : nullptr;
	}

	void MarkModified(UBlueprint* Blueprint)
	{
		if (Blueprint)
		{
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		}
	}

	/** 새 노드 객체를 만든다. 설정(Setup)은 핀을 만들기 전에 해야 하므로 PlaceNode와 나눴다. */
	template <typename T>
	T* CreateNodeObject(UEdGraph* Graph, UClass* NodeClass = T::StaticClass())
	{
		Graph->Modify();
		return NewObject<T>(Graph, NodeClass, NAME_None, RF_Transactional);
	}

	/** 그래프에 넣고 핀을 만든다. 편집기에서 노드를 놓을 때와 같은 순서다. */
	UEdGraphNode* PlaceNode(UEdGraph* Graph, UEdGraphNode* Node, int32 X, int32 Y)
	{
		Graph->AddNode(Node, /*bFromUI*/ false, /*bSelectNewNode*/ false);
		Node->CreateNewGuid();
		Node->PostPlacedNewNode();
		if (Node->Pins.Num() == 0)
		{
			Node->AllocateDefaultPins();
		}
		Node->NodePosX = X;
		Node->NodePosY = Y;
		MarkModified(BlueprintOf(Graph));
		return Node;
	}

	UEdGraphPin* FindPinLoose(UEdGraphNode* Node, const FString& PinName)
	{
		if (!Node)
		{
			return nullptr;
		}
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin && Pin->PinName.ToString().Equals(PinName, ESearchCase::IgnoreCase))
			{
				return Pin;
			}
		}
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin && Node->GetPinDisplayName(Pin).ToString().Equals(PinName, ESearchCase::IgnoreCase))
			{
				return Pin;
			}
		}
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Pin '%s' not found on %s"), *PinName, *Node->GetName());
		return nullptr;
	}

	/** 점으로 이은 경로를 따라가 마지막 속성과 그 값 위치를 찾는다. */
	bool ResolvePath(UObject* Object, const FString& Path, FProperty*& OutProperty, void*& OutValue)
	{
		TArray<FString> Segments;
		Path.ParseIntoArray(Segments, TEXT("."));
		if (!Object || Segments.Num() == 0)
		{
			return false;
		}

		void* Container = Object;
		const UStruct* Struct = Object->GetClass();
		for (int32 Index = 0; Index < Segments.Num(); ++Index)
		{
			FProperty* Property = FindFProperty<FProperty>(Struct, FName(*Segments[Index]));
			if (!Property)
			{
				UE_LOG(LogGraphAuthoring, Warning, TEXT("Property '%s' not found in %s (path %s)"), *Segments[Index], *Struct->GetName(), *Path);
				return false;
			}

			void* Value = Property->ContainerPtrToValuePtr<void>(Container);
			if (Index == Segments.Num() - 1)
			{
				OutProperty = Property;
				OutValue = Value;
				return true;
			}

			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty)
			{
				UE_LOG(LogGraphAuthoring, Warning, TEXT("'%s' is not a struct (path %s)"), *Segments[Index], *Path);
				return false;
			}
			Container = Value;
			Struct = StructProperty->Struct;
		}
		return false;
	}
}

UEdGraphNode* UGraphAuthoringLibrary::AddNode(UEdGraph* Graph, TSubclassOf<UEdGraphNode> NodeClass, int32 X, int32 Y)
{
	if (!Graph || !NodeClass)
	{
		return nullptr;
	}
	return PlaceNode(Graph, CreateNodeObject<UEdGraphNode>(Graph, NodeClass), X, Y);
}

UEdGraphNode* UGraphAuthoringLibrary::AddCallFunctionNode(UEdGraph* Graph, UClass* FunctionOwner, FName FunctionName, int32 X, int32 Y)
{
	const UFunction* Function = FunctionOwner ? FunctionOwner->FindFunctionByName(FunctionName) : nullptr;
	if (!Graph || !Function)
	{
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Function %s::%s not found"), *GetNameSafe(FunctionOwner), *FunctionName.ToString());
		return nullptr;
	}

	UK2Node_CallFunction* Node = CreateNodeObject<UK2Node_CallFunction>(Graph);
	Node->SetFromFunction(Function);
	return PlaceNode(Graph, Node, X, Y);
}

UEdGraphNode* UGraphAuthoringLibrary::AddVariableNode(UEdGraph* Graph, FName VariableName, bool bSetter, UClass* OwnerClass, int32 X, int32 Y)
{
	UBlueprint* Blueprint = BlueprintOf(Graph);
	if (!Blueprint)
	{
		return nullptr;
	}

	const bool bSelf = !OwnerClass || OwnerClass == Blueprint->GeneratedClass || OwnerClass == Blueprint->SkeletonGeneratedClass;
	UK2Node_Variable* Node = bSetter
		? static_cast<UK2Node_Variable*>(CreateNodeObject<UK2Node_VariableSet>(Graph))
		: static_cast<UK2Node_Variable*>(CreateNodeObject<UK2Node_VariableGet>(Graph));
	if (bSelf)
	{
		Node->VariableReference.SetSelfMember(VariableName);
	}
	else
	{
		Node->VariableReference.SetExternalMember(VariableName, OwnerClass);
	}
	return PlaceNode(Graph, Node, X, Y);
}

UEdGraphNode* UGraphAuthoringLibrary::AddCastNode(UEdGraph* Graph, UClass* TargetClass, int32 X, int32 Y)
{
	if (!Graph || !TargetClass)
	{
		return nullptr;
	}
	UK2Node_DynamicCast* Node = CreateNodeObject<UK2Node_DynamicCast>(Graph);
	Node->TargetType = TargetClass;
	return PlaceNode(Graph, Node, X, Y);
}

UEdGraphNode* UGraphAuthoringLibrary::AddEventNode(UEdGraph* Graph, UClass* EventOwner, FName EventName, int32 X, int32 Y)
{
	UBlueprint* Blueprint = BlueprintOf(Graph);
	if (!Blueprint || !EventOwner || !EventOwner->FindFunctionByName(EventName))
	{
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Event %s::%s not found"), *GetNameSafe(EventOwner), *EventName.ToString());
		return nullptr;
	}

	if (UK2Node_Event* Existing = FBlueprintEditorUtils::FindOverrideForFunction(Blueprint, EventOwner, EventName))
	{
		// 새 블루프린트에 회색으로 미리 놓인 이벤트(BeginPlay 등)는 켜서 쓴다.
		if (Existing->IsAutomaticallyPlacedGhostNode())
		{
			Existing->SetEnabledState(ENodeEnabledState::Enabled, false);
		}
		return Existing;
	}

	UK2Node_Event* Node = CreateNodeObject<UK2Node_Event>(Graph);
	Node->EventReference.SetExternalMember(EventName, EventOwner);
	Node->bOverrideFunction = true;
	return PlaceNode(Graph, Node, X, Y);
}

UEdGraphNode* UGraphAuthoringLibrary::AddComponentEventNode(UEdGraph* Graph, FName ComponentName, FName DelegateName, int32 X, int32 Y)
{
	UBlueprint* Blueprint = BlueprintOf(Graph);
	if (!Blueprint)
	{
		return nullptr;
	}

	if (const UK2Node_ComponentBoundEvent* Existing = FKismetEditorUtilities::FindBoundEventForComponent(Blueprint, DelegateName, ComponentName))
	{
		return const_cast<UK2Node_ComponentBoundEvent*>(Existing);
	}

	UClass* BlueprintClass = Blueprint->SkeletonGeneratedClass ? Blueprint->SkeletonGeneratedClass : Blueprint->GeneratedClass;
	const FObjectProperty* ComponentProperty = FindFProperty<FObjectProperty>(BlueprintClass, ComponentName);
	const FMulticastDelegateProperty* DelegateProperty = ComponentProperty
		? FindFProperty<FMulticastDelegateProperty>(ComponentProperty->PropertyClass, DelegateName)
		: nullptr;
	if (!DelegateProperty)
	{
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Component '%s' or delegate '%s' not found in %s"),
			*ComponentName.ToString(), *DelegateName.ToString(), *Blueprint->GetName());
		return nullptr;
	}

	UK2Node_ComponentBoundEvent* Node = CreateNodeObject<UK2Node_ComponentBoundEvent>(Graph);
	Node->InitializeComponentBoundEventParams(ComponentProperty, DelegateProperty);
	return PlaceNode(Graph, Node, X, Y);
}

bool UGraphAuthoringLibrary::LinkPins(UEdGraphNode* FromNode, const FString& FromPin, UEdGraphNode* ToNode, const FString& ToPin)
{
	UEdGraphPin* A = FindPinLoose(FromNode, FromPin);
	UEdGraphPin* B = FindPinLoose(ToNode, ToPin);
	if (!A || !B)
	{
		return false;
	}

	const UEdGraphSchema* Schema = FromNode->GetGraph()->GetSchema();
	const FPinConnectionResponse Response = Schema->CanCreateConnection(A, B);
	if (Response.Response == CONNECT_RESPONSE_DISALLOW)
	{
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Cannot link %s.%s -> %s.%s: %s"),
			*FromNode->GetName(), *FromPin, *ToNode->GetName(), *ToPin, *Response.Message.ToString());
		return false;
	}

	const bool bLinked = Schema->TryCreateConnection(A, B);
	MarkModified(BlueprintOf(FromNode));
	return bLinked;
}

bool UGraphAuthoringLibrary::BreakPinLinks(UEdGraphNode* Node, const FString& PinName)
{
	UEdGraphPin* Pin = FindPinLoose(Node, PinName);
	if (!Pin)
	{
		return false;
	}
	Node->GetGraph()->GetSchema()->BreakPinLinks(*Pin, true);
	MarkModified(BlueprintOf(Node));
	return true;
}

bool UGraphAuthoringLibrary::SetPinDefault(UEdGraphNode* Node, const FString& PinName, const FString& Value)
{
	UEdGraphPin* Pin = FindPinLoose(Node, PinName);
	if (!Pin)
	{
		return false;
	}
	Node->GetGraph()->GetSchema()->TrySetDefaultValue(*Pin, Value);
	MarkModified(BlueprintOf(Node));
	return true;
}

bool UGraphAuthoringLibrary::SetPinDefaultObject(UEdGraphNode* Node, const FString& PinName, UObject* Value)
{
	UEdGraphPin* Pin = FindPinLoose(Node, PinName);
	if (!Pin)
	{
		return false;
	}
	Node->GetGraph()->GetSchema()->TrySetDefaultObject(*Pin, Value);
	MarkModified(BlueprintOf(Node));
	return true;
}

TArray<FString> UGraphAuthoringLibrary::DescribePins(UEdGraphNode* Node)
{
	TArray<FString> Lines;
	if (!Node)
	{
		return Lines;
	}

	for (const UEdGraphPin* Pin : Node->Pins)
	{
		if (!Pin)
		{
			continue;
		}
		FString Type = Pin->PinType.PinCategory.ToString();
		if (Pin->PinType.PinSubCategoryObject.IsValid())
		{
			Type += TEXT("/") + Pin->PinType.PinSubCategoryObject->GetName();
		}
		FString Default = Pin->DefaultObject ? Pin->DefaultObject->GetPathName() : Pin->DefaultValue;
		Lines.Add(FString::Printf(TEXT("%s %s (%s) : %s = %s -> %d%s"),
			Pin->Direction == EGPD_Input ? TEXT("in ") : TEXT("out"),
			*Pin->PinName.ToString(),
			*Node->GetPinDisplayName(Pin).ToString(),
			*Type,
			*Default,
			Pin->LinkedTo.Num(),
			Pin->bHidden ? TEXT(" [hidden]") : TEXT("")));
	}
	return Lines;
}

FString UGraphAuthoringLibrary::DescribeNode(UEdGraphNode* Node)
{
	if (!Node)
	{
		return TEXT("None");
	}
	return FString::Printf(TEXT("%s [%s] @(%d, %d)"),
		*Node->GetNodeTitle(ENodeTitleType::ListView).ToString(), *Node->GetClass()->GetName(), Node->NodePosX, Node->NodePosY);
}

TArray<UEdGraphNode*> UGraphAuthoringLibrary::GetNodes(UEdGraph* Graph)
{
	TArray<UEdGraphNode*> Nodes;
	if (Graph)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			Nodes.Add(Node);
		}
	}
	return Nodes;
}

bool UGraphAuthoringLibrary::RemoveNode(UEdGraphNode* Node)
{
	UBlueprint* Blueprint = BlueprintOf(Node);
	if (!Blueprint)
	{
		return false;
	}
	FBlueprintEditorUtils::RemoveNode(Blueprint, Node, /*bDontRecompile*/ true);
	return true;
}

bool UGraphAuthoringLibrary::SetPropertyByPath(UObject* Object, const FString& Path, const FString& Value)
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
	if (!ResolvePath(Object, Path, Property, ValuePtr))
	{
		return false;
	}

	Object->Modify();
	if (!Property->ImportText_Direct(*Value, ValuePtr, Object, PPF_None))
	{
		UE_LOG(LogGraphAuthoring, Warning, TEXT("Cannot import '%s' into %s"), *Value, *Path);
		return false;
	}

	FPropertyChangedEvent ChangedEvent(Property, EPropertyChangeType::ValueSet);
	Object->PostEditChangeProperty(ChangedEvent);

	if (UEdGraphNode* Node = Cast<UEdGraphNode>(Object))
	{
		Node->ReconstructNode();
		MarkModified(BlueprintOf(Node));
	}
	return true;
}

FString UGraphAuthoringLibrary::GetPropertyByPath(UObject* Object, const FString& Path)
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
	FString Result;
	if (ResolvePath(Object, Path, Property, ValuePtr))
	{
		Property->ExportText_Direct(Result, ValuePtr, nullptr, Object, PPF_None);
	}
	return Result;
}
