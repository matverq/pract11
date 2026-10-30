#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <fstream>

using namespace std;

const int maxRobots = 10;

struct Robot {
    int id;
    int energy = 60;
    int score = 0;
    bool finish = false;
    bool paused = false;
    int* tests;
    int testCount;

    HANDLE hThread = NULL;
};

void Obstacle(Robot* r) {
    if (r->energy < 10) {
        cout << "Робот " << to_string(r->id) << " пропустил Препятствие (мало энергии)\n";
        return;
    }
    cout << "Робот " << to_string(r->id) << " выполняет Препятствие (3 сек)\n";
    Sleep(3000);

    r->energy -= 10;
    r->score += 15;

    cout << "Робот " << to_string(r->id) << " завершил Препятствие. Энергия: " << to_string(r->energy) << ", Очки: " << to_string(r->score) << "\n";
}

void Sprint(Robot* r) {
    if (r->energy < 20) {
        cout << "Робот " << to_string(r->id) << " пропустил Спринт (мало энергии)\n";
        return;
    }
    cout << "Робот " << to_string(r->id) << " выполняет Спринт (4 сек)\n";
    Sleep(4000);

    r->energy -= 20;
    r->score += 30;

    cout << "Робот " << to_string(r->id) << " завершил Спринт. Энергия: " << to_string(r->energy) << ", Очки: " << to_string(r->score) << "\n";
}

void Recharge(Robot* r) {
    int time = 1 + rand() % 5;
    cout << "Робот " << to_string(r->id) << " выполняет Подзарядку (" << to_string(time) << " сек)\n";
    Sleep(time * 1000);

    r->energy += time * 10;
    if (r->energy > 100) r->energy = 100;

    cout << "Робот " << to_string(r->id) << " завершил Подзарядку. Энергия: " << to_string(r->energy) << ", Очки: " << to_string(r->score) << "\n";
}

void Marathon(Robot* r) {
    if (r->energy < 60) {
        cout << "Робот " << to_string(r->id) << " пропустил Марафон (мало энергии)\n";
        return;
    }
    cout << "Робот " << to_string(r->id) << " выполняет Марафон (10 сек)\n";

    Sleep(10000);

    r->energy -= 60;
    r->score += 55;

    cout << "Робот " << to_string(r->id) << " завершил Марафон. Энергия: " << to_string(r->energy) << ", Очки: " << to_string(r->score) << "\n";
}

DWORD WINAPI RobotThread(LPVOID lpParam) {
    Robot* r = static_cast<Robot*>(lpParam);

    srand(time(NULL) + r->id);
    Sleep(r->id* 200);

    cout << "Робот " << to_string(r->id) << " начал испытание\n";

    for (int i = 0; i < r->testCount; i++)
    {
        switch (r->tests[i])
        {
        case 1: Obstacle(r); break;
        case 2: Sprint(r); break;
        case 3: Recharge(r); break;
        case 4: Marathon(r); break;
        default: continue;
        }
        if (i + 1 < r->testCount) Sleep((1 + rand() % 3) * 1000);
    }
    r->finish = true;

    cout << "Робот " + to_string(r->id) << " закончил все испытания, очков: " << to_string(r->score) << "\n";
    return 0;
};

bool allFinished(Robot* robots, int numRobots) {
    for (int i = 0; i < numRobots; i++) {
        if (!robots[i].finish) {
            return false;
        }
    }
    return true;
}

void switchRobot(Robot* robots, int numRobots) {
    if (_kbhit()) {
        char c = _getch();
        int ind = c - '1';
        if (ind >= 0 && ind < numRobots && !robots[ind].finish) {
            if (!robots[ind].paused) {
                SuspendThread(robots[ind].hThread);
                robots[ind].paused = true;
                cout << "Робот " << ind + 1 << " на паузе\n";
            }
            else {
                ResumeThread(robots[ind].hThread);
                robots[ind].paused = false;
                cout << "Робот " << ind + 1 << " продолжает\n";
            }
        }
    }
}

void sortRobots(Robot* robots, int numRobots) {
    for (int i = 0; i < numRobots - 1; i++) {
        for (int j = 0; j < numRobots - i - 1; j++) {
            if (robots[j].score < robots[j + 1].score) {
                Robot temp = robots[j];
                robots[j] = robots[j + 1];
                robots[j + 1] = temp;
            }
        }
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");
    ifstream file("arena.dat", ios::binary);
    if (!file) {
        cout << "Файл не найден\n";
        return 1;
    }

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);

    int totalTests = fileSize / sizeof(int);
    int* allTests = new int[totalTests];

    file.read(reinterpret_cast<char*>(allTests), fileSize);
    file.close();

    cout << "Загружено испытаний: " << totalTests << "\n";

    int numRobots = 0;
    while (numRobots < 3 || numRobots > maxRobots) {
        cout << "Введите число роботов (3-10): ";
        cin >> numRobots;
    }
    Robot* robots = new Robot[numRobots];

    for (int i = 0; i < numRobots; i++) {
        robots[i].id = i + 1;
        robots[i].tests = allTests;
        robots[i].testCount = totalTests;

        robots[i].hThread = CreateThread(NULL, 0, RobotThread, &robots[i], 0, NULL);
    }
    cout << "Нажмите цифру робота (1-9) для паузы/продолжения.\n";

    while (!allFinished(robots, numRobots)) {
        switchRobot(robots, numRobots);
        Sleep(50);
    }

    for (int i = 0; i < numRobots; i++) {
        WaitForSingleObject(robots[i].hThread, INFINITE);
        CloseHandle(robots[i].hThread);
    }
    sortRobots(robots, numRobots);
    cout << "\nРезультаты:\n";
    for (int i = 0; i < numRobots; i++) {
        cout << "Робот " << robots[i].id << ": Очки = " << robots[i].score << ", Энергия = " << robots[i].energy << "\n";
    }

    if (numRobots > 1 && robots[0].score == robots[1].score) {
        cout << "Ничья\n";
    }
    else {
        cout << "Победитель: Робот " << robots[0].id << "\n";
    }

    delete[] allTests;
    delete[] robots;
}
