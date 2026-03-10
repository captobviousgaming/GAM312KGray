#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableItem.generated.h"

// --- WEEK 2: RESOURCE ENUM ---
// This creates a drop-down menu in the Unreal Editor to select the item type
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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The physical 3D model of the item (the tree, rock, or bush mesh)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	// The type of resource this specific object will give the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	// How many resources this item holds before it is completely depleted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ResourceCapacity;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function called by the player when hit by the line trace
	// Returns true if successfully gathered, and passes back the type of resource
	bool GatherResource(EResourceType& OutResourceType);
};