// game_field.h - класс «Игровое поле»: состояние объектов, столкновения,
// запуск игрового цикла (метод runGameLoop - требование ТЗ).
#ifndef GAME_FIELD_H
#define GAME_FIELD_H

#include <vector>
#include <memory>
#include <random>
#include "figures.h"
#include "python_snake.h"
#include "database.h"

class GameField {
public:
    static constexpr int WIDTH  = 30; // клеток по горизонтали
    static constexpr int HEIGHT = 18; // клеток по вертикали
    static constexpr int OBSTACLE_COUNT = 8; // препятствий на поле
    static constexpr int FOOD_MAX       = 5; // одновременно добычи на поле

    // Имена - отображаемые имена вошедших игроков; p1Id/p2Id - ID_USER из базы USERS
    GameField(Database& db, const std::string& p1Name, int p1Id,
              const std::string& p2Name, int p2Id);

    // === Метод запуска игрового цикла (требование ТЗ) ===
    void runGameLoop();

    // Логика одного такта (отделена для тестируемости)
    void step();
    bool gameOver() const { return over_; }

    // Отрисовка текущего состояния поля (через виртуальный Figure::symbol())
    void render() const;

    // Обработка одной команды ввода; возвращает false, если запрошен выход ('q')
    bool handleKey(char key);

    Python& python1() { return *py1_; }
    Python& python2() { return *py2_; }

private:
    Point randomFreeCell() const;
    void  spawnFood();
    void  placeObstacles();
    bool  isObstacle(const Point& p) const;
    Food* foodAt(const Point& p) const;
    void  endGameAndSave();   // сохранение результатов партии в RATING (п.1.1.4)

    Database& db_;
    int  id1_, id2_;
    bool paused_ = false;
    bool over_   = false;
    int  tickMs_ = 180;                 // скорость игры (мс на такт)
    std::unique_ptr<Python> py1_, py2_;
    std::vector<std::unique_ptr<Obstacle>> obstacles_;
    std::vector<std::unique_ptr<Food>>     foods_;
    mutable std::mt19937 rng_{ std::random_device{}() };
};

#endif // GAME_FIELD_H
