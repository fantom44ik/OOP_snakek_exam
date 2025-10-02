#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

// перечисления для стен, яблок и цветов
enum GameObject : short { HALL, WALL, APPLE };
enum Color { BLACK = 1, DARKGREEN = 2, YELLOW = 14, RED = 12, BLUE = 9, WHITE = 15, DARKYELLOW = 6, DARKRED = 4 };
enum Key { ENTER = 13, ESCAPE = 27, SPACE = 32, W = 119, A = 97, S = 115, D = 100, LEFT = 75, RIGHT = 77, UP = 72, DOWN = 80, BACKSPACE = 8 };
enum Direction { NONE, UP_DIR, DOWN_DIR, LEFT_DIR, RIGHT_DIR };

class Game_map {
private:
	vector<vector<GameObject>> map; //создание двумерного вектора(массива) для карты
	int height, width; //высота и ширина карты

public:
    //заполнение карты пустыми клетками 
    Game_map(int y, int x) : height(y), width(x) {
		map.resize(height, vector<GameObject>(width, HALL));
    }

	//заполнение карты стенами по краям
    void fill() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                    map[y][x] = WALL;
                }
                else {
                    map[y][x] = HALL;
                }
            }
        }
    }

	//вывод карты на экран
    void print(HANDLE h) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                GameObject obj = map[y][x];
                switch (obj) {
                case HALL:
                    cout << " ";
                    break;
                case WALL:
                    SetConsoleTextAttribute(h, Color::WHITE);
                    cout << (char)178;
                    break;
                case APPLE:
                    SetConsoleTextAttribute(h, Color::RED);
                    cout << "o";
                    break;
                }
            }
            cout << "\n";
        }
        SetConsoleTextAttribute(h, Color::WHITE);
    }

	//доступ к элементам карты
    GameObject& at(int y, int x) {
        return map[y][x];
    }

	//константная версия для доступа к элементам карты
    const GameObject& at(int y, int x) const {
        return map[y][x];
    }

	//геттеры для высоты и ширины карты
    int getHeight() const { return height; }
    int getWidth() const { return width; }
};

class Snake {
private:
	vector<COORD> body;  //вектор(массив) для хранения координат частей змейки
	int length; //текущая длина змейки
	Direction currentDir; //текущее направление движения змейки

public:
	//спавн змейки в центре карты
    Snake(int startX, int startY) : length(1), currentDir(NONE) {
        body.resize(1000);
        body[0].X = startX;
        body[0].Y = startY;
    }

	//функция которая задает направление движения змейки 
    void setDirection(Direction newDir) {
        if (newDir == NONE) return;
        bool opposite = false;
        switch (newDir) {
        case UP_DIR:
            opposite = (currentDir == DOWN_DIR);
            break;
        case DOWN_DIR:
            opposite = (currentDir == UP_DIR);
            break;
        case LEFT_DIR:
            opposite = (currentDir == RIGHT_DIR);
            break;
        case RIGHT_DIR:
            opposite = (currentDir == LEFT_DIR);
            break;
        default:
            return;
        }
        if (!opposite) {
            currentDir = newDir;
        }
    }

	//геттер для текущего направления движения змейки
    Direction getCurrentDirection() const {
        return currentDir;
    }

	//геттер для вычисления новой позиции головы змейки
    COORD getNewHead() const {
        COORD head = body[0];
        switch (currentDir) {
        case UP_DIR:
            head.Y--;
            break;
        case DOWN_DIR:
            head.Y++;
            break;
        case LEFT_DIR:
            head.X--;
            break;
        case RIGHT_DIR:
            head.X++;
            break;
        default:
            break;
        }
        return head;
    }

	//проверка столкновения с стеной
    bool willCollideWall(const Game_map& game_map, COORD newHead) const {
        return newHead.X < 0 || newHead.X >= game_map.getWidth() ||
            newHead.Y < 0 || newHead.Y >= game_map.getHeight() ||
            game_map.at(newHead.Y, newHead.X) == WALL;
    }

	//проверка столкновения с телом змейки
    bool willCollideBody(COORD newHead) const {
        for (int i = 1; i < length; ++i) {
            if (body[i].X == newHead.X && body[i].Y == newHead.Y) {
                return true;
            }
        }
        return false;
    }

	//движение змейки
    void move() {
        COORD newHead = getNewHead();
        for (int i = length; i > 0; --i) {
            body[i] = body[i - 1];
        }
        body[0] = newHead;
    }

	//увеличение длины змейки
    void grow() {
        ++length;
    }

	//геттер для координат головы змейки
    COORD getHead() const {
        return body[0];
    }

	//геттер для текущей длины змейки
    int getLength() const {
        return length;
    }

	//отрисовка змейки на экране
    void draw(HANDLE h) {
		//отрисовка головы
        SetConsoleCursorPosition(h, body[0]);
        SetConsoleTextAttribute(h, Color::BLUE);
        cout << "H";
		//отрисовка тела
        SetConsoleTextAttribute(h, Color::BLUE);
        for (int i = 1; i < length; ++i) {
            SetConsoleCursorPosition(h, body[i]);
            cout << "o";
        }
    }

