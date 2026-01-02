# QUIZ EXAMINATION SYSTEM
## Requirement specification & Design document

Project: Quiz examination system  
Course: Software Engineering  
Group: 05
Version: v1.0  
Date: 2025-12-25

---

## System actors
- Student: User who takes quizzes and views their own results
- Teacher: User who creates questions, manages quizzes, and views statistics
- Admin: User responsible for system management and user accounts

## Functional requirements

### FR-01 User authentication
- Description: The system shall allow users (Student/Teacher/Admin) to log in using a username and password
- Primary actors: Student, Teacher, Admin
- Preconditions: The user account exists in the database and is in an active state
- Main flow:
  1. The user enters username and password
  2. The system validates the credentials against the database
  3. The system creates a session and redirects the user to their respective home page
- Alternative/exception flows:
  - Invalid credentials: The system shall show an error message: "Invalid username or password"
  - Account locking: If the user exceeds 5 failed attempts, the system shall temporarily lock the account for 30 minutes
- Postconditions: The user is authenticated and granted access to role-based features
- Acceptance criteria:
  - System grants access only with correct credentials
  - Account locks exactly after the 5th failed attempt

### FR-02 Manage question bank
- Description: The system shall allow a teacher to manage (create, edit, delete) multiple-choice questions in the question bank
- Primary actor: Teacher
- Data fields: Question text, answer options A/B/C/D, correct option(s), difficulty level (Easy/Medium/Hard), topic/tag
- Preconditions: Teacher is logged in and has permission to manage questions
- Main flow (Create):
  1. Teacher selects "Add new questions"
  2. Teacher enters data for all required fields
  3. System validates the data (e.g., at least one correct option selected)
  4. System saves the question and displays a success message
- Exception flows:
  - Duplicate question: System warns if an identical question text already exists
  - Delete active question: If a question is currently in an active quiz, the system shall prevent deletion and show a warning
- Acceptance criteria:
  - Questions are succesfully saved with all data fields
  - Non-authorized users cannot access management functions

### FR-03 Create and configure quiz
- Description: The system shall allow a teacher to create a new quiz by selecting questions and configuring parameters
- Primary actor: Teacher
- Preconditions: Teacher has successfully logged in and has access to at least one question bank
- Main flow:
  1. Teacher selects "Create new quiz"
  2. Teacher enter quiz title and selects a set of questions from the question bank
  3. Teacher configures settings: Time limit (minutes), total points, scoring rules (e.g., negative marking), and "Shuffle questions/answers" options
  4. System validates that the number of selected questions is greater than zero and settings are valid
  5. System saves the quiz and displays a "Quiz created successfully" message
- Exception flows:
  - Insufficient questions: If the teacher requests a random quiz with N questions but the bank has fewer than N, the system shall show an error message
  - Invalid configuration: If time limit or scoring is set to a negative value, the system shall prevent saving and highlight the invalid fields
- Postconditions: A new quiz is stored in the database and is ready to be assigned to students
- Acceptance criteria:
  - The teacher can succesfully save a quiz with all 5 settings configured
  - The shuffle setting must be correctly applied when a student starts the quiz

### FR-04 Take quiz attempt
- Description: The system shall allow a student to select an assigned quiz, perform the attempt within a time limit, and submit their responses for grading
- Primary actor: Student
- Preconditions:
  - Student is authenticated and authorized to the the specific quiz
  - The current time is within the quiz's active window
- Main flow:
  1. Student selects a quiz from the list of available assessments
  2. System initializes the attempt, starts the countdown timer, and displays the first set of questions
  3. Student selects answers and navigates through questions (Previous/Next)
  4. System shall automatically save the current progress when the student navigates between questions
  5. Student confirms and submits the attempt
  6. System records the completion time and final responses into the database
- Alternative/Exception flows:
  - Time expiration: If the countdown reaches zero, the system shall immediately, auto-submit all saved answers, and notify the student
  - Connection loss: System should notify the student and allow them to continue from the last saved state once reconnected (if time permits)
