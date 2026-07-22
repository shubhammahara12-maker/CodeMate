#include <bits/stdc++.h>
using namespace std;

const string USERS_FILE = "users.txt";
const string QUESTIONS_FILE = "questions.txt";
const string RACES_FILE = "races.txt";
const string REPLAYS_FILE = "replays.txt";

struct User {
    string username;
    string password;
    int totalPoints = 0;
    int racesPlayed = 0;
    int streak = 0;
    int badges = 0;
    bool isMentor = false;
    string inviteCode;
    int wins = 0;
    int losses = 0;
    unordered_map<string, int> topicWins;
    unordered_map<string, int> topicLosses;
    unordered_map<string, int> topicTime;
    int lastActiveDay = 0;
    int votesGiven = 0;
    int totalDifficultyScore = 0;
    string role = "student"; // student | teacher | admin
    string favoriteSubject;
    string enrollmentId;
    string forgotAnswer;
    bool teacherApproved = false;
    vector<string> assignedTopics;

    double getSkillScore() const {
        if (racesPlayed == 0) return 0.0;
        return double(totalPoints) / max(1, racesPlayed);
    }
    string getTier() const {
        double score = getSkillScore();
        if (score < 50) return "Beginner";
        if (score < 120) return "Intermediate";
        return "Advanced";
    }
};

struct Question {
    int id;
    string topic;
    string type;
    string prompt;
    string answer;
    string author;
    int difficulty;
    int votes = 0;
    int sumDifficultyVotes = 0;
};

struct ReplayEvent {
    string user;
    int questionId;
    int timestamp;
    bool solved;
};

static unordered_map<string, User> users;
static vector<Question> questions;

void exportFrontendData();

void clearScreen() {
    cout << "\033[2J\033[H";
}

void pageHeader(const string &title) {
    clearScreen();
    cout << "\033[1;34m";
    cout << "============================================\n";
    cout << "            CodeMate Competitive Arena       \n";
    cout << "============================================\n";
    cout << "\033[1;33m" << title << "\033[0m\n";
    cout << "--------------------------------------------\n";
}

void renderBox(const string &text) {
    cout << "\033[1;32m" << "[ " << text << " ]" << "\033[0m\n";
}

void persistUsers() {
    ofstream out(USERS_FILE);
    for (auto &p : users) {
        const User &u = p.second;
        out << u.username << "|" << u.password << "|" << u.totalPoints << "|" << u.racesPlayed << "|" << u.streak << "|" << u.badges << "|" << (u.isMentor ? 1 : 0) << "|" << u.inviteCode << "|" << u.wins << "|" << u.losses << "|";
        for (auto &it : u.topicWins) out << it.first << ":" << it.second << ",";
        out << "|";
        for (auto &it : u.topicLosses) out << it.first << ":" << it.second << ",";
        out << "|";
        for (auto &it : u.topicTime) out << it.first << ":" << it.second << ",";
        out << "|" << u.lastActiveDay << "|" << u.votesGiven << "|" << u.totalDifficultyScore << "|" << u.role << "|" << u.favoriteSubject << "|" << u.enrollmentId << "|" << u.forgotAnswer << "|" << (u.teacherApproved ? 1 : 0) << "|";
        for (int i = 0; i < u.assignedTopics.size(); ++i) {
            out << u.assignedTopics[i];
            if (i + 1 < u.assignedTopics.size()) out << ",";
        }
        out << "\n";
    }
    exportFrontendData();
}