	//стирание хвоста змейки с экрана
    void eraseTail(HANDLE h) {
        if (length > 0) {
            SetConsoleCursorPosition(h, body[length]);
            SetConsoleTextAttribute(h, Color::BLACK);
            cout << " ";
        }
    }

	//сброс змейки в начальное состояние
    void reset(int startX, int startY) {
        length = 1;
        currentDir = NONE;
        body[0].X = startX;
        body[0].Y = startY;
    }
};

class Apple {
private:
	COORD pos; //координаты яблока
public:
	//спавн яблока в случайной позиции на карте
    void spawn(Game_map& game_map, const Snake& snake) {
        int height = game_map.getHeight();
        int width = game_map.getWidth();
        COORD head = snake.getHead();
		//генерация случайных координат до тех пор, пока не будет найдена пустая клетка
        do {
            pos.X = rand() % (width - 1);
            pos.Y = rand() % (height - 1);
        } while (game_map.at(pos.Y, pos.X) != HALL ||
            (pos.X == head.X && pos.Y == head.Y));

        game_map.at(pos.Y, pos.X) = APPLE;

		//оторисовка яблока на экране
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(h, pos);
        SetConsoleTextAttribute(h, Color::RED);
        cout << "o";
    }

	//проверка на то находится ли голова змейки на позиции яблока
    bool isAt(const COORD& head) const {
        return pos.X == head.X && pos.Y == head.Y;
    }
};

class ScoreManager {
private:
	string filename = "snake_save.txt";  //имя файла для сохранения результатов
public:
	//сохранение результата в файл
    void saveScore(int score) {
        ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << score << "\n";
            file.close();
        }
    }

	//вывод результатов из файла
    void printScores() {
        ifstream inFile(filename);
        if (!inFile) {
            cout << "\nError: unable to open save file.\n";
            return;
        }

        string line;
        int index = 1;
        cout << "\n=== Previous results ===\n";
        while (getline(inFile, line)) {
            cout << index << ". " << line << "\n";
            ++index;
        }
        cout << "========================\n";

        inFile.close();
    }

	//очистка результатов из файла
    void clearScores() {
        ofstream outFile(filename);
        if (outFile) {
            outFile.close();
        }
        else {
            cout << "\nError: unable to open save file.\n";
        }
    }
};

class Game {
private:
	Game_map game_map; //карта игры
	Snake snake; //змейка
	Apple apple; //яблоко
	ScoreManager scores; //менеджер результатов
	int applesEaten; //количество съеденных яблок
    HANDLE h; 
	int height, width; //высота и ширина карты
	bool gameStarted; //начата ли игра

	//настройка консоли
    void setupConsole() {
		system("title Snake Game"); //установка заголовка окна
        CONSOLE_FONT_INFOEX font;
        font.cbSize = sizeof(font);
        font.dwFontSize.Y = 25;
        font.FontFamily = FF_DONTCARE;
        font.FontWeight = FW_NORMAL;
        wcscpy_s(font.FaceName, 9, L"Consolas");
        SetCurrentConsoleFontEx(h, 0, &font);

		//скрытие курсора
        CONSOLE_CURSOR_INFO cursor;
        cursor.bVisible = false;
        cursor.dwSize = 100;
        SetConsoleCursorInfo(h, &cursor);
    }

	//вывод информации об игре на экран
    void printGameInfo() {
		//информация о том как выйти из игры
        COORD infoPos;
        infoPos.X = width + 1;
        infoPos.Y = 1;
        SetConsoleCursorPosition(h, infoPos);
        SetConsoleTextAttribute(h, Color::YELLOW);
        cout << "Press ESC to exit the game\n";

		//информация об управлении
        infoPos.Y = 3;
        SetConsoleCursorPosition(h, infoPos);
        SetConsoleTextAttribute(h, Color::DARKYELLOW);
        cout << "Controls:  ";
        SetConsoleTextAttribute(h, Color::YELLOW);
        cout << "UP: W | DOWN: S | LEFT: A | RIGHT: D";

		//информация о количестве съеденных яблок
        infoPos.Y = 5;
        SetConsoleCursorPosition(h, infoPos);
        SetConsoleTextAttribute(h, Color::DARKRED);
        cout << "APPLES: ";
        SetConsoleTextAttribute(h, Color::RED);
        cout << applesEaten;
    }

	//обновление информации о количестве съеденных яблок
    void updateApplesInfo() {
        COORD infoPos;
        infoPos.X = width + 1;
        infoPos.Y = 5;
        SetConsoleCursorPosition(h, infoPos);
        SetConsoleTextAttribute(h, Color::DARKRED);
        cout << "APPLES: ";
        SetConsoleTextAttribute(h, Color::RED);
        cout << applesEaten << "  ";
    }

