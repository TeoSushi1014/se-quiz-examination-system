# QUIZ EXAMINATION SYSTEM
## Requirement specification & Design document

Project: Quiz examination system  
Course: Software Engineering  
Group: 05
Version: v1.4  
Date: 2026-01-14
Repository Github: https://github.com/TeoSushi1014/se-quiz-examination-system.git  
Task assignment sheet: https://docs.google.com/spreadsheets/d/1GyHSU7Leg57oTAMZHbpk6U7AmSn6TJQVEOCwnLIksFs/edit?usp=sharing

## Revision history (Change log)
| Version | Date       | Changes |
|---------|------------|---------|
| v1.0    | 2025-12-25 | Add initial SRS document for Quiz Examination System |
| v1.1    | 2026-01-04 | Enhance SRS document with detailed functional requirements and system scope |
| v1.2    | 2026-01-05 | Add functional requirement for managing quiz lifecycle and data retention |
| v1.3    | 2026-01-12 | Add multiple attempts feature, restructure FR as narrative, add use case descriptions and actor/UC tables |
| v1.4    | 2026-01-14 | Remove UC-05 (Auto grading) as system cannot be primary actor; integrate grading logic into UC-04 postconditions |

## Functional requirements

The Quiz Examination System is a robust, scalable, and secure platform for conducting online multiple-choice tests. The system automates the entire lifecycle of an examination: from question bank management by teachers to automated grading and reporting for students and admins. The system provides the following key functionalities:

**User authentication and account management:**
- Users (Students, Teachers, and Admins) must log in to the system using a username and password. The system validates credentials against the database and creates a session for authenticated users. If a user exceeds 5 failed login attempts, the account is temporarily locked for 30 minutes to prevent unauthorized access
- Authenticated users can change their own password by providing the current password and entering a new password that satisfies the system's password policy. After a successful password change, the user must log in again
- Users can log out at any time to terminate their authenticated session. After logout, all session tokens are invalidated and users cannot access role-based features without logging in again
- The system does not provide a self-service "Forgot password" feature. If a user forgets their password, only an Admin can reset it through the user management interface
- Admins have full control over user account management, including creating new accounts (Student or Teacher), disabling or enabling existing accounts, resetting passwords, and assigning roles. All admin actions are logged with admin identity and timestamp for audit purposes

**Question bank management:**
- Teachers can manage a question bank by creating, editing, and deleting multiple-choice questions. Each question includes question text, four answer options (A, B, C, D), exactly one correct option, difficulty level (easy, medium, hard), and topic or tag for categorization
- When creating or editing questions, the system validates that exactly one correct option is selected. If a teacher attempts to delete a question that is currently used in an active quiz, the system prevents deletion and displays a warning message

**Quiz creation and configuration:**
- Teachers can create new quizzes by selecting questions from the question bank and configuring various parameters including quiz title, time limit (in minutes), total points, scoring rules, question and answer shuffle options, result visibility settings (immediate: show results right after submission; after quiz end: show results only after the scheduled end time of the quiz; or manual release: teacher must manually release results), and maximum attempts allowed per student (a specific number such as 1, 2, 3, or unlimited)
- The maximum attempts setting controls how many times each student can attempt the quiz. If set to a specific number (e.g., 2), students can submit up to that many attempts. If set to unlimited, students can attempt the quiz as many times as they want within the scheduled time window. When a student has multiple attempts, the system records all attempts separately with their individual scores and timestamps
- The scoring system supports single-choice questions where students receive full points for selecting the correct option and zero points for incorrect or unanswered questions. The total score is the sum of points from all questions in the quiz
- The system validates all quiz settings before saving. If the number of selected questions is zero or if parameters such as time limit or total points have invalid values (e.g., negative numbers), the system prevents saving and highlights the invalid fields

**Quiz scheduling and assignment:**
- Teachers can schedule quizzes by specifying a start time and end time, creating a time window during which the quiz is accessible. Teachers assign quizzes to specific students, classes, or groups. The system validates that the start time is earlier than the end time and that at least one student is assigned
- Only assigned students can see and access scheduled quizzes, and only during the scheduled time window. Students outside the assignment or accessing outside the time window cannot start the quiz

