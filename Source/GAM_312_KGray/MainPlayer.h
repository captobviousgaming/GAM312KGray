#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainPlayer.generated.h"

UCLASS()
class GAM_312_KGRAY_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 1. Declare our First Person Camera Component so we can see through the character's eyes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCameraComponent;

	// 2. Declare our movement functions that will link to our W, A, S, D keys
	void MoveForward(float Value);
	void MoveRight(float Value);

	// --- WEEK 2: PLAYER STATS ---

	// Player's current health. Takes damage if hunger is zero.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	// Player's current hunger. Decreases continuously over time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger;

	// Player's energy. Decreases on tasks, restores over time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	// --- WEEK 2: RESOURCE INVENTORY ---

	// Amount of wood collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Wood;

	// Amount of stone collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Stone;

	// Amount of berries collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Berry;

	// --- WEEK 2: FUNCTIONS ---

	// Timer function to handle stat changes over time
	void HandleStatsOverTime();

	// The timer handle that runs the above function
	FTimerHandle StatTimerHandle;

	// Function to fire the trace collision for resource gathering
	void Interact();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};