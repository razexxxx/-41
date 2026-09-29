// python_snake.cpp - реализация класса «Питон»
#include "python_snake.h"

Python::Python(const std::string& ownerName, const Point& headPos, Dir dir)
    : ownerName_(ownerName), dir_(dir), nextDir_(dir) {
    // Начальное тело из 3 клеток позади головы
    body_.push_back(headPos);
    for (int i = 1; i < 3; ++i) {
        Point p = headPos;
        switch (dir) {
            case DIR_UP:    p.y += i; break;
            case DIR_DOWN:  p.y -= i; break;
            case DIR_LEFT:  p.x += i; break;
            case DIR_RIGHT: p.x -= i; break;
        }
        body_.push_back(p);
    }
}

bool Python::setDirection(Dir d) {
    // Запрет мгновенного разворота на 180 градусов (Таблица 3 ТЗ)
    if ((d == DIR_UP    && dir_ == DIR_DOWN)  ||
        (d == DIR_DOWN  && dir_ == DIR_UP)    ||
        (d == DIR_LEFT  && dir_ == DIR_RIGHT) ||
        (d == DIR_RIGHT && dir_ == DIR_LEFT))
        return false;
    nextDir_ = d;
    return true;
}

void Python::nextStep() {
    dir_ = nextDir_;
    Point np = body_.front();
    switch (dir_) {
        case DIR_UP:    --np.y; break;
        case DIR_DOWN:  ++np.y; break;
        case DIR_LEFT:  --np.x; break;
        case DIR_RIGHT: ++np.x; break;
    }
    body_.push_front(np);
    if (pendingGrowth_ > 0) --pendingGrowth_; // хвост не убираем -> рост
    else                    body_.pop_back();
}

bool Python::occupies(const Point& p) const {
    for (const auto& b : body_)
        if (b == p) return true;
    return false;
}

bool Python::occupiesAllExceptHeadTail(const Point& p) const {
    // Все клетки тела, кроме головы (index 0) и хвоста (last).
    // Используется и для отрисовки «фрагментов», и для проверки
    // столкновения со своим телом (хвост освобождает клетку на этом такте).
    for (size_t i = 1; i + 1 < body_.size(); ++i)
        if (body_[i] == p) return true;
    return false;
}

std::string Python::headSymbol() const {
    Head h(body_.front());
    return h.symbol();          // вызов виртуальной функции отображения
}

std::string Python::bodySymbol() const {
    BodyPart b(Point{});
    return b.symbol();
}

std::string Python::tailSymbol() const {
    Tail t(body_.back());
    return t.symbol();
}
