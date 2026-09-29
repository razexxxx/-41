// python_snake.h - класс «Питон» (змейка), управляемый одним игроком
#ifndef PYTHON_SNAKE_H
#define PYTHON_SNAKE_H

#include <deque>
#include <string>
#include "figures.h"

class Python {
public:
    Python(const std::string& ownerName, const Point& headPos, Dir dir);

    // Обработка команды направления игрока.
    // Возвращает false, если команда недопустима (мгновенный разворот на 180).
    bool setDirection(Dir d);

    void nextStep();                       // сдвиг на одну клетку в текущем направлении
    void grow(int segments) { pendingGrowth_ += segments; }

    bool   alive() const { return alive_; }
    void   kill()        { alive_ = false; }

    Point  head() const  { return body_.front(); }
    Point  tailPos() const { return body_.back(); }
    size_t length() const { return body_.size(); }
    int    score() const { return score_; }
    int    foodCount() const { return foodCount_; }
    const std::string& ownerName() const { return ownerName_; }
    Dir    direction() const { return dir_; }

    void addScore(int p) { score_ += p; }
    void ateFood()       { ++foodCount_; }

    bool occupies(const Point& p) const;               // любая клетка тела
    bool occupiesAllExceptHeadTail(const Point& p) const; // без головы и хвоста

    // Отображение частей через виртуальную функцию Figure::symbol()
    std::string headSymbol() const;
    std::string bodySymbol() const;
    std::string tailSymbol() const;

private:
    std::string       ownerName_;
    std::deque<Point> body_;        // front - голова, back - хвост
    Dir               dir_;
    Dir               nextDir_;
    int               pendingGrowth_ = 0;
    int               score_     = 0;
    int               foodCount_ = 0;
    bool              alive_     = true;
};

#endif // PYTHON_SNAKE_H