**Taking quiz attempts:**
- Students can view their list of available quizzes and select a quiz to begin an attempt. The number of attempts allowed per student depends on the maximum attempts setting configured by the teacher when creating the quiz. If a student has reached the maximum number of allowed attempts, the system prevents starting a new attempt and displays a message indicating the limit has been reached
- When a student starts a quiz, the system initializes the attempt, starts a countdown timer, and displays the questions (shuffled according to the quiz configuration). Students can select answers and navigate through questions using Previous/Next buttons
- The system automatically saves the student's current answers when navigating between questions to prevent data loss. Students can submit their attempt at any time before the timer expires
- If the countdown timer reaches zero before the student submits, the system automatically submits the last saved answers and notifies the student. The completion time and all responses are recorded in the database
- After submitting an attempt, if the student has not reached the maximum attempts limit and the quiz is still within the scheduled time window, the student can start a new attempt. Each attempt is recorded separately with its own timestamp and score

**Automated grading and results:**
- Immediately after a student submits a quiz, the system automatically retrieves the student's answers and compares them against the correct answers stored in the question bank
- The system calculates the total score based on the scoring rules configured for the quiz and generates an attempt summary including total score, number of correct answers, number of incorrect answers, and completion time
- The system stores the results permanently in the student's history and applies the visibility rule configured by the teacher. If the result visibility is set to "immediate", students can view their results right away. If set to "manual release", results are stored but hidden until the teacher explicitly releases them

**Viewing and exporting reports:**
- Teachers and Admins can view comprehensive reports of student attempts for specific quizzes. The report displays a table of all attempts including student username, attempt number (e.g., attempt 1 of 3), completion date, score, and time spent. When a student has multiple attempts, all attempts are displayed in the report with their individual scores
- Users can export these results to a CSV file that can be opened in spreadsheet software such as Excel. The exported file contains all displayed data fields including all attempts for each student. If no attempts exist for a quiz, the system displays a message indicating no results are available
- For student view, when a student has multiple attempts, the system displays all their attempts with scores, and highlights the best score achieved

**Quiz and attempt deletion:**
- Teachers can delete quizzes that have no existing student attempts. If a quiz has one or more submitted attempts, the system blocks teacher deletion and displays a clear message
- Admins have elevated privileges and can permanently purge quizzes even if they have student attempts. When an admin purges a quiz, the system deletes the quiz along with all related attempt records and results. All purge actions are logged with admin identity and timestamp
- Teachers and admins can permanently delete individual student attempts (useful for removing invalid attempts due to technical errors or granting additional attempts beyond the configured limit). Teachers can only delete attempts for quizzes they created, while admins can delete attempts for any quiz. After an attempt is deleted, all related data including saved answers are removed. If the student has not reached the maximum attempts limit after deletion, they can start a new attempt

---

## Non-functional requirements

### NFR-01 Performance
- Description: The system shall respond to critical actions (auto-save answer, start quiz attempt, generate CSV report) within specified time limits under concurrent load during demo

### NFR-02 Security
- Description: The system shall not store user passwords in plain text and shall enforce role based access control

### NFR-03 Availability
- Description: During a scheduled demo session, the system shall operate without service interruption

### NFR-04 Usability
- Description: A student shall be able to start a quiz attempt in ≤ 3 steps after logging in

### NFR-05 Maintainability
- Description: The project shall provide comprehensive documentation for environment setup, build instructions, and logical module architecture

### NFR-06 Deployment (Docker)
- Description: The backend service shall be runnable using Docker while the database is hosted on Supabase cloud

### NFR-07 Privileged deletion safety and referential integrity
- Description: The system shall enforce referential integrity when deleting data and support admin purge with proper logging

### NFR-08 Referential integrity for attempt deletion
- Description: The system shall ensure referential integrity when permanently deleting attempts and their related answer records

---

