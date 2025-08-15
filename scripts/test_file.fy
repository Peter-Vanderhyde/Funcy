/* University Student Management System */

class Course {
    func &Course(course_name, max_students) {
        &course_name = course_name;
        &max_students = max_students;
        &students = list();
    }

    func &enrollStudent(student) {
        if &students.size() < &max_students {
            &students.append(student);
            print("Enrolled " + student.getName() + " in " + &course_name);
            return true;
        } else {
            print("Enrollment failed: " + &course_name + " is full.");
            return false;
        }
    }

    func &listStudents() {
        if &students.size() == 0 {
            print("No students enrolled in " + &course_name);
        } else {
            print("Students in " + &course_name + ":");
            for student in &students {
                print("- " + student.getName() + " (" + str(student.getAge()) + " years old)");
            }
        }
    }
}

class Student {
    func &Student(name, age) {
        &name = name;
        &age = age;
        &courses = list();
    }

    func &getName() {
        return &name;
    }

    func &getAge() {
        return &age;
    }

    func &enroll(course) {
        if course not in &courses {
            if course.enrollStudent(this) {
                &courses.append(course);
            }
        } else {
            print(&name + " is already enrolled in " + course.course_name);
        }
    }

    func &listCourses() {
        if &courses.size() == 0 {
            print(&name + " is not enrolled in any courses.");
        } else {
            print(&name + "'s Courses:");
            for course in &courses {
                print("- " + course.course_name);
            }
        }
    }
}

class University {
    func &University(name) {
        &name = name;
        &courses = list();
        &students = list();
    }

    func &addCourse(course_name, max_students) {
        new_course = Course(course_name, max_students);
        &courses.append(new_course);
        print("Added course: " + course_name);
        return new_course;
    }

    func &addStudent(name, age) {
        new_student = Student(name, age);
        &students.append(new_student);
        print("Added student: " + name);
        return new_student;
    }

    func &listAllCourses() {
        if &courses.size() == 0 {
            print("No courses available in the university.");
        } else {
            print("Courses in " + &name + ":");
            for course in &courses {
                print("- " + course.course_name + " (Max students: " + str(course.max_students) + ")");
            }
        }
    }

    func &listAllStudents() {
        if &students.size() == 0 {
            print("No students are registered in the university.");
        } else {
            print("Students in " + &name + ":");
            for student in &students {
                print("- " + student.getName() + " (" + str(student.getAge()) + " years old)");
            }
        }
    }
}

func main() {
    uni = University("Tech University");

    # Add courses
    course_math = uni.addCourse("Mathematics", 2);
    course_cs = uni.addCourse("Computer Science", 3);
    course_physics = uni.addCourse("Physics", 2);

    # Add students
    student_john = uni.addStudent("John Doe", 20);
    student_jane = uni.addStudent("Jane Smith", 19);
    student_anna = uni.addStudent("Anna Lee", 22);

    # Enroll students in courses
    student_john.enroll(course_math);
    student_john.enroll(course_cs);
    student_jane.enroll(course_math);
    student_jane.enroll(course_physics);
    student_anna.enroll(course_cs);
    student_anna.enroll(course_physics);

    # Attempt to over-enroll a course
    student_anna.enroll(course_math);

    # List all courses and their students
    uni.listAllCourses();
    course_math.listStudents();
    course_cs.listStudents();
    course_physics.listStudents();

    # List all students and their courses
    uni.listAllStudents();
    student_john.listCourses();
    student_jane.listCourses();
    student_anna.listCourses();
}

main();