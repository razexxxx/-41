// database.cpp - реализация баз USERS и RATING (п.1.1.2 ТЗ)
#include "database.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <filesystem>

// ---------- User ----------
std::string User::hashPassword(const std::string& pwd) {
    // FNV-1a 64-bit + "соль" приложения: пароль в открытом виде не хранится
    uint64_t h = 1469598103934665603ULL;
    const std::string salted = pwd + "::python-game-salt";
    for (unsigned char c : salted) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(h));
    return buf;
}

bool User::isLoginValid(const std::string& login) {
    if (login.empty() || login.size() > 30) return false;
    for (char c : login)
        if (!std::isalnum(static_cast<unsigned char>(c))) return false; // латиница + цифры
    return true;
}

bool User::isNameValid(const std::string& name) {
    // допускаются русские/латинские буквы и цифры (UTF-8), непустое, <=50 байт
    if (name.empty() || name.size() > 50) return false;
    for (unsigned char c : name)
        if (c < 0x20 || c == 0x7f) return false; // без управляющих символов
    return true;
}

// ---------- Database ----------
int Database::nextUserId() const {
    int m = 0;
    for (const auto& u : users_) m = std::max(m, u.idUser);
    return m + 1;
}

int Database::nextRecordId() const {
    int m = 0;
    for (const auto& r : rating_) m = std::max(m, r.idRecord);
    return m + 1;
}

static std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> out;
    std::stringstream ss(line);
    std::string tok;
    while (std::getline(ss, tok, '|')) out.push_back(tok);
    return out;
}

bool Database::load() {
    users_.clear();
    rating_.clear();
    namespace fs = std::filesystem;

    if (fs::exists("data/users.txt")) {
        std::ifstream f("data/users.txt");
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto t = splitLine(line);
            if (t.size() >= 4) {
                User u;
                u.idUser       = std::stoi(t[0]);
                u.login        = t[1];
                u.passwordHash = t[2];
                u.playerName   = t[3];
                users_.push_back(u);
            }
        }
    }
    if (fs::exists("data/rating.txt")) {
        std::ifstream f("data/rating.txt");
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto t = splitLine(line);
            if (t.size() >= 6) {
                RatingRecord r;
                r.idRecord    = std::stoi(t[0]);
                r.idUser      = std::stoi(t[1]);
                r.score       = std::stoi(t[2]);
                r.foodCount   = std::stoi(t[3]);
                r.gameDate    = t[4];
                r.result      = t[5];
                r.playerLogin = (t.size() >= 7) ? t[6] : "";
                rating_.push_back(r);
            }
        }
    }
    return true; // пустые базы допустимы (первый запуск)
}

bool Database::save() const {
    std::filesystem::create_directories("data");
    {
        std::ofstream f("data/users.txt", std::ios::trunc);
        f << "#ID_USER|LOGIN|PASSWORD_HASH|PLAYER_NAME\n";
        for (const auto& u : users_)
            f << u.idUser << '|' << u.login << '|' << u.passwordHash
              << '|' << u.playerName << '\n';
    }
    {
        std::ofstream f("data/rating.txt", std::ios::trunc);
        f << "#ID_RECORD|ID_USER|SCORE|FOOD_COUNT|GAME_DATE|RESULT|LOGIN\n";
        for (const auto& r : rating_)
            f << r.idRecord << '|' << r.idUser << '|' << r.score << '|'
              << r.foodCount << '|' << r.gameDate << '|' << r.result << '|'
              << r.playerLogin << '\n';
    }
    return true;
}

bool Database::registerUser(const std::string& login, const std::string& password,
                            const std::string& playerName, std::string& error) {
    if (!User::isLoginValid(login)) {
        error = "Login must be non-empty, <=30 chars, latin letters and digits only.";
        return false;
    }
    if (password.empty() || password.size() > 30) {
        error = "Password must be non-empty and <=30 chars.";
        return false;
    }
    if (!User::isNameValid(playerName)) {
        error = "Player name must be non-empty and <=50 chars.";
        return false;
    }
    for (const auto& u : users_)
        if (u.login == login) { error = "This login already exists."; return false; }

    User u;
    u.idUser       = nextUserId();
    u.login        = login;
    u.passwordHash = User::hashPassword(password);
    u.playerName   = playerName;
    users_.push_back(u);
    return true;
}

User* Database::login(const std::string& userLogin, const std::string& password) {
    const std::string hash = User::hashPassword(password);
    for (auto& u : users_)
        if (u.login == userLogin && u.passwordHash == hash)
            return &u;
    return nullptr;
}

void Database::addRatingRecord(int idUser, int score, int foodCount, const std::string& result) {
    RatingRecord r;
    r.idRecord  = nextRecordId();
    r.idUser    = idUser;
    r.score     = std::max(0, score);
    r.foodCount = std::max(0, foodCount);
    r.result    = result;

    // GAME_DATE: ДД.ММ.ГГГГ
    std::time_t now = std::time(nullptr);
    std::tm tmv{};
    localtime_r(&now, &tmv);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d.%02d.%04d",
                  tmv.tm_mday, tmv.tm_mon + 1, tmv.tm_year + 1900);
    r.gameDate = buf;

    for (const auto& u : users_)
        if (u.idUser == idUser) { r.playerLogin = u.login; break; }

    rating_.push_back(r);
}

std::vector<RatingRecord> Database::getRatingSorted() const {
    auto v = rating_;
    std::sort(v.begin(), v.end(), [](const RatingRecord& a, const RatingRecord& b) {
        return a.score > b.score; // сортировка по SCORE (п.1.1.4 ТЗ)
    });
    return v;
}