## Data flow diagram

### DFD level 0
![DFD Level 0](dfd/image/DFD-level0.png)

### DFD level 1
![DFD Level 1](dfd/image/DFD_level1.png)

### DFD level 2 - Authentication
![DFD Level 2 - Authentication](dfd/image/DFD_level2-Authentication.png)

### DFD level 2 - Question bank management
![DFD Level 2 - Question Bank Management](dfd/image/DFD_level2-Question%20Bank%20Management.png)

### DFD level 2 - Quiz management
![DFD Level 2 - Quiz Management](dfd/image/DFD_level2-Quiz%20Management.png)

### DFD level 2 - Attempt management
![DFD Level 2 - Attempt Management](dfd/image/DFD-level2_Attempt%20Management.png)

### DFD level 2 - Grading and results
![DFD Level 2 - Grading and Results](dfd/image/DFD-level2_Grading_Results.png)

## Use case diagram

![Use Case Diagram](uc/uc.png)

### Actors

| No. | Actor   | Description                                                                                              |
|-----|---------|----------------------------------------------------------------------------------------------------------|
| 1   | Student | User who takes quizzes and views their own results                                                       |
| 2   | Teacher | User who creates questions, manages quizzes, views statistics and student results                        |
| 3   | Admin   | User responsible for system management and user accounts (create, disable, reset password, assign roles) |

### Use cases

| No. | Use Case                    | Description                                                                                        |
|-----|-----------------------------|----------------------------------------------------------------------------------------------------|
| 1   | User authentication         | Log in to the system using username and password                                                   |
| 2   | Manage question bank        | Manage (create, edit, delete) multiple choice questions in the question bank                       |
| 3   | Create and configure quiz   | Create a new quiz by selecting questions and configuring parameters                                |
| 4   | Take quiz attempt           | Student selects a quiz, performs the attempt within time limit, and submits responses              |
| 5   | Auto grading and results    | System automatically calculates score based on correct answers and stores results                  |
| 6   | View and export reports     | View list of student attempts for a specific quiz and export results to CSV file                   |
| 7   | Schedule and assign quiz    | Schedule and assign quiz to specific students within a time window                                 |
| 8   | Manage user accounts        | Manage user accounts (create, disable, reset password, assign roles)                               |
| 9   | Delete and purge quiz       | Delete quiz (teacher: only with no attempts, admin: permanently delete even with attempts)         |
| 10  | Delete student attempt      | Permanently delete a student's attempt so the student can start a new attempt                      |
| 11  | User logout                 | Log out and terminate the current authenticated session                                            |
| 12  | Change password             | Change own password                                                                                |
| 13  | Forgot password             | Admin resets password for users (no self-service reset feature)                                    |

## Use case specifications

### UC-01 User authentication
- Description: The system allows users (Student/Teacher/Admin) to log in using a username and password
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

### UC-02 Manage question bank
- Description: The system allows a teacher to manage (create, edit, delete) multiple choice questions in the question bank
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

### UC-03 Create and configure quiz
- Description: The system allows a teacher to create a new quiz by selecting questions and configuring parameters (time limit, scoring rules, shuffle options, result visibility, maximum attempts)
- Primary actor: teacher
- Preconditions: teacher has successfully logged in and has access to at least one question bank
- Main flow:
  1. Teacher selects "Create new quiz"
  2. Teacher enters quiz title and selects a set of questions from the question bank
  3. Teacher configures settings: time limit in minutes, total points, scoring rules, shuffle questions, shuffle answers options, result visibility (immediate: show after submission / after quiz end: show after scheduled end time / manual release: teacher releases manually), and maximum attempts per student (specific number or unlimited)
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

### UC-04 Take quiz attempt
- Description: The system allows a student to select an assigned quiz, perform the attempt within a time limit, and submit their responses for grading
- Primary actor: Student
- Preconditions:
  - Student is authenticated and authorized for the specific quiz
  - The current time is within the scheduled time window of the quiz (FR-07)
  - Student has not reached the maximum attempts limit configured for this quiz