void loadUsers() {
    users.clear();
    ifstream in(USERS_FILE);
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        vector<string> parts;
        string token;
        stringstream ss(line);
        while (getline(ss, token, '|')) parts.push_back(token);
        User u;
        if (parts.size() >= 21) {
            u.username = parts[0];
            u.password = parts[1];
            u.totalPoints = stoi(parts[2]);
            u.racesPlayed = stoi(parts[3]);
            u.streak = stoi(parts[4]);
            u.badges = stoi(parts[5]);
            u.isMentor = (parts[6] == "1");
            u.inviteCode = parts[7];
            u.wins = stoi(parts[8]);
            u.losses = stoi(parts[9]);
        } else {
            continue;
        }
        auto splitMap = [&](const string &s, unordered_map<string, int> &m) {
            if (s.empty()) return;
            string tmp;
            stringstream ss2(s);
            while (getline(ss2, tmp, ',')) {
                if (tmp.empty()) continue;
                auto pos = tmp.find(":");
                if (pos == string::npos) continue;
                m[tmp.substr(0,pos)] = stoi(tmp.substr(pos+1));
            }
        };
        splitMap(parts[10], u.topicWins);
        splitMap(parts[11], u.topicLosses);
        splitMap(parts[12], u.topicTime);
        u.lastActiveDay = stoi(parts[13]);
        u.votesGiven = stoi(parts[14]);
        u.totalDifficultyScore = stoi(parts[15]);
        u.role = parts[16];
        u.favoriteSubject = parts[17];
        u.enrollmentId = parts[18];
        u.forgotAnswer = parts[19];
        if (parts.size() > 20) u.teacherApproved = (parts[20] == "1");
        if (parts.size() > 21) {
            stringstream ts(parts[21]);
            string t;
            while (getline(ts, t, ',')) {
                if (!t.empty()) u.assignedTopics.push_back(t);
            }
        }
        users[u.username] = u;
    }
}

string escapeJson(const string &s) {
    string r;
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '"') r += "\\\"";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else r += c;
    }
    return r;
}

void exportFrontendData() {
    system("mkdir frontend\\data >nul 2>nul");
    ofstream out1("frontend/data/users.json");
    out1 << "{\"users\":[\n";
    int idx=0;
    for (auto &p : users) {
        const User &u = p.second;
        out1 << "  {\"username\":\"" << escapeJson(u.username) << "\",\"tier\":\"" << u.getTier() << "\",\"points\":" << u.totalPoints << ",\"racesPlayed\":" << u.racesPlayed << ",\"badges\":" << u.badges << ",\"role\":\"" << escapeJson(u.role) << "\",\"approved\":" << (u.teacherApproved ? "true" : "false") << ",\"assignedTopics\":[";
        for (int i = 0; i < u.assignedTopics.size(); ++i) {
            out1 << "\"" << escapeJson(u.assignedTopics[i]) << "\"";
            if (i + 1 < u.assignedTopics.size()) out1 << ",";
        }
        out1 << "]}";
        if (++idx < users.size()) out1 << ",\n";
        else out1 << "\n";
    }
    out1 << "]}\n";
    ofstream out2("frontend/data/questions.json");
    out2 << "{\"questions\":[\n";
    for (int i=0;i<questions.size();i++) {
        auto &q = questions[i];
        out2 << "  {\"id\": " << q.id << ",\"topic\":\"" << escapeJson(q.topic) << "\",\"type\":\"" << escapeJson(q.type) << "\",\"prompt\":\"" << escapeJson(q.prompt) << "\",\"difficulty\":" << q.difficulty << ",\"author\":\"" << escapeJson(q.author) << "\"}";
        if (i+1<questions.size()) out2 << ",\n";
        else out2 << "\n";
    }
    out2 << "]}\n";
}

void persistRaceLog(const string &p1, const string &p2, const string &topic, int s1, int s2, const string &winner) {
    ofstream out(RACES_FILE, ios::app);
    out << p1 << "|" << p2 << "|" << topic << "|" << s1 << "|" << s2 << "|" << winner << "\n";
}

void persistQuestions() {
    ofstream out(QUESTIONS_FILE);
    for (auto &q : questions) {
        out << q.id << "|" << q.topic << "|" << q.type << "|" << q.prompt << "|" << q.answer << "|" << q.author << "|" << q.difficulty << "|" << q.votes << "|" << q.sumDifficultyVotes << "\n";
    }
    exportFrontendData();
}

void loadQuestions() {
    questions.clear();
    ifstream in(QUESTIONS_FILE);
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        vector<string> parts;
        string cur;
        stringstream ss(line);
        while (getline(ss, cur, '|')) parts.push_back(cur);
        if (parts.size() < 9) continue;
        Question q;
        q.id = stoi(parts[0]);
        q.topic = parts[1];
        q.type = parts[2];
        q.prompt = parts[3];
        q.answer = parts[4];
        q.author = parts[5];
        q.difficulty = stoi(parts[6]);
        q.votes = stoi(parts[7]);
        q.sumDifficultyVotes = stoi(parts[8]);
        questions.push_back(q);
    }
}

