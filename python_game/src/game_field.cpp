// game_field.cpp - реализация класса «Игровое поле» (п.1.1.1 ТЗ)
#include "game_field.h"
#include "console_io.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>

GameField::GameField(Database& db, const std::string& p1Name, int p1Id,
                     const std::string& p2Name, int p2Id)
    : db_(db), id1_(p1Id), id2_(p2Id) {
    // На игровом поле одновременно существуют два объекта «Питон»,
    // каждый из которых управляется своим игроком (требование ТЗ).
    py1_ = std::make_unique<Python>(p1Name, Point{5, HEIGHT / 2}, DIR_RIGHT);
    py2_ = std::make_unique<Python>(p2Name, Point{WIDTH - 6, HEIGHT / 2}, DIR_LEFT);
    placeObstacles();
    for (int i = 0; i < FOOD_MAX; ++i) spawnFood();
}

Point GameField::randomFreeCell() const {
    for (int tries = 0; tries < 1000; ++tries) {
        Point p{ std::uniform_int_distribution<int>(0, WIDTH - 1)(rng_),
                 std::uniform_int_distribution<int>(0, HEIGHT - 1)(rng_) };
        if (py1_->occupies(p) || py2_->occupies(p)) continue;
        if (isObstacle(p)) continue;
        bool foodHere = false;
        for (const auto& f : foods_) if (f->pos() == p) { foodHere = true; break; }
        if (!foodHere) return p;
    }
    return Point{-1, -1};
}

void GameField::placeObstacles() {
    obstacles_.clear();
    for (int i = 0; i < OBSTACLE_COUNT; ++i) {
        Point p = randomFreeCell();
        if (p.x < 0) break;
        obstacles_.push_back(std::make_unique<Obstacle>(p));
    }
}

void GameField::spawnFood() {
    if ((int)foods_.size() >= FOOD_MAX) return;
    Point p = randomFreeCell();
    if (p.x < 0) return;
    std::uniform_int_distribution<int> type(0, 2);
    switch (type(rng_)) {
        case 0:  foods_.push_back(std::make_unique<Mouse>(p));  break; // мышь
        case 1:  foods_.push_back(std::make_unique<Rabbit>(p)); break; // кролик
        default: foods_.push_back(std::make_unique<Cow>(p));    break; // корова
    }
}

bool GameField::isObstacle(const Point& p) const {
    for (const auto& o : obstacles_) if (o->pos() == p) return true;
    return false;
}

Food* GameField::foodAt(const Point& p) const {
    for (const auto& f : foods_) if (f->pos() == p) return f.get();
    return nullptr;
}

// ---------- Логика одного такта цикла игры ----------
void GameField::step() {
    if (py1_->alive()) py1_->nextStep();
    if (py2_->alive()) py2_->nextStep();

    // Встречные головы: оба питона погибают
    if (py1_->alive() && py2_->alive() && py1_->head() == py2_->head()) {
        py1_->kill();
        py2_->kill();
    }

    for (int i = 0; i < 2; ++i) {
        Python& py    = (i == 0) ? *py1_ : *py2_;
        Python& other = (i == 0) ? *py2_ : *py1_;
        if (!py.alive()) continue;
        Point h = py.head();

        // Смена направления происходит при столкновении с препятствием или
        // границей поля - в данной реализации такое столкновение приводит к
        // гибели питона (поражению), что соответствует п.1.1.1 ТЗ.
        if (h.x < 0 || h.x >= WIDTH || h.y < 0 || h.y >= HEIGHT) { py.kill(); continue; }
        if (isObstacle(h)) { py.kill(); continue; }
        if (py.occupiesAllExceptHeadTail(h)) { py.kill(); continue; }   // своё тело
        if (other.alive() && other.occupies(h)) { py.kill(); continue; } // чужой питон

        // Захват добычи: начисление очков и рост тела
        if (Food* f = foodAt(h)) {
            py.addScore(f->points());
            py.grow(f->growth());
            py.ateFood();
            foods_.erase(std::remove_if(foods_.begin(), foods_.end(),
                        [&](const std::unique_ptr<Food>& up) { return up.get() == f; }),
                         foods_.end());
            spawnFood();
        }
    }

    if (!py1_->alive() || !py2_->alive()) over_ = true;
}

// ---------- Сохранение результатов партии в базу RATING (п.1.1.4 ТЗ) ----------
void GameField::endGameAndSave() {
    static const std::string WIN  = "\320\277\320\276\320\261\320\265\320\264\320\260";
    static const std::string LOSE = "\320\277\320\276\321\200\320\260\320\266\320\265\320\275\320\270\320\265";
    static const std::string DRAW = "\320\275\320\270\321\207\321\214\321\217";

    std::string res1, res2;
    if (py1_->score() > py2_->score())      { res1 = WIN;  res2 = LOSE; }
    else if (py1_->score() < py2_->score()) { res1 = LOSE; res2 = WIN;  }
    else                                    { res1 = DRAW; res2 = DRAW; }

    db_.addRatingRecord(id1_, py1_->score(), py1_->foodCount(), res1);
    db_.addRatingRecord(id2_, py2_->score(), py2_->foodCount(), res2);
    db_.save();
}

