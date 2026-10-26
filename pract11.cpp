#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <fstream>

using namespace std;

const int maxRobots = 10;

struct Robot {
    int id = 0;
    int energy = 60;
    int score = 0;
    bool finish = false;
    bool paused = false;
    int* tests;
    int testCount;

    HANDLE hThread = NULL;
};

DWORD WINAPI RobotThread(LPVOID lpParam) {
    Robot* r = static_cast<Robot*>(lpParam);

    srand(time(NULL) + r->id);
    Sleep(r->id * 200);

    string mes1 = "Робот " + to_string(r->id) + " начал испытание\n";
    cout << mes1;

    for (int i = 0; i < r->testCount; i++)
    {
        int type = r->tests[i];
        int duration = 0;
        bool canPas = true;

        switch (type)
        {
        case 1: duration = 3; canPas = (r->energy >= 10); break;
        case 2: duration = 4; canPas = (r->energy >= 20); break;
        case 3: duration = 1 + rand() % 5; canPas = true; break;
        case 4: duration = 10; canPas = (r->energy >= 60); break;
        default: continue;
        }

        if (!canPas) {
            string mes2 = "Робот " + to_string(r->id) + " пропустил испытание (" + to_string(i + 1) + ") мало энергии\n";
            cout << mes2;
        }
        else {
            string mes3 = "Робот " + to_string(r->id) + " выполняет испытание (" + to_string(i + 1) + ") " + to_string(duration) + " секунд\n";
            cout << mes3;

            Sleep(duration * 1000);

            switch (type)
            {
            case 1: r->energy -= 10; r->score += 15; break;
            case 2: r->energy -= 20; r->score += 30; break;
            case 3: r->energy += duration * 10; if (r->energy > 100) r->energy = 100; break;
            case 4: r->energy -= 60; r->score += 55; break;
            }

            string mes4 = "Робот " + to_string(r->id) + " завершил испытание (" + to_string(i + 1) + ") энергия: " + to_string(r->energy) + " очки: " + to_string(r->score) + "\n";
            cout << mes4;
        }

        if (i + 1 < r->testCount) Sleep((1 + rand() % 3) * 1000);
    }
    r->finish = true;

    string mes5 = "Робот " + to_string(r->id) + " закончил все испытания, очков: " + to_string(r->score) + "\n";
    cout << mes5;
    return 0;
};

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
        cin.ignore();
    }
    Robot* robots = new Robot[numRobots];

    for (int i = 0; i < numRobots; i++) {
        robots[i].id = i + 1;
        robots[i].tests = allTests;
        robots[i].testCount = totalTests;

        robots[i].hThread = CreateThread(NULL, 0, RobotThread, &robots[i], 0, NULL);
    }
    cout << "Нажмите цифру робота (1-9) для паузы/продолжения.\n";

    while (true) {
        bool allFinished = true;
        for (int i = 0; i < numRobots; i++) {
            if (!robots[i].finish) {
                allFinished = false;
                break;
            }
        }
        if (allFinished) break;
        if (_kbhit()) {
            char c = _getch();
            int idx = c - '1';

            if (idx >= 0 && idx < numRobots && !robots[idx].finish) {
                if (!robots[idx].paused) {
                    SuspendThread(robots[idx].hThread);
                    robots[idx].paused = true;
                    cout << "Робот " << idx + 1 << " на паузе\n";
                }
                else {
                    ResumeThread(robots[idx].hThread);
                    robots[idx].paused = false;
                    cout << "Робот " << idx + 1 << " продолжает\n";
                }
            }
        }
        Sleep(50);
    }

    for (int i = 0; i < numRobots; i++) {
        WaitForSingleObject(robots[i].hThread, INFINITE);
        CloseHandle(robots[i].hThread);
    }

    for (int i = 0; i < numRobots - 1; i++) {
        for (int j = 0; j < numRobots - i - 1; j++) {
            if (robots[j].score < robots[j + 1].score) {
                Robot temp = robots[j];
                robots[j] = robots[j + 1];
                robots[j + 1] = temp;
            }
        }
    }
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
