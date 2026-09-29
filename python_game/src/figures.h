// figures.h - иерархия отображаемых объектов (Таблица 4 ТЗ)
// Класс «Фигура» содержит ВИРТУАЛЬНУЮ функцию отображения symbol().
#ifndef FIGURES_H
#define FIGURES_H

#include <string>

struct Point {
    int x = 0, y = 0;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const { return !(*this == o); }
};

enum Dir { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

// ---------- Базовый класс «Фигура» ----------
class Figure {
public:
    virtual ~Figure() = default;
    // Виртуальная функция отображения (требование ТЗ):
    // возвращает символ, которым фигура выводится на экран.
    virtual std::string symbol() const = 0;
    Point pos() const { return pos_; }
    void  setPos(const Point& p) { pos_ = p; }
protected:
    Point pos_;
};

// ---------- Класс «Препятствие» ----------
class Obstacle : public Figure {
public:
    explicit Obstacle(const Point& p) { pos_ = p; }
    std::string symbol() const override { return "\360\237\247\212"; } // ледяная глыба
};

// ---------- Иерархия добычи: Добыча -> Мышь / Кролик / Корова ----------
class Food : public Figure {
public:
    Food(const Point& p, int points, int growth)
        : points_(points), growth_(growth) { pos_ = p; }
    int points() const { return points_; } // очки за захват
    int growth() const { return growth_; } // на сколько сегментов вырастет питон
protected:
    int points_, growth_;
};

class Mouse : public Food {
public:
    explicit Mouse(const Point& p) : Food(p, 10, 1) {}
    std::string symbol() const override { return "\360\237\220\255"; }
};

class Rabbit : public Food {
public:
    explicit Rabbit(const Point& p) : Food(p, 25, 2) {}
    std::string symbol() const override { return "\360\237\220\260"; }
};

class Cow : public Food {
public:
    explicit Cow(const Point& p) : Food(p, 50, 3) {}
    std::string symbol() const override { return "\360\237\220\204"; }
};

// ---------- Иерархия питона: Участок тела -> Голова / Фрагмент тела / Хвост ----------
class BodyPart : public Figure {          // «Фрагмент тела»
public:
    explicit BodyPart(const Point& p) { pos_ = p; }
    std::string symbol() const override { return "\360\237\237\242"; }
};

class Head : public BodyPart {            // «Голова»
public:
    explicit Head(const Point& p) : BodyPart(p) {}
    std::string symbol() const override { return "\360\237\220\262"; }
};

class Tail : public BodyPart {            // «Хвост»
public:
    explicit Tail(const Point& p) : BodyPart(p) {}
    std::string symbol() const override { return "\360\237\237\242"; }
};

#endif // FIGURES_H
