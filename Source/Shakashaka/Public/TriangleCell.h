// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LpSolver.h"
#include "CoreMinimal.h"
#include "Cell.h"
#include "InputCoreTypes.h" 
#include "TriangleCell.generated.h"

/**
 * 
 */
UCLASS()
class SHAKASHAKA_API ATriangleCell : public ACell
{
	GENERATED_BODY()
	

public:
	ATriangleCell();
	LpSolver::CellType GetType();
	LpSolver::CellType GetNextType();
	void ChangeToNext();
	void ChangeType(LpSolver::CellType t);

	UFUNCTION(BlueprintCallable)
	void OnClick(UPrimitiveComponent* pComponent, FKey button);

private:
	void SetMeshComponent() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mEmpty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mUpperLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mUpperRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBottomRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBottomLeft;

private:
	LpSolver::CellType mType = LpSolver::CellType::Empty;
};
