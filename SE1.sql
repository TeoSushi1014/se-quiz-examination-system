--Vibe coding for placeholder atm. I will look into it later.

CREATE TABLE roles (
    role_id SERIAL PRIMARY KEY,
    role_name VARCHAR(20) UNIQUE NOT NULL
);

CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    role_id INT NOT NULL REFERENCES roles(role_id),
    is_active BOOLEAN DEFAULT TRUE,

    failed_attempts INT DEFAULT 0,
    locked_until TIMESTAMP NULL,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE questions (
    question_id SERIAL PRIMARY KEY,
    question_text TEXT NOT NULL,
    difficulty VARCHAR(10) CHECK (difficulty IN ('EASY', 'MEDIUM', 'HARD')),
    topic VARCHAR(100),
    created_by INT NOT NULL REFERENCES users(user_id),
    is_active BOOLEAN DEFAULT TRUE,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    UNIQUE (question_text)
);

CREATE TABLE question_options (
    option_id SERIAL PRIMARY KEY,
    question_id INT NOT NULL REFERENCES questions(question_id) ON DELETE CASCADE,
    option_label CHAR(1) CHECK (option_label IN ('A','B','C','D')),
    option_text TEXT NOT NULL,
    is_correct BOOLEAN DEFAULT FALSE,

    UNIQUE (question_id, option_label)
);

CREATE TABLE quizzes (
    quiz_id SERIAL PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    created_by INT NOT NULL REFERENCES users(user_id),

    time_limit_minutes INT CHECK (time_limit_minutes > 0),
    total_points INT CHECK (total_points >= 0),
    negative_marking BOOLEAN DEFAULT FALSE,

    shuffle_questions BOOLEAN DEFAULT FALSE,
    shuffle_answers BOOLEAN DEFAULT FALSE,

    show_results_immediately BOOLEAN DEFAULT TRUE,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE quiz_questions (
    quiz_id INT REFERENCES quizzes(quiz_id) ON DELETE CASCADE,
    question_id INT REFERENCES questions(question_id),

    PRIMARY KEY (quiz_id, question_id)
);

CREATE TABLE quiz_assignments (
    assignment_id SERIAL PRIMARY KEY,
    quiz_id INT NOT NULL REFERENCES quizzes(quiz_id),
    student_id INT NOT NULL REFERENCES users(user_id),

    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP NOT NULL,

    max_attempts INT DEFAULT 1 CHECK (max_attempts > 0),

    UNIQUE (quiz_id, student_id)
);

CREATE TABLE quiz_attempts (
    attempt_id SERIAL PRIMARY KEY,
    quiz_id INT NOT NULL REFERENCES quizzes(quiz_id),
    student_id INT NOT NULL REFERENCES users(user_id),

    start_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    submit_time TIMESTAMP,
    status VARCHAR(20) CHECK (status IN ('IN_PROGRESS', 'SUBMITTED', 'AUTO_SUBMITTED')),

    time_spent_seconds INT,

    UNIQUE (quiz_id, student_id, start_time)
);

CREATE TABLE attempt_answers (
    attempt_id INT REFERENCES quiz_attempts(attempt_id) ON DELETE CASCADE,
    question_id INT REFERENCES questions(question_id),
    selected_option_ids INT[] NOT NULL,

    PRIMARY KEY (attempt_id, question_id)
);

CREATE TABLE attempt_results (
    attempt_id INT PRIMARY KEY REFERENCES quiz_attempts(attempt_id) ON DELETE CASCADE,

    total_score NUMERIC(5,2),
    correct_count INT,
    incorrect_count INT,

    graded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
