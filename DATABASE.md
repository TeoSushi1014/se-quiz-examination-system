# Quiz Examination System - Database Documentation

## Overview

Database hosted on Supabase (PostgreSQL) with Row Level Security (RLS) enabled on all tables.

## Tables

### users

Stores user account information for students, teachers, and administrators.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | varchar(6) | NO | - | PRIMARY KEY |
| username | text | NO | - | UNIQUE |
| password_hash | text | NO | - | CHECK (is_pbkdf2_hash OR is_legacy_hash) |
| role | text | YES | - | CHECK (Admin, Teacher, Student) |
| failed_login_count | integer | YES | 0 | - |
| status | text | YES | Active | CHECK (Active, Inactive) |
| locked_until | timestamp | YES | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |
| updated_at | timestamp | YES | CURRENT_TIMESTAMP | - |
| created_by | char(6) | YES | - | FK -> users.id |

### questions

Stores multiple-choice questions for quizzes.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | varchar(6) | NO | - | PRIMARY KEY |
| created_by | varchar(6) | NO | - | FK -> users.id |
| question_text | text | NO | - | - |
| option_a | text | NO | - | - |
| option_b | text | NO | - | - |
| option_c | text | NO | - | - |
| option_d | text | NO | - | - |
| correct_option | varchar(1) | NO | - | CHECK (A, B, C, D) |
| difficulty_level | varchar(10) | NO | - | CHECK (Easy, Medium, Hard) |
| topic | text | YES | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |
| updated_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### quizzes

Stores quiz configurations and settings.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | varchar(6) | NO | - | PRIMARY KEY |
| created_by | varchar(6) | NO | - | FK -> users.id |
| title | varchar(255) | NO | - | - |
| time_limit_minutes | integer | NO | - | CHECK (> 0) |
| total_points | integer | NO | - | CHECK (> 0) |
| shuffle_questions | boolean | YES | false | - |
| shuffle_answers | boolean | YES | false | - |
| result_visibility | varchar(20) | NO | immediate | CHECK (Immediate, After quiz end, Manual release) |
| max_attempts | varchar(10) | NO | unlimited | CHECK (number or Unlimited) |
| result_released | boolean | YES | false | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |
| updated_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### quiz_questions

Junction table linking quizzes to questions with point values.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | char(6) | NO | - | PRIMARY KEY |
| quiz_id | varchar(6) | NO | - | FK -> quizzes.id |
| question_id | varchar(6) | NO | - | FK -> questions.id |
| points | integer | NO | - | CHECK (> 0) |
| order_num | integer | NO | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### quiz_assignments

Stores quiz assignments to students with time windows.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | char(6) | NO | - | PRIMARY KEY |
| quiz_id | varchar(6) | NO | - | FK -> quizzes.id |
| assigned_to | varchar(6) | NO | - | FK -> users.id |
| start_time | timestamp | NO | - | - |
| end_time | timestamp | NO | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### quiz_attempts

Stores student quiz attempts and results.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | varchar(6) | NO | - | PRIMARY KEY |
| quiz_id | varchar(6) | NO | - | FK -> quizzes.id |
| student_id | varchar(6) | NO | - | FK -> users.id |
| attempt_number | integer | NO | - | - |
| score | integer | YES | - | - |
| total_points | integer | YES | - | - |
| correct_count | integer | YES | 0 | - |
| incorrect_count | integer | YES | 0 | - |
| status | varchar(20) | NO | in_progress | CHECK (in_progress, submitted) |
| started_at | timestamp | NO | - | - |
| submitted_at | timestamp | YES | - | - |
| time_spent_seconds | integer | YES | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### quiz_answers

Stores individual answers for each quiz attempt.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | char(6) | NO | - | PRIMARY KEY |
| attempt_id | varchar(6) | NO | - | FK -> quiz_attempts.id |
| question_id | varchar(6) | NO | - | FK -> questions.id |
| selected_option | varchar(1) | YES | - | - |
| is_correct | boolean | YES | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |

### audit_logs

Stores system activity logs for auditing purposes.

| Column | Type | Nullable | Default | Constraints |
|--------|------|----------|---------|-------------|
| id | uuid | NO | gen_random_uuid() | PRIMARY KEY |
| action | varchar(100) | NO | - | - |
| actor_id | varchar(6) | NO | - | FK -> users.id |
| target_table | varchar(50) | YES | - | - |
| target_id | varchar(6) | YES | - | - |
| details | jsonb | YES | - | - |
| created_at | timestamp | YES | CURRENT_TIMESTAMP | - |

## Entity Relationships

```
users (1) ----< (N) questions
users (1) ----< (N) quizzes
users (1) ----< (N) quiz_assignments
users (1) ----< (N) quiz_attempts
users (1) ----< (N) audit_logs

quizzes (1) ----< (N) quiz_questions
quizzes (1) ----< (N) quiz_assignments
quizzes (1) ----< (N) quiz_attempts

questions (1) ----< (N) quiz_questions
questions (1) ----< (N) quiz_answers

quiz_attempts (1) ----< (N) quiz_answers
```

## Stored Functions

### Authentication

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| login_user | p_username, p_password | jsonb | Authenticate user and return JWT token |
| handle_login_success | input_username | json | Reset failed login count on successful login |
| handle_login_failure | input_username | json | Increment failed login count, lock after 5 failures |
| check_account_status | input_username | json | Check if account is locked or inactive |
| auto_unlock_expired_locks | - | TABLE | Automatically unlock accounts after 30 minutes |

