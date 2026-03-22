#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableItem.generated.h"

// Added Wall, Floor, and Roof to the enum so they can show up in your dynamic Inventory array.
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EResourceType ResourceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ResourceCapacity;

public:
	bool GatherResource(EResourceType& OutResourceType);
};