- Attempt limit:
  - The number of attempts allowed is determined by the maximum attempts setting configured by the teacher when creating the quiz (can be a specific number like 1, 2, 3, or unlimited)
- Main flow:
  1. Student selects a quiz from the list of available assessments
  2. System initializes the attempt, starts the countdown timer, and displays the first set of questions
  3. Student selects answers and navigates through questions (Previous/Next)
  4. The system shall automatically save the student's current answers when the student navigates between questions
  5. Student confirms and submits the attempt
  6. The system records the completion time and final responses into the database
- Alternative/Exception flows:
  - Time expiration:
    - If the countdown reaches zero, the system shall auto submit the last saved answers and notify the student
  - Maximum attempts reached:
    - If the student has already reached the maximum number of allowed attempts for this quiz, the system shall prevent starting a new attempt and show a message indicating the limit has been reached and displaying all previous attempt scores
- Postconditions:
  - The attempt is marked as submitted
  - The system automatically calculates the score by comparing the student's answers against the correct answers from the question bank based on the scoring rules (UC-03)
  - The system generates an attempt summary including total score, number of correct answers, number of incorrect answers, and completion time
  - The system stores the results permanently in the student's history
  - The system applies the result visibility rule: if set to "immediate", results are shown right away; if set to "after quiz end", results are hidden until the scheduled end time passes; if set to "manual release", results are hidden until the teacher explicitly releases them
- Acceptance criteria:
  - A student cannot exceed the maximum attempts limit configured for the quiz
  - Answers are stored correctly even if the attempt is auto submitted due to time expiration
  - Each attempt is recorded separately with its own timestamp and score
  - Grading must be accurate based on the answer key
  - During the project demo under 30 concurrent students, results must be stored in the database within 5 seconds after submission

### UC-05 View and export reports
- Description: The system allows teachers and admins to view a list of student attempts for a specific quiz and export these results to a CSV file
- Primary actors: Teacher, admin
- Preconditions: The user is logged in with appropriate permissions and at least one quiz attempt exists
- Main flow:
  1. User selects a specific quiz from the management dashboard
  2. System displays a table of all existing attempts (student username, attempt number, completion date, score, time spent). When a student has multiple attempts, all attempts are listed separately
  3. User chooses the "Export to CSV" option
  4. System generates and downloads a CSV file containing all displayed data including all attempts for each student
- Alternative/Exception flows:
  - No attempts found:
    - If no attempts exist for the quiz, the system shall display a message: "No results available to display or export"
- Postconditions: A CSV file is downloaded to the user's device
- Acceptance criteria:
  - The displayed report must match the data in the database for that specific quiz
  - The exported CSV file must be readable by spreadsheet software (e.g., Excel) and contain all required fields

### UC-06 Schedule and assign quiz
- Description: The system allows a teacher to schedule a quiz by specifying a start time and an end time, and assign the quiz to specific students (or a class or group)
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
- Alternative/Exception flows:
  - Invalid time window: If the start time is not earlier than the end time, the system shall prevent saving and show an error message
  - No students assigned: If no student or class is selected, the system shall prevent saving and show an error message
- Postconditions:
  - The quiz schedule and assignment are stored
  - Students can start an attempt only if they are assigned and the current time is within the scheduled time window
- Acceptance criteria:
  - Unassigned students cannot see or start the quiz
  - Assigned students can start the quiz only during the scheduled time window
  - The system rejects invalid time windows and empty assignments

### UC-07 Manage user accounts
- Description: The system allows an admin to manage user accounts including creating accounts, disabling accounts, resetting passwords, and assigning roles
- Primary actor: Admin
- Preconditions:
  - Admin is authenticated (FR-01)
- Main flow:
  1. Admin opens the user management screen
  2. Admin creates a new user by entering username, initial password, and selecting a role
  3. Admin disables or enables an existing user account
  4. Admin resets a user password and provides a new initial password to the user
  5. The system saves the changes and logs the action with the admin identity and time
