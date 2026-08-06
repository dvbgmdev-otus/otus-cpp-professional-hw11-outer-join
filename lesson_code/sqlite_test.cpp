// sqlite_test.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "sqlite3.h"




void CallbackTest()
{
    std::cout << "EXECUTED!" << std::endl;
}


int SelectCallback(void* data, int num, char** a, char** b)
{
    int debug = 10;
    std::cout << "==========TABLE=================" << std::endl;

    

    for (int i = 0; i < num; ++i) {
        std::cout << a[i] << std::endl;
    }

    for (int i = 0; i < num; ++i) {
        std::cout << b[i] << std::endl;
    }

    std::cout << "==========END=================" << std::endl;

    return 0;
}

int SelectCallback2(void* data, int num, char** a, char** b)
{
    int debug = 10;
    std::cout << "==========TABLE=================" << std::endl;


    for (int i = 0; i < num; ++i) {
        std::cout << a[i] << std::endl;
    }

    for (int i = 0; i < num; ++i) {
        std::cout << b[i] << std::endl;
    }

    std::cout << "==========END=================" << std::endl;

    return 1;
}


struct MyData{};




int main()
{
    sqlite3* db = nullptr;
    int open_result = sqlite3_open("TestDB",&db);
    std::cout << "OPEN_RESULT:" << open_result << std::endl;

    if (open_result) {
        std::cout << "Unable to open DB!" <<  std::endl;
    }


    

    char* erm = nullptr;

    //0x0000020d656e47d0

    int exec = sqlite3_exec(db, "CREATE TABLE TestTable(a INT,b TEXT, c TEXT)",0,0,&erm);

    int exec2 = sqlite3_exec(db, "CREATE TABLE TestTable(a INT,b TEXT, c TEXT)", 0, 0, &erm);

    //int insert = sqlite3_exec(db, "INSERT INTO TestTable VALUES('10','PRIVET','PRIVET!')", 0, 0, &erm);

    //insert = sqlite3_exec(db, "INSERT INTO TestTable VALUES(10,'PRIVET2','PRIVET2!')", 0, 0, &erm);




    /*std::cout << "EXEC_RESULT:" << exec << std::endl;
    std::cout << "INSERT_RESULT:" << insert << std::endl;*/

    //int select1 = sqlite3_exec(db, "SELECT a,b,c FROM TestTable", SelectCallback, 0, &erm);
    int select2 = sqlite3_exec(db, "SELECT* FROM TestTable", SelectCallback2, 0, &erm);


    int close_result = sqlite3_close(db);

    std::cout << "CLOSE_RESULT:" << close_result << std::endl;



}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
