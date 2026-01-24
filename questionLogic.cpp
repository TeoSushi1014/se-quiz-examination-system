//Only for question logic purpose
#include <iostream>
#include <vector>
#include <string>
using namespace std;

class User {
private:
    string id;
    string role;     // Admin | Teacher | Student
    string status;   // ACTIVE | DISABLED

public:
    User(const string& id, const string& role, const string& status)
        : id(id), role(role), status(status) {
    }

    string getId() const { return id; }
    string getRole() const { return role; }
    string getStatus() const { return status; }
};

class Option {
private:
    char label;          // A, B, C, D
    string content;      
    bool isCorrect;

public:
    Option(char label, const string& content, bool isCorrect)
        : label(label), content(content), isCorrect(isCorrect) {
    }

    char getLabel() const { return label; }
    string getContent() const { return content; }
    bool getIsCorrect() const { return isCorrect; }
};

class Question {
private:
    string id;
    string teacherID;
    string questionText;
    string difficulty;
    string topic;
    vector<Option> options;

public:
    Question() {}

    Question(const string& id,
        const string& teacherID,
        const string& text,
        const string& difficulty,
        const string& topic)
        : id(id),
        teacherID(teacherID),
        questionText(text),
        difficulty(difficulty),
        topic(topic) {
    }

    // setters
    void setId(const string& v) { id = v; }
    void setTeacherID(const string& v) { teacherID = v; }
    void setQuestionText(const string& v) { questionText = v; }
    void setDifficulty(const string& v) { difficulty = v; }
    void setTopic(const string& v) { topic = v; }

    void addOption(const Option& op) {
        options.push_back(op);
    }

    // getters
    string getId() const { return id; }
    string getTeacherID() const { return teacherID; }
    string getQuestionText() const { return questionText; }
    string getDifficulty() const { return difficulty; }
    string getTopic() const { return topic; }
    const vector<Option>& getOptions() const { return options; }
};

bool canCreateQuestion(const User& user) {
    if (user.getStatus() != "ACTIVE") {
        cout << "Account is not active.\n";
        return false;
    }

    if (user.getRole() != "Teacher") {
        cout << "Only teachers can create questions.\n";
        return false;
    }

    return true;
}

bool isValidDifficulty(const string& diff) {
    return diff == "easy" || diff == "medium" || diff == "hard";
}

bool validateQuestion(const Question& q) {
    if (q.getQuestionText().empty()) {
        cout << "Question text cannot be empty.\n";
        return false;
    }

    if (q.getTopic().empty()) {
        cout << "Topic cannot be empty.\n";
        return false;
    }

    if (!isValidDifficulty(q.getDifficulty())) {
        cout << "Invalid difficulty.\n";
        return false;
    }

    const auto& options = q.getOptions();
    if (options.size() != 4) {
        cout << "Question must have exactly 4 options.\n";
        return false;
    }

    int correctCount = 0;
    for (const auto& op : options) {
        if (op.getContent().empty()) {
            cout << "Option " << op.getLabel() << " cannot be empty.\n";
            return false;
        }
        if (op.getIsCorrect()) correctCount++;
    }

    if (correctCount != 1) {
        cout << "There must be exactly one correct option.\n";
        return false;
    }

    return true;
}

Question createQuestion(const User& teacher) {
    Question q;

    string id, text, topic, difficulty;
    cout << "Enter question ID: ";
    cin >> id;
    cin.ignore();

    cout << "Enter question text: ";
    getline(cin, text);

    cout << "Enter topic: ";
    getline(cin, topic);

    cout << "Enter difficulty (easy / medium / hard): ";
    cin >> difficulty;
    cin.ignore();

    q.setId(id);
    q.setTeacherID(teacher.getId());
    q.setQuestionText(text);
    q.setTopic(topic);
    q.setDifficulty(difficulty);

    int correctIndex;
    cout << "Correct option index (0=A, 1=B, 2=C, 3=D): ";
    cin >> correctIndex;
    cin.ignore();

    char labels[4] = { 'A', 'B', 'C', 'D' };
    for (int i = 0; i < 4; i++) {
        string content;
        cout << "Option " << labels[i] << ": ";
        getline(cin, content);

        q.addOption(Option(labels[i], content, i == correctIndex));
    }

    return q;
}

bool teacherCreateQuestion(const User& user, Question& outQuestion) {
    if (!canCreateQuestion(user))
        return false;

    Question q = createQuestion(user);

    if (!validateQuestion(q))
        return false;

    outQuestion = q;
    return true;
}

int main() {
    User teacher("T01", "Teacher", "ACTIVE");

    Question q;
    if (teacherCreateQuestion(teacher, q)) {
        cout << "Question created successfully.\n";
    }
    else {
        cout << "Failed to create question.\n";
    }

    return 0;
}