### User Management

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| create_user_by_admin | p_admin_username, p_new_username, p_password_hash, p_role | json | Create new user account (admin only) |

### Question Management

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| create_question_validated | p_id, p_created_by, p_question_text, options, p_correct_option, p_difficulty_level, p_topic | json | Create question with validation |
| update_question_validated | p_id, p_question_text, options, p_correct_option, p_difficulty_level, p_topic | jsonb | Update question with validation |
| delete_question_safe | question_id_input | json | Delete question if not used in active quiz |

### Quiz Management

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| create_quiz_full | title, created_by, settings, p_questions | jsonb | Create quiz with questions in single transaction |
| delete_quiz_teacher | quiz_id_input, teacher_id | json | Delete quiz (teacher, no attempts only) |
| purge_quiz_admin | quiz_id_input, admin_id | json | Purge quiz with all attempts (admin only) |
| purge_quiz_cascade | target_quiz_id | json | Cascade delete quiz and related data |
| toggle_result_release | p_quiz_id, p_teacher_id, p_released | json | Toggle result visibility for manual release |

### Quiz Assignment

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| assign_quiz | p_quiz_id, p_student_ids, p_start_time, p_end_time | jsonb | Assign quiz to multiple students |
| get_quiz_assignments | p_quiz_id | TABLE | Get all assignments for a quiz |
| get_students_for_assignment | - | TABLE | Get list of students for assignment |

### Quiz Taking

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| validate_quiz_access | input_student_id, input_quiz_id | TABLE | Check if student can access quiz |
| get_student_quizzes | input_student_id | TABLE | Get available quizzes for student |
| get_quiz_questions | input_quiz_id | TABLE | Get questions for quiz attempt |
| submit_quiz_attempt | input_student_id, input_quiz_id, input_answers, input_time_spent_seconds | TABLE | Submit and grade quiz attempt |

### Reporting

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| get_quiz_attempts_report | input_quiz_id | TABLE | Get all attempts for a quiz (reports) |
| get_quiz_attempts_for_review | p_quiz_id | TABLE | Get attempts with details for review |
| get_attempt_results | input_student_id, input_quiz_id | TABLE | Get student's attempt results |
| get_attempt_details_with_answers | input_attempt_id | TABLE | Get detailed answers for an attempt |

### Attempt Management

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| delete_quiz_attempt | p_attempt_id, p_teacher_id | json | Delete a specific attempt |

### Password Utilities

| Function | Arguments | Returns | Description |
|----------|-----------|---------|-------------|
| generate_pbkdf2_hash | password | text | Generate PBKDF2-SHA256 password hash |
| is_pbkdf2_hash | hash_value | boolean | Check if hash is PBKDF2 format |
| is_legacy_hash | hash_value | boolean | Check if hash is legacy format |

## Row Level Security Policies

### users

| Policy | Command | Description |
|--------|---------|-------------|
| anon_read_users | SELECT | Anonymous users can read all users |
| admin_update_users | UPDATE | Anonymous users can update (handled by app) |

### questions

| Policy | Command | Description |
|--------|---------|-------------|
| Allow authenticated read questions | SELECT | All can read questions |
| Teachers can create questions | INSERT | Teachers can create questions |
| Teachers can update own questions | UPDATE | Teachers can update questions |

### quizzes

| Policy | Command | Description |
|--------|---------|-------------|
| Allow authenticated read quizzes | SELECT | All can read quizzes |
| Teachers can create quizzes | INSERT | Teachers can create quizzes |
| Teachers can update own quizzes | UPDATE | Teachers can update quizzes |
| Teachers can delete own quizzes | DELETE | Teachers can delete quizzes |

### quiz_questions

| Policy | Command | Description |
|--------|---------|-------------|
| View quiz questions | SELECT | All can view quiz questions |

### quiz_assignments

| Policy | Command | Description |
|--------|---------|-------------|
| View assigned quizzes | SELECT | Users can view their assignments |
| Teachers can assign their quizzes | INSERT | Teachers can create assignments |
| Teachers can update their quiz assignments | UPDATE | Teachers can update assignments |
| Teachers can delete their quiz assignments | DELETE | Teachers can delete assignments |
| Admins can manage all quiz assignments | ALL | Admins have full access |

### quiz_attempts

| Policy | Command | Description |
|--------|---------|-------------|
| Students can view own attempts | SELECT | Students see own attempts |
| Students can create own attempts | INSERT | Students can create attempts |
| Teachers and admins can view all attempts | SELECT | Teachers/admins see all |

### quiz_answers

| Policy | Command | Description |
|--------|---------|-------------|
| View answers for own attempt | SELECT | Users see answers for own attempts |

### audit_logs

| Policy | Command | Description |
|--------|---------|-------------|
| Allow anon users full access to audit logs | ALL | Anonymous can read/write logs |
| Allow authenticated users to read audit logs | SELECT | Authenticated can read logs |
| Allow service_role full access to audit logs | ALL | Service role has full access |

## Audit Log Actions

| Action | Description |
|--------|-------------|
| LOGIN_SUCCESS | User logged in successfully |
| LOGIN_FAILURE | Failed login attempt |
| CREATE_USER | Admin created new user |
| CHANGE_ROLE | Admin changed user role |
| RESET_PASSWORD | Admin reset user password |
| ENABLE_USER | Admin enabled user account |
| DISABLE_USER | Admin disabled user account |
| PURGE_QUIZ | Admin purged quiz with attempts |
| DELETE_ATTEMPT | Teacher/admin deleted attempt |
| SYSTEM_INIT | System initialization |