- Alternative/Exception flows:
  - Duplicate username: If the username already exists, the system shall reject the creation and show an error message
  - Invalid role assignment: If the admin selects an unsupported role, the system shall reject the change and show an error message
- Postconditions:
  - User account changes are stored and take effect immediately
- Acceptance criteria:
  - Admin can create a student account and the student can log in using the provided credentials
  - Admin can disable an account and the disabled user cannot log in
  - Admin can reset a password and the user can log in using the new password

### UC-08 Delete and purge quiz
- Description: The system allows teacher to delete a quiz only when it has no existing attempts and allows admin to purge a quiz permanently even if it has attempts
- Primary actors: Teacher, admin
- Preconditions:
  - User is authenticated (FR-01)
  - User has permission to manage quizzes
- Main flow (Teacher delete quiz with zero attempts):
  1. Teacher selects a quiz from the quiz management screen
  2. Teacher chooses "Delete"
  3. System validates that the quiz has zero existing attempts
  4. System shows a confirmation dialog: "Delete this quiz? This action cannot be undone"
  5. Teacher confirms
  6. System deletes the quiz and displays a success message: "Quiz deleted successfully"
- Main flow (Admin purge quiz permanently):
  1. Admin selects a quiz from the quiz management screen
  2. Admin chooses "Purge permanently"
  3. System shows a confirmation dialog: "Purge permanently? This will delete the quiz and all attempts results and cannot be undone"
  4. Admin confirms
  5. System permanently deletes the quiz and all related attempts results
  6. System displays a success message: "Quiz purged successfully"
- Alternative/Exception flows:
  - Teacher delete blocked due to attempts:
    - If the quiz has one or more submitted attempts, the system shall block teacher deletion and show a message: "Cannot delete because this quiz has student attempts"
  - Non admin purge blocked:
    - If a non admin user attempts to purge, the system shall block and show a message: "Only admin can purge data permanently"
- Postconditions:
  - Teacher delete removes only quizzes with zero attempts
  - Admin purge removes the quiz and all related attempts results
- Acceptance criteria:
  - Teacher can delete a quiz only when the quiz has zero existing attempts
  - Admin can purge a quiz even when the quiz has one or more submitted attempts
  - After admin purge, related attempts results are removed and the quiz no longer appears in reports

### UC-09 Delete student attempt
- Description: The system allows teacher and admin to permanently delete a student's quiz attempt from the database (useful for removing invalid attempts or granting additional attempts beyond the configured limit)
- Primary actors: Teacher, admin
- Preconditions:
  - User is authenticated (FR-01)
  - The quiz exists and the attempt exists
  - The attempt belongs to the selected quiz
- Authorization rules:
  - Teacher can delete attempts only for quizzes created by that teacher
  - Admin can delete attempts for any quiz
- Main flow:
  1. Teacher admin selects a quiz from the management dashboard
  2. System displays the list of attempts for that quiz
  3. Teacher admin selects a specific student attempt
  4. Teacher admin chooses "Delete attempt"
  5. System shows a confirmation dialog: "Delete this attempt permanently? This action cannot be undone"
  6. Teacher admin confirms
  7. System permanently deletes the attempt from the database
  8. System permanently deletes all related attempt data including saved answers for that attempt
  9. System allows the student to start a new attempt for that quiz
- Alternative/Exception flows:
  - Unauthorized access:
    - If teacher tries to delete an attempt for a quiz not owned by that teacher the system shall block the operation and show a clear message
  - Attempt not found:
    - If the attempt no longer exists the system shall show an error message and refresh the attempt list
- Postconditions:
  - The deleted attempt is removed from the database and is no longer accessible
  - If the student has not reached the maximum attempts limit after deletion, they can start a new attempt for that quiz
- Acceptance criteria:
  - Teacher admin can delete an attempt and the student can start a new attempt
  - After deletion the attempt and its related answers do not appear in any report export or history query

### UC-10 User logout
- Description: The system allows an authenticated user (Student/Teacher/Admin) to log out and terminate the current authenticated session
- Primary actors: Student, teacher, admin
- Preconditions:
  - The user is authenticated (FR-01) and has an active session