	//обработка ввода с клавиатуры
    void handleInput() {
        if (_kbhit()) {
            int code = _getch();
            if (code == 224) {
                code = _getch();
            }
            if (code == Key::ESCAPE) {
                exitGame();
                return;
            }
            Direction dir = NONE;
            switch (code) {
            case Key::W:
            case Key::UP:
                dir = UP_DIR;
                break;
            case Key::S:
            case Key::DOWN:
                dir = DOWN_DIR;
                break;
            case Key::A:
            case Key::LEFT:
                dir = LEFT_DIR;
                break;
            case Key::D:
            case Key::RIGHT:
                dir = RIGHT_DIR;
                break;
            }
            if (dir != NONE) {
                snake.setDirection(dir);
            }
        }
    }

	//основной игровой цикл
    void gameLoop() {
        while (gameStarted) {
            handleInput();
			//движение змейки
            if (snake.getCurrentDirection() != NONE) { 
                COORD newHead = snake.getNewHead(); 
				bool alive = !snake.willCollideWall(game_map, newHead) && !snake.willCollideBody(newHead); //проверка на столкновение
				//если змейка столкнулась с чем-то, игра заканчивается
                if (!alive) {
                    gameOver();
                    return;
                }
                snake.move();
                COORD head = snake.getHead();
                bool ate = apple.isAt(head);
                if (ate) {
                    ++applesEaten;
                    game_map.at(head.Y, head.X) = HALL;
                    updateApplesInfo();
                    snake.grow();
                    apple.spawn(game_map, snake);
                }
                else {
                    snake.eraseTail(h);
                }
                snake.draw(h);
            }
            Sleep(200);
        }
    }

	//конец игры
    void gameOver() {
        system("cls");
        SetConsoleTextAttribute(h, Color::RED);
        COORD pos;
        pos.X = width / 2 - 5;
        pos.Y = height / 3;
        SetConsoleCursorPosition(h, pos);
        cout << "GAME OVER";
        cout << "\n\n";
        pos.Y += 2;
        SetConsoleCursorPosition(h, pos);
        cout << "Your score: " << applesEaten;
        cout << "\n\n";
        pos.Y += 2;
        SetConsoleCursorPosition(h, pos);
        cout << "Returning to main menu...\n\n\n\n";
        scores.saveScore(applesEaten);
        Sleep(5000);
        system("cls");
        gameStarted = false;
    }

	//выход из игры
    void exitGame() {
        system("cls");
        SetConsoleTextAttribute(h, Color::DARKGREEN);
        COORD pos;
        pos.X = width / 2 - 5;
        pos.Y = height / 2;
        SetConsoleCursorPosition(h, pos);
        cout << "Exiting game...\n\n\n\n";
        Sleep(500);
        gameStarted = false;
    }

public:
	//конструктор игры
    Game(int y, int x) : height(y), width(x), game_map(y, x), snake(x / 2, y / 2), applesEaten(0), gameStarted(false) {
        h = GetStdHandle(STD_OUTPUT_HANDLE);
        setupConsole();
    }

	//отображение главного меню
    void showMenu() {
        int choice;
        do {
            system("cls");
            cout << "=====================\n";
            cout << "  Welcome to snake!\n";
            cout << "=====================\n";
            cout << "1. Play\n";
            cout << "2. Records\n";
            cout << "3. Clear records\n";
            cout << "4. Exit\n";
            cout << "Choose an option: ";
            cin >> choice;
            switch (choice) {
            case 1:
                cout << "Starting game...\n";
                Sleep(1000);
                system("cls");
                startGame();
                break;
            case 2:
                system("cls");
                scores.printScores();
                Sleep(2000);
                cout << "\n\nPress any key and you will be redirected to the main menu...\n";
                _getch();
                break;
            case 3:
                system("cls");
                int confirm;
                cout << "Are you sure you want to clear the records? (1 - Yes, 0 - No): ";
                cin >> confirm;
                if (confirm == 1) {
                    cout << "Clearing records...\n";
                    Sleep(1000);
                    scores.clearScores();
                    cout << "Records cleared.\n";
                    Sleep(2000);
                }
                else {
                    cout << "Records not cleared.\n";
                    Sleep(2000);
                }
                break;
            case 4:
                cout << "Exiting game...\n";
                Sleep(1000);
                exit(0);
            default:
                cout << "Wrong choice. Try again.\n";
                Sleep(2000);
            }
        } while (true);
    }

	//начало игры
    void startGame() {
        this->game_map.fill();
		applesEaten = 0;
		snake.reset(width / 2, height / 2);
        game_map.print(h);
        apple.spawn(game_map, snake);
        COORD head = snake.getHead();
        game_map.at(head.Y, head.X) = HALL;
        SetConsoleCursorPosition(h, head);
        SetConsoleTextAttribute(h, Color::BLUE);
        cout << "H";
        printGameInfo();
        gameStarted = true;
        gameLoop();
    }
};

int main() {
    Game game(21, 51);
    game.showMenu();
    return 0;
}