int inputInt(const string &prompt, int minv=-1000000, int maxv=1000000) {
    while (true) {
        cout << prompt;
        int x;
        if (cin >> x) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (x < minv || x > maxv) {
                cout << "Enter a number between " << minv << " and " << maxv << "\n";
                continue;
            }
            return x;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid number. Try again.\n";
    }
}

string trimStr(string s) {
    while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
    while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
    return s;
}

void ensureInitialQuestions() {
    if (!questions.empty()) return;
    questions = {
        {1, "Arrays", "Output prediction", "What is the output of: int a[3] = {1,2,3}; cout << a[1];", "2", "system", 1,0,0},
        {2, "Math", "Bug identification", "What is wrong: for (int i=1; i<=n; i++) sum += i; (n can be huge)", "Risk overflow without long long", "system", 2,0,0},
        {3, "Sorting", "Algorithm reasoning", "Which sort is stable by default: quicksort or merge sort?", "merge sort", "system", 2,0,0},
        {4, "DP", "Output prediction", "Given Fibonacci recursion with memo, what's T(n) complexity?", "O(n)", "system", 3,0,0},
        {5, "Graphs", "Bug identification", "In BFS, should you mark visited when queued or when popped?", "when queued", "system", 3,0,0},
        {6, "Greedy", "Algorithm reasoning", "Is Kruskal's algorithm guaranteed to find MST if edge weights are unique?", "yes", "system", 3,0,0}
    };
    persistQuestions();
}

void showLeaderboard() {
    pageHeader("Global Leaderboard\n");
    vector<User> all;
    for (auto &p : users) all.push_back(p.second);
    sort(all.begin(), all.end(), [](const User &a, const User &b){
        if (a.totalPoints != b.totalPoints) return a.totalPoints > b.totalPoints;
        return a.getSkillScore() > b.getSkillScore();
    });
    cout << "\n===== Leaderboard =====\n";
    cout << "Rank | User | Points | Skill | Tier\n";
    int rank = 1;
    for (auto &u : all) {
        cout << rank++ << " | " << u.username << " | " << u.totalPoints << " | " << fixed << setprecision(1) << u.getSkillScore() << " | " << u.getTier() << "\n";
        if (rank > 10) break;
    }
    cout << "=======================\n";
}

void showHeatmap(const User &u) {
    cout << "\nPerformance Heatmap for " << u.username << "\n";
    vector<string> topics = {"Arrays","Math","Sorting","DP","Graphs","Greedy"};
    for (auto &t : topics) {
        int w = u.topicWins.count(t) ? u.topicWins.at(t) : 0;
        int l = u.topicLosses.count(t) ? u.topicLosses.at(t) : 0;
        int total = w + l;
        double ratio = total ? double(w) / total : 0;
        int bars = int(ratio * 20);
        cout << setw(10) << t << " [" << string(bars, '#') << string(20-bars, '-') << "] " << (int)(ratio*100) << "% (" << w << "W " << l << "L)\n";
    }
}

int chooseQuestionDifficulty() {
    int r = rand() % 100;
    if (r < 50) return 1;
    if (r < 85) return 2;
    return 3;
}

vector<Question> pickRaceQuestions(int n=3) {
    vector<Question> pool = questions;
    vector<Question> chosen;
    for (int i = 0; i < n; ++i) {
        int d = chooseQuestionDifficulty();
        vector<Question> byDiff;
        for (auto &q : pool) if (q.difficulty == d) byDiff.push_back(q);
        if (byDiff.empty()) byDiff = pool;
        if (byDiff.empty()) break;
        Question q = byDiff[rand() % byDiff.size()];
        chosen.push_back(q);
    }
    return chosen;
}

bool doAntiCheatChallenge(const User &u) {
    vector<Question> logic;
    for (auto &q : questions) if (q.type == "Output prediction" || q.type == "Bug identification" || q.type == "Algorithm reasoning") logic.push_back(q);
    if (logic.empty()) return true;
    Question q = logic[rand() % logic.size()];
    cout << "\n[Anti-Cheat] " << q.type << " - " << q.prompt << "\n";
    cout << "Answer: ";
    string ans;
    getline(cin, ans);
    string normAns = ans;
    transform(normAns.begin(), normAns.end(), normAns.begin(), ::tolower);
    string target = q.answer;
    transform(target.begin(), target.end(), target.begin(), ::tolower);
    bool correct = normAns.find(target) != string::npos || target.find(normAns) != string::npos;
    if (!correct) {
        cout << "Anti-cheat validation failed. This race will count with penalty.\n";
    }
    return correct;
}

