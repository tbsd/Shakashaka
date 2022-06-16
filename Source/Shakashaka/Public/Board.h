// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LpSolver.h"

#include <vector>
#include <utility>
#include <memory>

#include "Cell.h"

#include "GameFramework/Actor.h"
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h" 

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Board.generated.h"

UCLASS()
class SHAKASHAKA_API ABoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	bool GenerateNew(); // true if generated init field, false if failed
	UFUNCTION(BlueprintCallable)
	bool IsSolved();
	UFUNCTION(BlueprintCallable)
	void Solve();
	UFUNCTION(BlueprintCallable)
	void GetHint();
	UFUNCTION(BlueprintCallable)
	void ResetToInitBoard();

private:
	enum class CmpResult {
		Less, // lhs < rhs
		Equal, // lhs == rhs
		Greater, // lhs > rhs
		Incomparable // otherwise
	};
	void InitBoard();
	void ClearBoard();
	std::pair<int, int> GetRandomCell();
	// returns pointer to cell if it's possible to place a cell there, nullptr otherwise
	class ATriangleCell* GetIfOkToPlaceBlackCell(const std::pair<int, int>& point);
	bool IsTriangleCell(const std::pair<int, int>& point);
	CmpResult CmpBoards(std::shared_ptr<LpSolver::GameBoard> lhs, std::shared_ptr<LpSolver::GameBoard> rhs) const;
	std::shared_ptr<LpSolver::GameBoard> GetCurrentBoard();
	void SetCell(int i, int j, LpSolver::CellType type);
	void ResetToBoard(std::shared_ptr<LpSolver::GameBoard> board);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mWidth = 15;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mHeight = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mSquaredCellsCount = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mNumberedCellsCount = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mMaxGenerationTimeSec = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mTimeout = 10;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float mCellSz = 104.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ANumberCell> mNumberCellType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ASquareCell> mSquareCellType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATriangleCell> mTriangleCellType;

private:
	int mTimeoutSec = 4; // timeout for one try

	std::vector<std::vector<ACell*>> mBoard; // view
	LpSolver mSolver;
	std::shared_ptr<LpSolver::GameBoard> mInitBoard;
	//std::shared_ptr<LpSolver::GameBoard> mCurrentBoard; // do really need it? If check performed only on demand and not each turn
	std::vector<std::shared_ptr<LpSolver::GameBoard>> mSolutions;
};
