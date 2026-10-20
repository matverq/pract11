#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <string>
#include <fstream>

using namespace std;

const int maxRobots = 10;
const int maxTests = 100;

int m_tests[maxTests];
int m_testcount;

struct Robot {
    int id;
    int energy;
    int score;
    bool finish;
    bool paused;
    
    HANDLE hThread;
    DWORD threadId;
};

DWORD WINAPI RobotThread(LPVOID lpParam){
    Robot* r = static_cast<Robot*>(lpParam);
    cout << "Робот " << r->id << " начал испытание\n";

    for (int i = 0; i < m_testcount; i++)
    {
        int type = m_tests[i];
        int duration = 0;
        bool canPas = true;

        switch (type)
        {
        case 1: duration = 3; canPas = (r->energy >= 10); break;
        case 2: duration = 4; canPas = (r->energy >= 20); break;
        case 3: duration = 1+ rand()%5; canPas = true; break;
        case 4: duration = 10; canPas = (r->energy >= 60); break;
        default:
            break;
        }

        if (!canPas) {
            cout << "Робот " << r->id << " пропустил испытание " << i + 1 << " мало энергии\n";
        }
        else {
            cout << "Робот " << r->id << " выполняет испытание " << i + 1 << duration << " секунд\n";
        }

        Sleep(duration * 1000);

        switch (type)
        {
        case 1: r->energy -= 10; r->score += 15; break;
        case 2: r->energy -= 20; r->score += 30; break;
        case 3: r->energy += duration * 10; if (r->energy > 100) r->energy = 100; break;
        case 4: r->energy -= 60; r->score += 55; break;
        }

        string mes = "Робот " + to_string(r->id) + " завершил испытание " + to_string(i + 1) + " энергия: " + to_string(r->energy) + " очки: " + to_string(r->score);
        cout << mes << "\n";
        if (i + 1 < m_testcount) Sleep((1 + rand() % 3)*1000);
    }
    r->finish = true;
    cout << "Робот " << r->id << " закончилвсе испытания, очков: " << r->score<<"\n";
    return 0;
};

int main()
{
    setlocale(LC_ALL, "Russian");
    srand(static_cast<int>(time(NULL)));
    
    ifstream file("arena.dat", ios::binary);
    if (!file) {
        cout << "файл не найден";
        return 1;
    }

       
}