- Main flow:
  1. The user selects "Logout"
  2. The system invalidates the user's current session/token on the server
  3. The system removes the authentication credential on the client (e.g., deletes auth cookie / clears stored token)
  4. The system navigates the user to the login screen
- Alternative/Exception flows:
  - Session already expired:
    - If the session is already expired, the system shall still navigate the user to the login screen and show a message: "Session expired. Please log in again"
- Postconditions:
  - The user is no longer authenticated and cannot access any role-based features without logging in again
- Acceptance criteria:
  - After logout, using the browser back button (or reopening the app window) shall not restore access to authenticated pages
  - Any request made with the old session/token after logout shall be rejected by the backend

### UC-11 Change password
- Description: The system allows an authenticated user (Student/Teacher/Admin) to change their password
- Primary actors: Student, teacher, admin
- Preconditions:
  - The user is authenticated (FR-01)
  - The user account is active
- Data fields:
  - Current password
  - New password
  - Confirm new password
- Main flow:
  1. The user opens "Account settings" and selects "Change password"
  2. The user enters current password, new password, and confirm new password
  3. The system validates:
     - Current password is correct
     - New password and confirm new password match
     - New password satisfies password policy (NFR-02)
  4. The system updates the stored password hash
  5. The system invalidates the user's current session and requires the user to log in again
  6. The system shows a success message: "Password changed successfully. Please log in again"
- Alternative/Exception flows:
  - Incorrect current password:
    - The system shall show an error message: "Current password is incorrect"
  - New password mismatch:
    - The system shall show an error message: "New password and confirmation do not match"
  - Policy violation:
    - If the new password violates NFR-02 constraints (e.g., too long), the system shall reject the change and highlight invalid fields
- Postconditions:
  - The new password takes effect immediately
  - The user must re-authenticate to access the system again
- Acceptance criteria:
  - The user cannot log in using the old password after a successful password change
  - The user can log in using the new password after a successful password change

### UC-12 Forgot password
- Description: The system does not provide a self-service "Forgot password" feature for end users. Password reset is performed only by admin via user management functions
- Primary actor: Admin
- Supporting actors: Student, teacher
- Preconditions:
  - Admin is authenticated (FR-01)
  - The target user account exists and is in an active state
- Main flow:
  1. Admin opens the user management screen (FR-08)
  2. Admin selects a target user account
  3. Admin chooses "Reset password"
  4. Admin provides a new initial password for the user (or system generates one based on implementation)
  5. The system updates the stored password hash according to the password storage policy (NFR-02)
  6. The system invalidates any active sessions/tokens for that user (if applicable)
  7. The system logs the action with admin identity and time (FR-08)
  8. The system displays a success message: "Password reset successfully"
- Alternative/Exception flows:
  - User not found:
    - If the selected user no longer exists, the system shall show an error message and refresh the user list
  - Disabled account:
    - If the account is disabled, the system shall allow admin to reset password but the user still cannot log in until the account is enabled
- Postconditions:
  - The target user can log in using the new password (only if the account is enabled)
- Acceptance criteria:
  - The system does not show any "Forgot password" or self-service reset function on the login screen
  - After admin reset, the old password can no longer be used
  - All existing sessions of the target user are invalidated after reset (if the system uses sessions/tokens)

## Non-functional requirements specifications

### NFR-01 Performance
- Requirements:
  - During the project demo, the system shall respond to the "auto-save answer" action within 2 seconds for at least 95 percent of requests under 30 concurrent students
  - During the project demo, the system shall respond to the "start quiz attempt" action within 3 seconds under 30 concurrent students
  - During the project demo, the system shall generate and download the CSV report within 5 seconds for a quiz with up to 100 attempts
- Verification:
  - Perform a controlled demo scenario or a simple load test and record response times for the listed actions
  - Confirm that at least 95 percent of measured response times satisfy the thresholds

