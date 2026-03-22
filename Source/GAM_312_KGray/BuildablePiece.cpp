#include "BuildablePiece.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABuildablePiece::ABuildablePiece()
{
	// Building pieces are static structures, so I disabled Tick to save performance.
	PrimaryActorTick.bCanEverTick = false;

	// I initialize the visual mesh component and set it as the root of the object.
	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
	RootComponent = PieceMesh;

	// I initialize default material costs here. These will be overridden in the individual Blueprints.
	WoodCost = 5;
	StoneCost = 0;

	// NEW: Default to wall. Don't forget to change this in your Floor and Roof blueprints!
	PieceType = EPieceType::Wall;
}