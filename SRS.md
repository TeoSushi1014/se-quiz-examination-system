# QUIZ EXAMINATION SYSTEM
## Requirement specification & Design document

Project: Quiz examination system  
Course: Software Engineering  
Group: 05
Version: v1.1  
Date: 2026-01-04

## Revision history (Change log)
| Version | Date       | Changes |
|---------|------------|---------|
| v1.0    | 2025-12-25 | Add initial SRS document for Quiz Examination System |
| v1.1    | 2026-01-04 | Enhance SRS document with detailed functional requirements and system scope |

---

## System scope and purpose
- The purpose of the Quiz examination system is to provide a robust, scalable, and secure platform for conducting online multiple-choice tests. It aims to automate the entire lifecycle of an examination: from question bank management by teachers to automated grading and reporting for students and admins

## System actors
- Student: User who takes quizzes and views their own results
- Teacher: User who creates questions, manages quizzes, and views statistics
- Admin: User responsible for system management and user accounts

## Functional requirements

### FR-01 User authentication
- Description: The system shall allow users (Student/Teacher/Admin) to log in using a username and password
- Primary actors: Student, teacher, admin
- Preconditions: The user account exists in the database and is in an active state
- Main flow:
  1. The user enters username and password
  2. The system validates the credentials against the database
  3. The system creates a session and navigates the user to their respective home screen
- Alternative/exception flows:
  - Invalid credentials: The system shall show an error message: "Invalid username or password"
  - Account locking: If the user exceeds 5 failed attempts, the system shall temporarily lock the account for 30 minutes. After 30 minutes, the account shall be automatically unlocked
- Postconditions: The user is authenticated and granted access to role-based features
- Acceptance criteria:
  - System grants access only with correct credentials
  - Account locks exactly after the 5th failed attempt

### FR-02 Manage question bank
- Description: The system shall allow a teacher to manage (create, edit, delete) multiple choice questions in the question bank
- Primary actor: teacher
- Data fields: question text, answer options A B C D, correct option, difficulty level (easy medium hard), topic or tag
- Preconditions: teacher is logged in and has permission to manage questions
- Main flow (Create):
  1. Teacher selects "Add new questions"
  2. Teacher enters data for all required fields
  3. System validates the data including exactly one correct option selected
  4. System saves the question and displays a success message
- Exception flows:
  - Duplicate question: System warns if an identical question text already exists
  - Delete active question: If a question is currently in an active quiz, the system shall prevent deletion and show a warning
- Acceptance criteria:
  - Questions are successfully saved with all data fields
  - Non authorized users cannot access management functions

### FR-03 Create and configure quiz
- Description: The system shall allow a teacher to create a new quiz by selecting questions and configuring parameters
- Primary actor: teacher
- Preconditions: teacher has successfully logged in and has access to at least one question bank
- Main flow:
  1. Teacher selects "Create new quiz"
  2. Teacher enters quiz title and selects a set of questions from the question bank
  3. Teacher configures settings: time limit in minutes, total points, scoring rules, shuffle questions, shuffle answers options, and result visibility (immediate / after end time / manual release)
  4. System validates that the number of selected questions is greater than zero and settings are valid
  5. System saves the quiz and displays a "Quiz created successfully" message
- Exception flows:
  - Insufficient questions: If the teacher requests a random quiz with N questions but the bank has fewer than N, the system shall show an error message
  - Invalid configuration: If time limit or total points is set to a negative value, the system shall prevent saving and highlight the invalid fields
- Postconditions: A new quiz is stored in the database and is ready to be assigned to students
- Acceptance criteria:
  - The teacher can successfully save a quiz with all settings configured
  - The shuffle setting must be correctly applied when a student starts the quiz

#### Scoring rules
- The system shall support single choice questions with exactly one correct option
- For each question:
  - If the student selects the correct option, the system shall add the configured points for that question
  - If the student selects an incorrect option, the system shall add zero points
  - If the student leaves the question unanswered, the system shall add zero points
- The total score of an attempt shall be the sum of the points from all questions in that quiz

### FR-04 Take quiz attempt
- Description: The system shall allow a student to select an assigned quiz, perform the attempt within a time limit, and submit their responses for grading
- Primary actor: Student
- Preconditions:
  - Student is authenticated and authorized for the specific quiz
  - The current time is within the scheduled time window of the quiz (FR-07)
  - Student has not submitted an attempt for this quiz
- Attempt limit:
  - Each student shall be allowed to submit at most one attempt for each quiz
- Main flow:
  1. Student selects a quiz from the list of available assessments
  2. System initializes the attempt, starts the countdown timer, and displays the first set of questions
  3. Student selects answers and navigates through questions (Previous or Next)
  4. The system shall automatically save the student's current answers when the student navigates between questions
  5. Student confirms and submits the attempt
  6. The system records the completion time and final responses into the database
- Alternative or exception flows:
  - Time expiration: If the countdown reaches zero, the system shall auto submit the last saved answers and notify the student
  - Attempt limit reached: If the student has already submitted an attempt for this quiz, the system shall prevent starting a new attempt and show a clear message