### NFR-02 Security
- Requirements:
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
- Requirements:
  - During a scheduled demo session of at least 2 hours, the system shall operate without service interruption that prevents students from starting or submitting a quiz
  - Planned maintenance during the demo session shall not be performed
- Verification:
  - Observe system operation during the demo session and record any downtime incidents

### NFR-04 Usability
- Requirements: A student shall be able to start a quiz attempt in ≤ 3 steps after logging in
- Verification: Usability walkthrough with test users

### NFR-05 Maintainability
- Requirements: The project shall provide comprehensive documentation in a README file (hosted on GitHub) including environment setup, build instructions, and a logical module architecture (auth, quiz, question, result). The source code shall follow a consistent coding standard to ensure understandability
- Verification: A technical review where a new developer (not in the original team) can successfully set up the development environment and run the system following the README instructions within <= 60 minutes

### NFR-06 Deployment (Docker)
- Requirements:
  - The backend service shall be runnable using Docker
  - The database shall be hosted on Supabase (cloud) and is not packaged in Docker
  - The UI application runs natively on Windows and communicates with the backend via HTTP on localhost during demo
- Verification:
  1. Build the Docker image from the repository.
  2. Run the container and confirm the backend is accessible on localhost and can connect to the Supabase Postgres database.

### NFR-07 Privileged deletion safety and referential integrity
- Requirements:
  - The system shall enforce referential integrity when deleting data that has relationships using the database foreign key delete rules
  - The system shall support admin purge by deleting related attempts results together with the quiz to avoid foreign key constraint errors
  - The system shall log admin purge actions with admin identity and timestamp
- Verification:
  1. Create a quiz and submit at least one attempt then purge as admin and confirm the quiz and related attempts results are removed
  2. Attempt purge as teacher and confirm it is blocked
  3. Attempt teacher delete on a quiz with attempts and confirm it is blocked

### NFR-08 Referential integrity for attempt deletion
- Requirements:
  - The system shall ensure referential integrity when permanently deleting attempts and their related answer records
  - The database schema shall support deleting an attempt together with its related answer records without foreign key constraint errors
- Verification:
  1. Create a quiz and submit an attempt then delete the attempt and confirm the attempt and related answers are removed
  2. Confirm the student can start a new attempt after deletion

## Class diagram

![Class Diagram](classdiagram/classdiagram.png)

## Data model

![Data Model](datamodel/datamodel.png)

## Interface design description

### Login screen
![Login Screen](ui/Slide_001.png)

### Student dashboard
![Student Dashboard](ui/Slide_002.png)

### Quiz selection
![Quiz Selection](ui/Slide_003.png)

### Quiz instructions
![Quiz Instructions](ui/Slide_004.png)

### Quiz attempt
![Quiz Attempt](ui/Slide_005.png)

### Quiz submit confirmation
![Quiz Submit Confirmation](ui/Slide_006.png)

### Quiz result
![Quiz Result](ui/Slide_007.png)

### Teacher dashboard
![Teacher Dashboard](ui/Slide_008.png)

### Question bank management
![Question Bank Management](ui/Slide_009.png)

### Add new question
![Add New Question](ui/Slide_010.png)

### Edit question
![Edit Question](ui/Slide_011.png)

### Quiz management
![Quiz Management](ui/Slide_012.png)

### Create new quiz
![Create New Quiz](ui/Slide_013.png)

### Configure quiz settings
![Configure Quiz Settings](ui/Slide_014.png)

### Schedule and assign quiz
![Schedule and Assign Quiz](ui/Slide_015.png)

### View quiz results
![View Quiz Results](ui/Slide_016.png)

### Admin dashboard
![Admin Dashboard](ui/Slide_017.png)

### User management
![User Management](ui/Slide_018.png)

### System reports
![System Reports](ui/Slide_019.png)

## Tools
- Source code & team management: GitHub
- Diagrams (DFD/UML/DB schema): https://app.diagrams.net/
- IDE: VS Code / Visual Studio
- Test cases: Excel
- Bug tracking: Jira / Bugzilla / Mantis
- Packaging/Deployment: Docker