- Postconditions: The attempts is marked as "Submitted", and the data is ready for the grading process
- Acceptance criteria:
  - The timer mathces the "time limit" configured in FR-03
  - Answers are stored correctly even if the session is terminated by time expiration
  - A student cannot start the same quiz attempt more than the allowed number of times

### FR-05 Auto-grading and results
- Description: After a student submits a quiz, the system shall automatically calculate the score based on the pre-defined correct options and store the results
- Primary actor: System
- Preconditions: The student has succesfully submitted the quiz attempt (FR-04)
- Main flow:
  1. The system retrives the student's answers and the correct answers for the quiz from the database
  2. The system compares answer and calculates the total score based on the scoring rules (defined in FR-03)
  3. The system generates an "Attempt summary" including: Total score, number of correct/incorrect answers, and completion time
  4. The system stores the score and summary in the student's history
  5. The system applies the visibility rule: Results shall be shown immediately to the student (unless configured otherwise by the teacher in FR-03)
- Postconditions: The quiz result is permanently stored and linked to the student's account
- Acceptance criteria:
  - Grading must be 100% accurate based on the answer key
  - Results must be stored in the database within < 2 seconds after submission

### FR-06 View and export reports
- Description: The system shall allow teachers and admins to view a list of student attempts for a specific quiz and export these results to a CSV file
- Primary actors: Teacher, admin
- Preconditions: The user is logged in with appropriate permissions and at least one quiz attempt has been submitted
- Main flow:
  1. User selects a specific quiz from the management dashboard
  2. System displays a table of all attempts (Student name, completion date, score, time spent)
  3. User chooses the "Export to CSV" option
  4. System generates and downloads a CSV file containing all displayed data
- Alternative/Exception flows:
  - No attempts found: If no students have taken the quiz, the system shall display a message: "No results available to display or export"
- Postconditions: A CSV file is downloaded to the user's device
- Acceptance criteria:
  - The displayed report must mathc the data in the database for that specific quiz
  - The exported CSV file must be readable by spreadsheet software (e.g., Excel) and contain all required fields

---

## Non-Functional Requirements

### NFR-01 Performance
- Requirement: The system shall respond to "save answer/next question" action within <= 2 seconds for at least 95% of request under 200 concurrent students
- Verification: Perform an automated load test using a testing tool (like JMeter) or a manual test during the project demo to confirm response times match the metrics

### NFR-02 Security
- Requirement: The system shall not store user passwords in plain text. Passwords shall be secured using a strong hashing algorithm (e.g., SHA-256 or Bcrypt) before being stored in the database. Furthermore, the system shall enforce strict role-based access control (RBAC), ensuring that students, teachers, and admins can only access features authorized for their specific roles
- Verification: 
  1. Database audit: Verify that the password column contains only hashed values
  2. Access control testing: Attempt to access teacher/admin URLs using a student account to ensure the system blocks unauthorized requests

### NFR-03 Availability
- Requirement: The system shall maintain an availability of at least 99.9% during all scheduled quiz windows. Total downtime during these critical periods shall not exceed 10 seconds in any single day. Planned maintenance shall be announced at least 48 hours in advance and must not overlap with any scheduled examination
- Verification: Analysis of system uptime logs during a 72-hour operational test to ensure that the total cumulative downtime does not exceed the specified threshold

### NFR-04 Usability
- Requirement: A Student shall be able to start a quiz attempt in ≤ 3 steps after logging in
- Verification: Usability walkthrough with test users

### NFR-05 Maintainability
- Requirement: The project shall provide comprehensive documentation in a README file (hosted on Github) including environment setup, build instruction, and a logical module architecture (auth, quiz, question, result). The source code shall follow a consistent coding standard to ensure understandability
- Verification: A technical review where a new developer (not in the orginal team) can successfully set up the development environment and run the system following the README instructions within <= 60 minutes

---

## Tools
- Source code & team management: GitHub/GitLab
- Diagrams (DFD/UML/DB schema): https://app.diagrams.net/ [web:166]
- IDE: VS Code / Visual Studio / PyCharm / Eclipse / …
- Test cases: Excel
- Bug tracking: Jira / Bugzilla / Mantis
- Packaging/Deployment: Docker
