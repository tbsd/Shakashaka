// Fill out your copyright notice in the Description page of Project Settings.
#include "Board.h"

#include "SquareCell.h"
#include "TriangleCell.h"
#include "NumberCell.h"

#include "Containers/UnrealString.h" 
#include "Engine/Engine.h"

#include <ctime>
#include <cstdlib>
#include <vector>
#include <utility>
#include <chrono>


// Sets default values
ABoard::ABoard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	std::srand(std::time(NULL));
}
void ABoard::ResetToBoard(std::shared_ptr<LpSolver::GameBoard> board) {
	auto current = GetCurrentBoard();
	if (!board || board->empty() || board->front().empty()) 
		return;
	if (!current || current->size() != board->size()|| current->front().size() != board->front().size()) 
		return;
	for (size_t i = 0; i < mWidth; ++i) {
		for (size_t j = 0; j < mHeight; ++j) {
			if ((*current)[i][j] != (*board)[i][j])
			{
				SetCell(i, j, (*board)[i][j]);
			}
		}
	}
}

void ABoard::ResetToInitBoard() {
	ResetToBoard(mInitBoard);
}

void ABoard::Solve() {
	if (mSolutions.empty())
		return;
	auto current = GetCurrentBoard();
	auto solution = mSolutions.front();
	for (auto s : mSolutions) {
		auto cmp = CmpBoards(current, s);
		if (cmp == CmpResult::Equal || cmp == CmpResult::Less) {
			solution = s;
			break;
		}
	}
	ResetToBoard(solution);
}

void ABoard::SetCell(int i, int j, LpSolver::CellType type) {
	FTransform spawn_transform = GetActorTransform();
	spawn_transform.SetLocation(spawn_transform.TransformPosition(FVector(i * mCellSz - mWidth * mCellSz / 2, j * mCellSz - mHeight * mCellSz / 2, 0.f)));
	FActorSpawnParameters spawn_params;
	if (mBoard[i][j]) {
		mBoard[i][j]->Destroy();
	}
	switch (type) {
	case LpSolver::CellType::Black:
		mBoard[i][j] = GetWorld()->SpawnActor<ASquareCell>(mSquareCellType, spawn_transform, spawn_params);
		break;
	case LpSolver::CellType::Empty: [[fallthrough]];
	case LpSolver::CellType::UpperLeft: [[fallthrough]];
	case LpSolver::CellType::UpperRight: [[fallthrough]];
	case LpSolver::CellType::BottomRight: [[fallthrough]];
	case LpSolver::CellType::BottomLeft:
	{
		auto triangle = GetWorld()->SpawnActor<ATriangleCell>(mTriangleCellType, spawn_transform, spawn_params);
		triangle->ChangeType(type);
		mBoard[i][j] = triangle;
	}
	break;
	default:
	{
		auto numbered = GetWorld()->SpawnActor<ANumberCell>(mNumberCellType, spawn_transform, spawn_params);
		numbered->SetNumber(static_cast<int>(type));
		mBoard[i][j] = numbered;
	}
	}

}

void ABoard::GetHint() {
	if (!IsSolved()) {
		auto current = GetCurrentBoard();
		std::shared_ptr<LpSolver::GameBoard> future_solution;
		for (auto board : mSolutions) {
			if (CmpBoards(current, board) == CmpResult::Less) {
				future_solution = board;
				break;
			}
		}
		if (!future_solution) {
			return;
		}
		for (size_t i = 0; i < mWidth; ++i) {
			for (size_t j = 0; j < mHeight; ++j) {
				if ((*current)[i][j] != (*future_solution)[i][j])
				{
					SetCell(i, j, (*future_solution)[i][j]);
					return;
				}
			}
		}

	}
}

bool ABoard::IsSolved() {
	auto current = GetCurrentBoard();
	for (const auto board : mSolutions) {
		if (CmpBoards(current, board) == CmpResult::Equal) {
			GEngine->AddOnScreenDebugMessage(6, 10, FColor::Green, TEXT("Верное решение!"));
			return true;
		}
	}
	mSolver.InitBoard(current);
	mSolver.mTimeoutSec = 20;
	bool is_solvable = mSolver.Solve();
	if (is_solvable) {
		mSolutions.push_back(mSolver.GetSolution());
		if (CmpBoards(current, mSolutions.back()) == CmpResult::Equal) {
			GEngine->AddOnScreenDebugMessage(7, 10, FColor::Green, TEXT("Верное решение!"));
			return true;
		}
		GEngine->AddOnScreenDebugMessage(8, 10, FColor::Green, TEXT("Ещё не решение, но на верном пути"));
		return false;
	}
	GEngine->AddOnScreenDebugMessage(1, 10, FColor::Red, TEXT("Похоже, где-то есть ошибка"));
	return false;
}

