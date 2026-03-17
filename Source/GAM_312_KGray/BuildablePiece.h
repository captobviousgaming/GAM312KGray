#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildablePiece.generated.h"

UCLASS()
class GAM_312_KGRAY_API ABuildablePiece : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildablePiece();

protected:
	// The physical 3D model of my building piece (Wall, Floor, Ceiling)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PieceMesh;

	// --- BUILDING COSTS ---
	// The amount of wood required for the player to craft this specific piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 WoodCost;

	// The amount of stone required for the player to craft this specific piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 StoneCost;

public:
	// Getter functions that allow my Player Character to safely check the required costs 
	// before committing to placing the object and draining their inventory.
	int32 GetWoodCost() const { return WoodCost; }
	int32 GetStoneCost() const { return StoneCost; }
};