// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "SquareCell.generated.h"

/**
 * 
 */
UCLASS()
class SHAKASHAKA_API ASquareCell : public ACell
{
	GENERATED_BODY()
	
public:
	ASquareCell();
	
private:
	void SetMeshComponent() override;
};
