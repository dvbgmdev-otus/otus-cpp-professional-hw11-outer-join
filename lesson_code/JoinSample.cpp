// JoinSample.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "sqlite3.h"
#include "windows.h"
#include <vector>

/*
* ========
* SELECT* FROM Occupation
  WHERE Id > 2
* =======
* 
* SELECT* FROM Occupation
WHERE Id > 1
ORDER BY name DESC // Строки так же сортируются

DELETE FROM Country
WHERE Id == '2';

* 
========SQL COMMANDS=============
INSERT INTO PERSON ("Name","OccupationId")
VALUES ("Иван",0);


INSERT INTO PERSON ("Name","OccupationId")
VALUES ("Мария",1);

INSERT INTO PERSON ("Name","OccupationId")
VALUES ("Евгений",2);

INSERT INTO PERSON ("Name","OccupationId")
VALUES ("Светлана",1);


INSERT INTO Occupation("Name")
VALUES("Programmer");

INSERT INTO Occupation("Name")
VALUES("Accountant");

INSERT INTO Occupation("Name")
VALUES("System Administrator");

CREATE TABLE "Cars" (
    "Id"	INTEGER,
    "Model"	TEXT,
    PRIMARY KEY("Id" AUTOINCREMENT)
);

SELECT* FROM Person
LEFT JOIN Occupation
ON Person.OccupationId == Occupation.Id

SELECT* FROM Person
LEFT OUTER JOIN Occupation
ON Person.OccupationId == Occupation.Id

SELECT
Person.Name,
Occupation.Name
FROM Person
LEFT JOIN Occupation
ON Person.OccupationId == Occupation.Id
*/

struct Person
{
    int id;
    std::string name;
    std::string second_name;
    std::string occupation;
    int occupationId;
};

//const char* db_path = "C:\\otus_workspace\\с++pro\\rdbms\\rdbms.db";
const char* db_path = "D:\\rdbms.db";
const char* join_sample = 
"SELECT "
"Person.Name, "
"Occupation.Name "
"FROM Person "
"LEFT JOIN Occupation "
"ON Person.OccupationId == Occupation.Id ";





int PrintCallback2(void* p, int rows, char** rdata, char** cname)
{

    std::cout << "==========DATA ROW=============" << std::endl;
    for (int i = 0; i < rows; ++i) {
        std::cout << cname[i] << ' ';
    }

    std::cout << std::endl;

    for (int i = 0; i < rows; ++i) {


        
        /*std::string str(rdata[i]);
        if (str.size() < 1) return 0;*/
        
        if (!rdata[i]) {
            continue;
        }
        

        std::cout << rdata[i] << ' ';
    }

    std::cout << std::endl;
    std::cout << "==========END DATA ROW=============" << std::endl;
    return 0;
}

int CastExample(void* p, int columns, char** rdata, char** cname)
{
    
    /*  Name              OccupationName
    *   Светлана          Бухгалтер
    *   Евгений           Программист
    */

    std::vector<Person>* v_ptr = static_cast<std::vector<Person>*>(p);

    Person person;

    

    person.name = rdata[0] ? rdata[0] : "";
    person.occupation = rdata[1] ? rdata[1] : "";
    

    v_ptr->push_back(person);


        
    

    std::cout << std::endl;
    std::cout << "==========END DATA ROW=============" << std::endl;
    return 0;
}


void PrintVector(std::vector<Person>* v)
{
    for (int i = 0; i < v->size(); ++i) {
        std::cout << "Name:" << v->at(i).name << " " << "Occupation:" << v->at(i).occupation << std::endl;
    }
}

int main()
{
    
    SetConsoleOutputCP(65001);

    sqlite3* db;

    int exec;
    char* errmsg;
    int open = sqlite3_open(db_path, &db);

    if (open) {
        std::cout << "ERROR OPENING DB!" << std::endl;
        return 0;
    }
    
    
    std::vector<Person>* vector = new std::vector<Person>();

    exec = sqlite3_exec(db, join_sample, CastExample, vector, &errmsg);
    //exec = sqlite3_exec(db, join_sample, PrintCallback2, 0, &errmsg);

    if (exec != 0) {

        std::cout << "errmsg" << std::endl;

        sqlite3_free(errmsg);
    }
    

    //exec = sqlite3_exec(db, "SELECT* FROM Person", PrintCallback2, 0, &errmsg);

    if (exec) {
        std::cout << "ERROR Executing Query!" << std::endl;
    }

    PrintVector(vector);

    int close = sqlite3_close(db);

    if (close) {
        std::cout << "ERROR Closing DB!" << std::endl;
        return 0;
    }



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
