// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "NumberCell.generated.h"

/**
 * 
 */
UCLASS()
class SHAKASHAKA_API ANumberCell : public ACell
{
	GENERATED_BODY()
	
public:
	int GetNumber() const;
	void SetNumber(int n);

private:
	void SetMeshComponent() override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBlack1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBlack2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBlack3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mBlack4;
private:
	int mNumber;
};
