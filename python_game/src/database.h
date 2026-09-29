// database.h - классы работы с базами USERS и RATING (см. п.1.1.2 ТЗ)
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <cstdint>

// ---------- База USERS (Таблица 1 ТЗ) ----------
struct User {
    int         idUser      = 0;   // ID_USER: положительное, уникальное
    std::string login;             // LOGIN: латинские буквы и цифры, до 30, уникальный
    std::string passwordHash;      // PASSWORD: хранится в закрытом виде (FNV-1a)
    std::string playerName;        // PLAYER_NAME: русские/латинские буквы и цифры, до 50

    static std::string hashPassword(const std::string& pwd);
    static bool isLoginValid(const std::string& login); // только латиница/цифры, непустой, <=30
    static bool isNameValid(const std::string& name);   // непустое, <=50 байт UTF-8
};

// ---------- База RATING (Таблица 2 ТЗ) ----------
struct RatingRecord {
    int         idRecord   = 0;    // ID_RECORD: положительное, уникальное
    int         idUser     = 0;    // ID_USER: ссылка на USERS
    int         score      = 0;    // SCORE: >= 0
    int         foodCount  = 0;    // FOOD_COUNT: >= 0
    std::string gameDate;          // GAME_DATE: ДД.ММ.ГГГГ
    std::string result;            // RESULT: победа / поражение / ничья
    std::string playerLogin;       // логин для вывода отчета рейтинга
};

class Database {
public:
    bool load();                                   // читает data/users.txt и data/rating.txt
    bool save() const;                             // сохраняет обе базы

    // Регистрация нового игрока с проверкой ограничений из Таблицы 1
    bool registerUser(const std::string& login, const std::string& password,
                      const std::string& playerName, std::string& error);
    // Вход: возвращает указатель на пользователя или nullptr
    User* login(const std::string& userLogin, const std::string& password);

    void addRatingRecord(int idUser, int score, int foodCount, const std::string& result);
    // Записи, отсортированные по SCORE (убыв.) - п.1.1.4 ТЗ
    std::vector<RatingRecord> getRatingSorted() const;

    const std::vector<User>& users() const { return users_; }

private:
    int nextUserId()   const;
    int nextRecordId() const;

    std::vector<User>         users_;
    std::vector<RatingRecord> rating_;
};

#endif // DATABASE_H
