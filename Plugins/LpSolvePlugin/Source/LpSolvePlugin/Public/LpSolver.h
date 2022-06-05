// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <memory>

THIRD_PARTY_INCLUDES_START
#include "LpSolvePluginLibrary/lp_lib.h"
THIRD_PARTY_INCLUDES_END

/**
 * 
 */
class LPSOLVEPLUGIN_API LpSolver
{
public:
	enum class CellType {
		Black1 = 1,
		Black2 = 2,
		Black3 = 3,
		Black4 = 4,
		Black = 5,
		Empty = 6,
		UpperLeft = 7,
		UpperRight = 8,
		BottomRight = 9,
		BottomLeft = 10
	};
	using GameBoard = std::vector<std::vector<CellType>>;
	
	LpSolver();
	~LpSolver();

	void InitBoard(std::shared_ptr<GameBoard> board);
	// true if solution exists, false otherwise
	bool Solve(); 
	std::shared_ptr<GameBoard> GetInitBoard();
	std::shared_ptr<GameBoard> GetSolution();
	FString ToString() const;

	int mTimeoutSec = 0;

private:
	// index of first variable corrisponding to X_(x,y, type). CellType::Empty is first of x for this variable
	int GetVarNumber(int x, int y, CellType type = CellType::Empty);
	int GetVarNumber(int x_empty, CellType type);
	bool IsBlack(int x, int y);
	bool IsBlack(int n);

private:
	int mWidth = 0;
	int mHeight = 0;
	std::shared_ptr<GameBoard> mInitBoard;
	std::shared_ptr<GameBoard> mSolution;
	std::shared_ptr<lprec> lp;
	
};
