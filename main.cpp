/*
	copy right (c) 2026  WILLIAM POTTER
	Free software, and you are welcome to redistribute it
*/
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

#include <stdio.h>
#include <Windows.h>




wstring blocks[7];

int fieldWidth = 12, fieldHeight = 18;
unsigned char* field = nullptr;

int screenHeight = 30, screenWidth = 120; // change for cdm size or change cmd settings


/* px - pice x
* py - pice y
* r - rotation
*  reurn the new placement pice with rotation
*/
int rotate(int px, int py, int r) {

	switch (r % 4) {
	case 0: return py * 4 + px;			// 0 degrees
	case 1: return 12 + py - (px * 4);  // 90 degrees
	case 2: return 15 - (py * 4) - px;  // 180 degrees
	case 3: return 3 - py + (px * 4);   // 270 degrees
	}
	return 0;
}

bool doesPieceFit(int block, int rotation, int posX, int posY) {

	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {
			// index into piece
			int pi = rotate(px,py, rotation);
			// index into field
			int fi = (posY + py) * fieldWidth + (posX + px);

			if (posX + px >= 0 && posX + px < fieldWidth) {
				if (posY + py >= 0 && posY + py < fieldHeight) {
					if (blocks[block][pi] == L'X' && field[fi] != 0) {
						return false; // fails on first hit
					}
				}

			}

			
		} // end of py loop
	} // end of px loop


	return true;
}

int main() {
	// creating blocks
	blocks[0].append(L"..X.");
	blocks[0].append(L"..X.");
	blocks[0].append(L"..X.");
	blocks[0].append(L"..X.");

	blocks[1].append(L"..X.");
	blocks[1].append(L".XX.");
	blocks[1].append(L".X..");
	blocks[1].append(L"....");

	blocks[2].append(L".X..");
	blocks[2].append(L".XX.");
	blocks[2].append(L"..X.");
	blocks[2].append(L"....");

	blocks[3].append(L"....");
	blocks[3].append(L".XX.");
	blocks[3].append(L".XX.");
	blocks[3].append(L"....");

	blocks[4].append(L"..X.");
	blocks[4].append(L".XX.");
	blocks[4].append(L"..X.");
	blocks[4].append(L"....");

	blocks[5].append(L"....");
	blocks[5].append(L".XX.");
	blocks[5].append(L"..X.");
	blocks[5].append(L"..X.");

	blocks[6].append(L"....");
	blocks[6].append(L".XX.");
	blocks[6].append(L".X..");
	blocks[6].append(L".X..");


	field = new unsigned char[fieldHeight * fieldWidth]; // creat field
	for (int x = 0; x < fieldWidth; x++) { // board bounds
		for (int y = 0; y < fieldHeight; y++) {
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;
		}
	}
	// screen
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth*screenHeight; i++) screen[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;


	// logic stuff
	bool gameOver = false;
	int speed = 25;
	int tickCounter = 0;
	bool forceDown = false;
	vector<int> lines;
	int blocksDroped = 0;
	int score = 0;
	// piece stuff
	int currentPiece = 0;
	int currentRotation = 0;
	int currentX = fieldWidth / 2;
	int currentY = 0;

	bool keys[4];
	// keys locks
	bool rotateHold = false;



	while (!gameOver) {

		// GAME TIMING
		this_thread::sleep_for(50ms); // game tick
		tickCounter ++;
		forceDown = (tickCounter == speed);

		
		// INPUT
		for (int k = 0; k < 4; k++) {							// r l d arrows and z 
			keys[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// GAME LOGIC
		// player movement
		if (keys[0]) { // right
			if (doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) {
				currentX += 1;
			}
		}
		if (keys[1]) { // left
			if (doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) {
				currentX -= 1;
			}
		}
		if (keys[2]) { // down
			if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY += 1;
			}
		}
		// other keys can be written this this too
		if (keys[3]) {
			currentRotation += (!rotateHold && doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
			rotateHold = true;
		} else {
			rotateHold = false;
		}
		// game movemnt 
		if (forceDown) {
			if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY++;
			} else {
				// lock current piece in field
				for (int px = 0; px < 4; px++) {
					for (int py = 0; py < 4; py++) {

						if (blocks[currentPiece][rotate(px, py, currentRotation)] == L'X') {
							field[(currentY + py) * fieldWidth + (currentX + px)] = currentPiece + 1; // +1, 0 is entpy space and a shape too
						}

					}
				}
				blocksDroped++;
				if (blocksDroped % 10 == 0) {
					if (speed > 5) speed--;
				}
				// check full hor lines
				// can only be the rows of lasted placed block
				for (int py = 0; py < 4; py++) {
					if (currentY + py < fieldHeight - 1) {
						bool line = true; // assume line is made, then check for gaps
						for (int px = 1; px < fieldWidth - 1; px++) {
							line &= (field[(currentY + py) * fieldWidth + px]) != 0;
						}
					

							if (line) {
								// remove line and set to =
								for (int px = 1; px < fieldWidth - 1; px++) {
									field[(currentY + py) * fieldWidth + px] = 8;
								}
								lines.push_back(currentY + py);
							} // end if line is true
							
					} // end of if py in bounds
				}// end of py loop

				score += 20;
				if (!lines.empty()) score += (1 << lines.size()) * 120;

				// choose next piece
				currentX = fieldWidth / 2;
				currentY = 0;
				currentRotation = 0;
				currentPiece = rand() % 7; // 7 diffrent pieces
				// if piece can fit
				gameOver = !doesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}
			tickCounter = 0;
		}


		// RENDER OUTPUT

		// draw field
		for (int x = 0; x < fieldWidth; x++) { 
			for (int y = 0; y < fieldHeight; y++) {
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[field[y * fieldWidth + x]];
			}
		}
		// draw current piece
		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				// +2 is the offset, +65 for askkey letters a B C D ect...
				if (blocks[currentPiece][rotate(px, py, currentRotation)] == L'X') {
					screen[(currentY + py + 2) * screenWidth + (currentX + px + 2)] = currentPiece + 65;
				}

			}
		}
		// draw score
		swprintf_s(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE: %8d", score);

		if (!lines.empty()) {
			// dislay frame
			WriteConsoleOutputCharacterW(console, screen, screenWidth * screenHeight, { 0,0 }, &bytesWritten);
			this_thread::sleep_for(435ms); // delay a bit

			for (auto& v : lines) {
				for (int px = 1; px < fieldWidth - 1; px++) {
					for (int py = v; py > 0; py--) {
						field[py * fieldWidth + px] = field[(py - 1) * fieldWidth + px];
					}
					field[px] = 0;
				}
			} // end of for v lines
			lines.clear();
		}


		// display frame
		WriteConsoleOutputCharacterW(console, screen, screenWidth * screenHeight, { 0,0 }, &bytesWritten);
	}
	// game over 
	CloseHandle(console);
	cout << "Game over :(\n\t SCORE: " << score << endl;
	system("pause");

	return 0;
}