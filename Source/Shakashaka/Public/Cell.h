// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cell.generated.h"

UCLASS()
class SHAKASHAKA_API ACell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Set visual mesh for this cell
	virtual void SetMeshComponent();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int mX;
	int mY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float mSize = 8;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* mCellMesh;
};