void showAchievements(const User &u) {
    cout << "\n=== Achievements for " << u.username << " ===\n";
    cout << "Total points: " << u.totalPoints << "\n";
    cout << "Races played: " << u.racesPlayed << "\n";
    cout << "Current streak: " << u.streak << "\n";
    cout << "Badges: " << u.badges << "\n";
    cout << "Mentor: " << (u.isMentor ? "Yes" : "No") << "\n";
    if (u.streak >= 7) cout << "Badge unlocked: 7-day streak!\n";
    if (u.totalPoints >= 500) cout << "Badge unlocked: 500 points milestone!\n";
    if (u.wins >= 10) cout << "Badge unlocked: 10 wins!\n";
    cout << "============================\n";
}

void recommendTopics(const User &u) {
    vector<pair<double, string>> rank;
    vector<string> topics = {"Arrays","Math","Sorting","DP","Graphs","Greedy"};
    for (auto &t : topics) {
        int w = u.topicWins.count(t) ? u.topicWins.at(t) : 0;
        int l = u.topicLosses.count(t) ? u.topicLosses.at(t) : 0;
        int total = w + l;
        double ratio = total ? double(w) / total : 0.0;
        rank.push_back({ratio, t});
    }
    sort(rank.begin(), rank.end());
    cout << "\nTopic recommendations (need improvement):\n";
    for (int i = 0; i < 3 && i < rank.size(); ++i) {
        cout << i+1 << ". " << rank[i].second << " (win ratio " << fixed << setprecision(1) << rank[i].first * 100 << "%)\n";
    }
}