- Postconditions:
  - The attempt is marked as Submitted and the data is ready for the grading process
- Acceptance criteria:
  - A student cannot submit more than one attempt for the same quiz
  - Answers are stored correctly even if the attempt is auto submitted due to time expiration

### FR-05 Auto grading and results
- Description: After a student submits a quiz, the system shall automatically calculate the score based on the pre defined correct options and store the results
- Primary actor: system
- Preconditions: The student has successfully submitted the quiz attempt (FR-04)
- Main flow:
  1. The system retrieves the student's answers and the correct answers for the quiz from the database
  2. The system compares answers and calculates the total score based on the scoring rules defined in FR-03
  3. The system generates an "Attempt summary" including total score, number of correct answers, number of incorrect answers, and completion time
  4. The system stores the score and summary in the student's history
  5. The system applies the visibility rule: results shall be shown immediately to the student unless configured otherwise by the teacher in FR-03
  6. If the quiz result visibility is set to "manual release", the system shall store the result but shall not display it to the student until the teacher releases results for that quiz
- Postconditions: The quiz result is permanently stored and linked to the student's account
- Acceptance criteria:
  - Grading must be accurate based on the answer key
  - During the project demo under 30 concurrent students, results must be stored in the database within 5 seconds after submission

### FR-06 View and export reports
- Description: The system shall allow teachers and admins to view a list of student attempts for a specific quiz and export these results to a CSV file
- Primary actors: Teacher, admin
- Preconditions: The user is logged in with appropriate permissions and at least one quiz attempt has been submitted
- Main flow:
  1. User selects a specific quiz from the management dashboard
  2. System displays a table of all attempts (student username, completion date, score, time spent)
  3. User chooses the "Export to CSV" option
  4. System generates and downloads a CSV file containing all displayed data
- Alternative/Exception flows:
  - No attempts found: If no students have taken the quiz, the system shall display a message: "No results available to display or export"
- Postconditions: A CSV file is downloaded to the user's device
- Acceptance criteria:
  - The displayed report must match the data in the database for that specific quiz
  - The exported CSV file must be readable by spreadsheet software (e.g., Excel) and contain all required fields

### FR-07 Schedule and assign quiz
- Description: The system shall allow a teacher to schedule a quiz by specifying a start time and an end time, and assign the quiz to specific students (or a class or group), so that only authorized students can access the quiz during the scheduled time window
- Primary actor: teacher
- Supporting actors: student
- Data fields:
  - Start time
  - End time
  - Assigned students
- Preconditions:
  - Teacher is authenticated (FR-01)
  - The quiz has been created (FR-03)
- Main flow:
  1. Teacher selects an existing quiz from the quiz management screen
  2. Teacher enters the start time and the end time of the quiz availability
  3. Teacher selects the target students
  4. The system validates that the start time is earlier than the end time and that at least one student is assigned
  5. The system saves the schedule time window
  6. The system makes the quiz visible and accessible only to assigned students during the scheduled time window
- Alternative or exception flows:
  - Invalid time window: If the start time is not earlier than the end time, the system shall prevent saving and show an error message
  - No students assigned: If no student or class is selected, the system shall prevent saving and show an error message
- Postconditions:
  - The quiz schedule and assignment are stored
  - Students can start an attempt only if they are assigned and the current time is within the scheduled time window
- Acceptance criteria:
  - Unassigned students cannot see or start the quiz
  - Assigned students can start the quiz only during the scheduled time window
  - The system rejects invalid time windows and empty assignments

### FR-08 Manage user accounts
- Description: The system shall allow an admin to manage user accounts including creating accounts, disabling accounts, resetting passwords, and assigning roles for student and teacher
- Primary actor: Admin
- Preconditions:
  - Admin is authenticated (FR-01)
- Main flow:
  1. Admin opens the user management screen
  2. Admin creates a new user by entering username, initial password, and selecting a role
  3. Admin disables or enables an existing user account
  4. Admin resets a user password and provides a new initial password to the user
  5. The system saves the changes and logs the action with the admin identity and time
- Alternative or exception flows:
  - Duplicate username: If the username already exists, the system shall reject the creation and show an error message
  - Invalid role assignment: If the admin selects an unsupported role, the system shall reject the change and show an error message
- Postconditions:
  - User account changes are stored and take effect immediately
- Acceptance criteria:
  - Admin can create a student account and the student can log in using the provided credentials
  - Admin can disable an account and the disabled user cannot log in
  - Admin can reset a password and the user can log in using the new password

### FR-09 Manage quiz lifecycle (Archive/Delete)
- Description: The system shall allow a teacher (or admin) to manage the lifecycle of quizzes by archiving quizzes and deleting quizzes permanently only when safe
- Primary actors: Teacher, admin
- Preconditions:
  - User is authenticated (FR-01)
  - User has permission to manage quizzes
- Definitions:
  - Archived quiz: A quiz that is hidden from students and cannot be started, but its historical data (attempts/results) is retained for reporting and audit
  - Permanent delete: Physically removes the quiz record from the system database
