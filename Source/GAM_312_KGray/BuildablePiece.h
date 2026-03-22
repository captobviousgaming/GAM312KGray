#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildablePiece.generated.h"

// NEW: We need to know what type of piece this is so we can enforce specific snapping rules!
UENUM(BlueprintType)
enum class EPieceType : uint8
{
	Wall UMETA(DisplayName = "Wall"),
	Floor UMETA(DisplayName = "Floor"),
	Roof UMETA(DisplayName = "Roof")
};

UCLASS()
class GAM_312_KGRAY_API ABuildablePiece : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildablePiece();

protected:
	// The physical 3D model of my building piece
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PieceMesh;

	// The amount of wood required for the player to craft this specific piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 WoodCost;

	// The amount of stone required for the player to craft this specific piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Cost")
	int32 StoneCost;

	// NEW: What type of piece is this? Set this in the Details panel of your Blueprints.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Rules")
	EPieceType PieceType;

public:
	// Getter functions that allow my Player Character to check costs
	int32 GetWoodCost() const { return WoodCost; }
	int32 GetStoneCost() const { return StoneCost; }

	// Getter for the piece type so the player knows what it is looking at
	EPieceType GetPieceType() const { return PieceType; }

	// Exposing the mesh so the Player script can swap its material to a translucent silhouette!
	class UStaticMeshComponent* GetPieceMesh() const { return PieceMesh; }
};