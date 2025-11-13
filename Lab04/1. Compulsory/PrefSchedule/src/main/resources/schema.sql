-- Schema for PrefSchedule
-- Tables: students, instructors, packs, courses
-- Uses IDENTITY columns for portability between H2 and PostgreSQL (Postgres 10+ supports IDENTITY)

CREATE TABLE IF NOT EXISTS students (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    code VARCHAR(50) NOT NULL UNIQUE,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255),
    student_year INT NOT NULL
);

CREATE TABLE IF NOT EXISTS instructors (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255)
);

CREATE TABLE IF NOT EXISTS packs (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    pack_year INT NOT NULL,
    semester INT NOT NULL,
    name VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS courses (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    type VARCHAR(50) NOT NULL, -- 'COMPULSORY' or 'OPTIONAL'
    code VARCHAR(50) NOT NULL,
    abbr VARCHAR(50),
    name VARCHAR(255) NOT NULL,
    instructor_id BIGINT,
    pack_id BIGINT,
    group_count INT,
    description TEXT,
    CONSTRAINT fk_course_instructor FOREIGN KEY (instructor_id) REFERENCES instructors(id),
    CONSTRAINT fk_course_pack FOREIGN KEY (pack_id) REFERENCES packs(id)
);

-- Notes:
-- - Optional courses can be identified by type = 'OPTIONAL' and are grouped via pack_id.
-- - A student-to-course assignment ("one course from each pack") can be represented by
--   an assignments table (student_id, course_id) and application-level checks or DB
--   constraints that ensure at most one course per (student, pack). For simplicity this
--   script only creates the domain tables requested.