- Main flow (Archive quiz):
  1. Teacher/admin selects a quiz from the quiz management screen
  2. Teacher/admin chooses "Archive"
  3. System shows a confirmation dialog: "Archive this quiz? Students will not be able to start new attempts"
  4. Teacher/admin confirms
  5. System marks the quiz as Archived and displays a success message: "Quiz archived successfully"
- Effects of archiving:
  - The quiz shall not appear in the student's "Available Quizzes" list (FR-04)
  - The quiz shall not be startable even if the current time is within the scheduled window (FR-07)
  - Teachers/admins shall still be able to view/export reports for existing attempts (FR-06)
- Main flow (Permanent delete quiz):
  1. Teacher/admin selects a quiz
  2. Teacher/admin chooses "Delete permanently"
  3. System validates that the quiz has zero submitted attempts
  4. If valid, system shows a confirmation dialog: "Delete permanently? This action cannot be undone"
  5. Teacher/admin confirms
  6. System deletes the quiz and displays a success message: "Quiz deleted successfully"
- Alternative/exception flows:
  - Quiz has attempts:
    - If the quiz has one or more submitted attempts, the system shall block permanent deletion and show a message:
      "Cannot delete permanently because this quiz has student attempts. Please archive the quiz instead"
- Postconditions:
  - Archive: quiz is hidden from students, historical attempts/results remain available for reporting
  - Permanent delete: quiz is removed only when no attempts exist
- Acceptance criteria:
  - If a quiz has ≥ 1 attempt, permanent delete is blocked and Archive remains available
  - Archived quizzes are not visible/startable by students but remain visible to teacher/admin for reporting/export

---

## Non-Functional Requirements

### NFR-01 Performance
- Requirement:
  - During the project demo, the system shall respond to the "auto-save answer" action within 2 seconds for at least 95 percent of requests under 30 concurrent students
  - During the project demo, the system shall respond to the "start quiz attempt" action within 3 seconds under 30 concurrent students
  - During the project demo, the system shall generate and download the CSV report within 5 seconds for a quiz with up to 100 attempts
- Verification:
  - Perform a controlled demo scenario or a simple load test and record response times for the listed actions
  - Confirm that at least 95 percent of measured response times satisfy the thresholds

### NFR-02 Security
- Requirement:
  - The system shall not store user passwords in plain text
  - The system shall store passwords using bcrypt with a configurable work factor
  - The bcrypt work factor shall be at least 10
  - The system shall enforce a maximum password length of 72 bytes
  - If the provided password exceeds 72 bytes, the system shall reject the operation and show a validation message
  - The system shall enforce role based access control so that student, teacher, and admin can access only features permitted for their roles
- Verification:
  1. Database audit to confirm that no password is stored in plain text and that stored password values are hashed
  2. Validation test: attempt to set/register a password longer than 72 bytes and confirm the system rejects it
  3. Access control testing by attempting to access teacher or admin features using a student account to confirm requests are blocked

### NFR-03 Availability
- Requirement:
  - During a scheduled demo session of at least 2 hours, the system shall operate without service interruption that prevents students from starting or submitting a quiz
  - Planned maintenance during the demo session shall not be performed
- Verification:
  - Observe system operation during the demo session and record any downtime incidents

### NFR-04 Usability
- Requirement: A student shall be able to start a quiz attempt in ≤ 3 steps after logging in
- Verification: Usability walkthrough with test users

### NFR-05 Maintainability
- Requirement: The project shall provide comprehensive documentation in a README file (hosted on GitHub) including environment setup, build instructions, and a logical module architecture (auth, quiz, question, result). The source code shall follow a consistent coding standard to ensure understandability
- Verification: A technical review where a new developer (not in the original team) can successfully set up the development environment and run the system following the README instructions within <= 60 minutes

### NFR-06 Deployment (Docker)
- Requirement:
  - The backend service shall be runnable using Docker
  - The database shall be hosted on Supabase (cloud) and is not packaged in Docker
  - The UI application runs natively on Windows and communicates with the backend via HTTP on localhost during demo
- Verification:
  1. Build the Docker image from the repository.
  2. Run the container and confirm the backend is accessible on localhost and can connect to the Supabase Postgres database.

### NFR-07 Data retention & referential integrity
- Requirement:
  - The system shall retain historical quiz attempt records and results for reporting and later review, even if a quiz is no longer active
  - The system shall prevent operations that would break referential integrity between quizzes and their related attempts/results
  - Permanent deletion of a quiz shall be allowed only when doing so does not orphan related records (i.e., there are zero attempts)
- Verification:
  1. Create a quiz, submit at least one attempt, then attempt "Delete permanently" and confirm the system blocks the operation and suggests archiving
  2. Archive the quiz and confirm:
     - Students cannot see/start the quiz.
     - Teacher/admin can still view/export the report for existing attempts

---

## Data Flow Diagram

## Use Case Diagram

## Class Diagram

## Data Model

## Interface Design Description

## Tools
- Source code & team management: GitHub
- Diagrams (DFD/UML/DB schema): https://app.diagrams.net/
- IDE: VS Code / Visual Studio
- Test cases: Excel
- Bug tracking: Jira / Bugzilla / Mantis
- Packaging/Deployment: Docker
