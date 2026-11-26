-- MySQL C Wrapper demo setup.
--
-- Creates the database, the Cars table and the two routines that
-- C/mysql_c_wrapper/main.c calls, so the demo can be run against a real server.
--
-- Run it with:
--     mysql -u root -p < setup.sql
--
-- On Windows, from the folder this file is in:
--     mysql -u root -p -e "source setup.sql"
--
-- The demo connects as root / test to database testdb on localhost:3306, so
-- either create that account or edit the Connector_connect call in main.c.
-- Note this script drops and recreates the table, so do not run it against a
-- database whose Cars table holds data you care about.

CREATE DATABASE IF NOT EXISTS testdb;
USE testdb;

DROP PROCEDURE IF EXISTS cars_by_make;
DROP FUNCTION IF EXISTS count_cars;
DROP TABLE IF EXISTS Cars;

CREATE TABLE Cars
(
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    make VARCHAR(50) NOT NULL,
    model VARCHAR(50) NOT NULL,
    year SMALLINT UNSIGNED NOT NULL,
    price DECIMAL(10,2) NOT NULL DEFAULT 0.00,
    PRIMARY KEY (id),
    KEY make_year (make, year)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO Cars (make, model, year, price) VALUES
    ('Ford', 'Model T', 1913, 850.00),
    ('Ford', 'Mustang', 1965, 2895.00),
    ('Ford', 'GT', 2005, 149995.00),
    ('Chevrolet', 'Bel Air', 1957, 2499.00),
    ('Chevrolet', 'Corvette', 2019, 55995.00),
    ('Toyota', 'Corolla', 2020, 19600.00),
    ('Dodge', 'Charger', 1969, 3100.00);

-- A make with an apostrophe in it, so the quoting done by Connector_queryf has
-- something to escape. Try:
--     select * from Cars where make = 'O\'Brien Motors'
INSERT INTO Cars (make, model, year, price) VALUES
    ('O''Brien Motors', 'Roadster', 2010, 45000.00);

-- The client needs a different statement delimiter while a routine body is
-- being written; that is what the DELIMITER lines are for, and they are a
-- mysql client feature rather than part of the server's SQL.
DELIMITER $$

-- Returns two result sets on purpose. A procedure can do that, and MySQL will
-- not accept the next statement until both have been read, which is the case
-- Connector_call and Connector_call_rows exist to handle.
CREATE PROCEDURE cars_by_make(IN p_make VARCHAR(50))
BEGIN
    SELECT id, make, model, year, price
    FROM Cars
    WHERE make = p_make
    ORDER BY year;

    SELECT COUNT(*) AS matching_cars
    FROM Cars
    WHERE make = p_make;
END$$

-- DETERMINISTIC and READS SQL DATA are declared because a server running with
-- binary logging enabled refuses to create a function that declares neither.
CREATE FUNCTION count_cars() RETURNS INT
DETERMINISTIC
READS SQL DATA
BEGIN
    DECLARE total INT;
    SELECT COUNT(*) INTO total FROM Cars;
    RETURN total;
END$$

DELIMITER ;

-- Sanity check: with the rows above this prints 8, then the three Ford rows
-- with a matching_cars count of 3, then total_cars 8.
SELECT COUNT(*) AS cars FROM Cars;
CALL cars_by_make('Ford');
SELECT count_cars() AS total_cars;