string runCommandCapture(const string &cmd) {
    string result;
    FILE *pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "Failed to run command.";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

void compileAndRunCode(User &me) {
    pageHeader("C/C++ Compiler Practice\n");
    cout << "Enter language (c/cpp): ";
    string lang;
    getline(cin, lang);
    if (lang != "c" && lang != "cpp") {
        cout << "Invalid language. Choose c or cpp.\n";
        return;
    }
    string filename = (lang == "c") ? "compile_temp.c" : "compile_temp.cpp";
    string exe = "compile_temp.exe";
    cout << "Paste code below. Enter a line containing only 'EOF' to finish:\n";
    ofstream f(filename);
    while (true) {
        string line;
        getline(cin, line);
        if (line == "EOF") break;
        f << line << "\n";
    }
    f.close();
    string compileCmd;
    if (lang == "c") compileCmd = "gcc " + filename + " -o " + exe + " 2>&1";
    else compileCmd = "g++ " + filename + " -o " + exe + " 2>&1";
    cout << "Compiling...\n";
    string compileOut = runCommandCapture(compileCmd);
    if (!compileOut.empty()) {
        cout << "Compile output:\n" << compileOut << "\n";
    }
    if (ifstream(exe)) {
        cout << "Compilation success. Enter input for program (one line, optional): ";
        string userInput;
        getline(cin, userInput);
        string runCmd = exe;
        if (!userInput.empty()) runCmd += " < compile_input.txt";
        cout << "Running program...\n";
        string runOut = runCommandCapture(runCmd);
        cout << "Program output:\n" << runOut << "\n";
    } else {
        cout << "Compilation failed. Fix errors and try again.\n";
    }
}

void raceReplayTimeline(const vector<ReplayEvent> &events) {
    cout << "\nRace Replay Timeline:\n";
    for (auto &e : events) {
        cout << "[" << e.timestamp << "s] " << e.user << " " << (e.solved ? "solved" : "attempted") << " Q" << e.questionId << "\n";
    }
}

void teacherQuestionPage(User &me) {
    pageHeader("Teacher Question Builder\n");
    cout << "1) Add new coding question\n";
    cout << "2) View existing questions\n";
    cout << "3) Back\n";
    int c = inputInt("Choose: ",1,3);
    if (c == 1) {
        Question q;
        q.id = questions.empty() ? 1 : questions.back().id + 1;
        cout << "Topic: "; getline(cin, q.topic);
        cout << "Type (Output prediction/Bug identification/Algorithm reasoning): "; getline(cin, q.type);
        cout << "Prompt: "; getline(cin, q.prompt);
        cout << "Answer: "; getline(cin, q.answer);
        q.difficulty = inputInt("Difficulty (1-3): ",1,3);
        q.author = me.username;
        bool already = false;
        for (auto &qq : questions) {
            if (qq.topic == q.topic && qq.prompt == q.prompt) {
                already = true;
                break;
            }
        }
        if (already) {
            cout << "Question already exists for this topic. Skipped.\n";
        } else {
            questions.push_back(q);
            persistQuestions();
            cout << "Question added with ID " << q.id << "\n";
        }
    } else if (c == 2) {
        cout << "\nExisting Questions:\n";
        for (auto &q : questions) {
            cout << q.id << ". [" << q.topic << "] " << q.prompt << " (" << q.difficulty << ")\n";
        }
    }
}

void adminDashboard(User &me) {
    pageHeader("Admin Dashboard\n");
    cout << "1) View all users\n";
    cout << "2) Approve teacher requests\n";
    cout << "3) Assign topics to teachers\n";
    cout << "4) Back\n";
    int c = inputInt("Choose: ",1,4);
    if (c == 1) {
        for (auto &p : users) {
            auto &u = p.second;
            cout << u.username << " (" << u.role << ") approved=" << (u.teacherApproved ? "yes" : "no") << "\n";
        }
    } else if (c == 2) {
        cout << "Pending teacher approvals:\n";
        for (auto &p : users) {
            auto &u = p.second;
            if (u.role == "teacher" && !u.teacherApproved) {
                cout << "Approve " << u.username << "? (1 yes, 0 no): ";
                int v = inputInt("",0,1);
                if (v == 1) {
                    u.teacherApproved = true;
                    users[u.username] = u;
                    cout << "Approved " << u.username << "\n";
                }
            }
        }
        persistUsers();
    } else if (c == 3) {
        cout << "Assign teacher topic (example: Arrays, DP, Graphs):\n";
        string teacherName;
        cout << "Teacher username: "; getline(cin, teacherName);
        if (!users.count(teacherName) || users[teacherName].role != "teacher") {
            cout << "Teacher user not found.\n";
        } else {
            string topic;
            cout << "Topic to assign: "; getline(cin, topic);
            auto &tuser = users[teacherName];
            bool found = false;
            for (auto &tt : tuser.assignedTopics) if (tt == topic) found = true;
            if (!found) {
                tuser.assignedTopics.push_back(topic);
                users[teacherName] = tuser;
                persistUsers();
                cout << "Assigned topic " << topic << " to " << teacherName << "\n";
            } else {
                cout << "Teacher already has this topic assigned.\n";
            }
        }
    }
}

void friendChallengePage(User &me) {
    pageHeader("Partner & Request Race\n");
    cout << "1) Create your invite code\n";
    cout << "2) Challenge by invite code\n";
    cout << "3) Challenge by username\n";
    cout << "4) Back\n";
    int c = inputInt("Choose: ",1,4);
    if (c == 1) {
        me.inviteCode = "CM" + to_string(rand()%9000+1000);
        users[me.username] = me;
        persistUsers();
        cout << "Invite code created: " << me.inviteCode << "\n";
        cout << "Share this with partner to start same-topic race.\n";
    } else if (c == 2) {
        string code;
        cout << "Enter partner invite code: ";
        getline(cin, code);
        User *opp = nullptr;
        for (auto &p : users) {
            if (p.second.inviteCode == code) {
                opp = &p.second;
                break;
            }
        }
        if (!opp) {
            cout << "Code not found.\n";
            return;
        }
        cout << "Enter topic for this race (same topic required): ";
        string topic;
        getline(cin, topic);
        topic = trimStr(topic);
        if (topic.empty()) {
            cout << "Topic cannot be empty.\n";
            return;
        }
        vector<Question> pool;
        for (auto &q : questions) if (q.topic == topic) pool.push_back(q);
        if (pool.empty()) {
            cout << "No questions available for this topic. Add questions first.\n";
            return;
        }
        vector<Question> raceQs;
        for (int i = 0; i < 3; ++i) raceQs.push_back(pool[rand()%pool.size()]);

        map<string,int> score;
        map<string,int> totalTime;
        int base = 20;
        for (auto u : {&me, opp}) {
            int pts = 0;
            int ttime = 0;
            for (auto &q : raceQs) {
                bool solved = rand()%100 < 80;
                if (solved) {
                    pts += q.difficulty * 25;
                    u->topicWins[topic]++;
                } else {
                    u->topicLosses[topic]++;
                }
                int tt = base + rand()%20 + q.difficulty*8;
                ttime += tt;
            }
            score[u->username] = pts;
            totalTime[u->username] = ttime;
            u->racesPlayed++;
            u->totalPoints += pts;
        }
        string winner = score[me.username] >= score[opp->username] ? me.username : opp->username;
        if (winner == me.username) {
            me.wins++;
            me.streak++;
            opp->losses++;
            opp->streak = 0;
        } else {
            opp->wins++;
            opp->streak++;
            me.losses++;
            me.streak = 0;
        }
        if (me.streak % 7 == 0 && me.streak > 0) me.badges++;
        if (opp->streak % 7 == 0 && opp->streak > 0) opp->badges++;
        users[me.username] = me;
        users[opp->username] = *opp;
        persistUsers();
        persistRaceLog(me.username, opp->username, topic, score[me.username], score[opp->username], winner);
        cout << "Race complete. " << winner << " wins! " << me.username << " " << score[me.username] << " - " << score[opp->username] << " " << opp->username << "\n";
    } else if (c == 3) {
        cout << "Enter partner username: ";
        string uname;
        getline(cin, uname);
        if (!users.count(uname) || uname == me.username) {
            cout << "Partner user invalid.\n";
            return;
        }
        User &opp = users[uname];
        cout << "Enter topic (must match both teacher-assigned topics if they are teachers): ";
        string topic;
        getline(cin, topic);
        topic = trimStr(topic);
        if (topic.empty()) {
            cout << "Topic required.\n";
            return;
        }
        if (me.role == "teacher" && !me.teacherApproved) {
            cout << "You are not approved as teacher yet.\n";
            return;
        }
        if (opp.role == "teacher" && !opp.teacherApproved) {
            cout << opp.username << " is not approved as teacher yet.\n";
            return;
        }
        vector<Question> pool;
        for (auto &q : questions) if (q.topic == topic) pool.push_back(q);
        if (pool.empty()) {
            cout << "No questions for topic " << topic << ".\n";
            return;
        }
        vector<Question> raceQs;
        for (int i = 0; i < 3; ++i) raceQs.push_back(pool[rand() % pool.size()]);

        map<string,int> score;
        map<string,int> totalTime;
        int base = 20;
        for (auto u : {&me, &opp}) {
            int pts = 0;
            int ttime = 0;
            for (auto &q : raceQs) {
                bool solved = rand()%100 < 80;
                if (solved) {
                    pts += q.difficulty * 25;
                    u->topicWins[topic]++;
                } else {
                    u->topicLosses[topic]++;
                }
                int tt = base + rand()%20 + q.difficulty*8;
                ttime += tt;
            }
            score[u->username] = pts;
            totalTime[u->username] = ttime;
            u->racesPlayed++;
            u->totalPoints += pts;
        }
        string winner = score[me.username] >= score[opp.username] ? me.username : opp.username;
        if (winner == me.username) {
            me.wins++;
            me.streak++;
            opp.losses++;
            opp.streak = 0;
        } else {
            opp.wins++;
            opp.streak++;
            me.losses++;
            me.streak = 0;
        }
        if (me.streak % 7 == 0 && me.streak > 0) me.badges++;
        if (opp.streak % 7 == 0 && opp.streak > 0) opp.badges++;
        users[me.username] = me;
        users[opp.username] = opp;
        persistUsers();
        persistRaceLog(me.username, opp.username, topic, score[me.username], score[opp.username], winner);
        cout << "Race with " << opp.username << " completed. " << winner << " wins! " << score[me.username] << " vs " << score[opp.username] << "\n";
    }
}

void quickMatchPage(User &me) {
    pageHeader("Quick Partner Race\n");
    cout << "Enter partner username (or leave blank for auto-match): ";
    string partnerName;
    getline(cin, partnerName);
    partnerName = trimStr(partnerName);
    User *partner = nullptr;
    if (!partnerName.empty()) {
        if (!users.count(partnerName) || partnerName == me.username) {
            cout << "Partner invalid. Using auto-match.\n";
        } else {
            partner = &users[partnerName];
        }
    }
    cout << "Choose topic (same-topic race): ";
    string topic;
    getline(cin, topic);
    topic = trimStr(topic);
    if (topic.empty()) {
        cout << "Topic cannot be empty.\n";
        return;
    }
    vector<Question> topicList;
    for (auto &q : questions) {
        if (q.topic == topic) topicList.push_back(q);
    }
    if (topicList.empty()) {
        cout << "No questions for topic " << topic << ". Ask an approved teacher to add.\n";
        return;
    }
    if (!partner) {
        double target = me.getSkillScore();
        double bestDiff = 1e9;
        User *best = nullptr;
        for (auto &p : users) {
            if (p.first == me.username) continue;
            double diff = fabs(p.second.getSkillScore() - target);
            if (!best || diff < bestDiff) { bestDiff = diff; best = &p.second; }
        }
        if (!best) { cout << "No opponents available.\n"; return; }
        partner = best;
    }
    if (partner->role == "teacher" && !partner->teacherApproved) {
        cout << "Partner teacher is not approved.\n";
        return;
    }
    if (me.role == "teacher" && !me.teacherApproved) {
        cout << "You are not approved as teacher.\n";
        return;
    }
    vector<Question> raceQs;
    for (int i = 0; i < 3; ++i) {
        raceQs.push_back(topicList[rand() % topicList.size()]);
    }
    map<string,int> timeTaken;
    map<string,int> points;
    for (auto u : {&me, partner}) {
        int totalTime = 0;
        int totalPoints = 0;
        for (auto &q : raceQs) {
            int t = 15 + rand()%16 + q.difficulty * 5;
            totalTime += t;
            bool solved = rand()%100 < 85;
            if (solved) totalPoints += max(10, 120 - t + q.difficulty*12);
            else totalPoints += 5;
        }
        timeTaken[u->username] = totalTime;
        points[u->username] = totalPoints;
    }
    vector<pair<int,string>> ranking;
    ranking.push_back({points[me.username], me.username});
    ranking.push_back({points[partner->username], partner->username});
    sort(ranking.rbegin(), ranking.rend());
    cout << "\n=== Race Results ===\n";
    for (int i = 0; i < ranking.size(); ++i) {
        cout << i+1 << ". " << ranking[i].second << " - " << points[ranking[i].second] << " pts, " << timeTaken[ranking[i].second] << "s\n";
    }
    string winner = ranking[0].second;
    cout << "Winner: " << winner << "\n";
    users[me.username].racesPlayed++;
    users[me.username].totalPoints += points[me.username];
    users[me.username].topicWins[topic]++;
    users[partner->username].racesPlayed++;
    users[partner->username].totalPoints += points[partner->username];
    users[partner->username].topicWins[topic]++;
    if (winner == me.username) users[me.username].wins++, users[partner->username].losses++;
    else users[partner->username].wins++, users[me.username].losses++;
    persistRaceLog(me.username, partner->username, topic, points[me.username], points[partner->username], winner);
    persistUsers();
    persistQuestions();
    showLeaderboard();
}

void userDashboard(User &me) {
    while (true) {
        pageHeader("Dashboard | " + me.username + " | Tier: " + me.getTier());
        cout << "Points: " << me.totalPoints << " | Races: " << me.racesPlayed << " | Streak: " << me.streak << " | Badges: " << me.badges << "\n";
        renderBox("Quick Match to Rank Up");
        renderBox("Friend Challenge Room");
        renderBox("Skill Heatmap & Recommendations");
        renderBox("Achievements & Mentor Mode");
        cout << "\n1) Quick Match\n";
        cout << "2) Friend Challenge\n";
        cout << "3) Profile & Heatmap\n";
        cout << "4) Achievements\n";
        cout << "5) Leaderboard\n";
        cout << "6) Code Compiler Practice (C/C++)\n";
        if (me.role == "teacher") cout << "7) Teacher Question Builder\n";
        if (me.role == "admin") cout << "8) Admin Dashboard\n";
        cout << "0) Logout\n";
        int c = inputInt("Choose your page: ",0,8);
        if (c == 1) {
            quickMatchPage(me);
        } else if (c == 2) {
            friendChallengePage(me);
        } else if (c == 3) {
            showHeatmap(me);
            recommendTopics(me);
        } else if (c == 4) {
            showAchievements(me);
        } else if (c == 5) {
            showLeaderboard();
        } else if (c == 6) {
            compileAndRunCode(me);
        } else if (c == 7 && me.role == "teacher") {
            if (!me.teacherApproved) {
                cout << "Teacher access pending approval by admin.\n";
            } else {
                teacherQuestionPage(me);
            }
        } else if (c == 8 && me.role == "admin") {
            adminDashboard(me);
        } else if (c == 0) {
            break;
        } else {
            cout << "Invalid choice.\n";
        }
        me = users[me.username];
    }
}

int main() {
    srand((unsigned)time(NULL));
    loadUsers();
    if (!users.count("admin")) {
        User a;
        a.username = "admin";
        a.password = "admin123";
        a.role = "admin";
        a.teacherApproved = true;
        users["admin"] = a;
    }
    loadQuestions();
    ensureInitialQuestions();
    exportFrontendData();
    while (true) {
        pageHeader("Welcome to CodeMate - Your Daily Competitive Coding Arena\n");
        cout << "1) Register\n";
        cout << "2) Login\n";
        cout << "3) Launch Web GUI\n";
        cout << "4) Exit\n";
        cout << "\n[Important] Data stored in users.txt, questions.txt, races.txt, and frontend/data/*.json.\n";
        cout << "[Tip] Use option 3 to open the web GUI and refresh after backend updates.\n";
        int c = inputInt("Choose option (1-4): ",1,4);
        if (c == 1) {
            string uname, pwd, enroll, forgot;
            cout << "Username: "; getline(cin, uname);
            string role;
            cout << "Password: "; getline(cin, pwd);
            cout << "Role (student/teacher): "; getline(cin, role);
            string teachTopic = "";
            if (role == "teacher") {
                cout << "Teaching topic (e.g. Arrays, DP): "; getline(cin, teachTopic);
            }
            cout << "Enrollment ID: "; getline(cin, enroll);
            cout << "Forgot password answer (favorite algorithm): "; getline(cin, forgot);
            uname = trimStr(uname);
            if (uname.empty() || pwd.empty() || role.empty() || enroll.empty() || forgot.empty() || (role == "teacher" && teachTopic.empty())) { cout << "All fields are required.\n"; continue; }
            if (role != "student" && role != "teacher") { cout << "Role must be student or teacher.\n"; continue; }
            if (users.count(uname)) {
                cout << "User already exists.\n";
                continue;
            }
            User u;
            u.username = uname;
            u.password = pwd;
            u.role = role;
            u.favoriteSubject = (role == "teacher" ? teachTopic : "");
            u.enrollmentId = enroll;
            u.forgotAnswer = forgot;
            u.inviteCode = "CM" + to_string(rand()%9000+1000);
            u.teacherApproved = (role == "student");
            if (role == "teacher" && !teachTopic.empty()) u.assignedTopics.push_back(teachTopic);
            users[uname] = u;
            persistUsers();
            cout << "Registered successfully. Invite code: " << u.inviteCode << "\n";
        } else if (c == 2) {
            string uname, pwd;
            cout << "Username: "; getline(cin, uname);
            cout << "Password (or type 'forgot'): "; getline(cin, pwd);
            if (!users.count(uname)) {
                cout << "User not found.\n";
                continue;
            }
            User &me = users[uname];
            if (pwd == "forgot") {
                string ans;
                cout << "Security: What is your favorite algorithm? "; getline(cin, ans);
                if (trimStr(ans) != trimStr(me.forgotAnswer)) {
                    cout << "Answer mismatch. Cannot reset.\n";
                    continue;
                }
                string np;
                cout << "New password: "; getline(cin, np);
                me.password = np;
                users[uname] = me;
                persistUsers();
                cout << "Password updated. Login again.\n";
                continue;
            }
            if (me.password != pwd) {
                cout << "Login failed.\n";
                continue;
            }
            userDashboard(me);
            persistUsers();
        } else if (c == 3) {
            exportFrontendData();
            cout << "Launching web GUI...\n";
            #ifdef _WIN32
            system("start frontend\\index.html");
            #else
            system("xdg-open frontend/index.html");
            #endif
        } else {
            cout << "Goodbye!\n";
            break;
        }
    }
    return 0;
}