// ---------- Отрисовка состояния через виртуальную функцию Figure::symbol() ----------
void GameField::render() const {
    static const char* DOT     = "\xc2\xb7";       // пустая клетка
    static const char* WALL_H  = "\xe2\x94\x80";   // горизонталь рамки
    static const char* WALL_V  = "\xe2\x94\x82";   // вертикаль рамки
    static const char* WALL_TL = "\xe2\x94\x8c";
    static const char* WALL_TR = "\xe2\x94\x90";
    static const char* WALL_BL = "\xe2\x94\x94";
    static const char* WALL_BR = "\xe2\x94\x98";

    std::string out;
    out += "\033[2J\033[H"; // очистка экрана, курсор в начало

    out += WALL_TL;
    for (int x = 0; x < WIDTH; ++x) out += WALL_H;
    out += WALL_TR;
    out += "\n";

    for (int y = 0; y < HEIGHT; ++y) {
        out += WALL_V;
        for (int x = 0; x < WIDTH; ++x) {
            Point p{x, y};
            std::string cell;
            if (py1_->alive() && py1_->head() == p)       cell = py1_->headSymbol();
            else if (py2_->alive() && py2_->head() == p)  cell = py2_->headSymbol();
            else if (py1_->occupiesAllExceptHeadTail(p))  cell = py1_->bodySymbol();
            else if (py2_->occupiesAllExceptHeadTail(p))  cell = py2_->bodySymbol();
            else if (py1_->tailPos() == p)                cell = py1_->tailSymbol();
            else if (py2_->tailPos() == p)                cell = py2_->tailSymbol();
            else if (isObstacle(p))                       cell = Obstacle(p).symbol();
            else if (const Food* f = foodAt(p))           cell = f->symbol(); // виртуальный вызов
            else                                          cell = DOT;
            out += cell;
            out += ' '; // эмодзи занимает 2 колонки терминала
        }
        out += WALL_V;
        out += "\n";
    }

    out += WALL_BL;
    for (int x = 0; x < WIDTH; ++x) out += WALL_H;
    out += WALL_BR;
    out += "\n\n";

    out += "  P1 " + py1_->ownerName() +
           "   score: " + std::to_string(py1_->score()) +
           "   len: "   + std::to_string(py1_->length()) + "\n";
    out += "  P2 " + py2_->ownerName() +
           "   score: " + std::to_string(py2_->score()) +
           "   len: "   + std::to_string(py2_->length()) + "\n\n";
    out += "  P1: W A S D    P2: \342\254\206 \342\254\207 \342\254\205 \342\236\241   [P] \320\277\320\260\321\203\320\267\320\260   [Q] \320\262\321\213\321\205\320\276\320\264\n";
    if (paused_)
        out += "  \320\237\320\220\320\243\320\227\320\220! \320\235\320\260\320\266\320\274\320\270\321\202\320\265 P \320\264\320\273\321\217 \320\277\321\200\320\276\320\264\320\276\320\273\320\266\320\265\320\275\320\270\321\217\n";

    printUtf8(out);
}

// ---------- Обработка команд управления (Таблица 3 ТЗ) ----------
bool GameField::handleKey(char key) {
    if (key == '\0') return true;
    switch (key) {
        // Игрок 1: WASD
        case 'w': case 'W': py1_->setDirection(DIR_UP);    break;
        case 's': case 'S': py1_->setDirection(DIR_DOWN);  break;
        case 'a': case 'A': py1_->setDirection(DIR_LEFT);  break;
        case 'd': case 'D': py1_->setDirection(DIR_RIGHT); break;
        // Игрок 2: стрелки (readKeyNonBlocking отдаёт коды \x01..\x04)
        case '\x01': py2_->setDirection(DIR_UP);    break;
        case '\x02': py2_->setDirection(DIR_DOWN);  break;
        case '\x03': py2_->setDirection(DIR_LEFT);  break;
        case '\x04': py2_->setDirection(DIR_RIGHT); break;
        // Пауза / Продолжить
        case 'p': case 'P': paused_ = !paused_; break;
        // Выход из партии (результаты сохраняются)
        case 'q': case 'Q': return false;
        default: break;
    }
    return true;
}

// ===== ЗАПУСК ЦИКЛА ИГРЫ - метод класса «Игровое поле» (требование ТЗ) =====
void GameField::runGameLoop() {
    enableRawMode();
    render();
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (true) {
        // Команды двух игроков обрабатываются независимо (Таблица 3 ТЗ)
        char k;
        while ((k = readKeyNonBlocking()) != '\0') {
            if (!handleKey(k)) {           // команда выхода 'q'
                disableRawMode();
                endGameAndSave();
                return;
            }
        }

        auto now = clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() >= tickMs_) {
            last = now;
            if (!paused_) {
                step();
                render();
                if (over_) {               // завершение игрового цикла
                    disableRawMode();
                    endGameAndSave();
                    return;
                }
            } else {
                render();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
