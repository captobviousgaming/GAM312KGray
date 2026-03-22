#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableItem.generated.h"

// [Week 2] This enum defines the different types of resources the player can gather.
// [Week 3] I expanded this list to include Wall, Floor, and Roof so the player's dynamic inventory can track crafted building pieces too.
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Wood UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	Berry UMETA(DisplayName = "Berry"),
	Wall UMETA(DisplayName = "Wall"),
	Floor UMETA(DisplayName = "Floor"),
	Roof UMETA(DisplayName = "Roof")
};

UCLASS()
class GAM_312_KGRAY_API AInteractableItem : public AActor
{
	GENERATED_BODY()

public:
	AInteractableItem();

protected:
	// [Week 2] The physical mesh of the resource node (like a tree or rock).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	// [Week 2] What type of resource does this node give the player?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	// [Week 2] How many times can the player hit this node before it's empty?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ResourceCapacity;

public:
	// [Week 2] The function called by the player's line trace when they interact with this node.
	bool GatherResource(EResourceType& OutResourceType);
};