// Called when the game starts or when spawned
void ABoard::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoard::ClearBoard() {
	for (size_t i = 0; i < mBoard.size(); ++i) {
		for (size_t j = 0; j < mBoard[i].size(); ++j) {
			mBoard[i][j]->Destroy();
		}
		mBoard[i].clear();
	}
	mBoard.clear();
	mInitBoard = nullptr;
	mSolutions.clear();
}

std::pair<int, int> ABoard::GetRandomCell() {
	return { std::rand() % (mWidth - 2) + 1, std::rand() % (mHeight - 2) + 1 };
}

bool ABoard::GenerateNew() {
	UE_LOG(LogTemp, Log, TEXT("HEHMDA ABoard::GenerateNew(): Tests start width: %d, height: %d"), mWidth, mHeight);
	GEngine->AddOnScreenDebugMessage(2, 10, FColor::Green, TEXT("Генерируем новое поле..."));
	auto current_timestamp = []() {return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); };
	auto start = current_timestamp();
	auto generation_end = start + mMaxGenerationTimeSec;
	const int change_timeout = 5;
	const int timeout_delta = 3;
	int attempt_count = 0;
	//int timeout = 4 - timeout_delta;
	int timeout = mTimeout;
	int found = 0; // HEHMDA
	while (true) {
	//while (current_timestamp() < generation_end) {
		//if (attempt_count % change_timeout == 0)
			//timeout += timeout_delta;
		++attempt_count;
		UE_LOG(LogTemp, Log, TEXT("HEHMDA ABoard::GenerateNew(): Tests time passed: %d"), current_timestamp() - start);
		UE_LOG(LogTemp, Log, TEXT("HEHMDA ABoard::GenerateNew(): Tests generating init board. Attempt: %d, timeout: %d, found %d"), attempt_count, timeout, found);
		UE_LOG(LogTemp, Log, TEXT("ABoard::GenerateNew(): generating init board. Attempt: %d, timeout: %d"), attempt_count, timeout);
		ClearBoard();
		InitBoard();
		std::shared_ptr<LpSolver::GameBoard> board = GetCurrentBoard();
		mSolver.InitBoard(board);
		mSolver.mTimeoutSec = timeout;
		bool is_solved = mSolver.Solve();
		//UE_LOG(LogTemp, Log, TEXT("ABoard::GenerateNew():\n%s"), *(mSolver.ToString()));
		if (is_solved) {
			mInitBoard = GetCurrentBoard();
			mSolutions.push_back(mSolver.GetSolution());
			++found; // HEHMDA
			attempt_count = 0;
			//return true;
			if (found == 10)
				return true;
		}
		UE_LOG(LogTemp, Log, TEXT("HEHMDA1 ABoard::GenerateNew(): Tests generating init board. width, height, timout, attempt, isfound, time_elapsed, black, numbered: %d,%d,%d,%d,%d,%f,%d,%d"), 
			mWidth, mHeight, timeout, attempt_count, is_solved, mSolver.GetTimeElapsed(), mSquaredCellsCount, mNumberedCellsCount);
	}
	UE_LOG(LogTemp, Log, TEXT("ABoard::GenerateNew(): failed to generate init board"));
	GEngine->AddOnScreenDebugMessage(3, 10, FColor::Red, TEXT("Не удалось сгенерировать поле, попробуйте ещё раз"));
	ClearBoard();
	return false; // failed to generate
}

bool ABoard::IsTriangleCell(const std::pair<int, int>& p) {
	if (dynamic_cast<ATriangleCell*>(mBoard[p.first][p.second]))
		return true;
	return false;
}

ATriangleCell* ABoard::GetIfOkToPlaceBlackCell(const std::pair<int, int>& p) {
	return dynamic_cast<ATriangleCell*>(mBoard[p.first][p.second]);
}


