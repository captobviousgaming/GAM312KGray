#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableItem.generated.h"

// --- ENUM DECLARATION ---
// I am defining the specific types of resources the player can harvest here. (Can easily build more)
// BlueprintType allows me to select this enum from a drop-down menu in the Unreal Editor.
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Wood UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	Berry UMETA(DisplayName = "Berry")
};

UCLASS()
class GAM_312_KGRAY_API AInteractableItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableItem();

protected:
	// The physical 3D representation of my resource node ( Tree, Rock, Bush)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	// The specific type of resource this node contains, which I can set in the Editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	// The number of times this specific node can be harvested before it is destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ResourceCapacity;

public:
	// This function is called by the player's Line Trace. 
	// It returns true if harvesting was successful and passes the resource type back to the player.
	bool GatherResource(EResourceType& OutResourceType);
};