void ABoard::InitBoard() {
	if (GetWorld()) {
		mBoard.reserve(mWidth);
		for (size_t i = 0; i < mWidth; ++i) {
			mBoard.push_back({});
			mBoard[i].reserve(mHeight);
			for (size_t j = 0; j < mHeight; ++j) {
				FTransform spawn_transform = GetActorTransform();
				spawn_transform.SetLocation(spawn_transform.TransformPosition(FVector(i * mCellSz - mWidth * mCellSz / 2, j * mCellSz - mHeight * mCellSz / 2, 0.f)));
				FActorSpawnParameters spawn_params;
				if (i == 0 || j == 0 || i == mWidth - 1 || j == mHeight - 1) { // border

					mBoard[i].emplace_back(GetWorld()->SpawnActor<ASquareCell>(mSquareCellType, spawn_transform, spawn_params));
				}
				else { // main field
					mBoard[i].emplace_back(GetWorld()->SpawnActor<ATriangleCell>(mTriangleCellType, spawn_transform, spawn_params));
				}
			}
		}
		auto add_black_cells = [&](const int max_count, LpSolver::CellType type) {
			int count = max_count;
			int fail_count = 0;
			const int fail_max = 100;
			while (count > 0) {
				auto point = GetRandomCell();
				if (auto* c = GetIfOkToPlaceBlackCell(point)) {
					FTransform spawn_transform = GetActorTransform();
					spawn_transform.SetLocation(spawn_transform.TransformPosition(FVector(point.first * mCellSz - mWidth * mCellSz / 2, point.second * mCellSz - mHeight * mCellSz / 2, 0.f)));
					FActorSpawnParameters spawn_params;
					c->Destroy();
					switch (type) {
					case LpSolver::CellType::Black:
						mBoard[point.first][point.second] = GetWorld()->SpawnActor<ASquareCell>(mSquareCellType, spawn_transform, spawn_params);
						break;
					default:
						auto* new_c = GetWorld()->SpawnActor<ANumberCell>(mNumberCellType, spawn_transform, spawn_params);
						new_c->SetNumber((std::rand() % 4) + 1);
						mBoard[point.first][point.second] = new_c;
						break;
					}
					--count;
				}
				else {
					if (++fail_count >= fail_max) {
						UE_LOG(LogTemp, Warning, TEXT("Failed to add more cells!"));
						break;
					}
				}
			}
		};
		add_black_cells(mSquaredCellsCount, LpSolver::CellType::Black);
		add_black_cells(mNumberedCellsCount, LpSolver::CellType::Black1);

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("InitBoard(): getworld == nullptr"));
	}
}

ABoard::CmpResult ABoard::CmpBoards(std::shared_ptr<LpSolver::GameBoard> lhs, std::shared_ptr<LpSolver::GameBoard> rhs) const {
	CmpResult res = CmpResult::Equal;
	for (size_t i = 0; i < mWidth; ++i) {
		for (size_t j = 0; j < mHeight; ++j) {
			if (6 <= static_cast<int>((*lhs)[i][j]) && static_cast<int>((*lhs)[i][j]) <= 10) {
				if ((*lhs)[i][j] == (*rhs)[i][j])
					continue;
				else if ((*lhs)[i][j] == LpSolver::CellType::Empty && res != CmpResult::Greater)
					res = CmpResult::Less;
				else if ((*rhs)[i][j] == LpSolver::CellType::Empty && res != CmpResult::Less)
					res = CmpResult::Greater;
				else
					return CmpResult::Incomparable;
			}
		}
	}
	return res;
}

std::shared_ptr<LpSolver::GameBoard> ABoard::GetCurrentBoard() {
	std::shared_ptr<LpSolver::GameBoard> board = std::make_shared<LpSolver::GameBoard>();
	board->reserve(mBoard.size());
	for (size_t i = 0; i < mBoard.size(); ++i) {
		board->emplace_back();
		(*board)[i].reserve(mBoard[i].size());
		for (size_t j = 0; j < mBoard[i].size(); ++j) {
			LpSolver::CellType cell_type = LpSolver::CellType::Empty;
			ACell* cell = mBoard[i][j];
			if (!cell) {
				UE_LOG(LogTemp, Warning, TEXT("GenerateNew(): cell == nullptr"));
			}
			else if (auto* c1 = dynamic_cast<ASquareCell*>(cell)) {
				cell_type = LpSolver::CellType::Black;
			}
			else if (auto* c2 = dynamic_cast<ANumberCell*>(cell)) {
				switch (c2->GetNumber()) {
				case 1:
					cell_type = LpSolver::CellType::Black1;
					break;
				case 2:
					cell_type = LpSolver::CellType::Black2;
					break;
				case 3:
					cell_type = LpSolver::CellType::Black3;
					break;
				case 4:
					cell_type = LpSolver::CellType::Black4;
					break;
				default:
					UE_LOG(LogTemp, Warning, TEXT("GenerateNew(): Black < 1 || Black > 4"));
				}
			}
			else if (auto* c3 = dynamic_cast<ATriangleCell*>(cell)) {
				cell_type = c3->GetType();
			}
			else {

				UE_LOG(LogTemp, Warning, TEXT("GenerateNew(): Unexpected cell type"));
			}
			(*board)[i].push_back(cell_type);
		}
	}
	return board;
}

